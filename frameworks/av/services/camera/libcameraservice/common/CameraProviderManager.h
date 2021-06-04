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

#ifndef ANDROID_SERVERS_CAMERA_CAMERAPROVIDER_H
#define ANDROID_SERVERS_CAMERA_CAMERAPROVIDER_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <mutex>

#include <camera/CameraParameters2.h>
#include <camera/CameraMetadata.h>
#include <camera/CameraBase.h>
#include <utils/Errors.h>
#include <android/hardware/camera/common/1.0/types.h>
#include <android/hardware/camera/provider/2.5/ICameraProvider.h>
#include <android/hardware/camera/device/3.4/ICameraDeviceSession.h>
#include <android/hidl/manager/1.0/IServiceNotification.h>
#include <camera/VendorTagDescriptor.h>

namespace android {

/**
 * The vendor tag descriptor class that takes HIDL vendor tag information as
 * input. Not part of VendorTagDescriptor class because that class is used
 * in AIDL generated sources which don't have access to HIDL headers.
 */
class HidlVendorTagDescriptor : public VendorTagDescriptor {
public:
    /**
     * Create a VendorTagDescriptor object from the HIDL VendorTagSection
     * vector.
     *
     * Returns OK on success, or a negative error code.
     */
    static status_t createDescriptorFromHidl(
            const hardware::hidl_vec<hardware::camera::common::V1_0::VendorTagSection>& vts,
            /*out*/
            sp<VendorTagDescriptor>& descriptor);
};

/**
 * A manager for all camera providers available on an Android device.
 *
 * Responsible for enumerating providers and the individual camera devices
 * they export, both at startup and as providers and devices are added/removed.
 *
 * Provides methods for requesting information about individual devices and for
 * opening them for active use.
 *
 */
class CameraProviderManager : virtual public hidl::manager::V1_0::IServiceNotification {
public:

    ~CameraProviderManager();

    // Tiny proxy for the static methods in a HIDL interface that communicate with the hardware
    // service manager, to be replacable in unit tests with a fake.
    struct ServiceInteractionProxy {
        virtual bool registerForNotifications(
                const std::string &serviceName,
                const sp<hidl::manager::V1_0::IServiceNotification>
                &notification) = 0;
        virtual sp<hardware::camera::provider::V2_4::ICameraProvider> getService(
                const std::string &serviceName) = 0;
        virtual hardware::hidl_vec<hardware::hidl_string> listServices() = 0;
        virtual ~ServiceInteractionProxy() {}
    };

    // Standard use case - call into the normal generated static methods which invoke
    // the real hardware service manager
    struct HardwareServiceInteractionProxy : public ServiceInteractionProxy {
        virtual bool registerForNotifications(
                const std::string &serviceName,
                const sp<hidl::manager::V1_0::IServiceNotification>
                &notification) override {
            return hardware::camera::provider::V2_4::ICameraProvider::registerForNotifications(
                    serviceName, notification);
        }
        virtual sp<hardware::camera::provider::V2_4::ICameraProvider> getService(
                const std::string &serviceName) override {
            return hardware::camera::provider::V2_4::ICameraProvider::getService(serviceName);
        }

        virtual hardware::hidl_vec<hardware::hidl_string> listServices() override;
    };

    /**
     * Listener interface for device/torch status changes
     */
    struct StatusListener : virtual public RefBase {
        ~StatusListener() {}

        virtual void onDeviceStatusChanged(const String8 &cameraId,
                hardware::camera::common::V1_0::CameraDeviceStatus newStatus) = 0;
        virtual void onTorchStatusChanged(const String8 &cameraId,
                hardware::camera::common::V1_0::TorchModeStatus newStatus) = 0;
        virtual void onNewProviderRegistered() = 0;
    };

    /**
     * Represents the mode a camera device is currently in
     */
    enum class DeviceMode {
        TORCH,
        CAMERA
    };

