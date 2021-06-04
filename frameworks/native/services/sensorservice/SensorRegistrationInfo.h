/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef ANDROID_SENSOR_REGISTRATION_INFO_H
#define ANDROID_SENSOR_REGISTRATION_INFO_H

#include "SensorServiceUtils.h"
#include <utils/Thread.h>
#include <iomanip>
#include <sstream>

namespace android {

class SensorService;

class SensorService::SensorRegistrationInfo : public SensorServiceUtil::Dumpable {
public:
    SensorRegistrationInfo() : mPackageName() {
        mSensorHandle = mSamplingRateUs = mMaxReportLatencyUs = INT32_MIN;
        mHour = mMin = mSec = INT8_MIN;
        mActivated = false;
    }

    SensorRegistrationInfo(int32_t handle, const String8 &packageName,
                           int64_t samplingRateNs, int64_t maxReportLatencyNs, bool activate) {
        mSensorHandle = handle;
        mPackageName = packageName;

        mSamplingRateUs = static_cast<int64_t>(samplingRateNs/1000);
        mMaxReportLatencyUs = static_cast<int64_t>(maxReportLatencyNs/1000);
        mActivated = activate;

        IPCThreadState *thread = IPCThreadState::self();
        mPid = (thread != nullptr) ? thread->getCallingPid() : -1;
        mUid = (thread != nullptr) ? thread->getCallingUid() : -1;

        time_t rawtime = time(nullptr);
        struct tm * timeinfo = localtime(&rawtime);
        mHour = static_cast<int8_t>(timeinfo->tm_hour);
        mMin = static_cast<int8_t>(timeinfo->tm_min);
        mSec = static_cast<int8_t>(timeinfo->tm_sec);
    }

    static bool isSentinel(const SensorRegistrationInfo& info) {
       return (info.mHour == INT8_MIN &&
               info.mMin == INT8_MIN &&
               info.mSec == INT8_MIN);
    }

    // Dumpable interface
    virtual std::string dump() const override {
        std::ostringstream ss;
        ss << std::setfill('0') << std::setw(2) << static_cast<int>(mHour) << ":"
           << std::setw(2) << static_cast<int>(mMin) << ":"
           << std::setw(2) << static_cast<int>(mSec)
           << (mActivated ? " +" : " -")
           << " 0x" << std::hex << std::setw(8) << mSensorHandle << std::dec
           << std::setfill(' ') << " pid=" << std::setw(5) << mPid
           << " uid=" << std::setw(5) << mUid << " package=" << mPackageName;
        if (mActivated) {
           ss  << " samplingPeriod=" << mSamplingRateUs << "us"
               << " batchingPeriod=" << mMaxReportLatencyUs << "us";
        };
        return ss.str();
    }

private:
    int32_t mSensorHandle;
    String8 mPackageName;
    pid_t   mPid;
    uid_t   mUid;
    int64_t mSamplingRateUs;
    int64_t mMaxReportLatencyUs;
    bool mActivated;
    int8_t mHour, mMin, mSec;

};

} // namespace android;

#endif // ANDROID_SENSOR_REGISTRATION_INFO_H


