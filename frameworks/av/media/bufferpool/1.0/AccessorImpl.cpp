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

#define LOG_TAG "BufferPoolAccessor"
//#define LOG_NDEBUG 0

#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utils/Log.h>
#include "AccessorImpl.h"
#include "Connection.h"

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V1_0 {
namespace implementation {

namespace {
    static constexpr int64_t kCleanUpDurationUs = 500000; // TODO tune 0.5 sec
    static constexpr int64_t kLogDurationUs = 5000000; // 5 secs

    static constexpr size_t kMinAllocBytesForEviction = 1024*1024*15;
    static constexpr size_t kMinBufferCountForEviction = 40;
}

// Buffer structure in bufferpool process
struct InternalBuffer {
    BufferId mId;
    size_t mOwnerCount;
    size_t mTransactionCount;
    const std::shared_ptr<BufferPoolAllocation> mAllocation;
    const size_t mAllocSize;
    const std::vector<uint8_t> mConfig;

    InternalBuffer(
            BufferId id,
            const std::shared_ptr<BufferPoolAllocation> &alloc,
            const size_t allocSize,
            const std::vector<uint8_t> &allocConfig)
            : mId(id), mOwnerCount(0), mTransactionCount(0),
            mAllocation(alloc), mAllocSize(allocSize), mConfig(allocConfig) {}

    const native_handle_t *handle() {
        return mAllocation->handle();
    }
};

struct TransactionStatus {
    TransactionId mId;
    BufferId mBufferId;
    ConnectionId mSender;
    ConnectionId mReceiver;
    BufferStatus mStatus;
    int64_t mTimestampUs;
    bool mSenderValidated;

    TransactionStatus(const BufferStatusMessage &message, int64_t timestampUs) {
        mId = message.transactionId;
        mBufferId = message.bufferId;
        mStatus = message.newStatus;
        mTimestampUs = timestampUs;
        if (mStatus == BufferStatus::TRANSFER_TO) {
            mSender = message.connectionId;
            mReceiver = message.targetConnectionId;
            mSenderValidated = true;
        } else {
            mSender = -1LL;
            mReceiver = message.connectionId;
            mSenderValidated = false;
        }
    }
};

// Helper template methods for handling map of set.
template<class T, class U>
bool insert(std::map<T, std::set<U>> *mapOfSet, T key, U value) {
    auto iter = mapOfSet->find(key);
    if (iter == mapOfSet->end()) {
        std::set<U> valueSet{value};
        mapOfSet->insert(std::make_pair(key, valueSet));
        return true;
    } else if (iter->second.find(value)  == iter->second.end()) {
        iter->second.insert(value);
        return true;
    }
    return false;
}

template<class T, class U>
bool erase(std::map<T, std::set<U>> *mapOfSet, T key, U value) {
    bool ret = false;
    auto iter = mapOfSet->find(key);
    if (iter != mapOfSet->end()) {
        if (iter->second.erase(value) > 0) {
            ret = true;
        }
        if (iter->second.size() == 0) {
            mapOfSet->erase(iter);
        }
    }
    return ret;
}

template<class T, class U>
bool contains(std::map<T, std::set<U>> *mapOfSet, T key, U value) {
    auto iter = mapOfSet->find(key);
    if (iter != mapOfSet->end()) {
        auto setIter = iter->second.find(value);
        return setIter != iter->second.end();
    }
    return false;
}

int32_t Accessor::Impl::sPid = getpid();
uint32_t Accessor::Impl::sSeqId = time(nullptr);

Accessor::Impl::Impl(
        const std::shared_ptr<BufferPoolAllocator> &allocator)
        : mAllocator(allocator) {}

Accessor::Impl::~Impl() {
}

ResultStatus Accessor::Impl::connect(
        const sp<Accessor> &accessor, sp<Connection> *connection,
        ConnectionId *pConnectionId, const QueueDescriptor** fmqDescPtr) {
    sp<Connection> newConnection = new Connection();
    ResultStatus status = ResultStatus::CRITICAL_ERROR;
    {
        std::lock_guard<std::mutex> lock(mBufferPool.mMutex);
        if (newConnection) {
            ConnectionId id = (int64_t)sPid << 32 | sSeqId;
            status = mBufferPool.mObserver.open(id, fmqDescPtr);
            if (status == ResultStatus::OK) {
                newConnection->initialize(accessor, id);
                *connection = newConnection;
                *pConnectionId = id;
                mBufferPool.mConnectionIds.insert(id);
                ++sSeqId;
            }
        }
        mBufferPool.processStatusMessages();
        mBufferPool.cleanUp();
    }
    return status;
}

ResultStatus Accessor::Impl::close(ConnectionId connectionId) {
    std::lock_guard<std::mutex> lock(mBufferPool.mMutex);
    mBufferPool.processStatusMessages();
    mBufferPool.handleClose(connectionId);
    mBufferPool.mObserver.close(connectionId);
    // Since close# will be called after all works are finished, it is OK to
    // evict unused buffers.
    mBufferPool.cleanUp(true);
    return ResultStatus::OK;
}

ResultStatus Accessor::Impl::allocate(
        ConnectionId connectionId, const std::vector<uint8_t>& params,
        BufferId *bufferId, const native_handle_t** handle) {
    std::unique_lock<std::mutex> lock(mBufferPool.mMutex);
    mBufferPool.processStatusMessages();
    ResultStatus status = ResultStatus::OK;
    if (!mBufferPool.getFreeBuffer(mAllocator, params, bufferId, handle)) {
        lock.unlock();
        std::shared_ptr<BufferPoolAllocation> alloc;
        size_t allocSize;
        status = mAllocator->allocate(params, &alloc, &allocSize);
        lock.lock();
        if (status == ResultStatus::OK) {
            status = mBufferPool.addNewBuffer(alloc, allocSize, params, bufferId, handle);
        }
        ALOGV("create a buffer %d : %u %p",
              status == ResultStatus::OK, *bufferId, *handle);
    }
    if (status == ResultStatus::OK) {
        // TODO: handle ownBuffer failure
        mBufferPool.handleOwnBuffer(connectionId, *bufferId);
    }
    mBufferPool.cleanUp();
    return status;
}

ResultStatus Accessor::Impl::fetch(
        ConnectionId connectionId, TransactionId transactionId,
        BufferId bufferId, const native_handle_t** handle) {
    std::lock_guard<std::mutex> lock(mBufferPool.mMutex);
    mBufferPool.processStatusMessages();
    auto found = mBufferPool.mTransactions.find(transactionId);
    if (found != mBufferPool.mTransactions.end() &&
            contains(&mBufferPool.mPendingTransactions,
                     connectionId, transactionId)) {
        if (found->second->mSenderValidated &&
                found->second->mStatus == BufferStatus::TRANSFER_FROM &&
                found->second->mBufferId == bufferId) {
            found->second->mStatus = BufferStatus::TRANSFER_FETCH;
            auto bufferIt = mBufferPool.mBuffers.find(bufferId);
            if (bufferIt != mBufferPool.mBuffers.end()) {
                mBufferPool.mStats.onBufferFetched();
                *handle = bufferIt->second->handle();
                return ResultStatus::OK;
            }
        }
    }
    mBufferPool.cleanUp();
    return ResultStatus::CRITICAL_ERROR;
}

void Accessor::Impl::cleanUp(bool clearCache) {
    // transaction timeout, buffer cacheing TTL handling
    std::lock_guard<std::mutex> lock(mBufferPool.mMutex);
    mBufferPool.processStatusMessages();
    mBufferPool.cleanUp(clearCache);
}

Accessor::Impl::Impl::BufferPool::BufferPool()
    : mTimestampUs(getTimestampNow()),
      mLastCleanUpUs(mTimestampUs),
      mLastLogUs(mTimestampUs),
      mSeq(0) {}


// Statistics helper
template<typename T, typename S>
int percentage(T base, S total) {
    return int(total ? 0.5 + 100. * static_cast<S>(base) / total : 0);
}

Accessor::Impl::Impl::BufferPool::~BufferPool() {
    std::lock_guard<std::mutex> lock(mMutex);
    ALOGD("Destruction - bufferpool %p "
          "cached: %zu/%zuM, %zu/%d%% in use; "
          "allocs: %zu, %d%% recycled; "
          "transfers: %zu, %d%% unfetced",
          this, mStats.mBuffersCached, mStats.mSizeCached >> 20,
          mStats.mBuffersInUse, percentage(mStats.mBuffersInUse, mStats.mBuffersCached),
          mStats.mTotalAllocations, percentage(mStats.mTotalRecycles, mStats.mTotalAllocations),
          mStats.mTotalTransfers,
          percentage(mStats.mTotalTransfers - mStats.mTotalFetches, mStats.mTotalTransfers));
}

bool Accessor::Impl::BufferPool::handleOwnBuffer(
        ConnectionId connectionId, BufferId bufferId) {

    bool added = insert(&mUsingBuffers, connectionId, bufferId);
    if (added) {
        auto iter = mBuffers.find(bufferId);
        iter->second->mOwnerCount++;
    }
    insert(&mUsingConnections, bufferId, connectionId);
    return added;
}

bool Accessor::Impl::BufferPool::handleReleaseBuffer(
        ConnectionId connectionId, BufferId bufferId) {
    bool deleted = erase(&mUsingBuffers, connectionId, bufferId);
    if (deleted) {
        auto iter = mBuffers.find(bufferId);
        iter->second->mOwnerCount--;
        if (iter->second->mOwnerCount == 0 &&
                iter->second->mTransactionCount == 0) {
            mStats.onBufferUnused(iter->second->mAllocSize);
            mFreeBuffers.insert(bufferId);
        }
    }
    erase(&mUsingConnections, bufferId, connectionId);
    ALOGV("release buffer %u : %d", bufferId, deleted);
    return deleted;
}

bool Accessor::Impl::BufferPool::handleTransferTo(const BufferStatusMessage &message) {
    auto completed = mCompletedTransactions.find(
            message.transactionId);
    if (completed != mCompletedTransactions.end()) {
        // already completed
        mCompletedTransactions.erase(completed);
        return true;
    }
    // the buffer should exist and be owned.
    auto bufferIter = mBuffers.find(message.bufferId);
    if (bufferIter == mBuffers.end() ||
            !contains(&mUsingBuffers, message.connectionId, message.bufferId)) {
        return false;
    }
    auto found = mTransactions.find(message.transactionId);
    if (found != mTransactions.end()) {
        // transfer_from was received earlier.
        found->second->mSender = message.connectionId;
        found->second->mSenderValidated = true;
        return true;
    }
    if (mConnectionIds.find(message.targetConnectionId) == mConnectionIds.end()) {
        // N.B: it could be fake or receive connection already closed.
        ALOGD("bufferpool %p receiver connection %lld is no longer valid",
              this, (long long)message.targetConnectionId);
        return false;
    }
    mStats.onBufferSent();
    mTransactions.insert(std::make_pair(
            message.transactionId,
            std::make_unique<TransactionStatus>(message, mTimestampUs)));
    insert(&mPendingTransactions, message.targetConnectionId,
           message.transactionId);
    bufferIter->second->mTransactionCount++;
    return true;
}

bool Accessor::Impl::BufferPool::handleTransferFrom(const BufferStatusMessage &message) {
    auto found = mTransactions.find(message.transactionId);
    if (found == mTransactions.end()) {
        // TODO: is it feasible to check ownership here?
        mStats.onBufferSent();
        mTransactions.insert(std::make_pair(
                message.transactionId,
                std::make_unique<TransactionStatus>(message, mTimestampUs)));
        insert(&mPendingTransactions, message.connectionId,
               message.transactionId);
        auto bufferIter = mBuffers.find(message.bufferId);
        bufferIter->second->mTransactionCount++;
    } else {
        if (message.connectionId == found->second->mReceiver) {
            found->second->mStatus = BufferStatus::TRANSFER_FROM;
        }
    }
    return true;
}

bool Accessor::Impl::BufferPool::handleTransferResult(const BufferStatusMessage &message) {
    auto found = mTransactions.find(message.transactionId);
    if (found != mTransactions.end()) {
        bool deleted = erase(&mPendingTransactions, message.connectionId,
                             message.transactionId);
        if (deleted) {
            if (!found->second->mSenderValidated) {
                mCompletedTransactions.insert(message.transactionId);
            }
            auto bufferIter = mBuffers.find(message.bufferId);
            if (message.newStatus == BufferStatus::TRANSFER_OK) {
                handleOwnBuffer(message.connectionId, message.bufferId);
            }
            bufferIter->second->mTransactionCount--;
            if (bufferIter->second->mOwnerCount == 0
                && bufferIter->second->mTransactionCount == 0) {
                mStats.onBufferUnused(bufferIter->second->mAllocSize);
                mFreeBuffers.insert(message.bufferId);
            }
            mTransactions.erase(found);
        }
        ALOGV("transfer finished %llu %u - %d", (unsigned long long)message.transactionId,
              message.bufferId, deleted);
        return deleted;
    }
    ALOGV("transfer not found %llu %u", (unsigned long long)message.transactionId,
          message.bufferId);
    return false;
}

void Accessor::Impl::BufferPool::processStatusMessages() {
    std::vector<BufferStatusMessage> messages;
    mObserver.getBufferStatusChanges(messages);
    mTimestampUs = getTimestampNow();
    for (BufferStatusMessage& message: messages) {
        bool ret = false;
        switch (message.newStatus) {
            case BufferStatus::NOT_USED:
                ret = handleReleaseBuffer(
                        message.connectionId, message.bufferId);
                break;
            case BufferStatus::USED:
                // not happening
                break;
            case BufferStatus::TRANSFER_TO:
                ret = handleTransferTo(message);
                break;
            case BufferStatus::TRANSFER_FROM:
                ret = handleTransferFrom(message);
                break;
            case BufferStatus::TRANSFER_TIMEOUT:
                // TODO
                break;
            case BufferStatus::TRANSFER_LOST:
                // TODO
                break;
            case BufferStatus::TRANSFER_FETCH:
                // not happening
                break;
            case BufferStatus::TRANSFER_OK:
            case BufferStatus::TRANSFER_ERROR:
                ret = handleTransferResult(message);
                break;
        }
        if (ret == false) {
            ALOGW("buffer status message processing failure - message : %d connection : %lld",
                  message.newStatus, (long long)message.connectionId);
        }
    }
    messages.clear();
}

bool Accessor::Impl::BufferPool::handleClose(ConnectionId connectionId) {
    // Cleaning buffers
    auto buffers = mUsingBuffers.find(connectionId);
    if (buffers != mUsingBuffers.end()) {
        for (const BufferId& bufferId : buffers->second) {
            bool deleted = erase(&mUsingConnections, bufferId, connectionId);
            if (deleted) {
                auto bufferIter = mBuffers.find(bufferId);
                bufferIter->second->mOwnerCount--;
                if (bufferIter->second->mOwnerCount == 0 &&
                        bufferIter->second->mTransactionCount == 0) {
                    // TODO: handle freebuffer insert fail
                    mStats.onBufferUnused(bufferIter->second->mAllocSize);
                    mFreeBuffers.insert(bufferId);
                }
            }
        }
        mUsingBuffers.erase(buffers);
    }

    // Cleaning transactions
    auto pending = mPendingTransactions.find(connectionId);
    if (pending != mPendingTransactions.end()) {
        for (const TransactionId& transactionId : pending->second) {
            auto iter = mTransactions.find(transactionId);
            if (iter != mTransactions.end()) {
                if (!iter->second->mSenderValidated) {
                    mCompletedTransactions.insert(transactionId);
                }
                BufferId bufferId = iter->second->mBufferId;
                auto bufferIter = mBuffers.find(bufferId);
                bufferIter->second->mTransactionCount--;
                if (bufferIter->second->mOwnerCount == 0 &&
                    bufferIter->second->mTransactionCount == 0) {
                    // TODO: handle freebuffer insert fail
                    mStats.onBufferUnused(bufferIter->second->mAllocSize);
                    mFreeBuffers.insert(bufferId);
                }
                mTransactions.erase(iter);
            }
        }
    }
    mConnectionIds.erase(connectionId);
    return true;
}

bool Accessor::Impl::BufferPool::getFreeBuffer(
        const std::shared_ptr<BufferPoolAllocator> &allocator,
        const std::vector<uint8_t> &params, BufferId *pId,
        const native_handle_t** handle) {
    auto bufferIt = mFreeBuffers.begin();
    for (;bufferIt != mFreeBuffers.end(); ++bufferIt) {
        BufferId bufferId = *bufferIt;
        if (allocator->compatible(params, mBuffers[bufferId]->mConfig)) {
            break;
        }
    }
    if (bufferIt != mFreeBuffers.end()) {
        BufferId id = *bufferIt;
        mFreeBuffers.erase(bufferIt);
        mStats.onBufferRecycled(mBuffers[id]->mAllocSize);
        *handle = mBuffers[id]->handle();
        *pId = id;
        ALOGV("recycle a buffer %u %p", id, *handle);
        return true;
    }
    return false;
}

ResultStatus Accessor::Impl::BufferPool::addNewBuffer(
        const std::shared_ptr<BufferPoolAllocation> &alloc,
        const size_t allocSize,
        const std::vector<uint8_t> &params,
        BufferId *pId,
        const native_handle_t** handle) {

    BufferId bufferId = mSeq++;
    if (mSeq == Connection::SYNC_BUFFERID) {
        mSeq = 0;
    }
    std::unique_ptr<InternalBuffer> buffer =
            std::make_unique<InternalBuffer>(
                    bufferId, alloc, allocSize, params);
    if (buffer) {
        auto res = mBuffers.insert(std::make_pair(
                bufferId, std::move(buffer)));
        if (res.second) {
            mStats.onBufferAllocated(allocSize);
            *handle = alloc->handle();
            *pId = bufferId;
            return ResultStatus::OK;
        }
    }
    return ResultStatus::NO_MEMORY;
}

void Accessor::Impl::BufferPool::cleanUp(bool clearCache) {
    if (clearCache || mTimestampUs > mLastCleanUpUs + kCleanUpDurationUs) {
        mLastCleanUpUs = mTimestampUs;
        if (mTimestampUs > mLastLogUs + kLogDurationUs) {
            mLastLogUs = mTimestampUs;
            ALOGD("bufferpool %p : %zu(%zu size) total buffers - "
                  "%zu(%zu size) used buffers - %zu/%zu (recycle/alloc) - "
                  "%zu/%zu (fetch/transfer)",
                  this, mStats.mBuffersCached, mStats.mSizeCached,
                  mStats.mBuffersInUse, mStats.mSizeInUse,
                  mStats.mTotalRecycles, mStats.mTotalAllocations,
                  mStats.mTotalFetches, mStats.mTotalTransfers);
        }
        for (auto freeIt = mFreeBuffers.begin(); freeIt != mFreeBuffers.end();) {
            if (!clearCache && mStats.mSizeCached < kMinAllocBytesForEviction
                    && mBuffers.size() < kMinBufferCountForEviction) {
                break;
            }
            auto it = mBuffers.find(*freeIt);
            if (it != mBuffers.end() &&
                    it->second->mOwnerCount == 0 && it->second->mTransactionCount == 0) {
                mStats.onBufferEvicted(it->second->mAllocSize);
                mBuffers.erase(it);
                freeIt = mFreeBuffers.erase(freeIt);
            } else {
                ++freeIt;
                ALOGW("bufferpool inconsistent!");
            }
        }
    }
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android
