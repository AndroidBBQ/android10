/*
 * Copyright 2017 The Android Open Source Project
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


#define LOG_TAG "AAudioStreamParameters"
#include <utils/Log.h>
#include <system/audio.h>

#include "AAudioStreamParameters.h"

using namespace aaudio;

// TODO These defines should be moved to a central place in audio.
#define SAMPLES_PER_FRAME_MIN        1
// TODO Remove 8 channel limitation.
#define SAMPLES_PER_FRAME_MAX        FCC_8
#define SAMPLE_RATE_HZ_MIN           8000
// HDMI supports up to 32 channels at 1536000 Hz.
#define SAMPLE_RATE_HZ_MAX           1600000

AAudioStreamParameters::AAudioStreamParameters() {}
AAudioStreamParameters::~AAudioStreamParameters() {}

void AAudioStreamParameters::copyFrom(const AAudioStreamParameters &other) {
    mSamplesPerFrame      = other.mSamplesPerFrame;
    mSampleRate           = other.mSampleRate;
    mDeviceId             = other.mDeviceId;
    mSessionId            = other.mSessionId;
    mSharingMode          = other.mSharingMode;
    mAudioFormat          = other.mAudioFormat;
    mDirection            = other.mDirection;
    mBufferCapacity       = other.mBufferCapacity;
    mUsage                = other.mUsage;
    mContentType          = other.mContentType;
    mInputPreset          = other.mInputPreset;
    mAllowedCapturePolicy = other.mAllowedCapturePolicy;
}

static aaudio_result_t isFormatValid(audio_format_t format) {
    switch (format) {
        case AUDIO_FORMAT_DEFAULT:
        case AUDIO_FORMAT_PCM_16_BIT:
        case AUDIO_FORMAT_PCM_FLOAT:
            break; // valid
        default:
            ALOGD("audioFormat not valid, audio_format_t = 0x%08x", format);
            return AAUDIO_ERROR_INVALID_FORMAT;
            // break;
    }
    return AAUDIO_OK;
}

aaudio_result_t AAudioStreamParameters::validate() const {
    if (mSamplesPerFrame != AAUDIO_UNSPECIFIED
        && (mSamplesPerFrame < SAMPLES_PER_FRAME_MIN || mSamplesPerFrame > SAMPLES_PER_FRAME_MAX)) {
        ALOGD("channelCount out of range = %d", mSamplesPerFrame);
        return AAUDIO_ERROR_OUT_OF_RANGE;
    }

    if (mDeviceId < 0) {
        ALOGD("deviceId out of range = %d", mDeviceId);
        return AAUDIO_ERROR_OUT_OF_RANGE;
    }

    // All Session ID values are legal.
    switch (mSessionId) {
        case AAUDIO_SESSION_ID_NONE:
        case AAUDIO_SESSION_ID_ALLOCATE:
            break;
        default:
            break;
    }

    switch (mSharingMode) {
        case AAUDIO_SHARING_MODE_EXCLUSIVE:
        case AAUDIO_SHARING_MODE_SHARED:
            break;
        default:
            ALOGD("illegal sharingMode = %d", mSharingMode);
            return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
            // break;
    }

    aaudio_result_t result = isFormatValid (mAudioFormat);
    if (result != AAUDIO_OK) return result;

    if (mSampleRate != AAUDIO_UNSPECIFIED
        && (mSampleRate < SAMPLE_RATE_HZ_MIN || mSampleRate > SAMPLE_RATE_HZ_MAX)) {
        ALOGD("sampleRate out of range = %d", mSampleRate);
        return AAUDIO_ERROR_INVALID_RATE;
    }

    if (mBufferCapacity < 0) {
        ALOGD("bufferCapacity out of range = %d", mBufferCapacity);
        return AAUDIO_ERROR_OUT_OF_RANGE;
    }

    switch (mDirection) {
        case AAUDIO_DIRECTION_INPUT:
        case AAUDIO_DIRECTION_OUTPUT:
            break; // valid
        default:
            ALOGD("direction not valid = %d", mDirection);
            return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
            // break;
    }

    switch (mUsage) {
        case AAUDIO_UNSPECIFIED:
        case AAUDIO_USAGE_MEDIA:
        case AAUDIO_USAGE_VOICE_COMMUNICATION:
        case AAUDIO_USAGE_VOICE_COMMUNICATION_SIGNALLING:
        case AAUDIO_USAGE_ALARM:
        case AAUDIO_USAGE_NOTIFICATION:
        case AAUDIO_USAGE_NOTIFICATION_RINGTONE:
        case AAUDIO_USAGE_NOTIFICATION_EVENT:
        case AAUDIO_USAGE_ASSISTANCE_ACCESSIBILITY:
        case AAUDIO_USAGE_ASSISTANCE_NAVIGATION_GUIDANCE:
        case AAUDIO_USAGE_ASSISTANCE_SONIFICATION:
        case AAUDIO_USAGE_GAME:
        case AAUDIO_USAGE_ASSISTANT:
            break; // valid
        default:
            ALOGD("usage not valid = %d", mUsage);
            return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
            // break;
    }

    switch (mContentType) {
        case AAUDIO_UNSPECIFIED:
        case AAUDIO_CONTENT_TYPE_MUSIC:
        case AAUDIO_CONTENT_TYPE_MOVIE:
        case AAUDIO_CONTENT_TYPE_SONIFICATION:
        case AAUDIO_CONTENT_TYPE_SPEECH:
            break; // valid
        default:
            ALOGD("content type not valid = %d", mContentType);
            return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
            // break;
    }

    switch (mInputPreset) {
        case AAUDIO_UNSPECIFIED:
        case AAUDIO_INPUT_PRESET_GENERIC:
        case AAUDIO_INPUT_PRESET_CAMCORDER:
        case AAUDIO_INPUT_PRESET_VOICE_COMMUNICATION:
        case AAUDIO_INPUT_PRESET_VOICE_RECOGNITION:
        case AAUDIO_INPUT_PRESET_UNPROCESSED:
        case AAUDIO_INPUT_PRESET_VOICE_PERFORMANCE:
            break; // valid
        default:
            ALOGD("input preset not valid = %d", mInputPreset);
            return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
            // break;
    }

    switch (mAllowedCapturePolicy) {
        case AAUDIO_UNSPECIFIED:
        case AAUDIO_ALLOW_CAPTURE_BY_ALL:
        case AAUDIO_ALLOW_CAPTURE_BY_SYSTEM:
        case AAUDIO_ALLOW_CAPTURE_BY_NONE:
            break; // valid
        default:
            ALOGD("allowed capture policy not valid = %d", mAllowedCapturePolicy);
            return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
            // break;
    }

    return AAUDIO_OK;
}

void AAudioStreamParameters::dump() const {
    ALOGD("mDeviceId             = %6d", mDeviceId);
    ALOGD("mSessionId            = %6d", mSessionId);
    ALOGD("mSampleRate           = %6d", mSampleRate);
    ALOGD("mSamplesPerFrame      = %6d", mSamplesPerFrame);
    ALOGD("mSharingMode          = %6d", (int)mSharingMode);
    ALOGD("mAudioFormat          = %6d", (int)mAudioFormat);
    ALOGD("mDirection            = %6d", mDirection);
    ALOGD("mBufferCapacity       = %6d", mBufferCapacity);
    ALOGD("mUsage                = %6d", mUsage);
    ALOGD("mContentType          = %6d", mContentType);
    ALOGD("mInputPreset          = %6d", mInputPreset);
    ALOGD("mAllowedCapturePolicy = %6d", mAllowedCapturePolicy);
}
