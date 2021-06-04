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

#define LOG_TAG "AudioStreamLegacy"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>

#include <aaudio/AAudio.h>
#include <audio_utils/primitives.h>
#include <media/AudioTrack.h>
#include <media/AudioTimestamp.h>
#include <utils/String16.h>

#include "core/AudioStream.h"
#include "legacy/AudioStreamLegacy.h"

using namespace android;
using namespace aaudio;

AudioStreamLegacy::AudioStreamLegacy()
        : AudioStream()
        , mDeviceCallback(new StreamDeviceCallback(this)) {
}

AudioStreamLegacy::~AudioStreamLegacy() {
}

// Called from AudioTrack.cpp or AudioRecord.cpp
static void AudioStreamLegacy_callback(int event, void* userData, void *info) {
    AudioStreamLegacy *streamLegacy = (AudioStreamLegacy *) userData;
    streamLegacy->processCallback(event, info);
}

aaudio_legacy_callback_t AudioStreamLegacy::getLegacyCallback() {
    return AudioStreamLegacy_callback;
}

aaudio_data_callback_result_t AudioStreamLegacy::callDataCallbackFrames(uint8_t *buffer,
                                                                        int32_t numFrames) {
    void *finalAudioData = buffer;
    if (getDirection() == AAUDIO_DIRECTION_INPUT) {
        // Increment before because we already got the data from the device.
        incrementFramesRead(numFrames);
        finalAudioData = (void *) maybeConvertDeviceData(buffer, numFrames);
    }

    // Call using the AAudio callback interface.
    aaudio_data_callback_result_t callbackResult = maybeCallDataCallback(finalAudioData, numFrames);

    if (callbackResult == AAUDIO_CALLBACK_RESULT_CONTINUE
            && getDirection() == AAUDIO_DIRECTION_OUTPUT) {
        // Increment after because we are going to write the data to the device.
        incrementFramesWritten(numFrames);
    }
    return callbackResult;
}

// Implement FixedBlockProcessor
int32_t AudioStreamLegacy::onProcessFixedBlock(uint8_t *buffer, int32_t numBytes) {
    int32_t numFrames = numBytes / getBytesPerDeviceFrame();
    return (int32_t) callDataCallbackFrames(buffer, numFrames);
}

void AudioStreamLegacy::processCallbackCommon(aaudio_callback_operation_t opcode, void *info) {
    aaudio_data_callback_result_t callbackResult;
    // This illegal size can be used to tell AudioRecord or AudioTrack to stop calling us.
    // This takes advantage of them killing the stream when they see a size out of range.
    // That is an undocumented behavior.
    // TODO add to API in AudioRecord and AudioTrack
    const size_t SIZE_STOP_CALLBACKS = SIZE_MAX;

    switch (opcode) {
        case AAUDIO_CALLBACK_OPERATION_PROCESS_DATA: {
            (void) checkForDisconnectRequest(true);

            // Note that this code assumes an AudioTrack::Buffer is the same as
            // AudioRecord::Buffer
            // TODO define our own AudioBuffer and pass it from the subclasses.
            AudioTrack::Buffer *audioBuffer = static_cast<AudioTrack::Buffer *>(info);
            if (getState() == AAUDIO_STREAM_STATE_DISCONNECTED) {
                ALOGW("processCallbackCommon() data, stream disconnected");
                audioBuffer->size = SIZE_STOP_CALLBACKS;
            } else if (!mCallbackEnabled.load()) {
                ALOGW("processCallbackCommon() no data because callback disabled");
                audioBuffer->size = SIZE_STOP_CALLBACKS;
            } else {
                if (audioBuffer->frameCount == 0) {
                    ALOGW("processCallbackCommon() data, frameCount is zero");
                    return;
                }

                // If the caller specified an exact size then use a block size adapter.
                if (mBlockAdapter != nullptr) {
                    int32_t byteCount = audioBuffer->frameCount * getBytesPerDeviceFrame();
                    callbackResult = mBlockAdapter->processVariableBlock(
                            (uint8_t *) audioBuffer->raw, byteCount);
                } else {
                    // Call using the AAudio callback interface.
                    callbackResult = callDataCallbackFrames((uint8_t *)audioBuffer->raw,
                                                            audioBuffer->frameCount);
                }
                if (callbackResult == AAUDIO_CALLBACK_RESULT_CONTINUE) {
                    audioBuffer->size = audioBuffer->frameCount * getBytesPerDeviceFrame();
                } else {
                    if (callbackResult == AAUDIO_CALLBACK_RESULT_STOP) {
                        ALOGD("%s() callback returned AAUDIO_CALLBACK_RESULT_STOP", __func__);
                    } else {
                        ALOGW("%s() callback returned invalid result = %d",
                              __func__, callbackResult);
                    }
                    audioBuffer->size = 0;
                    systemStopFromCallback();
                    // Disable the callback just in case the system keeps trying to call us.
                    mCallbackEnabled.store(false);
                }

                if (updateStateMachine() != AAUDIO_OK) {
                    forceDisconnect();
                    mCallbackEnabled.store(false);
                }
            }
        }
            break;

        // Stream got rerouted so we disconnect.
        case AAUDIO_CALLBACK_OPERATION_DISCONNECTED:
            ALOGD("processCallbackCommon() stream disconnected");
            forceDisconnect();
            mCallbackEnabled.store(false);
            break;

        default:
            break;
    }
}

