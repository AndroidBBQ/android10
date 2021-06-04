/*
 * Copyright 2019 The Android Open Source Project
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
#define LOG_TAG "MediaPlayerService-DeathNotifier"
#include <android-base/logging.h>

#include "DeathNotifier.h"

namespace android {

class DeathNotifier::DeathRecipient :
        public IBinder::DeathRecipient,
        public hardware::hidl_death_recipient {
public:
    using Notify = DeathNotifier::Notify;

    DeathRecipient(Notify const& notify): mNotify{notify} {
    }

    virtual void binderDied(wp<IBinder> const&) override {
        mNotify();
    }

    virtual void serviceDied(uint64_t, wp<HBase> const&) override {
        mNotify();
    }

private:
    Notify mNotify;
};

DeathNotifier::DeathNotifier(sp<IBinder> const& service, Notify const& notify)
      : mService{std::in_place_index<1>, service},
        mDeathRecipient{new DeathRecipient(notify)} {
    service->linkToDeath(mDeathRecipient);
}

DeathNotifier::DeathNotifier(sp<HBase> const& service, Notify const& notify)
      : mService{std::in_place_index<2>, service},
        mDeathRecipient{new DeathRecipient(notify)} {
    service->linkToDeath(mDeathRecipient, 0);
}

DeathNotifier::DeathNotifier(DeathNotifier&& other)
      : mService{other.mService}, mDeathRecipient{other.mDeathRecipient} {
    other.mService.emplace<0>();
    other.mDeathRecipient = nullptr;
}

DeathNotifier::~DeathNotifier() {
    switch (mService.index()) {
    case 0:
        break;
    case 1:
        std::get<1>(mService)->unlinkToDeath(mDeathRecipient);
        break;
    case 2:
        std::get<2>(mService)->unlinkToDeath(mDeathRecipient);
        break;
    default:
        CHECK(false) << "Corrupted service type during destruction.";
    }
}

} // namespace android

