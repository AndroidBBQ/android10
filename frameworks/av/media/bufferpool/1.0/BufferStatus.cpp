/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "BufferPoolStatus"
//#define LOG_NDEBUG 0

#include <time.h>
#include "BufferStatus.h"

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V1_0 {
namespace implementation {

int64_t getTimestampNow() {
    int64_t stamp;
    struct timespec ts;
    // TODO: CLOCK_MONOTONIC_COARSE?
    clock_gettime(CLOCK_MONOTONIC, &ts);
    stamp = ts.tv_nsec / 1000;
    stamp += (ts.tv_sec * 1000000LL);
    return stamp;
}

static constexpr int kNumElementsInQueue = 1024*16;
static constexpr int kMinElementsToSyncInQueue = 128;

ResultStatus BufferStatusObserver::open(
        ConnectionId id, const QueueDescriptor** fmqDescPtr) {
    if (mBufferStatusQueues.find(id) != mBufferStatusQueues.end()) {
        // TODO: id collision log?
        return ResultStatus::CRITICAL_ERROR;
    }
    std::unique_ptr<BufferStatusQueue> queue =
            std::make_unique<BufferStatusQueue>(kNumElementsInQueue);
    if (!queue || queue->isValid() == false) {
        *fmqDescPtr = nullptr;
        return ResultStatus::NO_MEMORY;
    } else {
        *fmqDescPtr = queue->getDesc();
    }
    auto result = mBufferStatusQueues.insert(
            std::make_pair(id, std::move(queue)));
    if (!result.second) {
        *fmqDescPtr = nullptr;
        return ResultStatus::NO_MEMORY;
    }
    return ResultStatus::OK;
}

ResultStatus BufferStatusObserver::close(ConnectionId id) {
    if (mBufferStatusQueues.find(id) == mBufferStatusQueues.end()) {
        return ResultStatus::CRITICAL_ERROR;
    }
    mBufferStatusQueues.erase(id);
    return ResultStatus::OK;
}

void BufferStatusObserver::getBufferStatusChanges(std::vector<BufferStatusMessage> &messages) {
    for (auto it = mBufferStatusQueues.begin(); it != mBufferStatusQueues.end(); ++it) {
        BufferStatusMessage message;
        size_t avail = it->second->availableToRead();
        while (avail > 0) {
            if (!it->second->read(&message, 1)) {
                // Since avaliable # of reads are already confirmed,
                // this should not happen.
                // TODO: error handling (spurious client?)
                ALOGW("FMQ message cannot be read from %lld", (long long)it->first);
                return;
            }
            message.connectionId = it->first;
            messages.push_back(message);
            --avail;
        }
    }
}

BufferStatusChannel::BufferStatusChannel(
        const QueueDescriptor &fmqDesc) {
    std::unique_ptr<BufferStatusQueue> queue =
            std::make_unique<BufferStatusQueue>(fmqDesc);
    if (!queue || queue->isValid() == false) {
        mValid = false;
        return;
    }
    mValid  = true;
    mBufferStatusQueue = std::move(queue);
}

bool BufferStatusChannel::isValid() {
    return mValid;
}

bool BufferStatusChannel::needsSync() {
    if (mValid) {
        size_t avail = mBufferStatusQueue->availableToWrite();
        return avail + kMinElementsToSyncInQueue < kNumElementsInQueue;
    }
    return false;
}

void BufferStatusChannel::postBufferRelease(
        ConnectionId connectionId,
        std::list<BufferId> &pending, std::list<BufferId> &posted) {
    if (mValid && pending.size() > 0) {
        size_t avail = mBufferStatusQueue->availableToWrite();
        avail = std::min(avail, pending.size());
        BufferStatusMessage message;
        for (size_t i = 0 ; i < avail; ++i) {
            BufferId id = pending.front();
            message.newStatus = BufferStatus::NOT_USED;
            message.bufferId = id;
            message.connectionId = connectionId;
            if (!mBufferStatusQueue->write(&message, 1)) {
                // Since avaliable # of writes are already confirmed,
                // this should not happen.
                // TODO: error handing?
                ALOGW("FMQ message cannot be sent from %lld", (long long)connectionId);
                return;
            }
            pending.pop_front();
            posted.push_back(id);
        }
    }
}

bool BufferStatusChannel::postBufferStatusMessage(
        TransactionId transactionId, BufferId bufferId,
        BufferStatus status, ConnectionId connectionId, ConnectionId targetId,
        std::list<BufferId> &pending, std::list<BufferId> &posted) {
    if (mValid) {
        size_t avail = mBufferStatusQueue->availableToWrite();
        size_t numPending = pending.size();
        if (avail >= numPending + 1) {
            BufferStatusMessage release, message;
            for (size_t i = 0; i < numPending; ++i) {
                BufferId id = pending.front();
                release.newStatus = BufferStatus::NOT_USED;
                release.bufferId = id;
                release.connectionId = connectionId;
                if (!mBufferStatusQueue->write(&release, 1)) {
                    // Since avaliable # of writes are already confirmed,
                    // this should not happen.
                    // TODO: error handling?
                    ALOGW("FMQ message cannot be sent from %lld", (long long)connectionId);
                    return false;
                }
                pending.pop_front();
                posted.push_back(id);
            }
            message.transactionId = transactionId;
            message.bufferId = bufferId;
            message.newStatus = status;
            message.connectionId = connectionId;
            message.targetConnectionId = targetId;
            // TODO : timesatamp
            message.timestampUs = 0;
            if (!mBufferStatusQueue->write(&message, 1)) {
                // Since avaliable # of writes are already confirmed,
                // this should not happen.
                ALOGW("FMQ message cannot be sent from %lld", (long long)connectionId);
                return false;
            }
            return true;
        }
    }
    return false;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android

