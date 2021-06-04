/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef ANDROID_AAUDIO_AUDIO_STREAM_INTERNAL_PLAY_H
#define ANDROID_AAUDIO_AUDIO_STREAM_INTERNAL_PLAY_H

#include <stdint.h>
#include <aaudio/AAudio.h>

#include "binding/AAudioServiceInterface.h"
#include "client/AAudioFlowGraph.h"
#include "client/AudioStreamInternal.h"

using android::sp;
using android::IAAudioService;

namespace aaudio {

class AudioStreamInternalPlay : public AudioStreamInternal {
public:
    AudioStreamInternalPlay(AAudioServiceInterface  &serviceInterface, bool inService = false);
    virtual ~AudioStreamInternalPlay();

    aaudio_result_t open(const AudioStreamBuilder &builder) override;

    aaudio_result_t requestPause() override;

    aaudio_result_t requestFlush() override;

    bool isFlushSupported() const override {
        // Only implement FLUSH for OUTPUT streams.
        return true;
    }

    bool isPauseSupported() const override {
        // Only implement PAUSE for OUTPUT streams.
        return true;
    }

    aaudio_result_t write(const void *buffer,
                          int32_t numFrames,
                          int64_t timeoutNanoseconds) override;

    int64_t getFramesRead() override;
    int64_t getFramesWritten() override;

    void *callbackLoop() override;

    aaudio_direction_t getDirection() const override {
        return AAUDIO_DIRECTION_OUTPUT;
    }

protected:

    void advanceClientToMatchServerPosition() override;

    void onFlushFromServer() override;

    android::status_t doSetVolume() override;

/**
 * Low level write that will not block. It will just write as much as it can.
 *
 * It passed back a recommended time to wake up if wakeTimePtr is not NULL.
 *
 * @return the number of frames written or a negative error code.
 */
    aaudio_result_t processDataNow(void *buffer,
                             int32_t numFrames,
                             int64_t currentTimeNanos,
                             int64_t *wakeTimePtr) override;
private:
    /*
     * Asynchronous write with data conversion.
     * @param buffer
     * @param numFrames
     * @return fdrames written or negative error
     */
    aaudio_result_t writeNowWithConversion(const void *buffer,
                                           int32_t numFrames);

    int64_t                  mLastFramesRead = 0; // used to prevent retrograde motion

    AAudioFlowGraph          mFlowGraph;

};

} /* namespace aaudio */

#endif //ANDROID_AAUDIO_AUDIO_STREAM_INTERNAL_PLAY_H
