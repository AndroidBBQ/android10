/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <stdint.h>
#include <math.h>
#include <sys/types.h>

#include <cutils/atomic.h>
#include <utils/Errors.h>
#include <utils/Singleton.h>

#include <binder/BinderService.h>
#include <binder/Parcel.h>

#include "BatteryService.h"

namespace android {
// ---------------------------------------------------------------------------

BatteryService::BatteryService() : mBatteryStatService(nullptr) {
}

bool BatteryService::addSensor(uid_t uid, int handle) {
    Mutex::Autolock _l(mActivationsLock);
    Info key(uid, handle);
    ssize_t index = mActivations.indexOf(key);
    if (index < 0) {
        index = mActivations.add(key);
    }
    Info& info(mActivations.editItemAt(index));
    info.count++;
    return info.count == 1;
}

bool BatteryService::removeSensor(uid_t uid, int handle) {
    Mutex::Autolock _l(mActivationsLock);
    ssize_t index = mActivations.indexOf(Info(uid, handle));
    if (index < 0) return false;
    Info& info(mActivations.editItemAt(index));
    info.count--;
    return info.count == 0;
}


void BatteryService::enableSensorImpl(uid_t uid, int handle) {
    if (checkService()) {
        if (addSensor(uid, handle)) {
            int64_t identity = IPCThreadState::self()->clearCallingIdentity();
            mBatteryStatService->noteStartSensor(uid, handle);
            IPCThreadState::self()->restoreCallingIdentity(identity);
        }
    }
}
void BatteryService::disableSensorImpl(uid_t uid, int handle) {
    if (checkService()) {
        if (removeSensor(uid, handle)) {
            int64_t identity = IPCThreadState::self()->clearCallingIdentity();
            mBatteryStatService->noteStopSensor(uid, handle);
            IPCThreadState::self()->restoreCallingIdentity(identity);
        }
    }
}

void BatteryService::cleanupImpl(uid_t uid) {
    if (checkService()) {
        Mutex::Autolock _l(mActivationsLock);
        int64_t identity = IPCThreadState::self()->clearCallingIdentity();
        for (size_t i=0 ; i<mActivations.size() ; ) {
            const Info& info(mActivations[i]);
            if (info.uid == uid) {
                mBatteryStatService->noteStopSensor(info.uid, info.handle);
                mActivations.removeAt(i);
            } else {
              i++;
            }
        }
        IPCThreadState::self()->restoreCallingIdentity(identity);
    }
}

bool BatteryService::checkService() {
    if (mBatteryStatService == nullptr) {
        const sp<IServiceManager> sm(defaultServiceManager());
        if (sm != nullptr) {
            const String16 name("batterystats");
            mBatteryStatService = interface_cast<IBatteryStats>(sm->getService(name));
        }
    }
    return mBatteryStatService != nullptr;
}

ANDROID_SINGLETON_STATIC_INSTANCE(BatteryService)

// ---------------------------------------------------------------------------
}; // namespace android
