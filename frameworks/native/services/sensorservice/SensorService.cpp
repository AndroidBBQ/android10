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
#include <android/content/pm/IPackageManagerNative.h>
#include <binder/ActivityManager.h>
#include <binder/BinderService.h>
#include <binder/IServiceManager.h>
#include <binder/PermissionCache.h>
#include <binder/PermissionController.h>
#include <cutils/ashmem.h>
#include <cutils/misc.h>
#include <cutils/properties.h>
#include <hardware/sensors.h>
#include <hardware_legacy/power.h>
#include <log/log.h>
#include <openssl/digest.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <sensor/SensorEventQueue.h>
#include <sensorprivacy/SensorPrivacyManager.h>
#include <utils/SystemClock.h>

#include "BatteryService.h"
#include "CorrectedGyroSensor.h"
#include "GravitySensor.h"
#include "LinearAccelerationSensor.h"
#include "OrientationSensor.h"
#include "RotationVectorSensor.h"
#include "SensorFusion.h"
#include "SensorInterface.h"

#include "SensorService.h"
#include "SensorDirectConnection.h"
#include "SensorEventAckReceiver.h"
#include "SensorEventConnection.h"
#include "SensorRecord.h"
#include "SensorRegistrationInfo.h"

#include <ctime>
#include <inttypes.h>
#include <math.h>
#include <sched.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <private/android_filesystem_config.h>

