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

#include <stdint.h>
#include <math.h>
#include <sys/types.h>

#include <utils/Errors.h>

#include <hardware/sensors.h>

#include "CorrectedGyroSensor.h"
#include "SensorDevice.h"
#include "SensorFusion.h"

namespace android {
// ---------------------------------------------------------------------------

CorrectedGyroSensor::CorrectedGyroSensor(sensor_t const* list, size_t count)
    : VirtualSensor() {
    for (size_t i=0 ; i<count ; i++) {
        if (list[i].type == SENSOR_TYPE_GYROSCOPE) {
            mGyro = Sensor(list + i);
            break;
        }
    }

    const sensor_t sensor = {
            .name       = "Corrected Gyroscope Sensor",
            .vendor     = "AOSP",
            .version    = 1,
            .handle     = '_cgy',
            .type       = SENSOR_TYPE_GYROSCOPE,
            .maxRange   = mGyro.getMaxValue(),
            .resolution = mGyro.getResolution(),
            .power      = mSensorFusion.getPowerUsage(),
            .minDelay   = mGyro.getMinDelay(),
    };
    mSensor = Sensor(&sensor);
}

bool CorrectedGyroSensor::process(sensors_event_t* outEvent,
        const sensors_event_t& event)
{
    if (event.type == SENSOR_TYPE_GYROSCOPE) {
        const vec3_t bias(mSensorFusion.getGyroBias());
        *outEvent = event;
        outEvent->data[0] -= bias.x;
        outEvent->data[1] -= bias.y;
        outEvent->data[2] -= bias.z;
        outEvent->sensor = '_cgy';
        return true;
    }
    return false;
}

status_t CorrectedGyroSensor::activate(void* ident, bool enabled) {
    mSensorDevice.activate(ident, mGyro.getHandle(), enabled);
    return mSensorFusion.activate(FUSION_9AXIS, ident, enabled);
}

status_t CorrectedGyroSensor::setDelay(void* ident, int /*handle*/, int64_t ns) {
    mSensorDevice.setDelay(ident, mGyro.getHandle(), ns);
    return mSensorFusion.setDelay(FUSION_9AXIS, ident, ns);
}

// ---------------------------------------------------------------------------
}; // namespace android

