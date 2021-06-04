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
#define LOG_TAG "BufferPoolConnection"

#include "Accessor.h"
#include "AccessorImpl.h"
#include "Connection.h"

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V2_0 {
namespace implementation {

void ConnectionDeathRecipient::add(
        int64_t connectionId,
        const sp<Accessor> &accessor) {
    std::lock_guard<std::mutex> lock(mLock);
    if (mAccessors.find(connectionId) == mAccessors.end()) {
        mAccessors.insert(std::make_pair(connectionId, accessor));
    }
}

void ConnectionDeathRecipient::remove(int64_t connectionId) {
    std::lock_guard<std::mutex> lock(mLock);
    mAccessors.erase(connectionId);
    auto it = mConnectionToCookie.find(connectionId);
    if (it != mConnectionToCookie.end()) {
        uint64_t cookie = it->second;
        mConnectionToCookie.erase(it);
        auto cit = mCookieToConnections.find(cookie);
        if (cit != mCookieToConnections.end()) {
            cit->second.erase(connectionId);
            if (cit->second.size() == 0) {
                mCookieToConnections.erase(cit);
            }
        }
    }
}

void ConnectionDeathRecipient::addCookieToConnection(
        uint64_t cookie,
        int64_t connectionId) {
    std::lock_guard<std::mutex> lock(mLock);
    if (mAccessors.find(connectionId) == mAccessors.end()) {
        return;
    }
    mConnectionToCookie.insert(std::make_pair(connectionId, cookie));
    auto it = mCookieToConnections.find(cookie);
    if (it != mCookieToConnections.end()) {
        it->second.insert(connectionId);
    } else {
        mCookieToConnections.insert(std::make_pair(
                cookie, std::set<int64_t>{connectionId}));
    }
}

void ConnectionDeathRecipient::serviceDied(
        uint64_t cookie,
        const wp<::android::hidl::base::V1_0::IBase>& /* who */
        ) {
    std::map<int64_t, const wp<Accessor>> connectionsToClose;
    {
        std::lock_guard<std::mutex> lock(mLock);

        auto it = mCookieToConnections.find(cookie);
        if (it != mCookieToConnections.end()) {
            for (auto conIt = it->second.begin(); conIt != it->second.end(); ++conIt) {
                auto accessorIt = mAccessors.find(*conIt);
                if (accessorIt != mAccessors.end()) {
                    connectionsToClose.insert(std::make_pair(*conIt, accessorIt->second));
                    mAccessors.erase(accessorIt);
                }
                mConnectionToCookie.erase(*conIt);
            }
            mCookieToConnections.erase(it);
        }
    }

    if (connectionsToClose.size() > 0) {
        sp<Accessor> accessor;
        for (auto it = connectionsToClose.begin(); it != connectionsToClose.end(); ++it) {
            accessor = it->second.promote();

            if (accessor) {
                accessor->close(it->first);
                ALOGD("connection %lld closed on death", (long long)it->first);
            }
        }
    }
}

namespace {
static sp<ConnectionDeathRecipient> sConnectionDeathRecipient =
        new ConnectionDeathRecipient();
}

sp<ConnectionDeathRecipient> Accessor::getConnectionDeathRecipient() {
    return sConnectionDeathRecipient;
}

void Accessor::createInvalidator() {
    Accessor::Impl::createInvalidator();
}

// Methods from ::android::hardware::media::bufferpool::V2_0::IAccessor follow.
Return<void> Accessor::connect(
        const sp<::android::hardware::media::bufferpool::V2_0::IObserver>& observer,
        connect_cb _hidl_cb) {
    sp<Connection> connection;
    ConnectionId connectionId;
    uint32_t msgId;
    const StatusDescriptor* fmqDesc;
    const InvalidationDescriptor* invDesc;

    ResultStatus status = connect(
            observer, false, &connection, &connectionId, &msgId, &fmqDesc, &invDesc);
    if (status == ResultStatus::OK) {
        _hidl_cb(status, connection, connectionId, msgId, *fmqDesc, *invDesc);
    } else {
        _hidl_cb(status, nullptr, -1LL, 0,
                 android::hardware::MQDescriptorSync<BufferStatusMessage>(
                         std::vector<android::hardware::GrantorDescriptor>(),
                         nullptr /* nhandle */, 0 /* size */),
                 android::hardware::MQDescriptorUnsync<BufferInvalidationMessage>(
                         std::vector<android::hardware::GrantorDescriptor>(),
                         nullptr /* nhandle */, 0 /* size */));
    }
    return Void();
}

Accessor::Accessor(const std::shared_ptr<BufferPoolAllocator> &allocator)
    : mImpl(new Impl(allocator)) {}

Accessor::~Accessor() {
}

bool Accessor::isValid() {
    return (bool)mImpl && mImpl->isValid();
}

ResultStatus Accessor::flush() {
    if (mImpl) {
        mImpl->flush();
        return ResultStatus::OK;
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus Accessor::allocate(
        ConnectionId connectionId,
        const std::vector<uint8_t> &params,
        BufferId *bufferId, const native_handle_t** handle) {
    if (mImpl) {
        return mImpl->allocate(connectionId, params, bufferId, handle);
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus Accessor::fetch(
        ConnectionId connectionId, TransactionId transactionId,
        BufferId bufferId, const native_handle_t** handle) {
    if (mImpl) {
        return mImpl->fetch(connectionId, transactionId, bufferId, handle);
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus Accessor::connect(
        const sp<IObserver> &observer, bool local,
        sp<Connection> *connection, ConnectionId *pConnectionId,
        uint32_t *pMsgId,
        const StatusDescriptor** statusDescPtr,
        const InvalidationDescriptor** invDescPtr) {
    if (mImpl) {
        ResultStatus status = mImpl->connect(
                this, observer, connection, pConnectionId, pMsgId,
                statusDescPtr, invDescPtr);
        if (!local && status == ResultStatus::OK) {
            sp<Accessor> accessor(this);
            sConnectionDeathRecipient->add(*pConnectionId, accessor);
        }
        return status;
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus Accessor::close(ConnectionId connectionId) {
    if (mImpl) {
        ResultStatus status = mImpl->close(connectionId);
        sConnectionDeathRecipient->remove(connectionId);
        return status;
    }
    return ResultStatus::CRITICAL_ERROR;
}

void Accessor::cleanUp(bool clearCache) {
    if (mImpl) {
        mImpl->cleanUp(clearCache);
    }
}

//IAccessor* HIDL_FETCH_IAccessor(const char* /* name */) {
//    return new Accessor();
//}

}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android