namespace android {
// ---------------------------------------------------------------------------

/*
 * Notes:
 *
 * - what about a gyro-corrected magnetic-field sensor?
 * - run mag sensor from time to time to force calibration
 * - gravity sensor length is wrong (=> drift in linear-acc sensor)
 *
 */

const char* SensorService::WAKE_LOCK_NAME = "SensorService_wakelock";
uint8_t SensorService::sHmacGlobalKey[128] = {};
bool SensorService::sHmacGlobalKeyIsValid = false;
std::map<String16, int> SensorService::sPackageTargetVersion;
Mutex SensorService::sPackageTargetVersionLock;
AppOpsManager SensorService::sAppOpsManager;

#define SENSOR_SERVICE_DIR "/data/system/sensor_service"
#define SENSOR_SERVICE_HMAC_KEY_FILE  SENSOR_SERVICE_DIR "/hmac_key"
#define SENSOR_SERVICE_SCHED_FIFO_PRIORITY 10

// Permissions.
static const String16 sDumpPermission("android.permission.DUMP");
static const String16 sLocationHardwarePermission("android.permission.LOCATION_HARDWARE");
static const String16 sManageSensorsPermission("android.permission.MANAGE_SENSORS");

SensorService::SensorService()
    : mInitCheck(NO_INIT), mSocketBufferSize(SOCKET_BUFFER_SIZE_NON_BATCHED),
      mWakeLockAcquired(false) {
    mUidPolicy = new UidPolicy(this);
    mSensorPrivacyPolicy = new SensorPrivacyPolicy(this);
}

bool SensorService::initializeHmacKey() {
    int fd = open(SENSOR_SERVICE_HMAC_KEY_FILE, O_RDONLY|O_CLOEXEC);
    if (fd != -1) {
        int result = read(fd, sHmacGlobalKey, sizeof(sHmacGlobalKey));
        close(fd);
        if (result == sizeof(sHmacGlobalKey)) {
            return true;
        }
        ALOGW("Unable to read HMAC key; generating new one.");
    }

    if (RAND_bytes(sHmacGlobalKey, sizeof(sHmacGlobalKey)) == -1) {
        ALOGW("Can't generate HMAC key; dynamic sensor getId() will be wrong.");
        return false;
    }

    // We need to make sure this is only readable to us.
    bool wroteKey = false;
    mkdir(SENSOR_SERVICE_DIR, S_IRWXU);
    fd = open(SENSOR_SERVICE_HMAC_KEY_FILE, O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,
              S_IRUSR|S_IWUSR);
    if (fd != -1) {
        int result = write(fd, sHmacGlobalKey, sizeof(sHmacGlobalKey));
        close(fd);
        wroteKey = (result == sizeof(sHmacGlobalKey));
    }
    if (wroteKey) {
        ALOGI("Generated new HMAC key.");
    } else {
        ALOGW("Unable to write HMAC key; dynamic sensor getId() will change "
              "after reboot.");
    }
    // Even if we failed to write the key we return true, because we did
    // initialize the HMAC key.
    return true;
}

// Set main thread to SCHED_FIFO to lower sensor event latency when system is under load
void SensorService::enableSchedFifoMode() {
    struct sched_param param = {0};
    param.sched_priority = SENSOR_SERVICE_SCHED_FIFO_PRIORITY;
    if (sched_setscheduler(getTid(), SCHED_FIFO | SCHED_RESET_ON_FORK, &param) != 0) {
        ALOGE("Couldn't set SCHED_FIFO for SensorService thread");
    }
}

void SensorService::onFirstRef() {
    ALOGD("nuSensorService starting...");
    SensorDevice& dev(SensorDevice::getInstance());

    sHmacGlobalKeyIsValid = initializeHmacKey();

    if (dev.initCheck() == NO_ERROR) {
        sensor_t const* list;
        ssize_t count = dev.getSensorList(&list);
        if (count > 0) {
            ssize_t orientationIndex = -1;
            bool hasGyro = false, hasAccel = false, hasMag = false;
            uint32_t virtualSensorsNeeds =
                    (1<<SENSOR_TYPE_GRAVITY) |
                    (1<<SENSOR_TYPE_LINEAR_ACCELERATION) |
                    (1<<SENSOR_TYPE_ROTATION_VECTOR) |
                    (1<<SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR) |
                    (1<<SENSOR_TYPE_GAME_ROTATION_VECTOR);

            for (ssize_t i=0 ; i<count ; i++) {
                bool useThisSensor=true;

                switch (list[i].type) {
                    case SENSOR_TYPE_ACCELEROMETER:
                        hasAccel = true;
                        break;
                    case SENSOR_TYPE_MAGNETIC_FIELD:
                        hasMag = true;
                        break;
                    case SENSOR_TYPE_ORIENTATION:
                        orientationIndex = i;
                        break;
                    case SENSOR_TYPE_GYROSCOPE:
                    case SENSOR_TYPE_GYROSCOPE_UNCALIBRATED:
                        hasGyro = true;
                        break;
                    case SENSOR_TYPE_GRAVITY:
                    case SENSOR_TYPE_LINEAR_ACCELERATION:
                    case SENSOR_TYPE_ROTATION_VECTOR:
                    case SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR:
                    case SENSOR_TYPE_GAME_ROTATION_VECTOR:
                        if (IGNORE_HARDWARE_FUSION) {
                            useThisSensor = false;
                        } else {
                            virtualSensorsNeeds &= ~(1<<list[i].type);
                        }
                        break;
                }
                if (useThisSensor) {
                    registerSensor( new HardwareSensor(list[i]) );
                }
            }

            // it's safe to instantiate the SensorFusion object here
            // (it wants to be instantiated after h/w sensors have been
            // registered)
            SensorFusion::getInstance();

            if (hasGyro && hasAccel && hasMag) {
                // Add Android virtual sensors if they're not already
                // available in the HAL
                bool needRotationVector =
                        (virtualSensorsNeeds & (1<<SENSOR_TYPE_ROTATION_VECTOR)) != 0;

                registerSensor(new RotationVectorSensor(), !needRotationVector, true);
                registerSensor(new OrientationSensor(), !needRotationVector, true);

                // virtual debugging sensors are not for user
                registerSensor( new CorrectedGyroSensor(list, count), true, true);
                registerSensor( new GyroDriftSensor(), true, true);
            }

            if (hasAccel && hasGyro) {
                bool needGravitySensor = (virtualSensorsNeeds & (1<<SENSOR_TYPE_GRAVITY)) != 0;
                registerSensor(new GravitySensor(list, count), !needGravitySensor, true);

                bool needLinearAcceleration =
                        (virtualSensorsNeeds & (1<<SENSOR_TYPE_LINEAR_ACCELERATION)) != 0;
                registerSensor(new LinearAccelerationSensor(list, count),
                               !needLinearAcceleration, true);

                bool needGameRotationVector =
                        (virtualSensorsNeeds & (1<<SENSOR_TYPE_GAME_ROTATION_VECTOR)) != 0;
                registerSensor(new GameRotationVectorSensor(), !needGameRotationVector, true);
            }

            if (hasAccel && hasMag) {
                bool needGeoMagRotationVector =
                        (virtualSensorsNeeds & (1<<SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR)) != 0;
                registerSensor(new GeoMagRotationVectorSensor(), !needGeoMagRotationVector, true);
            }

            // Check if the device really supports batching by looking at the FIFO event
            // counts for each sensor.
            bool batchingSupported = false;
            mSensors.forEachSensor(
                    [&batchingSupported] (const Sensor& s) -> bool {
                        if (s.getFifoMaxEventCount() > 0) {
                            batchingSupported = true;
                        }
                        return !batchingSupported;
                    });

            if (batchingSupported) {
                // Increase socket buffer size to a max of 100 KB for batching capabilities.
                mSocketBufferSize = MAX_SOCKET_BUFFER_SIZE_BATCHED;
            } else {
                mSocketBufferSize = SOCKET_BUFFER_SIZE_NON_BATCHED;
            }

            // Compare the socketBufferSize value against the system limits and limit
            // it to maxSystemSocketBufferSize if necessary.
            FILE *fp = fopen("/proc/sys/net/core/wmem_max", "r");
            char line[128];
            if (fp != nullptr && fgets(line, sizeof(line), fp) != nullptr) {
                line[sizeof(line) - 1] = '\0';
                size_t maxSystemSocketBufferSize;
                sscanf(line, "%zu", &maxSystemSocketBufferSize);
                if (mSocketBufferSize > maxSystemSocketBufferSize) {
                    mSocketBufferSize = maxSystemSocketBufferSize;
                }
            }
            if (fp) {
                fclose(fp);
            }

            mWakeLockAcquired = false;
            mLooper = new Looper(false);
            const size_t minBufferSize = SensorEventQueue::MAX_RECEIVE_BUFFER_EVENT_COUNT;
            mSensorEventBuffer = new sensors_event_t[minBufferSize];
            mSensorEventScratch = new sensors_event_t[minBufferSize];
            mMapFlushEventsToConnections = new wp<const SensorEventConnection> [minBufferSize];
            mCurrentOperatingMode = NORMAL;

            mNextSensorRegIndex = 0;
            for (int i = 0; i < SENSOR_REGISTRATIONS_BUF_SIZE; ++i) {
                mLastNSensorRegistrations.push();
            }

            mInitCheck = NO_ERROR;
            mAckReceiver = new SensorEventAckReceiver(this);
            mAckReceiver->run("SensorEventAckReceiver", PRIORITY_URGENT_DISPLAY);
            run("SensorService", PRIORITY_URGENT_DISPLAY);

            // priority can only be changed after run
            enableSchedFifoMode();

            // Start watching UID changes to apply policy.
            mUidPolicy->registerSelf();

            // Start watching sensor privacy changes
            mSensorPrivacyPolicy->registerSelf();
        }
    }
}

void SensorService::setSensorAccess(uid_t uid, bool hasAccess) {
    ConnectionSafeAutolock connLock = mConnectionHolder.lock(mLock);
    for (const sp<SensorEventConnection>& conn : connLock.getActiveConnections()) {
        if (conn->getUid() == uid) {
            conn->setSensorAccess(hasAccess);
        }
    }
}

const Sensor& SensorService::registerSensor(SensorInterface* s, bool isDebug, bool isVirtual) {
    int handle = s->getSensor().getHandle();
    int type = s->getSensor().getType();
    if (mSensors.add(handle, s, isDebug, isVirtual)){
        mRecentEvent.emplace(handle, new SensorServiceUtil::RecentEventLogger(type));
        return s->getSensor();
    } else {
        return mSensors.getNonSensor();
    }
}

const Sensor& SensorService::registerDynamicSensorLocked(SensorInterface* s, bool isDebug) {
    return registerSensor(s, isDebug);
}

bool SensorService::unregisterDynamicSensorLocked(int handle) {
    bool ret = mSensors.remove(handle);

    const auto i = mRecentEvent.find(handle);
    if (i != mRecentEvent.end()) {
        delete i->second;
        mRecentEvent.erase(i);
    }
    return ret;
}

const Sensor& SensorService::registerVirtualSensor(SensorInterface* s, bool isDebug) {
    return registerSensor(s, isDebug, true);
}

SensorService::~SensorService() {
    for (auto && entry : mRecentEvent) {
        delete entry.second;
    }
    mUidPolicy->unregisterSelf();
    mSensorPrivacyPolicy->unregisterSelf();
}

status_t SensorService::dump(int fd, const Vector<String16>& args) {
    String8 result;
    if (!PermissionCache::checkCallingPermission(sDumpPermission)) {
        result.appendFormat("Permission Denial: can't dump SensorService from pid=%d, uid=%d\n",
                IPCThreadState::self()->getCallingPid(),
                IPCThreadState::self()->getCallingUid());
    } else {
        bool privileged = IPCThreadState::self()->getCallingUid() == 0;
        if (args.size() > 2) {
           return INVALID_OPERATION;
        }
        ConnectionSafeAutolock connLock = mConnectionHolder.lock(mLock);
        SensorDevice& dev(SensorDevice::getInstance());
        if (args.size() == 2 && args[0] == String16("restrict")) {
            // If already in restricted mode. Ignore.
            if (mCurrentOperatingMode == RESTRICTED) {
                return status_t(NO_ERROR);
            }
            // If in any mode other than normal, ignore.
            if (mCurrentOperatingMode != NORMAL) {
                return INVALID_OPERATION;
            }

            mCurrentOperatingMode = RESTRICTED;
            // temporarily stop all sensor direct report and disable sensors
            disableAllSensorsLocked(&connLock);
            mWhiteListedPackage.setTo(String8(args[1]));
            return status_t(NO_ERROR);
        } else if (args.size() == 1 && args[0] == String16("enable")) {
            // If currently in restricted mode, reset back to NORMAL mode else ignore.
            if (mCurrentOperatingMode == RESTRICTED) {
                mCurrentOperatingMode = NORMAL;
                // enable sensors and recover all sensor direct report
                enableAllSensorsLocked(&connLock);
            }
            if (mCurrentOperatingMode == DATA_INJECTION) {
               resetToNormalModeLocked();
            }
            mWhiteListedPackage.clear();
            return status_t(NO_ERROR);
        } else if (args.size() == 2 && args[0] == String16("data_injection")) {
            if (mCurrentOperatingMode == NORMAL) {
                dev.disableAllSensors();
                status_t err = dev.setMode(DATA_INJECTION);
                if (err == NO_ERROR) {
                    mCurrentOperatingMode = DATA_INJECTION;
                } else {
                    // Re-enable sensors.
                    dev.enableAllSensors();
                }
                mWhiteListedPackage.setTo(String8(args[1]));
                return NO_ERROR;
            } else if (mCurrentOperatingMode == DATA_INJECTION) {
                // Already in DATA_INJECTION mode. Treat this as a no_op.
                return NO_ERROR;
            } else {
                // Transition to data injection mode supported only from NORMAL mode.
                return INVALID_OPERATION;
            }
        } else if (!mSensors.hasAnySensor()) {
            result.append("No Sensors on the device\n");
            result.appendFormat("devInitCheck : %d\n", SensorDevice::getInstance().initCheck());
        } else {
            // Default dump the sensor list and debugging information.
            //
            timespec curTime;
            clock_gettime(CLOCK_REALTIME, &curTime);
            struct tm* timeinfo = localtime(&(curTime.tv_sec));
            result.appendFormat("Captured at: %02d:%02d:%02d.%03d\n", timeinfo->tm_hour,
                                timeinfo->tm_min, timeinfo->tm_sec, (int)ns2ms(curTime.tv_nsec));
            result.append("Sensor Device:\n");
            result.append(SensorDevice::getInstance().dump().c_str());

            result.append("Sensor List:\n");
            result.append(mSensors.dump().c_str());

            result.append("Fusion States:\n");
            SensorFusion::getInstance().dump(result);

            result.append("Recent Sensor events:\n");
            for (auto&& i : mRecentEvent) {
                sp<SensorInterface> s = mSensors.getInterface(i.first);
                if (!i.second->isEmpty()) {
                    if (privileged || s->getSensor().getRequiredPermission().isEmpty()) {
                        i.second->setFormat("normal");
                    } else {
                        i.second->setFormat("mask_data");
                    }
                    // if there is events and sensor does not need special permission.
                    result.appendFormat("%s: ", s->getSensor().getName().string());
                    result.append(i.second->dump().c_str());
                }
            }

            result.append("Active sensors:\n");
            SensorDevice& dev = SensorDevice::getInstance();
            for (size_t i=0 ; i<mActiveSensors.size() ; i++) {
                int handle = mActiveSensors.keyAt(i);
                if (dev.isSensorActive(handle)) {
                    result.appendFormat("%s (handle=0x%08x, connections=%zu)\n",
                            getSensorName(handle).string(),
                            handle,
                            mActiveSensors.valueAt(i)->getNumConnections());
                }
            }

            result.appendFormat("Socket Buffer size = %zd events\n",
                                mSocketBufferSize/sizeof(sensors_event_t));
            result.appendFormat("WakeLock Status: %s \n", mWakeLockAcquired ? "acquired" :
                    "not held");
            result.appendFormat("Mode :");
            switch(mCurrentOperatingMode) {
               case NORMAL:
                   result.appendFormat(" NORMAL\n");
                   break;
               case RESTRICTED:
                   result.appendFormat(" RESTRICTED : %s\n", mWhiteListedPackage.string());
                   break;
               case DATA_INJECTION:
                   result.appendFormat(" DATA_INJECTION : %s\n", mWhiteListedPackage.string());
            }
            result.appendFormat("Sensor Privacy: %s\n",
                    mSensorPrivacyPolicy->isSensorPrivacyEnabled() ? "enabled" : "disabled");

            const auto& activeConnections = connLock.getActiveConnections();
            result.appendFormat("%zd active connections\n", activeConnections.size());
            for (size_t i=0 ; i < activeConnections.size() ; i++) {
                result.appendFormat("Connection Number: %zu \n", i);
                activeConnections[i]->dump(result);
            }

            const auto& directConnections = connLock.getDirectConnections();
            result.appendFormat("%zd direct connections\n", directConnections.size());
            for (size_t i = 0 ; i < directConnections.size() ; i++) {
                result.appendFormat("Direct connection %zu:\n", i);
                directConnections[i]->dump(result);
            }

            result.appendFormat("Previous Registrations:\n");
            // Log in the reverse chronological order.
            int currentIndex = (mNextSensorRegIndex - 1 + SENSOR_REGISTRATIONS_BUF_SIZE) %
                SENSOR_REGISTRATIONS_BUF_SIZE;
            const int startIndex = currentIndex;
            do {
                const SensorRegistrationInfo& reg_info = mLastNSensorRegistrations[currentIndex];
                if (SensorRegistrationInfo::isSentinel(reg_info)) {
                    // Ignore sentinel, proceed to next item.
                    currentIndex = (currentIndex - 1 + SENSOR_REGISTRATIONS_BUF_SIZE) %
                        SENSOR_REGISTRATIONS_BUF_SIZE;
                    continue;
                }
                result.appendFormat("%s\n", reg_info.dump().c_str());
                currentIndex = (currentIndex - 1 + SENSOR_REGISTRATIONS_BUF_SIZE) %
                        SENSOR_REGISTRATIONS_BUF_SIZE;
            } while(startIndex != currentIndex);
        }
    }
    write(fd, result.string(), result.size());
    return NO_ERROR;
}

void SensorService::disableAllSensors() {
    ConnectionSafeAutolock connLock = mConnectionHolder.lock(mLock);
    disableAllSensorsLocked(&connLock);
}

void SensorService::disableAllSensorsLocked(ConnectionSafeAutolock* connLock) {
    SensorDevice& dev(SensorDevice::getInstance());
    for (const sp<SensorDirectConnection>& connection : connLock->getDirectConnections()) {
        connection->stopAll(true /* backupRecord */);
    }
    dev.disableAllSensors();
    // Clear all pending flush connections for all active sensors. If one of the active
    // connections has called flush() and the underlying sensor has been disabled before a
    // flush complete event is returned, we need to remove the connection from this queue.
    for (size_t i=0 ; i< mActiveSensors.size(); ++i) {
        mActiveSensors.valueAt(i)->clearAllPendingFlushConnections();
    }
}

void SensorService::enableAllSensors() {
    ConnectionSafeAutolock connLock = mConnectionHolder.lock(mLock);
    enableAllSensorsLocked(&connLock);
}

void SensorService::enableAllSensorsLocked(ConnectionSafeAutolock* connLock) {
    // sensors should only be enabled if the operating state is not restricted and sensor
    // privacy is not enabled.
    if (mCurrentOperatingMode == RESTRICTED || mSensorPrivacyPolicy->isSensorPrivacyEnabled()) {
        ALOGW("Sensors cannot be enabled: mCurrentOperatingMode = %d, sensor privacy = %s",
              mCurrentOperatingMode,
              mSensorPrivacyPolicy->isSensorPrivacyEnabled() ? "enabled" : "disabled");
        return;
    }
    SensorDevice& dev(SensorDevice::getInstance());
    dev.enableAllSensors();
    for (const sp<SensorDirectConnection>& connection : connLock->getDirectConnections()) {
        connection->recoverAll();
    }
}


// NOTE: This is a remote API - make sure all args are validated
status_t SensorService::shellCommand(int in, int out, int err, Vector<String16>& args) {
    if (!checkCallingPermission(sManageSensorsPermission, nullptr, nullptr)) {
        return PERMISSION_DENIED;
    }
    if (in == BAD_TYPE || out == BAD_TYPE || err == BAD_TYPE) {
        return BAD_VALUE;
    }
    if (args[0] == String16("set-uid-state")) {
        return handleSetUidState(args, err);
    } else if (args[0] == String16("reset-uid-state")) {
        return handleResetUidState(args, err);
    } else if (args[0] == String16("get-uid-state")) {
        return handleGetUidState(args, out, err);
    } else if (args.size() == 1 && args[0] == String16("help")) {
        printHelp(out);
        return NO_ERROR;
    }
    printHelp(err);
    return BAD_VALUE;
}

static status_t getUidForPackage(String16 packageName, int userId, /*inout*/uid_t& uid, int err) {
    PermissionController pc;
    uid = pc.getPackageUid(packageName, 0);
    if (uid <= 0) {
        ALOGE("Unknown package: '%s'", String8(packageName).string());
        dprintf(err, "Unknown package: '%s'\n", String8(packageName).string());
        return BAD_VALUE;
    }

    if (userId < 0) {
        ALOGE("Invalid user: %d", userId);
        dprintf(err, "Invalid user: %d\n", userId);
        return BAD_VALUE;
    }

    uid = multiuser_get_uid(userId, uid);
    return NO_ERROR;
}

status_t SensorService::handleSetUidState(Vector<String16>& args, int err) {
    // Valid arg.size() is 3 or 5, args.size() is 5 with --user option.
    if (!(args.size() == 3 || args.size() == 5)) {
        printHelp(err);
        return BAD_VALUE;
    }

    bool active = false;
    if (args[2] == String16("active")) {
        active = true;
    } else if ((args[2] != String16("idle"))) {
        ALOGE("Expected active or idle but got: '%s'", String8(args[2]).string());
        return BAD_VALUE;
    }

    int userId = 0;
    if (args.size() == 5 && args[3] == String16("--user")) {
        userId = atoi(String8(args[4]));
    }

    uid_t uid;
    if (getUidForPackage(args[1], userId, uid, err) != NO_ERROR) {
        return BAD_VALUE;
    }

    mUidPolicy->addOverrideUid(uid, active);
    return NO_ERROR;
}

status_t SensorService::handleResetUidState(Vector<String16>& args, int err) {
    // Valid arg.size() is 2 or 4, args.size() is 4 with --user option.
    if (!(args.size() == 2 || args.size() == 4)) {
        printHelp(err);
        return BAD_VALUE;
    }

    int userId = 0;
    if (args.size() == 4 && args[2] == String16("--user")) {
        userId = atoi(String8(args[3]));
    }

    uid_t uid;
    if (getUidForPackage(args[1], userId, uid, err) == BAD_VALUE) {
        return BAD_VALUE;
    }

    mUidPolicy->removeOverrideUid(uid);
    return NO_ERROR;
}

status_t SensorService::handleGetUidState(Vector<String16>& args, int out, int err) {
    // Valid arg.size() is 2 or 4, args.size() is 4 with --user option.
    if (!(args.size() == 2 || args.size() == 4)) {
        printHelp(err);
        return BAD_VALUE;
    }

    int userId = 0;
    if (args.size() == 4 && args[2] == String16("--user")) {
        userId = atoi(String8(args[3]));
    }

    uid_t uid;
    if (getUidForPackage(args[1], userId, uid, err) == BAD_VALUE) {
        return BAD_VALUE;
    }

    if (mUidPolicy->isUidActive(uid)) {
        return dprintf(out, "active\n");
    } else {
        return dprintf(out, "idle\n");
    }
}

status_t SensorService::printHelp(int out) {
    return dprintf(out, "Sensor service commands:\n"
        "  get-uid-state <PACKAGE> [--user USER_ID] gets the uid state\n"
        "  set-uid-state <PACKAGE> <active|idle> [--user USER_ID] overrides the uid state\n"
        "  reset-uid-state <PACKAGE> [--user USER_ID] clears the uid state override\n"
        "  help print this message\n");
}

//TODO: move to SensorEventConnection later
void SensorService::cleanupAutoDisabledSensorLocked(const sp<SensorEventConnection>& connection,
        sensors_event_t const* buffer, const int count) {
    for (int i=0 ; i<count ; i++) {
        int handle = buffer[i].sensor;
        if (buffer[i].type == SENSOR_TYPE_META_DATA) {
            handle = buffer[i].meta_data.sensor;
        }
        if (connection->hasSensor(handle)) {
            sp<SensorInterface> si = getSensorInterfaceFromHandle(handle);
            // If this buffer has an event from a one_shot sensor and this connection is registered
            // for this particular one_shot sensor, try cleaning up the connection.
            if (si != nullptr &&
                si->getSensor().getReportingMode() == AREPORTING_MODE_ONE_SHOT) {
                si->autoDisable(connection.get(), handle);
                cleanupWithoutDisableLocked(connection, handle);
            }

        }
   }
}

bool SensorService::threadLoop() {
    ALOGD("nuSensorService thread starting...");

    // each virtual sensor could generate an event per "real" event, that's why we need to size
    // numEventMax much smaller than MAX_RECEIVE_BUFFER_EVENT_COUNT.  in practice, this is too
    // aggressive, but guaranteed to be enough.
    const size_t vcount = mSensors.getVirtualSensors().size();
    const size_t minBufferSize = SensorEventQueue::MAX_RECEIVE_BUFFER_EVENT_COUNT;
    const size_t numEventMax = minBufferSize / (1 + vcount);

    SensorDevice& device(SensorDevice::getInstance());

    const int halVersion = device.getHalDeviceVersion();
    do {
        ssize_t count = device.poll(mSensorEventBuffer, numEventMax);
        if (count < 0) {
            if(count == DEAD_OBJECT && device.isReconnecting()) {
                device.reconnect();
                continue;
            } else {
                ALOGE("sensor poll failed (%s)", strerror(-count));
                break;
            }
        }

        // Reset sensors_event_t.flags to zero for all events in the buffer.
        for (int i = 0; i < count; i++) {
             mSensorEventBuffer[i].flags = 0;
        }
        ConnectionSafeAutolock connLock = mConnectionHolder.lock(mLock);

        // Poll has returned. Hold a wakelock if one of the events is from a wake up sensor. The
        // rest of this loop is under a critical section protected by mLock. Acquiring a wakeLock,
        // sending events to clients (incrementing SensorEventConnection::mWakeLockRefCount) should
        // not be interleaved with decrementing SensorEventConnection::mWakeLockRefCount and
        // releasing the wakelock.
        uint32_t wakeEvents = 0;
        for (int i = 0; i < count; i++) {
            if (isWakeUpSensorEvent(mSensorEventBuffer[i])) {
                wakeEvents++;
            }
        }

        if (wakeEvents > 0) {
            if (!mWakeLockAcquired) {
                setWakeLockAcquiredLocked(true);
            }
            device.writeWakeLockHandled(wakeEvents);
        }
        recordLastValueLocked(mSensorEventBuffer, count);

        // handle virtual sensors
        if (count && vcount) {
            sensors_event_t const * const event = mSensorEventBuffer;
            if (!mActiveVirtualSensors.empty()) {
                size_t k = 0;
                SensorFusion& fusion(SensorFusion::getInstance());
                if (fusion.isEnabled()) {
                    for (size_t i=0 ; i<size_t(count) ; i++) {
                        fusion.process(event[i]);
                    }
                }
                for (size_t i=0 ; i<size_t(count) && k<minBufferSize ; i++) {
                    for (int handle : mActiveVirtualSensors) {
                        if (count + k >= minBufferSize) {
                            ALOGE("buffer too small to hold all events: "
                                    "count=%zd, k=%zu, size=%zu",
                                    count, k, minBufferSize);
                            break;
                        }
                        sensors_event_t out;
                        sp<SensorInterface> si = mSensors.getInterface(handle);
                        if (si == nullptr) {
                            ALOGE("handle %d is not an valid virtual sensor", handle);
                            continue;
                        }

                        if (si->process(&out, event[i])) {
                            mSensorEventBuffer[count + k] = out;
                            k++;
                        }
                    }
                }
                if (k) {
                    // record the last synthesized values
                    recordLastValueLocked(&mSensorEventBuffer[count], k);
                    count += k;
                    // sort the buffer by time-stamps
                    sortEventBuffer(mSensorEventBuffer, count);
                }
            }
        }

        // handle backward compatibility for RotationVector sensor
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_0) {
            for (int i = 0; i < count; i++) {
                if (mSensorEventBuffer[i].type == SENSOR_TYPE_ROTATION_VECTOR) {
                    // All the 4 components of the quaternion should be available
                    // No heading accuracy. Set it to -1
                    mSensorEventBuffer[i].data[4] = -1;
                }
            }
        }

        // Cache the list of active connections, since we use it in multiple places below but won't
        // modify it here
        const std::vector<sp<SensorEventConnection>> activeConnections = connLock.getActiveConnections();

        for (int i = 0; i < count; ++i) {
            // Map flush_complete_events in the buffer to SensorEventConnections which called flush
            // on the hardware sensor. mapFlushEventsToConnections[i] will be the
            // SensorEventConnection mapped to the corresponding flush_complete_event in
            // mSensorEventBuffer[i] if such a mapping exists (NULL otherwise).
            mMapFlushEventsToConnections[i] = nullptr;
            if (mSensorEventBuffer[i].type == SENSOR_TYPE_META_DATA) {
                const int sensor_handle = mSensorEventBuffer[i].meta_data.sensor;
                SensorRecord* rec = mActiveSensors.valueFor(sensor_handle);
                if (rec != nullptr) {
                    mMapFlushEventsToConnections[i] = rec->getFirstPendingFlushConnection();
                    rec->removeFirstPendingFlushConnection();
                }
            }

            // handle dynamic sensor meta events, process registration and unregistration of dynamic
            // sensor based on content of event.
            if (mSensorEventBuffer[i].type == SENSOR_TYPE_DYNAMIC_SENSOR_META) {
                if (mSensorEventBuffer[i].dynamic_sensor_meta.connected) {
                    int handle = mSensorEventBuffer[i].dynamic_sensor_meta.handle;
                    const sensor_t& dynamicSensor =
                            *(mSensorEventBuffer[i].dynamic_sensor_meta.sensor);
                    ALOGI("Dynamic sensor handle 0x%x connected, type %d, name %s",
                          handle, dynamicSensor.type, dynamicSensor.name);

                    if (mSensors.isNewHandle(handle)) {
                        const auto& uuid = mSensorEventBuffer[i].dynamic_sensor_meta.uuid;
                        sensor_t s = dynamicSensor;
                        // make sure the dynamic sensor flag is set
                        s.flags |= DYNAMIC_SENSOR_MASK;
                        // force the handle to be consistent
                        s.handle = handle;

                        SensorInterface *si = new HardwareSensor(s, uuid);

                        // This will release hold on dynamic sensor meta, so it should be called
                        // after Sensor object is created.
                        device.handleDynamicSensorConnection(handle, true /*connected*/);
                        registerDynamicSensorLocked(si);
                    } else {
                        ALOGE("Handle %d has been used, cannot use again before reboot.", handle);
                    }
                } else {
                    int handle = mSensorEventBuffer[i].dynamic_sensor_meta.handle;
                    ALOGI("Dynamic sensor handle 0x%x disconnected", handle);

                    device.handleDynamicSensorConnection(handle, false /*connected*/);
                    if (!unregisterDynamicSensorLocked(handle)) {
                        ALOGE("Dynamic sensor release error.");
                    }

                    for (const sp<SensorEventConnection>& connection : activeConnections) {
                        connection->removeSensor(handle);
                    }
                }
            }
        }

        // Send our events to clients. Check the state of wake lock for each client and release the
        // lock if none of the clients need it.
        bool needsWakeLock = false;
        for (const sp<SensorEventConnection>& connection : activeConnections) {
            connection->sendEvents(mSensorEventBuffer, count, mSensorEventScratch,
                    mMapFlushEventsToConnections);
            needsWakeLock |= connection->needsWakeLock();
            // If the connection has one-shot sensors, it may be cleaned up after first trigger.
            // Early check for one-shot sensors.
            if (connection->hasOneShotSensors()) {
                cleanupAutoDisabledSensorLocked(connection, mSensorEventBuffer, count);
            }
        }

        if (mWakeLockAcquired && !needsWakeLock) {
            setWakeLockAcquiredLocked(false);
        }
    } while (!Thread::exitPending());

