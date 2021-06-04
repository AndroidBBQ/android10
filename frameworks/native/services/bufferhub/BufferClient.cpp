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

#include <bufferhub/BufferClient.h>
#include <bufferhub/BufferHubService.h>
#include <hidl/HidlSupport.h>
#include <log/log.h>

namespace android {
namespace frameworks {
namespace bufferhub {
namespace V1_0 {
namespace implementation {

using hardware::hidl_handle;
using hardware::Void;

BufferClient* BufferClient::create(BufferHubService* service,
                                   const std::shared_ptr<BufferNode>& node) {
    if (!service) {
        ALOGE("%s: service cannot be nullptr.", __FUNCTION__);
        return nullptr;
    } else if (!node) {
        ALOGE("%s: node cannot be nullptr.", __FUNCTION__);
        return nullptr;
    }
    return new BufferClient(service, node);
}

BufferClient::~BufferClient() {
    {
        std::lock_guard<std::mutex> lock(mClosedMutex);
        if (!mClosed) {
            ALOGW("%s: client of buffer #%d destroyed without close. Closing it now.", __FUNCTION__,
                  mBufferNode->id());
        }
    }

    close();
}

Return<BufferHubStatus> BufferClient::close() {
    std::lock_guard<std::mutex> lock(mClosedMutex);
    if (mClosed) {
        return BufferHubStatus::CLIENT_CLOSED;
    }

    getService()->onClientClosed(this);
    mBufferNode.reset();
    mClosed = true;
    return BufferHubStatus::NO_ERROR;
}

Return<void> BufferClient::duplicate(duplicate_cb _hidl_cb) {
    std::lock_guard<std::mutex> lock(mClosedMutex);
    if (mClosed) {
        _hidl_cb(/*token=*/hidl_handle(), /*status=*/BufferHubStatus::CLIENT_CLOSED);
        return Void();
    }

    if (!mBufferNode) {
        // Should never happen
        ALOGE("%s: node is missing.", __FUNCTION__);
        _hidl_cb(/*token=*/hidl_handle(), /*status=*/BufferHubStatus::BUFFER_FREED);
        return Void();
    }

    const hidl_handle token = getService()->registerToken(this);
    _hidl_cb(/*token=*/token, /*status=*/BufferHubStatus::NO_ERROR);
    return Void();
}

sp<BufferHubService> BufferClient::getService() {
    sp<BufferHubService> service = mService.promote();
    if (service == nullptr) {
        // Should never happen. Kill the process.
        LOG_FATAL("%s: service died.", __FUNCTION__);
    }

    return service;
}

} // namespace implementation
} // namespace V1_0
} // namespace bufferhub
} // namespace frameworks
} // namespace android