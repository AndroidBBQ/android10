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

#define LOG_TAG "ITransactionCompletedListener"
//#define LOG_NDEBUG 0

#include <gui/ITransactionCompletedListener.h>

namespace android {

namespace { // Anonymous

enum class Tag : uint32_t {
    ON_TRANSACTION_COMPLETED = IBinder::FIRST_CALL_TRANSACTION,
    LAST = ON_TRANSACTION_COMPLETED,
};

} // Anonymous namespace

status_t SurfaceStats::writeToParcel(Parcel* output) const {
    status_t err = output->writeStrongBinder(surfaceControl);
    if (err != NO_ERROR) {
        return err;
    }
    err = output->writeInt64(acquireTime);
    if (err != NO_ERROR) {
        return err;
    }
    if (previousReleaseFence) {
        err = output->writeBool(true);
        if (err != NO_ERROR) {
            return err;
        }
        err = output->write(*previousReleaseFence);
    } else {
        err = output->writeBool(false);
    }
    return err;
}

status_t SurfaceStats::readFromParcel(const Parcel* input) {
    status_t err = input->readStrongBinder(&surfaceControl);
    if (err != NO_ERROR) {
        return err;
    }
    err = input->readInt64(&acquireTime);
    if (err != NO_ERROR) {
        return err;
    }
    bool hasFence = false;
    err = input->readBool(&hasFence);
    if (err != NO_ERROR) {
        return err;
    }
    if (hasFence) {
        previousReleaseFence = new Fence();
        err = input->read(*previousReleaseFence);
        if (err != NO_ERROR) {
            return err;
        }
    }
    return NO_ERROR;
}

status_t TransactionStats::writeToParcel(Parcel* output) const {
    status_t err = output->writeInt64Vector(callbackIds);
    if (err != NO_ERROR) {
        return err;
    }
    err = output->writeInt64(latchTime);
    if (err != NO_ERROR) {
        return err;
    }
    if (presentFence) {
        err = output->writeBool(true);
        if (err != NO_ERROR) {
            return err;
        }
        err = output->write(*presentFence);
    } else {
        err = output->writeBool(false);
    }
    if (err != NO_ERROR) {
        return err;
    }
    return output->writeParcelableVector(surfaceStats);
}

status_t TransactionStats::readFromParcel(const Parcel* input) {
    status_t err = input->readInt64Vector(&callbackIds);
    if (err != NO_ERROR) {
        return err;
    }
    err = input->readInt64(&latchTime);
    if (err != NO_ERROR) {
        return err;
    }
    bool hasFence = false;
    err = input->readBool(&hasFence);
    if (err != NO_ERROR) {
        return err;
    }
    if (hasFence) {
        presentFence = new Fence();
        err = input->read(*presentFence);
        if (err != NO_ERROR) {
            return err;
        }
    }
    return input->readParcelableVector(&surfaceStats);
}

status_t ListenerStats::writeToParcel(Parcel* output) const {
    status_t err = output->writeInt32(static_cast<int32_t>(transactionStats.size()));
    if (err != NO_ERROR) {
        return err;
    }
    for (const auto& stats : transactionStats) {
        err = output->writeParcelable(stats);
        if (err != NO_ERROR) {
            return err;
        }
    }
    return NO_ERROR;
}

status_t ListenerStats::readFromParcel(const Parcel* input) {
    int32_t transactionStats_size = input->readInt32();

    for (int i = 0; i < transactionStats_size; i++) {
        TransactionStats stats;
        status_t err = input->readParcelable(&stats);
        if (err != NO_ERROR) {
            return err;
        }
        transactionStats.push_back(stats);
    }
    return NO_ERROR;
}

ListenerStats ListenerStats::createEmpty(const sp<ITransactionCompletedListener>& listener,
                                         const std::unordered_set<CallbackId>& callbackIds) {
    ListenerStats listenerStats;
    listenerStats.listener = listener;
    listenerStats.transactionStats.emplace_back(callbackIds);

    return listenerStats;
}

class BpTransactionCompletedListener : public SafeBpInterface<ITransactionCompletedListener> {
public:
    explicit BpTransactionCompletedListener(const sp<IBinder>& impl)
          : SafeBpInterface<ITransactionCompletedListener>(impl, "BpTransactionCompletedListener") {
    }

    ~BpTransactionCompletedListener() override;

    void onTransactionCompleted(ListenerStats stats) override {
        callRemoteAsync<decltype(&ITransactionCompletedListener::
                                         onTransactionCompleted)>(Tag::ON_TRANSACTION_COMPLETED,
                                                                  stats);
    }
};

// Out-of-line virtual method definitions to trigger vtable emission in this translation unit (see
// clang warning -Wweak-vtables)
BpTransactionCompletedListener::~BpTransactionCompletedListener() = default;

IMPLEMENT_META_INTERFACE(TransactionCompletedListener, "android.gui.ITransactionComposerListener");

status_t BnTransactionCompletedListener::onTransact(uint32_t code, const Parcel& data,
                                                    Parcel* reply, uint32_t flags) {
    if (code < IBinder::FIRST_CALL_TRANSACTION || code > static_cast<uint32_t>(Tag::LAST)) {
        return BBinder::onTransact(code, data, reply, flags);
    }
    auto tag = static_cast<Tag>(code);
    switch (tag) {
        case Tag::ON_TRANSACTION_COMPLETED:
            return callLocalAsync(data, reply,
                                  &ITransactionCompletedListener::onTransactionCompleted);
    }
}

}; // namespace android