    ALOGW("Exiting SensorService::threadLoop => aborting...");
    abort();
    return false;
}

sp<Looper> SensorService::getLooper() const {
    return mLooper;
}

void SensorService::resetAllWakeLockRefCounts() {
    ConnectionSafeAutolock connLock = mConnectionHolder.lock(mLock);
    for (const sp<SensorEventConnection>& connection : connLock.getActiveConnections()) {
        connection->resetWakeLockRefCount();
    }
    setWakeLockAcquiredLocked(false);
}

void SensorService::setWakeLockAcquiredLocked(bool acquire) {
    if (acquire) {
        if (!mWakeLockAcquired) {
            acquire_wake_lock(PARTIAL_WAKE_LOCK, WAKE_LOCK_NAME);
            mWakeLockAcquired = true;
        }
        mLooper->wake();
    } else {
        if (mWakeLockAcquired) {
            release_wake_lock(WAKE_LOCK_NAME);
            mWakeLockAcquired = false;
        }
    }
}

bool SensorService::isWakeLockAcquired() {
    Mutex::Autolock _l(mLock);
    return mWakeLockAcquired;
}

bool SensorService::SensorEventAckReceiver::threadLoop() {
    ALOGD("new thread SensorEventAckReceiver");
    sp<Looper> looper = mService->getLooper();
    do {
        bool wakeLockAcquired = mService->isWakeLockAcquired();
        int timeout = -1;
        if (wakeLockAcquired) timeout = 5000;
        int ret = looper->pollOnce(timeout);
        if (ret == ALOOPER_POLL_TIMEOUT) {
           mService->resetAllWakeLockRefCounts();
        }
    } while(!Thread::exitPending());
    return false;
}

