/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "AudioStreamBuilder"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <new>
#include <stdint.h>

#include <aaudio/AAudio.h>
#include <aaudio/AAudioTesting.h>

#include "binding/AAudioBinderClient.h"
#include "client/AudioStreamInternalCapture.h"
#include "client/AudioStreamInternalPlay.h"
#include "core/AudioStream.h"
#include "core/AudioStreamBuilder.h"
#include "legacy/AudioStreamRecord.h"
#include "legacy/AudioStreamTrack.h"

using namespace aaudio;

#define AAUDIO_MMAP_POLICY_DEFAULT             AAUDIO_POLICY_NEVER
#define AAUDIO_MMAP_EXCLUSIVE_POLICY_DEFAULT   AAUDIO_POLICY_NEVER

// These values are for a pre-check before we ask the lower level service to open a stream.
// So they are just outside the maximum conceivable range of value,
// on the edge of being ridiculous.
// TODO These defines should be moved to a central place in audio.
#define SAMPLES_PER_FRAME_MIN        1
// TODO Remove 8 channel limitation.
#define SAMPLES_PER_FRAME_MAX        FCC_8
#define SAMPLE_RATE_HZ_MIN           8000
// HDMI supports up to 32 channels at 1536000 Hz.
#define SAMPLE_RATE_HZ_MAX           1600000
#define FRAMES_PER_DATA_CALLBACK_MIN 1
#define FRAMES_PER_DATA_CALLBACK_MAX (1024 * 1024)

/*
 * AudioStreamBuilder
 */
AudioStreamBuilder::AudioStreamBuilder() {
}

AudioStreamBuilder::~AudioStreamBuilder() {
}

static aaudio_result_t builder_createStream(aaudio_direction_t direction,
                                         aaudio_sharing_mode_t sharingMode,
                                         bool tryMMap,
                                         AudioStream **audioStreamPtr) {
    *audioStreamPtr = nullptr;
    aaudio_result_t result = AAUDIO_OK;

    switch (direction) {

        case AAUDIO_DIRECTION_INPUT:
            if (tryMMap) {
                *audioStreamPtr = new AudioStreamInternalCapture(AAudioBinderClient::getInstance(),
                                                                 false);
            } else {
                *audioStreamPtr = new AudioStreamRecord();
            }
            break;

        case AAUDIO_DIRECTION_OUTPUT:
            if (tryMMap) {
                *audioStreamPtr = new AudioStreamInternalPlay(AAudioBinderClient::getInstance(),
                                                              false);
            } else {
                *audioStreamPtr = new AudioStreamTrack();
            }
            break;

        default:
            ALOGE("%s() bad direction = %d", __func__, direction);
            result = AAUDIO_ERROR_ILLEGAL_ARGUMENT;
    }
    return result;
}

// Try to open using MMAP path if that is allowed.
// Fall back to Legacy path if MMAP not available.
// Exact behavior is controlled by MMapPolicy.
aaudio_result_t AudioStreamBuilder::build(AudioStream** streamPtr) {
    AudioStream *audioStream = nullptr;
    if (streamPtr == nullptr) {
        ALOGE("%s() streamPtr is null", __func__);
        return AAUDIO_ERROR_NULL;
    }
    *streamPtr = nullptr;

    logParameters();

    aaudio_result_t result = validate();
    if (result != AAUDIO_OK) {
        return result;
    }

    // The API setting is the highest priority.
    aaudio_policy_t mmapPolicy = AAudio_getMMapPolicy();
    // If not specified then get from a system property.
    if (mmapPolicy == AAUDIO_UNSPECIFIED) {
        mmapPolicy = AAudioProperty_getMMapPolicy();
    }
    // If still not specified then use the default.
    if (mmapPolicy == AAUDIO_UNSPECIFIED) {
        mmapPolicy = AAUDIO_MMAP_POLICY_DEFAULT;
    }

    int32_t mapExclusivePolicy = AAudioProperty_getMMapExclusivePolicy();
    if (mapExclusivePolicy == AAUDIO_UNSPECIFIED) {
        mapExclusivePolicy = AAUDIO_MMAP_EXCLUSIVE_POLICY_DEFAULT;
    }

    aaudio_sharing_mode_t sharingMode = getSharingMode();
    if ((sharingMode == AAUDIO_SHARING_MODE_EXCLUSIVE)
        && (mapExclusivePolicy == AAUDIO_POLICY_NEVER)) {
        ALOGD("%s() EXCLUSIVE sharing mode not supported. Use SHARED.", __func__);
        sharingMode = AAUDIO_SHARING_MODE_SHARED;
        setSharingMode(sharingMode);
    }

    bool allowMMap = mmapPolicy != AAUDIO_POLICY_NEVER;
    bool allowLegacy = mmapPolicy != AAUDIO_POLICY_ALWAYS;

    // TODO Support other performance settings in MMAP mode.
    // Disable MMAP if low latency not requested.
    if (getPerformanceMode() != AAUDIO_PERFORMANCE_MODE_LOW_LATENCY) {
        ALOGD("%s() MMAP not available because AAUDIO_PERFORMANCE_MODE_LOW_LATENCY not used.",
              __func__);
        allowMMap = false;
    }

    // SessionID and Effects are only supported in Legacy mode.
    if (getSessionId() != AAUDIO_SESSION_ID_NONE) {
        ALOGD("%s() MMAP not available because sessionId used.", __func__);
        allowMMap = false;
    }

    if (!allowMMap && !allowLegacy) {
        ALOGE("%s() no backend available: neither MMAP nor legacy path are allowed", __func__);
        return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
    }

    result = builder_createStream(getDirection(), sharingMode, allowMMap, &audioStream);
    if (result == AAUDIO_OK) {
        // Open the stream using the parameters from the builder.
        result = audioStream->open(*this);
        if (result == AAUDIO_OK) {
            *streamPtr = audioStream;
        } else {
            bool isMMap = audioStream->isMMap();
            delete audioStream;
            audioStream = nullptr;

            if (isMMap && allowLegacy) {
                ALOGV("%s() MMAP stream did not open so try Legacy path", __func__);
                // If MMAP stream failed to open then TRY using a legacy stream.
                result = builder_createStream(getDirection(), sharingMode,
                                              false, &audioStream);
                if (result == AAUDIO_OK) {
                    result = audioStream->open(*this);
                    if (result == AAUDIO_OK) {
                        *streamPtr = audioStream;
                    } else {
                        delete audioStream;
                    }
                }
            }
        }
    }

    return result;
}

