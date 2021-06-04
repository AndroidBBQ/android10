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

#ifndef ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_BUFFERSTATUS_H
#define ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_BUFFERSTATUS_H

#include <android/hardware/media/bufferpool/2.0/types.h>
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
namespace V2_0 {
namespace implementation {

/** Returns monotonic timestamp in Us since fixed point in time. */
int64_t getTimestampNow();

bool isMessageLater(uint32_t curMsgId, uint32_t prevMsgId);

bool isBufferInRange(BufferId from, BufferId to, BufferId bufferId);

/**
 * A collection of buffer status message FMQ for a buffer pool. buffer
 * ownership/status change messages are sent via the FMQs from the clients.
 */
class BufferStatusObserver {
private:
    std::map<ConnectionId, std::unique_ptr<BufferStatusQueue>>
            mBufferStatusQueues;

public:
    /** Creates a buffer status message FMQ for the specified
     * connection(client).
     *
     * @param connectionId  connection Id of the specified client.
     * @param fmqDescPtr    double ptr of created FMQ's descriptor.
     *
     * @return OK if FMQ is created successfully.
     *         NO_MEMORY when there is no memory.
     *         CRITICAL_ERROR otherwise.
     */
    ResultStatus open(ConnectionId id, const StatusDescriptor** fmqDescPtr);

    /** Closes a buffer status message FMQ for the specified
     * connection(client).
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
 * A buffer status message FMQ for a buffer pool client. Buffer ownership/status
 * change messages are sent via the fmq to the buffer pool.
 */
class BufferStatusChannel {
private:
    bool mValid;
    std::unique_ptr<BufferStatusQueue> mBufferStatusQueue;

public:
    /**
     * Connects to a buffer status message FMQ from a descriptor of
     * the created FMQ.
     *
     * @param fmqDesc   Descriptor of the created FMQ.
     */
    BufferStatusChannel(const StatusDescriptor &fmqDesc);

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

    /**
     * Posts a buffer invaliadation messge to the buffer pool.
     *
     * @param connectionId  connection Id of the client.
     * @param invalidateId  invalidation ack to the buffer pool.
     *                      if invalidation id is zero, the ack will not be
     *                      posted.
     * @param invalidated   sets {@code true} only when the invalidation ack is
     *                      posted.
     */
    void postBufferInvalidateAck(
            ConnectionId connectionId,
            uint32_t invalidateId,
            bool *invalidated);
};

/**
 * A buffer invalidation FMQ for a buffer pool client. Buffer invalidation
 * messages are received via the fmq from the buffer pool. Buffer invalidation
 * messages are handled as soon as possible.
 */
class BufferInvalidationListener {
private:
    bool mValid;
    std::unique_ptr<BufferInvalidationQueue> mBufferInvalidationQueue;

public:
    /**
     * Connects to a buffer invalidation FMQ from a descriptor of the created FMQ.
     *
     * @param fmqDesc   Descriptor of the created FMQ.
     */
    BufferInvalidationListener(const InvalidationDescriptor &fmqDesc);

    /** Retrieves all pending buffer invalidation messages from the buffer pool.
     *
     * @param messages  retrieved pending messages.
     */
    void getInvalidations(std::vector<BufferInvalidationMessage> &messages);

    /** Returns whether the FMQ is connected succesfully. */
    bool isValid();
};

/**
 * A buffer invalidation FMQ for a buffer pool. A buffer pool will send buffer
 * invalidation messages to the clients via the FMQ. The FMQ is shared among
 * buffer pool clients.
 */
class BufferInvalidationChannel {
private:
    bool mValid;
    std::unique_ptr<BufferInvalidationQueue> mBufferInvalidationQueue;

public:
    /**
     * Creates a buffer invalidation FMQ for a buffer pool.
     */
    BufferInvalidationChannel();

    /** Returns whether the FMQ is connected succesfully. */
    bool isValid();

    /**
     * Retrieves the descriptor of a buffer invalidation FMQ. the descriptor may
     * be passed to the client for buffer invalidation handling.
     *
     * @param fmqDescPtr    double ptr of created FMQ's descriptor.
     */
    void getDesc(const InvalidationDescriptor **fmqDescPtr);

    /** Posts a buffer invalidation for invalidated buffers.
     *
     * @param msgId     Invalidation message id which is used when clients send
     *                  acks back via BufferStatusMessage
     * @param fromId    The start bufferid of the invalidated buffers(inclusive)
     * @param toId      The end bufferId of the invalidated buffers(inclusive)
     */
    void postInvalidation(uint32_t msgId, BufferId fromId, BufferId toId);
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_BUFFERSTATUS_H
