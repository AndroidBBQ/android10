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

#include "SensorDevice.h"

#include "android/hardware/sensors/2.0/ISensorsCallback.h"
#include "android/hardware/sensors/2.0/types.h"
#include "SensorService.h"

#include <android-base/logging.h>
#include <sensors/convert.h>
#include <cutils/atomic.h>
#include <utils/Errors.h>
#include <utils/Singleton.h>

#include <chrono>
#include <cinttypes>
#include <thread>

using namespace android::hardware::sensors;
using namespace android::hardware::sensors::V1_0;
using namespace android::hardware::sensors::V1_0::implementation;
using android::hardware::sensors::V2_0::ISensorsCallback;
using android::hardware::sensors::V2_0::EventQueueFlagBits;
using android::hardware::sensors::V2_0::WakeLockQueueFlagBits;
using android::hardware::hidl_vec;
using android::hardware::Return;
using android::SensorDeviceUtils::HidlServiceRegistrationWaiter;

namespace android {
// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(SensorDevice)

namespace {

status_t statusFromResult(Result result) {
    switch (result) {
        case Result::OK:
            return OK;
        case Result::BAD_VALUE:
            return BAD_VALUE;
        case Result::PERMISSION_DENIED:
            return PERMISSION_DENIED;
        case Result::INVALID_OPERATION:
            return INVALID_OPERATION;
        case Result::NO_MEMORY:
            return NO_MEMORY;
    }
}

template<typename EnumType>
constexpr typename std::underlying_type<EnumType>::type asBaseType(EnumType value) {
    return static_cast<typename std::underlying_type<EnumType>::type>(value);
}

// Used internally by the framework to wake the Event FMQ. These values must start after
// the last value of EventQueueFlagBits
enum EventQueueFlagBitsInternal : uint32_t {
    INTERNAL_WAKE =  1 << 16,
};

}  // anonymous namespace

void SensorsHalDeathReceivier::serviceDied(
        uint64_t /* cookie */,
        const wp<::android::hidl::base::V1_0::IBase>& /* service */) {
    ALOGW("Sensors HAL died, attempting to reconnect.");
    SensorDevice::getInstance().prepareForReconnect();
}

struct SensorsCallback : public ISensorsCallback {
    using Result = ::android::hardware::sensors::V1_0::Result;
    Return<void> onDynamicSensorsConnected(
            const hidl_vec<SensorInfo> &dynamicSensorsAdded) override {
        return SensorDevice::getInstance().onDynamicSensorsConnected(dynamicSensorsAdded);
    }

