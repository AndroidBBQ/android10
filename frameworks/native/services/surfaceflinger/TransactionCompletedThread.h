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

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <android-base/thread_annotations.h>

#include <binder/IBinder.h>
#include <gui/ITransactionCompletedListener.h>
#include <ui/Fence.h>

namespace android {

struct CallbackIdsHash {
    // CallbackId vectors have several properties that let us get away with this simple hash.
    // 1) CallbackIds are never 0 so if something has gone wrong and our CallbackId vector is
    // empty we can still hash 0.
    // 2) CallbackId vectors for the same listener either are identical or contain none of the
    // same members. It is sufficient to just check the first CallbackId in the vectors. If
    // they match, they are the same. If they do not match, they are not the same.
    std::size_t operator()(const std::vector<CallbackId>& callbackIds) const {
        return std::hash<CallbackId>{}((callbackIds.empty()) ? 0 : callbackIds.front());
    }
};

class CallbackHandle : public RefBase {
public:
    CallbackHandle(const sp<ITransactionCompletedListener>& transactionListener,
                   const std::vector<CallbackId>& ids, const sp<IBinder>& sc);

    sp<ITransactionCompletedListener> listener;
    std::vector<CallbackId> callbackIds;
    wp<IBinder> surfaceControl;

    bool releasePreviousBuffer = false;
    sp<Fence> previousReleaseFence;
    nsecs_t acquireTime = -1;
    nsecs_t latchTime = -1;
};

class TransactionCompletedThread {
public:
    ~TransactionCompletedThread();

    void run();

    // Adds listener and callbackIds in case there are no SurfaceControls that are supposed
    // to be included in the callback. This functions should be call before attempting to add any
    // callback handles.
    status_t addCallback(const sp<ITransactionCompletedListener>& transactionListener,
                         const std::vector<CallbackId>& callbackIds);

    // Informs the TransactionCompletedThread that there is a Transaction with a CallbackHandle
    // that needs to be latched and presented this frame. This function should be called once the
    // layer has received the CallbackHandle so the TransactionCompletedThread knows not to send
    // a callback for that Listener/Transaction pair until that CallbackHandle has been latched and
    // presented.
    status_t registerPendingCallbackHandle(const sp<CallbackHandle>& handle);
    // Notifies the TransactionCompletedThread that a pending CallbackHandle has been presented.
    status_t addPresentedCallbackHandles(const std::deque<sp<CallbackHandle>>& handles);

    // Adds the Transaction CallbackHandle from a layer that does not need to be relatched and
    // presented this frame.
    status_t addUnpresentedCallbackHandle(const sp<CallbackHandle>& handle);

    void addPresentFence(const sp<Fence>& presentFence);

    void sendCallbacks();

private:
    void threadMain();

    status_t findTransactionStats(const sp<ITransactionCompletedListener>& listener,
                                  const std::vector<CallbackId>& callbackIds,
                                  TransactionStats** outTransactionStats) REQUIRES(mMutex);

    status_t addCallbackHandle(const sp<CallbackHandle>& handle) REQUIRES(mMutex);

    class ThreadDeathRecipient : public IBinder::DeathRecipient {
    public:
        // This function is a no-op. isBinderAlive needs a linked DeathRecipient to work.
        // Death recipients needs a binderDied function.
        //
        // (isBinderAlive checks if BpBinder's mAlive is 0. mAlive is only set to 0 in sendObituary.
        // sendObituary is only called if linkToDeath was called with a DeathRecipient.)
        void binderDied(const wp<IBinder>& /*who*/) override {}
    };
    sp<ThreadDeathRecipient> mDeathRecipient;

    struct ITransactionCompletedListenerHash {
        std::size_t operator()(const sp<ITransactionCompletedListener>& listener) const {
            return std::hash<IBinder*>{}((listener) ? IInterface::asBinder(listener).get()
                                                    : nullptr);
        }
    };

    // Protects the creation and destruction of mThread
    std::mutex mThreadMutex;

    std::thread mThread GUARDED_BY(mThreadMutex);

    std::mutex mMutex;
    std::condition_variable_any mConditionVariable;

    std::unordered_map<
            sp<ITransactionCompletedListener>,
            std::unordered_map<std::vector<CallbackId>, uint32_t /*count*/, CallbackIdsHash>,
            ITransactionCompletedListenerHash>
            mPendingTransactions GUARDED_BY(mMutex);
    std::unordered_map<sp<ITransactionCompletedListener>, std::deque<TransactionStats>,
                       ITransactionCompletedListenerHash>
            mCompletedTransactions GUARDED_BY(mMutex);

    bool mRunning GUARDED_BY(mMutex) = false;
    bool mKeepRunning GUARDED_BY(mMutex) = true;

    sp<Fence> mPresentFence GUARDED_BY(mMutex);
};

} // namespace android
