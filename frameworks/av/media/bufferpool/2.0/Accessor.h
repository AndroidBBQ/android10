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

#ifndef ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_ACCESSOR_H
#define ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_ACCESSOR_H

#include <android/hardware/media/bufferpool/2.0/IAccessor.h>
#include <android/hardware/media/bufferpool/2.0/IObserver.h>
#include <bufferpool/BufferPoolTypes.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "BufferStatus.h"

#include <set>

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V2_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Accessor;
struct Connection;

/**
 * Receives death notifications from remote connections.
 * On death notifications, the connections are closed and used resources
 * are released.
 */
struct ConnectionDeathRecipient : public hardware::hidl_death_recipient {
    /**
     * Registers a newly connected connection from remote processes.
     */
    void add(int64_t connectionId, const sp<Accessor> &accessor);

    /**
     * Removes a connection.
     */
    void remove(int64_t connectionId);

    void addCookieToConnection(uint64_t cookie, int64_t connectionId);

    virtual void serviceDied(
            uint64_t /* cookie */,
            const wp<::android::hidl::base::V1_0::IBase>& /* who */
            ) override;

private:
    std::mutex mLock;
    std::map<uint64_t, std::set<int64_t>>  mCookieToConnections;
    std::map<int64_t, uint64_t> mConnectionToCookie;
    std::map<int64_t, const wp<Accessor>> mAccessors;
};

/**
 * A buffer pool accessor which enables a buffer pool to communicate with buffer
 * pool clients. 1:1 correspondense holds between a buffer pool and an accessor.
 */
struct Accessor : public IAccessor {
    // Methods from ::android::hardware::media::bufferpool::V2_0::IAccessor follow.
    Return<void> connect(const sp<::android::hardware::media::bufferpool::V2_0::IObserver>& observer, connect_cb _hidl_cb) override;

    /**
     * Creates a buffer pool accessor which uses the specified allocator.
     *
     * @param allocator buffer allocator.
     */
    explicit Accessor(const std::shared_ptr<BufferPoolAllocator> &allocator);

    /** Destructs a buffer pool accessor. */
    ~Accessor();

    /** Returns whether the accessor is valid. */
    bool isValid();

    /** Invalidates all buffers which are owned by bufferpool */
    ResultStatus flush();

    /** Allocates a buffer from a buffer pool.
     *
     * @param connectionId  the connection id of the client.
     * @param params        the allocation parameters.
     * @param bufferId      the id of the allocated buffer.
     * @param handle        the native handle of the allocated buffer.
     *
     * @return OK when a buffer is successfully allocated.
     *         NO_MEMORY when there is no memory.
     *         CRITICAL_ERROR otherwise.
     */
    ResultStatus allocate(
            ConnectionId connectionId,
            const std::vector<uint8_t>& params,
            BufferId *bufferId,
            const native_handle_t** handle);

    /**
     * Fetches a buffer for the specified transaction.
     *
     * @param connectionId  the id of receiving connection(client).
     * @param transactionId the id of the transfer transaction.
     * @param bufferId      the id of the buffer to be fetched.
     * @param handle        the native handle of the fetched buffer.
     *
     * @return OK when a buffer is successfully fetched.
     *         NO_MEMORY when there is no memory.
     *         CRITICAL_ERROR otherwise.
     */
    ResultStatus fetch(
            ConnectionId connectionId,
            TransactionId transactionId,
            BufferId bufferId,
            const native_handle_t** handle);

    /**
     * Makes a connection to the buffer pool. The buffer pool client uses the
     * created connection in order to communicate with the buffer pool. An
     * FMQ for buffer status message is also created for the client.
     *
     * @param observer      client observer for buffer invalidation
     * @param local         true when a connection request comes from local process,
     *                      false otherwise.
     * @param connection    created connection
     * @param pConnectionId the id of the created connection
     * @param pMsgId        the id of the recent buffer pool message
     * @param statusDescPtr FMQ descriptor for shared buffer status message
     *                      queue between a buffer pool and the client.
     * @param invDescPtr    FMQ descriptor for buffer invalidation message
     *                      queue from a buffer pool to the client.
     *
     * @return OK when a connection is successfully made.
     *         NO_MEMORY when there is no memory.
     *         CRITICAL_ERROR otherwise.
     */
    ResultStatus connect(
            const sp<IObserver>& observer,
            bool local,
            sp<Connection> *connection, ConnectionId *pConnectionId,
            uint32_t *pMsgId,
            const StatusDescriptor** statusDescPtr,
            const InvalidationDescriptor** invDescPtr);

    /**
     * Closes the specified connection to the client.
     *
     * @param connectionId  the id of the connection.
     *
     * @return OK when the connection is closed.
     *         CRITICAL_ERROR otherwise.
     */
    ResultStatus close(ConnectionId connectionId);

    /**
     * Processes pending buffer status messages and perfoms periodic cache
     * cleaning.
     *
     * @param clearCache    if clearCache is true, it frees all buffers waiting
     *                      to be recycled.
     */
    void cleanUp(bool clearCache);

    /**
     * Gets a hidl_death_recipient for remote connection death.
     */
    static sp<ConnectionDeathRecipient> getConnectionDeathRecipient();

    static void createInvalidator();

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_ACCESSOR_H
