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

#ifndef ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_UTILS_H
#define ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_UTILS_H

#include <android/frameworks/sensorservice/1.0/types.h>
#include <android/hardware/sensors/1.0/types.h>
#include <hidl/HidlSupport.h>
#include <sensor/Sensor.h>
namespace android {
namespace frameworks {
namespace sensorservice {
namespace V1_0 {
namespace implementation {

::android::hardware::sensors::V1_0::SensorInfo convertSensor(const ::android::Sensor &src);
Result convertResult(status_t status);

::android::hardware::sensors::V1_0::Event convertEvent(const ::ASensorEvent &event);

}  // namespace implementation
}  // namespace V1_0
}  // namespace sensorservice
}  // namespace frameworks
}  // namespace android

#endif  // ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_UTILS_H