    /**
     * Initialize the manager and give it a status listener; optionally accepts a service
     * interaction proxy.
     *
     * The default proxy communicates via the hardware service manager; alternate proxies can be
     * used for testing. The lifetime of the proxy must exceed the lifetime of the manager.
     */
    status_t initialize(wp<StatusListener> listener,
            ServiceInteractionProxy *proxy = &sHardwareServiceInteractionProxy);

    /**
     * Retrieve the total number of available cameras. This value may change dynamically as cameras
     * are added or removed.
     */
    int getCameraCount() const;

    std::vector<std::string> getCameraDeviceIds() const;

    /**
     * Retrieve the number of API1 compatible cameras; these are internal and
     * backwards-compatible. This is the set of cameras that will be
     * accessible via the old camera API.
     * The return value may change dynamically due to external camera hotplug.
     */
    std::vector<std::string> getAPI1CompatibleCameraDeviceIds() const;

    /**
     * Return true if a device with a given ID and major version exists
     */
    bool isValidDevice(const std::string &id, uint16_t majorVersion) const;

    /**
     * Return true if a device with a given ID has a flash unit. Returns false
     * for devices that are unknown.
     */
    bool hasFlashUnit(const std::string &id) const;

    /**
     * Return the resource cost of this camera device
     */
    status_t getResourceCost(const std::string &id,
            hardware::camera::common::V1_0::CameraResourceCost* cost) const;

    /**
     * Return the old camera API camera info
     */
    status_t getCameraInfo(const std::string &id,
            hardware::CameraInfo* info) const;

    /**
     * Return API2 camera characteristics - returns NAME_NOT_FOUND if a device ID does
     * not have a v3 or newer HAL version.
     */
    status_t getCameraCharacteristics(const std::string &id,
            CameraMetadata* characteristics) const;

    /**
     * Check for device support of specific stream combination.
     */
    status_t isSessionConfigurationSupported(const std::string& id,
            const hardware::camera::device::V3_4::StreamConfiguration &configuration,
            bool *status /*out*/) const;

    /**
     * Return the highest supported device interface version for this ID
     */
    status_t getHighestSupportedVersion(const std::string &id,
            hardware::hidl_version *v);

    /**
     * Check if a given camera device support setTorchMode API.
     */
    bool supportSetTorchMode(const std::string &id) const;

    /**
     * Turn on or off the flashlight on a given camera device.
     * May fail if the device does not support this API, is in active use, or if the device
     * doesn't exist, etc.
     */
    status_t setTorchMode(const std::string &id, bool enabled);

    /**
     * Setup vendor tags for all registered providers
     */
    status_t setUpVendorTags();

    /**
     * Inform registered providers about a device state change, such as folding or unfolding
     */
    status_t notifyDeviceStateChange(
        android::hardware::hidl_bitfield<hardware::camera::provider::V2_5::DeviceState> newState);

    /**
     * Open an active session to a camera device.
     *
     * This fully powers on the camera device hardware, and returns a handle to a
     * session to be used for hardware configuration and operation.
     */
    status_t openSession(const std::string &id,
            const sp<hardware::camera::device::V3_2::ICameraDeviceCallback>& callback,
            /*out*/
            sp<hardware::camera::device::V3_2::ICameraDeviceSession> *session);

    status_t openSession(const std::string &id,
            const sp<hardware::camera::device::V1_0::ICameraDeviceCallback>& callback,
            /*out*/
            sp<hardware::camera::device::V1_0::ICameraDevice> *session);

    /**
     * Save the ICameraProvider while it is being used by a camera or torch client
     */
    void saveRef(DeviceMode usageType, const std::string &cameraId,
            sp<hardware::camera::provider::V2_4::ICameraProvider> provider);

    /**
     * Notify that the camera or torch is no longer being used by a camera client
     */
    void removeRef(DeviceMode usageType, const std::string &cameraId);

    /**
     * IServiceNotification::onRegistration
     * Invoked by the hardware service manager when a new camera provider is registered
     */
    virtual hardware::Return<void> onRegistration(const hardware::hidl_string& fqName,
            const hardware::hidl_string& name,
            bool preexisting) override;

