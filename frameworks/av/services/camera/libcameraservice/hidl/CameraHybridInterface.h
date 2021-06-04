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

#ifndef ANDROID_CAMERA_HYBRIDINTERFACE_H
#define ANDROID_CAMERA_HYBRIDINTERFACE_H

#include <vector>
#include <mutex>

#include <binder/Parcel.h>
#include <hidl/HidlSupport.h>

namespace android {
namespace camerahybrid {
typedef ::android::hidl::base::V1_0::IBase HInterface;

template <
        typename HINTERFACE,
        typename INTERFACE,
        typename BNINTERFACE >
class H2BConverter : public BNINTERFACE {
public:
    typedef H2BConverter<HINTERFACE, INTERFACE, BNINTERFACE> CBase; // Converter Base
    typedef INTERFACE BaseInterface;
    typedef HINTERFACE HalInterface;

    H2BConverter(const sp<HalInterface>& base) : mBase(base) {}
    virtual sp<HalInterface> getHalInterface() { return mBase; }
    virtual status_t linkToDeath(
            const sp<IBinder::DeathRecipient>& recipient,
            void* cookie = nullptr,
            uint32_t flags = 0);
    virtual status_t unlinkToDeath(
            const wp<IBinder::DeathRecipient>& recipient,
            void* cookie = nullptr,
            uint32_t flags = 0,
            wp<IBinder::DeathRecipient>* outRecipient = nullptr);

protected:
    sp<HalInterface> mBase;
    struct Obituary : public hardware::hidl_death_recipient {
        wp<IBinder::DeathRecipient> recipient;
        void* cookie;
        uint32_t flags;
        wp<IBinder> who;
        Obituary(
                const wp<IBinder::DeathRecipient>& r,
                void* c, uint32_t f,
                const wp<IBinder>& w) :
            recipient(r), cookie(c), flags(f), who(w) {
        }
        Obituary(const Obituary& o) :
            recipient(o.recipient),
            cookie(o.cookie),
            flags(o.flags),
            who(o.who) {
        }
        Obituary& operator=(const Obituary& o) {
            recipient = o.recipient;
            cookie = o.cookie;
            flags = o.flags;
            who = o.who;
            return *this;
        }
        void serviceDied(uint64_t, const wp<HInterface>&) override {
            sp<IBinder::DeathRecipient> dr = recipient.promote();
            if (dr != nullptr) {
                dr->binderDied(who);
            }
        }
    };
    std::mutex mObituariesLock;
    std::vector<sp<Obituary> > mObituaries;
};

template <
        typename HINTERFACE,
        typename INTERFACE,
        typename BNINTERFACE>
status_t H2BConverter<HINTERFACE, INTERFACE, BNINTERFACE>::
        linkToDeath(
        const sp<IBinder::DeathRecipient>& recipient,
        void* cookie, uint32_t flags) {
    LOG_ALWAYS_FATAL_IF(recipient == nullptr,
            "linkToDeath(): recipient must be non-nullptr");
    {
        std::lock_guard<std::mutex> lock(mObituariesLock);
        mObituaries.push_back(new Obituary(recipient, cookie, flags, this));
        if (!mBase->linkToDeath(mObituaries.back(), 0)) {
           return DEAD_OBJECT;
        }
    }
    return NO_ERROR;
}

template <
        typename HINTERFACE,
        typename INTERFACE,
        typename BNINTERFACE>
status_t H2BConverter<HINTERFACE, INTERFACE, BNINTERFACE>::
        unlinkToDeath(
        const wp<IBinder::DeathRecipient>& recipient,
        void* cookie, uint32_t flags,
        wp<IBinder::DeathRecipient>* outRecipient) {
    std::lock_guard<std::mutex> lock(mObituariesLock);
    for (auto i = mObituaries.begin(); i != mObituaries.end(); ++i) {
        if ((flags = (*i)->flags) && (
                (recipient == (*i)->recipient) ||
                ((recipient == nullptr) && (cookie == (*i)->cookie)))) {
            if (outRecipient != nullptr) {
                *outRecipient = (*i)->recipient;
            }
            bool success = mBase->unlinkToDeath(*i);
            mObituaries.erase(i);
            return success ? NO_ERROR : DEAD_OBJECT;
        }
    }
    return NAME_NOT_FOUND;
}

} // namespace camerahybrid
} // namespace android

#endif // ANDROID_CAMERA_HYBRIDINTERFACE_H

