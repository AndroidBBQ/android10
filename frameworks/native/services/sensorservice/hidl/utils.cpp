/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "utils.h"

#include <sensors/convert.h>

namespace android {
namespace frameworks {
namespace sensorservice {
namespace V1_0 {
namespace implementation {

using ::android::Sensor;
using ::android::hardware::hidl_string;
using ::android::hardware::sensors::V1_0::SensorInfo;

SensorInfo convertSensor(const Sensor& src) {
    SensorInfo dst;
    const String8& name = src.getName();
    const String8& vendor = src.getVendor();
    dst.name = hidl_string{name.string(), name.size()};
    dst.vendor = hidl_string{vendor.string(), vendor.size()};
    dst.version = src.getVersion();
    dst.sensorHandle = src.getHandle();
    dst.type = static_cast<::android::hardware::sensors::V1_0::SensorType>(
            src.getType());
    // maxRange uses maxValue because ::android::Sensor wraps the
    // internal sensor_t in this way.
    dst.maxRange = src.getMaxValue();
    dst.resolution = src.getResolution();
    dst.power = src.getPowerUsage();
    dst.minDelay = src.getMinDelay();
    dst.fifoReservedEventCount = src.getFifoReservedEventCount();
    dst.fifoMaxEventCount = src.getFifoMaxEventCount();
    dst.typeAsString = src.getStringType();
    dst.requiredPermission = src.getRequiredPermission();
    dst.maxDelay = src.getMaxDelay();
    dst.flags = src.getFlags();
    return dst;
}

Result convertResult(status_t status) {
    switch (status) {
        case OK:
            return Result::OK;
        case NAME_NOT_FOUND:
            return Result::NOT_EXIST;
        case NO_MEMORY:
            return Result::NO_MEMORY;
        case NO_INIT:
            return Result::NO_INIT;
        case PERMISSION_DENIED:
            return Result::PERMISSION_DENIED;
        case BAD_VALUE:
            return Result::BAD_VALUE;
        case INVALID_OPERATION:
            return Result::INVALID_OPERATION;
        default:
            return Result::UNKNOWN_ERROR;
    }
}

::android::hardware::sensors::V1_0::Event convertEvent(const ::ASensorEvent& src) {
    ::android::hardware::sensors::V1_0::Event dst;
    ::android::hardware::sensors::V1_0::implementation::convertFromSensorEvent(
            reinterpret_cast<const sensors_event_t&>(src), &dst);
    return dst;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace sensorservice
}  // namespace frameworks
}  // namespace android
