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

#include "SensorInterface.h"
#include "SensorDevice.h"
#include "SensorFusion.h"

#include <stdint.h>
#include <sys/types.h>

namespace android {
// ---------------------------------------------------------------------------

namespace {
const sensor_t DUMMY_SENSOR = {
        .name = "", .vendor = "", .stringType = "", .requiredPermission = ""};
} //unnamed namespace

BaseSensor::BaseSensor(const sensor_t& sensor) :
        mSensorDevice(SensorDevice::getInstance()),
        mSensor(&sensor, mSensorDevice.getHalDeviceVersion()) {
}

BaseSensor::BaseSensor(const sensor_t& sensor, const uint8_t (&uuid)[16]) :
        mSensorDevice(SensorDevice::getInstance()),
        mSensor(sensor, Sensor::uuid_t(uuid), mSensorDevice.getHalDeviceVersion()) {
}

// ---------------------------------------------------------------------------

HardwareSensor::HardwareSensor(const sensor_t& sensor):
        BaseSensor(sensor) {
}

HardwareSensor::HardwareSensor(const sensor_t& sensor, const uint8_t (&uuid)[16]):
        BaseSensor(sensor, uuid) {
}

HardwareSensor::~HardwareSensor() {
}

bool HardwareSensor::process(sensors_event_t* outEvent,
        const sensors_event_t& event) {
    *outEvent = event;
    return true;
}

status_t HardwareSensor::activate(void* ident, bool enabled) {
    return mSensorDevice.activate(ident, mSensor.getHandle(), enabled);
}

status_t HardwareSensor::batch(void* ident, int /*handle*/, int flags,
                               int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    return mSensorDevice.batch(ident, mSensor.getHandle(), flags, samplingPeriodNs,
                               maxBatchReportLatencyNs);
}

status_t HardwareSensor::flush(void* ident, int handle) {
    return mSensorDevice.flush(ident, handle);
}

status_t HardwareSensor::setDelay(void* ident, int handle, int64_t ns) {
    return mSensorDevice.setDelay(ident, handle, ns);
}

void HardwareSensor::autoDisable(void *ident, int handle) {
    mSensorDevice.autoDisable(ident, handle);
}

VirtualSensor::VirtualSensor() :
        BaseSensor(DUMMY_SENSOR), mSensorFusion(SensorFusion::getInstance()) {
}

// ---------------------------------------------------------------------------
}; // namespace android