void SensorService::recordLastValueLocked(
        const sensors_event_t* buffer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (buffer[i].type == SENSOR_TYPE_META_DATA ||
            buffer[i].type == SENSOR_TYPE_DYNAMIC_SENSOR_META ||
            buffer[i].type == SENSOR_TYPE_ADDITIONAL_INFO) {
            continue;
        }

        auto logger = mRecentEvent.find(buffer[i].sensor);
        if (logger != mRecentEvent.end()) {
            logger->second->addEvent(buffer[i]);
        }
    }
}

void SensorService::sortEventBuffer(sensors_event_t* buffer, size_t count) {
    struct compar {
        static int cmp(void const* lhs, void const* rhs) {
            sensors_event_t const* l = static_cast<sensors_event_t const*>(lhs);
            sensors_event_t const* r = static_cast<sensors_event_t const*>(rhs);
            return l->timestamp - r->timestamp;
        }
    };
    qsort(buffer, count, sizeof(sensors_event_t), compar::cmp);
}

String8 SensorService::getSensorName(int handle) const {
    return mSensors.getName(handle);
}

bool SensorService::isVirtualSensor(int handle) const {
    sp<SensorInterface> sensor = getSensorInterfaceFromHandle(handle);
    return sensor != nullptr && sensor->isVirtual();
}