aaudio_result_t AudioStreamLegacy::checkForDisconnectRequest(bool errorCallbackEnabled) {
    if (mRequestDisconnect.isRequested()) {
        ALOGD("checkForDisconnectRequest() mRequestDisconnect acknowledged");
        forceDisconnect(errorCallbackEnabled);
        mRequestDisconnect.acknowledge();
        mCallbackEnabled.store(false);
        return AAUDIO_ERROR_DISCONNECTED;
    } else {
        return AAUDIO_OK;
    }
}

void AudioStreamLegacy::forceDisconnect(bool errorCallbackEnabled) {
    if (getState() != AAUDIO_STREAM_STATE_DISCONNECTED) {
        setState(AAUDIO_STREAM_STATE_DISCONNECTED);
        if (errorCallbackEnabled) {
            maybeCallErrorCallback(AAUDIO_ERROR_DISCONNECTED);
        }
    }
}

aaudio_result_t AudioStreamLegacy::getBestTimestamp(clockid_t clockId,
                                                   int64_t *framePosition,
                                                   int64_t *timeNanoseconds,
                                                   ExtendedTimestamp *extendedTimestamp) {
    int timebase;
    switch (clockId) {
        case CLOCK_BOOTTIME:
            timebase = ExtendedTimestamp::TIMEBASE_BOOTTIME;
            break;
        case CLOCK_MONOTONIC:
            timebase = ExtendedTimestamp::TIMEBASE_MONOTONIC;
            break;
        default:
            ALOGE("getTimestamp() - Unrecognized clock type %d", (int) clockId);
            return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
            break;
    }
    ExtendedTimestamp::Location location = ExtendedTimestamp::Location::LOCATION_INVALID;
    int64_t localPosition;
    status_t status = extendedTimestamp->getBestTimestamp(&localPosition, timeNanoseconds,
                                                          timebase, &location);
    if (status == OK) {
        // use MonotonicCounter to prevent retrograde motion.
        mTimestampPosition.update32((int32_t) localPosition);
        *framePosition = mTimestampPosition.get();
    }

//    ALOGD("getBestTimestamp() fposition: server = %6lld, kernel = %6lld, location = %d",
//          (long long) extendedTimestamp->mPosition[ExtendedTimestamp::Location::LOCATION_SERVER],
//          (long long) extendedTimestamp->mPosition[ExtendedTimestamp::Location::LOCATION_KERNEL],
//          (int)location);
    return AAudioConvert_androidToAAudioResult(status);
}

void AudioStreamLegacy::onAudioDeviceUpdate(audio_port_handle_t deviceId)
{
    // Device routing is a common source of errors and DISCONNECTS.
    // Please leave this log in place.
    ALOGD("%s() devId %d => %d", __func__, (int) getDeviceId(), (int)deviceId);
    if (getDeviceId() != AAUDIO_UNSPECIFIED && getDeviceId() != deviceId &&
            getState() != AAUDIO_STREAM_STATE_DISCONNECTED) {
        // Note that isDataCallbackActive() is affected by state so call it before DISCONNECTING.
        // If we have a data callback and the stream is active, then ask the data callback
        // to DISCONNECT and call the error callback.
        if (isDataCallbackActive()) {
            ALOGD("onAudioDeviceUpdate() request DISCONNECT in data callback due to device change");
            // If the stream is stopped before the data callback has a chance to handle the
            // request then the requestStop() and requestPause() methods will handle it after
            // the callback has stopped.
            mRequestDisconnect.request();
        } else {
            ALOGD("onAudioDeviceUpdate() DISCONNECT the stream now");
            forceDisconnect();
        }
    }
    setDeviceId(deviceId);
}
