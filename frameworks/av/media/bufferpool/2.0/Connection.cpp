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

#include "Connection.h"

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V2_0 {
namespace implementation {

// Methods from ::android::hardware::media::bufferpool::V2_0::IConnection follow.
Return<void> Connection::fetch(uint64_t transactionId, uint32_t bufferId, fetch_cb _hidl_cb) {
    ResultStatus status = ResultStatus::CRITICAL_ERROR;
    if (mInitialized && mAccessor) {
        if (bufferId != SYNC_BUFFERID) {
            const native_handle_t *handle = nullptr;
            status = mAccessor->fetch(
                    mConnectionId, transactionId, bufferId, &handle);
            if (status == ResultStatus::OK) {
                Buffer buffer = {};
                buffer.id = bufferId;
                buffer.buffer = handle;
                _hidl_cb(status, buffer);
                return Void();
            }
        } else {
            mAccessor->cleanUp(false);
        }
    }

    Buffer buffer = {};
    buffer.id = 0;
    buffer.buffer = nullptr;

    _hidl_cb(status, buffer);
    return Void();
}

Connection::Connection() : mInitialized(false), mConnectionId(-1LL) {}

Connection::~Connection() {
    if (mInitialized && mAccessor) {
        mAccessor->close(mConnectionId);
    }
}

void Connection::initialize(
        const sp<Accessor>& accessor, ConnectionId connectionId) {
    if (!mInitialized) {
        mAccessor = accessor;
        mConnectionId = connectionId;
        mInitialized = true;
    }
}

ResultStatus Connection::flush() {
    if (mInitialized && mAccessor) {
        return mAccessor->flush();
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus Connection::allocate(
        const std::vector<uint8_t> &params, BufferId *bufferId,
        const native_handle_t **handle) {
    if (mInitialized && mAccessor) {
        return mAccessor->allocate(mConnectionId, params, bufferId, handle);
    }
    return ResultStatus::CRITICAL_ERROR;
}

void Connection::cleanUp(bool clearCache) {
    if (mInitialized && mAccessor) {
        mAccessor->cleanUp(clearCache);
    }
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

//IConnection* HIDL_FETCH_IConnection(const char* /* name */) {
//    return new Connection();
//}

}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android
