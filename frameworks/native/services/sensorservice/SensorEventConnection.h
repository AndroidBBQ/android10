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

#ifndef ANDROID_SENSOR_EVENT_CONNECTION_H
#define ANDROID_SENSOR_EVENT_CONNECTION_H

#include <stdint.h>
#include <sys/types.h>
#include <unordered_map>

#include <utils/Vector.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <utils/threads.h>
#include <utils/AndroidThreads.h>
#include <utils/RefBase.h>
#include <utils/Looper.h>
#include <utils/String8.h>

#include <binder/BinderService.h>

#include <sensor/Sensor.h>
#include <sensor/BitTube.h>
#include <sensor/ISensorServer.h>
#include <sensor/ISensorEventConnection.h>

#include "SensorService.h"

namespace android {

class SensorService;

class SensorService::SensorEventConnection:
    public BnSensorEventConnection, public LooperCallback {

    friend class SensorService;

public:
    SensorEventConnection(const sp<SensorService>& service, uid_t uid, String8 packageName,
                          bool isDataInjectionMode, const String16& opPackageName,
                          bool hasSensorAccess);

    status_t sendEvents(sensors_event_t const* buffer, size_t count, sensors_event_t* scratch,
                        wp<const SensorEventConnection> const * mapFlushEventsToConnections = nullptr);
    bool hasSensor(int32_t handle) const;
    bool hasAnySensor() const;
    bool hasOneShotSensors() const;
    bool addSensor(int32_t handle);
    bool removeSensor(int32_t handle);
    void setFirstFlushPending(int32_t handle, bool value);
    void dump(String8& result);
    bool needsWakeLock();
    void resetWakeLockRefCount();
    String8 getPackageName() const;

    uid_t getUid() const { return mUid; }

    void setSensorAccess(const bool hasAccess);

private:
    virtual ~SensorEventConnection();
    virtual void onFirstRef();
    virtual sp<BitTube> getSensorChannel() const;
    virtual status_t enableDisable(int handle, bool enabled, nsecs_t samplingPeriodNs,
                                   nsecs_t maxBatchReportLatencyNs, int reservedFlags);
    virtual status_t setEventRate(int handle, nsecs_t samplingPeriodNs);
    virtual status_t flush();
    virtual int32_t configureChannel(int handle, int rateLevel);
    virtual void destroy();

    // Count the number of flush complete events which are about to be dropped in the buffer.
    // Increment mPendingFlushEventsToSend in mSensorInfo. These flush complete events will be sent
    // separately before the next batch of events.
    void countFlushCompleteEventsLocked(sensors_event_t const* scratch, int numEventsDropped);

    // Check if there are any wake up events in the buffer. If yes, return the index of the first
    // wake_up sensor event in the buffer else return -1.  This wake_up sensor event will have the
    // flag WAKE_UP_SENSOR_EVENT_NEEDS_ACK set. Exactly one event per packet will have the wake_up
    // flag set. SOCK_SEQPACKET ensures that either the entire packet is read or dropped.
    int findWakeUpSensorEventLocked(sensors_event_t const* scratch, int count);

    // Send pending flush_complete events. There may have been flush_complete_events that are
    // dropped which need to be sent separately before other events. On older HALs (1_0) this method
    // emulates the behavior of flush().
    void sendPendingFlushEventsLocked();

    // Writes events from mEventCache to the socket.
    void writeToSocketFromCache();

    // Compute the approximate cache size from the FIFO sizes of various sensors registered for this
    // connection. Wake up and non-wake up sensors have separate FIFOs but FIFO may be shared
    // amongst wake-up sensors and non-wake up sensors.
    int computeMaxCacheSizeLocked() const;

    // When more sensors register, the maximum cache size desired may change.  Compute max cache
    // size, reallocate memory and copy over events from the older cache.
    void reAllocateCacheLocked(sensors_event_t const* scratch, int count);

    // Add the events to the cache. If the cache would be exceeded, drop events at the beginning of
    // the cache.
    void appendEventsToCacheLocked(sensors_event_t const* events, int count);

    // LooperCallback method. If there is data to read on this fd, it is an ack from the app that it
    // has read events from a wake up sensor, decrement mWakeLockRefCount.  If this fd is available
    // for writing send the data from the cache.
    virtual int handleEvent(int fd, int events, void* data);

    // Increment mPendingFlushEventsToSend for the given sensor handle.
    void incrementPendingFlushCount(int32_t handle);

    // Add or remove the file descriptor associated with the BitTube to the looper. If mDead is set
    // to true or there are no more sensors for this connection, the file descriptor is removed if
    // it has been previously added to the Looper. Depending on the state of the connection FD may
    // be added to the Looper. The flags to set are determined by the internal state of the
    // connection. FDs are added to the looper when wake-up sensors are registered (to poll for
    // acknowledgements) and when write fails on the socket when there are too many error and the
    // other end hangs up or when this client unregisters for this connection.
    void updateLooperRegistration(const sp<Looper>& looper); void
            updateLooperRegistrationLocked(const sp<Looper>& looper);

    // Returns whether sensor access is available based on both the uid being active and sensor
    // privacy not being enabled.
    bool hasSensorAccess();

    // Call noteOp for the sensor if the sensor requires a permission
    bool noteOpIfRequired(const sensors_event_t& event);

    sp<SensorService> const mService;
    sp<BitTube> mChannel;
    uid_t mUid;
    mutable Mutex mConnectionLock;
    // Number of events from wake up sensors which are still pending and haven't been delivered to
    // the corresponding application. It is incremented by one unit for each write to the socket.
    uint32_t mWakeLockRefCount;

    // If this flag is set to true, it means that the file descriptor associated with the BitTube
    // has been added to the Looper in SensorService. This flag is typically set when this
    // connection has wake-up sensors associated with it or when write has failed on this connection
    // and we're storing some events in the cache.
    bool mHasLooperCallbacks;
    // If there are any errors associated with the Looper this flag is set to true and
    // mWakeLockRefCount is reset to zero. needsWakeLock method will always return false, if this
    // flag is set.
    bool mDead;

    bool mDataInjectionMode;
    struct FlushInfo {
        // The number of flush complete events dropped for this sensor is stored here.  They are
        // sent separately before the next batch of events.
        int mPendingFlushEventsToSend;

        // Every activate is preceded by a flush. Only after the first flush complete is received,
        // the events for the sensor are sent on that *connection*.
        bool mFirstFlushPending;

        FlushInfo() : mPendingFlushEventsToSend(0), mFirstFlushPending(false) {}
    };
    // protected by SensorService::mLock. Key for this vector is the sensor handle.
    KeyedVector<int, FlushInfo> mSensorInfo;

    sensors_event_t *mEventCache;
    int mCacheSize, mMaxCacheSize;
    int64_t mTimeOfLastEventDrop;
    int mEventsDropped;
    String8 mPackageName;
    const String16 mOpPackageName;
#if DEBUG_CONNECTIONS
    int mEventsReceived, mEventsSent, mEventsSentFromCache;
    int mTotalAcksNeeded, mTotalAcksReceived;
#endif

    mutable Mutex mDestroyLock;
    bool mDestroyed;
    bool mHasSensorAccess;

    // Store a mapping of sensor handles to required AppOp for a sensor. This map only contains a
    // valid mapping for sensors that require a permission in order to reduce the lookup time.
    std::unordered_map<int32_t, int32_t> mHandleToAppOp;
};

} // namepsace android

#endif // ANDROID_SENSOR_EVENT_CONNECTION_H

