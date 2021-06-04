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

#ifndef ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_EVENTQUEUE_H
#define ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_EVENTQUEUE_H

#include "SensorManager.h"

#include <android/frameworks/sensorservice/1.0/IEventQueue.h>
#include <android/frameworks/sensorservice/1.0/IEventQueueCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <sensor/SensorManager.h>

namespace android {
namespace frameworks {
namespace sensorservice {
namespace V1_0 {
namespace implementation {

using ::android::frameworks::sensorservice::V1_0::IEventQueue;
using ::android::frameworks::sensorservice::V1_0::IEventQueueCallback;
using ::android::frameworks::sensorservice::V1_0::Result;
using ::android::hardware::Return;
using ::android::sp;

struct EventQueue final : public IEventQueue {
    EventQueue(
        sp<IEventQueueCallback> callback,
        sp<::android::Looper> looper,
        sp<::android::SensorEventQueue> internalQueue);
    void onLastStrongRef(const void *) override;

    // Methods from ::android::frameworks::sensorservice::V1_0::IEventQueue follow.
    Return<Result> enableSensor(int32_t sensorHandle, int32_t samplingPeriodUs, int64_t maxBatchReportLatencyUs) override;
    Return<Result> disableSensor(int32_t sensorHandle) override;

private:
    friend class EventQueueLooperCallback;
    sp<::android::Looper> mLooper;
    sp<::android::SensorEventQueue> mInternalQueue;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace sensorservice
}  // namespace frameworks
}  // namespace android

#endif  // ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_EVENTQUEUE_H