bool SensorService::isWakeUpSensorEvent(const sensors_event_t& event) const {
    int handle = event.sensor;
    if (event.type == SENSOR_TYPE_META_DATA) {
        handle = event.meta_data.sensor;
    }
    sp<SensorInterface> sensor = getSensorInterfaceFromHandle(handle);
    return sensor != nullptr && sensor->getSensor().isWakeUpSensor();
}

int32_t SensorService::getIdFromUuid(const Sensor::uuid_t &uuid) const {
    if ((uuid.i64[0] == 0) && (uuid.i64[1] == 0)) {
        // UUID is not supported for this device.
        return 0;
    }
    if ((uuid.i64[0] == INT64_C(~0)) && (uuid.i64[1] == INT64_C(~0))) {
        // This sensor can be uniquely identified in the system by
        // the combination of its type and name.
        return -1;
    }

    // We have a dynamic sensor.

    if (!sHmacGlobalKeyIsValid) {
        // Rather than risk exposing UUIDs, we cripple dynamic sensors.
        ALOGW("HMAC key failure; dynamic sensor getId() will be wrong.");
        return 0;
    }

    // We want each app author/publisher to get a different ID, so that the
    // same dynamic sensor cannot be tracked across apps by multiple
    // authors/publishers.  So we use both our UUID and our User ID.
    // Note potential confusion:
    //     UUID => Universally Unique Identifier.
    //     UID  => User Identifier.
    // We refrain from using "uid" except as needed by API to try to
    // keep this distinction clear.

    auto appUserId = IPCThreadState::self()->getCallingUid();
    uint8_t uuidAndApp[sizeof(uuid) + sizeof(appUserId)];
    memcpy(uuidAndApp, &uuid, sizeof(uuid));
    memcpy(uuidAndApp + sizeof(uuid), &appUserId, sizeof(appUserId));

    // Now we use our key on our UUID/app combo to get the hash.
    uint8_t hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;
    if (HMAC(EVP_sha256(),
             sHmacGlobalKey, sizeof(sHmacGlobalKey),
             uuidAndApp, sizeof(uuidAndApp),
             hash, &hashLen) == nullptr) {
        // Rather than risk exposing UUIDs, we cripple dynamic sensors.
        ALOGW("HMAC failure; dynamic sensor getId() will be wrong.");
        return 0;
    }

    int32_t id = 0;
    if (hashLen < sizeof(id)) {
        // We never expect this case, but out of paranoia, we handle it.
        // Our 'id' length is already quite small, we don't want the
        // effective length of it to be even smaller.
        // Rather than risk exposing UUIDs, we cripple dynamic sensors.
        ALOGW("HMAC insufficient; dynamic sensor getId() will be wrong.");
        return 0;
    }

    // This is almost certainly less than all of 'hash', but it's as secure
    // as we can be with our current 'id' length.
    memcpy(&id, hash, sizeof(id));

    // Note at the beginning of the function that we return the values of
    // 0 and -1 to represent special cases.  As a result, we can't return
    // those as dynamic sensor IDs.  If we happened to hash to one of those
    // values, we change 'id' so we report as a dynamic sensor, and not as
    // one of those special cases.
    if (id == -1) {
        id = -2;
    } else if (id == 0) {
        id = 1;
    }
    return id;
}

void SensorService::makeUuidsIntoIdsForSensorList(Vector<Sensor> &sensorList) const {
    for (auto &sensor : sensorList) {
        int32_t id = getIdFromUuid(sensor.getUuid());
        sensor.setId(id);
    }
}

Vector<Sensor> SensorService::getSensorList(const String16& /* opPackageName */) {
    char value[PROPERTY_VALUE_MAX];
    property_get("debug.sensors", value, "0");
    const Vector<Sensor>& initialSensorList = (atoi(value)) ?
            mSensors.getUserDebugSensors() : mSensors.getUserSensors();
    Vector<Sensor> accessibleSensorList;
    for (size_t i = 0; i < initialSensorList.size(); i++) {
        Sensor sensor = initialSensorList[i];
        accessibleSensorList.add(sensor);
    }
    makeUuidsIntoIdsForSensorList(accessibleSensorList);
    return accessibleSensorList;
}

Vector<Sensor> SensorService::getDynamicSensorList(const String16& opPackageName) {
    Vector<Sensor> accessibleSensorList;
    mSensors.forEachSensor(
            [&opPackageName, &accessibleSensorList] (const Sensor& sensor) -> bool {
                if (sensor.isDynamicSensor()) {
                    if (canAccessSensor(sensor, "getDynamicSensorList", opPackageName)) {
                        accessibleSensorList.add(sensor);
                    } else {
                        ALOGI("Skipped sensor %s because it requires permission %s and app op %" PRId32,
                              sensor.getName().string(),
                              sensor.getRequiredPermission().string(),
                              sensor.getRequiredAppOp());
                    }
                }
                return true;
            });
    makeUuidsIntoIdsForSensorList(accessibleSensorList);
    return accessibleSensorList;
}

sp<ISensorEventConnection> SensorService::createSensorEventConnection(const String8& packageName,
        int requestedMode, const String16& opPackageName) {
    // Only 2 modes supported for a SensorEventConnection ... NORMAL and DATA_INJECTION.
    if (requestedMode != NORMAL && requestedMode != DATA_INJECTION) {
        return nullptr;
    }

    Mutex::Autolock _l(mLock);
    // To create a client in DATA_INJECTION mode to inject data, SensorService should already be
    // operating in DI mode.
    if (requestedMode == DATA_INJECTION) {
        if (mCurrentOperatingMode != DATA_INJECTION) return nullptr;
        if (!isWhiteListedPackage(packageName)) return nullptr;
    }

    uid_t uid = IPCThreadState::self()->getCallingUid();
    pid_t pid = IPCThreadState::self()->getCallingPid();

    String8 connPackageName =
            (packageName == "") ? String8::format("unknown_package_pid_%d", pid) : packageName;
    String16 connOpPackageName =
            (opPackageName == String16("")) ? String16(connPackageName) : opPackageName;
    bool hasSensorAccess = mUidPolicy->isUidActive(uid);
    sp<SensorEventConnection> result(new SensorEventConnection(this, uid, connPackageName,
            requestedMode == DATA_INJECTION, connOpPackageName, hasSensorAccess));
    if (requestedMode == DATA_INJECTION) {
        mConnectionHolder.addEventConnectionIfNotPresent(result);
        // Add the associated file descriptor to the Looper for polling whenever there is data to
        // be injected.
        result->updateLooperRegistration(mLooper);
    }
    return result;
}

int SensorService::isDataInjectionEnabled() {
    Mutex::Autolock _l(mLock);
    return (mCurrentOperatingMode == DATA_INJECTION);
}

sp<ISensorEventConnection> SensorService::createSensorDirectConnection(
        const String16& opPackageName, uint32_t size, int32_t type, int32_t format,
        const native_handle *resource) {
    ConnectionSafeAutolock connLock = mConnectionHolder.lock(mLock);

    // No new direct connections are allowed when sensor privacy is enabled
    if (mSensorPrivacyPolicy->isSensorPrivacyEnabled()) {
        ALOGE("Cannot create new direct connections when sensor privacy is enabled");
        return nullptr;
    }

    struct sensors_direct_mem_t mem = {
        .type = type,
        .format = format,
        .size = size,
        .handle = resource,
    };
    uid_t uid = IPCThreadState::self()->getCallingUid();

    if (mem.handle == nullptr) {
        ALOGE("Failed to clone resource handle");
        return nullptr;
    }

    // check format
    if (format != SENSOR_DIRECT_FMT_SENSORS_EVENT) {
        ALOGE("Direct channel format %d is unsupported!", format);
        return nullptr;
    }

    // check for duplication
    for (const sp<SensorDirectConnection>& connection : connLock.getDirectConnections()) {
        if (connection->isEquivalent(&mem)) {
            ALOGE("Duplicate create channel request for the same share memory");
            return nullptr;
        }
    }

    // check specific to memory type
    switch(type) {
        case SENSOR_DIRECT_MEM_TYPE_ASHMEM: { // channel backed by ashmem
            if (resource->numFds < 1) {
                ALOGE("Ashmem direct channel requires a memory region to be supplied");
                android_errorWriteLog(0x534e4554, "70986337");  // SafetyNet
                return nullptr;
            }
            int fd = resource->data[0];
            int size2 = ashmem_get_size_region(fd);
            // check size consistency
            if (size2 < static_cast<int64_t>(size)) {
                ALOGE("Ashmem direct channel size %" PRIu32 " greater than shared memory size %d",
                      size, size2);
                return nullptr;
            }
            break;
        }
        case SENSOR_DIRECT_MEM_TYPE_GRALLOC:
            // no specific checks for gralloc
            break;
        default:
            ALOGE("Unknown direct connection memory type %d", type);
            return nullptr;
    }

    native_handle_t *clone = native_handle_clone(resource);
    if (!clone) {
        return nullptr;
    }

    sp<SensorDirectConnection> conn;
    SensorDevice& dev(SensorDevice::getInstance());
    int channelHandle = dev.registerDirectChannel(&mem);

    if (channelHandle <= 0) {
        ALOGE("SensorDevice::registerDirectChannel returns %d", channelHandle);
    } else {
        mem.handle = clone;
        conn = new SensorDirectConnection(this, uid, &mem, channelHandle, opPackageName);
    }

    if (conn == nullptr) {
        native_handle_close(clone);
        native_handle_delete(clone);
    } else {
        // add to list of direct connections
        // sensor service should never hold pointer or sp of SensorDirectConnection object.
        mConnectionHolder.addDirectConnection(conn);
    }
    return conn;
}

