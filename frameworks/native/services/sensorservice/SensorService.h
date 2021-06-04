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

#ifndef ANDROID_SENSOR_SERVICE_H
#define ANDROID_SENSOR_SERVICE_H

#include "SensorList.h"
#include "RecentEventLogger.h"

#include <android-base/macros.h>
#include <binder/AppOpsManager.h>
#include <binder/BinderService.h>
#include <binder/IUidObserver.h>
#include <cutils/compiler.h>
#include <cutils/multiuser.h>
#include <sensor/ISensorServer.h>
#include <sensor/ISensorEventConnection.h>
#include <sensor/Sensor.h>
#include "android/hardware/BnSensorPrivacyListener.h"

#include <utils/AndroidThreads.h>
#include <utils/KeyedVector.h>
#include <utils/Looper.h>
#include <utils/SortedVector.h>
#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/threads.h>

#include <stdint.h>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#if __clang__
// Clang warns about SensorEventConnection::dump hiding BBinder::dump. The cause isn't fixable
// without changing the API, so let's tell clang this is indeed intentional.
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

// ---------------------------------------------------------------------------
#define IGNORE_HARDWARE_FUSION  false
#define DEBUG_CONNECTIONS   false
// Max size is 100 KB which is enough to accept a batch of about 1000 events.
#define MAX_SOCKET_BUFFER_SIZE_BATCHED (100 * 1024)
// For older HALs which don't support batching, use a smaller socket buffer size.
#define SOCKET_BUFFER_SIZE_NON_BATCHED (4 * 1024)

#define SENSOR_REGISTRATIONS_BUF_SIZE 200