    /**
     * Dump out information about available providers and devices
     */
    status_t dump(int fd, const Vector<String16>& args);

    /**
     * Conversion methods between HAL Status and status_t and strings
     */
    static status_t mapToStatusT(const hardware::camera::common::V1_0::Status& s);
    static const char* statusToString(const hardware::camera::common::V1_0::Status& s);

    /*
     * Return provider type for a specific device.
     */
    metadata_vendor_id_t getProviderTagIdLocked(const std::string& id,
            hardware::hidl_version minVersion = hardware::hidl_version{0,0},
            hardware::hidl_version maxVersion = hardware::hidl_version{1000,0}) const;

    /*
     * Check if a camera is a logical camera. And if yes, return
     * the physical camera ids.
     */
    bool isLogicalCamera(const std::string& id, std::vector<std::string>* physicalCameraIds);

    bool isPublicallyHiddenSecureCamera(const std::string& id) const;
    bool isHiddenPhysicalCamera(const std::string& cameraId) const;

    static const float kDepthARTolerance;
private:
    // All private members, unless otherwise noted, expect mInterfaceMutex to be locked before use
    mutable std::mutex mInterfaceMutex;

    // the status listener update callbacks will lock mStatusMutex
    mutable std::mutex mStatusListenerMutex;
    wp<StatusListener> mListener;
    ServiceInteractionProxy* mServiceProxy;

    // Current overall Android device physical status
    android::hardware::hidl_bitfield<hardware::camera::provider::V2_5::DeviceState> mDeviceState;

    // mProviderLifecycleLock is locked during onRegistration and removeProvider
    mutable std::mutex mProviderLifecycleLock;

    static HardwareServiceInteractionProxy sHardwareServiceInteractionProxy;

    // Mapping from CameraDevice IDs to CameraProviders. This map is used to keep the
    // ICameraProvider alive while it is in use by the camera with the given ID for camera
    // capabilities
    std::unordered_map<std::string, sp<hardware::camera::provider::V2_4::ICameraProvider>>
            mCameraProviderByCameraId;

    // Mapping from CameraDevice IDs to CameraProviders. This map is used to keep the
    // ICameraProvider alive while it is in use by the camera with the given ID for torch
    // capabilities
    std::unordered_map<std::string, sp<hardware::camera::provider::V2_4::ICameraProvider>>
            mTorchProviderByCameraId;

    // Lock for accessing mCameraProviderByCameraId and mTorchProviderByCameraId
    std::mutex mProviderInterfaceMapLock;

