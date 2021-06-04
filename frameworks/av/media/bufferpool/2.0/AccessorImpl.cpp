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
#include <thread>
#include "AccessorImpl.h"
#include "Connection.h"

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V2_0 {
namespace implementation {

namespace {
    static constexpr int64_t kCleanUpDurationUs = 500000; // TODO tune 0.5 sec
    static constexpr int64_t kLogDurationUs = 5000000; // 5 secs

    static constexpr size_t kMinAllocBytesForEviction = 1024*1024*15;
    static constexpr size_t kMinBufferCountForEviction = 25;
}

// Buffer structure in bufferpool process
struct InternalBuffer {
    BufferId mId;
    size_t mOwnerCount;
    size_t mTransactionCount;
    const std::shared_ptr<BufferPoolAllocation> mAllocation;
    const size_t mAllocSize;
    const std::vector<uint8_t> mConfig;
    bool mInvalidated;

    InternalBuffer(
            BufferId id,
            const std::shared_ptr<BufferPoolAllocation> &alloc,
            const size_t allocSize,
            const std::vector<uint8_t> &allocConfig)
            : mId(id), mOwnerCount(0), mTransactionCount(0),
            mAllocation(alloc), mAllocSize(allocSize), mConfig(allocConfig),
            mInvalidated(false) {}

    const native_handle_t *handle() {
        return mAllocation->handle();
    }

    void invalidate() {
        mInvalidated = true;
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
        const sp<Accessor> &accessor, const sp<IObserver> &observer,
        sp<Connection> *connection,
        ConnectionId *pConnectionId,
        uint32_t *pMsgId,
        const StatusDescriptor** statusDescPtr,
        const InvalidationDescriptor** invDescPtr) {
    sp<Connection> newConnection = new Connection();
    ResultStatus status = ResultStatus::CRITICAL_ERROR;
    {
        std::lock_guard<std::mutex> lock(mBufferPool.mMutex);
        if (newConnection) {
            ConnectionId id = (int64_t)sPid << 32 | sSeqId;
            status = mBufferPool.mObserver.open(id, statusDescPtr);
            if (status == ResultStatus::OK) {
                newConnection->initialize(accessor, id);
                *connection = newConnection;
                *pConnectionId = id;
                *pMsgId = mBufferPool.mInvalidation.mInvalidationId;
                mBufferPool.mConnectionIds.insert(id);
                mBufferPool.mInvalidationChannel.getDesc(invDescPtr);
                mBufferPool.mInvalidation.onConnect(id, observer);
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
    ALOGV("connection close %lld: %u", (long long)connectionId, mBufferPool.mInvalidation.mId);
    mBufferPool.processStatusMessages();
    mBufferPool.handleClose(connectionId);
    mBufferPool.mObserver.close(connectionId);
    mBufferPool.mInvalidation.onClose(connectionId);
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

void Accessor::Impl::flush() {
    std::lock_guard<std::mutex> lock(mBufferPool.mMutex);
    mBufferPool.processStatusMessages();
    mBufferPool.flush(shared_from_this());
}

void Accessor::Impl::handleInvalidateAck() {
    std::map<ConnectionId, const sp<IObserver>> observers;
    uint32_t invalidationId;
    {
        std::lock_guard<std::mutex> lock(mBufferPool.mMutex);
        mBufferPool.processStatusMessages();
        mBufferPool.mInvalidation.onHandleAck(&observers, &invalidationId);
    }
    // Do not hold lock for send invalidations
    size_t deadClients = 0;
    for (auto it = observers.begin(); it != observers.end(); ++it) {
        const sp<IObserver> observer = it->second;
        if (observer) {
            Return<void> transResult = observer->onMessage(it->first, invalidationId);
            if (!transResult.isOk()) {
                ++deadClients;
            }
        }
    }
    if (deadClients > 0) {
        ALOGD("During invalidation found %zu dead clients", deadClients);
    }
}

bool Accessor::Impl::isValid() {
    return mBufferPool.isValid();
}

Accessor::Impl::Impl::BufferPool::BufferPool()
    : mTimestampUs(getTimestampNow()),
      mLastCleanUpUs(mTimestampUs),
      mLastLogUs(mTimestampUs),
      mSeq(0),
      mStartSeq(0) {
    mValid = mInvalidationChannel.isValid();
}


// Statistics helper
template<typename T, typename S>
int percentage(T base, S total) {
    return int(total ? 0.5 + 100. * static_cast<S>(base) / total : 0);
}

std::atomic<std::uint32_t> Accessor::Impl::BufferPool::Invalidation::sInvSeqId(0);

Accessor::Impl::Impl::BufferPool::~BufferPool() {
    std::lock_guard<std::mutex> lock(mMutex);
    ALOGD("Destruction - bufferpool2 %p "
          "cached: %zu/%zuM, %zu/%d%% in use; "
          "allocs: %zu, %d%% recycled; "
          "transfers: %zu, %d%% unfetced",
          this, mStats.mBuffersCached, mStats.mSizeCached >> 20,
          mStats.mBuffersInUse, percentage(mStats.mBuffersInUse, mStats.mBuffersCached),
          mStats.mTotalAllocations, percentage(mStats.mTotalRecycles, mStats.mTotalAllocations),
          mStats.mTotalTransfers,
          percentage(mStats.mTotalTransfers - mStats.mTotalFetches, mStats.mTotalTransfers));
}

void Accessor::Impl::BufferPool::Invalidation::onConnect(
        ConnectionId conId, const sp<IObserver>& observer) {
    mAcks[conId] = mInvalidationId; // starts from current invalidationId
    mObservers.insert(std::make_pair(conId, observer));
}

void Accessor::Impl::BufferPool::Invalidation::onClose(ConnectionId conId) {
    mAcks.erase(conId);
    mObservers.erase(conId);
}

void Accessor::Impl::BufferPool::Invalidation::onAck(
        ConnectionId conId,
        uint32_t msgId) {
    auto it = mAcks.find(conId);
    if (it == mAcks.end()) {
        ALOGW("ACK from inconsistent connection! %lld", (long long)conId);
        return;
    }
    if (isMessageLater(msgId, it->second)) {
        mAcks[conId] = msgId;
    }
}

void Accessor::Impl::BufferPool::Invalidation::onBufferInvalidated(
        BufferId bufferId,
        BufferInvalidationChannel &channel) {
    for (auto it = mPendings.begin(); it != mPendings.end();) {
        if (it->isInvalidated(bufferId)) {
            uint32_t msgId = 0;
            if (it->mNeedsAck) {
                msgId = ++mInvalidationId;
                if (msgId == 0) {
                    // wrap happens
                    msgId = ++mInvalidationId;
                }
            }
            channel.postInvalidation(msgId, it->mFrom, it->mTo);
            it = mPendings.erase(it);
            continue;
        }
        ++it;
    }
}

void Accessor::Impl::BufferPool::Invalidation::onInvalidationRequest(
        bool needsAck,
        uint32_t from,
        uint32_t to,
        size_t left,
        BufferInvalidationChannel &channel,
        const std::shared_ptr<Accessor::Impl> &impl) {
        uint32_t msgId = 0;
    if (needsAck) {
        msgId = ++mInvalidationId;
        if (msgId == 0) {
            // wrap happens
            msgId = ++mInvalidationId;
        }
    }
    ALOGV("bufferpool2 invalidation requested and queued");
    if (left == 0) {
        channel.postInvalidation(msgId, from, to);
    } else {
        // TODO: sending hint message?
        ALOGV("bufferpoo2 invalidation requested and pending");
        Pending pending(needsAck, from, to, left, impl);
        mPendings.push_back(pending);
    }
    sInvalidator->addAccessor(mId, impl);
}

void Accessor::Impl::BufferPool::Invalidation::onHandleAck(
        std::map<ConnectionId, const sp<IObserver>> *observers,
        uint32_t *invalidationId) {
    if (mInvalidationId != 0) {
        *invalidationId = mInvalidationId;
        std::set<int> deads;
        for (auto it = mAcks.begin(); it != mAcks.end(); ++it) {
            if (it->second != mInvalidationId) {
                const sp<IObserver> observer = mObservers[it->first];
                if (observer) {
                    observers->emplace(it->first, observer);
                    ALOGV("connection %lld will call observer (%u: %u)",
                          (long long)it->first, it->second, mInvalidationId);
                    // N.B: onMessage will be called later. ignore possibility of
                    // onMessage# oneway call being lost.
                    it->second = mInvalidationId;
                } else {
                    ALOGV("bufferpool2 observer died %lld", (long long)it->first);
                    deads.insert(it->first);
                }
            }
        }
        if (deads.size() > 0) {
            for (auto it = deads.begin(); it != deads.end(); ++it) {
                onClose(*it);
            }
        }
    }
    if (mPendings.size() == 0) {
        // All invalidation Ids are synced and no more pending invalidations.
        sInvalidator->delAccessor(mId);
    }
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
            if (!iter->second->mInvalidated) {
                mStats.onBufferUnused(iter->second->mAllocSize);
                mFreeBuffers.insert(bufferId);
            } else {
                mStats.onBufferUnused(iter->second->mAllocSize);
                mStats.onBufferEvicted(iter->second->mAllocSize);
                mBuffers.erase(iter);
                mInvalidation.onBufferInvalidated(bufferId, mInvalidationChannel);
            }
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
        ALOGD("bufferpool2 %p receiver connection %lld is no longer valid",
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
                if (!bufferIter->second->mInvalidated) {
                    mStats.onBufferUnused(bufferIter->second->mAllocSize);
                    mFreeBuffers.insert(message.bufferId);
                } else {
                    mStats.onBufferUnused(bufferIter->second->mAllocSize);
                    mStats.onBufferEvicted(bufferIter->second->mAllocSize);
                    mBuffers.erase(bufferIter);
                    mInvalidation.onBufferInvalidated(message.bufferId, mInvalidationChannel);
                }
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
            case BufferStatus::INVALIDATION_ACK:
                mInvalidation.onAck(message.connectionId, message.bufferId);
                ret = true;
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
                    if (!bufferIter->second->mInvalidated) {
                        mStats.onBufferUnused(bufferIter->second->mAllocSize);
                        mFreeBuffers.insert(bufferId);
                    } else {
                        mStats.onBufferUnused(bufferIter->second->mAllocSize);
                        mStats.onBufferEvicted(bufferIter->second->mAllocSize);
                        mBuffers.erase(bufferIter);
                        mInvalidation.onBufferInvalidated(bufferId, mInvalidationChannel);
                    }
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
                    if (!bufferIter->second->mInvalidated) {
                        mStats.onBufferUnused(bufferIter->second->mAllocSize);
                        mFreeBuffers.insert(bufferId);
                    } else {
                        mStats.onBufferUnused(bufferIter->second->mAllocSize);
                        mStats.onBufferEvicted(bufferIter->second->mAllocSize);
                        mBuffers.erase(bufferIter);
                        mInvalidation.onBufferInvalidated(bufferId, mInvalidationChannel);
                    }
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
            ALOGD("bufferpool2 %p : %zu(%zu size) total buffers - "
                  "%zu(%zu size) used buffers - %zu/%zu (recycle/alloc) - "
                  "%zu/%zu (fetch/transfer)",
                  this, mStats.mBuffersCached, mStats.mSizeCached,
                  mStats.mBuffersInUse, mStats.mSizeInUse,
                  mStats.mTotalRecycles, mStats.mTotalAllocations,
                  mStats.mTotalFetches, mStats.mTotalTransfers);
        }
        for (auto freeIt = mFreeBuffers.begin(); freeIt != mFreeBuffers.end();) {
            if (!clearCache && (mStats.mSizeCached < kMinAllocBytesForEviction
                    || mBuffers.size() < kMinBufferCountForEviction)) {
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
                ALOGW("bufferpool2 inconsistent!");
            }
        }
    }
}

void Accessor::Impl::BufferPool::invalidate(
        bool needsAck, BufferId from, BufferId to,
        const std::shared_ptr<Accessor::Impl> &impl) {
    for (auto freeIt = mFreeBuffers.begin(); freeIt != mFreeBuffers.end();) {
        if (isBufferInRange(from, to, *freeIt)) {
            auto it = mBuffers.find(*freeIt);
            if (it != mBuffers.end() &&
                it->second->mOwnerCount == 0 && it->second->mTransactionCount == 0) {
                mStats.onBufferEvicted(it->second->mAllocSize);
                mBuffers.erase(it);
                freeIt = mFreeBuffers.erase(freeIt);
                continue;
            } else {
                ALOGW("bufferpool2 inconsistent!");
            }
        }
        ++freeIt;
    }

    size_t left = 0;
    for (auto it = mBuffers.begin(); it != mBuffers.end(); ++it) {
        if (isBufferInRange(from, to, it->first)) {
            it->second->invalidate();
            ++left;
        }
    }
    mInvalidation.onInvalidationRequest(needsAck, from, to, left, mInvalidationChannel, impl);
}

void Accessor::Impl::BufferPool::flush(const std::shared_ptr<Accessor::Impl> &impl) {
    BufferId from = mStartSeq;
    BufferId to = mSeq;
    mStartSeq = mSeq;
    // TODO: needsAck params 
    ALOGV("buffer invalidation request bp:%u %u %u", mInvalidation.mId, from, to);
    if (from != to) {
        invalidate(true, from, to, impl);
    }
}

void Accessor::Impl::invalidatorThread(
            std::map<uint32_t, const std::weak_ptr<Accessor::Impl>> &accessors,
            std::mutex &mutex,
            std::condition_variable &cv,
            bool &ready) {
    constexpr uint32_t NUM_SPIN_TO_INCREASE_SLEEP = 1024;
    constexpr uint32_t NUM_SPIN_TO_LOG = 1024*8;
    constexpr useconds_t MAX_SLEEP_US = 10000;
    uint32_t numSpin = 0;
    useconds_t sleepUs = 1;

    while(true) {
        std::map<uint32_t, const std::weak_ptr<Accessor::Impl>> copied;
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!ready) {
                numSpin = 0;
                sleepUs = 1;
                cv.wait(lock);
            }
            copied.insert(accessors.begin(), accessors.end());
        }
        std::list<ConnectionId> erased;
        for (auto it = copied.begin(); it != copied.end(); ++it) {
            const std::shared_ptr<Accessor::Impl> impl = it->second.lock();
            if (!impl) {
                erased.push_back(it->first);
            } else {
                impl->handleInvalidateAck();
            }
        }
        {
            std::unique_lock<std::mutex> lock(mutex);
            for (auto it = erased.begin(); it != erased.end(); ++it) {
                accessors.erase(*it);
            }
            if (accessors.size() == 0) {
                ready = false;
            } else {
                // TODO Use an efficient way to wait over FMQ.
                // N.B. Since there is not a efficient way to wait over FMQ,
                // polling over the FMQ is the current way to prevent draining
                // CPU.
                lock.unlock();
                ++numSpin;
                if (numSpin % NUM_SPIN_TO_INCREASE_SLEEP == 0 &&
                    sleepUs < MAX_SLEEP_US) {
                    sleepUs *= 10;
                }
                if (numSpin % NUM_SPIN_TO_LOG == 0) {
                    ALOGW("invalidator thread spinning");
                }
                ::usleep(sleepUs);
            }
        }
    }
}

Accessor::Impl::AccessorInvalidator::AccessorInvalidator() : mReady(false) {
    std::thread invalidator(
            invalidatorThread,
            std::ref(mAccessors),
            std::ref(mMutex),
            std::ref(mCv),
            std::ref(mReady));
    invalidator.detach();
}

void Accessor::Impl::AccessorInvalidator::addAccessor(
        uint32_t accessorId, const std::weak_ptr<Accessor::Impl> &impl) {
    bool notify = false;
    std::unique_lock<std::mutex> lock(mMutex);
    if (mAccessors.find(accessorId) == mAccessors.end()) {
        if (!mReady) {
            mReady = true;
            notify = true;
        }
        mAccessors.insert(std::make_pair(accessorId, impl));
        ALOGV("buffer invalidation added bp:%u %d", accessorId, notify);
    }
    lock.unlock();
    if (notify) {
        mCv.notify_one();
    }
}

void Accessor::Impl::AccessorInvalidator::delAccessor(uint32_t accessorId) {
    std::lock_guard<std::mutex> lock(mMutex);
    mAccessors.erase(accessorId);
    ALOGV("buffer invalidation deleted bp:%u", accessorId);
    if (mAccessors.size() == 0) {
        mReady = false;
    }
}

std::unique_ptr<Accessor::Impl::AccessorInvalidator> Accessor::Impl::sInvalidator;

void Accessor::Impl::createInvalidator() {
    if (!sInvalidator) {
        sInvalidator = std::make_unique<Accessor::Impl::AccessorInvalidator>();
    }
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android
