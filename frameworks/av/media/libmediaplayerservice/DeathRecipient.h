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

#ifndef ANDROID_MEDIASERVICE_DEATHRECIPIENT_H
#define ANDROID_MEDIASERVICE_DEATHRECIPIENT_H

#include <binder/binder.h>
#include <hidl/HidlSupport.h>

#include <variant>

class DeathNotifier :
        public IBinder::DeathRecipient,
        public ::android::hardware::hidl_death_recipient {
public:
    using Service = std::variant<
            sp<IBinder> const&,
            sp<android::hidl::base::V1_0::IBase> const&>;

    DeathNotifier(std::variant<sp<IBinder> const&, 
            const sp<IBinder>& service,
            const sp<MediaPlayerBase>& listener,
            int which,
            const std::string& name);
    DeathNotifier(
            const sp<android::hidl::base::V1_0::IBase>& hService,
            const sp<MediaPlayerBase>& listener,
            int which,
            const std::string& name);
    virtual ~DeathNotifier() = default;
    virtual void binderDied(const wp<IBinder>& who);
    virtual void serviceDied(
            uint64_t cookie,
            const wp<::android::hidl::base::V1_0::IBase>& who);
    void unlinkToDeath();

private:
    sp<IBinder> mService;
    sp<android::hidl::base::V1_0::IBase> mHService; // HIDL service
    wp<MediaPlayerBase> mListener;
    int mWhich;
    std::string mName;
};

#endif // ANDROID_MEDIASERVICE_DEATHRECIPIENT_H

