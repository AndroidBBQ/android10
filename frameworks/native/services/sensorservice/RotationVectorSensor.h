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

#ifndef ANDROID_ROTATION_VECTOR_SENSOR_H
#define ANDROID_ROTATION_VECTOR_SENSOR_H

#include <stdint.h>
#include <sys/types.h>

#include <sensor/Sensor.h>

#include "SensorDevice.h"
#include "SensorInterface.h"

#include "Fusion.h"
#include "SensorFusion.h"

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

class RotationVectorSensor : public VirtualSensor {
public:
    explicit RotationVectorSensor(int mode = FUSION_9AXIS);
    virtual bool process(sensors_event_t* outEvent, const sensors_event_t& event) override;
    virtual status_t activate(void* ident, bool enabled) override;
    virtual status_t setDelay(void* ident, int handle, int64_t ns) override;

protected:
    const int mMode;
    int getSensorType() const;
    const char* getSensorName() const ;
    int getSensorToken() const ;
};

class GameRotationVectorSensor : public RotationVectorSensor {
public:
    GameRotationVectorSensor() : RotationVectorSensor(FUSION_NOMAG) {}
};

class GeoMagRotationVectorSensor : public RotationVectorSensor {
public:
    GeoMagRotationVectorSensor() : RotationVectorSensor(FUSION_NOGYRO) {}
};

class GyroDriftSensor : public VirtualSensor {
public:
    GyroDriftSensor();
    virtual bool process(sensors_event_t* outEvent, const sensors_event_t& event) override;
    virtual status_t activate(void* ident, bool enabled) override;
    virtual status_t setDelay(void* ident, int handle, int64_t ns) override;
};

// ---------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_ROTATION_VECTOR_SENSOR_H
