/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef ANDROID_BINDING_AAUDIO_STREAM_REQUEST_H
#define ANDROID_BINDING_AAUDIO_STREAM_REQUEST_H

#include <stdint.h>

#include <aaudio/AAudio.h>
#include <binder/Parcel.h>
#include <binder/Parcelable.h>

#include "binding/AAudioStreamConfiguration.h"

using android::status_t;
using android::Parcel;
using android::Parcelable;

namespace aaudio {

class AAudioStreamRequest : public Parcelable {
public:
    AAudioStreamRequest();
    virtual ~AAudioStreamRequest();

    uid_t getUserId() const {
        return mUserId;
    }

    void setUserId(uid_t userId) {
        mUserId = userId;
    }

    pid_t getProcessId() const {
        return mProcessId;
    }

    void setProcessId(pid_t processId) {
        mProcessId = processId;
    }

    bool isSharingModeMatchRequired() const {
        return mSharingModeMatchRequired;
    }

    void setSharingModeMatchRequired(bool required) {
        mSharingModeMatchRequired = required;
    }

    const AAudioStreamConfiguration &getConstantConfiguration() const {
        return mConfiguration;
    }

    AAudioStreamConfiguration &getConfiguration() {
        return mConfiguration;
    }

    bool isInService() const {
        return mInService;
    }

    void setInService(bool inService) {
        mInService = inService;
    }

    virtual status_t writeToParcel(Parcel* parcel) const override;

    virtual status_t readFromParcel(const Parcel* parcel) override;

    aaudio_result_t validate() const;

    void dump() const;

protected:
    AAudioStreamConfiguration  mConfiguration;
    uid_t                      mUserId = (uid_t) -1;
    pid_t                      mProcessId = (pid_t) -1;
    bool                       mSharingModeMatchRequired = false;
    bool                       mInService = false; // Stream opened by AAudioservice
};

} /* namespace aaudio */

#endif //ANDROID_BINDING_AAUDIO_STREAM_REQUEST_H
