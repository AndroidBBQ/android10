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

#ifndef ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_BUFFERPOOLCLIENT_H
#define ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_BUFFERPOOLCLIENT_H

#include <memory>
#include <android/hardware/media/bufferpool/2.0/IAccessor.h>
#include <android/hardware/media/bufferpool/2.0/IConnection.h>
#include <android/hardware/media/bufferpool/2.0/IObserver.h>
#include <bufferpool/BufferPoolTypes.h>
#include <cutils/native_handle.h>
#include "Accessor.h"

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V2_0 {
namespace implementation {

using ::android::hardware::media::bufferpool::V2_0::IAccessor;
using ::android::hardware::media::bufferpool::V2_0::IConnection;
using ::android::hardware::media::bufferpool::V2_0::IObserver;
using ::android::hardware::media::bufferpool::V2_0::ResultStatus;
using ::android::sp;

/**
 * A buffer pool client for a buffer pool. For a specific buffer pool, at most
 * one buffer pool client exists per process. This class will not be exposed
 * outside. A buffer pool client will be used via ClientManager.
 */
class BufferPoolClient {
public:
    /**
     * Creates a buffer pool client from a local buffer pool
     * (via ClientManager#create).
     */
    explicit BufferPoolClient(const sp<Accessor> &accessor,
                              const sp<IObserver> &observer);

    /**
     * Creates a buffer pool client from a remote buffer pool
     * (via ClientManager#registerSender).
     * Note: A buffer pool client created with remote buffer pool cannot
     * allocate a buffer.
     */
    explicit BufferPoolClient(const sp<IAccessor> &accessor,
                              const sp<IObserver> &observer);

    /** Destructs a buffer pool client. */
    ~BufferPoolClient();

private:
    bool isValid();

    bool isLocal();

    bool isActive(int64_t *lastTransactionUs, bool clearCache);

    ConnectionId getConnectionId();

    ResultStatus getAccessor(sp<IAccessor> *accessor);

    void receiveInvalidation(uint32_t msgId);

    ResultStatus flush();

    ResultStatus allocate(const std::vector<uint8_t> &params,
                          native_handle_t **handle,
                          std::shared_ptr<BufferPoolData> *buffer);

    ResultStatus receive(TransactionId transactionId,
                         BufferId bufferId,
                         int64_t timestampUs,
                         native_handle_t **handle,
                         std::shared_ptr<BufferPoolData> *buffer);

    ResultStatus postSend(ConnectionId receiver,
                          const std::shared_ptr<BufferPoolData> &buffer,
                          TransactionId *transactionId,
                          int64_t *timestampUs);

    class Impl;
    std::shared_ptr<Impl> mImpl;

    friend struct ClientManager;
    friend struct Observer;
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_BUFFERPOOLCLIENT_H
