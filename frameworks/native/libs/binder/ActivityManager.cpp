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

#include <mutex>
#include <unistd.h>

#include <binder/ActivityManager.h>
#include <binder/Binder.h>
#include <binder/IServiceManager.h>

#include <utils/SystemClock.h>

namespace android {

ActivityManager::ActivityManager()
{
}

sp<IActivityManager> ActivityManager::getService()
{
    std::lock_guard<Mutex> scoped_lock(mLock);
    int64_t startTime = 0;
    sp<IActivityManager> service = mService;
    while (service == nullptr || !IInterface::asBinder(service)->isBinderAlive()) {
        sp<IBinder> binder = defaultServiceManager()->checkService(String16("activity"));
        if (binder == nullptr) {
            // Wait for the activity service to come back...
            if (startTime == 0) {
                startTime = uptimeMillis();
                ALOGI("Waiting for activity service");
            } else if ((uptimeMillis() - startTime) > 1000000) {
                ALOGW("Waiting too long for activity service, giving up");
                service = nullptr;
                break;
            }
            usleep(25000);
        } else {
            service = interface_cast<IActivityManager>(binder);
            mService = service;
        }
    }
    return service;
}

int ActivityManager::openContentUri(const String16& stringUri)
{
    sp<IActivityManager> service = getService();
    return service != nullptr ? service->openContentUri(stringUri) : -1;
}

void ActivityManager::registerUidObserver(const sp<IUidObserver>& observer,
                                          const int32_t event,
                                          const int32_t cutpoint,
                                          const String16& callingPackage)
{
    sp<IActivityManager> service = getService();
    if (service != nullptr) {
        service->registerUidObserver(observer, event, cutpoint, callingPackage);
    }
}

void ActivityManager::unregisterUidObserver(const sp<IUidObserver>& observer)
{
    sp<IActivityManager> service = getService();
    if (service != nullptr) {
        service->unregisterUidObserver(observer);
    }
}

bool ActivityManager::isUidActive(const uid_t uid, const String16& callingPackage)
{
    sp<IActivityManager> service = getService();
    if (service != nullptr) {
        return service->isUidActive(uid, callingPackage);
    }
    return false;
}

int32_t ActivityManager::getUidProcessState(const uid_t uid, const String16& callingPackage)
{
    sp<IActivityManager> service = getService();
    if (service != nullptr) {
        return service->getUidProcessState(uid, callingPackage);
    }
    return PROCESS_STATE_UNKNOWN;
}

status_t ActivityManager::linkToDeath(const sp<IBinder::DeathRecipient>& recipient) {
    sp<IActivityManager> service = getService();
    if (service != nullptr) {
        return IInterface::asBinder(service)->linkToDeath(recipient);
    }
    return INVALID_OPERATION;
}

status_t ActivityManager::unlinkToDeath(const sp<IBinder::DeathRecipient>& recipient) {
    sp<IActivityManager> service = getService();
    if (service != nullptr) {
        return IInterface::asBinder(service)->unlinkToDeath(recipient);
    }
    return INVALID_OPERATION;
}

}; // namespace android
