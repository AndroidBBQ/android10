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

#ifndef ANDROID_SENSOR_DEVICE_H
#define ANDROID_SENSOR_DEVICE_H

#include "SensorDeviceUtils.h"
#include "SensorServiceUtils.h"
#include "SensorsWrapper.h"

#include <fmq/MessageQueue.h>
#include <sensor/SensorEventQueue.h>
#include <sensor/Sensor.h>
#include <stdint.h>
#include <sys/types.h>
#include <utils/KeyedVector.h>
#include <utils/Singleton.h>
#include <utils/String8.h>
#include <utils/Timers.h>

#include <string>
#include <unordered_map>
#include <algorithm> //std::max std::min

#include "RingBuffer.h"

// ---------------------------------------------------------------------------

namespace android {

// ---------------------------------------------------------------------------
class SensorsHalDeathReceivier : public android::hardware::hidl_death_recipient {
    virtual void serviceDied(uint64_t cookie,
                             const wp<::android::hidl::base::V1_0::IBase>& service) override;
};

class SensorDevice : public Singleton<SensorDevice>,
                     public SensorServiceUtil::Dumpable {
public:
    class HidlTransportErrorLog {
     public:

        HidlTransportErrorLog() {
            mTs = 0;
            mCount = 0;
        }

        HidlTransportErrorLog(time_t ts, int count) {
            mTs = ts;
            mCount = count;
        }

        String8 toString() const {
            String8 result;
            struct tm *timeInfo = localtime(&mTs);
            result.appendFormat("%02d:%02d:%02d :: %d", timeInfo->tm_hour, timeInfo->tm_min,
                                timeInfo->tm_sec, mCount);
            return result;
        }

    private:
        time_t mTs; // timestamp of the error
        int mCount;   // number of transport errors observed
    };

    ~SensorDevice();
    void prepareForReconnect();
    void reconnect();

    ssize_t getSensorList(sensor_t const** list);

    void handleDynamicSensorConnection(int handle, bool connected);
    status_t initCheck() const;
    int getHalDeviceVersion() const;

    ssize_t poll(sensors_event_t* buffer, size_t count);
    void writeWakeLockHandled(uint32_t count);

    status_t activate(void* ident, int handle, int enabled);
    status_t batch(void* ident, int handle, int flags, int64_t samplingPeriodNs,
                   int64_t maxBatchReportLatencyNs);
    // Call batch with timeout zero instead of calling setDelay() for newer devices.
    status_t setDelay(void* ident, int handle, int64_t ns);
    status_t flush(void* ident, int handle);
    status_t setMode(uint32_t mode);

    bool isDirectReportSupported() const;
    int32_t registerDirectChannel(const sensors_direct_mem_t *memory);
    void unregisterDirectChannel(int32_t channelHandle);
    int32_t configureDirectChannel(int32_t sensorHandle,
            int32_t channelHandle, const struct sensors_direct_cfg_t *config);

    void disableAllSensors();
    void enableAllSensors();
    void autoDisable(void *ident, int handle);

    status_t injectSensorData(const sensors_event_t *event);
    void notifyConnectionDestroyed(void *ident);

    using Result = ::android::hardware::sensors::V1_0::Result;
    hardware::Return<void> onDynamicSensorsConnected(
            const hardware::hidl_vec<hardware::sensors::V1_0::SensorInfo> &dynamicSensorsAdded);
    hardware::Return<void> onDynamicSensorsDisconnected(
            const hardware::hidl_vec<int32_t> &dynamicSensorHandlesRemoved);

    bool isReconnecting() const {
        return mReconnecting;
    }

    bool isSensorActive(int handle) const;

    // Dumpable
    virtual std::string dump() const;
private:
    friend class Singleton<SensorDevice>;

    sp<SensorServiceUtil::ISensorsWrapper> mSensors;
    Vector<sensor_t> mSensorList;
    std::unordered_map<int32_t, sensor_t*> mConnectedDynamicSensors;

    static const nsecs_t MINIMUM_EVENTS_PERIOD =   1000000; // 1000 Hz
    mutable Mutex mLock; // protect mActivationCount[].batchParams
    // fixed-size array after construction

    // Struct to store all the parameters(samplingPeriod, maxBatchReportLatency and flags) from
    // batch call. For continous mode clients, maxBatchReportLatency is set to zero.
    struct BatchParams {
      nsecs_t mTSample, mTBatch;
      BatchParams() : mTSample(INT64_MAX), mTBatch(INT64_MAX) {}
      BatchParams(nsecs_t tSample, nsecs_t tBatch): mTSample(tSample), mTBatch(tBatch) {}
      bool operator != (const BatchParams& other) {
          return !(mTSample == other.mTSample && mTBatch == other.mTBatch);
      }
      // Merge another parameter with this one. The updated mTSample will be the min of the two.
      // The update mTBatch will be the min of original mTBatch and the apparent batch period
      // of the other. the apparent batch is the maximum of mTBatch and mTSample,
      void merge(const BatchParams &other) {
          mTSample = std::min(mTSample, other.mTSample);
          mTBatch = std::min(mTBatch, std::max(other.mTBatch, other.mTSample));
      }
    };

    // Store batch parameters in the KeyedVector and the optimal batch_rate and timeout in
    // bestBatchParams. For every batch() call corresponding params are stored in batchParams
    // vector. A continuous mode request is batch(... timeout=0 ..) followed by activate(). A batch
    // mode request is batch(... timeout > 0 ...) followed by activate().
    // Info is a per-sensor data structure which contains the batch parameters for each client that
    // has registered for this sensor.
    struct Info {
        BatchParams bestBatchParams;
        // Key is the unique identifier(ident) for each client, value is the batch parameters
        // requested by the client.
        KeyedVector<void*, BatchParams> batchParams;

        // Flag to track if the sensor is active
        bool isActive = false;

        // Sets batch parameters for this ident. Returns error if this ident is not already present
        // in the KeyedVector above.
        status_t setBatchParamsForIdent(void* ident, int flags, int64_t samplingPeriodNs,
                                        int64_t maxBatchReportLatencyNs);
        // Finds the optimal parameters for batching and stores them in bestBatchParams variable.
        void selectBatchParams();
        // Removes batchParams for an ident and re-computes bestBatchParams. Returns the index of
        // the removed ident. If index >=0, ident is present and successfully removed.
        ssize_t removeBatchParamsForIdent(void* ident);

        int numActiveClients() const;
    };
    DefaultKeyedVector<int, Info> mActivationCount;

    // Keep track of any hidl transport failures
    SensorServiceUtil::RingBuffer<HidlTransportErrorLog> mHidlTransportErrors;
    int mTotalHidlTransportErrors;

    // Use this vector to determine which client is activated or deactivated.
    SortedVector<void *> mDisabledClients;
    SensorDevice();
    bool connectHidlService();
    void initializeSensorList();
    void reactivateSensors(const DefaultKeyedVector<int, Info>& previousActivations);
    static bool sensorHandlesChanged(const Vector<sensor_t>& oldSensorList,
                                     const Vector<sensor_t>& newSensorList);
    static bool sensorIsEquivalent(const sensor_t& prevSensor, const sensor_t& newSensor);

    enum HalConnectionStatus {
        CONNECTED, // Successfully connected to the HAL
        DOES_NOT_EXIST, // Could not find the HAL
        FAILED_TO_CONNECT, // Found the HAL but failed to connect/initialize
        UNKNOWN,
    };
    HalConnectionStatus connectHidlServiceV1_0();
    HalConnectionStatus connectHidlServiceV2_0();

    ssize_t pollHal(sensors_event_t* buffer, size_t count);
    ssize_t pollFmq(sensors_event_t* buffer, size_t count);
    status_t activateLocked(void* ident, int handle, int enabled);
    status_t batchLocked(void* ident, int handle, int flags, int64_t samplingPeriodNs,
                         int64_t maxBatchReportLatencyNs);

    void handleHidlDeath(const std::string &detail);
    template<typename T>
    void checkReturn(const Return<T>& ret) {
        if (!ret.isOk()) {
            handleHidlDeath(ret.description());
        }
    }
    status_t checkReturnAndGetStatus(const Return<Result>& ret);
    //TODO(b/67425500): remove waiter after bug is resolved.
    sp<SensorDeviceUtils::HidlServiceRegistrationWaiter> mRestartWaiter;

    bool isClientDisabled(void* ident);
    bool isClientDisabledLocked(void* ident);

    using Event = hardware::sensors::V1_0::Event;
    using SensorInfo = hardware::sensors::V1_0::SensorInfo;

    void convertToSensorEvent(const Event &src, sensors_event_t *dst);

    void convertToSensorEvents(
            const hardware::hidl_vec<Event> &src,
            const hardware::hidl_vec<SensorInfo> &dynamicSensorsAdded,
            sensors_event_t *dst);

    bool mIsDirectReportSupported;

    typedef hardware::MessageQueue<Event, hardware::kSynchronizedReadWrite> EventMessageQueue;
    typedef hardware::MessageQueue<uint32_t, hardware::kSynchronizedReadWrite> WakeLockQueue;
    std::unique_ptr<EventMessageQueue> mEventQueue;
    std::unique_ptr<WakeLockQueue> mWakeLockQueue;

    hardware::EventFlag* mEventQueueFlag;
    hardware::EventFlag* mWakeLockQueueFlag;

    std::array<Event, SensorEventQueue::MAX_RECEIVE_BUFFER_EVENT_COUNT> mEventBuffer;

    sp<SensorsHalDeathReceivier> mSensorsHalDeathReceiver;
    std::atomic_bool mReconnecting;
};

// ---------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_SENSOR_DEVICE_H