    Return<void> onDynamicSensorsDisconnected(
            const hidl_vec<int32_t> &dynamicSensorHandlesRemoved) override {
        return SensorDevice::getInstance().onDynamicSensorsDisconnected(
                dynamicSensorHandlesRemoved);
    }
};

SensorDevice::SensorDevice()
        : mHidlTransportErrors(20),
          mRestartWaiter(new HidlServiceRegistrationWaiter()),
          mEventQueueFlag(nullptr),
          mWakeLockQueueFlag(nullptr),
          mReconnecting(false) {
    if (!connectHidlService()) {
        return;
    }

    initializeSensorList();

    mIsDirectReportSupported =
            (checkReturnAndGetStatus(mSensors->unregisterDirectChannel(-1)) != INVALID_OPERATION);
}

void SensorDevice::initializeSensorList() {
    float minPowerMa = 0.001; // 1 microAmp

    checkReturn(mSensors->getSensorsList(
            [&](const auto &list) {
                const size_t count = list.size();

                mActivationCount.setCapacity(count);
                Info model;
                for (size_t i=0 ; i < count; i++) {
                    sensor_t sensor;
                    convertToSensor(list[i], &sensor);
                    // Sanity check and clamp power if it is 0 (or close)
                    if (sensor.power < minPowerMa) {
                        ALOGI("Reported power %f not deemed sane, clamping to %f",
                              sensor.power, minPowerMa);
                        sensor.power = minPowerMa;
                    }
                    mSensorList.push_back(sensor);

                    mActivationCount.add(list[i].sensorHandle, model);

                    checkReturn(mSensors->activate(list[i].sensorHandle, 0 /* enabled */));
                }
            }));
}

SensorDevice::~SensorDevice() {
    if (mEventQueueFlag != nullptr) {
        hardware::EventFlag::deleteEventFlag(&mEventQueueFlag);
        mEventQueueFlag = nullptr;
    }

    if (mWakeLockQueueFlag != nullptr) {
        hardware::EventFlag::deleteEventFlag(&mWakeLockQueueFlag);
        mWakeLockQueueFlag = nullptr;
    }
}

bool SensorDevice::connectHidlService() {
    HalConnectionStatus status = connectHidlServiceV2_0();
    if (status == HalConnectionStatus::DOES_NOT_EXIST) {
        status = connectHidlServiceV1_0();
    }
    return (status == HalConnectionStatus::CONNECTED);
}

SensorDevice::HalConnectionStatus SensorDevice::connectHidlServiceV1_0() {
    // SensorDevice will wait for HAL service to start if HAL is declared in device manifest.
    size_t retry = 10;
    HalConnectionStatus connectionStatus = HalConnectionStatus::UNKNOWN;

    while (retry-- > 0) {
        sp<V1_0::ISensors> sensors = V1_0::ISensors::getService();
        if (sensors == nullptr) {
            // no sensor hidl service found
            connectionStatus = HalConnectionStatus::DOES_NOT_EXIST;
            break;
        }

        mSensors = new SensorServiceUtil::SensorsWrapperV1_0(sensors);
        mRestartWaiter->reset();
        // Poke ISensor service. If it has lingering connection from previous generation of
        // system server, it will kill itself. There is no intention to handle the poll result,
        // which will be done since the size is 0.
        if(mSensors->poll(0, [](auto, const auto &, const auto &) {}).isOk()) {
            // ok to continue
            connectionStatus = HalConnectionStatus::CONNECTED;
            break;
        }

        // hidl service is restarting, pointer is invalid.
        mSensors = nullptr;
        connectionStatus = HalConnectionStatus::FAILED_TO_CONNECT;
        ALOGI("%s unsuccessful, remaining retry %zu.", __FUNCTION__, retry);
        mRestartWaiter->wait();
    }

    return connectionStatus;
}

SensorDevice::HalConnectionStatus SensorDevice::connectHidlServiceV2_0() {
    HalConnectionStatus connectionStatus = HalConnectionStatus::UNKNOWN;
    sp<V2_0::ISensors> sensors = V2_0::ISensors::getService();

    if (sensors == nullptr) {
        connectionStatus = HalConnectionStatus::DOES_NOT_EXIST;
    } else {
        mSensors = new SensorServiceUtil::SensorsWrapperV2_0(sensors);

        mEventQueue = std::make_unique<EventMessageQueue>(
                SensorEventQueue::MAX_RECEIVE_BUFFER_EVENT_COUNT,
                true /* configureEventFlagWord */);

        mWakeLockQueue = std::make_unique<WakeLockQueue>(
                SensorEventQueue::MAX_RECEIVE_BUFFER_EVENT_COUNT,
                true /* configureEventFlagWord */);

        hardware::EventFlag::deleteEventFlag(&mEventQueueFlag);
        hardware::EventFlag::createEventFlag(mEventQueue->getEventFlagWord(), &mEventQueueFlag);

        hardware::EventFlag::deleteEventFlag(&mWakeLockQueueFlag);
        hardware::EventFlag::createEventFlag(mWakeLockQueue->getEventFlagWord(),
                                             &mWakeLockQueueFlag);

        CHECK(mSensors != nullptr && mEventQueue != nullptr &&
                mWakeLockQueue != nullptr && mEventQueueFlag != nullptr &&
                mWakeLockQueueFlag != nullptr);

        status_t status = checkReturnAndGetStatus(mSensors->initialize(
                *mEventQueue->getDesc(),
                *mWakeLockQueue->getDesc(),
                new SensorsCallback()));

        if (status != NO_ERROR) {
            connectionStatus = HalConnectionStatus::FAILED_TO_CONNECT;
            ALOGE("Failed to initialize Sensors HAL (%s)", strerror(-status));
        } else {
            connectionStatus = HalConnectionStatus::CONNECTED;
            mSensorsHalDeathReceiver = new SensorsHalDeathReceivier();
            sensors->linkToDeath(mSensorsHalDeathReceiver, 0 /* cookie */);
        }
    }

    return connectionStatus;
}

void SensorDevice::prepareForReconnect() {
    mReconnecting = true;

    // Wake up the polling thread so it returns and allows the SensorService to initiate
    // a reconnect.
    mEventQueueFlag->wake(asBaseType(INTERNAL_WAKE));
}

void SensorDevice::reconnect() {
    Mutex::Autolock _l(mLock);
    mSensors = nullptr;

    auto previousActivations = mActivationCount;
    auto previousSensorList = mSensorList;

    mActivationCount.clear();
    mSensorList.clear();

    if (connectHidlServiceV2_0() == HalConnectionStatus::CONNECTED) {
        initializeSensorList();

        if (sensorHandlesChanged(previousSensorList, mSensorList)) {
            LOG_ALWAYS_FATAL("Sensor handles changed, cannot re-enable sensors.");
        } else {
            reactivateSensors(previousActivations);
        }
    }
    mReconnecting = false;
}

bool SensorDevice::sensorHandlesChanged(const Vector<sensor_t>& oldSensorList,
                                        const Vector<sensor_t>& newSensorList) {
    bool didChange = false;

    if (oldSensorList.size() != newSensorList.size()) {
        ALOGI("Sensor list size changed from %zu to %zu", oldSensorList.size(),
              newSensorList.size());
        didChange = true;
    }

    for (size_t i = 0; i < newSensorList.size() && !didChange; i++) {
        bool found = false;
        const sensor_t& newSensor = newSensorList[i];
        for (size_t j = 0; j < oldSensorList.size() && !found; j++) {
            const sensor_t& prevSensor = oldSensorList[j];
            if (prevSensor.handle == newSensor.handle) {
                found = true;
                if (!sensorIsEquivalent(prevSensor, newSensor)) {
                    ALOGI("Sensor %s not equivalent to previous version", newSensor.name);
                    didChange = true;
                }
            }
        }

        if (!found) {
            // Could not find the new sensor in the old list of sensors, the lists must
            // have changed.
            ALOGI("Sensor %s (handle %d) did not exist before", newSensor.name, newSensor.handle);
            didChange = true;
        }
    }
    return didChange;
}

bool SensorDevice::sensorIsEquivalent(const sensor_t& prevSensor, const sensor_t& newSensor) {
    bool equivalent = true;
    if (prevSensor.handle != newSensor.handle ||
            (strcmp(prevSensor.vendor, newSensor.vendor) != 0) ||
            (strcmp(prevSensor.stringType, newSensor.stringType) != 0) ||
            (strcmp(prevSensor.requiredPermission, newSensor.requiredPermission) != 0) ||
            (prevSensor.version != newSensor.version) ||
            (prevSensor.type != newSensor.type) ||
            (std::abs(prevSensor.maxRange - newSensor.maxRange) > 0.001f) ||
            (std::abs(prevSensor.resolution - newSensor.resolution) > 0.001f) ||
            (std::abs(prevSensor.power - newSensor.power) > 0.001f) ||
            (prevSensor.minDelay != newSensor.minDelay) ||
            (prevSensor.fifoReservedEventCount != newSensor.fifoReservedEventCount) ||
            (prevSensor.fifoMaxEventCount != newSensor.fifoMaxEventCount) ||
            (prevSensor.maxDelay != newSensor.maxDelay) ||
            (prevSensor.flags != newSensor.flags)) {
        equivalent = false;
    }
    return equivalent;
}

void SensorDevice::reactivateSensors(const DefaultKeyedVector<int, Info>& previousActivations) {
    for (size_t i = 0; i < mSensorList.size(); i++) {
        int handle = mSensorList[i].handle;
        ssize_t activationIndex = previousActivations.indexOfKey(handle);
        if (activationIndex < 0 || previousActivations[activationIndex].numActiveClients() <= 0) {
            continue;
        }

        const Info& info = previousActivations[activationIndex];
        for (size_t j = 0; j < info.batchParams.size(); j++) {
            const BatchParams& batchParams = info.batchParams[j];
            status_t res = batchLocked(info.batchParams.keyAt(j), handle, 0 /* flags */,
                    batchParams.mTSample, batchParams.mTBatch);

            if (res == NO_ERROR) {
                activateLocked(info.batchParams.keyAt(j), handle, true /* enabled */);
            }
        }
    }
}

void SensorDevice::handleDynamicSensorConnection(int handle, bool connected) {
    // not need to check mSensors because this is is only called after successful poll()
    if (connected) {
        Info model;
        mActivationCount.add(handle, model);
        checkReturn(mSensors->activate(handle, 0 /* enabled */));
    } else {
        mActivationCount.removeItem(handle);
    }
}

std::string SensorDevice::dump() const {
    if (mSensors == nullptr) return "HAL not initialized\n";

    String8 result;
    result.appendFormat("Total %zu h/w sensors, %zu running:\n",
                        mSensorList.size(), mActivationCount.size());

    Mutex::Autolock _l(mLock);
    for (const auto & s : mSensorList) {
        int32_t handle = s.handle;
        const Info& info = mActivationCount.valueFor(handle);
        if (info.numActiveClients() == 0) continue;

        result.appendFormat("0x%08x) active-count = %zu; ", handle, info.batchParams.size());

        result.append("sampling_period(ms) = {");
        for (size_t j = 0; j < info.batchParams.size(); j++) {
            const BatchParams& params = info.batchParams[j];
            result.appendFormat("%.1f%s", params.mTSample / 1e6f,
                j < info.batchParams.size() - 1 ? ", " : "");
        }
        result.appendFormat("}, selected = %.2f ms; ", info.bestBatchParams.mTSample / 1e6f);

        result.append("batching_period(ms) = {");
        for (size_t j = 0; j < info.batchParams.size(); j++) {
            const BatchParams& params = info.batchParams[j];
            result.appendFormat("%.1f%s", params.mTBatch / 1e6f,
                    j < info.batchParams.size() - 1 ? ", " : "");
        }
        result.appendFormat("}, selected = %.2f ms\n", info.bestBatchParams.mTBatch / 1e6f);
    }

    return result.string();
}

ssize_t SensorDevice::getSensorList(sensor_t const** list) {
    *list = &mSensorList[0];

    return mSensorList.size();
}

status_t SensorDevice::initCheck() const {
    return mSensors != nullptr ? NO_ERROR : NO_INIT;
}

ssize_t SensorDevice::poll(sensors_event_t* buffer, size_t count) {
    if (mSensors == nullptr) return NO_INIT;

    ssize_t eventsRead = 0;
    if (mSensors->supportsMessageQueues()) {
        eventsRead = pollFmq(buffer, count);
    } else if (mSensors->supportsPolling()) {
        eventsRead = pollHal(buffer, count);
    } else {
        ALOGE("Must support polling or FMQ");
        eventsRead = -1;
    }
    return eventsRead;
}

ssize_t SensorDevice::pollHal(sensors_event_t* buffer, size_t count) {
    ssize_t err;
    int numHidlTransportErrors = 0;
    bool hidlTransportError = false;

    do {
        auto ret = mSensors->poll(
                count,
                [&](auto result,
                    const auto &events,
                    const auto &dynamicSensorsAdded) {
                    if (result == Result::OK) {
                        convertToSensorEvents(events, dynamicSensorsAdded, buffer);
                        err = (ssize_t)events.size();
                    } else {
                        err = statusFromResult(result);
                    }
                });

        if (ret.isOk())  {
            hidlTransportError = false;
        } else {
            hidlTransportError = true;
            numHidlTransportErrors++;
            if (numHidlTransportErrors > 50) {
                // Log error and bail
                ALOGE("Max Hidl transport errors this cycle : %d", numHidlTransportErrors);
                handleHidlDeath(ret.description());
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    } while (hidlTransportError);

    if(numHidlTransportErrors > 0) {
        ALOGE("Saw %d Hidl transport failures", numHidlTransportErrors);
        HidlTransportErrorLog errLog(time(nullptr), numHidlTransportErrors);
        mHidlTransportErrors.add(errLog);
        mTotalHidlTransportErrors++;
    }

    return err;
}

ssize_t SensorDevice::pollFmq(sensors_event_t* buffer, size_t maxNumEventsToRead) {
    ssize_t eventsRead = 0;
    size_t availableEvents = mEventQueue->availableToRead();

    if (availableEvents == 0) {
        uint32_t eventFlagState = 0;

        // Wait for events to become available. This is necessary so that the Event FMQ's read() is
        // able to be called with the correct number of events to read. If the specified number of
        // events is not available, then read() would return no events, possibly introducing
        // additional latency in delivering events to applications.
        mEventQueueFlag->wait(asBaseType(EventQueueFlagBits::READ_AND_PROCESS) |
                              asBaseType(INTERNAL_WAKE), &eventFlagState);
        availableEvents = mEventQueue->availableToRead();

        if ((eventFlagState & asBaseType(INTERNAL_WAKE)) && mReconnecting) {
            ALOGD("Event FMQ internal wake, returning from poll with no events");
            return DEAD_OBJECT;
        }
    }

    size_t eventsToRead = std::min({availableEvents, maxNumEventsToRead, mEventBuffer.size()});
    if (eventsToRead > 0) {
        if (mEventQueue->read(mEventBuffer.data(), eventsToRead)) {
            // Notify the Sensors HAL that sensor events have been read. This is required to support
            // the use of writeBlocking by the Sensors HAL.
            mEventQueueFlag->wake(asBaseType(EventQueueFlagBits::EVENTS_READ));

            for (size_t i = 0; i < eventsToRead; i++) {
                convertToSensorEvent(mEventBuffer[i], &buffer[i]);
            }
            eventsRead = eventsToRead;
        } else {
            ALOGW("Failed to read %zu events, currently %zu events available",
                    eventsToRead, availableEvents);
        }
    }

    return eventsRead;
}

Return<void> SensorDevice::onDynamicSensorsConnected(
        const hidl_vec<SensorInfo> &dynamicSensorsAdded) {
    // Allocate a sensor_t structure for each dynamic sensor added and insert
    // it into the dictionary of connected dynamic sensors keyed by handle.
    for (size_t i = 0; i < dynamicSensorsAdded.size(); ++i) {
        const SensorInfo &info = dynamicSensorsAdded[i];

        auto it = mConnectedDynamicSensors.find(info.sensorHandle);
        CHECK(it == mConnectedDynamicSensors.end());

        sensor_t *sensor = new sensor_t();
        convertToSensor(info, sensor);

        mConnectedDynamicSensors.insert(
                std::make_pair(sensor->handle, sensor));
    }

    return Return<void>();
}

Return<void> SensorDevice::onDynamicSensorsDisconnected(
        const hidl_vec<int32_t> &dynamicSensorHandlesRemoved) {
    (void) dynamicSensorHandlesRemoved;
    // TODO: Currently dynamic sensors do not seem to be removed
    return Return<void>();
}

void SensorDevice::writeWakeLockHandled(uint32_t count) {
    if (mSensors != nullptr && mSensors->supportsMessageQueues()) {
        if (mWakeLockQueue->write(&count)) {
            mWakeLockQueueFlag->wake(asBaseType(WakeLockQueueFlagBits::DATA_WRITTEN));
        } else {
            ALOGW("Failed to write wake lock handled");
        }
    }
}

void SensorDevice::autoDisable(void *ident, int handle) {
    Mutex::Autolock _l(mLock);
    ssize_t activationIndex = mActivationCount.indexOfKey(handle);
    if (activationIndex < 0) {
        ALOGW("Handle %d cannot be found in activation record", handle);
        return;
    }
    Info& info(mActivationCount.editValueAt(activationIndex));
    info.removeBatchParamsForIdent(ident);
    if (info.numActiveClients() == 0) {
        info.isActive = false;
    }
}

status_t SensorDevice::activate(void* ident, int handle, int enabled) {
    if (mSensors == nullptr) return NO_INIT;

    Mutex::Autolock _l(mLock);
    return activateLocked(ident, handle, enabled);
}

status_t SensorDevice::activateLocked(void* ident, int handle, int enabled) {
    bool actuateHardware = false;

    status_t err(NO_ERROR);

    ssize_t activationIndex = mActivationCount.indexOfKey(handle);
    if (activationIndex < 0) {
        ALOGW("Handle %d cannot be found in activation record", handle);
        return BAD_VALUE;
    }
    Info& info(mActivationCount.editValueAt(activationIndex));

    ALOGD_IF(DEBUG_CONNECTIONS,
             "SensorDevice::activate: ident=%p, handle=0x%08x, enabled=%d, count=%zu",
             ident, handle, enabled, info.batchParams.size());

    if (enabled) {
        ALOGD_IF(DEBUG_CONNECTIONS, "enable index=%zd", info.batchParams.indexOfKey(ident));

        if (isClientDisabledLocked(ident)) {
            ALOGE("SensorDevice::activate, isClientDisabledLocked(%p):true, handle:%d",
                    ident, handle);
            return INVALID_OPERATION;
        }

        if (info.batchParams.indexOfKey(ident) >= 0) {
            if (info.numActiveClients() > 0 && !info.isActive) {
                actuateHardware = true;
            }
        } else {
            // Log error. Every activate call should be preceded by a batch() call.
            ALOGE("\t >>>ERROR: activate called without batch");
        }
    } else {
        ALOGD_IF(DEBUG_CONNECTIONS, "disable index=%zd", info.batchParams.indexOfKey(ident));

        // If a connected dynamic sensor is deactivated, remove it from the
        // dictionary.
        auto it = mConnectedDynamicSensors.find(handle);
        if (it != mConnectedDynamicSensors.end()) {
            delete it->second;
            mConnectedDynamicSensors.erase(it);
        }

        if (info.removeBatchParamsForIdent(ident) >= 0) {
            if (info.numActiveClients() == 0) {
                // This is the last connection, we need to de-activate the underlying h/w sensor.
                actuateHardware = true;
            } else {
                // Call batch for this sensor with the previously calculated best effort
                // batch_rate and timeout. One of the apps has unregistered for sensor
                // events, and the best effort batch parameters might have changed.
                ALOGD_IF(DEBUG_CONNECTIONS,
                         "\t>>> actuating h/w batch 0x%08x %" PRId64 " %" PRId64, handle,
                         info.bestBatchParams.mTSample, info.bestBatchParams.mTBatch);
                checkReturn(mSensors->batch(
                        handle, info.bestBatchParams.mTSample, info.bestBatchParams.mTBatch));
            }
        } else {
            // sensor wasn't enabled for this ident
        }

        if (isClientDisabledLocked(ident)) {
            return NO_ERROR;
        }
    }

    if (actuateHardware) {
        ALOGD_IF(DEBUG_CONNECTIONS, "\t>>> actuating h/w activate handle=%d enabled=%d", handle,
                 enabled);
        err = checkReturnAndGetStatus(mSensors->activate(handle, enabled));
        ALOGE_IF(err, "Error %s sensor %d (%s)", enabled ? "activating" : "disabling", handle,
                 strerror(-err));

        if (err != NO_ERROR && enabled) {
            // Failure when enabling the sensor. Clean up on failure.
            info.removeBatchParamsForIdent(ident);
        } else {
            // Update the isActive flag if there is no error. If there is an error when disabling a
            // sensor, still set the flag to false since the batch parameters have already been
            // removed. This ensures that everything remains in-sync.
            info.isActive = enabled;
        }
    }

    return err;
}

status_t SensorDevice::batch(
        void* ident,
        int handle,
        int flags,
        int64_t samplingPeriodNs,
        int64_t maxBatchReportLatencyNs) {
    if (mSensors == nullptr) return NO_INIT;

    if (samplingPeriodNs < MINIMUM_EVENTS_PERIOD) {
        samplingPeriodNs = MINIMUM_EVENTS_PERIOD;
    }
    if (maxBatchReportLatencyNs < 0) {
        maxBatchReportLatencyNs = 0;
    }

    ALOGD_IF(DEBUG_CONNECTIONS,
             "SensorDevice::batch: ident=%p, handle=0x%08x, flags=%d, period_ns=%" PRId64 " timeout=%" PRId64,
             ident, handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);

    Mutex::Autolock _l(mLock);
    return batchLocked(ident, handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);
}

status_t SensorDevice::batchLocked(void* ident, int handle, int flags, int64_t samplingPeriodNs,
                                   int64_t maxBatchReportLatencyNs) {
    ssize_t activationIndex = mActivationCount.indexOfKey(handle);
    if (activationIndex < 0) {
        ALOGW("Handle %d cannot be found in activation record", handle);
        return BAD_VALUE;
    }
    Info& info(mActivationCount.editValueAt(activationIndex));

    if (info.batchParams.indexOfKey(ident) < 0) {
        BatchParams params(samplingPeriodNs, maxBatchReportLatencyNs);
        info.batchParams.add(ident, params);
    } else {
        // A batch has already been called with this ident. Update the batch parameters.
        info.setBatchParamsForIdent(ident, flags, samplingPeriodNs, maxBatchReportLatencyNs);
    }

    BatchParams prevBestBatchParams = info.bestBatchParams;
    // Find the minimum of all timeouts and batch_rates for this sensor.
    info.selectBatchParams();

    ALOGD_IF(DEBUG_CONNECTIONS,
             "\t>>> curr_period=%" PRId64 " min_period=%" PRId64
             " curr_timeout=%" PRId64 " min_timeout=%" PRId64,
             prevBestBatchParams.mTSample, info.bestBatchParams.mTSample,
             prevBestBatchParams.mTBatch, info.bestBatchParams.mTBatch);

    status_t err(NO_ERROR);
    // If the min period or min timeout has changed since the last batch call, call batch.
    if (prevBestBatchParams != info.bestBatchParams) {
        ALOGD_IF(DEBUG_CONNECTIONS, "\t>>> actuating h/w BATCH 0x%08x %" PRId64 " %" PRId64, handle,
                 info.bestBatchParams.mTSample, info.bestBatchParams.mTBatch);
        err = checkReturnAndGetStatus(mSensors->batch(
                handle, info.bestBatchParams.mTSample, info.bestBatchParams.mTBatch));
        if (err != NO_ERROR) {
            ALOGE("sensor batch failed %p 0x%08x %" PRId64 " %" PRId64 " err=%s",
                  mSensors.get(), handle, info.bestBatchParams.mTSample,
                  info.bestBatchParams.mTBatch, strerror(-err));
            info.removeBatchParamsForIdent(ident);
        }
    }
    return err;
}

status_t SensorDevice::setDelay(void* ident, int handle, int64_t samplingPeriodNs) {
    return batch(ident, handle, 0, samplingPeriodNs, 0);
}

int SensorDevice::getHalDeviceVersion() const {
    if (mSensors == nullptr) return -1;
    return SENSORS_DEVICE_API_VERSION_1_4;
}

status_t SensorDevice::flush(void* ident, int handle) {
    if (mSensors == nullptr) return NO_INIT;
    if (isClientDisabled(ident)) return INVALID_OPERATION;
    ALOGD_IF(DEBUG_CONNECTIONS, "\t>>> actuating h/w flush %d", handle);
    return checkReturnAndGetStatus(mSensors->flush(handle));
}

bool SensorDevice::isClientDisabled(void* ident) {
    Mutex::Autolock _l(mLock);
    return isClientDisabledLocked(ident);
}

bool SensorDevice::isClientDisabledLocked(void* ident) {
    return mDisabledClients.indexOf(ident) >= 0;
}

bool SensorDevice::isSensorActive(int handle) const {
    Mutex::Autolock _l(mLock);
    ssize_t activationIndex = mActivationCount.indexOfKey(handle);
    if (activationIndex < 0) {
        return false;
    }
    return mActivationCount.valueAt(activationIndex).numActiveClients() > 0;
}

void SensorDevice::enableAllSensors() {
    if (mSensors == nullptr) return;
    Mutex::Autolock _l(mLock);
    mDisabledClients.clear();
    ALOGI("cleared mDisabledClients");
    for (size_t i = 0; i< mActivationCount.size(); ++i) {
        Info& info = mActivationCount.editValueAt(i);
        if (info.batchParams.isEmpty()) continue;
        info.selectBatchParams();
        const int sensor_handle = mActivationCount.keyAt(i);
        ALOGD_IF(DEBUG_CONNECTIONS, "\t>> reenable actuating h/w sensor enable handle=%d ",
                   sensor_handle);
        status_t err = checkReturnAndGetStatus(mSensors->batch(
                sensor_handle,
                info.bestBatchParams.mTSample,
                info.bestBatchParams.mTBatch));
        ALOGE_IF(err, "Error calling batch on sensor %d (%s)", sensor_handle, strerror(-err));

        if (err == NO_ERROR) {
            err = checkReturnAndGetStatus(mSensors->activate(sensor_handle, 1 /* enabled */));
            ALOGE_IF(err, "Error activating sensor %d (%s)", sensor_handle, strerror(-err));
        }

        if (err == NO_ERROR) {
            info.isActive = true;
        }
    }
}

void SensorDevice::disableAllSensors() {
    if (mSensors == nullptr) return;
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i< mActivationCount.size(); ++i) {
        Info& info = mActivationCount.editValueAt(i);
        // Check if this sensor has been activated previously and disable it.
        if (info.batchParams.size() > 0) {
           const int sensor_handle = mActivationCount.keyAt(i);
           ALOGD_IF(DEBUG_CONNECTIONS, "\t>> actuating h/w sensor disable handle=%d ",
                   sensor_handle);
           checkReturn(mSensors->activate(sensor_handle, 0 /* enabled */));

           // Add all the connections that were registered for this sensor to the disabled
           // clients list.
           for (size_t j = 0; j < info.batchParams.size(); ++j) {
               mDisabledClients.add(info.batchParams.keyAt(j));
               ALOGI("added %p to mDisabledClients", info.batchParams.keyAt(j));
           }

           info.isActive = false;
        }
    }
}

status_t SensorDevice::injectSensorData(
        const sensors_event_t *injected_sensor_event) {
    if (mSensors == nullptr) return NO_INIT;
    ALOGD_IF(DEBUG_CONNECTIONS,
            "sensor_event handle=%d ts=%" PRId64 " data=%.2f, %.2f, %.2f %.2f %.2f %.2f",
            injected_sensor_event->sensor,
            injected_sensor_event->timestamp, injected_sensor_event->data[0],
            injected_sensor_event->data[1], injected_sensor_event->data[2],
            injected_sensor_event->data[3], injected_sensor_event->data[4],
            injected_sensor_event->data[5]);

    Event ev;
    convertFromSensorEvent(*injected_sensor_event, &ev);

    return checkReturnAndGetStatus(mSensors->injectSensorData(ev));
}

status_t SensorDevice::setMode(uint32_t mode) {
    if (mSensors == nullptr) return NO_INIT;
    return checkReturnAndGetStatus(mSensors->setOperationMode(
            static_cast<hardware::sensors::V1_0::OperationMode>(mode)));
}

int32_t SensorDevice::registerDirectChannel(const sensors_direct_mem_t* memory) {
    if (mSensors == nullptr) return NO_INIT;
    Mutex::Autolock _l(mLock);

    SharedMemType type;
    switch (memory->type) {
        case SENSOR_DIRECT_MEM_TYPE_ASHMEM:
            type = SharedMemType::ASHMEM;
            break;
        case SENSOR_DIRECT_MEM_TYPE_GRALLOC:
            type = SharedMemType::GRALLOC;
            break;
        default:
            return BAD_VALUE;
    }

    SharedMemFormat format;
    if (memory->format != SENSOR_DIRECT_FMT_SENSORS_EVENT) {
        return BAD_VALUE;
    }
    format = SharedMemFormat::SENSORS_EVENT;

    SharedMemInfo mem = {
        .type = type,
        .format = format,
        .size = static_cast<uint32_t>(memory->size),
        .memoryHandle = memory->handle,
    };

    int32_t ret;
    checkReturn(mSensors->registerDirectChannel(mem,
            [&ret](auto result, auto channelHandle) {
                if (result == Result::OK) {
                    ret = channelHandle;
                } else {
                    ret = statusFromResult(result);
                }
            }));
    return ret;
}

void SensorDevice::unregisterDirectChannel(int32_t channelHandle) {
    if (mSensors == nullptr) return;
    Mutex::Autolock _l(mLock);
    checkReturn(mSensors->unregisterDirectChannel(channelHandle));
}

int32_t SensorDevice::configureDirectChannel(int32_t sensorHandle,
        int32_t channelHandle, const struct sensors_direct_cfg_t *config) {
    if (mSensors == nullptr) return NO_INIT;
    Mutex::Autolock _l(mLock);

    RateLevel rate;
    switch(config->rate_level) {
        case SENSOR_DIRECT_RATE_STOP:
            rate = RateLevel::STOP;
            break;
        case SENSOR_DIRECT_RATE_NORMAL:
            rate = RateLevel::NORMAL;
            break;
        case SENSOR_DIRECT_RATE_FAST:
            rate = RateLevel::FAST;
            break;
        case SENSOR_DIRECT_RATE_VERY_FAST:
            rate = RateLevel::VERY_FAST;
            break;
        default:
            return BAD_VALUE;
    }

    int32_t ret;
    checkReturn(mSensors->configDirectReport(sensorHandle, channelHandle, rate,
            [&ret, rate] (auto result, auto token) {
                if (rate == RateLevel::STOP) {
                    ret = statusFromResult(result);
                } else {
                    if (result == Result::OK) {
                        ret = token;
                    } else {
                        ret = statusFromResult(result);
                    }
                }
            }));

    return ret;
}

// ---------------------------------------------------------------------------

int SensorDevice::Info::numActiveClients() const {
    SensorDevice& device(SensorDevice::getInstance());
    int num = 0;
    for (size_t i = 0; i < batchParams.size(); ++i) {
        if (!device.isClientDisabledLocked(batchParams.keyAt(i))) {
            ++num;
        }
    }
    return num;
}

status_t SensorDevice::Info::setBatchParamsForIdent(void* ident, int,
                                                    int64_t samplingPeriodNs,
                                                    int64_t maxBatchReportLatencyNs) {
    ssize_t index = batchParams.indexOfKey(ident);
    if (index < 0) {
        ALOGE("Info::setBatchParamsForIdent(ident=%p, period_ns=%" PRId64
              " timeout=%" PRId64 ") failed (%s)",
              ident, samplingPeriodNs, maxBatchReportLatencyNs, strerror(-index));
        return BAD_INDEX;
    }
    BatchParams& params = batchParams.editValueAt(index);
    params.mTSample = samplingPeriodNs;
    params.mTBatch = maxBatchReportLatencyNs;
    return NO_ERROR;
}

void SensorDevice::Info::selectBatchParams() {
    BatchParams bestParams; // default to max Tsample and max Tbatch
    SensorDevice& device(SensorDevice::getInstance());

    for (size_t i = 0; i < batchParams.size(); ++i) {
        if (device.isClientDisabledLocked(batchParams.keyAt(i))) {
            continue;
        }
        bestParams.merge(batchParams[i]);
    }
    // if mTBatch <= mTSample, it is in streaming mode. set mTbatch to 0 to demand this explicitly.
    if (bestParams.mTBatch <= bestParams.mTSample) {
        bestParams.mTBatch = 0;
    }
    bestBatchParams = bestParams;
}

ssize_t SensorDevice::Info::removeBatchParamsForIdent(void* ident) {
    ssize_t idx = batchParams.removeItem(ident);
    if (idx >= 0) {
        selectBatchParams();
    }
    return idx;
}

void SensorDevice::notifyConnectionDestroyed(void* ident) {
    Mutex::Autolock _l(mLock);
    mDisabledClients.remove(ident);
}

bool SensorDevice::isDirectReportSupported() const {
    return mIsDirectReportSupported;
}

void SensorDevice::convertToSensorEvent(
        const Event &src, sensors_event_t *dst) {
    ::android::hardware::sensors::V1_0::implementation::convertToSensorEvent(
            src, dst);

    if (src.sensorType == SensorType::DYNAMIC_SENSOR_META) {
        const DynamicSensorInfo &dyn = src.u.dynamic;

        dst->dynamic_sensor_meta.connected = dyn.connected;
        dst->dynamic_sensor_meta.handle = dyn.sensorHandle;
        if (dyn.connected) {
            auto it = mConnectedDynamicSensors.find(dyn.sensorHandle);
            CHECK(it != mConnectedDynamicSensors.end());

            dst->dynamic_sensor_meta.sensor = it->second;

            memcpy(dst->dynamic_sensor_meta.uuid,
                   dyn.uuid.data(),
                   sizeof(dst->dynamic_sensor_meta.uuid));
        }
    }
}

void SensorDevice::convertToSensorEvents(
        const hidl_vec<Event> &src,
        const hidl_vec<SensorInfo> &dynamicSensorsAdded,
        sensors_event_t *dst) {

    if (dynamicSensorsAdded.size() > 0) {
        onDynamicSensorsConnected(dynamicSensorsAdded);
    }

    for (size_t i = 0; i < src.size(); ++i) {
        convertToSensorEvent(src[i], &dst[i]);
    }
}

void SensorDevice::handleHidlDeath(const std::string & detail) {
    if (!mSensors->supportsMessageQueues()) {
        // restart is the only option at present.
        LOG_ALWAYS_FATAL("Abort due to ISensors hidl service failure, detail: %s.", detail.c_str());
    } else {
        ALOGD("ISensors HAL died, death recipient will attempt reconnect");
    }
}

status_t SensorDevice::checkReturnAndGetStatus(const Return<Result>& ret) {
    checkReturn(ret);
    return (!ret.isOk()) ? DEAD_OBJECT : statusFromResult(ret);
}

// ---------------------------------------------------------------------------
}; // namespace android
