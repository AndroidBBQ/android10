/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "SensorDevice.h"
#include "SensorDirectConnection.h"
#include <hardware/sensors.h>

#define UNUSED(x) (void)(x)

namespace android {

SensorService::SensorDirectConnection::SensorDirectConnection(const sp<SensorService>& service,
        uid_t uid, const sensors_direct_mem_t *mem, int32_t halChannelHandle,
        const String16& opPackageName)
        : mService(service), mUid(uid), mMem(*mem),
        mHalChannelHandle(halChannelHandle),
        mOpPackageName(opPackageName), mDestroyed(false) {
    ALOGD_IF(DEBUG_CONNECTIONS, "Created SensorDirectConnection");
}

SensorService::SensorDirectConnection::~SensorDirectConnection() {
    ALOGD_IF(DEBUG_CONNECTIONS, "~SensorDirectConnection %p", this);
    destroy();
}

void SensorService::SensorDirectConnection::destroy() {
    Mutex::Autolock _l(mDestroyLock);
    // destroy once only
    if (mDestroyed) {
        return;
    }

    stopAll();
    mService->cleanupConnection(this);
    if (mMem.handle != nullptr) {
        native_handle_close(mMem.handle);
        native_handle_delete(const_cast<struct native_handle*>(mMem.handle));
    }
    mDestroyed = true;
}

void SensorService::SensorDirectConnection::onFirstRef() {
}

void SensorService::SensorDirectConnection::dump(String8& result) const {
    Mutex::Autolock _l(mConnectionLock);
    result.appendFormat("\tPackage %s, HAL channel handle %d, total sensor activated %zu\n",
            String8(mOpPackageName).string(), getHalChannelHandle(), mActivated.size());
    for (auto &i : mActivated) {
        result.appendFormat("\t\tSensor %#08x, rate %d\n", i.first, i.second);
    }
}

sp<BitTube> SensorService::SensorDirectConnection::getSensorChannel() const {
    return nullptr;
}

status_t SensorService::SensorDirectConnection::enableDisable(
        int handle, bool enabled, nsecs_t samplingPeriodNs, nsecs_t maxBatchReportLatencyNs,
        int reservedFlags) {
    // SensorDirectConnection does not support enableDisable, parameters not used
    UNUSED(handle);
    UNUSED(enabled);
    UNUSED(samplingPeriodNs);
    UNUSED(maxBatchReportLatencyNs);
    UNUSED(reservedFlags);
    return INVALID_OPERATION;
}

status_t SensorService::SensorDirectConnection::setEventRate(
        int handle, nsecs_t samplingPeriodNs) {
    // SensorDirectConnection does not support setEventRate, parameters not used
    UNUSED(handle);
    UNUSED(samplingPeriodNs);
    return INVALID_OPERATION;
}

status_t SensorService::SensorDirectConnection::flush() {
    // SensorDirectConnection does not support flush
    return INVALID_OPERATION;
}

int32_t SensorService::SensorDirectConnection::configureChannel(int handle, int rateLevel) {

    if (handle == -1 && rateLevel == SENSOR_DIRECT_RATE_STOP) {
        stopAll();
        return NO_ERROR;
    }

    if (!mService->isOperationPermitted(mOpPackageName)) {
        return PERMISSION_DENIED;
    }

    sp<SensorInterface> si = mService->getSensorInterfaceFromHandle(handle);
    if (si == nullptr) {
        return NAME_NOT_FOUND;
    }

    const Sensor& s = si->getSensor();
    if (!SensorService::canAccessSensor(s, "config direct channel", mOpPackageName)) {
        return PERMISSION_DENIED;
    }

    if (s.getHighestDirectReportRateLevel() == 0
            || rateLevel > s.getHighestDirectReportRateLevel()
            || !s.isDirectChannelTypeSupported(mMem.type)) {
        return INVALID_OPERATION;
    }

    struct sensors_direct_cfg_t config = {
        .rate_level = rateLevel
    };

    Mutex::Autolock _l(mConnectionLock);
    SensorDevice& dev(SensorDevice::getInstance());
    int ret = dev.configureDirectChannel(handle, getHalChannelHandle(), &config);

    if (rateLevel == SENSOR_DIRECT_RATE_STOP) {
        if (ret == NO_ERROR) {
            mActivated.erase(handle);
        } else if (ret > 0) {
            ret = UNKNOWN_ERROR;
        }
    } else {
        if (ret > 0) {
            mActivated[handle] = rateLevel;
        }
    }

    return ret;
}

void SensorService::SensorDirectConnection::stopAll(bool backupRecord) {

    struct sensors_direct_cfg_t config = {
        .rate_level = SENSOR_DIRECT_RATE_STOP
    };

    Mutex::Autolock _l(mConnectionLock);
    SensorDevice& dev(SensorDevice::getInstance());
    for (auto &i : mActivated) {
        dev.configureDirectChannel(i.first, getHalChannelHandle(), &config);
    }

    if (backupRecord && mActivatedBackup.empty()) {
        mActivatedBackup = mActivated;
    }
    mActivated.clear();
}

void SensorService::SensorDirectConnection::recoverAll() {
    stopAll(false);

    Mutex::Autolock _l(mConnectionLock);
    SensorDevice& dev(SensorDevice::getInstance());

    // recover list of report from backup
    mActivated = mActivatedBackup;
    mActivatedBackup.clear();

    // re-enable them
    for (auto &i : mActivated) {
        struct sensors_direct_cfg_t config = {
            .rate_level = i.second
        };
        dev.configureDirectChannel(i.first, getHalChannelHandle(), &config);
    }
}

int32_t SensorService::SensorDirectConnection::getHalChannelHandle() const {
    return mHalChannelHandle;
}

bool SensorService::SensorDirectConnection::isEquivalent(const sensors_direct_mem_t *mem) const {
    bool ret = false;

    if (mMem.type == mem->type) {
        switch (mMem.type) {
            case SENSOR_DIRECT_MEM_TYPE_ASHMEM: {
                // there is no known method to test if two ashmem fds are equivalent besides
                // trivially comparing the fd values (ino number from fstat() are always the
                // same, pointing to "/dev/ashmem").
                int fd1 = mMem.handle->data[0];
                int fd2 = mem->handle->data[0];
                ret = (fd1 == fd2);
                break;
            }
            case SENSOR_DIRECT_MEM_TYPE_GRALLOC:
                // there is no known method to test if two gralloc handle are equivalent
                ret = false;
                break;
            default:
                // should never happen
                ALOGE("Unexpected mem type %d", mMem.type);
                ret = true;
                break;
        }
    }
    return ret;
}

} // namespace android