int SensorService::setOperationParameter(
            int32_t handle, int32_t type,
            const Vector<float> &floats, const Vector<int32_t> &ints) {
    Mutex::Autolock _l(mLock);

    if (!checkCallingPermission(sLocationHardwarePermission, nullptr, nullptr)) {
        return PERMISSION_DENIED;
    }

    bool isFloat = true;
    bool isCustom = false;
    size_t expectSize = INT32_MAX;
    switch (type) {
        case AINFO_LOCAL_GEOMAGNETIC_FIELD:
            isFloat = true;
            expectSize = 3;
            break;
        case AINFO_LOCAL_GRAVITY:
            isFloat = true;
            expectSize = 1;
            break;
        case AINFO_DOCK_STATE:
        case AINFO_HIGH_PERFORMANCE_MODE:
        case AINFO_MAGNETIC_FIELD_CALIBRATION:
            isFloat = false;
            expectSize = 1;
            break;
        default:
            // CUSTOM events must only contain float data; it may have variable size
            if (type < AINFO_CUSTOM_START || type >= AINFO_DEBUGGING_START ||
                    ints.size() ||
                    sizeof(additional_info_event_t::data_float)/sizeof(float) < floats.size() ||
                    handle < 0) {
                return BAD_VALUE;
            }
            isFloat = true;
            isCustom = true;
            expectSize = floats.size();
            break;
    }

    if (!isCustom && handle != -1) {
        return BAD_VALUE;
    }

    // three events: first one is begin tag, last one is end tag, the one in the middle
    // is the payload.
    sensors_event_t event[3];
    int64_t timestamp = elapsedRealtimeNano();
    for (sensors_event_t* i = event; i < event + 3; i++) {
        *i = (sensors_event_t) {
            .version = sizeof(sensors_event_t),
            .sensor = handle,
            .type = SENSOR_TYPE_ADDITIONAL_INFO,
            .timestamp = timestamp++,
            .additional_info = (additional_info_event_t) {
                .serial = 0
            }
        };
    }

    event[0].additional_info.type = AINFO_BEGIN;
    event[1].additional_info.type = type;
    event[2].additional_info.type = AINFO_END;

    if (isFloat) {
        if (floats.size() != expectSize) {
            return BAD_VALUE;
        }
        for (size_t i = 0; i < expectSize; ++i) {
            event[1].additional_info.data_float[i] = floats[i];
        }
    } else {
        if (ints.size() != expectSize) {
            return BAD_VALUE;
        }
        for (size_t i = 0; i < expectSize; ++i) {
            event[1].additional_info.data_int32[i] = ints[i];
        }
    }

    SensorDevice& dev(SensorDevice::getInstance());
    for (sensors_event_t* i = event; i < event + 3; i++) {
        int ret = dev.injectSensorData(i);
        if (ret != NO_ERROR) {
            return ret;
        }
    }
    return NO_ERROR;
}

status_t SensorService::resetToNormalMode() {
    Mutex::Autolock _l(mLock);
    return resetToNormalModeLocked();
}

status_t SensorService::resetToNormalModeLocked() {
    SensorDevice& dev(SensorDevice::getInstance());
    status_t err = dev.setMode(NORMAL);
    if (err == NO_ERROR) {
        mCurrentOperatingMode = NORMAL;
        dev.enableAllSensors();
    }
    return err;
}

void SensorService::cleanupConnection(SensorEventConnection* c) {
    ConnectionSafeAutolock connLock = mConnectionHolder.lock(mLock);
    const wp<SensorEventConnection> connection(c);
    size_t size = mActiveSensors.size();
    ALOGD_IF(DEBUG_CONNECTIONS, "%zu active sensors", size);
    for (size_t i=0 ; i<size ; ) {
        int handle = mActiveSensors.keyAt(i);
        if (c->hasSensor(handle)) {
            ALOGD_IF(DEBUG_CONNECTIONS, "%zu: disabling handle=0x%08x", i, handle);
            sp<SensorInterface> sensor = getSensorInterfaceFromHandle(handle);
            if (sensor != nullptr) {
                sensor->activate(c, false);
            } else {
                ALOGE("sensor interface of handle=0x%08x is null!", handle);
            }
            c->removeSensor(handle);
        }
        SensorRecord* rec = mActiveSensors.valueAt(i);
        ALOGE_IF(!rec, "mActiveSensors[%zu] is null (handle=0x%08x)!", i, handle);
        ALOGD_IF(DEBUG_CONNECTIONS,
                "removing connection %p for sensor[%zu].handle=0x%08x",
                c, i, handle);

        if (rec && rec->removeConnection(connection)) {
            ALOGD_IF(DEBUG_CONNECTIONS, "... and it was the last connection");
            mActiveSensors.removeItemsAt(i, 1);
            mActiveVirtualSensors.erase(handle);
            delete rec;
            size--;
        } else {
            i++;
        }
    }
    c->updateLooperRegistration(mLooper);
    mConnectionHolder.removeEventConnection(connection);
    BatteryService::cleanup(c->getUid());
    if (c->needsWakeLock()) {
        checkWakeLockStateLocked(&connLock);
    }

    {
        Mutex::Autolock packageLock(sPackageTargetVersionLock);
        auto iter = sPackageTargetVersion.find(c->mOpPackageName);
        if (iter != sPackageTargetVersion.end()) {
            sPackageTargetVersion.erase(iter);
        }
    }

    SensorDevice& dev(SensorDevice::getInstance());
    dev.notifyConnectionDestroyed(c);
}

void SensorService::cleanupConnection(SensorDirectConnection* c) {
    Mutex::Autolock _l(mLock);

    SensorDevice& dev(SensorDevice::getInstance());
    dev.unregisterDirectChannel(c->getHalChannelHandle());
    mConnectionHolder.removeDirectConnection(c);
}

sp<SensorInterface> SensorService::getSensorInterfaceFromHandle(int handle) const {
    return mSensors.getInterface(handle);
}

