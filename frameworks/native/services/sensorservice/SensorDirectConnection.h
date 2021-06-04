/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_SENSOR_DIRECT_CONNECTION_H
#define ANDROID_SENSOR_DIRECT_CONNECTION_H

#include <stdint.h>
#include <sys/types.h>

#include <binder/BinderService.h>

#include <sensor/Sensor.h>
#include <sensor/BitTube.h>
#include <sensor/ISensorServer.h>
#include <sensor/ISensorEventConnection.h>

#include "SensorService.h"

namespace android {

class SensorService;
class BitTube;

class SensorService::SensorDirectConnection: public BnSensorEventConnection {
public:
    SensorDirectConnection(const sp<SensorService>& service, uid_t uid,
            const sensors_direct_mem_t *mem, int32_t halChannelHandle,
            const String16& opPackageName);
    void dump(String8& result) const;
    uid_t getUid() const { return mUid; }
    int32_t getHalChannelHandle() const;
    bool isEquivalent(const sensors_direct_mem_t *mem) const;

    // stop all active sensor report. if backupRecord is set to false,
    // those report can be recovered by recoverAll
    // called by SensorService when enter restricted mode
    void stopAll(bool backupRecord = false);

    // recover sensor reports previously stopped by stopAll(true)
    // called by SensorService when return to NORMAL mode.
    void recoverAll();

protected:
    virtual ~SensorDirectConnection();
    // ISensorEventConnection functions
    virtual void onFirstRef();
    virtual sp<BitTube> getSensorChannel() const;
    virtual status_t enableDisable(int handle, bool enabled, nsecs_t samplingPeriodNs,
                                   nsecs_t maxBatchReportLatencyNs, int reservedFlags);
    virtual status_t setEventRate(int handle, nsecs_t samplingPeriodNs);
    virtual status_t flush();
    virtual int32_t configureChannel(int handle, int rateLevel);
    virtual void destroy();
private:
    const sp<SensorService> mService;
    const uid_t mUid;
    const sensors_direct_mem_t mMem;
    const int32_t mHalChannelHandle;
    const String16 mOpPackageName;

    mutable Mutex mConnectionLock;
    std::unordered_map<int, int> mActivated;
    std::unordered_map<int, int> mActivatedBackup;

    mutable Mutex mDestroyLock;
    bool mDestroyed;
};

} // namepsace android

#endif // ANDROID_SENSOR_DIRECT_CONNECTION_H

