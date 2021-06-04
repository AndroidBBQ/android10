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

#ifndef ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_CONNECTION_H
#define ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_CONNECTION_H

#include <android/hardware/media/bufferpool/2.0/IConnection.h>
#include <bufferpool/BufferPoolTypes.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "Accessor.h"

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
using ::android::hardware::media::bufferpool::V2_0::implementation::Accessor;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Connection : public IConnection {
    // Methods from ::android::hardware::media::bufferpool::V2_0::IConnection follow.
    Return<void> fetch(uint64_t transactionId, uint32_t bufferId, fetch_cb _hidl_cb) override;

    /**
     * Invalidates all buffers which are active and/or are ready to be recycled.
     */
    ResultStatus flush();

    /**
     * Allocates a buffer using the specified parameters. Recycles a buffer if
     * it is possible. The returned buffer can be transferred to other remote
     * clients(Connection).
     *
     * @param params    allocation parameters.
     * @param bufferId  Id of the allocated buffer.
     * @param handle    native handle of the allocated buffer.
     *
     * @return OK if a buffer is successfully allocated.
     *         NO_MEMORY when there is no memory.
     *         CRITICAL_ERROR otherwise.
     */
    ResultStatus allocate(const std::vector<uint8_t> &params,
                          BufferId *bufferId, const native_handle_t **handle);

    /**
     * Processes pending buffer status messages and performs periodic cache cleaning
     * from bufferpool.
     *
     * @param clearCache    if clearCache is true, bufferpool frees all buffers
     *                      waiting to be recycled.
     */
    void cleanUp(bool clearCache);

    /** Destructs a connection. */
    ~Connection();

    /** Creates a connection. */
    Connection();

    /**
     * Initializes with the specified buffer pool and the connection id.
     * The connection id should be unique in the whole system.
     *
     * @param accessor      the specified buffer pool.
     * @param connectionId  Id.
     */
    void initialize(const sp<Accessor> &accessor, ConnectionId connectionId);

    enum : uint32_t {
        SYNC_BUFFERID = UINT32_MAX,
    };

private:
    bool mInitialized;
    sp<Accessor> mAccessor;
    ConnectionId mConnectionId;
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_CONNECTION_H
