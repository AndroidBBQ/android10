/*
 * Copyright 2018 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "TransactionCompletedThread"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "TransactionCompletedThread.h"

#include <cinttypes>

#include <binder/IInterface.h>
#include <gui/ITransactionCompletedListener.h>
#include <utils/RefBase.h>

namespace android {

// Returns 0 if they are equal
//         <0 if the first id that doesn't match is lower in c2 or all ids match but c2 is shorter
//         >0 if the first id that doesn't match is greater in c2 or all ids match but c2 is longer
//
// See CallbackIdsHash for a explaniation of why this works
static int compareCallbackIds(const std::vector<CallbackId>& c1,
                              const std::vector<CallbackId>& c2) {
    if (c1.empty()) {
        return !c2.empty();
    }
    return c1.front() - c2.front();
}

TransactionCompletedThread::~TransactionCompletedThread() {
    std::lock_guard lockThread(mThreadMutex);

    {
        std::lock_guard lock(mMutex);
        mKeepRunning = false;
        mConditionVariable.notify_all();
    }

    if (mThread.joinable()) {
        mThread.join();
    }

    {
        std::lock_guard lock(mMutex);
        for (const auto& [listener, transactionStats] : mCompletedTransactions) {
            IInterface::asBinder(listener)->unlinkToDeath(mDeathRecipient);
        }
    }
}

void TransactionCompletedThread::run() {
    std::lock_guard lock(mMutex);
    if (mRunning || !mKeepRunning) {
        return;
    }
    mDeathRecipient = new ThreadDeathRecipient();
    mRunning = true;

    std::lock_guard lockThread(mThreadMutex);
    mThread = std::thread(&TransactionCompletedThread::threadMain, this);
}

status_t TransactionCompletedThread::addCallback(const sp<ITransactionCompletedListener>& listener,
                                                 const std::vector<CallbackId>& callbackIds) {
    std::lock_guard lock(mMutex);
    if (!mRunning) {
        ALOGE("cannot add callback because the callback thread isn't running");
        return BAD_VALUE;
    }

    if (mCompletedTransactions.count(listener) == 0) {
        status_t err = IInterface::asBinder(listener)->linkToDeath(mDeathRecipient);
        if (err != NO_ERROR) {
            ALOGE("cannot add callback because linkToDeath failed, err: %d", err);
            return err;
        }
    }

    auto& transactionStatsDeque = mCompletedTransactions[listener];
    transactionStatsDeque.emplace_back(callbackIds);
    return NO_ERROR;
}

status_t TransactionCompletedThread::registerPendingCallbackHandle(
        const sp<CallbackHandle>& handle) {
    std::lock_guard lock(mMutex);
    if (!mRunning) {
        ALOGE("cannot register callback handle because the callback thread isn't running");
        return BAD_VALUE;
    }

    // If we can't find the transaction stats something has gone wrong. The client should call
    // addCallback before trying to register a pending callback handle.
    TransactionStats* transactionStats;
    status_t err = findTransactionStats(handle->listener, handle->callbackIds, &transactionStats);
    if (err != NO_ERROR) {
        ALOGE("cannot find transaction stats");
        return err;
    }

    mPendingTransactions[handle->listener][handle->callbackIds]++;
    return NO_ERROR;
}

status_t TransactionCompletedThread::addPresentedCallbackHandles(
        const std::deque<sp<CallbackHandle>>& handles) {
    std::lock_guard lock(mMutex);
    if (!mRunning) {
        ALOGE("cannot add presented callback handle because the callback thread isn't running");
        return BAD_VALUE;
    }

    for (const auto& handle : handles) {
        auto listener = mPendingTransactions.find(handle->listener);
        if (listener != mPendingTransactions.end()) {
            auto& pendingCallbacks = listener->second;
            auto pendingCallback = pendingCallbacks.find(handle->callbackIds);

            if (pendingCallback != pendingCallbacks.end()) {
                auto& pendingCount = pendingCallback->second;

                // Decrease the pending count for this listener
                if (--pendingCount == 0) {
                    pendingCallbacks.erase(pendingCallback);
                }
            } else {
                ALOGW("there are more latched callbacks than there were registered callbacks");
            }
            if (listener->second.size() == 0) {
                mPendingTransactions.erase(listener);
            }
        } else {
            ALOGW("cannot find listener in mPendingTransactions");
        }

        status_t err = addCallbackHandle(handle);
        if (err != NO_ERROR) {
            ALOGE("could not add callback handle");
            return err;
        }
    }

    return NO_ERROR;
}

status_t TransactionCompletedThread::addUnpresentedCallbackHandle(
        const sp<CallbackHandle>& handle) {
    std::lock_guard lock(mMutex);
    if (!mRunning) {
        ALOGE("cannot add unpresented callback handle because the callback thread isn't running");
        return BAD_VALUE;
    }

    return addCallbackHandle(handle);
}

status_t TransactionCompletedThread::findTransactionStats(
        const sp<ITransactionCompletedListener>& listener,
        const std::vector<CallbackId>& callbackIds, TransactionStats** outTransactionStats) {
    auto& transactionStatsDeque = mCompletedTransactions[listener];

    // Search back to front because the most recent transactions are at the back of the deque
    auto itr = transactionStatsDeque.rbegin();
    for (; itr != transactionStatsDeque.rend(); itr++) {
        if (compareCallbackIds(itr->callbackIds, callbackIds) == 0) {
            *outTransactionStats = &(*itr);
            return NO_ERROR;
        }
    }

    ALOGE("could not find transaction stats");
    return BAD_VALUE;
}

status_t TransactionCompletedThread::addCallbackHandle(const sp<CallbackHandle>& handle) {
    // If we can't find the transaction stats something has gone wrong. The client should call
    // addCallback before trying to add a presnted callback handle.
    TransactionStats* transactionStats;
    status_t err = findTransactionStats(handle->listener, handle->callbackIds, &transactionStats);
    if (err != NO_ERROR) {
        return err;
    }

    transactionStats->latchTime = handle->latchTime;
    // If the layer has already been destroyed, don't add the SurfaceControl to the callback.
    // The client side keeps a sp<> to the SurfaceControl so if the SurfaceControl has been
    // destroyed the client side is dead and there won't be anyone to send the callback to.
    sp<IBinder> surfaceControl = handle->surfaceControl.promote();
    if (surfaceControl) {
        transactionStats->surfaceStats.emplace_back(surfaceControl, handle->acquireTime,
                                                    handle->previousReleaseFence);
    }
    return NO_ERROR;
}

void TransactionCompletedThread::addPresentFence(const sp<Fence>& presentFence) {
    std::lock_guard<std::mutex> lock(mMutex);
    mPresentFence = presentFence;
}

void TransactionCompletedThread::sendCallbacks() {
    std::lock_guard lock(mMutex);
    if (mRunning) {
        mConditionVariable.notify_all();
    }
}

void TransactionCompletedThread::threadMain() {
    std::lock_guard lock(mMutex);

    while (mKeepRunning) {
        mConditionVariable.wait(mMutex);
        std::vector<ListenerStats> completedListenerStats;

        // For each listener
        auto completedTransactionsItr = mCompletedTransactions.begin();
        while (completedTransactionsItr != mCompletedTransactions.end()) {
            auto& [listener, transactionStatsDeque] = *completedTransactionsItr;
            ListenerStats listenerStats;
            listenerStats.listener = listener;

            // For each transaction
            auto transactionStatsItr = transactionStatsDeque.begin();
            while (transactionStatsItr != transactionStatsDeque.end()) {
                auto& transactionStats = *transactionStatsItr;

                // If we are still waiting on the callback handles for this transaction, stop
                // here because all transaction callbacks for the same listener must come in order
                auto pendingTransactions = mPendingTransactions.find(listener);
                if (pendingTransactions != mPendingTransactions.end() &&
                    pendingTransactions->second.count(transactionStats.callbackIds) != 0) {
                    break;
                }

                // If the transaction has been latched
                if (transactionStats.latchTime >= 0) {
                    if (!mPresentFence) {
                        break;
                    }
                    transactionStats.presentFence = mPresentFence;
                }

                // Remove the transaction from completed to the callback
                listenerStats.transactionStats.push_back(std::move(transactionStats));
                transactionStatsItr = transactionStatsDeque.erase(transactionStatsItr);
            }
            // If the listener has completed transactions
            if (!listenerStats.transactionStats.empty()) {
                // If the listener is still alive
                if (IInterface::asBinder(listener)->isBinderAlive()) {
                    // Send callback
                    listenerStats.listener->onTransactionCompleted(listenerStats);
                    IInterface::asBinder(listener)->unlinkToDeath(mDeathRecipient);
                }
                completedTransactionsItr = mCompletedTransactions.erase(completedTransactionsItr);
            } else {
                completedTransactionsItr++;
            }

            completedListenerStats.push_back(std::move(listenerStats));
        }

        if (mPresentFence) {
            mPresentFence.clear();
        }

        // If everyone else has dropped their reference to a layer and its listener is dead,
        // we are about to cause the layer to be deleted. If this happens at the wrong time and
        // we are holding mMutex, we will cause a deadlock.
        //
        // The deadlock happens because this thread is holding on to mMutex and when we delete
        // the layer, it grabs SF's mStateLock. A different SF binder thread grabs mStateLock,
        // then call's TransactionCompletedThread::run() which tries to grab mMutex.
        //
        // To avoid this deadlock, we need to unlock mMutex when dropping our last reference to
        // to the layer.
        mMutex.unlock();
        completedListenerStats.clear();
        mMutex.lock();
    }
}

// -----------------------------------------------------------------------

CallbackHandle::CallbackHandle(const sp<ITransactionCompletedListener>& transactionListener,
                               const std::vector<CallbackId>& ids, const sp<IBinder>& sc)
      : listener(transactionListener), callbackIds(ids), surfaceControl(sc) {}

} // namespace android
