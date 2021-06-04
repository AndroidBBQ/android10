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
#include <binder/PermissionController.h>
#include <binder/Binder.h>
#include <binder/IServiceManager.h>

#include <utils/SystemClock.h>

namespace android {

PermissionController::PermissionController()
{
}

sp<IPermissionController> PermissionController::getService()
{
    std::lock_guard<Mutex> scoped_lock(mLock);
    int64_t startTime = 0;
    sp<IPermissionController> service = mService;
    while (service == nullptr || !IInterface::asBinder(service)->isBinderAlive()) {
        sp<IBinder> binder = defaultServiceManager()->checkService(String16("permission"));
        if (binder == nullptr) {
            // Wait for the activity service to come back...
            if (startTime == 0) {
                startTime = uptimeMillis();
                ALOGI("Waiting for permission service");
            } else if ((uptimeMillis() - startTime) > 10000) {
                ALOGW("Waiting too long for permission service, giving up");
                service = nullptr;
                break;
            }
            sleep(1);
        } else {
            service = interface_cast<IPermissionController>(binder);
            mService = service;
        }
    }
    return service;
}

bool PermissionController::checkPermission(const String16& permission, int32_t pid, int32_t uid)
{
    sp<IPermissionController> service = getService();
    return service != nullptr ? service->checkPermission(permission, pid, uid) : false;
}

int32_t PermissionController::noteOp(const String16& op, int32_t uid, const String16& packageName)
{
    sp<IPermissionController> service = getService();
    return service != nullptr ? service->noteOp(op, uid, packageName) : MODE_ERRORED;
}

void PermissionController::getPackagesForUid(const uid_t uid, Vector<String16> &packages)
{
    sp<IPermissionController> service = getService();
    if (service != nullptr) {
        service->getPackagesForUid(uid, packages);
    }
}

bool PermissionController::isRuntimePermission(const String16& permission)
{
    sp<IPermissionController> service = getService();
    return service != nullptr ? service->isRuntimePermission(permission) : false;
}

int PermissionController::getPackageUid(const String16& package, int flags)
{
    sp<IPermissionController> service = getService();
    return service != nullptr ? service->getPackageUid(package, flags) : -1;
}

}; // namespace android
