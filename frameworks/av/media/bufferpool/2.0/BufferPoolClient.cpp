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

#define LOG_TAG "BufferPoolClient"
//#define LOG_NDEBUG 0

#include <thread>
#include <utils/Log.h>
#include "BufferPoolClient.h"
#include "Connection.h"

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V2_0 {
namespace implementation {

static constexpr int64_t kReceiveTimeoutUs = 1000000; // 100ms
static constexpr int kPostMaxRetry = 3;
static constexpr int kCacheTtlUs = 1000000; // TODO: tune

class BufferPoolClient::Impl
        : public std::enable_shared_from_this<BufferPoolClient::Impl> {
public:
    explicit Impl(const sp<Accessor> &accessor, const sp<IObserver> &observer);

    explicit Impl(const sp<IAccessor> &accessor, const sp<IObserver> &observer);

    bool isValid() {
        return mValid;
    }

    bool isLocal() {
        return mValid && mLocal;
    }

    ConnectionId getConnectionId() {
        return mConnectionId;
    }

    sp<IAccessor> &getAccessor() {
        return mAccessor;
    }

    bool isActive(int64_t *lastTransactionUs, bool clearCache);

    void receiveInvalidation(uint32_t msgID);

    ResultStatus flush();

    ResultStatus allocate(const std::vector<uint8_t> &params,
                          native_handle_t **handle,
                          std::shared_ptr<BufferPoolData> *buffer);

    ResultStatus receive(
            TransactionId transactionId, BufferId bufferId,
            int64_t timestampUs,
            native_handle_t **handle, std::shared_ptr<BufferPoolData> *buffer);

    void postBufferRelease(BufferId bufferId);

    bool postSend(
            BufferId bufferId, ConnectionId receiver,
            TransactionId *transactionId, int64_t *timestampUs);
private:

    bool postReceive(
            BufferId bufferId, TransactionId transactionId,
            int64_t timestampUs);

    bool postReceiveResult(
            BufferId bufferId, TransactionId transactionId, bool result, bool *needsSync);

    void trySyncFromRemote();

    bool syncReleased(uint32_t msgId = 0);

    void evictCaches(bool clearCache = false);

    void invalidateBuffer(BufferId id);

    void invalidateRange(BufferId from, BufferId to);

    ResultStatus allocateBufferHandle(
            const std::vector<uint8_t>& params, BufferId *bufferId,
            native_handle_t **handle);

    ResultStatus fetchBufferHandle(
            TransactionId transactionId, BufferId bufferId,
            native_handle_t **handle);

    struct BlockPoolDataDtor;
    struct ClientBuffer;

    bool mLocal;
    bool mValid;
    sp<IAccessor> mAccessor;
    sp<Connection> mLocalConnection;
    sp<IConnection> mRemoteConnection;
    uint32_t mSeqId;
    ConnectionId mConnectionId;
    int64_t mLastEvictCacheUs;
    std::unique_ptr<BufferInvalidationListener> mInvalidationListener;

    // CachedBuffers
    struct BufferCache {
        std::mutex mLock;
        bool mCreating;
        std::condition_variable mCreateCv;
        std::map<BufferId, std::unique_ptr<ClientBuffer>> mBuffers;
        int mActive;
        int64_t mLastChangeUs;

        BufferCache() : mCreating(false), mActive(0), mLastChangeUs(getTimestampNow()) {}

        void incActive_l() {
            ++mActive;
            mLastChangeUs = getTimestampNow();
        }

        void decActive_l() {
            --mActive;
            mLastChangeUs = getTimestampNow();
        }
    } mCache;

    // FMQ - release notifier
    struct ReleaseCache {
        std::mutex mLock;
        // TODO: use only one list?(using one list may dealy sending messages?)
        std::list<BufferId> mReleasingIds;
        std::list<BufferId> mReleasedIds;
        uint32_t mInvalidateId; // TODO: invalidation ACK to bufferpool
        bool mInvalidateAck;
        std::unique_ptr<BufferStatusChannel> mStatusChannel;

        ReleaseCache() : mInvalidateId(0), mInvalidateAck(true) {}
    } mReleasing;

    // This lock is held during synchronization from remote side.
    // In order to minimize remote calls and locking durtaion, this lock is held
    // by best effort approach using try_lock().
    std::mutex mRemoteSyncLock;
};

struct BufferPoolClient::Impl::BlockPoolDataDtor {
    BlockPoolDataDtor(const std::shared_ptr<BufferPoolClient::Impl> &impl)
            : mImpl(impl) {}

    void operator()(BufferPoolData *buffer) {
        BufferId id = buffer->mId;
        delete buffer;

        auto impl = mImpl.lock();
        if (impl && impl->isValid()) {
            impl->postBufferRelease(id);
        }
    }
    const std::weak_ptr<BufferPoolClient::Impl> mImpl;
};

struct BufferPoolClient::Impl::ClientBuffer {
private:
    int64_t mExpireUs;
    bool mHasCache;
    ConnectionId mConnectionId;
    BufferId mId;
    native_handle_t *mHandle;
    std::weak_ptr<BufferPoolData> mCache;

    void updateExpire() {
        mExpireUs = getTimestampNow() + kCacheTtlUs;
    }

public:
    ClientBuffer(
            ConnectionId connectionId, BufferId id, native_handle_t *handle)
            : mHasCache(false), mConnectionId(connectionId),
              mId(id), mHandle(handle) {
        mExpireUs = getTimestampNow() + kCacheTtlUs;
    }

    ~ClientBuffer() {
        if (mHandle) {
            native_handle_close(mHandle);
            native_handle_delete(mHandle);
        }
    }

    BufferId id() const {
        return mId;
    }

    bool expire() const {
        int64_t now = getTimestampNow();
        return now >= mExpireUs;
    }

    bool hasCache() const {
        return mHasCache;
    }

    std::shared_ptr<BufferPoolData> fetchCache(native_handle_t **pHandle) {
        if (mHasCache) {
            std::shared_ptr<BufferPoolData> cache = mCache.lock();
            if (cache) {
                *pHandle = mHandle;
            }
            return cache;
        }
        return nullptr;
    }

    std::shared_ptr<BufferPoolData> createCache(
            const std::shared_ptr<BufferPoolClient::Impl> &impl,
            native_handle_t **pHandle) {
        if (!mHasCache) {
            // Allocates a raw ptr in order to avoid sending #postBufferRelease
            // from deleter, in case of native_handle_clone failure.
            BufferPoolData *ptr = new BufferPoolData(mConnectionId, mId);
            if (ptr) {
                std::shared_ptr<BufferPoolData> cache(ptr, BlockPoolDataDtor(impl));
                if (cache) {
                    mCache = cache;
                    mHasCache = true;
                    *pHandle = mHandle;
                    return cache;
                }
            }
            if (ptr) {
                delete ptr;
            }
        }
        return nullptr;
    }

    bool onCacheRelease() {
        if (mHasCache) {
            // TODO: verify mCache is not valid;
            updateExpire();
            mHasCache = false;
            return true;
        }
        return false;
    }
};

BufferPoolClient::Impl::Impl(const sp<Accessor> &accessor, const sp<IObserver> &observer)
    : mLocal(true), mValid(false), mAccessor(accessor), mSeqId(0),
      mLastEvictCacheUs(getTimestampNow()) {
    const StatusDescriptor *statusDesc;
    const InvalidationDescriptor *invDesc;
    ResultStatus status = accessor->connect(
            observer, true,
            &mLocalConnection, &mConnectionId, &mReleasing.mInvalidateId,
            &statusDesc, &invDesc);
    if (status == ResultStatus::OK) {
        mReleasing.mStatusChannel =
                std::make_unique<BufferStatusChannel>(*statusDesc);
        mInvalidationListener =
                std::make_unique<BufferInvalidationListener>(*invDesc);
        mValid = mReleasing.mStatusChannel &&
                mReleasing.mStatusChannel->isValid() &&
                mInvalidationListener &&
                mInvalidationListener->isValid();
    }
}

BufferPoolClient::Impl::Impl(const sp<IAccessor> &accessor, const sp<IObserver> &observer)
    : mLocal(false), mValid(false), mAccessor(accessor), mSeqId(0),
      mLastEvictCacheUs(getTimestampNow()) {
    bool valid = false;
    sp<IConnection>& outConnection = mRemoteConnection;
    ConnectionId& id = mConnectionId;
    uint32_t& outMsgId = mReleasing.mInvalidateId;
    std::unique_ptr<BufferStatusChannel>& outChannel =
            mReleasing.mStatusChannel;
    std::unique_ptr<BufferInvalidationListener>& outObserver =
            mInvalidationListener;
    Return<void> transResult = accessor->connect(
            observer,
            [&valid, &outConnection, &id, &outMsgId, &outChannel, &outObserver]
            (ResultStatus status, sp<IConnection> connection,
             ConnectionId connectionId, uint32_t msgId,
             const StatusDescriptor& statusDesc,
             const InvalidationDescriptor& invDesc) {
                if (status == ResultStatus::OK) {
                    outConnection = connection;
                    id = connectionId;
                    outMsgId = msgId;
                    outChannel = std::make_unique<BufferStatusChannel>(statusDesc);
                    outObserver = std::make_unique<BufferInvalidationListener>(invDesc);
                    if (outChannel && outChannel->isValid() &&
                        outObserver && outObserver->isValid()) {
                        valid = true;
                    }
                }
            });
    mValid = transResult.isOk() && valid;
}

bool BufferPoolClient::Impl::isActive(int64_t *lastTransactionUs, bool clearCache) {
    bool active = false;
    {
        std::lock_guard<std::mutex> lock(mCache.mLock);
        syncReleased();
        evictCaches(clearCache);
        *lastTransactionUs = mCache.mLastChangeUs;
        active = mCache.mActive > 0;
    }
    if (mValid && mLocal && mLocalConnection) {
        mLocalConnection->cleanUp(clearCache);
        return true;
    }
    return active;
}

void BufferPoolClient::Impl::receiveInvalidation(uint32_t messageId) {
    std::lock_guard<std::mutex> lock(mCache.mLock);
    syncReleased(messageId);
    // TODO: evict cache required?
}

ResultStatus BufferPoolClient::Impl::flush() {
    if (!mLocal || !mLocalConnection || !mValid) {
        return ResultStatus::CRITICAL_ERROR;
    }
    {
        std::unique_lock<std::mutex> lock(mCache.mLock);
        syncReleased();
        evictCaches();
        return mLocalConnection->flush();
    }
}

ResultStatus BufferPoolClient::Impl::allocate(
        const std::vector<uint8_t> &params,
        native_handle_t **pHandle,
        std::shared_ptr<BufferPoolData> *buffer) {
    if (!mLocal || !mLocalConnection || !mValid) {
        return ResultStatus::CRITICAL_ERROR;
    }
    BufferId bufferId;
    native_handle_t *handle = nullptr;
    buffer->reset();
    ResultStatus status = allocateBufferHandle(params, &bufferId, &handle);
    if (status == ResultStatus::OK) {
        if (handle) {
            std::unique_lock<std::mutex> lock(mCache.mLock);
            syncReleased();
            evictCaches();
            auto cacheIt = mCache.mBuffers.find(bufferId);
            if (cacheIt != mCache.mBuffers.end()) {
                // TODO: verify it is recycled. (not having active ref)
                mCache.mBuffers.erase(cacheIt);
            }
            auto clientBuffer = std::make_unique<ClientBuffer>(
                    mConnectionId, bufferId, handle);
            if (clientBuffer) {
                auto result = mCache.mBuffers.insert(std::make_pair(
                        bufferId, std::move(clientBuffer)));
                if (result.second) {
                    *buffer = result.first->second->createCache(
                            shared_from_this(), pHandle);
                    if (*buffer) {
                        mCache.incActive_l();
                    }
                }
            }
        }
        if (!*buffer) {
            ALOGV("client cache creation failure %d: %lld",
                  handle != nullptr, (long long)mConnectionId);
            status = ResultStatus::NO_MEMORY;
            postBufferRelease(bufferId);
        }
    }
    return status;
}

ResultStatus BufferPoolClient::Impl::receive(
        TransactionId transactionId, BufferId bufferId, int64_t timestampUs,
        native_handle_t **pHandle,
        std::shared_ptr<BufferPoolData> *buffer) {
    if (!mValid) {
        return ResultStatus::CRITICAL_ERROR;
    }
    if (timestampUs != 0) {
        timestampUs += kReceiveTimeoutUs;
    }
    if (!postReceive(bufferId, transactionId, timestampUs)) {
        return ResultStatus::CRITICAL_ERROR;
    }
    ResultStatus status = ResultStatus::CRITICAL_ERROR;
    buffer->reset();
    while(1) {
        std::unique_lock<std::mutex> lock(mCache.mLock);
        syncReleased();
        evictCaches();
        auto cacheIt = mCache.mBuffers.find(bufferId);
        if (cacheIt != mCache.mBuffers.end()) {
            if (cacheIt->second->hasCache()) {
                *buffer = cacheIt->second->fetchCache(pHandle);
                if (!*buffer) {
                    // check transfer time_out
                    lock.unlock();
                    std::this_thread::yield();
                    continue;
                }
                ALOGV("client receive from reference %lld", (long long)mConnectionId);
                break;
            } else {
                *buffer = cacheIt->second->createCache(shared_from_this(), pHandle);
                if (*buffer) {
                    mCache.incActive_l();
                }
                ALOGV("client receive from cache %lld", (long long)mConnectionId);
                break;
            }
        } else {
            if (!mCache.mCreating) {
                mCache.mCreating = true;
                lock.unlock();
                native_handle_t* handle = nullptr;
                status = fetchBufferHandle(transactionId, bufferId, &handle);
                lock.lock();
                if (status == ResultStatus::OK) {
                    if (handle) {
                        auto clientBuffer = std::make_unique<ClientBuffer>(
                                mConnectionId, bufferId, handle);
                        if (clientBuffer) {
                            auto result = mCache.mBuffers.insert(
                                    std::make_pair(bufferId, std::move(
                                            clientBuffer)));
                            if (result.second) {
                                *buffer = result.first->second->createCache(
                                        shared_from_this(), pHandle);
                                if (*buffer) {
                                    mCache.incActive_l();
                                }
                            }
                        }
                    }
                    if (!*buffer) {
                        status = ResultStatus::NO_MEMORY;
                    }
                }
                mCache.mCreating = false;
                lock.unlock();
                mCache.mCreateCv.notify_all();
                break;
            }
            mCache.mCreateCv.wait(lock);
        }
    }
    bool needsSync = false;
    bool posted = postReceiveResult(bufferId, transactionId,
                                      *buffer ? true : false, &needsSync);
    ALOGV("client receive %lld - %u : %s (%d)", (long long)mConnectionId, bufferId,
          *buffer ? "ok" : "fail", posted);
    if (mValid && mLocal && mLocalConnection) {
        mLocalConnection->cleanUp(false);
    }
    if (needsSync && mRemoteConnection) {
        trySyncFromRemote();
    }
    if (*buffer) {
        if (!posted) {
            buffer->reset();
            return ResultStatus::CRITICAL_ERROR;
        }
        return ResultStatus::OK;
    }
    return status;
}


void BufferPoolClient::Impl::postBufferRelease(BufferId bufferId) {
    std::lock_guard<std::mutex> lock(mReleasing.mLock);
    mReleasing.mReleasingIds.push_back(bufferId);
    mReleasing.mStatusChannel->postBufferRelease(
            mConnectionId, mReleasing.mReleasingIds, mReleasing.mReleasedIds);
}

// TODO: revise ad-hoc posting data structure
bool BufferPoolClient::Impl::postSend(
        BufferId bufferId, ConnectionId receiver,
        TransactionId *transactionId, int64_t *timestampUs) {
    {
        // TODO: don't need to call syncReleased every time
        std::lock_guard<std::mutex> lock(mCache.mLock);
        syncReleased();
    }
    bool ret = false;
    bool needsSync = false;
    {
        std::lock_guard<std::mutex> lock(mReleasing.mLock);
        *timestampUs = getTimestampNow();
        *transactionId = (mConnectionId << 32) | mSeqId++;
        // TODO: retry, add timeout, target?
        ret =  mReleasing.mStatusChannel->postBufferStatusMessage(
                *transactionId, bufferId, BufferStatus::TRANSFER_TO, mConnectionId,
                receiver, mReleasing.mReleasingIds, mReleasing.mReleasedIds);
        needsSync = !mLocal && mReleasing.mStatusChannel->needsSync();
    }
    if (mValid && mLocal && mLocalConnection) {
        mLocalConnection->cleanUp(false);
    }
    if (needsSync && mRemoteConnection) {
        trySyncFromRemote();
    }
    return ret;
}

bool BufferPoolClient::Impl::postReceive(
        BufferId bufferId, TransactionId transactionId, int64_t timestampUs) {
    for (int i = 0; i < kPostMaxRetry; ++i) {
        std::unique_lock<std::mutex> lock(mReleasing.mLock);
        int64_t now = getTimestampNow();
        if (timestampUs == 0 || now < timestampUs) {
            bool result = mReleasing.mStatusChannel->postBufferStatusMessage(
                    transactionId, bufferId, BufferStatus::TRANSFER_FROM,
                    mConnectionId, -1, mReleasing.mReleasingIds,
                    mReleasing.mReleasedIds);
            if (result) {
                return true;
            }
            lock.unlock();
            std::this_thread::yield();
        } else {
            mReleasing.mStatusChannel->postBufferStatusMessage(
                    transactionId, bufferId, BufferStatus::TRANSFER_TIMEOUT,
                    mConnectionId, -1, mReleasing.mReleasingIds,
                    mReleasing.mReleasedIds);
            return false;
        }
    }
    return false;
}

bool BufferPoolClient::Impl::postReceiveResult(
        BufferId bufferId, TransactionId transactionId, bool result, bool *needsSync) {
    std::lock_guard<std::mutex> lock(mReleasing.mLock);
    // TODO: retry, add timeout
    bool ret = mReleasing.mStatusChannel->postBufferStatusMessage(
            transactionId, bufferId,
            result ? BufferStatus::TRANSFER_OK : BufferStatus::TRANSFER_ERROR,
            mConnectionId, -1, mReleasing.mReleasingIds,
            mReleasing.mReleasedIds);
    *needsSync = !mLocal && mReleasing.mStatusChannel->needsSync();
    return ret;
}

void BufferPoolClient::Impl::trySyncFromRemote() {
    if (mRemoteSyncLock.try_lock()) {
        bool needsSync = false;
        {
            std::lock_guard<std::mutex> lock(mReleasing.mLock);
            needsSync = mReleasing.mStatusChannel->needsSync();
        }
        if (needsSync) {
            TransactionId transactionId = (mConnectionId << 32);
            BufferId bufferId = Connection::SYNC_BUFFERID;
            Return<void> transResult = mRemoteConnection->fetch(
                    transactionId, bufferId,
                    []
                    (ResultStatus outStatus, Buffer outBuffer) {
                        (void) outStatus;
                        (void) outBuffer;
                    });
            if (!transResult.isOk()) {
                ALOGD("sync from client %lld failed: bufferpool process died.",
                      (long long)mConnectionId);
            }
        }
        mRemoteSyncLock.unlock();
    }
}

// should have mCache.mLock
bool BufferPoolClient::Impl::syncReleased(uint32_t messageId) {
    bool cleared = false;
    {
        std::lock_guard<std::mutex> lock(mReleasing.mLock);
        if (mReleasing.mReleasingIds.size() > 0) {
            mReleasing.mStatusChannel->postBufferRelease(
                    mConnectionId, mReleasing.mReleasingIds,
                    mReleasing.mReleasedIds);
        }
        if (mReleasing.mReleasedIds.size() > 0) {
            for (BufferId& id: mReleasing.mReleasedIds) {
                ALOGV("client release buffer %lld - %u", (long long)mConnectionId, id);
                auto found = mCache.mBuffers.find(id);
                if (found != mCache.mBuffers.end()) {
                    if (found->second->onCacheRelease()) {
                        mCache.decActive_l();
                    } else {
                        // should not happen!
                        ALOGW("client %lld cache release status inconsitent!",
                            (long long)mConnectionId);
                    }
                } else {
                    // should not happen!
                    ALOGW("client %lld cache status inconsitent!", (long long)mConnectionId);
                }
            }
            mReleasing.mReleasedIds.clear();
            cleared = true;
        }
    }
    std::vector<BufferInvalidationMessage> invalidations;
    mInvalidationListener->getInvalidations(invalidations);
    uint32_t lastMsgId = 0;
    if (invalidations.size() > 0) {
        for (auto it = invalidations.begin(); it != invalidations.end(); ++it) {
            if (it->messageId != 0) {
                lastMsgId = it->messageId;
            }
            if (it->fromBufferId == it->toBufferId) {
                // TODO: handle fromBufferId = UINT32_MAX
                invalidateBuffer(it->fromBufferId);
            } else {
                invalidateRange(it->fromBufferId, it->toBufferId);
            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(mReleasing.mLock);
        if (lastMsgId != 0) {
            if (isMessageLater(lastMsgId, mReleasing.mInvalidateId)) {
                mReleasing.mInvalidateId = lastMsgId;
                mReleasing.mInvalidateAck = false;
            }
        } else if (messageId != 0) {
            // messages are drained.
            if (isMessageLater(messageId, mReleasing.mInvalidateId)) {
                mReleasing.mInvalidateId = messageId;
                mReleasing.mInvalidateAck = true;
            }
        }
        if (!mReleasing.mInvalidateAck) {
            // post ACK
            mReleasing.mStatusChannel->postBufferInvalidateAck(
                    mConnectionId,
                    mReleasing.mInvalidateId, &mReleasing.mInvalidateAck);
            ALOGV("client %lld invalidateion ack (%d) %u",
                (long long)mConnectionId,
                mReleasing.mInvalidateAck, mReleasing.mInvalidateId);
        }
    }
    return cleared;
}

// should have mCache.mLock
void BufferPoolClient::Impl::evictCaches(bool clearCache) {
    int64_t now = getTimestampNow();
    if (now >= mLastEvictCacheUs + kCacheTtlUs || clearCache) {
        size_t evicted = 0;
        for (auto it = mCache.mBuffers.begin(); it != mCache.mBuffers.end();) {
            if (!it->second->hasCache() && (it->second->expire() || clearCache)) {
                it = mCache.mBuffers.erase(it);
                ++evicted;
            } else {
                ++it;
            }
        }
        ALOGV("cache count %lld : total %zu, active %d, evicted %zu",
              (long long)mConnectionId, mCache.mBuffers.size(), mCache.mActive, evicted);
        mLastEvictCacheUs = now;
    }
}

// should have mCache.mLock
void BufferPoolClient::Impl::invalidateBuffer(BufferId id) {
    for (auto it = mCache.mBuffers.begin(); it != mCache.mBuffers.end(); ++it) {
        if (id == it->second->id()) {
            if (!it->second->hasCache()) {
                mCache.mBuffers.erase(it);
                ALOGV("cache invalidated %lld : buffer %u",
                      (long long)mConnectionId, id);
            } else {
                ALOGW("Inconsitent invalidation %lld : activer buffer!! %u",
                      (long long)mConnectionId, (unsigned int)id);
            }
            break;
        }
    }
}

// should have mCache.mLock
void BufferPoolClient::Impl::invalidateRange(BufferId from, BufferId to) {
    size_t invalidated = 0;
    for (auto it = mCache.mBuffers.begin(); it != mCache.mBuffers.end();) {
        if (!it->second->hasCache()) {
            BufferId bid = it->second->id();
            if (from < to) {
                if (from <= bid && bid < to) {
                    ++invalidated;
                    it = mCache.mBuffers.erase(it);
                    continue;
                }
            } else {
                if (from <= bid || bid < to) {
                    ++invalidated;
                    it = mCache.mBuffers.erase(it);
                    continue;
                }
            }
        }
        ++it;
    }
    ALOGV("cache invalidated %lld : # of invalidated %zu",
          (long long)mConnectionId, invalidated);
}

ResultStatus BufferPoolClient::Impl::allocateBufferHandle(
        const std::vector<uint8_t>& params, BufferId *bufferId,
        native_handle_t** handle) {
    if (mLocalConnection) {
        const native_handle_t* allocHandle = nullptr;
        ResultStatus status = mLocalConnection->allocate(
                params, bufferId, &allocHandle);
        if (status == ResultStatus::OK) {
            *handle = native_handle_clone(allocHandle);
        }
        ALOGV("client allocate result %lld %d : %u clone %p",
              (long long)mConnectionId, status == ResultStatus::OK,
              *handle ? *bufferId : 0 , *handle);
        return status;
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus BufferPoolClient::Impl::fetchBufferHandle(
        TransactionId transactionId, BufferId bufferId,
        native_handle_t **handle) {
    sp<IConnection> connection;
    if (mLocal) {
        connection = mLocalConnection;
    } else {
        connection = mRemoteConnection;
    }
    ResultStatus status;
    Return<void> transResult = connection->fetch(
            transactionId, bufferId,
            [&status, &handle]
            (ResultStatus outStatus, Buffer outBuffer) {
                status = outStatus;
                if (status == ResultStatus::OK) {
                    *handle = native_handle_clone(
                            outBuffer.buffer.getNativeHandle());
                }
            });
    return transResult.isOk() ? status : ResultStatus::CRITICAL_ERROR;
}


BufferPoolClient::BufferPoolClient(const sp<Accessor> &accessor,
                                   const sp<IObserver> &observer) {
    mImpl = std::make_shared<Impl>(accessor, observer);
}

BufferPoolClient::BufferPoolClient(const sp<IAccessor> &accessor,
                                   const sp<IObserver> &observer) {
    mImpl = std::make_shared<Impl>(accessor, observer);
}

BufferPoolClient::~BufferPoolClient() {
    // TODO: how to handle orphaned buffers?
}

bool BufferPoolClient::isValid() {
    return mImpl && mImpl->isValid();
}

bool BufferPoolClient::isLocal() {
    return mImpl && mImpl->isLocal();
}

bool BufferPoolClient::isActive(int64_t *lastTransactionUs, bool clearCache) {
    if (!isValid()) {
        *lastTransactionUs = 0;
        return false;
    }
    return mImpl->isActive(lastTransactionUs, clearCache);
}

ConnectionId BufferPoolClient::getConnectionId() {
    if (isValid()) {
        return mImpl->getConnectionId();
    }
    return -1;
}

ResultStatus BufferPoolClient::getAccessor(sp<IAccessor> *accessor) {
    if (isValid()) {
        *accessor = mImpl->getAccessor();
        return ResultStatus::OK;
    }
    return ResultStatus::CRITICAL_ERROR;
}

void BufferPoolClient::receiveInvalidation(uint32_t msgId) {
    ALOGV("bufferpool2 client recv inv %u", msgId);
    if (isValid()) {
        mImpl->receiveInvalidation(msgId);
    }
}

ResultStatus BufferPoolClient::flush() {
    if (isValid()) {
        return mImpl->flush();
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus BufferPoolClient::allocate(
        const std::vector<uint8_t> &params,
        native_handle_t **handle,
        std::shared_ptr<BufferPoolData> *buffer) {
    if (isValid()) {
        return mImpl->allocate(params, handle, buffer);
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus BufferPoolClient::receive(
        TransactionId transactionId, BufferId bufferId, int64_t timestampUs,
        native_handle_t **handle, std::shared_ptr<BufferPoolData> *buffer) {
    if (isValid()) {
        return mImpl->receive(transactionId, bufferId, timestampUs, handle, buffer);
    }
    return ResultStatus::CRITICAL_ERROR;
}

ResultStatus BufferPoolClient::postSend(
        ConnectionId receiverId,
        const std::shared_ptr<BufferPoolData> &buffer,
        TransactionId *transactionId,
        int64_t *timestampUs) {
    if (isValid()) {
        bool result = mImpl->postSend(
                buffer->mId, receiverId, transactionId, timestampUs);
        return result ? ResultStatus::OK : ResultStatus::CRITICAL_ERROR;
    }
    return ResultStatus::CRITICAL_ERROR;
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android