aaudio_result_t AudioStreamBuilder::validate() const {

    // Check for values that are ridiculously out of range to prevent math overflow exploits.
    // The service will do a better check.
    aaudio_result_t result = AAudioStreamParameters::validate();
    if (result != AAUDIO_OK) {
        return result;
    }

    switch (mPerformanceMode) {
        case AAUDIO_PERFORMANCE_MODE_NONE:
        case AAUDIO_PERFORMANCE_MODE_POWER_SAVING:
        case AAUDIO_PERFORMANCE_MODE_LOW_LATENCY:
            break;
        default:
            ALOGE("illegal performanceMode = %d", mPerformanceMode);
            return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
            // break;
    }

    // Prevent ridiculous values from causing problems.
    if (mFramesPerDataCallback != AAUDIO_UNSPECIFIED
        && (mFramesPerDataCallback < FRAMES_PER_DATA_CALLBACK_MIN
            || mFramesPerDataCallback > FRAMES_PER_DATA_CALLBACK_MAX)) {
        ALOGE("framesPerDataCallback out of range = %d",
              mFramesPerDataCallback);
        return AAUDIO_ERROR_OUT_OF_RANGE;
    }

    return AAUDIO_OK;
}

static const char *AAudio_convertSharingModeToShortText(aaudio_sharing_mode_t sharingMode) {
    switch (sharingMode) {
        case AAUDIO_SHARING_MODE_EXCLUSIVE:
            return "EX";
        case AAUDIO_SHARING_MODE_SHARED:
            return "SH";
        default:
            return "?!";
    }
}

static const char *AAudio_convertDirectionToText(aaudio_direction_t direction) {
    switch (direction) {
        case AAUDIO_DIRECTION_OUTPUT:
            return "OUTPUT";
        case AAUDIO_DIRECTION_INPUT:
            return "INPUT";
        default:
            return "?!";
    }
}

void AudioStreamBuilder::logParameters() const {
    // This is very helpful for debugging in the future. Please leave it in.
    ALOGI("rate   = %6d, channels  = %d, format   = %d, sharing = %s, dir = %s",
          getSampleRate(), getSamplesPerFrame(), getFormat(),
          AAudio_convertSharingModeToShortText(getSharingMode()),
          AAudio_convertDirectionToText(getDirection()));
    ALOGI("device = %6d, sessionId = %d, perfMode = %d, callback: %s with frames = %d",
          getDeviceId(),
          getSessionId(),
          getPerformanceMode(),
          ((getDataCallbackProc() != nullptr) ? "ON" : "OFF"),
          mFramesPerDataCallback);
    ALOGI("usage  = %6d, contentType = %d, inputPreset = %d, allowedCapturePolicy = %d",
          getUsage(), getContentType(), getInputPreset(), getAllowedCapturePolicy());
}
