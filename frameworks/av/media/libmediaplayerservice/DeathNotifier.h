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

#ifndef ANDROID_MEDIASERVICE_DEATHNOTIFIER_H
#define ANDROID_MEDIASERVICE_DEATHNOTIFIER_H

#include <android/hidl/base/1.0/IBase.h>
#include <binder/Binder.h>
#include <hidl/HidlSupport.h>

#include <variant>

namespace android {

class DeathNotifier {
public:
    using HBase = hidl::base::V1_0::IBase;
    using Notify = std::function<void()>;

    DeathNotifier(sp<IBinder> const& service, Notify const& notify);
    DeathNotifier(sp<HBase> const& service, Notify const& notify);
    DeathNotifier(DeathNotifier&& other);
    ~DeathNotifier();

private:
    std::variant<std::monostate, sp<IBinder>, sp<HBase>> mService;

    class DeathRecipient;
    sp<DeathRecipient> mDeathRecipient;
};

} // namespace android

#endif // ANDROID_MEDIASERVICE_DEATHNOTIFIER_H

