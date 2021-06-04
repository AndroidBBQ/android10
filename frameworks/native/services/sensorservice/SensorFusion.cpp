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

#include "SensorDevice.h"
#include "SensorFusion.h"
#include "SensorService.h"

namespace android {
// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(SensorFusion)

SensorFusion::SensorFusion()
    : mSensorDevice(SensorDevice::getInstance()),
      mAttitude(mAttitudes[FUSION_9AXIS]),
      mGyroTime(0), mAccTime(0)
{
    sensor_t const* list;
    Sensor uncalibratedGyro;
    ssize_t count = mSensorDevice.getSensorList(&list);

    mEnabled[FUSION_9AXIS] = false;
    mEnabled[FUSION_NOMAG] = false;
    mEnabled[FUSION_NOGYRO] = false;

    if (count > 0) {
        for (size_t i=0 ; i<size_t(count) ; i++) {
            if (list[i].type == SENSOR_TYPE_ACCELEROMETER) {
                mAcc = Sensor(list + i);
            }
            if (list[i].type == SENSOR_TYPE_MAGNETIC_FIELD) {
                mMag = Sensor(list + i);
            }
            if (list[i].type == SENSOR_TYPE_GYROSCOPE) {
                mGyro = Sensor(list + i);
            }
            if (list[i].type == SENSOR_TYPE_GYROSCOPE_UNCALIBRATED) {
                uncalibratedGyro = Sensor(list + i);
            }
        }

        // Use the uncalibrated gyroscope for sensor fusion when available
        if (uncalibratedGyro.getType() == SENSOR_TYPE_GYROSCOPE_UNCALIBRATED) {
            mGyro = uncalibratedGyro;
        }

        // 200 Hz for gyro events is a good compromise between precision
        // and power/cpu usage.
        mEstimatedGyroRate = 200;
        mTargetDelayNs = 1000000000LL/mEstimatedGyroRate;

        for (int i = 0; i<NUM_FUSION_MODE; ++i) {
            mFusions[i].init(i);
        }
    }
}

void SensorFusion::process(const sensors_event_t& event) {

    if (event.type == mGyro.getType()) {
        float dT;
        if ( event.timestamp - mGyroTime> 0 &&
             event.timestamp - mGyroTime< (int64_t)(5e7) ) { //0.05sec

            dT = (event.timestamp - mGyroTime) / 1000000000.0f;
            // here we estimate the gyro rate (useful for debugging)
            const float freq = 1 / dT;
            if (freq >= 100 && freq<1000) { // filter values obviously wrong
                const float alpha = 1 / (1 + dT); // 1s time-constant
                mEstimatedGyroRate = freq + (mEstimatedGyroRate - freq)*alpha;
            }

            const vec3_t gyro(event.data);
            for (int i = 0; i<NUM_FUSION_MODE; ++i) {
                if (mEnabled[i]) {
                    // fusion in no gyro mode will ignore
                    mFusions[i].handleGyro(gyro, dT);
                }
            }
        }
        mGyroTime = event.timestamp;
    } else if (event.type == SENSOR_TYPE_MAGNETIC_FIELD) {
        const vec3_t mag(event.data);
        for (int i = 0; i<NUM_FUSION_MODE; ++i) {
            if (mEnabled[i]) {
                mFusions[i].handleMag(mag);// fusion in no mag mode will ignore
            }
        }
    } else if (event.type == SENSOR_TYPE_ACCELEROMETER) {
        float dT;
        if ( event.timestamp - mAccTime> 0 &&
             event.timestamp - mAccTime< (int64_t)(1e8) ) { //0.1sec
            dT = (event.timestamp - mAccTime) / 1000000000.0f;

            const vec3_t acc(event.data);
            for (int i = 0; i<NUM_FUSION_MODE; ++i) {
                if (mEnabled[i]) {
                    mFusions[i].handleAcc(acc, dT);
                    mAttitudes[i] = mFusions[i].getAttitude();
                }
            }
        }
        mAccTime = event.timestamp;
    }
}

template <typename T> inline T min(T a, T b) { return a<b ? a : b; }
template <typename T> inline T max(T a, T b) { return a>b ? a : b; }

status_t SensorFusion::activate(int mode, void* ident, bool enabled) {

    ALOGD_IF(DEBUG_CONNECTIONS,
            "SensorFusion::activate(mode=%d, ident=%p, enabled=%d)",
            mode, ident, enabled);

    const ssize_t idx = mClients[mode].indexOf(ident);
    if (enabled) {
        if (idx < 0) {
            mClients[mode].add(ident);
        }
    } else {
        if (idx >= 0) {
            mClients[mode].removeItemsAt(idx);
        }
    }

    const bool newState = mClients[mode].size() != 0;
    if (newState != mEnabled[mode]) {
        mEnabled[mode] = newState;
        if (newState) {
            mFusions[mode].init(mode);
        }
    }

    mSensorDevice.activate(ident, mAcc.getHandle(), enabled);
    if (mode != FUSION_NOMAG) {
        mSensorDevice.activate(ident, mMag.getHandle(), enabled);
    }
    if (mode != FUSION_NOGYRO) {
        mSensorDevice.activate(ident, mGyro.getHandle(), enabled);
    }

    return NO_ERROR;
}

status_t SensorFusion::setDelay(int mode, void* ident, int64_t ns) {
    // Call batch with timeout zero instead of setDelay().
    if (ns > (int64_t)5e7) {
        ns = (int64_t)(5e7);
    }
    mSensorDevice.batch(ident, mAcc.getHandle(), 0, ns, 0);
    if (mode != FUSION_NOMAG) {
        mSensorDevice.batch(ident, mMag.getHandle(), 0, ms2ns(10), 0);
    }
    if (mode != FUSION_NOGYRO) {
        mSensorDevice.batch(ident, mGyro.getHandle(), 0, mTargetDelayNs, 0);
    }
    return NO_ERROR;
}


float SensorFusion::getPowerUsage(int mode) const {
    float power =   mAcc.getPowerUsage() +
                    ((mode != FUSION_NOMAG) ? mMag.getPowerUsage() : 0) +
                    ((mode != FUSION_NOGYRO) ? mGyro.getPowerUsage() : 0);
    return power;
}

int32_t SensorFusion::getMinDelay() const {
    return mAcc.getMinDelay();
}

void SensorFusion::dump(String8& result) {
    const Fusion& fusion_9axis(mFusions[FUSION_9AXIS]);
    result.appendFormat("9-axis fusion %s (%zd clients), gyro-rate=%7.2fHz, "
            "q=< %g, %g, %g, %g > (%g), "
            "b=< %g, %g, %g >\n",
            mEnabled[FUSION_9AXIS] ? "enabled" : "disabled",
            mClients[FUSION_9AXIS].size(),
            mEstimatedGyroRate,
            fusion_9axis.getAttitude().x,
            fusion_9axis.getAttitude().y,
            fusion_9axis.getAttitude().z,
            fusion_9axis.getAttitude().w,
            length(fusion_9axis.getAttitude()),
            fusion_9axis.getBias().x,
            fusion_9axis.getBias().y,
            fusion_9axis.getBias().z);

    const Fusion& fusion_nomag(mFusions[FUSION_NOMAG]);
    result.appendFormat("game fusion(no mag) %s (%zd clients), "
            "gyro-rate=%7.2fHz, "
            "q=< %g, %g, %g, %g > (%g), "
            "b=< %g, %g, %g >\n",
            mEnabled[FUSION_NOMAG] ? "enabled" : "disabled",
            mClients[FUSION_NOMAG].size(),
            mEstimatedGyroRate,
            fusion_nomag.getAttitude().x,
            fusion_nomag.getAttitude().y,
            fusion_nomag.getAttitude().z,
            fusion_nomag.getAttitude().w,
            length(fusion_nomag.getAttitude()),
            fusion_nomag.getBias().x,
            fusion_nomag.getBias().y,
            fusion_nomag.getBias().z);

    const Fusion& fusion_nogyro(mFusions[FUSION_NOGYRO]);
    result.appendFormat("geomag fusion (no gyro) %s (%zd clients), "
            "gyro-rate=%7.2fHz, "
            "q=< %g, %g, %g, %g > (%g), "
            "b=< %g, %g, %g >\n",
            mEnabled[FUSION_NOGYRO] ? "enabled" : "disabled",
            mClients[FUSION_NOGYRO].size(),
            mEstimatedGyroRate,
            fusion_nogyro.getAttitude().x,
            fusion_nogyro.getAttitude().y,
            fusion_nogyro.getAttitude().z,
            fusion_nogyro.getAttitude().w,
            length(fusion_nogyro.getAttitude()),
            fusion_nogyro.getBias().x,
            fusion_nogyro.getBias().y,
            fusion_nogyro.getBias().z);
}

// ---------------------------------------------------------------------------
}; // namespace android