    struct ProviderInfo :
            virtual public hardware::camera::provider::V2_4::ICameraProviderCallback,
            virtual public hardware::hidl_death_recipient
    {
        const std::string mProviderName;
        const metadata_vendor_id_t mProviderTagid;
        int mMinorVersion;
        sp<VendorTagDescriptor> mVendorTagDescriptor;
        bool mSetTorchModeSupported;
        bool mIsRemote;

        // Current overall Android device physical status
        hardware::hidl_bitfield<hardware::camera::provider::V2_5::DeviceState> mDeviceState;

        // This pointer is used to keep a reference to the ICameraProvider that was last accessed.
        wp<hardware::camera::provider::V2_4::ICameraProvider> mActiveInterface;

        sp<hardware::camera::provider::V2_4::ICameraProvider> mSavedInterface;

        ProviderInfo(const std::string &providerName,
                CameraProviderManager *manager);
        ~ProviderInfo();

        status_t initialize(sp<hardware::camera::provider::V2_4::ICameraProvider>& interface,
                hardware::hidl_bitfield<hardware::camera::provider::V2_5::DeviceState>
                    currentDeviceState);

        const sp<hardware::camera::provider::V2_4::ICameraProvider> startProviderInterface();

        const std::string& getType() const;

        status_t addDevice(const std::string& name,
                hardware::camera::common::V1_0::CameraDeviceStatus initialStatus =
                hardware::camera::common::V1_0::CameraDeviceStatus::PRESENT,
                /*out*/ std::string *parsedId = nullptr);

        status_t dump(int fd, const Vector<String16>& args) const;

        // ICameraProviderCallbacks interface - these lock the parent mInterfaceMutex
        virtual hardware::Return<void> cameraDeviceStatusChange(
                const hardware::hidl_string& cameraDeviceName,
                hardware::camera::common::V1_0::CameraDeviceStatus newStatus) override;
        virtual hardware::Return<void> torchModeStatusChange(
                const hardware::hidl_string& cameraDeviceName,
                hardware::camera::common::V1_0::TorchModeStatus newStatus) override;

        // hidl_death_recipient interface - this locks the parent mInterfaceMutex
        virtual void serviceDied(uint64_t cookie, const wp<hidl::base::V1_0::IBase>& who) override;

        /**
         * Setup vendor tags for this provider
         */
        status_t setUpVendorTags();

        /**
         * Notify provider about top-level device physical state changes
         */
        status_t notifyDeviceStateChange(
                hardware::hidl_bitfield<hardware::camera::provider::V2_5::DeviceState>
                    newDeviceState);

        // Basic device information, common to all camera devices
        struct DeviceInfo {
            const std::string mName;  // Full instance name
            const std::string mId;    // ID section of full name
            const hardware::hidl_version mVersion;
            const metadata_vendor_id_t mProviderTagid;
            bool mIsLogicalCamera;
            std::vector<std::string> mPhysicalIds;
            hardware::CameraInfo mInfo;
            sp<IBase> mSavedInterface;
            bool mIsPublicallyHiddenSecureCamera = false;

            const hardware::camera::common::V1_0::CameraResourceCost mResourceCost;

            hardware::camera::common::V1_0::CameraDeviceStatus mStatus;

            sp<ProviderInfo> mParentProvider;

            bool hasFlashUnit() const { return mHasFlashUnit; }
            virtual status_t setTorchMode(bool enabled) = 0;
            virtual status_t getCameraInfo(hardware::CameraInfo *info) const = 0;
            virtual bool isAPI1Compatible() const = 0;
            virtual status_t dumpState(int fd) = 0;
            virtual status_t getCameraCharacteristics(CameraMetadata *characteristics) const {
                (void) characteristics;
                return INVALID_OPERATION;
            }
            virtual status_t getPhysicalCameraCharacteristics(const std::string& physicalCameraId,
                    CameraMetadata *characteristics) const {
                (void) physicalCameraId;
                (void) characteristics;
                return INVALID_OPERATION;
            }

            virtual status_t isSessionConfigurationSupported(
                    const hardware::camera::device::V3_4::StreamConfiguration &/*configuration*/,
                    bool * /*status*/) {
                return INVALID_OPERATION;
            }

            template<class InterfaceT>
            sp<InterfaceT> startDeviceInterface();

            DeviceInfo(const std::string& name, const metadata_vendor_id_t tagId,
                    const std::string &id, const hardware::hidl_version& version,
                    const std::vector<std::string>& publicCameraIds,
                    const hardware::camera::common::V1_0::CameraResourceCost& resourceCost,
                    sp<ProviderInfo> parentProvider) :
                    mName(name), mId(id), mVersion(version), mProviderTagid(tagId),
                    mIsLogicalCamera(false), mResourceCost(resourceCost),
                    mStatus(hardware::camera::common::V1_0::CameraDeviceStatus::PRESENT),
                    mParentProvider(parentProvider), mHasFlashUnit(false),
                    mPublicCameraIds(publicCameraIds) {}
            virtual ~DeviceInfo();
        protected:
            bool mHasFlashUnit;
            const std::vector<std::string>& mPublicCameraIds;

            template<class InterfaceT>
            static status_t setTorchMode(InterfaceT& interface, bool enabled);

            template<class InterfaceT>
            status_t setTorchModeForDevice(bool enabled) {
                // Don't save the ICameraProvider interface here because we assume that this was
                // called from CameraProviderManager::setTorchMode(), which does save it.
                const sp<InterfaceT> interface = startDeviceInterface<InterfaceT>();
                return DeviceInfo::setTorchMode(interface, enabled);
            }
        };
        std::vector<std::unique_ptr<DeviceInfo>> mDevices;
        std::unordered_set<std::string> mUniqueCameraIds;
        int mUniqueDeviceCount;
        std::vector<std::string> mUniqueAPI1CompatibleCameraIds;
        // The initial public camera IDs published by the camera provider.
        // Currently logical multi-camera is not supported for hot-plug camera.
        // And we use this list to keep track of initial public camera IDs
        // advertised by the provider, and to distinguish against "hidden"
        // physical camera IDs.
        std::vector<std::string> mProviderPublicCameraIds;

        // HALv1-specific camera fields, including the actual device interface
        struct DeviceInfo1 : public DeviceInfo {
            typedef hardware::camera::device::V1_0::ICameraDevice InterfaceT;

            virtual status_t setTorchMode(bool enabled) override;
            virtual status_t getCameraInfo(hardware::CameraInfo *info) const override;
            //In case of Device1Info assume that we are always API1 compatible
            virtual bool isAPI1Compatible() const override { return true; }
            virtual status_t dumpState(int fd) override;
            DeviceInfo1(const std::string& name, const metadata_vendor_id_t tagId,
                    const std::string &id, uint16_t minorVersion,
                    const hardware::camera::common::V1_0::CameraResourceCost& resourceCost,
                    sp<ProviderInfo> parentProvider,
                    const std::vector<std::string>& publicCameraIds,
                    sp<InterfaceT> interface);
            virtual ~DeviceInfo1();
        private:
            CameraParameters2 mDefaultParameters;
            status_t cacheCameraInfo(sp<InterfaceT> interface);
        };

        // HALv3-specific camera fields, including the actual device interface
        struct DeviceInfo3 : public DeviceInfo {
            typedef hardware::camera::device::V3_2::ICameraDevice InterfaceT;

            virtual status_t setTorchMode(bool enabled) override;
            virtual status_t getCameraInfo(hardware::CameraInfo *info) const override;
            virtual bool isAPI1Compatible() const override;
            virtual status_t dumpState(int fd) override;
            virtual status_t getCameraCharacteristics(
                    CameraMetadata *characteristics) const override;
            virtual status_t getPhysicalCameraCharacteristics(const std::string& physicalCameraId,
                    CameraMetadata *characteristics) const override;
            virtual status_t isSessionConfigurationSupported(
                    const hardware::camera::device::V3_4::StreamConfiguration &configuration,
                    bool *status /*out*/)
                    override;

            DeviceInfo3(const std::string& name, const metadata_vendor_id_t tagId,
                    const std::string &id, uint16_t minorVersion,
                    const hardware::camera::common::V1_0::CameraResourceCost& resourceCost,
                    sp<ProviderInfo> parentProvider,
                    const std::vector<std::string>& publicCameraIds, sp<InterfaceT> interface);
            virtual ~DeviceInfo3();
        private:
            CameraMetadata mCameraCharacteristics;
            std::unordered_map<std::string, CameraMetadata> mPhysicalCameraCharacteristics;
            void queryPhysicalCameraIds();
            bool isPublicallyHiddenSecureCamera();
            status_t fixupMonochromeTags();
            status_t addDynamicDepthTags();
            static void getSupportedSizes(const CameraMetadata& ch, uint32_t tag,
                    android_pixel_format_t format,
                    std::vector<std::tuple<size_t, size_t>> *sizes /*out*/);
            void getSupportedDurations( const CameraMetadata& ch, uint32_t tag,
                    android_pixel_format_t format,
                    const std::vector<std::tuple<size_t, size_t>>& sizes,
                    std::vector<int64_t> *durations/*out*/);
            void getSupportedDynamicDepthDurations(const std::vector<int64_t>& depthDurations,
                    const std::vector<int64_t>& blobDurations,
                    std::vector<int64_t> *dynamicDepthDurations /*out*/);
            static bool isDepthPhotoLibraryPresent();
            static void getSupportedDynamicDepthSizes(
                    const std::vector<std::tuple<size_t, size_t>>& blobSizes,
                    const std::vector<std::tuple<size_t, size_t>>& depthSizes,
                    std::vector<std::tuple<size_t, size_t>> *dynamicDepthSizes /*out*/,
                    std::vector<std::tuple<size_t, size_t>> *internalDepthSizes /*out*/);
            status_t removeAvailableKeys(CameraMetadata& c, const std::vector<uint32_t>& keys,
                    uint32_t keyTag);
            status_t fillHeicStreamCombinations(std::vector<int32_t>* outputs,
                    std::vector<int64_t>* durations,
                    std::vector<int64_t>* stallDurations,
                    const camera_metadata_entry& halStreamConfigs,
                    const camera_metadata_entry& halStreamDurations);
            status_t deriveHeicTags();
        };

    private:
        std::string mType;
        uint32_t mId;

        std::mutex mLock;

        CameraProviderManager *mManager;

        bool mInitialized = false;

        // Templated method to instantiate the right kind of DeviceInfo and call the
        // right CameraProvider getCameraDeviceInterface_* method.
        template<class DeviceInfoT>
        std::unique_ptr<DeviceInfo> initializeDeviceInfo(const std::string &name,
                const metadata_vendor_id_t tagId, const std::string &id,
                uint16_t minorVersion);

        // Helper for initializeDeviceInfo to use the right CameraProvider get method.
        template<class InterfaceT>
        sp<InterfaceT> startDeviceInterface(const std::string &name);

        // Parse provider instance name for type and id
        static status_t parseProviderName(const std::string& name,
                std::string *type, uint32_t *id);

        // Parse device instance name for device version, type, and id.
        static status_t parseDeviceName(const std::string& name,
                uint16_t *major, uint16_t *minor, std::string *type, std::string *id);

        // Generate vendor tag id
        static metadata_vendor_id_t generateVendorTagId(const std::string &name);

        void removeDevice(std::string id);
    };

