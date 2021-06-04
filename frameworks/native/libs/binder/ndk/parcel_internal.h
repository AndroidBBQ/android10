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

#pragma once

#include <android/binder_parcel.h>

#include <sys/cdefs.h>

#include <binder/Parcel.h>
#include "ibinder_internal.h"

struct AParcel {
    const ::android::Parcel* get() const { return mParcel; }
    ::android::Parcel* get() { return mParcel; }

    explicit AParcel(const AIBinder* binder)
        : AParcel(binder, new ::android::Parcel, true /*owns*/) {}
    AParcel(const AIBinder* binder, ::android::Parcel* parcel, bool owns)
        : mBinder(binder), mParcel(parcel), mOwns(owns) {}

    ~AParcel() {
        if (mOwns) {
            delete mParcel;
        }
    }

    static const AParcel readOnly(const AIBinder* binder, const ::android::Parcel* parcel) {
        return AParcel(binder, const_cast<::android::Parcel*>(parcel), false);
    }

    const AIBinder* getBinder() { return mBinder; }

   private:
    // This object is associated with a calls to a specific AIBinder object. This is used for sanity
    // checking to make sure that a parcel is one that is expected.
    const AIBinder* mBinder;

    ::android::Parcel* mParcel;
    bool mOwns;
};
