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

#ifndef ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V1_0_BUFFERSTATUS_H
#define ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V1_0_BUFFERSTATUS_H

#include <android/hardware/media/bufferpool/1.0/types.h>
#include <bufferpool/BufferPoolTypes.h>
#include <fmq/MessageQueue.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <memory>
#include <mutex>
#include <vector>
#include <list>

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V1_0 {
namespace implementation {

/** Returns monotonic timestamp in Us since fixed point in time. */
int64_t getTimestampNow();

/**
 * A collection of FMQ for a buffer pool. buffer ownership/status change
 * messages are sent via the FMQs from the clients.
 */
class BufferStatusObserver {
private:
    std::map<ConnectionId, std::unique_ptr<BufferStatusQueue>>
            mBufferStatusQueues;

public:
    /** Creates an FMQ for the specified connection(client).
     *
     * @param connectionId  connection Id of the specified client.
     * @param fmqDescPtr    double ptr of created FMQ's descriptor.
     *
     * @return OK if FMQ is created successfully.
     *         NO_MEMORY when there is no memory.
     *         CRITICAL_ERROR otherwise.
     */
    ResultStatus open(ConnectionId id, const QueueDescriptor** fmqDescPtr);

    /** Closes an FMQ for the specified connection(client).
     *
     * @param connectionId  connection Id of the specified client.
     *
     * @return OK if the specified connection is closed successfully.
     *         CRITICAL_ERROR otherwise.
     */
    ResultStatus close(ConnectionId id);

    /** Retrieves all pending FMQ buffer status messages from clients.
     *
     * @param messages  retrieved pending messages.
     */
    void getBufferStatusChanges(std::vector<BufferStatusMessage> &messages);
};

/**
 * An FMQ for a buffer pool client. Buffer ownership/status change messages
 * are sent via the fmq to the buffer pool.
 */
class BufferStatusChannel {
private:
    bool mValid;
    std::unique_ptr<BufferStatusQueue> mBufferStatusQueue;

public:
    /**
     * Connects to an FMQ from a descriptor of the created FMQ.
     *
     * @param fmqDesc   Descriptor of the created FMQ.
     */
    BufferStatusChannel(const QueueDescriptor &fmqDesc);

    /** Returns whether the FMQ is connected successfully. */
    bool isValid();

    /** Returns whether the FMQ needs to be synced from the buffer pool */
    bool needsSync();

    /**
     * Posts a buffer release message to the buffer pool.
     *
     * @param connectionId  connection Id of the client.
     * @param pending       currently pending buffer release messages.
     * @param posted        posted buffer release messages.
     */
    void postBufferRelease(
            ConnectionId connectionId,
            std::list<BufferId> &pending, std::list<BufferId> &posted);

    /**
     * Posts a buffer status message regarding the specified buffer
     * transfer transaction.
     *
     * @param transactionId Id of the specified transaction.
     * @param bufferId      buffer Id of the specified transaction.
     * @param status        new status of the buffer.
     * @param connectionId  connection Id of the client.
     * @param targetId      connection Id of the receiver(only when the sender
     *                      posts a status message).
     * @param pending       currently pending buffer release messages.
     * @param posted        posted buffer release messages.
     *
     * @return {@code true} when the specified message is posted,
     *         {@code false} otherwise.
     */
    bool postBufferStatusMessage(
            TransactionId transactionId,
            BufferId bufferId,
            BufferStatus status,
            ConnectionId connectionId,
            ConnectionId targetId,
            std::list<BufferId> &pending, std::list<BufferId> &posted);
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V1_0_BUFFERSTATUS_H
