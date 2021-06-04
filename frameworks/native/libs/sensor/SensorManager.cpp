/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "Sensors"

#include <sensor/SensorManager.h>

#include <stdint.h>
#include <sys/types.h>

#include <cutils/native_handle.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Singleton.h>

#include <binder/IBinder.h>
#include <binder/IPermissionController.h>
#include <binder/IServiceManager.h>

#include <sensor/ISensorServer.h>
#include <sensor/ISensorEventConnection.h>
#include <sensor/Sensor.h>
#include <sensor/SensorEventQueue.h>

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

Mutex SensorManager::sLock;
std::map<String16, SensorManager*> SensorManager::sPackageInstances;

SensorManager& SensorManager::getInstanceForPackage(const String16& packageName) {
    waitForSensorService(nullptr);

    Mutex::Autolock _l(sLock);
    SensorManager* sensorManager;
    auto iterator = sPackageInstances.find(packageName);

    if (iterator != sPackageInstances.end()) {
        sensorManager = iterator->second;
    } else {
        String16 opPackageName = packageName;

        // It is possible that the calling code has no access to the package name.
        // In this case we will get the packages for the calling UID and pick the
        // first one for attributing the app op. This will work correctly for
        // runtime permissions as for legacy apps we will toggle the app op for
        // all packages in the UID. The caveat is that the operation may be attributed
        // to the wrong package and stats based on app ops may be slightly off.
        if (opPackageName.size() <= 0) {
            sp<IBinder> binder = defaultServiceManager()->getService(String16("permission"));
            if (binder != nullptr) {
                const uid_t uid = IPCThreadState::self()->getCallingUid();
                Vector<String16> packages;
                interface_cast<IPermissionController>(binder)->getPackagesForUid(uid, packages);
                if (!packages.isEmpty()) {
                    opPackageName = packages[0];
                } else {
                    ALOGE("No packages for calling UID");
                }
            } else {
                ALOGE("Cannot get permission service");
            }
        }

        sensorManager = new SensorManager(opPackageName);

        // If we had no package name, we looked it up from the UID and the sensor
        // manager instance we created should also be mapped to the empty package
        // name, to avoid looking up the packages for a UID and get the same result.
        if (packageName.size() <= 0) {
            sPackageInstances.insert(std::make_pair(String16(), sensorManager));
        }

        // Stash the per package sensor manager.
        sPackageInstances.insert(std::make_pair(opPackageName, sensorManager));
    }

    return *sensorManager;
}

SensorManager::SensorManager(const String16& opPackageName)
    : mSensorList(nullptr), mOpPackageName(opPackageName), mDirectConnectionHandle(1) {
    Mutex::Autolock _l(mLock);
    assertStateLocked();
}

SensorManager::~SensorManager() {
    free(mSensorList);
}

status_t SensorManager::waitForSensorService(sp<ISensorServer> *server) {
    // try for 300 seconds (60*5(getService() tries for 5 seconds)) before giving up ...
    sp<ISensorServer> s;
    const String16 name("sensorservice");
    for (int i = 0; i < 60; i++) {
        status_t err = getService(name, &s);
        switch (err) {
            case NAME_NOT_FOUND:
                sleep(1);
                continue;
            case NO_ERROR:
                if (server != nullptr) {
                    *server = s;
                }
                return NO_ERROR;
            default:
                return err;
        }
    }
    return TIMED_OUT;
}

void SensorManager::sensorManagerDied() {
    Mutex::Autolock _l(mLock);
    mSensorServer.clear();
    free(mSensorList);
    mSensorList = nullptr;
    mSensors.clear();
}

status_t SensorManager::assertStateLocked() {
    bool initSensorManager = false;
    if (mSensorServer == nullptr) {
        initSensorManager = true;
    } else {
        // Ping binder to check if sensorservice is alive.
        status_t err = IInterface::asBinder(mSensorServer)->pingBinder();
        if (err != NO_ERROR) {
            initSensorManager = true;
        }
    }
    if (initSensorManager) {
        waitForSensorService(&mSensorServer);
        LOG_ALWAYS_FATAL_IF(mSensorServer == nullptr, "getService(SensorService) NULL");

        class DeathObserver : public IBinder::DeathRecipient {
            SensorManager& mSensorManager;
            virtual void binderDied(const wp<IBinder>& who) {
                ALOGW("sensorservice died [%p]", who.unsafe_get());
                mSensorManager.sensorManagerDied();
            }
        public:
            explicit DeathObserver(SensorManager& mgr) : mSensorManager(mgr) { }
        };

        mDeathObserver = new DeathObserver(*const_cast<SensorManager *>(this));
        IInterface::asBinder(mSensorServer)->linkToDeath(mDeathObserver);

        mSensors = mSensorServer->getSensorList(mOpPackageName);
        size_t count = mSensors.size();
        mSensorList =
                static_cast<Sensor const**>(malloc(count * sizeof(Sensor*)));
        LOG_ALWAYS_FATAL_IF(mSensorList == nullptr, "mSensorList NULL");

        for (size_t i=0 ; i<count ; i++) {
            mSensorList[i] = mSensors.array() + i;
        }
    }

    return NO_ERROR;
}

