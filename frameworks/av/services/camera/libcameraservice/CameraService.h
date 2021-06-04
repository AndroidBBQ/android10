/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA_CAMERASERVICE_H
#define ANDROID_SERVERS_CAMERA_CAMERASERVICE_H

#include <android/hardware/BnCameraService.h>
#include <android/hardware/BnSensorPrivacyListener.h>
#include <android/hardware/ICameraServiceListener.h>
#include <android/hardware/ICameraServiceProxy.h>

#include <cutils/multiuser.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <binder/AppOpsManager.h>
#include <binder/BinderService.h>
#include <binder/IAppOpsCallback.h>
#include <binder/IUidObserver.h>
#include <hardware/camera.h>

#include <android/hardware/camera/common/1.0/types.h>

#include <camera/VendorTagDescriptor.h>
#include <camera/CaptureResult.h>
#include <camera/CameraParameters.h>

#include "CameraFlashlight.h"

#include "common/CameraProviderManager.h"
#include "media/RingBuffer.h"
#include "utils/AutoConditionLock.h"
#include "utils/ClientManager.h"

#include <set>
#include <string>
#include <map>
#include <memory>
#include <utility>
#include <unordered_map>
#include <unordered_set>

namespace android {

extern volatile int32_t gLogLevel;

class MemoryHeapBase;
class MediaPlayer;

class CameraService :
    public BinderService<CameraService>,
    public virtual ::android::hardware::BnCameraService,
    public virtual IBinder::DeathRecipient,
    public virtual CameraProviderManager::StatusListener
{
    friend class BinderService<CameraService>;
    friend class CameraClient;
public:
    class Client;
    class BasicClient;

    // The effective API level.  The Camera2 API running in LEGACY mode counts as API_1.
    enum apiLevel {
        API_1 = 1,
        API_2 = 2
    };

    // 3 second busy timeout when other clients are connecting
    static const nsecs_t DEFAULT_CONNECT_TIMEOUT_NS = 3000000000;

    // 1 second busy timeout when other clients are disconnecting
    static const nsecs_t DEFAULT_DISCONNECT_TIMEOUT_NS = 1000000000;

    // Default number of messages to store in eviction log
    static const size_t DEFAULT_EVENT_LOG_LENGTH = 100;

    // Event log ID
    static const int SN_EVENT_LOG_ID = 0x534e4554;

    // Implementation of BinderService<T>
    static char const* getServiceName() { return "media.camera"; }

                        CameraService();
    virtual             ~CameraService();

    /////////////////////////////////////////////////////////////////////
    // HAL Callbacks - implements CameraProviderManager::StatusListener

    virtual void        onDeviceStatusChanged(const String8 &cameraId,
            hardware::camera::common::V1_0::CameraDeviceStatus newHalStatus) override;
    virtual void        onTorchStatusChanged(const String8& cameraId,
            hardware::camera::common::V1_0::TorchModeStatus newStatus) override;
    virtual void        onNewProviderRegistered() override;

    /////////////////////////////////////////////////////////////////////
    // ICameraService
    virtual binder::Status     getNumberOfCameras(int32_t type, int32_t* numCameras);

    virtual binder::Status     getCameraInfo(int cameraId,
            hardware::CameraInfo* cameraInfo);
    virtual binder::Status     getCameraCharacteristics(const String16& cameraId,
            CameraMetadata* cameraInfo);
    virtual binder::Status     getCameraVendorTagDescriptor(
            /*out*/
            hardware::camera2::params::VendorTagDescriptor* desc);
    virtual binder::Status     getCameraVendorTagCache(
            /*out*/
            hardware::camera2::params::VendorTagDescriptorCache* cache);

    virtual binder::Status     connect(const sp<hardware::ICameraClient>& cameraClient,
            int32_t cameraId, const String16& clientPackageName,
            int32_t clientUid, int clientPid,
            /*out*/
            sp<hardware::ICamera>* device);

    virtual binder::Status     connectLegacy(const sp<hardware::ICameraClient>& cameraClient,
            int32_t cameraId, int32_t halVersion,
            const String16& clientPackageName, int32_t clientUid,
            /*out*/
            sp<hardware::ICamera>* device);

    virtual binder::Status     connectDevice(
            const sp<hardware::camera2::ICameraDeviceCallbacks>& cameraCb, const String16& cameraId,
            const String16& clientPackageName, int32_t clientUid,
            /*out*/
            sp<hardware::camera2::ICameraDeviceUser>* device);

    virtual binder::Status    addListener(const sp<hardware::ICameraServiceListener>& listener,
            /*out*/
            std::vector<hardware::CameraStatus>* cameraStatuses);
    virtual binder::Status    removeListener(
            const sp<hardware::ICameraServiceListener>& listener);

    virtual binder::Status    getLegacyParameters(
            int32_t cameraId,
            /*out*/
            String16* parameters);

    virtual binder::Status    setTorchMode(const String16& cameraId, bool enabled,
            const sp<IBinder>& clientBinder);

    virtual binder::Status    notifySystemEvent(int32_t eventId,
            const std::vector<int32_t>& args);

    virtual binder::Status    notifyDeviceStateChange(int64_t newState);

    // OK = supports api of that version, -EOPNOTSUPP = does not support
    virtual binder::Status    supportsCameraApi(
            const String16& cameraId, int32_t apiVersion,
            /*out*/
            bool *isSupported);

    virtual binder::Status    isHiddenPhysicalCamera(
            const String16& cameraId,
            /*out*/
            bool *isSupported);

    // Extra permissions checks
    virtual status_t    onTransact(uint32_t code, const Parcel& data,
                                   Parcel* reply, uint32_t flags);

    virtual status_t    dump(int fd, const Vector<String16>& args);

    virtual status_t    shellCommand(int in, int out, int err, const Vector<String16>& args);

    binder::Status      addListenerHelper(const sp<hardware::ICameraServiceListener>& listener,
            /*out*/
            std::vector<hardware::CameraStatus>* cameraStatuses, bool isVendor = false);

    // Monitored UIDs availability notification
    void                notifyMonitoredUids();

    /////////////////////////////////////////////////////////////////////
    // Client functionality

    enum sound_kind {
        SOUND_SHUTTER = 0,
        SOUND_RECORDING_START = 1,
        SOUND_RECORDING_STOP = 2,
        NUM_SOUNDS
    };

    void                playSound(sound_kind kind);
    void                loadSoundLocked(sound_kind kind);
    void                decreaseSoundRef();
    void                increaseSoundRef();
    /**
     * Update the state of a given camera device (open/close/active/idle) with
     * the camera proxy service in the system service
     */
    static void         updateProxyDeviceState(
            int newState,
            const String8& cameraId,
            int facing,
            const String16& clientName,
            int apiLevel);

    /////////////////////////////////////////////////////////////////////
    // CameraDeviceFactory functionality
    int                 getDeviceVersion(const String8& cameraId, int* facing = NULL);

    /////////////////////////////////////////////////////////////////////
    // Shared utilities
    static binder::Status filterGetInfoErrorCode(status_t err);

    /////////////////////////////////////////////////////////////////////
    // CameraClient functionality

    class BasicClient : public virtual RefBase {
    public:
        virtual status_t       initialize(sp<CameraProviderManager> manager,
                const String8& monitorTags) = 0;
        virtual binder::Status disconnect();

        // because we can't virtually inherit IInterface, which breaks
        // virtual inheritance
        virtual sp<IBinder>    asBinderWrapper() = 0;

        // Return the remote callback binder object (e.g. ICameraDeviceCallbacks)
        sp<IBinder>            getRemote() {
            return mRemoteBinder;
        }

        // Disallows dumping over binder interface
        virtual status_t dump(int fd, const Vector<String16>& args);
        // Internal dump method to be called by CameraService
        virtual status_t dumpClient(int fd, const Vector<String16>& args) = 0;

        // Return the package name for this client
        virtual String16 getPackageName() const;

        // Notify client about a fatal error
        virtual void notifyError(int32_t errorCode,
                const CaptureResultExtras& resultExtras) = 0;

        // Get the UID of the application client using this
        virtual uid_t getClientUid() const;

        // Get the PID of the application client using this
        virtual int getClientPid() const;

        // Check what API level is used for this client. This is used to determine which
        // superclass this can be cast to.
        virtual bool canCastToApiClient(apiLevel level) const;

        // Block the client form using the camera
        virtual void block();
    protected:
        BasicClient(const sp<CameraService>& cameraService,
                const sp<IBinder>& remoteCallback,
                const String16& clientPackageName,
                const String8& cameraIdStr,
                int cameraFacing,
                int clientPid,
                uid_t clientUid,
                int servicePid);

        virtual ~BasicClient();

        // the instance is in the middle of destruction. When this is set,
        // the instance should not be accessed from callback.
        // CameraService's mClientLock should be acquired to access this.
        // - subclasses should set this to true in their destructors.
        bool                            mDestructionStarted;

        // these are initialized in the constructor.
        static sp<CameraService>        sCameraService;
        const String8                   mCameraIdStr;
        const int                       mCameraFacing;
        String16                        mClientPackageName;
        pid_t                           mClientPid;
        const uid_t                     mClientUid;
        const pid_t                     mServicePid;
        bool                            mDisconnected;

        // - The app-side Binder interface to receive callbacks from us
        sp<IBinder>                     mRemoteBinder;   // immutable after constructor

        // permissions management
        status_t                        startCameraOps();
        status_t                        finishCameraOps();

    private:
        std::unique_ptr<AppOpsManager>  mAppOpsManager = nullptr;

        class OpsCallback : public BnAppOpsCallback {
        public:
            explicit OpsCallback(wp<BasicClient> client);
            virtual void opChanged(int32_t op, const String16& packageName);

        private:
            wp<BasicClient> mClient;

        }; // class OpsCallback

        sp<OpsCallback> mOpsCallback;
        // Track whether startCameraOps was called successfully, to avoid
        // finishing what we didn't start.
        bool            mOpsActive;

        // IAppOpsCallback interface, indirected through opListener
        virtual void opChanged(int32_t op, const String16& packageName);
    }; // class BasicClient

    class Client : public hardware::BnCamera, public BasicClient
    {
    public:
        typedef hardware::ICameraClient TCamCallbacks;

        // ICamera interface (see ICamera for details)
        virtual binder::Status disconnect();
        virtual status_t      connect(const sp<hardware::ICameraClient>& client) = 0;
        virtual status_t      lock() = 0;
        virtual status_t      unlock() = 0;
        virtual status_t      setPreviewTarget(const sp<IGraphicBufferProducer>& bufferProducer)=0;
        virtual void          setPreviewCallbackFlag(int flag) = 0;
        virtual status_t      setPreviewCallbackTarget(
                const sp<IGraphicBufferProducer>& callbackProducer) = 0;
        virtual status_t      startPreview() = 0;
        virtual void          stopPreview() = 0;
        virtual bool          previewEnabled() = 0;
        virtual status_t      setVideoBufferMode(int32_t videoBufferMode) = 0;
        virtual status_t      startRecording() = 0;
        virtual void          stopRecording() = 0;
        virtual bool          recordingEnabled() = 0;
        virtual void          releaseRecordingFrame(const sp<IMemory>& mem) = 0;
        virtual status_t      autoFocus() = 0;
        virtual status_t      cancelAutoFocus() = 0;
        virtual status_t      takePicture(int msgType) = 0;
        virtual status_t      setParameters(const String8& params) = 0;
        virtual String8       getParameters() const = 0;
        virtual status_t      sendCommand(int32_t cmd, int32_t arg1, int32_t arg2) = 0;
        virtual status_t      setVideoTarget(const sp<IGraphicBufferProducer>& bufferProducer) = 0;

        // Interface used by CameraService
        Client(const sp<CameraService>& cameraService,
                const sp<hardware::ICameraClient>& cameraClient,
                const String16& clientPackageName,
                const String8& cameraIdStr,
                int api1CameraId,
                int cameraFacing,
                int clientPid,
                uid_t clientUid,
                int servicePid);
        ~Client();

        // return our camera client
        const sp<hardware::ICameraClient>&    getRemoteCallback() {
            return mRemoteCallback;
        }

        virtual sp<IBinder> asBinderWrapper() {
            return asBinder(this);
        }

        virtual void         notifyError(int32_t errorCode,
                                         const CaptureResultExtras& resultExtras);

        // Check what API level is used for this client. This is used to determine which
        // superclass this can be cast to.
        virtual bool canCastToApiClient(apiLevel level) const;
    protected:
        // Initialized in constructor

        // - The app-side Binder interface to receive callbacks from us
        sp<hardware::ICameraClient>               mRemoteCallback;

        int mCameraId;  // All API1 clients use integer camera IDs
    }; // class Client

    /**
     * A listener class that implements the LISTENER interface for use with a ClientManager, and
     * implements the following methods:
     *    void onClientRemoved(const ClientDescriptor<KEY, VALUE>& descriptor);
     *    void onClientAdded(const ClientDescriptor<KEY, VALUE>& descriptor);
     */
    class ClientEventListener {
    public:
        void onClientAdded(const resource_policy::ClientDescriptor<String8,
                sp<CameraService::BasicClient>>& descriptor);
        void onClientRemoved(const resource_policy::ClientDescriptor<String8,
                sp<CameraService::BasicClient>>& descriptor);
    }; // class ClientEventListener

    typedef std::shared_ptr<resource_policy::ClientDescriptor<String8,
            sp<CameraService::BasicClient>>> DescriptorPtr;

    /**
     * A container class for managing active camera clients that are using HAL devices.  Active
     * clients are represented by ClientDescriptor objects that contain strong pointers to the
     * actual BasicClient subclass binder interface implementation.
     *
     * This class manages the eviction behavior for the camera clients.  See the parent class
     * implementation in utils/ClientManager for the specifics of this behavior.
     */
    class CameraClientManager : public resource_policy::ClientManager<String8,
            sp<CameraService::BasicClient>, ClientEventListener> {
    public:
        CameraClientManager();
        virtual ~CameraClientManager();

        /**
         * Return a strong pointer to the active BasicClient for this camera ID, or an empty
         * if none exists.
         */
        sp<CameraService::BasicClient> getCameraClient(const String8& id) const;

        /**
         * Return a string describing the current state.
         */
        String8 toString() const;

        /**
         * Make a ClientDescriptor object wrapping the given BasicClient strong pointer.
         */
        static DescriptorPtr makeClientDescriptor(const String8& key, const sp<BasicClient>& value,
                int32_t cost, const std::set<String8>& conflictingKeys, int32_t score,
                int32_t ownerId, int32_t state);

        /**
         * Make a ClientDescriptor object wrapping the given BasicClient strong pointer with
         * values intialized from a prior ClientDescriptor.
         */
        static DescriptorPtr makeClientDescriptor(const sp<BasicClient>& value,
                const CameraService::DescriptorPtr& partial);

    }; // class CameraClientManager

private:

    typedef hardware::camera::common::V1_0::CameraDeviceStatus CameraDeviceStatus;

    /**
     * Typesafe version of device status, containing both the HAL-layer and the service interface-
     * layer values.
     */
    enum class StatusInternal : int32_t {
        NOT_PRESENT = static_cast<int32_t>(CameraDeviceStatus::NOT_PRESENT),
        PRESENT = static_cast<int32_t>(CameraDeviceStatus::PRESENT),
        ENUMERATING = static_cast<int32_t>(CameraDeviceStatus::ENUMERATING),
        NOT_AVAILABLE = static_cast<int32_t>(hardware::ICameraServiceListener::STATUS_NOT_AVAILABLE),
        UNKNOWN = static_cast<int32_t>(hardware::ICameraServiceListener::STATUS_UNKNOWN)
    };

    /**
     * Container class for the state of each logical camera device, including: ID, status, and
     * dependencies on other devices.  The mapping of camera ID -> state saved in mCameraStates
     * represents the camera devices advertised by the HAL (and any USB devices, when we add
     * those).
     *
     * This container does NOT represent an active camera client.  These are represented using
     * the ClientDescriptors stored in mActiveClientManager.
     */
    class CameraState {
    public:

        /**
         * Make a new CameraState and set the ID, cost, and conflicting devices using the values
         * returned in the HAL's camera_info struct for each device.
         */
        CameraState(const String8& id, int cost, const std::set<String8>& conflicting,
                bool isHidden);
        virtual ~CameraState();

        /**
         * Return the status for this device.
         *
         * This method acquires mStatusLock.
         */
        StatusInternal getStatus() const;

        /**
         * This function updates the status for this camera device, unless the given status
         * is in the given list of rejected status states, and execute the function passed in
         * with a signature onStatusUpdateLocked(const String8&, int32_t)
         * if the status has changed.
         *
         * This method is idempotent, and will not result in the function passed to
         * onStatusUpdateLocked being called more than once for the same arguments.
         * This method aquires mStatusLock.
         */
        template<class Func>
        void updateStatus(StatusInternal status,
                const String8& cameraId,
                std::initializer_list<StatusInternal> rejectSourceStates,
                Func onStatusUpdatedLocked);

        /**
         * Return the last set CameraParameters object generated from the information returned by
         * the HAL for this device (or an empty CameraParameters object if none has been set).
         */
        CameraParameters getShimParams() const;

        /**
         * Set the CameraParameters for this device.
         */
        void setShimParams(const CameraParameters& params);

        /**
         * Return the resource_cost advertised by the HAL for this device.
         */
        int getCost() const;

        /**
         * Return a set of the IDs of conflicting devices advertised by the HAL for this device.
         */
        std::set<String8> getConflicting() const;

        /**
         * Return the ID of this camera device.
         */
        String8 getId() const;

        /**
         * Return if the camera device is a publically hidden secure camera
         */
        bool isPublicallyHiddenSecureCamera() const;

    private:
        const String8 mId;
        StatusInternal mStatus; // protected by mStatusLock
        const int mCost;
        std::set<String8> mConflicting;
        mutable Mutex mStatusLock;
        CameraParameters mShimParams;
        const bool mIsPublicallyHiddenSecureCamera;
    }; // class CameraState

    // Observer for UID lifecycle enforcing that UIDs in idle
    // state cannot use the camera to protect user privacy.
    class UidPolicy : public BnUidObserver, public virtual IBinder::DeathRecipient {
    public:
        explicit UidPolicy(sp<CameraService> service)
                : mRegistered(false), mService(service) {}

        void registerSelf();
        void unregisterSelf();

        bool isUidActive(uid_t uid, String16 callingPackage);
        int32_t getProcState(uid_t uid);

        void onUidGone(uid_t uid, bool disabled);
        void onUidActive(uid_t uid);
        void onUidIdle(uid_t uid, bool disabled);
        void onUidStateChanged(uid_t uid, int32_t procState, int64_t procStateSeq);

        void addOverrideUid(uid_t uid, String16 callingPackage, bool active);
        void removeOverrideUid(uid_t uid, String16 callingPackage);

        void registerMonitorUid(uid_t uid);
        void unregisterMonitorUid(uid_t uid);

        // IBinder::DeathRecipient implementation
        virtual void binderDied(const wp<IBinder> &who);
    private:
        bool isUidActiveLocked(uid_t uid, String16 callingPackage);
        int32_t getProcStateLocked(uid_t uid);
        void updateOverrideUid(uid_t uid, String16 callingPackage, bool active, bool insert);

        Mutex mUidLock;
        bool mRegistered;
        wp<CameraService> mService;
        std::unordered_set<uid_t> mActiveUids;
        // Monitored uid map to cached procState and refCount pair
        std::unordered_map<uid_t, std::pair<int32_t, size_t>> mMonitoredUids;
        std::unordered_map<uid_t, bool> mOverrideUids;
    }; // class UidPolicy

    // If sensor privacy is enabled then all apps, including those that are active, should be
    // prevented from accessing the camera.
    class SensorPrivacyPolicy : public hardware::BnSensorPrivacyListener,
            public virtual IBinder::DeathRecipient {
        public:
            explicit SensorPrivacyPolicy(wp<CameraService> service)
                    : mService(service), mSensorPrivacyEnabled(false), mRegistered(false) {}

            void registerSelf();
            void unregisterSelf();

            bool isSensorPrivacyEnabled();

            binder::Status onSensorPrivacyChanged(bool enabled);

            // IBinder::DeathRecipient implementation
            virtual void binderDied(const wp<IBinder> &who);

        private:
            wp<CameraService> mService;
            Mutex mSensorPrivacyLock;
            bool mSensorPrivacyEnabled;
            bool mRegistered;
    };

    sp<UidPolicy> mUidPolicy;

    sp<SensorPrivacyPolicy> mSensorPrivacyPolicy;

    // Delay-load the Camera HAL module
    virtual void onFirstRef();

    // Eumerate all camera providers in the system
    status_t enumerateProviders();

    // Add/remove a new camera to camera and torch state lists or remove an unplugged one
    // Caller must not hold mServiceLock
    void addStates(const String8 id);
    void removeStates(const String8 id);

    // Check if we can connect, before we acquire the service lock.
    // The returned originalClientPid is the PID of the original process that wants to connect to
    // camera.
    // The returned clientPid is the PID of the client that directly connects to camera.
    // originalClientPid and clientPid are usually the same except when the application uses
    // mediaserver to connect to camera (using MediaRecorder to connect to camera). In that case,
    // clientPid is the PID of mediaserver and originalClientPid is the PID of the application.
    binder::Status validateConnectLocked(const String8& cameraId, const String8& clientName8,
            /*inout*/int& clientUid, /*inout*/int& clientPid, /*out*/int& originalClientPid) const;
    binder::Status validateClientPermissionsLocked(const String8& cameraId, const String8& clientName8,
            /*inout*/int& clientUid, /*inout*/int& clientPid, /*out*/int& originalClientPid) const;

    // Handle active client evictions, and update service state.
    // Only call with with mServiceLock held.
    status_t handleEvictionsLocked(const String8& cameraId, int clientPid,
        apiLevel effectiveApiLevel, const sp<IBinder>& remoteCallback, const String8& packageName,
        /*out*/
        sp<BasicClient>* client,
        std::shared_ptr<resource_policy::ClientDescriptor<String8, sp<BasicClient>>>* partial);

    // Should an operation attempt on a cameraId be rejected, if the camera id is
    // advertised as a publically hidden secure camera, by the camera HAL ?
    bool shouldRejectHiddenCameraConnection(const String8& cameraId);

    bool isPublicallyHiddenSecureCamera(const String8& cameraId);

    // Single implementation shared between the various connect calls
    template<class CALLBACK, class CLIENT>
    binder::Status connectHelper(const sp<CALLBACK>& cameraCb, const String8& cameraId,
            int api1CameraId, int halVersion, const String16& clientPackageName,
            int clientUid, int clientPid, apiLevel effectiveApiLevel, bool shimUpdateOnly,
            /*out*/sp<CLIENT>& device);

    // Lock guarding camera service state
    Mutex               mServiceLock;

    // Condition to use with mServiceLock, used to handle simultaneous connect calls from clients
    std::shared_ptr<WaitableMutexWrapper> mServiceLockWrapper;

    // Return NO_ERROR if the device with a give ID can be connected to
    status_t checkIfDeviceIsUsable(const String8& cameraId) const;

    // Container for managing currently active application-layer clients
    CameraClientManager mActiveClientManager;

    // Mapping from camera ID -> state for each device, map is protected by mCameraStatesLock
    std::map<String8, std::shared_ptr<CameraState>> mCameraStates;

    // Mutex guarding mCameraStates map
    mutable Mutex mCameraStatesLock;

    // Circular buffer for storing event logging for dumps
    RingBuffer<String8> mEventLog;
    Mutex mLogLock;

    // The last monitored tags set by client
    String8 mMonitorTags;

    // Currently allowed user IDs
    std::set<userid_t> mAllowedUsers;

    /**
     * Get the camera state for a given camera id.
     *
     * This acquires mCameraStatesLock.
     */
    std::shared_ptr<CameraService::CameraState> getCameraState(const String8& cameraId) const;

    /**
     * Evict client who's remote binder has died.  Returns true if this client was in the active
     * list and was disconnected.
     *
     * This method acquires mServiceLock.
     */
    bool evictClientIdByRemote(const wp<IBinder>& cameraClient);

    /**
     * Remove the given client from the active clients list; does not disconnect the client.
     *
     * This method acquires mServiceLock.
     */
    void removeByClient(const BasicClient* client);

    /**
     * Add new client to active clients list after conflicting clients have disconnected using the
     * values set in the partial descriptor passed in to construct the actual client descriptor.
     * This is typically called at the end of a connect call.
     *
     * This method must be called with mServiceLock held.
     */
    void finishConnectLocked(const sp<BasicClient>& client, const DescriptorPtr& desc);

    /**
     * Returns the underlying camera Id string mapped to a camera id int
     * Empty string is returned when the cameraIdInt is invalid.
     */
    String8 cameraIdIntToStr(int cameraIdInt);

    /**
     * Returns the underlying camera Id string mapped to a camera id int
     * Empty string is returned when the cameraIdInt is invalid.
     */
    std::string cameraIdIntToStrLocked(int cameraIdInt);

    /**
     * Remove a single client corresponding to the given camera id from the list of active clients.
     * If none exists, return an empty strongpointer.
     *
     * This method must be called with mServiceLock held.
     */
    sp<CameraService::BasicClient> removeClientLocked(const String8& cameraId);

    /**
     * Handle a notification that the current device user has changed.
     */
    void doUserSwitch(const std::vector<int32_t>& newUserIds);

    /**
     * Add an event log message.
     */
    void logEvent(const char* event);

    /**
     * Add an event log message that a client has been disconnected.
     */
    void logDisconnected(const char* cameraId, int clientPid, const char* clientPackage);

    /**
     * Add an event log message that a client has been connected.
     */
    void logConnected(const char* cameraId, int clientPid, const char* clientPackage);

    /**
     * Add an event log message that a client's connect attempt has been rejected.
     */
    void logRejected(const char* cameraId, int clientPid, const char* clientPackage,
            const char* reason);

    /**
     * Add an event log message when a client calls setTorchMode succesfully.
     */
    void logTorchEvent(const char* cameraId, const char *torchState, int clientPid);

    /**
     * Add an event log message that the current device user has been switched.
     */
    void logUserSwitch(const std::set<userid_t>& oldUserIds,
        const std::set<userid_t>& newUserIds);

    /**
     * Add an event log message that a device has been removed by the HAL
     */
    void logDeviceRemoved(const char* cameraId, const char* reason);

    /**
     * Add an event log message that a device has been added by the HAL
     */
    void logDeviceAdded(const char* cameraId, const char* reason);

    /**
     * Add an event log message that a client has unexpectedly died.
     */
    void logClientDied(int clientPid, const char* reason);

    /**
     * Add a event log message that a serious service-level error has occured
     * The errorCode should be one of the Android Errors
     */
    void logServiceError(const char* msg, int errorCode);

    /**
     * Dump the event log to an FD
     */
    void dumpEventLog(int fd);

    /**
     * This method will acquire mServiceLock
     */
    void updateCameraNumAndIds();

    int                 mNumberOfCameras;

    std::vector<std::string> mNormalDeviceIds;

    // sounds
    sp<MediaPlayer>     newMediaPlayer(const char *file);

    Mutex               mSoundLock;
    sp<MediaPlayer>     mSoundPlayer[NUM_SOUNDS];
    int                 mSoundRef;  // reference count (release all MediaPlayer when 0)

    // Basic flag on whether the camera subsystem is in a usable state
    bool                mInitialized;

    sp<CameraProviderManager> mCameraProviderManager;

    class ServiceListener : public virtual IBinder::DeathRecipient {
        public:
            ServiceListener(sp<CameraService> parent, sp<hardware::ICameraServiceListener> listener,
                    int uid, int pid, bool openCloseCallbackAllowed) : mParent(parent),
                    mListener(listener), mListenerUid(uid), mListenerPid(pid),
                    mOpenCloseCallbackAllowed(openCloseCallbackAllowed) {}

            status_t initialize() {
                return IInterface::asBinder(mListener)->linkToDeath(this);
            }

            virtual void binderDied(const wp<IBinder> &/*who*/) {
                auto parent = mParent.promote();
                if (parent.get() != nullptr) {
                    parent->removeListener(mListener);
                }
            }

            int getListenerUid() { return mListenerUid; }
            int getListenerPid() { return mListenerPid; }
            sp<hardware::ICameraServiceListener> getListener() { return mListener; }
            bool isOpenCloseCallbackAllowed() { return mOpenCloseCallbackAllowed; }

        private:
            wp<CameraService> mParent;
            sp<hardware::ICameraServiceListener> mListener;
            int mListenerUid;
            int mListenerPid;
            bool mOpenCloseCallbackAllowed = false;
    };

    // Guarded by mStatusListenerMutex
    std::vector<std::pair<bool, sp<ServiceListener>>> mListenerList;

    Mutex       mStatusListenerLock;

    /**
     * Update the status for the given camera id (if that device exists), and broadcast the
     * status update to all current ICameraServiceListeners if the status has changed.  Any
     * statuses in rejectedSourceStates will be ignored.
     *
     * This method must be idempotent.
     * This method acquires mStatusLock and mStatusListenerLock.
     */
    void updateStatus(StatusInternal status,
            const String8& cameraId,
            std::initializer_list<StatusInternal>
                rejectedSourceStates);
    void updateStatus(StatusInternal status,
            const String8& cameraId);

    /**
     * Update the opened/closed status of the given camera id.
     *
     * This method acqiures mStatusListenerLock.
     */
    void updateOpenCloseStatus(const String8& cameraId, bool open, const String16& packageName);

    // flashlight control
    sp<CameraFlashlight> mFlashlight;
    // guard mTorchStatusMap
    Mutex                mTorchStatusMutex;
    // guard mTorchClientMap
    Mutex                mTorchClientMapMutex;
    // guard mTorchUidMap
    Mutex                mTorchUidMapMutex;
    // camera id -> torch status
    KeyedVector<String8, hardware::camera::common::V1_0::TorchModeStatus>
            mTorchStatusMap;
    // camera id -> torch client binder
    // only store the last client that turns on each camera's torch mode
    KeyedVector<String8, sp<IBinder>> mTorchClientMap;
    // camera id -> [incoming uid, current uid] pair
    std::map<String8, std::pair<int, int>> mTorchUidMap;

    // check and handle if torch client's process has died
    void handleTorchClientBinderDied(const wp<IBinder> &who);

    // handle torch mode status change and invoke callbacks. mTorchStatusMutex
    // should be locked.
    void onTorchStatusChangedLocked(const String8& cameraId,
            hardware::camera::common::V1_0::TorchModeStatus newStatus);

    // get a camera's torch status. mTorchStatusMutex should be locked.
    status_t getTorchStatusLocked(const String8 &cameraId,
             hardware::camera::common::V1_0::TorchModeStatus *status) const;

    // set a camera's torch status. mTorchStatusMutex should be locked.
    status_t setTorchStatusLocked(const String8 &cameraId,
            hardware::camera::common::V1_0::TorchModeStatus status);

    // IBinder::DeathRecipient implementation
    virtual void        binderDied(const wp<IBinder> &who);

    /**
     * Initialize and cache the metadata used by the HAL1 shim for a given cameraId.
     *
     * Sets Status to a service-specific error on failure
     */
    binder::Status      initializeShimMetadata(int cameraId);

    /**
     * Get the cached CameraParameters for the camera. If they haven't been
     * cached yet, then initialize them for the first time.
     *
     * Sets Status to a service-specific error on failure
     */
    binder::Status      getLegacyParametersLazy(int cameraId, /*out*/CameraParameters* parameters);

    // Blocks all clients from the UID
    void blockClientsForUid(uid_t uid);

    // Blocks all active clients.
    void blockAllClients();

    // Overrides the UID state as if it is idle
    status_t handleSetUidState(const Vector<String16>& args, int err);

    // Clears the override for the UID state
    status_t handleResetUidState(const Vector<String16>& args, int err);

    // Gets the UID state
    status_t handleGetUidState(const Vector<String16>& args, int out, int err);

    // Prints the shell command help
    status_t printHelp(int out);

    /**
     * Get the current system time as a formatted string.
     */
    static String8 getFormattedCurrentTime();

    static binder::Status makeClient(const sp<CameraService>& cameraService,
            const sp<IInterface>& cameraCb, const String16& packageName, const String8& cameraId,
            int api1CameraId, int facing, int clientPid, uid_t clientUid, int servicePid,
            int halVersion, int deviceVersion, apiLevel effectiveApiLevel,
            /*out*/sp<BasicClient>* client);

    status_t checkCameraAccess(const String16& opPackageName);

    static String8 toString(std::set<userid_t> intSet);
    static int32_t mapToInterface(hardware::camera::common::V1_0::TorchModeStatus status);
    static StatusInternal mapToInternal(hardware::camera::common::V1_0::CameraDeviceStatus status);
    static int32_t mapToInterface(StatusInternal status);

    // Guard mCameraServiceProxy
    static Mutex sProxyMutex;
    // Cached interface to the camera service proxy in system service
    static sp<hardware::ICameraServiceProxy> sCameraServiceProxy;

    static sp<hardware::ICameraServiceProxy> getCameraServiceProxy();
    static void pingCameraServiceProxy();

    void broadcastTorchModeStatus(const String8& cameraId,
            hardware::camera::common::V1_0::TorchModeStatus status);
};

} // namespace android

#endif
