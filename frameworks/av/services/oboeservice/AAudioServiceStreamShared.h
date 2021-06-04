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

#ifndef AAUDIO_AAUDIO_SERVICE_STREAM_SHARED_H
#define AAUDIO_AAUDIO_SERVICE_STREAM_SHARED_H

#include "fifo/FifoBuffer.h"
#include "binding/AAudioServiceMessage.h"
#include "binding/AAudioStreamRequest.h"
#include "binding/AAudioStreamConfiguration.h"

#include "AAudioService.h"
#include "AAudioServiceStreamBase.h"

namespace aaudio {

// We expect the queue to only have a few commands.
// This should be way more than we need.
#define QUEUE_UP_CAPACITY_COMMANDS (128)

class AAudioEndpointManager;
class AAudioServiceEndpoint;
class SharedRingBuffer;

/**
 * One of these is created for every MODE_SHARED stream in the AAudioService.
 *
 * Each Shared stream will register itself with an AAudioServiceEndpoint when it is opened.
 */
class AAudioServiceStreamShared : public AAudioServiceStreamBase {

public:
    explicit AAudioServiceStreamShared(android::AAudioService &aAudioService);
    virtual ~AAudioServiceStreamShared() = default;

    static std::string dumpHeader();

    std::string dump() const override;

    aaudio_result_t open(const aaudio::AAudioStreamRequest &request) override;

    aaudio_result_t close() override;

    /**
     * This must be locked when calling getAudioDataFifoBuffer_l() and while
     * using the FifoBuffer it returns.
     */
    std::mutex &getAudioDataQueueLock() {
        return mAudioDataQueueLock;
    }

    /**
     * This must only be call under getAudioDataQueueLock().
     * @return
     */
    android::FifoBuffer *getAudioDataFifoBuffer_l() { return (mAudioDataQueue == nullptr)
                                                      ? nullptr
                                                      : mAudioDataQueue->getFifoBuffer(); }

    /* Keep a record of when a buffer transfer completed.
     * This allows for a more accurate timing model.
     */
    void markTransferTime(Timestamp &timestamp);

    void setTimestampPositionOffset(int64_t deltaFrames) {
        mTimestampPositionOffset.store(deltaFrames);
    }

    void incrementXRunCount() {
        sendXRunCount(++mXRunCount);
    }

    int32_t getXRunCount() const {
        return mXRunCount.load();
    }

    const char *getTypeText() const override { return "Shared"; }

protected:

    aaudio_result_t getAudioDataDescription(AudioEndpointParcelable &parcelable) override;

    aaudio_result_t getFreeRunningPosition(int64_t *positionFrames, int64_t *timeNanos) override;

    aaudio_result_t getHardwareTimestamp(int64_t *positionFrames, int64_t *timeNanos) override;

    /**
     * @param requestedCapacityFrames
     * @param framesPerBurst
     * @return capacity or negative error
     */
    static int32_t calculateBufferCapacity(int32_t requestedCapacityFrames,
                                            int32_t framesPerBurst);

private:
    SharedRingBuffer        *mAudioDataQueue = nullptr; // protected by mAudioDataQueueLock
    std::mutex               mAudioDataQueueLock;

    std::atomic<int64_t>     mTimestampPositionOffset;
    std::atomic<int32_t>     mXRunCount;

};

} /* namespace aaudio */

#endif //AAUDIO_AAUDIO_SERVICE_STREAM_SHARED_H
