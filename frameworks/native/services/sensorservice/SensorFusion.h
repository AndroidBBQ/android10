/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef ANDROID_SENSOR_FUSION_H
#define ANDROID_SENSOR_FUSION_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/SortedVector.h>
#include <utils/Singleton.h>
#include <utils/String8.h>

#include <sensor/Sensor.h>

#include "Fusion.h"

// ---------------------------------------------------------------------------

namespace android {
// ---------------------------------------------------------------------------

class SensorDevice;

class SensorFusion : public Singleton<SensorFusion> {
    friend class Singleton<SensorFusion>;

    SensorDevice& mSensorDevice;
    Sensor mAcc;
    Sensor mMag;
    Sensor mGyro;

    Fusion mFusions[NUM_FUSION_MODE]; // normal, no_mag, no_gyro

    bool mEnabled[NUM_FUSION_MODE];

    vec4_t &mAttitude;
    vec4_t mAttitudes[NUM_FUSION_MODE];

    SortedVector<void*> mClients[3];

    float mEstimatedGyroRate;
    nsecs_t mTargetDelayNs;

    nsecs_t mGyroTime;
    nsecs_t mAccTime;

    SensorFusion();

public:
    void process(const sensors_event_t& event);

    bool isEnabled() const {
        return mEnabled[FUSION_9AXIS] ||
                mEnabled[FUSION_NOMAG] ||
                mEnabled[FUSION_NOGYRO];
    }

    bool hasEstimate(int mode = FUSION_9AXIS) const {
        return mFusions[mode].hasEstimate();
    }

    mat33_t getRotationMatrix(int mode = FUSION_9AXIS) const {
        return mFusions[mode].getRotationMatrix();
    }

    vec4_t getAttitude(int mode = FUSION_9AXIS) const {
        return mAttitudes[mode];
    }

    vec3_t getGyroBias() const { return mFusions[FUSION_9AXIS].getBias(); }
    float getEstimatedRate() const { return mEstimatedGyroRate; }

    status_t activate(int mode, void* ident, bool enabled);
    status_t setDelay(int mode, void* ident, int64_t ns);

    float getPowerUsage(int mode=FUSION_9AXIS) const;
    int32_t getMinDelay() const;

    void dump(String8& result);
};


// ---------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_SENSOR_FUSION_H
