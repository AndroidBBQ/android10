/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <mutex>
#include <unistd.h>

#include <binder/Binder.h>
#include <binder/IServiceManager.h>
#include <sensorprivacy/SensorPrivacyManager.h>

#include <utils/SystemClock.h>

namespace android {

SensorPrivacyManager::SensorPrivacyManager()
{
}

sp<hardware::ISensorPrivacyManager> SensorPrivacyManager::getService()
{
    std::lock_guard<Mutex> scoped_lock(mLock);
    int64_t startTime = 0;
    sp<hardware::ISensorPrivacyManager> service = mService;
    while (service == nullptr || !IInterface::asBinder(service)->isBinderAlive()) {
        sp<IBinder> binder = defaultServiceManager()->checkService(String16("sensor_privacy"));
        if (binder == nullptr) {
            // Wait for the sensor privacy service to come back...
            if (startTime == 0) {
                startTime = uptimeMillis();
                ALOGI("Waiting for sensor privacy service");
            } else if ((uptimeMillis() - startTime) > 1000000) {
                ALOGW("Waiting too long for sensor privacy service, giving up");
                service = nullptr;
                break;
            }
            usleep(25000);
        } else {
            service = interface_cast<hardware::ISensorPrivacyManager>(binder);
            mService = service;
        }
    }
    return service;
}

void SensorPrivacyManager::addSensorPrivacyListener(
        const sp<hardware::ISensorPrivacyListener>& listener)
{
    sp<hardware::ISensorPrivacyManager> service = getService();
    if (service != nullptr) {
        service->addSensorPrivacyListener(listener);
    }
}

void SensorPrivacyManager::removeSensorPrivacyListener(
        const sp<hardware::ISensorPrivacyListener>& listener)
{
    sp<hardware::ISensorPrivacyManager> service = getService();
    if (service != nullptr) {
        service->removeSensorPrivacyListener(listener);
    }
}

bool SensorPrivacyManager::isSensorPrivacyEnabled()
{
    sp<hardware::ISensorPrivacyManager> service = getService();
    if (service != nullptr) {
        bool result;
        service->isSensorPrivacyEnabled(&result);
        return result;
    }
    // if the SensorPrivacyManager is not available then assume sensor privacy is disabled
    return false;
}

status_t SensorPrivacyManager::linkToDeath(const sp<IBinder::DeathRecipient>& recipient)
{
    sp<hardware::ISensorPrivacyManager> service = getService();
    if (service != nullptr) {
        return IInterface::asBinder(service)->linkToDeath(recipient);
    }
    return INVALID_OPERATION;
}

status_t SensorPrivacyManager::unlinkToDeath(const sp<IBinder::DeathRecipient>& recipient)
{
    sp<hardware::ISensorPrivacyManager> service = getService();
    if (service != nullptr) {
        return IInterface::asBinder(service)->unlinkToDeath(recipient);
    }
    return INVALID_OPERATION;
}

}; // namespace android