    // Utility to find a DeviceInfo by ID; pointer is only valid while mInterfaceMutex is held
    // and the calling code doesn't mutate the list of providers or their lists of devices.
    // Finds the first device of the given ID that falls within the requested version range
    //   minVersion <= deviceVersion < maxVersion
    // No guarantees on the order of traversal
    ProviderInfo::DeviceInfo* findDeviceInfoLocked(const std::string& id,
            hardware::hidl_version minVersion = hardware::hidl_version{0,0},
            hardware::hidl_version maxVersion = hardware::hidl_version{1000,0}) const;

    status_t addProviderLocked(const std::string& newProvider);

    status_t removeProvider(const std::string& provider);
    sp<StatusListener> getStatusListener() const;

    bool isValidDeviceLocked(const std::string &id, uint16_t majorVersion) const;

    std::vector<sp<ProviderInfo>> mProviders;

    void addProviderToMap(
            const std::string &cameraId,
            sp<hardware::camera::provider::V2_4::ICameraProvider> provider,
            bool isTorchUsage);
    void removeCameraIdFromMap(
        std::unordered_map<std::string, sp<hardware::camera::provider::V2_4::ICameraProvider>> &map,
        const std::string &cameraId);

    static const char* deviceStatusToString(
        const hardware::camera::common::V1_0::CameraDeviceStatus&);
    static const char* torchStatusToString(
        const hardware::camera::common::V1_0::TorchModeStatus&);

    status_t getCameraCharacteristicsLocked(const std::string &id,
            CameraMetadata* characteristics) const;

    bool isPublicallyHiddenSecureCameraLocked(const std::string& id) const;

    void filterLogicalCameraIdsLocked(std::vector<std::string>& deviceIds) const;

    bool isPublicallyHiddenSecureCameraLocked(const std::string& id);

    std::pair<bool, CameraProviderManager::ProviderInfo::DeviceInfo *>
            isHiddenPhysicalCameraInternal(const std::string& cameraId) const;
};

} // namespace android

#endif