status_t SensorService::enable(const sp<SensorEventConnection>& connection,
        int handle, nsecs_t samplingPeriodNs, nsecs_t maxBatchReportLatencyNs, int reservedFlags,
        const String16& opPackageName) {
    if (mInitCheck != NO_ERROR)
        return mInitCheck;

    sp<SensorInterface> sensor = getSensorInterfaceFromHandle(handle);
    if (sensor == nullptr ||
        !canAccessSensor(sensor->getSensor(), "Tried enabling", opPackageName)) {
        return BAD_VALUE;
    }

    ConnectionSafeAutolock connLock = mConnectionHolder.lock(mLock);
    if (mCurrentOperatingMode != NORMAL
           && !isWhiteListedPackage(connection->getPackageName())) {
        return INVALID_OPERATION;
    }

    SensorRecord* rec = mActiveSensors.valueFor(handle);
    if (rec == nullptr) {
        rec = new SensorRecord(connection);
        mActiveSensors.add(handle, rec);
        if (sensor->isVirtual()) {
            mActiveVirtualSensors.emplace(handle);
        }

        // There was no SensorRecord for this sensor which means it was previously disabled. Mark
        // the recent event as stale to ensure that the previous event is not sent to a client. This
        // ensures on-change events that were generated during a previous sensor activation are not
        // erroneously sent to newly connected clients, especially if a second client registers for
        // an on-change sensor before the first client receives the updated event. Once an updated
        // event is received, the recent events will be marked as current, and any new clients will
        // immediately receive the most recent event.
        if (sensor->getSensor().getReportingMode() == AREPORTING_MODE_ON_CHANGE) {
            auto logger = mRecentEvent.find(handle);
            if (logger != mRecentEvent.end()) {
                logger->second->setLastEventStale();
            }
        }
    } else {
        if (rec->addConnection(connection)) {
            // this sensor is already activated, but we are adding a connection that uses it.
            // Immediately send down the last known value of the requested sensor if it's not a
            // "continuous" sensor.
            if (sensor->getSensor().getReportingMode() == AREPORTING_MODE_ON_CHANGE) {
                // NOTE: The wake_up flag of this event may get set to
                // WAKE_UP_SENSOR_EVENT_NEEDS_ACK if this is a wake_up event.

                auto logger = mRecentEvent.find(handle);
                if (logger != mRecentEvent.end()) {
                    sensors_event_t event;
                    // Verify that the last sensor event was generated from the current activation
                    // of the sensor. If not, it is possible for an on-change sensor to receive a
                    // sensor event that is stale if two clients re-activate the sensor
                    // simultaneously.
                    if(logger->second->populateLastEventIfCurrent(&event)) {
                        event.sensor = handle;
                        if (event.version == sizeof(sensors_event_t)) {
                            if (isWakeUpSensorEvent(event) && !mWakeLockAcquired) {
                                setWakeLockAcquiredLocked(true);
                            }
                            connection->sendEvents(&event, 1, nullptr);
                            if (!connection->needsWakeLock() && mWakeLockAcquired) {
                                checkWakeLockStateLocked(&connLock);
                            }
                        }
                    }
                }
            }
        }
    }

    if (connection->addSensor(handle)) {
        BatteryService::enableSensor(connection->getUid(), handle);
        // the sensor was added (which means it wasn't already there)
        // so, see if this connection becomes active
        mConnectionHolder.addEventConnectionIfNotPresent(connection);
    } else {
        ALOGW("sensor %08x already enabled in connection %p (ignoring)",
            handle, connection.get());
    }

    // Check maximum delay for the sensor.
    nsecs_t maxDelayNs = sensor->getSensor().getMaxDelay() * 1000LL;
    if (maxDelayNs > 0 && (samplingPeriodNs > maxDelayNs)) {
        samplingPeriodNs = maxDelayNs;
    }

    nsecs_t minDelayNs = sensor->getSensor().getMinDelayNs();
    if (samplingPeriodNs < minDelayNs) {
        samplingPeriodNs = minDelayNs;
    }

    ALOGD_IF(DEBUG_CONNECTIONS, "Calling batch handle==%d flags=%d"
                                "rate=%" PRId64 " timeout== %" PRId64"",
             handle, reservedFlags, samplingPeriodNs, maxBatchReportLatencyNs);

    status_t err = sensor->batch(connection.get(), handle, 0, samplingPeriodNs,
                                 maxBatchReportLatencyNs);

    // Call flush() before calling activate() on the sensor. Wait for a first
    // flush complete event before sending events on this connection. Ignore
    // one-shot sensors which don't support flush(). Ignore on-change sensors
    // to maintain the on-change logic (any on-change events except the initial
    // one should be trigger by a change in value). Also if this sensor isn't
    // already active, don't call flush().
    if (err == NO_ERROR &&
            sensor->getSensor().getReportingMode() == AREPORTING_MODE_CONTINUOUS &&
            rec->getNumConnections() > 1) {
        connection->setFirstFlushPending(handle, true);
        status_t err_flush = sensor->flush(connection.get(), handle);
        // Flush may return error if the underlying h/w sensor uses an older HAL.
        if (err_flush == NO_ERROR) {
            rec->addPendingFlushConnection(connection.get());
        } else {
            connection->setFirstFlushPending(handle, false);
        }
    }

    if (err == NO_ERROR) {
        ALOGD_IF(DEBUG_CONNECTIONS, "Calling activate on %d", handle);
        err = sensor->activate(connection.get(), true);
    }

    if (err == NO_ERROR) {
        connection->updateLooperRegistration(mLooper);

        if (sensor->getSensor().getRequiredPermission().size() > 0 &&
                sensor->getSensor().getRequiredAppOp() >= 0) {
            connection->mHandleToAppOp[handle] = sensor->getSensor().getRequiredAppOp();
        }

        mLastNSensorRegistrations.editItemAt(mNextSensorRegIndex) =
                SensorRegistrationInfo(handle, connection->getPackageName(),
                                       samplingPeriodNs, maxBatchReportLatencyNs, true);
        mNextSensorRegIndex = (mNextSensorRegIndex + 1) % SENSOR_REGISTRATIONS_BUF_SIZE;
    }

    if (err != NO_ERROR) {
        // batch/activate has failed, reset our state.
        cleanupWithoutDisableLocked(connection, handle);
    }
    return err;
}

status_t SensorService::disable(const sp<SensorEventConnection>& connection, int handle) {
    if (mInitCheck != NO_ERROR)
        return mInitCheck;

    Mutex::Autolock _l(mLock);
    status_t err = cleanupWithoutDisableLocked(connection, handle);
    if (err == NO_ERROR) {
        sp<SensorInterface> sensor = getSensorInterfaceFromHandle(handle);
        err = sensor != nullptr ? sensor->activate(connection.get(), false) : status_t(BAD_VALUE);

    }
    if (err == NO_ERROR) {
        mLastNSensorRegistrations.editItemAt(mNextSensorRegIndex) =
                SensorRegistrationInfo(handle, connection->getPackageName(), 0, 0, false);
        mNextSensorRegIndex = (mNextSensorRegIndex + 1) % SENSOR_REGISTRATIONS_BUF_SIZE;
    }
    return err;
}

status_t SensorService::cleanupWithoutDisable(
        const sp<SensorEventConnection>& connection, int handle) {
    Mutex::Autolock _l(mLock);
    return cleanupWithoutDisableLocked(connection, handle);
}

status_t SensorService::cleanupWithoutDisableLocked(
        const sp<SensorEventConnection>& connection, int handle) {
    SensorRecord* rec = mActiveSensors.valueFor(handle);
    if (rec) {
        // see if this connection becomes inactive
        if (connection->removeSensor(handle)) {
            BatteryService::disableSensor(connection->getUid(), handle);
        }
        if (connection->hasAnySensor() == false) {
            connection->updateLooperRegistration(mLooper);
            mConnectionHolder.removeEventConnection(connection);
        }
        // see if this sensor becomes inactive
        if (rec->removeConnection(connection)) {
            mActiveSensors.removeItem(handle);
            mActiveVirtualSensors.erase(handle);
            delete rec;
        }
        return NO_ERROR;
    }
    return BAD_VALUE;
}

status_t SensorService::setEventRate(const sp<SensorEventConnection>& connection,
        int handle, nsecs_t ns, const String16& opPackageName) {
    if (mInitCheck != NO_ERROR)
        return mInitCheck;

    sp<SensorInterface> sensor = getSensorInterfaceFromHandle(handle);
    if (sensor == nullptr ||
        !canAccessSensor(sensor->getSensor(), "Tried configuring", opPackageName)) {
        return BAD_VALUE;
    }

    if (ns < 0)
        return BAD_VALUE;

    nsecs_t minDelayNs = sensor->getSensor().getMinDelayNs();
    if (ns < minDelayNs) {
        ns = minDelayNs;
    }

    return sensor->setDelay(connection.get(), handle, ns);
}

status_t SensorService::flushSensor(const sp<SensorEventConnection>& connection,
        const String16& opPackageName) {
    if (mInitCheck != NO_ERROR) return mInitCheck;
    SensorDevice& dev(SensorDevice::getInstance());
    const int halVersion = dev.getHalDeviceVersion();
    status_t err(NO_ERROR);
    Mutex::Autolock _l(mLock);
    // Loop through all sensors for this connection and call flush on each of them.
    for (size_t i = 0; i < connection->mSensorInfo.size(); ++i) {
        const int handle = connection->mSensorInfo.keyAt(i);
        sp<SensorInterface> sensor = getSensorInterfaceFromHandle(handle);
        if (sensor == nullptr) {
            continue;
        }
        if (sensor->getSensor().getReportingMode() == AREPORTING_MODE_ONE_SHOT) {
            ALOGE("flush called on a one-shot sensor");
            err = INVALID_OPERATION;
            continue;
        }
        if (halVersion <= SENSORS_DEVICE_API_VERSION_1_0 || isVirtualSensor(handle)) {
            // For older devices just increment pending flush count which will send a trivial
            // flush complete event.
            connection->incrementPendingFlushCount(handle);
        } else {
            if (!canAccessSensor(sensor->getSensor(), "Tried flushing", opPackageName)) {
                err = INVALID_OPERATION;
                continue;
            }
            status_t err_flush = sensor->flush(connection.get(), handle);
            if (err_flush == NO_ERROR) {
                SensorRecord* rec = mActiveSensors.valueFor(handle);
                if (rec != nullptr) rec->addPendingFlushConnection(connection);
            }
            err = (err_flush != NO_ERROR) ? err_flush : err;
        }
    }
    return err;
}

bool SensorService::canAccessSensor(const Sensor& sensor, const char* operation,
        const String16& opPackageName) {
    // Check if a permission is required for this sensor
    if (sensor.getRequiredPermission().length() <= 0) {
        return true;
    }

    const int32_t opCode = sensor.getRequiredAppOp();
    const int32_t appOpMode = sAppOpsManager.checkOp(opCode,
            IPCThreadState::self()->getCallingUid(), opPackageName);
    bool appOpAllowed = appOpMode == AppOpsManager::MODE_ALLOWED;

    bool canAccess = false;
    if (hasPermissionForSensor(sensor)) {
        // Ensure that the AppOp is allowed, or that there is no necessary app op for the sensor
        if (opCode < 0 || appOpAllowed) {
            canAccess = true;
        }
    } else if (sensor.getType() == SENSOR_TYPE_STEP_COUNTER ||
            sensor.getType() == SENSOR_TYPE_STEP_DETECTOR) {
        int targetSdkVersion = getTargetSdkVersion(opPackageName);
        // Allow access to the sensor if the application targets pre-Q, which is before the
        // requirement to hold the AR permission to access Step Counter and Step Detector events
        // was introduced, and the user hasn't revoked the app op.
        //
        // Verifying the app op is required to ensure that the user hasn't revoked the necessary
        // permissions to access the Step Detector and Step Counter when the application targets
        // pre-Q. Without this check, if the user revokes the pre-Q install-time GMS Core AR
        // permission, the app would still be able to receive Step Counter and Step Detector events.
        if (appOpAllowed &&
                targetSdkVersion > 0 &&
                targetSdkVersion <= __ANDROID_API_P__) {
            canAccess = true;
        }
    }

    if (canAccess) {
        sAppOpsManager.noteOp(opCode, IPCThreadState::self()->getCallingUid(), opPackageName);
    } else {
        ALOGE("%s a sensor (%s) without holding its required permission: %s",
                operation, sensor.getName().string(), sensor.getRequiredPermission().string());
    }

    return canAccess;
}

