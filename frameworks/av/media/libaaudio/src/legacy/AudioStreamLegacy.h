/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef LEGACY_AUDIO_STREAM_LEGACY_H
#define LEGACY_AUDIO_STREAM_LEGACY_H

#include <media/AudioTimestamp.h>
#include <media/AudioSystem.h>

#include <aaudio/AAudio.h>

#include "AudioStream.h"
#include "AAudioLegacy.h"
#include "utility/AAudioUtilities.h"
#include "utility/FixedBlockAdapter.h"

namespace aaudio {


typedef void (*aaudio_legacy_callback_t)(int event, void* user, void *info);

enum {
    /**
     * Request that the callback function should fill the data buffer of an output stream,
     * or process the data of an input stream.
     * The address parameter passed to the callback function will point to a data buffer.
     * For an input stream, the data is read-only.
     * The value1 parameter will be the number of frames.
     * The value2 parameter is reserved and will be set to zero.
     * The callback should return AAUDIO_CALLBACK_RESULT_CONTINUE or AAUDIO_CALLBACK_RESULT_STOP.
     */
            AAUDIO_CALLBACK_OPERATION_PROCESS_DATA,

    /**
     * Inform the callback function that the stream was disconnected.
     * The address parameter passed to the callback function will be NULL.
     * The value1 will be an error code or AAUDIO_OK.
     * The value2 parameter is reserved and will be set to zero.
     * The callback return value will be ignored.
     */
            AAUDIO_CALLBACK_OPERATION_DISCONNECTED,
};
typedef int32_t aaudio_callback_operation_t;


class AudioStreamLegacy : public AudioStream, public FixedBlockProcessor {
public:
    AudioStreamLegacy();

    virtual ~AudioStreamLegacy();

    aaudio_legacy_callback_t getLegacyCallback();

    int32_t callDataCallbackFrames(uint8_t *buffer, int32_t numFrames);

    // This is public so it can be called from the C callback function.
    // This is called from the AudioTrack/AudioRecord client.
    virtual void processCallback(int event, void *info) = 0;

    void processCallbackCommon(aaudio_callback_operation_t opcode, void *info);

    // Implement FixedBlockProcessor
    int32_t onProcessFixedBlock(uint8_t *buffer, int32_t numBytes) override;

    virtual int64_t incrementClientFrameCounter(int32_t frames)  = 0;

    virtual int64_t getFramesWritten() override {
        return mFramesWritten.get();
    }

    virtual int64_t getFramesRead() override {
        return mFramesRead.get();
    }

protected:

    class StreamDeviceCallback : public android::AudioSystem::AudioDeviceCallback
    {
    public:

        StreamDeviceCallback(AudioStreamLegacy *parent) : mParent(parent) {}
        virtual ~StreamDeviceCallback() {}

        virtual void onAudioDeviceUpdate(audio_io_handle_t audioIo __unused,
                                         audio_port_handle_t deviceId) {
            if (mParent != nullptr) {
                mParent->onAudioDeviceUpdate(deviceId);
            }
        }

        AudioStreamLegacy *mParent;
    };

    aaudio_result_t getBestTimestamp(clockid_t clockId,
                                     int64_t *framePosition,
                                     int64_t *timeNanoseconds,
                                     android::ExtendedTimestamp *extendedTimestamp);

    void onAudioDeviceUpdate(audio_port_handle_t deviceId);

    /*
     * Check to see whether a callback thread has requested a disconnected.
     * @param errorCallbackEnabled set true to call errorCallback on disconnect
     * @return AAUDIO_OK or AAUDIO_ERROR_DISCONNECTED
     */
    aaudio_result_t checkForDisconnectRequest(bool errorCallbackEnabled);

    void forceDisconnect(bool errorCallbackEnabled = true);

    int64_t incrementFramesWritten(int32_t frames) {
        return mFramesWritten.increment(frames);
    }

    int64_t incrementFramesRead(int32_t frames) {
        return mFramesRead.increment(frames);
    }

    MonotonicCounter           mFramesWritten;
    MonotonicCounter           mFramesRead;
    MonotonicCounter           mTimestampPosition;

    FixedBlockAdapter         *mBlockAdapter = nullptr;
    aaudio_wrapping_frames_t   mPositionWhenStarting = 0;
    int32_t                    mCallbackBufferSize = 0;
    const android::sp<StreamDeviceCallback>   mDeviceCallback;

    AtomicRequestor            mRequestDisconnect;
};

} /* namespace aaudio */

#endif //LEGACY_AUDIO_STREAM_LEGACY_H
