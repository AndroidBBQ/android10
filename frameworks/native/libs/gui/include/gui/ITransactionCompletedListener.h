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

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/Parcelable.h>
#include <binder/SafeInterface.h>

#include <ui/Fence.h>
#include <utils/Timers.h>

#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace android {

class ITransactionCompletedListener;

using CallbackId = int64_t;

class SurfaceStats : public Parcelable {
public:
    status_t writeToParcel(Parcel* output) const override;
    status_t readFromParcel(const Parcel* input) override;

    SurfaceStats() = default;
    SurfaceStats(const sp<IBinder>& sc, nsecs_t time, const sp<Fence>& prevReleaseFence)
          : surfaceControl(sc), acquireTime(time), previousReleaseFence(prevReleaseFence) {}

    sp<IBinder> surfaceControl;
    nsecs_t acquireTime = -1;
    sp<Fence> previousReleaseFence;
};

class TransactionStats : public Parcelable {
public:
    status_t writeToParcel(Parcel* output) const override;
    status_t readFromParcel(const Parcel* input) override;

    TransactionStats() = default;
    TransactionStats(const std::vector<CallbackId>& ids) : callbackIds(ids) {}
    TransactionStats(const std::unordered_set<CallbackId>& ids)
          : callbackIds(ids.begin(), ids.end()) {}
    TransactionStats(const std::vector<CallbackId>& ids, nsecs_t latch, const sp<Fence>& present,
                     const std::vector<SurfaceStats>& surfaces)
          : callbackIds(ids), latchTime(latch), presentFence(present), surfaceStats(surfaces) {}

    std::vector<CallbackId> callbackIds;
    nsecs_t latchTime = -1;
    sp<Fence> presentFence = nullptr;
    std::vector<SurfaceStats> surfaceStats;
};

class ListenerStats : public Parcelable {
public:
    status_t writeToParcel(Parcel* output) const override;
    status_t readFromParcel(const Parcel* input) override;

    static ListenerStats createEmpty(const sp<ITransactionCompletedListener>& listener,
                                     const std::unordered_set<CallbackId>& callbackIds);

    sp<ITransactionCompletedListener> listener;
    std::vector<TransactionStats> transactionStats;
};

class ITransactionCompletedListener : public IInterface {
public:
    DECLARE_META_INTERFACE(TransactionCompletedListener)

    virtual void onTransactionCompleted(ListenerStats stats) = 0;
};

class BnTransactionCompletedListener : public SafeBnInterface<ITransactionCompletedListener> {
public:
    BnTransactionCompletedListener()
          : SafeBnInterface<ITransactionCompletedListener>("BnTransactionCompletedListener") {}

    status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                        uint32_t flags = 0) override;
};

class ListenerCallbacks {
public:
    ListenerCallbacks(const sp<ITransactionCompletedListener>& listener,
                      const std::unordered_set<CallbackId>& callbacks)
          : transactionCompletedListener(listener),
            callbackIds(callbacks.begin(), callbacks.end()) {}

    ListenerCallbacks(const sp<ITransactionCompletedListener>& listener,
                      const std::vector<CallbackId>& ids)
          : transactionCompletedListener(listener), callbackIds(ids) {}

    sp<ITransactionCompletedListener> transactionCompletedListener;
    std::vector<CallbackId> callbackIds;
};

} // namespace android
