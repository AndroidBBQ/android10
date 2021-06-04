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

#ifndef A_SENSOR_MANAGER_H_

#define A_SENSOR_MANAGER_H_

#include <android-base/macros.h>
#include <android/frameworks/sensorservice/1.0/ISensorManager.h>
#include <android/sensor.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

struct ALooper;

struct ASensorManager {
    static ASensorManager *getInstance();

    ASensorManager();
    android::status_t initCheck() const;

    // Returns error or number of sensors returned.
    int getSensorList(ASensorList *list);

    ASensorRef getDefaultSensor(int type);
    ASensorRef getDefaultSensorEx(int type, bool wakeup);

    ASensorEventQueue *createEventQueue(
            ALooper *looper,
            int ident,
            ALooper_callbackFunc callback,
            void *data);

    void destroyEventQueue(ASensorEventQueue *queue);

private:

    struct SensorDeathRecipient : public android::hardware::hidl_death_recipient
    {
        // hidl_death_recipient interface
        virtual void serviceDied(uint64_t cookie,
                const ::android::wp<::android::hidl::base::V1_0::IBase>& who) override;
    };

    using ISensorManager = android::frameworks::sensorservice::V1_0::ISensorManager;
    using SensorInfo = android::hardware::sensors::V1_0::SensorInfo;

    static ASensorManager *sInstance;
    android::sp<SensorDeathRecipient> mDeathRecipient = nullptr;

    android::status_t mInitCheck;
    android::sp<ISensorManager> mManager;

    mutable android::Mutex mLock;
    android::hardware::hidl_vec<SensorInfo> mSensors;
    std::unique_ptr<ASensorRef[]> mSensorList;

    DISALLOW_COPY_AND_ASSIGN(ASensorManager);
};

#endif  // A_SENSOR_MANAGER_H_
