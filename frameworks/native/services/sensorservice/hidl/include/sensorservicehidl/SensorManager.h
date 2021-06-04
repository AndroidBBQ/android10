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

#ifndef ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_SENSORMANAGER_H
#define ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_SENSORMANAGER_H

#include <jni.h>

#include <mutex>
#include <thread>

#include <android/frameworks/sensorservice/1.0/ISensorManager.h>
#include <android/frameworks/sensorservice/1.0/types.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <sensor/SensorManager.h>
#include <utils/Looper.h>

namespace android {
namespace frameworks {
namespace sensorservice {
namespace V1_0 {
namespace implementation {

using ::android::hardware::sensors::V1_0::SensorType;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_memory;
using ::android::hardware::Return;
using ::android::sp;

struct SensorManager final : public ISensorManager {

    explicit SensorManager(JavaVM* vm);
    ~SensorManager();

    // Methods from ::android::frameworks::sensorservice::V1_0::ISensorManager follow.
    Return<void> getSensorList(getSensorList_cb _hidl_cb) override;
    Return<void> getDefaultSensor(SensorType type, getDefaultSensor_cb _hidl_cb) override;
    Return<void> createAshmemDirectChannel(const hidl_memory& mem, uint64_t size, createAshmemDirectChannel_cb _hidl_cb) override;
    Return<void> createGrallocDirectChannel(const hidl_handle& buffer, uint64_t size, createGrallocDirectChannel_cb _hidl_cb) override;
    Return<void> createEventQueue(const sp<IEventQueueCallback> &callback, createEventQueue_cb _hidl_cb);

private:
    // Block until ::android::SensorManager is initialized.
    ::android::SensorManager& getInternalManager();
    sp<Looper> getLooper();

    std::mutex mInternalManagerMutex;
    ::android::SensorManager* mInternalManager = nullptr; // does not own
    sp<Looper> mLooper;

    volatile bool mStopThread;
    std::mutex mThreadMutex; //protects mPollThread
    std::thread mPollThread;

    JavaVM* mJavaVm;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace sensorservice
}  // namespace frameworks
}  // namespace android

#endif  // ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_SENSORMANAGER_H
