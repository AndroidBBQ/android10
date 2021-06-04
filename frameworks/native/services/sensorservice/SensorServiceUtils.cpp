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

#include "SensorServiceUtils.h"

#include <hardware/sensors.h>

namespace android {
namespace SensorServiceUtil {

// Keep in sync with sSensorReportingMode in Sensor.java
size_t eventSizeBySensorType(int type) {
    if (type >= SENSOR_TYPE_DEVICE_PRIVATE_BASE) {
        return 16;
    }
    switch (type) {
        case SENSOR_TYPE_POSE_6DOF:
            return 16;

        case SENSOR_TYPE_ROTATION_VECTOR:
        case SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR:
            return 5;

        case SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED:
        case SENSOR_TYPE_GYROSCOPE_UNCALIBRATED:
            return 6;

        case SENSOR_TYPE_GAME_ROTATION_VECTOR:
            return 4;

        case SENSOR_TYPE_SIGNIFICANT_MOTION:
        case SENSOR_TYPE_STEP_DETECTOR:
        case SENSOR_TYPE_STEP_COUNTER:
        case SENSOR_TYPE_HEART_RATE:
        case SENSOR_TYPE_TILT_DETECTOR:
        case SENSOR_TYPE_WAKE_GESTURE:
        case SENSOR_TYPE_GLANCE_GESTURE:
        case SENSOR_TYPE_PICK_UP_GESTURE:
        case SENSOR_TYPE_WRIST_TILT_GESTURE:
        case SENSOR_TYPE_DEVICE_ORIENTATION:
        case SENSOR_TYPE_STATIONARY_DETECT:
        case SENSOR_TYPE_MOTION_DETECT:
        case SENSOR_TYPE_HEART_BEAT:
        case SENSOR_TYPE_LOW_LATENCY_OFFBODY_DETECT:
            return 1;

        default:
            return 3;
    }
}

} // namespace SensorServiceUtil
} // namespace android;
