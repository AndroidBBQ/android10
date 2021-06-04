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
#define LOG_TAG "BufferPoolManager"
//#define LOG_NDEBUG 0

#include <bufferpool/ClientManager.h>
#include <hidl/HidlTransportSupport.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utils/Log.h>
#include "BufferPoolClient.h"
#include "Observer.h"
#include "Accessor.h"

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V2_0 {
namespace implementation {

static constexpr int64_t kRegisterTimeoutUs = 500000; // 0.5 sec
static constexpr int64_t kCleanUpDurationUs = 1000000; // TODO: 1 sec tune
static constexpr int64_t kClientTimeoutUs = 5000000; // TODO: 5 secs tune

/**
 * The holder of the cookie of remote IClientManager.
 * The cookie is process locally unique for each IClientManager.
 * (The cookie is used to notify death of clients to bufferpool process.)
 */
class ClientManagerCookieHolder {
public:
    /**
     * Creates a cookie holder for remote IClientManager(s).
     */
    ClientManagerCookieHolder();

    /**
     * Gets a cookie for a remote IClientManager.
     *
     * @param manager   the specified remote IClientManager.
     * @param added     true when the specified remote IClientManager is added
     *                  newly, false otherwise.
     *
     * @return the process locally unique cookie for the specified IClientManager.
     */
    uint64_t getCookie(const sp<IClientManager> &manager, bool *added);

private:
    uint64_t mSeqId;
    std::mutex mLock;
    std::list<std::pair<const wp<IClientManager>, uint64_t>> mManagers;
};

ClientManagerCookieHolder::ClientManagerCookieHolder() : mSeqId(0){}

uint64_t ClientManagerCookieHolder::getCookie(
        const sp<IClientManager> &manager,
        bool *added) {
    std::lock_guard<std::mutex> lock(mLock);
    for (auto it = mManagers.begin(); it != mManagers.end();) {
        const sp<IClientManager> key = it->first.promote();
        if (key) {
            if (interfacesEqual(key, manager)) {
                *added = false;
                return it->second;
            }
            ++it;
        } else {
            it = mManagers.erase(it);
        }
    }
    uint64_t id = mSeqId++;
    *added = true;
    mManagers.push_back(std::make_pair(manager, id));
    return id;
}

class ClientManager::Impl {
public:
    Impl();

    // BnRegisterSender
    ResultStatus registerSender(const sp<IAccessor> &accessor,
                                ConnectionId *pConnectionId);

    // BpRegisterSender
    ResultStatus registerSender(const sp<IClientManager> &receiver,
                                ConnectionId senderId,
                                ConnectionId *receiverId);

    ResultStatus create(const std::shared_ptr<BufferPoolAllocator> &allocator,
                        ConnectionId *pConnectionId);

    ResultStatus close(ConnectionId connectionId);

    ResultStatus flush(ConnectionId connectionId);

    ResultStatus allocate(ConnectionId connectionId,
                          const std::vector<uint8_t> &params,
                          native_handle_t **handle,
                          std::shared_ptr<BufferPoolData> *buffer);

    ResultStatus receive(ConnectionId connectionId,
                         TransactionId transactionId,
                         BufferId bufferId,
                         int64_t timestampUs,
                         native_handle_t **handle,
                         std::shared_ptr<BufferPoolData> *buffer);

    ResultStatus postSend(ConnectionId receiverId,
                          const std::shared_ptr<BufferPoolData> &buffer,
                          TransactionId *transactionId,
                          int64_t *timestampUs);

    ResultStatus getAccessor(ConnectionId connectionId,
                             sp<IAccessor> *accessor);

    void cleanUp(bool clearCache = false);

private:
    // In order to prevent deadlock between multiple locks,
    // always lock ClientCache.lock before locking ActiveClients.lock.
    struct ClientCache {
        // This lock is held for brief duration.
        // Blocking operation is not performed while holding the lock.
        std::mutex mMutex;
        std::list<std::pair<const wp<IAccessor>, const std::weak_ptr<BufferPoolClient>>>
                mClients;
        std::condition_variable mConnectCv;
        bool mConnecting;
        int64_t mLastCleanUpUs;

        ClientCache() : mConnecting(false), mLastCleanUpUs(getTimestampNow()) {}
    } mCache;

    // Active clients which can be retrieved via ConnectionId
    struct ActiveClients {
        // This lock is held for brief duration.
        // Blocking operation is not performed holding the lock.
        std::mutex mMutex;
        std::map<ConnectionId, const std::shared_ptr<BufferPoolClient>>
                mClients;
    } mActive;

    sp<Observer> mObserver;

    ClientManagerCookieHolder mRemoteClientCookies;
};

ClientManager::Impl::Impl()
    : mObserver(new Observer()) {}

ResultStatus ClientManager::Impl::registerSender(
        const sp<IAccessor> &accessor, ConnectionId *pConnectionId) {
    cleanUp();
    int64_t timeoutUs = getTimestampNow() + kRegisterTimeoutUs;
    do {
        std::unique_lock<std::mutex> lock(mCache.mMutex);
        for (auto it = mCache.mClients.begin(); it != mCache.mClients.end(); ++it) {
            sp<IAccessor> sAccessor = it->first.promote();
            if (sAccessor && interfacesEqual(sAccessor, accessor)) {
                const std::shared_ptr<BufferPoolClient> client = it->second.lock();
                if (client) {
                    std::lock_guard<std::mutex> lock(mActive.mMutex);
                    *pConnectionId = client->getConnectionId();
                    if (mActive.mClients.find(*pConnectionId) != mActive.mClients.end()) {
                        ALOGV("register existing connection %lld", (long long)*pConnectionId);
                        return ResultStatus::ALREADY_EXISTS;
                    }
                }
                mCache.mClients.erase(it);
                break;
            }
        }
        if (!mCache.mConnecting) {
            mCache.mConnecting = true;
            lock.unlock();
            ResultStatus result = ResultStatus::OK;
            const std::shared_ptr<BufferPoolClient> client =
                    std::make_shared<BufferPoolClient>(accessor, mObserver);
            lock.lock();
            if (!client) {
                result = ResultStatus::NO_MEMORY;
            } else if (!client->isValid()) {
                result = ResultStatus::CRITICAL_ERROR;
            }
            if (result == ResultStatus::OK) {
                // TODO: handle insert fail. (malloc fail)
                const std::weak_ptr<BufferPoolClient> wclient = client;
                mCache.mClients.push_back(std::make_pair(accessor, wclient));
                ConnectionId conId = client->getConnectionId();
                mObserver->addClient(conId, wclient);
                {
                    std::lock_guard<std::mutex> lock(mActive.mMutex);
                    mActive.mClients.insert(std::make_pair(conId, client));
                }
                *pConnectionId = conId;
                ALOGV("register new connection %lld", (long long)*pConnectionId);
            }
            mCache.mConnecting = false;
            lock.unlock();
            mCache.mConnectCv.notify_all();
            return result;
        }
        mCache.mConnectCv.wait_for(
                lock, std::chrono::microseconds(kRegisterTimeoutUs));
    } while (getTimestampNow() < timeoutUs);
    // TODO: return timeout error
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus ClientManager::Impl::registerSender(
        const sp<IClientManager> &receiver,
        ConnectionId senderId,
        ConnectionId *receiverId) {
    sp<IAccessor> accessor;
    bool local = false;
    {
        std::lock_guard<std::mutex> lock(mActive.mMutex);
        auto it = mActive.mClients.find(senderId);
        if (it == mActive.mClients.end()) {
            return ResultStatus::NOT_FOUND;
        }
        it->second->getAccessor(&accessor);
        local = it->second->isLocal();
    }
    ResultStatus rs = ResultStatus::CRITICAL_ERROR;
    if (accessor) {
       Return<void> transResult = receiver->registerSender(
                accessor,
                [&rs, receiverId](
                        ResultStatus status,
                        int64_t connectionId) {
                    rs = status;
                    *receiverId = connectionId;
                });
        if (!transResult.isOk()) {
            return ResultStatus::CRITICAL_ERROR;
        } else if (local && rs == ResultStatus::OK) {
            sp<ConnectionDeathRecipient> recipient = Accessor::getConnectionDeathRecipient();
            if (recipient)  {
                ALOGV("client death recipient registered %lld", (long long)*receiverId);
                bool added;
                uint64_t cookie = mRemoteClientCookies.getCookie(receiver, &added);
                recipient->addCookieToConnection(cookie, *receiverId);
                if (added) {
                    Return<bool> transResult = receiver->linkToDeath(recipient, cookie);
                }
            }
        }
    }
    return rs;
}

ResultStatus ClientManager::Impl::create(
        const std::shared_ptr<BufferPoolAllocator> &allocator,
        ConnectionId *pConnectionId) {
    const sp<Accessor> accessor = new Accessor(allocator);
    if (!accessor || !accessor->isValid()) {
        return ResultStatus::CRITICAL_ERROR;
    }
    // TODO: observer is local. use direct call instead of hidl call.
    std::shared_ptr<BufferPoolClient> client =
            std::make_shared<BufferPoolClient>(accessor, mObserver);
    if (!client || !client->isValid()) {
        return ResultStatus::CRITICAL_ERROR;
    }
    // Since a new bufferpool is created, evict memories which are used by
    // existing bufferpools and clients.
    cleanUp(true);
    {
        // TODO: handle insert fail. (malloc fail)
        std::lock_guard<std::mutex> lock(mCache.mMutex);
        const std::weak_ptr<BufferPoolClient> wclient = client;
        mCache.mClients.push_back(std::make_pair(accessor, wclient));
        ConnectionId conId = client->getConnectionId();
        mObserver->addClient(conId, wclient);
        {
            std::lock_guard<std::mutex> lock(mActive.mMutex);
            mActive.mClients.insert(std::make_pair(conId, client));
        }
        *pConnectionId = conId;
        ALOGV("create new connection %lld", (long long)*pConnectionId);
    }
    return ResultStatus::OK;
}

ResultStatus ClientManager::Impl::close(ConnectionId connectionId) {
    std::unique_lock<std::mutex> lock1(mCache.mMutex);
    std::unique_lock<std::mutex> lock2(mActive.mMutex);
    auto it = mActive.mClients.find(connectionId);
    if (it != mActive.mClients.end()) {
        sp<IAccessor> accessor;
        it->second->getAccessor(&accessor);
        std::shared_ptr<BufferPoolClient> closing = it->second;
        mActive.mClients.erase(connectionId);
        for (auto cit = mCache.mClients.begin(); cit != mCache.mClients.end();) {
            // clean up dead client caches
            sp<IAccessor> cAccessor = cit->first.promote();
            if (!cAccessor || (accessor && interfacesEqual(cAccessor, accessor))) {
                cit = mCache.mClients.erase(cit);
            } else {
                cit++;
            }
        }
        lock2.unlock();
        lock1.unlock();
        closing->flush();
        return ResultStatus::OK;
    }
    return ResultStatus::NOT_FOUND;
}

ResultStatus ClientManager::Impl::flush(ConnectionId connectionId) {
    std::shared_ptr<BufferPoolClient> client;
    {
        std::lock_guard<std::mutex> lock(mActive.mMutex);
        auto it = mActive.mClients.find(connectionId);
        if (it == mActive.mClients.end()) {
            return ResultStatus::NOT_FOUND;
        }
        client = it->second;
    }
    return client->flush();
}

ResultStatus ClientManager::Impl::allocate(
        ConnectionId connectionId, const std::vector<uint8_t> &params,
        native_handle_t **handle, std::shared_ptr<BufferPoolData> *buffer) {
    std::shared_ptr<BufferPoolClient> client;
    {
        std::lock_guard<std::mutex> lock(mActive.mMutex);
        auto it = mActive.mClients.find(connectionId);
        if (it == mActive.mClients.end()) {
            return ResultStatus::NOT_FOUND;
        }
        client = it->second;
    }
#ifdef BUFFERPOOL_CLONE_HANDLES
    native_handle_t *origHandle;
    ResultStatus res = client->allocate(params, &origHandle, buffer);
    if (res != ResultStatus::OK) {
        return res;
    }
    *handle = native_handle_clone(origHandle);
    if (handle == NULL) {
        buffer->reset();
        return ResultStatus::NO_MEMORY;
    }
    return ResultStatus::OK;
#else
    return client->allocate(params, handle, buffer);
#endif
}

ResultStatus ClientManager::Impl::receive(
        ConnectionId connectionId, TransactionId transactionId,
        BufferId bufferId, int64_t timestampUs,
        native_handle_t **handle, std::shared_ptr<BufferPoolData> *buffer) {
    std::shared_ptr<BufferPoolClient> client;
    {
        std::lock_guard<std::mutex> lock(mActive.mMutex);
        auto it = mActive.mClients.find(connectionId);
        if (it == mActive.mClients.end()) {
            return ResultStatus::NOT_FOUND;
        }
        client = it->second;
    }
#ifdef BUFFERPOOL_CLONE_HANDLES
    native_handle_t *origHandle;
    ResultStatus res = client->receive(
            transactionId, bufferId, timestampUs, &origHandle, buffer);
    if (res != ResultStatus::OK) {
        return res;
    }
    *handle = native_handle_clone(origHandle);
    if (handle == NULL) {
        buffer->reset();
        return ResultStatus::NO_MEMORY;
    }
    return ResultStatus::OK;
#else
    return client->receive(transactionId, bufferId, timestampUs, handle, buffer);
#endif
}

ResultStatus ClientManager::Impl::postSend(
        ConnectionId receiverId, const std::shared_ptr<BufferPoolData> &buffer,
        TransactionId *transactionId, int64_t *timestampUs) {
    ConnectionId connectionId = buffer->mConnectionId;
    std::shared_ptr<BufferPoolClient> client;
    {
        std::lock_guard<std::mutex> lock(mActive.mMutex);
        auto it = mActive.mClients.find(connectionId);
        if (it == mActive.mClients.end()) {
            return ResultStatus::NOT_FOUND;
        }
        client = it->second;
    }
    return client->postSend(receiverId, buffer, transactionId, timestampUs);
}

ResultStatus ClientManager::Impl::getAccessor(
        ConnectionId connectionId, sp<IAccessor> *accessor) {
    std::shared_ptr<BufferPoolClient> client;
    {
        std::lock_guard<std::mutex> lock(mActive.mMutex);
        auto it = mActive.mClients.find(connectionId);
        if (it == mActive.mClients.end()) {
            return ResultStatus::NOT_FOUND;
        }
        client = it->second;
    }
    return client->getAccessor(accessor);
}

void ClientManager::Impl::cleanUp(bool clearCache) {
    int64_t now = getTimestampNow();
    int64_t lastTransactionUs;
    std::lock_guard<std::mutex> lock1(mCache.mMutex);
    if (clearCache || mCache.mLastCleanUpUs + kCleanUpDurationUs < now) {
        std::lock_guard<std::mutex> lock2(mActive.mMutex);
        int cleaned = 0;
        for (auto it = mActive.mClients.begin(); it != mActive.mClients.end();) {
            if (!it->second->isActive(&lastTransactionUs, clearCache)) {
                if (lastTransactionUs + kClientTimeoutUs < now) {
                    sp<IAccessor> accessor;
                    it->second->getAccessor(&accessor);
                    it = mActive.mClients.erase(it);
                    ++cleaned;
                    continue;
                }
            }
            ++it;
        }
        for (auto cit = mCache.mClients.begin(); cit != mCache.mClients.end();) {
            // clean up dead client caches
            sp<IAccessor> cAccessor = cit->first.promote();
            if (!cAccessor) {
                cit = mCache.mClients.erase(cit);
            } else {
                ++cit;
            }
        }
        ALOGV("# of cleaned connections: %d", cleaned);
        mCache.mLastCleanUpUs = now;
    }
}

// Methods from ::android::hardware::media::bufferpool::V2_0::IClientManager follow.
Return<void> ClientManager::registerSender(const sp<::android::hardware::media::bufferpool::V2_0::IAccessor>& bufferPool, registerSender_cb _hidl_cb) {
    if (mImpl) {
        ConnectionId connectionId = -1;
        ResultStatus status = mImpl->registerSender(bufferPool, &connectionId);
        _hidl_cb(status, connectionId);
    } else {
        _hidl_cb(ResultStatus::CRITICAL_ERROR, -1);
    }
    return Void();
}

// Methods for local use.
sp<ClientManager> ClientManager::sInstance;
std::mutex ClientManager::sInstanceLock;

sp<ClientManager> ClientManager::getInstance() {
    std::lock_guard<std::mutex> lock(sInstanceLock);
    if (!sInstance) {
        sInstance = new ClientManager();
    }
    Accessor::createInvalidator();
    return sInstance;
}

ClientManager::ClientManager() : mImpl(new Impl()) {}

ClientManager::~ClientManager() {
}

ResultStatus ClientManager::create(
        const std::shared_ptr<BufferPoolAllocator> &allocator,
        ConnectionId *pConnectionId) {
    if (mImpl) {
        return mImpl->create(allocator, pConnectionId);
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus ClientManager::registerSender(
        const sp<IClientManager> &receiver,
        ConnectionId senderId,
        ConnectionId *receiverId) {
    if (mImpl) {
        return mImpl->registerSender(receiver, senderId, receiverId);
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus ClientManager::close(ConnectionId connectionId) {
    if (mImpl) {
        return mImpl->close(connectionId);
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus ClientManager::flush(ConnectionId connectionId) {
    if (mImpl) {
        return mImpl->flush(connectionId);
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus ClientManager::allocate(
        ConnectionId connectionId, const std::vector<uint8_t> &params,
        native_handle_t **handle, std::shared_ptr<BufferPoolData> *buffer) {
    if (mImpl) {
        return mImpl->allocate(connectionId, params, handle, buffer);
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus ClientManager::receive(
        ConnectionId connectionId, TransactionId transactionId,
        BufferId bufferId, int64_t timestampUs,
        native_handle_t **handle, std::shared_ptr<BufferPoolData> *buffer) {
    if (mImpl) {
        return mImpl->receive(connectionId, transactionId, bufferId,
                              timestampUs, handle, buffer);
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus ClientManager::postSend(
        ConnectionId receiverId, const std::shared_ptr<BufferPoolData> &buffer,
        TransactionId *transactionId, int64_t* timestampUs) {
    if (mImpl && buffer) {
        return mImpl->postSend(receiverId, buffer, transactionId, timestampUs);
    }
    return ResultStatus::CRITICAL_ERROR;
}

void ClientManager::cleanUp() {
    if (mImpl) {
        mImpl->cleanUp(true);
    }
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android
