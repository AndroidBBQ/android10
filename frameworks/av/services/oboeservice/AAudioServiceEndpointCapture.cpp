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

#define LOG_TAG "AAudioServiceEndpointCapture"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <assert.h>
#include <map>
#include <mutex>
#include <utils/Singleton.h>

#include "AAudioEndpointManager.h"
#include "AAudioServiceEndpoint.h"

#include "core/AudioStreamBuilder.h"
#include "AAudioServiceEndpoint.h"
#include "AAudioServiceStreamShared.h"
#include "AAudioServiceEndpointCapture.h"
#include "AAudioServiceEndpointShared.h"

using namespace android;  // TODO just import names needed
using namespace aaudio;   // TODO just import names needed

AAudioServiceEndpointCapture::AAudioServiceEndpointCapture(AAudioService &audioService)
        : mStreamInternalCapture(audioService, true) {
    mStreamInternal = &mStreamInternalCapture;
}

AAudioServiceEndpointCapture::~AAudioServiceEndpointCapture() {
    delete mDistributionBuffer;
}

aaudio_result_t AAudioServiceEndpointCapture::open(const aaudio::AAudioStreamRequest &request) {
    aaudio_result_t result = AAudioServiceEndpointShared::open(request);
    if (result == AAUDIO_OK) {
        delete mDistributionBuffer;
        int distributionBufferSizeBytes = getStreamInternal()->getFramesPerBurst()
                                          * getStreamInternal()->getBytesPerFrame();
        mDistributionBuffer = new uint8_t[distributionBufferSizeBytes];
    }
    return result;
}

// Read data from the shared MMAP stream and then distribute it to the client streams.
void *AAudioServiceEndpointCapture::callbackLoop() {
    ALOGD("callbackLoop() entering");
    aaudio_result_t result = AAUDIO_OK;
    int64_t timeoutNanos = getStreamInternal()->calculateReasonableTimeout();

    // result might be a frame count
    while (mCallbackEnabled.load() && getStreamInternal()->isActive() && (result >= 0)) {

        int64_t mmapFramesRead = getStreamInternal()->getFramesRead();

        // Read audio data from stream using a blocking read.
        result = getStreamInternal()->read(mDistributionBuffer, getFramesPerBurst(), timeoutNanos);
        if (result == AAUDIO_ERROR_DISCONNECTED) {
            disconnectRegisteredStreams();
            break;
        } else if (result != getFramesPerBurst()) {
            ALOGW("callbackLoop() read %d / %d",
                  result, getFramesPerBurst());
            break;
        }

        // Distribute data to each active stream.
        { // brackets are for lock_guard

            std::lock_guard <std::mutex> lock(mLockStreams);
            for (const auto& clientStream : mRegisteredStreams) {
                if (clientStream->isRunning() && !clientStream->isSuspended()) {
                    int64_t clientFramesWritten = 0;

                    sp<AAudioServiceStreamShared> streamShared =
                            static_cast<AAudioServiceStreamShared *>(clientStream.get());

                    {
                        // Lock the AudioFifo to protect against close.
                        std::lock_guard <std::mutex> lock(streamShared->getAudioDataQueueLock());

                        FifoBuffer *fifo = streamShared->getAudioDataFifoBuffer_l();
                        if (fifo != nullptr) {

                            // Determine offset between framePosition in client's stream
                            // vs the underlying MMAP stream.
                            clientFramesWritten = fifo->getWriteCounter();
                            // There are two indices that refer to the same frame.
                            int64_t positionOffset = mmapFramesRead - clientFramesWritten;
                            streamShared->setTimestampPositionOffset(positionOffset);

                            // Is the buffer too full to write a burst?
                            if (fifo->getEmptyFramesAvailable() <
                                    getFramesPerBurst()) {
                                streamShared->incrementXRunCount();
                            } else {
                                fifo->write(mDistributionBuffer, getFramesPerBurst());
                            }
                            clientFramesWritten = fifo->getWriteCounter();
                        }
                    }

                    if (clientFramesWritten > 0) {
                        // This timestamp represents the completion of data being written into the
                        // client buffer. It is sent to the client and used in the timing model
                        // to decide when data will be available to read.
                        Timestamp timestamp(clientFramesWritten, AudioClock::getNanoseconds());
                        streamShared->markTransferTime(timestamp);
                    }

                }
            }
        }
    }

    ALOGD("callbackLoop() exiting");
    return NULL; // TODO review
}
