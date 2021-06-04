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

#define LOG_TAG "AAudioServiceEndpointPlay"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <assert.h>
#include <map>
#include <mutex>
#include <utils/Singleton.h>

#include "AAudioEndpointManager.h"
#include "AAudioServiceEndpoint.h"
#include <algorithm>
#include <mutex>
#include <vector>

#include "core/AudioStreamBuilder.h"
#include "AAudioServiceEndpoint.h"
#include "AAudioServiceStreamShared.h"
#include "AAudioServiceEndpointPlay.h"
#include "AAudioServiceEndpointShared.h"
#include "AAudioServiceStreamBase.h"

using namespace android;  // TODO just import names needed
using namespace aaudio;   // TODO just import names needed

#define BURSTS_PER_BUFFER_DEFAULT   2

AAudioServiceEndpointPlay::AAudioServiceEndpointPlay(AAudioService &audioService)
        : mStreamInternalPlay(audioService, true) {
    mStreamInternal = &mStreamInternalPlay;
}

aaudio_result_t AAudioServiceEndpointPlay::open(const aaudio::AAudioStreamRequest &request) {
    aaudio_result_t result = AAudioServiceEndpointShared::open(request);
    if (result == AAUDIO_OK) {
        mMixer.allocate(getStreamInternal()->getSamplesPerFrame(),
                        getStreamInternal()->getFramesPerBurst());

        int32_t burstsPerBuffer = AAudioProperty_getMixerBursts();
        if (burstsPerBuffer == 0) {
            mLatencyTuningEnabled = true;
            burstsPerBuffer = BURSTS_PER_BUFFER_DEFAULT;
        }
        int32_t desiredBufferSize = burstsPerBuffer * getStreamInternal()->getFramesPerBurst();
        getStreamInternal()->setBufferSize(desiredBufferSize);
    }
    return result;
}

// Mix data from each application stream and write result to the shared MMAP stream.
void *AAudioServiceEndpointPlay::callbackLoop() {
    ALOGD("%s() entering >>>>>>>>>>>>>>> MIXER", __func__);
    aaudio_result_t result = AAUDIO_OK;
    int64_t timeoutNanos = getStreamInternal()->calculateReasonableTimeout();

    // result might be a frame count
    while (mCallbackEnabled.load() && getStreamInternal()->isActive() && (result >= 0)) {
        // Mix data from each active stream.
        mMixer.clear();

        { // brackets are for lock_guard
            int index = 0;
            int64_t mmapFramesWritten = getStreamInternal()->getFramesWritten();

            std::lock_guard <std::mutex> lock(mLockStreams);
            for (const auto& clientStream : mRegisteredStreams) {
                int64_t clientFramesRead = 0;
                bool allowUnderflow = true;

                if (clientStream->isSuspended()) {
                    continue; // dead stream
                }

                aaudio_stream_state_t state = clientStream->getState();
                if (state == AAUDIO_STREAM_STATE_STOPPING) {
                    allowUnderflow = false; // just read what is already in the FIFO
                } else if (state != AAUDIO_STREAM_STATE_STARTED) {
                    continue; // this stream is not running so skip it.
                }

                sp<AAudioServiceStreamShared> streamShared =
                        static_cast<AAudioServiceStreamShared *>(clientStream.get());

                {
                    // Lock the AudioFifo to protect against close.
                    std::lock_guard <std::mutex> lock(streamShared->getAudioDataQueueLock());

                    FifoBuffer *fifo = streamShared->getAudioDataFifoBuffer_l();
                    if (fifo != nullptr) {

                        // Determine offset between framePosition in client's stream
                        // vs the underlying MMAP stream.
                        clientFramesRead = fifo->getReadCounter();
                        // These two indices refer to the same frame.
                        int64_t positionOffset = mmapFramesWritten - clientFramesRead;
                        streamShared->setTimestampPositionOffset(positionOffset);

                        int32_t framesMixed = mMixer.mix(index, fifo, allowUnderflow);

                        if (streamShared->isFlowing()) {
                            // Consider it an underflow if we got less than a burst
                            // after the data started flowing.
                            bool underflowed = allowUnderflow
                                               && framesMixed < mMixer.getFramesPerBurst();
                            if (underflowed) {
                                streamShared->incrementXRunCount();
                            }
                        } else if (framesMixed > 0) {
                            // Mark beginning of data flow after a start.
                            streamShared->setFlowing(true);
                        }
                        clientFramesRead = fifo->getReadCounter();
                    }
                }

                if (clientFramesRead > 0) {
                    // This timestamp represents the completion of data being read out of the
                    // client buffer. It is sent to the client and used in the timing model
                    // to decide when the client has room to write more data.
                    Timestamp timestamp(clientFramesRead, AudioClock::getNanoseconds());
                    streamShared->markTransferTime(timestamp);
                }

                index++; // just used for labelling tracks in systrace
            }
        }

        // Write mixer output to stream using a blocking write.
        result = getStreamInternal()->write(mMixer.getOutputBuffer(),
                                            getFramesPerBurst(), timeoutNanos);
        if (result == AAUDIO_ERROR_DISCONNECTED) {
            AAudioServiceEndpointShared::disconnectRegisteredStreams();
            break;
        } else if (result != getFramesPerBurst()) {
            ALOGW("callbackLoop() wrote %d / %d",
                  result, getFramesPerBurst());
            break;
        }
    }

    ALOGD("%s() exiting, enabled = %d, state = %d, result = %d <<<<<<<<<<<<< MIXER",
          __func__, mCallbackEnabled.load(), getStreamInternal()->getState(), result);
    return NULL; // TODO review
}