ssize_t SensorManager::getSensorList(Sensor const* const** list) {
    Mutex::Autolock _l(mLock);
    status_t err = assertStateLocked();
    if (err < 0) {
        return static_cast<ssize_t>(err);
    }
    *list = mSensorList;
    return static_cast<ssize_t>(mSensors.size());
}

ssize_t SensorManager::getDynamicSensorList(Vector<Sensor> & dynamicSensors) {
    Mutex::Autolock _l(mLock);
    status_t err = assertStateLocked();
    if (err < 0) {
        return static_cast<ssize_t>(err);
    }

    dynamicSensors = mSensorServer->getDynamicSensorList(mOpPackageName);
    size_t count = dynamicSensors.size();

    return static_cast<ssize_t>(count);
}

Sensor const* SensorManager::getDefaultSensor(int type)
{
    Mutex::Autolock _l(mLock);
    if (assertStateLocked() == NO_ERROR) {
        bool wakeUpSensor = false;
        // For the following sensor types, return a wake-up sensor. These types are by default
        // defined as wake-up sensors. For the rest of the sensor types defined in sensors.h return
        // a non_wake-up version.
        if (type == SENSOR_TYPE_PROXIMITY || type == SENSOR_TYPE_SIGNIFICANT_MOTION ||
            type == SENSOR_TYPE_TILT_DETECTOR || type == SENSOR_TYPE_WAKE_GESTURE ||
            type == SENSOR_TYPE_GLANCE_GESTURE || type == SENSOR_TYPE_PICK_UP_GESTURE ||
            type == SENSOR_TYPE_WRIST_TILT_GESTURE ||
            type == SENSOR_TYPE_LOW_LATENCY_OFFBODY_DETECT) {
            wakeUpSensor = true;
        }
        // For now we just return the first sensor of that type we find.
        // in the future it will make sense to let the SensorService make
        // that decision.
        for (size_t i=0 ; i<mSensors.size() ; i++) {
            if (mSensorList[i]->getType() == type &&
                mSensorList[i]->isWakeUpSensor() == wakeUpSensor) {
                return mSensorList[i];
            }
        }
    }
    return nullptr;
}

sp<SensorEventQueue> SensorManager::createEventQueue(String8 packageName, int mode) {
    sp<SensorEventQueue> queue;

    Mutex::Autolock _l(mLock);
    while (assertStateLocked() == NO_ERROR) {
        sp<ISensorEventConnection> connection =
                mSensorServer->createSensorEventConnection(packageName, mode, mOpPackageName);
        if (connection == nullptr) {
            // SensorService just died or the app doesn't have required permissions.
            ALOGE("createEventQueue: connection is NULL.");
            return nullptr;
        }
        queue = new SensorEventQueue(connection);
        break;
    }
    return queue;
}

bool SensorManager::isDataInjectionEnabled() {
    Mutex::Autolock _l(mLock);
    if (assertStateLocked() == NO_ERROR) {
        return mSensorServer->isDataInjectionEnabled();
    }
    return false;
}

int SensorManager::createDirectChannel(
        size_t size, int channelType, const native_handle_t *resourceHandle) {
    Mutex::Autolock _l(mLock);
    if (assertStateLocked() != NO_ERROR) {
        return NO_INIT;
    }

    if (channelType != SENSOR_DIRECT_MEM_TYPE_ASHMEM
            && channelType != SENSOR_DIRECT_MEM_TYPE_GRALLOC) {
        ALOGE("Bad channel shared memory type %d", channelType);
        return BAD_VALUE;
    }

    sp<ISensorEventConnection> conn =
              mSensorServer->createSensorDirectConnection(mOpPackageName,
                  static_cast<uint32_t>(size),
                  static_cast<int32_t>(channelType),
                  SENSOR_DIRECT_FMT_SENSORS_EVENT, resourceHandle);
    if (conn == nullptr) {
        return NO_MEMORY;
    }

    int nativeHandle = mDirectConnectionHandle++;
    mDirectConnection.emplace(nativeHandle, conn);
    return nativeHandle;
}

void SensorManager::destroyDirectChannel(int channelNativeHandle) {
    Mutex::Autolock _l(mLock);
    if (assertStateLocked() == NO_ERROR) {
        mDirectConnection.erase(channelNativeHandle);
    }
}

int SensorManager::configureDirectChannel(int channelNativeHandle, int sensorHandle, int rateLevel) {
    Mutex::Autolock _l(mLock);
    if (assertStateLocked() != NO_ERROR) {
        return NO_INIT;
    }

    auto i = mDirectConnection.find(channelNativeHandle);
    if (i == mDirectConnection.end()) {
        ALOGE("Cannot find the handle in client direct connection table");
        return BAD_VALUE;
    }

    int ret;
    ret = i->second->configureChannel(sensorHandle, rateLevel);
    ALOGE_IF(ret < 0, "SensorManager::configureChannel (%d, %d) returns %d",
            static_cast<int>(sensorHandle), static_cast<int>(rateLevel),
            static_cast<int>(ret));
    return ret;
}

int SensorManager::setOperationParameter(
        int handle, int type,
        const Vector<float> &floats, const Vector<int32_t> &ints) {
    Mutex::Autolock _l(mLock);
    if (assertStateLocked() != NO_ERROR) {
        return NO_INIT;
    }
    return mSensorServer->setOperationParameter(handle, type, floats, ints);
}

// ----------------------------------------------------------------------------
}; // namespace android
