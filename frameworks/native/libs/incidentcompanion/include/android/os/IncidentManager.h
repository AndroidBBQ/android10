/**
 * Copyright (c) 2019, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <binder/Parcel.h>
#include <binder/Parcelable.h>
#include <utils/String16.h>

#include <set>
#include <vector>

namespace android {
namespace os {

class IncidentManager : public virtual RefBase {
public:
    class IncidentReport : public Parcelable {
    public:
        IncidentReport();
        virtual ~IncidentReport();

        virtual status_t writeToParcel(Parcel* out) const;
        virtual status_t readFromParcel(const Parcel* in);

        void setTimestampNs(int64_t val) { mTimestampNs = val; }
        int64_t getTimestampNs() const { return mTimestampNs; }
        int64_t getTimestampMs() const { return mTimestampNs / 1000000; }

        void setPrivacyPolicy(int32_t val) { mPrivacyPolicy = val; }
        // This was accidentally published as a long in the java api.
        int64_t getPrivacyPolicy() const { return mPrivacyPolicy; }
        // Dups the fd, so you retain ownership of the original one.  If there is a
        // previously set fd, closes that, since this object owns its own fd.
        status_t setFileDescriptor(int fd);

        // Does not dup the fd, so ownership is passed to this object.  If there is a
        // previously set fd, closes that, since this object owns its own fd.
        void takeFileDescriptor(int fd);

        // Returns the fd, which you don't own.  Call dup if you need a copy.
        int getFileDescriptor() const { return mFileDescriptor; }

    private:
        int64_t mTimestampNs;
        int32_t mPrivacyPolicy;
        int mFileDescriptor;
    };


private:
    // Not implemented for now.
    IncidentManager();
    virtual ~IncidentManager();
};
}
}