bool SensorService::hasPermissionForSensor(const Sensor& sensor) {
    bool hasPermission = false;
    const String8& requiredPermission = sensor.getRequiredPermission();

    // Runtime permissions can't use the cache as they may change.
    if (sensor.isRequiredPermissionRuntime()) {
        hasPermission = checkPermission(String16(requiredPermission),
                IPCThreadState::self()->getCallingPid(), IPCThreadState::self()->getCallingUid());
    } else {
        hasPermission = PermissionCache::checkCallingPermission(String16(requiredPermission));
    }
    return hasPermission;
}

int SensorService::getTargetSdkVersion(const String16& opPackageName) {
    Mutex::Autolock packageLock(sPackageTargetVersionLock);
    int targetSdkVersion = -1;
    auto entry = sPackageTargetVersion.find(opPackageName);
    if (entry != sPackageTargetVersion.end()) {
        targetSdkVersion = entry->second;
    } else {
        sp<IBinder> binder = defaultServiceManager()->getService(String16("package_native"));
        if (binder != nullptr) {
            sp<content::pm::IPackageManagerNative> packageManager =
                    interface_cast<content::pm::IPackageManagerNative>(binder);
            if (packageManager != nullptr) {
                binder::Status status = packageManager->getTargetSdkVersionForPackage(
                        opPackageName, &targetSdkVersion);
                if (!status.isOk()) {
                    targetSdkVersion = -1;
                }
            }
        }
        sPackageTargetVersion[opPackageName] = targetSdkVersion;
    }
    return targetSdkVersion;
}

void SensorService::checkWakeLockState() {
    ConnectionSafeAutolock connLock = mConnectionHolder.lock(mLock);
    checkWakeLockStateLocked(&connLock);
}

void SensorService::checkWakeLockStateLocked(ConnectionSafeAutolock* connLock) {
    if (!mWakeLockAcquired) {
        return;
    }
    bool releaseLock = true;
    for (const sp<SensorEventConnection>& connection : connLock->getActiveConnections()) {
        if (connection->needsWakeLock()) {
            releaseLock = false;
            break;
        }
    }
    if (releaseLock) {
        setWakeLockAcquiredLocked(false);
    }
}

void SensorService::sendEventsFromCache(const sp<SensorEventConnection>& connection) {
    Mutex::Autolock _l(mLock);
    connection->writeToSocketFromCache();
    if (connection->needsWakeLock()) {
        setWakeLockAcquiredLocked(true);
    }
}

bool SensorService::isWhiteListedPackage(const String8& packageName) {
    return (packageName.contains(mWhiteListedPackage.string()));
}

bool SensorService::isOperationPermitted(const String16& opPackageName) {
    Mutex::Autolock _l(mLock);
    if (mCurrentOperatingMode == RESTRICTED) {
        String8 package(opPackageName);
        return isWhiteListedPackage(package);
    }
    return true;
}

void SensorService::UidPolicy::registerSelf() {
    ActivityManager am;
    am.registerUidObserver(this, ActivityManager::UID_OBSERVER_GONE
            | ActivityManager::UID_OBSERVER_IDLE
            | ActivityManager::UID_OBSERVER_ACTIVE,
            ActivityManager::PROCESS_STATE_UNKNOWN,
            String16("android"));
}

void SensorService::UidPolicy::unregisterSelf() {
    ActivityManager am;
    am.unregisterUidObserver(this);
}

void SensorService::UidPolicy::onUidGone(__unused uid_t uid, __unused bool disabled) {
    onUidIdle(uid, disabled);
}

void SensorService::UidPolicy::onUidActive(uid_t uid) {
    {
        Mutex::Autolock _l(mUidLock);
        mActiveUids.insert(uid);
    }
    sp<SensorService> service = mService.promote();
    if (service != nullptr) {
        service->setSensorAccess(uid, true);
    }
}

void SensorService::UidPolicy::onUidIdle(uid_t uid, __unused bool disabled) {
    bool deleted = false;
    {
        Mutex::Autolock _l(mUidLock);
        if (mActiveUids.erase(uid) > 0) {
            deleted = true;
        }
    }
    if (deleted) {
        sp<SensorService> service = mService.promote();
        if (service != nullptr) {
            service->setSensorAccess(uid, false);
        }
    }
}

void SensorService::UidPolicy::addOverrideUid(uid_t uid, bool active) {
    updateOverrideUid(uid, active, true);
}

void SensorService::UidPolicy::removeOverrideUid(uid_t uid) {
    updateOverrideUid(uid, false, false);
}

void SensorService::UidPolicy::updateOverrideUid(uid_t uid, bool active, bool insert) {
    bool wasActive = false;
    bool isActive = false;
    {
        Mutex::Autolock _l(mUidLock);
        wasActive = isUidActiveLocked(uid);
        mOverrideUids.erase(uid);
        if (insert) {
            mOverrideUids.insert(std::pair<uid_t, bool>(uid, active));
        }
        isActive = isUidActiveLocked(uid);
    }
    if (wasActive != isActive) {
        sp<SensorService> service = mService.promote();
        if (service != nullptr) {
            service->setSensorAccess(uid, isActive);
        }
    }
}

bool SensorService::UidPolicy::isUidActive(uid_t uid) {
    // Non-app UIDs are considered always active
    if (uid < FIRST_APPLICATION_UID) {
        return true;
    }
    Mutex::Autolock _l(mUidLock);
    return isUidActiveLocked(uid);
}

bool SensorService::UidPolicy::isUidActiveLocked(uid_t uid) {
    // Non-app UIDs are considered always active
    if (uid < FIRST_APPLICATION_UID) {
        return true;
    }
    auto it = mOverrideUids.find(uid);
    if (it != mOverrideUids.end()) {
        return it->second;
    }
    return mActiveUids.find(uid) != mActiveUids.end();
}

void SensorService::SensorPrivacyPolicy::registerSelf() {
    SensorPrivacyManager spm;
    mSensorPrivacyEnabled = spm.isSensorPrivacyEnabled();
    spm.addSensorPrivacyListener(this);
}

void SensorService::SensorPrivacyPolicy::unregisterSelf() {
    SensorPrivacyManager spm;
    spm.removeSensorPrivacyListener(this);
}

bool SensorService::SensorPrivacyPolicy::isSensorPrivacyEnabled() {
    return mSensorPrivacyEnabled;
}

binder::Status SensorService::SensorPrivacyPolicy::onSensorPrivacyChanged(bool enabled) {
    mSensorPrivacyEnabled = enabled;
    sp<SensorService> service = mService.promote();
    if (service != nullptr) {
        if (enabled) {
            service->disableAllSensors();
        } else {
            service->enableAllSensors();
        }
    }
    return binder::Status::ok();
}

SensorService::ConnectionSafeAutolock::ConnectionSafeAutolock(
        SensorService::SensorConnectionHolder& holder, Mutex& mutex)
        : mConnectionHolder(holder), mAutolock(mutex) {}

template<typename ConnectionType>
const std::vector<sp<ConnectionType>>& SensorService::ConnectionSafeAutolock::getConnectionsHelper(
        const SortedVector<wp<ConnectionType>>& connectionList,
        std::vector<std::vector<sp<ConnectionType>>>* referenceHolder) {
    referenceHolder->emplace_back();
    std::vector<sp<ConnectionType>>& connections = referenceHolder->back();
    for (const wp<ConnectionType>& weakConnection : connectionList){
        sp<ConnectionType> connection = weakConnection.promote();
        if (connection != nullptr) {
            connections.push_back(std::move(connection));
        }
    }
    return connections;
}

const std::vector<sp<SensorService::SensorEventConnection>>&
        SensorService::ConnectionSafeAutolock::getActiveConnections() {
    return getConnectionsHelper(mConnectionHolder.mActiveConnections,
                                &mReferencedActiveConnections);
}

const std::vector<sp<SensorService::SensorDirectConnection>>&
        SensorService::ConnectionSafeAutolock::getDirectConnections() {
    return getConnectionsHelper(mConnectionHolder.mDirectConnections,
                                &mReferencedDirectConnections);
}

void SensorService::SensorConnectionHolder::addEventConnectionIfNotPresent(
        const sp<SensorService::SensorEventConnection>& connection) {
    if (mActiveConnections.indexOf(connection) < 0) {
        mActiveConnections.add(connection);
    }
}

void SensorService::SensorConnectionHolder::removeEventConnection(
        const wp<SensorService::SensorEventConnection>& connection) {
    mActiveConnections.remove(connection);
}

void SensorService::SensorConnectionHolder::addDirectConnection(
        const sp<SensorService::SensorDirectConnection>& connection) {
    mDirectConnections.add(connection);
}

void SensorService::SensorConnectionHolder::removeDirectConnection(
        const wp<SensorService::SensorDirectConnection>& connection) {
    mDirectConnections.remove(connection);
}

SensorService::ConnectionSafeAutolock SensorService::SensorConnectionHolder::lock(Mutex& mutex) {
    return ConnectionSafeAutolock(*this, mutex);
}

} // namespace android