namespace android {
// ---------------------------------------------------------------------------
class SensorInterface;

class SensorService :
        public BinderService<SensorService>,
        public BnSensorServer,
        protected Thread
{
    // nested class/struct for internal use
    class SensorEventConnection;
    class SensorDirectConnection;

public:
    void cleanupConnection(SensorEventConnection* connection);
    void cleanupConnection(SensorDirectConnection* c);

    status_t enable(const sp<SensorEventConnection>& connection, int handle,
                    nsecs_t samplingPeriodNs,  nsecs_t maxBatchReportLatencyNs, int reservedFlags,
                    const String16& opPackageName);

    status_t disable(const sp<SensorEventConnection>& connection, int handle);

    status_t setEventRate(const sp<SensorEventConnection>& connection, int handle, nsecs_t ns,
                          const String16& opPackageName);

    status_t flushSensor(const sp<SensorEventConnection>& connection,
                         const String16& opPackageName);


    virtual status_t shellCommand(int in, int out, int err, Vector<String16>& args);

private:
    friend class BinderService<SensorService>;

    // nested class/struct for internal use
    class ConnectionSafeAutolock;
    class SensorConnectionHolder;
    class SensorEventAckReceiver;
    class SensorRecord;
    class SensorRegistrationInfo;

    // Promoting a SensorEventConnection or SensorDirectConnection from wp to sp must be done with
    // mLock held, but destroying that sp must be done unlocked to avoid a race condition that
    // causes a deadlock (remote dies while we hold a local sp, then our decStrong() call invokes
    // the dtor -> cleanupConnection() tries to re-lock the mutex). This class ensures safe usage
    // by wrapping a Mutex::Autolock on SensorService's mLock, plus vectors that hold promoted sp<>
    // references until the lock is released, when they are safely destroyed.
    // All read accesses to the connection lists in mConnectionHolder must be done via this class.
    class ConnectionSafeAutolock final {
    public:
        // Returns a list of non-null promoted connection references
        const std::vector<sp<SensorEventConnection>>& getActiveConnections();
        const std::vector<sp<SensorDirectConnection>>& getDirectConnections();

    private:
        // Constructed via SensorConnectionHolder::lock()
        friend class SensorConnectionHolder;
        explicit ConnectionSafeAutolock(SensorConnectionHolder& holder, Mutex& mutex);
        DISALLOW_IMPLICIT_CONSTRUCTORS(ConnectionSafeAutolock);

        // NOTE: Order of these members is important, as the destructor for non-static members
        // get invoked in the reverse order of their declaration. Here we are relying on the
        // Autolock to be destroyed *before* the vectors, so the sp<> objects are destroyed without
        // the lock held, which avoids the deadlock.
        SensorConnectionHolder& mConnectionHolder;
        std::vector<std::vector<sp<SensorEventConnection>>> mReferencedActiveConnections;
        std::vector<std::vector<sp<SensorDirectConnection>>> mReferencedDirectConnections;
        Mutex::Autolock mAutolock;

        template<typename ConnectionType>
        const std::vector<sp<ConnectionType>>& getConnectionsHelper(
                const SortedVector<wp<ConnectionType>>& connectionList,
                std::vector<std::vector<sp<ConnectionType>>>* referenceHolder);
    };

    // Encapsulates the collection of active SensorEventConection and SensorDirectConnection
    // references. Write access is done through this class with mLock held, but all read access
    // must be routed through ConnectionSafeAutolock.
    class SensorConnectionHolder {
    public:
        void addEventConnectionIfNotPresent(const sp<SensorEventConnection>& connection);
        void removeEventConnection(const wp<SensorEventConnection>& connection);

        void addDirectConnection(const sp<SensorDirectConnection>& connection);
        void removeDirectConnection(const wp<SensorDirectConnection>& connection);

        // Pass in the mutex that protects this connection holder; acquires the lock and returns an
        // object that can be used to safely read the lists of connections
        ConnectionSafeAutolock lock(Mutex& mutex);

    private:
        friend class ConnectionSafeAutolock;
        SortedVector< wp<SensorEventConnection> > mActiveConnections;
        SortedVector< wp<SensorDirectConnection> > mDirectConnections;
    };

    // If accessing a sensor we need to make sure the UID has access to it. If
    // the app UID is idle then it cannot access sensors and gets no trigger
    // events, no on-change events, flush event behavior does not change, and
    // recurring events are the same as the first one delivered in idle state
    // emulating no sensor change. As soon as the app UID transitions to an
    // active state we will start reporting events as usual and vise versa. This
    // approach transparently handles observing sensors while the app UID transitions
    // between idle/active state avoiding to get stuck in a state receiving sensor
    // data while idle or not receiving sensor data while active.
    class UidPolicy : public BnUidObserver {
        public:
            explicit UidPolicy(wp<SensorService> service)
                    : mService(service) {}
            void registerSelf();
            void unregisterSelf();

            bool isUidActive(uid_t uid);

            void onUidGone(uid_t uid, bool disabled);
            void onUidActive(uid_t uid);
            void onUidIdle(uid_t uid, bool disabled);
            void onUidStateChanged(uid_t uid __unused, int32_t procState __unused,
                                   int64_t procStateSeq __unused) {}

            void addOverrideUid(uid_t uid, bool active);
            void removeOverrideUid(uid_t uid);
        private:
            bool isUidActiveLocked(uid_t uid);
            void updateOverrideUid(uid_t uid, bool active, bool insert);

            Mutex mUidLock;
            wp<SensorService> mService;
            std::unordered_set<uid_t> mActiveUids;
            std::unordered_map<uid_t, bool> mOverrideUids;
    };

    // Sensor privacy allows a user to disable access to all sensors on the device. When
    // enabled sensor privacy will prevent all apps, including active apps, from accessing
    // sensors, they will not receive trigger nor on-change events, flush event behavior
    // does not change, and recurring events are the same as the first one delivered when
    // sensor privacy was enabled. All sensor direct connections will be stopped as well
    // and new direct connections will not be allowed while sensor privacy is enabled.
    // Once sensor privacy is disabled access to sensors will be restored for active
    // apps, previously stopped direct connections will be restarted, and new direct
    // connections will be allowed again.
    class SensorPrivacyPolicy : public hardware::BnSensorPrivacyListener {
        public:
            explicit SensorPrivacyPolicy(wp<SensorService> service) : mService(service) {}
            void registerSelf();
            void unregisterSelf();

            bool isSensorPrivacyEnabled();

            binder::Status onSensorPrivacyChanged(bool enabled);

        private:
            wp<SensorService> mService;
            std::atomic_bool mSensorPrivacyEnabled;
    };

    enum Mode {
       // The regular operating mode where any application can register/unregister/call flush on
       // sensors.
       NORMAL = 0,
       // This mode is only used for testing purposes. Not all HALs support this mode. In this mode,
       // the HAL ignores the sensor data provided by physical sensors and accepts the data that is
       // injected from the SensorService as if it were the real sensor data. This mode is primarily
       // used for testing various algorithms like vendor provided SensorFusion, Step Counter and
       // Step Detector etc. Typically in this mode, there will be a client (a
       // SensorEventConnection) which will be injecting sensor data into the HAL. Normal apps can
       // unregister and register for any sensor that supports injection. Registering to sensors
       // that do not support injection will give an error.  TODO(aakella) : Allow exactly one
       // client to inject sensor data at a time.
       DATA_INJECTION = 1,
       // This mode is used only for testing sensors. Each sensor can be tested in isolation with
       // the required sampling_rate and maxReportLatency parameters without having to think about
       // the data rates requested by other applications. End user devices are always expected to be
       // in NORMAL mode. When this mode is first activated, all active sensors from all connections
       // are disabled. Calling flush() will return an error. In this mode, only the requests from
       // selected apps whose package names are whitelisted are allowed (typically CTS apps).  Only
       // these apps can register/unregister/call flush() on sensors. If SensorService switches to
       // NORMAL mode again, all sensors that were previously registered to are activated with the
       // corresponding paramaters if the application hasn't unregistered for sensors in the mean
       // time.  NOTE: Non whitelisted app whose sensors were previously deactivated may still
       // receive events if a whitelisted app requests data from the same sensor.
       RESTRICTED = 2

      // State Transitions supported.
      //     RESTRICTED   <---  NORMAL   ---> DATA_INJECTION
      //                  --->           <---

      // Shell commands to switch modes in SensorService.
      // 1) Put SensorService in RESTRICTED mode with packageName .cts. If it is already in
      // restricted mode it is treated as a NO_OP (and packageName is NOT changed).
      //
      //     $ adb shell dumpsys sensorservice restrict .cts.
      //
      // 2) Put SensorService in DATA_INJECTION mode with packageName .xts. If it is already in
      // data_injection mode it is treated as a NO_OP (and packageName is NOT changed).
      //
      //     $ adb shell dumpsys sensorservice data_injection .xts.
      //
      // 3) Reset sensorservice back to NORMAL mode.
      //     $ adb shell dumpsys sensorservice enable
    };

    static const char* WAKE_LOCK_NAME;
    static char const* getServiceName() ANDROID_API { return "sensorservice"; }
    SensorService() ANDROID_API;
    virtual ~SensorService();

    virtual void onFirstRef();

    // Thread interface
    virtual bool threadLoop();

    // ISensorServer interface
    virtual Vector<Sensor> getSensorList(const String16& opPackageName);
    virtual Vector<Sensor> getDynamicSensorList(const String16& opPackageName);
    virtual sp<ISensorEventConnection> createSensorEventConnection(
            const String8& packageName,
            int requestedMode, const String16& opPackageName);
    virtual int isDataInjectionEnabled();
    virtual sp<ISensorEventConnection> createSensorDirectConnection(const String16& opPackageName,
            uint32_t size, int32_t type, int32_t format, const native_handle *resource);
    virtual int setOperationParameter(
            int32_t handle, int32_t type, const Vector<float> &floats, const Vector<int32_t> &ints);
    virtual status_t dump(int fd, const Vector<String16>& args);
    String8 getSensorName(int handle) const;
    bool isVirtualSensor(int handle) const;
    sp<SensorInterface> getSensorInterfaceFromHandle(int handle) const;
    bool isWakeUpSensor(int type) const;
    void recordLastValueLocked(sensors_event_t const* buffer, size_t count);
    static void sortEventBuffer(sensors_event_t* buffer, size_t count);
    const Sensor& registerSensor(SensorInterface* sensor,
                                 bool isDebug = false, bool isVirtual = false);
    const Sensor& registerVirtualSensor(SensorInterface* sensor, bool isDebug = false);
    const Sensor& registerDynamicSensorLocked(SensorInterface* sensor, bool isDebug = false);
    bool unregisterDynamicSensorLocked(int handle);
    status_t cleanupWithoutDisable(const sp<SensorEventConnection>& connection, int handle);
    status_t cleanupWithoutDisableLocked(const sp<SensorEventConnection>& connection, int handle);
    void cleanupAutoDisabledSensorLocked(const sp<SensorEventConnection>& connection,
            sensors_event_t const* buffer, const int count);
    static bool canAccessSensor(const Sensor& sensor, const char* operation,
            const String16& opPackageName);
    static bool hasPermissionForSensor(const Sensor& sensor);
    static int getTargetSdkVersion(const String16& opPackageName);
    // SensorService acquires a partial wakelock for delivering events from wake up sensors. This
    // method checks whether all the events from these wake up sensors have been delivered to the
    // corresponding applications, if yes the wakelock is released.
    void checkWakeLockState();
    void checkWakeLockStateLocked(ConnectionSafeAutolock* connLock);
    bool isWakeLockAcquired();
    bool isWakeUpSensorEvent(const sensors_event_t& event) const;

    sp<Looper> getLooper() const;

    // Reset mWakeLockRefCounts for all SensorEventConnections to zero. This may happen if
    // SensorService did not receive any acknowledgements from apps which have registered for
    // wake_up sensors.
    void resetAllWakeLockRefCounts();

    // Acquire or release wake_lock. If wake_lock is acquired, set the timeout in the looper to 5
    // seconds and wake the looper.
    void setWakeLockAcquiredLocked(bool acquire);

    // Send events from the event cache for this particular connection.
    void sendEventsFromCache(const sp<SensorEventConnection>& connection);

    // If SensorService is operating in RESTRICTED mode, only select whitelisted packages are
    // allowed to register for or call flush on sensors. Typically only cts test packages are
    // allowed.
    bool isWhiteListedPackage(const String8& packageName);
    bool isOperationPermitted(const String16& opPackageName);

    // Reset the state of SensorService to NORMAL mode.
    status_t resetToNormalMode();
    status_t resetToNormalModeLocked();

    // Transforms the UUIDs for all the sensors into proper IDs.
    void makeUuidsIntoIdsForSensorList(Vector<Sensor> &sensorList) const;
    // Gets the appropriate ID from the given UUID.
    int32_t getIdFromUuid(const Sensor::uuid_t &uuid) const;
    // Either read from storage or create a new one.
    static bool initializeHmacKey();

    // Enable SCHED_FIFO priority for thread
    void enableSchedFifoMode();

    // Sets whether the given UID can get sensor data
    void setSensorAccess(uid_t uid, bool hasAccess);

    // Overrides the UID state as if it is idle
    status_t handleSetUidState(Vector<String16>& args, int err);
    // Clears the override for the UID state
    status_t handleResetUidState(Vector<String16>& args, int err);
    // Gets the UID state
    status_t handleGetUidState(Vector<String16>& args, int out, int err);
    // Prints the shell command help
    status_t printHelp(int out);

    // temporarily stops all active direct connections and disables all sensors
    void disableAllSensors();
    void disableAllSensorsLocked(ConnectionSafeAutolock* connLock);
    // restarts the previously stopped direct connections and enables all sensors
    void enableAllSensors();
    void enableAllSensorsLocked(ConnectionSafeAutolock* connLock);

    static uint8_t sHmacGlobalKey[128];
    static bool sHmacGlobalKeyIsValid;

    SensorServiceUtil::SensorList mSensors;
    status_t mInitCheck;

    // Socket buffersize used to initialize BitTube. This size depends on whether batching is
    // supported or not.
    uint32_t mSocketBufferSize;
    sp<Looper> mLooper;
    sp<SensorEventAckReceiver> mAckReceiver;

    // protected by mLock
    mutable Mutex mLock;
    DefaultKeyedVector<int, SensorRecord*> mActiveSensors;
    std::unordered_set<int> mActiveVirtualSensors;
    SensorConnectionHolder mConnectionHolder;
    bool mWakeLockAcquired;
    sensors_event_t *mSensorEventBuffer, *mSensorEventScratch;
    // WARNING: these SensorEventConnection instances must not be promoted to sp, except via
    // modification to add support for them in ConnectionSafeAutolock
    wp<const SensorEventConnection> * mMapFlushEventsToConnections;
    std::unordered_map<int, SensorServiceUtil::RecentEventLogger*> mRecentEvent;
    Mode mCurrentOperatingMode;

    // This packagaName is set when SensorService is in RESTRICTED or DATA_INJECTION mode. Only
    // applications with this packageName are allowed to activate/deactivate or call flush on
    // sensors. To run CTS this is can be set to ".cts." and only CTS tests will get access to
    // sensors.
    String8 mWhiteListedPackage;

    int mNextSensorRegIndex;
    Vector<SensorRegistrationInfo> mLastNSensorRegistrations;

    sp<UidPolicy> mUidPolicy;
    sp<SensorPrivacyPolicy> mSensorPrivacyPolicy;

    static AppOpsManager sAppOpsManager;
    static std::map<String16, int> sPackageTargetVersion;
    static Mutex sPackageTargetVersionLock;
};

} // namespace android
#endif // ANDROID_SENSOR_SERVICE_H
