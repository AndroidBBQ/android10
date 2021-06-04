/*
 * Copyright (C) 2013 The Android Open Source Project
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
#include <binder/AppOpsManager.h>
#include <binder/Binder.h>
#include <binder/IServiceManager.h>

#include <utils/SystemClock.h>

namespace android {

namespace {

#if defined(__BRILLO__)
// Because Brillo has no application model, security policy is managed
// statically (at build time) with SELinux controls.
// As a consequence, it also never runs the AppOpsManager service.
const int APP_OPS_MANAGER_UNAVAILABLE_MODE = AppOpsManager::MODE_ALLOWED;
#else
const int APP_OPS_MANAGER_UNAVAILABLE_MODE = AppOpsManager::MODE_IGNORED;
#endif  // defined(__BRILLO__)

}  // namespace

static String16 _appops("appops");
static pthread_mutex_t gTokenMutex = PTHREAD_MUTEX_INITIALIZER;
static sp<IBinder> gToken;

static const sp<IBinder>& getToken(const sp<IAppOpsService>& service) {
    pthread_mutex_lock(&gTokenMutex);
    if (gToken == nullptr || gToken->pingBinder() != NO_ERROR) {
        gToken = service->getToken(new BBinder());
    }
    pthread_mutex_unlock(&gTokenMutex);
    return gToken;
}

AppOpsManager::AppOpsManager()
{
}

#if defined(__BRILLO__)
// There is no AppOpsService on Brillo
sp<IAppOpsService> AppOpsManager::getService() { return NULL; }
#else
sp<IAppOpsService> AppOpsManager::getService()
{

    std::lock_guard<Mutex> scoped_lock(mLock);
    int64_t startTime = 0;
    sp<IAppOpsService> service = mService;
    while (service == nullptr || !IInterface::asBinder(service)->isBinderAlive()) {
        sp<IBinder> binder = defaultServiceManager()->checkService(_appops);
        if (binder == nullptr) {
            // Wait for the app ops service to come back...
            if (startTime == 0) {
                startTime = uptimeMillis();
                ALOGI("Waiting for app ops service");
            } else if ((uptimeMillis()-startTime) > 10000) {
                ALOGW("Waiting too long for app ops service, giving up");
                service = nullptr;
                break;
            }
            sleep(1);
        } else {
            service = interface_cast<IAppOpsService>(binder);
            mService = service;
        }
    }
    return service;
}
#endif  // defined(__BRILLO__)

int32_t AppOpsManager::checkOp(int32_t op, int32_t uid, const String16& callingPackage)
{
    sp<IAppOpsService> service = getService();
    return service != nullptr
            ? service->checkOperation(op, uid, callingPackage)
            : APP_OPS_MANAGER_UNAVAILABLE_MODE;
}

int32_t AppOpsManager::checkAudioOpNoThrow(int32_t op, int32_t usage, int32_t uid,
        const String16& callingPackage) {
    sp<IAppOpsService> service = getService();
    return service != nullptr
           ? service->checkAudioOperation(op, usage, uid, callingPackage)
           : APP_OPS_MANAGER_UNAVAILABLE_MODE;
}

int32_t AppOpsManager::noteOp(int32_t op, int32_t uid, const String16& callingPackage) {
    sp<IAppOpsService> service = getService();
    return service != nullptr
            ? service->noteOperation(op, uid, callingPackage)
            : APP_OPS_MANAGER_UNAVAILABLE_MODE;
}

int32_t AppOpsManager::startOpNoThrow(int32_t op, int32_t uid, const String16& callingPackage,
        bool startIfModeDefault) {
    sp<IAppOpsService> service = getService();
    return service != nullptr
            ? service->startOperation(getToken(service), op, uid, callingPackage,
                    startIfModeDefault) : APP_OPS_MANAGER_UNAVAILABLE_MODE;
}

void AppOpsManager::finishOp(int32_t op, int32_t uid, const String16& callingPackage) {
    sp<IAppOpsService> service = getService();
    if (service != nullptr) {
        service->finishOperation(getToken(service), op, uid, callingPackage);
    }
}

void AppOpsManager::startWatchingMode(int32_t op, const String16& packageName,
        const sp<IAppOpsCallback>& callback) {
    sp<IAppOpsService> service = getService();
    if (service != nullptr) {
        service->startWatchingMode(op, packageName, callback);
    }
}

void AppOpsManager::stopWatchingMode(const sp<IAppOpsCallback>& callback) {
    sp<IAppOpsService> service = getService();
    if (service != nullptr) {
        service->stopWatchingMode(callback);
    }
}

int32_t AppOpsManager::permissionToOpCode(const String16& permission) {
    sp<IAppOpsService> service = getService();
    if (service != nullptr) {
        return service->permissionToOpCode(permission);
    }
    return -1;
}


}; // namespace android
