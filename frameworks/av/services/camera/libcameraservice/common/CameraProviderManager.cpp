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

#define LOG_TAG "CameraProviderManager"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include "CameraProviderManager.h"

#include <android/hardware/camera/device/3.5/ICameraDevice.h>

#include <algorithm>
#include <chrono>
#include "common/DepthPhotoProcessor.h"
#include <dlfcn.h>
#include <future>
#include <inttypes.h>
#include <hardware/camera_common.h>
#include <android/hidl/manager/1.2/IServiceManager.h>
#include <hidl/ServiceManagement.h>
#include <functional>
#include <camera_metadata_hidden.h>
#include <android-base/parseint.h>
#include <android-base/logging.h>
#include <cutils/properties.h>
#include <hwbinder/IPCThreadState.h>
#include <utils/Trace.h>

#include "api2/HeicCompositeStream.h"

namespace android {

using namespace ::android::hardware::camera;
using namespace ::android::hardware::camera::common::V1_0;
using std::literals::chrono_literals::operator""s;

namespace {
const bool kEnableLazyHal(property_get_bool("ro.camera.enableLazyHal", false));
} // anonymous namespace

const float CameraProviderManager::kDepthARTolerance = .1f;

CameraProviderManager::HardwareServiceInteractionProxy
CameraProviderManager::sHardwareServiceInteractionProxy{};

CameraProviderManager::~CameraProviderManager() {
}

hardware::hidl_vec<hardware::hidl_string>
CameraProviderManager::HardwareServiceInteractionProxy::listServices() {
    hardware::hidl_vec<hardware::hidl_string> ret;
    auto manager = hardware::defaultServiceManager1_2();
    if (manager != nullptr) {
        manager->listManifestByInterface(provider::V2_4::ICameraProvider::descriptor,
                [&ret](const hardware::hidl_vec<hardware::hidl_string> &registered) {
                    ret = registered;
                });
    }
    return ret;
}

status_t CameraProviderManager::initialize(wp<CameraProviderManager::StatusListener> listener,
        ServiceInteractionProxy* proxy) {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);
    if (proxy == nullptr) {
        ALOGE("%s: No valid service interaction proxy provided", __FUNCTION__);
        return BAD_VALUE;
    }
    mListener = listener;
    mServiceProxy = proxy;
    mDeviceState = static_cast<hardware::hidl_bitfield<provider::V2_5::DeviceState>>(
        provider::V2_5::DeviceState::NORMAL);

    // Registering will trigger notifications for all already-known providers
    bool success = mServiceProxy->registerForNotifications(
        /* instance name, empty means no filter */ "",
        this);
    if (!success) {
        ALOGE("%s: Unable to register with hardware service manager for notifications "
                "about camera providers", __FUNCTION__);
        return INVALID_OPERATION;
    }


    for (const auto& instance : mServiceProxy->listServices()) {
        this->addProviderLocked(instance);
    }

    IPCThreadState::self()->flushCommands();

    return OK;
}

int CameraProviderManager::getCameraCount() const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);
    int count = 0;
    for (auto& provider : mProviders) {
        for (auto& id : provider->mUniqueCameraIds) {
            // Hidden secure camera ids are not to be exposed to camera1 api.
            if (isPublicallyHiddenSecureCameraLocked(id)) {
                continue;
            }
            count++;
        }
    }
    return count;
}

std::vector<std::string> CameraProviderManager::getCameraDeviceIds() const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);
    std::vector<std::string> deviceIds;
    for (auto& provider : mProviders) {
        for (auto& id : provider->mUniqueCameraIds) {
            deviceIds.push_back(id);
        }
    }
    return deviceIds;
}

std::vector<std::string> CameraProviderManager::getAPI1CompatibleCameraDeviceIds() const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);
    std::vector<std::string> deviceIds;
    for (auto& provider : mProviders) {
        std::vector<std::string> providerDeviceIds = provider->mUniqueAPI1CompatibleCameraIds;

        // API1 app doesn't handle logical and physical camera devices well. So
        // for each camera facing, only take the first id advertised by HAL in
        // all [logical, physical1, physical2, ...] id combos, and filter out the rest.
        filterLogicalCameraIdsLocked(providerDeviceIds);
        // Hidden secure camera ids are not to be exposed to camera1 api.
        providerDeviceIds.erase(std::remove_if(providerDeviceIds.begin(), providerDeviceIds.end(),
                [this](const std::string& s) {
                    return this->isPublicallyHiddenSecureCameraLocked(s);}),
                providerDeviceIds.end());
        deviceIds.insert(deviceIds.end(), providerDeviceIds.begin(), providerDeviceIds.end());
    }

    std::sort(deviceIds.begin(), deviceIds.end(),
            [](const std::string& a, const std::string& b) -> bool {
                uint32_t aUint = 0, bUint = 0;
                bool aIsUint = base::ParseUint(a, &aUint);
                bool bIsUint = base::ParseUint(b, &bUint);

                // Uint device IDs first
                if (aIsUint && bIsUint) {
                    return aUint < bUint;
                } else if (aIsUint) {
                    return true;
                } else if (bIsUint) {
                    return false;
                }
                // Simple string compare if both id are not uint
                return a < b;
            });
    return deviceIds;
}

bool CameraProviderManager::isValidDevice(const std::string &id, uint16_t majorVersion) const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);
    return isValidDeviceLocked(id, majorVersion);
}

bool CameraProviderManager::isValidDeviceLocked(const std::string &id, uint16_t majorVersion) const {
    for (auto& provider : mProviders) {
        for (auto& deviceInfo : provider->mDevices) {
            if (deviceInfo->mId == id && deviceInfo->mVersion.get_major() == majorVersion) {
                return true;
            }
        }
    }
    return false;
}

bool CameraProviderManager::hasFlashUnit(const std::string &id) const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);

    auto deviceInfo = findDeviceInfoLocked(id);
    if (deviceInfo == nullptr) return false;

    return deviceInfo->hasFlashUnit();
}

status_t CameraProviderManager::getResourceCost(const std::string &id,
        CameraResourceCost* cost) const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);

    auto deviceInfo = findDeviceInfoLocked(id);
    if (deviceInfo == nullptr) return NAME_NOT_FOUND;

    *cost = deviceInfo->mResourceCost;
    return OK;
}

status_t CameraProviderManager::getCameraInfo(const std::string &id,
        hardware::CameraInfo* info) const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);

    auto deviceInfo = findDeviceInfoLocked(id);
    if (deviceInfo == nullptr) return NAME_NOT_FOUND;

    return deviceInfo->getCameraInfo(info);
}

status_t CameraProviderManager::isSessionConfigurationSupported(const std::string& id,
        const hardware::camera::device::V3_4::StreamConfiguration &configuration,
        bool *status /*out*/) const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);

    auto deviceInfo = findDeviceInfoLocked(id);
    if (deviceInfo == nullptr) {
        return NAME_NOT_FOUND;
    }

    return deviceInfo->isSessionConfigurationSupported(configuration, status);
}

status_t CameraProviderManager::getCameraCharacteristics(const std::string &id,
        CameraMetadata* characteristics) const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);
    return getCameraCharacteristicsLocked(id, characteristics);
}

status_t CameraProviderManager::getHighestSupportedVersion(const std::string &id,
        hardware::hidl_version *v) {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);

    hardware::hidl_version maxVersion{0,0};
    bool found = false;
    for (auto& provider : mProviders) {
        for (auto& deviceInfo : provider->mDevices) {
            if (deviceInfo->mId == id) {
                if (deviceInfo->mVersion > maxVersion) {
                    maxVersion = deviceInfo->mVersion;
                    found = true;
                }
            }
        }
    }
    if (!found) {
        return NAME_NOT_FOUND;
    }
    *v = maxVersion;
    return OK;
}

bool CameraProviderManager::supportSetTorchMode(const std::string &id) const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);
    for (auto& provider : mProviders) {
        auto deviceInfo = findDeviceInfoLocked(id);
        if (deviceInfo != nullptr) {
            return provider->mSetTorchModeSupported;
        }
    }
    return false;
}

status_t CameraProviderManager::setTorchMode(const std::string &id, bool enabled) {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);

    auto deviceInfo = findDeviceInfoLocked(id);
    if (deviceInfo == nullptr) return NAME_NOT_FOUND;

    // Pass the camera ID to start interface so that it will save it to the map of ICameraProviders
    // that are currently in use.
    const sp<provider::V2_4::ICameraProvider> interface =
            deviceInfo->mParentProvider->startProviderInterface();
    if (interface == nullptr) {
        return DEAD_OBJECT;
    }
    saveRef(DeviceMode::TORCH, deviceInfo->mId, interface);

    return deviceInfo->setTorchMode(enabled);
}

status_t CameraProviderManager::setUpVendorTags() {
    sp<VendorTagDescriptorCache> tagCache = new VendorTagDescriptorCache();

    for (auto& provider : mProviders) {
        tagCache->addVendorDescriptor(provider->mProviderTagid, provider->mVendorTagDescriptor);
    }

    VendorTagDescriptorCache::setAsGlobalVendorTagCache(tagCache);

    return OK;
}

status_t CameraProviderManager::notifyDeviceStateChange(
        hardware::hidl_bitfield<provider::V2_5::DeviceState> newState) {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);
    mDeviceState = newState;
    status_t res = OK;
    for (auto& provider : mProviders) {
        ALOGV("%s: Notifying %s for new state 0x%" PRIx64,
                __FUNCTION__, provider->mProviderName.c_str(), newState);
        status_t singleRes = provider->notifyDeviceStateChange(mDeviceState);
        if (singleRes != OK) {
            ALOGE("%s: Unable to notify provider %s about device state change",
                    __FUNCTION__,
                    provider->mProviderName.c_str());
            res = singleRes;
            // continue to do the rest of the providers instead of returning now
        }
    }
    return res;
}

status_t CameraProviderManager::openSession(const std::string &id,
        const sp<device::V3_2::ICameraDeviceCallback>& callback,
        /*out*/
        sp<device::V3_2::ICameraDeviceSession> *session) {

    std::lock_guard<std::mutex> lock(mInterfaceMutex);

    auto deviceInfo = findDeviceInfoLocked(id,
            /*minVersion*/ {3,0}, /*maxVersion*/ {4,0});
    if (deviceInfo == nullptr) return NAME_NOT_FOUND;

    auto *deviceInfo3 = static_cast<ProviderInfo::DeviceInfo3*>(deviceInfo);
    const sp<provider::V2_4::ICameraProvider> provider =
            deviceInfo->mParentProvider->startProviderInterface();
    if (provider == nullptr) {
        return DEAD_OBJECT;
    }
    saveRef(DeviceMode::CAMERA, id, provider);

    Status status;
    hardware::Return<void> ret;
    auto interface = deviceInfo3->startDeviceInterface<
            CameraProviderManager::ProviderInfo::DeviceInfo3::InterfaceT>();
    if (interface == nullptr) {
        return DEAD_OBJECT;
    }

    ret = interface->open(callback, [&status, &session]
            (Status s, const sp<device::V3_2::ICameraDeviceSession>& cameraSession) {
                status = s;
                if (status == Status::OK) {
                    *session = cameraSession;
                }
            });
    if (!ret.isOk()) {
        removeRef(DeviceMode::CAMERA, id);
        ALOGE("%s: Transaction error opening a session for camera device %s: %s",
                __FUNCTION__, id.c_str(), ret.description().c_str());
        return DEAD_OBJECT;
    }
    return mapToStatusT(status);
}

status_t CameraProviderManager::openSession(const std::string &id,
        const sp<device::V1_0::ICameraDeviceCallback>& callback,
        /*out*/
        sp<device::V1_0::ICameraDevice> *session) {

    std::lock_guard<std::mutex> lock(mInterfaceMutex);

    auto deviceInfo = findDeviceInfoLocked(id,
            /*minVersion*/ {1,0}, /*maxVersion*/ {2,0});
    if (deviceInfo == nullptr) return NAME_NOT_FOUND;

    auto *deviceInfo1 = static_cast<ProviderInfo::DeviceInfo1*>(deviceInfo);
    const sp<provider::V2_4::ICameraProvider> provider =
            deviceInfo->mParentProvider->startProviderInterface();
    if (provider == nullptr) {
        return DEAD_OBJECT;
    }
    saveRef(DeviceMode::CAMERA, id, provider);

    auto interface = deviceInfo1->startDeviceInterface<
            CameraProviderManager::ProviderInfo::DeviceInfo1::InterfaceT>();
    if (interface == nullptr) {
        return DEAD_OBJECT;
    }
    hardware::Return<Status> status = interface->open(callback);
    if (!status.isOk()) {
        removeRef(DeviceMode::CAMERA, id);
        ALOGE("%s: Transaction error opening a session for camera device %s: %s",
                __FUNCTION__, id.c_str(), status.description().c_str());
        return DEAD_OBJECT;
    }
    if (status == Status::OK) {
        *session = interface;
    }
    return mapToStatusT(status);
}

void CameraProviderManager::saveRef(DeviceMode usageType, const std::string &cameraId,
        sp<provider::V2_4::ICameraProvider> provider) {
    if (!kEnableLazyHal) {
        return;
    }
    ALOGV("Saving camera provider %s for camera device %s", provider->descriptor, cameraId.c_str());
    std::lock_guard<std::mutex> lock(mProviderInterfaceMapLock);
    std::unordered_map<std::string, sp<provider::V2_4::ICameraProvider>> *primaryMap, *alternateMap;
    if (usageType == DeviceMode::TORCH) {
        primaryMap = &mTorchProviderByCameraId;
        alternateMap = &mCameraProviderByCameraId;
    } else {
        primaryMap = &mCameraProviderByCameraId;
        alternateMap = &mTorchProviderByCameraId;
    }
    auto id = cameraId.c_str();
    (*primaryMap)[id] = provider;
    auto search = alternateMap->find(id);
    if (search != alternateMap->end()) {
        ALOGW("%s: Camera device %s is using both torch mode and camera mode simultaneously. "
                "That should not be possible", __FUNCTION__, id);
    }
    ALOGV("%s: Camera device %s connected", __FUNCTION__, id);
}

void CameraProviderManager::removeRef(DeviceMode usageType, const std::string &cameraId) {
    if (!kEnableLazyHal) {
        return;
    }
    ALOGV("Removing camera device %s", cameraId.c_str());
    std::unordered_map<std::string, sp<provider::V2_4::ICameraProvider>> *providerMap;
    if (usageType == DeviceMode::TORCH) {
        providerMap = &mTorchProviderByCameraId;
    } else {
        providerMap = &mCameraProviderByCameraId;
    }
    std::lock_guard<std::mutex> lock(mProviderInterfaceMapLock);
    auto search = providerMap->find(cameraId.c_str());
    if (search != providerMap->end()) {
        // Drop the reference to this ICameraProvider. This is safe to do immediately (without an
        // added delay) because hwservicemanager guarantees to hold the reference for at least five
        // more seconds.  We depend on this behavior so that if the provider is unreferenced and
        // then referenced again quickly, we do not let the HAL exit and then need to immediately
        // restart it. An example when this could happen is switching from a front-facing to a
        // rear-facing camera. If the HAL were to exit during the camera switch, the camera could
        // appear janky to the user.
        providerMap->erase(cameraId.c_str());
        IPCThreadState::self()->flushCommands();
    } else {
        ALOGE("%s: Asked to remove reference for camera %s, but no reference to it was found. This "
                "could mean removeRef was called twice for the same camera ID.", __FUNCTION__,
                cameraId.c_str());
    }
}

hardware::Return<void> CameraProviderManager::onRegistration(
        const hardware::hidl_string& /*fqName*/,
        const hardware::hidl_string& name,
        bool /*preexisting*/) {
    std::lock_guard<std::mutex> providerLock(mProviderLifecycleLock);
    {
        std::lock_guard<std::mutex> lock(mInterfaceMutex);

        addProviderLocked(name);
    }

    sp<StatusListener> listener = getStatusListener();
    if (nullptr != listener.get()) {
        listener->onNewProviderRegistered();
    }

    IPCThreadState::self()->flushCommands();

    return hardware::Return<void>();
}

status_t CameraProviderManager::dump(int fd, const Vector<String16>& args) {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);

    for (auto& provider : mProviders) {
        provider->dump(fd, args);
    }
    return OK;
}

CameraProviderManager::ProviderInfo::DeviceInfo* CameraProviderManager::findDeviceInfoLocked(
        const std::string& id,
        hardware::hidl_version minVersion, hardware::hidl_version maxVersion) const {
    for (auto& provider : mProviders) {
        for (auto& deviceInfo : provider->mDevices) {
            if (deviceInfo->mId == id &&
                    minVersion <= deviceInfo->mVersion && maxVersion >= deviceInfo->mVersion) {
                return deviceInfo.get();
            }
        }
    }
    return nullptr;
}

metadata_vendor_id_t CameraProviderManager::getProviderTagIdLocked(
        const std::string& id, hardware::hidl_version minVersion,
        hardware::hidl_version maxVersion) const {
    metadata_vendor_id_t ret = CAMERA_METADATA_INVALID_VENDOR_ID;

    std::lock_guard<std::mutex> lock(mInterfaceMutex);
    for (auto& provider : mProviders) {
        for (auto& deviceInfo : provider->mDevices) {
            if (deviceInfo->mId == id &&
                    minVersion <= deviceInfo->mVersion &&
                    maxVersion >= deviceInfo->mVersion) {
                return provider->mProviderTagid;
            }
        }
    }

    return ret;
}

void CameraProviderManager::ProviderInfo::DeviceInfo3::queryPhysicalCameraIds() {
    camera_metadata_entry_t entryCap;

    entryCap = mCameraCharacteristics.find(ANDROID_REQUEST_AVAILABLE_CAPABILITIES);
    for (size_t i = 0; i < entryCap.count; ++i) {
        uint8_t capability = entryCap.data.u8[i];
        if (capability == ANDROID_REQUEST_AVAILABLE_CAPABILITIES_LOGICAL_MULTI_CAMERA) {
            mIsLogicalCamera = true;
            break;
        }
    }
    if (!mIsLogicalCamera) {
        return;
    }

    camera_metadata_entry_t entryIds = mCameraCharacteristics.find(
            ANDROID_LOGICAL_MULTI_CAMERA_PHYSICAL_IDS);
    const uint8_t* ids = entryIds.data.u8;
    size_t start = 0;
    for (size_t i = 0; i < entryIds.count; ++i) {
        if (ids[i] == '\0') {
            if (start != i) {
                mPhysicalIds.push_back((const char*)ids+start);
            }
            start = i+1;
        }
    }
}

bool CameraProviderManager::ProviderInfo::DeviceInfo3::isPublicallyHiddenSecureCamera() {
    camera_metadata_entry_t entryCap;
    entryCap = mCameraCharacteristics.find(ANDROID_REQUEST_AVAILABLE_CAPABILITIES);
    if (entryCap.count != 1) {
        // Do NOT hide this camera device if the capabilities specify anything more
        // than ANDROID_REQUEST_AVAILABLE_CAPABILITIES_SECURE_IMAGE_DATA.
        return false;
    }
    return entryCap.data.u8[0] == ANDROID_REQUEST_AVAILABLE_CAPABILITIES_SECURE_IMAGE_DATA;
}

void CameraProviderManager::ProviderInfo::DeviceInfo3::getSupportedSizes(
        const CameraMetadata& ch, uint32_t tag, android_pixel_format_t format,
        std::vector<std::tuple<size_t, size_t>> *sizes/*out*/) {
    if (sizes == nullptr) {
        return;
    }

    auto scalerDims = ch.find(tag);
    if (scalerDims.count > 0) {
        // Scaler entry contains 4 elements (format, width, height, type)
        for (size_t i = 0; i < scalerDims.count; i += 4) {
            if ((scalerDims.data.i32[i] == format) &&
                    (scalerDims.data.i32[i+3] ==
                     ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT)) {
                sizes->push_back(std::make_tuple(scalerDims.data.i32[i+1],
                            scalerDims.data.i32[i+2]));
            }
        }
    }
}

void CameraProviderManager::ProviderInfo::DeviceInfo3::getSupportedDurations(
        const CameraMetadata& ch, uint32_t tag, android_pixel_format_t format,
        const std::vector<std::tuple<size_t, size_t>>& sizes,
        std::vector<int64_t> *durations/*out*/) {
    if (durations == nullptr) {
        return;
    }

    auto availableDurations = ch.find(tag);
    if (availableDurations.count > 0) {
        // Duration entry contains 4 elements (format, width, height, duration)
        for (size_t i = 0; i < availableDurations.count; i += 4) {
            for (const auto& size : sizes) {
                int64_t width = std::get<0>(size);
                int64_t height = std::get<1>(size);
                if ((availableDurations.data.i64[i] == format) &&
                        (availableDurations.data.i64[i+1] == width) &&
                        (availableDurations.data.i64[i+2] == height)) {
                    durations->push_back(availableDurations.data.i64[i+3]);
                }
            }
        }
    }
}
void CameraProviderManager::ProviderInfo::DeviceInfo3::getSupportedDynamicDepthDurations(
        const std::vector<int64_t>& depthDurations, const std::vector<int64_t>& blobDurations,
        std::vector<int64_t> *dynamicDepthDurations /*out*/) {
    if ((dynamicDepthDurations == nullptr) || (depthDurations.size() != blobDurations.size())) {
        return;
    }

    // Unfortunately there is no direct way to calculate the dynamic depth stream duration.
    // Processing time on camera service side can vary greatly depending on multiple
    // variables which are not under our control. Make a guesstimate by taking the maximum
    // corresponding duration value from depth and blob.
    auto depthDuration = depthDurations.begin();
    auto blobDuration = blobDurations.begin();
    dynamicDepthDurations->reserve(depthDurations.size());
    while ((depthDuration != depthDurations.end()) && (blobDuration != blobDurations.end())) {
        dynamicDepthDurations->push_back(std::max(*depthDuration, *blobDuration));
        depthDuration++; blobDuration++;
    }
}

void CameraProviderManager::ProviderInfo::DeviceInfo3::getSupportedDynamicDepthSizes(
        const std::vector<std::tuple<size_t, size_t>>& blobSizes,
        const std::vector<std::tuple<size_t, size_t>>& depthSizes,
        std::vector<std::tuple<size_t, size_t>> *dynamicDepthSizes /*out*/,
        std::vector<std::tuple<size_t, size_t>> *internalDepthSizes /*out*/) {
    if (dynamicDepthSizes == nullptr || internalDepthSizes == nullptr) {
        return;
    }

    // The dynamic depth spec. does not mention how close the AR ratio should be.
    // Try using something appropriate.
    float ARTolerance = kDepthARTolerance;

    for (const auto& blobSize : blobSizes) {
        float jpegAR = static_cast<float> (std::get<0>(blobSize)) /
                static_cast<float>(std::get<1>(blobSize));
        bool found = false;
        for (const auto& depthSize : depthSizes) {
            if (depthSize == blobSize) {
                internalDepthSizes->push_back(depthSize);
                found = true;
                break;
            } else {
                float depthAR = static_cast<float> (std::get<0>(depthSize)) /
                    static_cast<float>(std::get<1>(depthSize));
                if (std::fabs(jpegAR - depthAR) <= ARTolerance) {
                    internalDepthSizes->push_back(depthSize);
                    found = true;
                    break;
                }
            }
        }

        if (found) {
            dynamicDepthSizes->push_back(blobSize);
        }
    }
}

bool CameraProviderManager::ProviderInfo::DeviceInfo3::isDepthPhotoLibraryPresent() {
    static bool libraryPresent = false;
    static bool initialized = false;
    if (initialized) {
        return libraryPresent;
    } else {
        initialized = true;
    }

    void* depthLibHandle = dlopen(camera3::kDepthPhotoLibrary, RTLD_NOW | RTLD_LOCAL);
    if (depthLibHandle == nullptr) {
        return false;
    }

    auto processFunc = dlsym(depthLibHandle, camera3::kDepthPhotoProcessFunction);
    if (processFunc != nullptr) {
        libraryPresent = true;
    } else {
        libraryPresent = false;
    }
    dlclose(depthLibHandle);

    return libraryPresent;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::addDynamicDepthTags() {
    uint32_t depthExclTag = ANDROID_DEPTH_DEPTH_IS_EXCLUSIVE;
    uint32_t depthSizesTag = ANDROID_DEPTH_AVAILABLE_DEPTH_STREAM_CONFIGURATIONS;
    auto& c = mCameraCharacteristics;
    std::vector<std::tuple<size_t, size_t>> supportedBlobSizes, supportedDepthSizes,
            supportedDynamicDepthSizes, internalDepthSizes;
    auto chTags = c.find(ANDROID_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
    if (chTags.count == 0) {
        ALOGE("%s: Supported camera characteristics is empty!", __FUNCTION__);
        return BAD_VALUE;
    }

    bool isDepthExclusivePresent = std::find(chTags.data.i32, chTags.data.i32 + chTags.count,
            depthExclTag) != (chTags.data.i32 + chTags.count);
    bool isDepthSizePresent = std::find(chTags.data.i32, chTags.data.i32 + chTags.count,
            depthSizesTag) != (chTags.data.i32 + chTags.count);
    if (!(isDepthExclusivePresent && isDepthSizePresent)) {
        // No depth support, nothing more to do.
        return OK;
    }

    auto depthExclusiveEntry = c.find(depthExclTag);
    if (depthExclusiveEntry.count > 0) {
        if (depthExclusiveEntry.data.u8[0] != ANDROID_DEPTH_DEPTH_IS_EXCLUSIVE_FALSE) {
            // Depth support is exclusive, nothing more to do.
            return OK;
        }
    } else {
        ALOGE("%s: Advertised depth exclusive tag but value is not present!", __FUNCTION__);
        return BAD_VALUE;
    }

    getSupportedSizes(c, ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, HAL_PIXEL_FORMAT_BLOB,
            &supportedBlobSizes);
    getSupportedSizes(c, depthSizesTag, HAL_PIXEL_FORMAT_Y16, &supportedDepthSizes);
    if (supportedBlobSizes.empty() || supportedDepthSizes.empty()) {
        // Nothing to do in this case.
        return OK;
    }

    getSupportedDynamicDepthSizes(supportedBlobSizes, supportedDepthSizes,
            &supportedDynamicDepthSizes, &internalDepthSizes);
    if (supportedDynamicDepthSizes.empty()) {
        // Nothing more to do.
        return OK;
    }

    if(!isDepthPhotoLibraryPresent()) {
        // Depth photo processing library is not present, nothing more to do.
        return OK;
    }

    std::vector<int32_t> dynamicDepthEntries;
    for (const auto& it : supportedDynamicDepthSizes) {
        int32_t entry[4] = {HAL_PIXEL_FORMAT_BLOB, static_cast<int32_t> (std::get<0>(it)),
                static_cast<int32_t> (std::get<1>(it)),
                ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT };
        dynamicDepthEntries.insert(dynamicDepthEntries.end(), entry, entry + 4);
    }

    std::vector<int64_t> depthMinDurations, depthStallDurations;
    std::vector<int64_t> blobMinDurations, blobStallDurations;
    std::vector<int64_t> dynamicDepthMinDurations, dynamicDepthStallDurations;

    getSupportedDurations(c, ANDROID_DEPTH_AVAILABLE_DEPTH_MIN_FRAME_DURATIONS,
            HAL_PIXEL_FORMAT_Y16, internalDepthSizes, &depthMinDurations);
    getSupportedDurations(c, ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS,
            HAL_PIXEL_FORMAT_BLOB, supportedDynamicDepthSizes, &blobMinDurations);
    if (blobMinDurations.empty() || depthMinDurations.empty() ||
            (depthMinDurations.size() != blobMinDurations.size())) {
        ALOGE("%s: Unexpected number of available depth min durations! %zu vs. %zu",
                __FUNCTION__, depthMinDurations.size(), blobMinDurations.size());
        return BAD_VALUE;
    }

    getSupportedDurations(c, ANDROID_DEPTH_AVAILABLE_DEPTH_STALL_DURATIONS,
            HAL_PIXEL_FORMAT_Y16, internalDepthSizes, &depthStallDurations);
    getSupportedDurations(c, ANDROID_SCALER_AVAILABLE_STALL_DURATIONS,
            HAL_PIXEL_FORMAT_BLOB, supportedDynamicDepthSizes, &blobStallDurations);
    if (blobStallDurations.empty() || depthStallDurations.empty() ||
            (depthStallDurations.size() != blobStallDurations.size())) {
        ALOGE("%s: Unexpected number of available depth stall durations! %zu vs. %zu",
                __FUNCTION__, depthStallDurations.size(), blobStallDurations.size());
        return BAD_VALUE;
    }

    getSupportedDynamicDepthDurations(depthMinDurations, blobMinDurations,
            &dynamicDepthMinDurations);
    getSupportedDynamicDepthDurations(depthStallDurations, blobStallDurations,
            &dynamicDepthStallDurations);
    if (dynamicDepthMinDurations.empty() || dynamicDepthStallDurations.empty() ||
            (dynamicDepthMinDurations.size() != dynamicDepthStallDurations.size())) {
        ALOGE("%s: Unexpected number of dynamic depth stall/min durations! %zu vs. %zu",
                __FUNCTION__, dynamicDepthMinDurations.size(), dynamicDepthStallDurations.size());
        return BAD_VALUE;
    }

    std::vector<int64_t> dynamicDepthMinDurationEntries;
    auto itDuration = dynamicDepthMinDurations.begin();
    auto itSize = supportedDynamicDepthSizes.begin();
    while (itDuration != dynamicDepthMinDurations.end()) {
        int64_t entry[4] = {HAL_PIXEL_FORMAT_BLOB, static_cast<int32_t> (std::get<0>(*itSize)),
                static_cast<int32_t> (std::get<1>(*itSize)), *itDuration};
        dynamicDepthMinDurationEntries.insert(dynamicDepthMinDurationEntries.end(), entry,
                entry + 4);
        itDuration++; itSize++;
    }

    std::vector<int64_t> dynamicDepthStallDurationEntries;
    itDuration = dynamicDepthStallDurations.begin();
    itSize = supportedDynamicDepthSizes.begin();
    while (itDuration != dynamicDepthStallDurations.end()) {
        int64_t entry[4] = {HAL_PIXEL_FORMAT_BLOB, static_cast<int32_t> (std::get<0>(*itSize)),
                static_cast<int32_t> (std::get<1>(*itSize)), *itDuration};
        dynamicDepthStallDurationEntries.insert(dynamicDepthStallDurationEntries.end(), entry,
                entry + 4);
        itDuration++; itSize++;
    }

    c.update(ANDROID_DEPTH_AVAILABLE_DYNAMIC_DEPTH_STREAM_CONFIGURATIONS,
            dynamicDepthEntries.data(), dynamicDepthEntries.size());
    c.update(ANDROID_DEPTH_AVAILABLE_DYNAMIC_DEPTH_MIN_FRAME_DURATIONS,
            dynamicDepthMinDurationEntries.data(), dynamicDepthMinDurationEntries.size());
    c.update(ANDROID_DEPTH_AVAILABLE_DYNAMIC_DEPTH_STALL_DURATIONS,
            dynamicDepthStallDurationEntries.data(), dynamicDepthStallDurationEntries.size());

    std::vector<int32_t> supportedChTags;
    supportedChTags.reserve(chTags.count + 3);
    supportedChTags.insert(supportedChTags.end(), chTags.data.i32,
            chTags.data.i32 + chTags.count);
    supportedChTags.push_back(ANDROID_DEPTH_AVAILABLE_DYNAMIC_DEPTH_STREAM_CONFIGURATIONS);
    supportedChTags.push_back(ANDROID_DEPTH_AVAILABLE_DYNAMIC_DEPTH_MIN_FRAME_DURATIONS);
    supportedChTags.push_back(ANDROID_DEPTH_AVAILABLE_DYNAMIC_DEPTH_STALL_DURATIONS);
    c.update(ANDROID_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS, supportedChTags.data(),
            supportedChTags.size());

    return OK;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::fixupMonochromeTags() {
    status_t res = OK;
    auto& c = mCameraCharacteristics;

    // Override static metadata for MONOCHROME camera with older device version
    if (mVersion.get_major() == 3 && mVersion.get_minor() < 5) {
        camera_metadata_entry cap = c.find(ANDROID_REQUEST_AVAILABLE_CAPABILITIES);
        for (size_t i = 0; i < cap.count; i++) {
            if (cap.data.u8[i] == ANDROID_REQUEST_AVAILABLE_CAPABILITIES_MONOCHROME) {
                // ANDROID_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT
                uint8_t cfa = ANDROID_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_MONO;
                res = c.update(ANDROID_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, &cfa, 1);
                if (res != OK) {
                    ALOGE("%s: Failed to update COLOR_FILTER_ARRANGEMENT: %s (%d)",
                          __FUNCTION__, strerror(-res), res);
                    return res;
                }

                // ANDROID_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS
                const std::vector<uint32_t> sKeys = {
                        ANDROID_SENSOR_REFERENCE_ILLUMINANT1,
                        ANDROID_SENSOR_REFERENCE_ILLUMINANT2,
                        ANDROID_SENSOR_CALIBRATION_TRANSFORM1,
                        ANDROID_SENSOR_CALIBRATION_TRANSFORM2,
                        ANDROID_SENSOR_COLOR_TRANSFORM1,
                        ANDROID_SENSOR_COLOR_TRANSFORM2,
                        ANDROID_SENSOR_FORWARD_MATRIX1,
                        ANDROID_SENSOR_FORWARD_MATRIX2,
                };
                res = removeAvailableKeys(c, sKeys,
                        ANDROID_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
                if (res != OK) {
                    ALOGE("%s: Failed to update REQUEST_AVAILABLE_CHARACTERISTICS_KEYS: %s (%d)",
                            __FUNCTION__, strerror(-res), res);
                    return res;
                }

                // ANDROID_REQUEST_AVAILABLE_REQUEST_KEYS
                const std::vector<uint32_t> reqKeys = {
                        ANDROID_COLOR_CORRECTION_MODE,
                        ANDROID_COLOR_CORRECTION_TRANSFORM,
                        ANDROID_COLOR_CORRECTION_GAINS,
                };
                res = removeAvailableKeys(c, reqKeys, ANDROID_REQUEST_AVAILABLE_REQUEST_KEYS);
                if (res != OK) {
                    ALOGE("%s: Failed to update REQUEST_AVAILABLE_REQUEST_KEYS: %s (%d)",
                            __FUNCTION__, strerror(-res), res);
                    return res;
                }

                // ANDROID_REQUEST_AVAILABLE_RESULT_KEYS
                const std::vector<uint32_t> resKeys = {
                        ANDROID_SENSOR_GREEN_SPLIT,
                        ANDROID_SENSOR_NEUTRAL_COLOR_POINT,
                        ANDROID_COLOR_CORRECTION_MODE,
                        ANDROID_COLOR_CORRECTION_TRANSFORM,
                        ANDROID_COLOR_CORRECTION_GAINS,
                };
                res = removeAvailableKeys(c, resKeys, ANDROID_REQUEST_AVAILABLE_RESULT_KEYS);
                if (res != OK) {
                    ALOGE("%s: Failed to update REQUEST_AVAILABLE_RESULT_KEYS: %s (%d)",
                            __FUNCTION__, strerror(-res), res);
                    return res;
                }

                // ANDROID_SENSOR_BLACK_LEVEL_PATTERN
                camera_metadata_entry blEntry = c.find(ANDROID_SENSOR_BLACK_LEVEL_PATTERN);
                for (size_t j = 1; j < blEntry.count; j++) {
                    blEntry.data.i32[j] = blEntry.data.i32[0];
                }
            }
        }
    }
    return res;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::removeAvailableKeys(
        CameraMetadata& c, const std::vector<uint32_t>& keys, uint32_t keyTag) {
    status_t res = OK;

    camera_metadata_entry keysEntry = c.find(keyTag);
    if (keysEntry.count == 0) {
        ALOGE("%s: Failed to find tag %u: %s (%d)", __FUNCTION__, keyTag, strerror(-res), res);
        return res;
    }
    std::vector<int32_t> vKeys;
    vKeys.reserve(keysEntry.count);
    for (size_t i = 0; i < keysEntry.count; i++) {
        if (std::find(keys.begin(), keys.end(), keysEntry.data.i32[i]) == keys.end()) {
            vKeys.push_back(keysEntry.data.i32[i]);
        }
    }
    res = c.update(keyTag, vKeys.data(), vKeys.size());
    return res;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::fillHeicStreamCombinations(
        std::vector<int32_t>* outputs,
        std::vector<int64_t>* durations,
        std::vector<int64_t>* stallDurations,
        const camera_metadata_entry& halStreamConfigs,
        const camera_metadata_entry& halStreamDurations) {
    if (outputs == nullptr || durations == nullptr || stallDurations == nullptr) {
        return BAD_VALUE;
    }

    static bool supportInMemoryTempFile =
            camera3::HeicCompositeStream::isInMemoryTempFileSupported();
    if (!supportInMemoryTempFile) {
        ALOGI("%s: No HEIC support due to absence of in memory temp file support",
                __FUNCTION__);
        return OK;
    }

    for (size_t i = 0; i < halStreamConfigs.count; i += 4) {
        int32_t format = halStreamConfigs.data.i32[i];
        // Only IMPLEMENTATION_DEFINED and YUV_888 can be used to generate HEIC
        // image.
        if (format != HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED &&
                format != HAL_PIXEL_FORMAT_YCBCR_420_888) {
            continue;
        }

        bool sizeAvail = false;
        for (size_t j = 0; j < outputs->size(); j+= 4) {
            if ((*outputs)[j+1] == halStreamConfigs.data.i32[i+1] &&
                    (*outputs)[j+2] == halStreamConfigs.data.i32[i+2]) {
                sizeAvail = true;
                break;
            }
        }
        if (sizeAvail) continue;

        int64_t stall = 0;
        bool useHeic, useGrid;
        if (camera3::HeicCompositeStream::isSizeSupportedByHeifEncoder(
                halStreamConfigs.data.i32[i+1], halStreamConfigs.data.i32[i+2],
                &useHeic, &useGrid, &stall)) {
            if (useGrid != (format == HAL_PIXEL_FORMAT_YCBCR_420_888)) {
                continue;
            }

            // HEIC configuration
            int32_t config[] = {HAL_PIXEL_FORMAT_BLOB, halStreamConfigs.data.i32[i+1],
                    halStreamConfigs.data.i32[i+2], 0 /*isInput*/};
            outputs->insert(outputs->end(), config, config + 4);

            // HEIC minFrameDuration
            for (size_t j = 0; j < halStreamDurations.count; j += 4) {
                if (halStreamDurations.data.i64[j] == format &&
                        halStreamDurations.data.i64[j+1] == halStreamConfigs.data.i32[i+1] &&
                        halStreamDurations.data.i64[j+2] == halStreamConfigs.data.i32[i+2]) {
                    int64_t duration[] = {HAL_PIXEL_FORMAT_BLOB, halStreamConfigs.data.i32[i+1],
                            halStreamConfigs.data.i32[i+2], halStreamDurations.data.i64[j+3]};
                    durations->insert(durations->end(), duration, duration+4);
                    break;
                }
            }

            // HEIC stallDuration
            int64_t stallDuration[] = {HAL_PIXEL_FORMAT_BLOB, halStreamConfigs.data.i32[i+1],
                    halStreamConfigs.data.i32[i+2], stall};
            stallDurations->insert(stallDurations->end(), stallDuration, stallDuration+4);
        }
    }
    return OK;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::deriveHeicTags() {
    auto& c = mCameraCharacteristics;

    camera_metadata_entry halHeicSupport = c.find(ANDROID_HEIC_INFO_SUPPORTED);
    if (halHeicSupport.count > 1) {
        ALOGE("%s: Invalid entry count %zu for ANDROID_HEIC_INFO_SUPPORTED",
                __FUNCTION__, halHeicSupport.count);
        return BAD_VALUE;
    } else if (halHeicSupport.count == 0 ||
            halHeicSupport.data.u8[0] == ANDROID_HEIC_INFO_SUPPORTED_FALSE) {
        // Camera HAL doesn't support mandatory stream combinations for HEIC.
        return OK;
    }

    camera_metadata_entry maxJpegAppsSegments =
            c.find(ANDROID_HEIC_INFO_MAX_JPEG_APP_SEGMENTS_COUNT);
    if (maxJpegAppsSegments.count != 1 || maxJpegAppsSegments.data.u8[0] == 0 ||
            maxJpegAppsSegments.data.u8[0] > 16) {
        ALOGE("%s: ANDROID_HEIC_INFO_MAX_JPEG_APP_SEGMENTS_COUNT must be within [1, 16]",
                __FUNCTION__);
        return BAD_VALUE;
    }

    // Populate HEIC output configurations and its related min frame duration
    // and stall duration.
    std::vector<int32_t> heicOutputs;
    std::vector<int64_t> heicDurations;
    std::vector<int64_t> heicStallDurations;

    camera_metadata_entry halStreamConfigs =
            c.find(ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);
    camera_metadata_entry minFrameDurations =
            c.find(ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS);

    status_t res = fillHeicStreamCombinations(&heicOutputs, &heicDurations, &heicStallDurations,
            halStreamConfigs, minFrameDurations);
    if (res != OK) {
        ALOGE("%s: Failed to fill HEIC stream combinations: %s (%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }

    c.update(ANDROID_HEIC_AVAILABLE_HEIC_STREAM_CONFIGURATIONS,
           heicOutputs.data(), heicOutputs.size());
    c.update(ANDROID_HEIC_AVAILABLE_HEIC_MIN_FRAME_DURATIONS,
            heicDurations.data(), heicDurations.size());
    c.update(ANDROID_HEIC_AVAILABLE_HEIC_STALL_DURATIONS,
            heicStallDurations.data(), heicStallDurations.size());

    return OK;
}

bool CameraProviderManager::isLogicalCamera(const std::string& id,
        std::vector<std::string>* physicalCameraIds) {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);

    auto deviceInfo = findDeviceInfoLocked(id);
    if (deviceInfo == nullptr) return false;

    if (deviceInfo->mIsLogicalCamera && physicalCameraIds != nullptr) {
        *physicalCameraIds = deviceInfo->mPhysicalIds;
    }
    return deviceInfo->mIsLogicalCamera;
}

bool CameraProviderManager::isPublicallyHiddenSecureCamera(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mInterfaceMutex);
    return isPublicallyHiddenSecureCameraLocked(id);
}

bool CameraProviderManager::isPublicallyHiddenSecureCameraLocked(const std::string& id) const {
    auto deviceInfo = findDeviceInfoLocked(id);
    if (deviceInfo != nullptr) {
        return deviceInfo->mIsPublicallyHiddenSecureCamera;
    }
    // If this is a hidden physical camera, we should return what kind of
    // camera the enclosing logical camera is.
    auto isHiddenAndParent = isHiddenPhysicalCameraInternal(id);
    if (isHiddenAndParent.first) {
        LOG_ALWAYS_FATAL_IF(id == isHiddenAndParent.second->mId,
                "%s: hidden physical camera id %s and enclosing logical camera id %s are the same",
                __FUNCTION__, id.c_str(), isHiddenAndParent.second->mId.c_str());
        return isPublicallyHiddenSecureCameraLocked(isHiddenAndParent.second->mId);
    }
    // Invalid camera id
    return true;
}

bool CameraProviderManager::isHiddenPhysicalCamera(const std::string& cameraId) const {
    return isHiddenPhysicalCameraInternal(cameraId).first;
}

std::pair<bool, CameraProviderManager::ProviderInfo::DeviceInfo *>
CameraProviderManager::isHiddenPhysicalCameraInternal(const std::string& cameraId) const {
    auto falseRet = std::make_pair(false, nullptr);
    for (auto& provider : mProviders) {
        for (auto& deviceInfo : provider->mDevices) {
            if (deviceInfo->mId == cameraId) {
                // cameraId is found in public camera IDs advertised by the
                // provider.
                return falseRet;
            }
        }
    }

    for (auto& provider : mProviders) {
        for (auto& deviceInfo : provider->mDevices) {
            CameraMetadata info;
            status_t res = deviceInfo->getCameraCharacteristics(&info);
            if (res != OK) {
                ALOGE("%s: Failed to getCameraCharacteristics for id %s", __FUNCTION__,
                        deviceInfo->mId.c_str());
                return falseRet;
            }

            std::vector<std::string> physicalIds;
            if (deviceInfo->mIsLogicalCamera) {
                if (std::find(deviceInfo->mPhysicalIds.begin(), deviceInfo->mPhysicalIds.end(),
                        cameraId) != deviceInfo->mPhysicalIds.end()) {
                    int deviceVersion = HARDWARE_DEVICE_API_VERSION(
                            deviceInfo->mVersion.get_major(), deviceInfo->mVersion.get_minor());
                    if (deviceVersion < CAMERA_DEVICE_API_VERSION_3_5) {
                        ALOGE("%s: Wrong deviceVersion %x for hiddenPhysicalCameraId %s",
                                __FUNCTION__, deviceVersion, cameraId.c_str());
                        return falseRet;
                    } else {
                        return std::make_pair(true, deviceInfo.get());
                    }
                }
            }
        }
    }

    return falseRet;
}

status_t CameraProviderManager::addProviderLocked(const std::string& newProvider) {
    for (const auto& providerInfo : mProviders) {
        if (providerInfo->mProviderName == newProvider) {
            ALOGW("%s: Camera provider HAL with name '%s' already registered", __FUNCTION__,
                    newProvider.c_str());
            return ALREADY_EXISTS;
        }
    }

    sp<provider::V2_4::ICameraProvider> interface;
    interface = mServiceProxy->getService(newProvider);

    if (interface == nullptr) {
        ALOGE("%s: Camera provider HAL '%s' is not actually available", __FUNCTION__,
                newProvider.c_str());
        return BAD_VALUE;
    }

    sp<ProviderInfo> providerInfo = new ProviderInfo(newProvider, this);
    status_t res = providerInfo->initialize(interface, mDeviceState);
    if (res != OK) {
        return res;
    }

    mProviders.push_back(providerInfo);

    return OK;
}

status_t CameraProviderManager::removeProvider(const std::string& provider) {
    std::lock_guard<std::mutex> providerLock(mProviderLifecycleLock);
    std::unique_lock<std::mutex> lock(mInterfaceMutex);
    std::vector<String8> removedDeviceIds;
    status_t res = NAME_NOT_FOUND;
    for (auto it = mProviders.begin(); it != mProviders.end(); it++) {
        if ((*it)->mProviderName == provider) {
            removedDeviceIds.reserve((*it)->mDevices.size());
            for (auto& deviceInfo : (*it)->mDevices) {
                removedDeviceIds.push_back(String8(deviceInfo->mId.c_str()));
            }
            mProviders.erase(it);
            res = OK;
            break;
        }
    }
    if (res != OK) {
        ALOGW("%s: Camera provider HAL with name '%s' is not registered", __FUNCTION__,
                provider.c_str());
    } else {
        // Inform camera service of loss of presence for all the devices from this provider,
        // without lock held for reentrancy
        sp<StatusListener> listener = getStatusListener();
        if (listener != nullptr) {
            lock.unlock();
            for (auto& id : removedDeviceIds) {
                listener->onDeviceStatusChanged(id, CameraDeviceStatus::NOT_PRESENT);
            }
        }
    }
    return res;
}

sp<CameraProviderManager::StatusListener> CameraProviderManager::getStatusListener() const {
    return mListener.promote();
}

/**** Methods for ProviderInfo ****/


CameraProviderManager::ProviderInfo::ProviderInfo(
        const std::string &providerName,
        CameraProviderManager *manager) :
        mProviderName(providerName),
        mProviderTagid(generateVendorTagId(providerName)),
        mUniqueDeviceCount(0),
        mManager(manager) {
    (void) mManager;
}

status_t CameraProviderManager::ProviderInfo::initialize(
        sp<provider::V2_4::ICameraProvider>& interface,
        hardware::hidl_bitfield<provider::V2_5::DeviceState> currentDeviceState) {
    status_t res = parseProviderName(mProviderName, &mType, &mId);
    if (res != OK) {
        ALOGE("%s: Invalid provider name, ignoring", __FUNCTION__);
        return BAD_VALUE;
    }
    ALOGI("Connecting to new camera provider: %s, isRemote? %d",
            mProviderName.c_str(), interface->isRemote());

    // Determine minor version
    auto castResult = provider::V2_5::ICameraProvider::castFrom(interface);
    if (castResult.isOk()) {
        mMinorVersion = 5;
    } else {
        mMinorVersion = 4;
    }

    // cameraDeviceStatusChange callbacks may be called (and causing new devices added)
    // before setCallback returns
    hardware::Return<Status> status = interface->setCallback(this);
    if (!status.isOk()) {
        ALOGE("%s: Transaction error setting up callbacks with camera provider '%s': %s",
                __FUNCTION__, mProviderName.c_str(), status.description().c_str());
        return DEAD_OBJECT;
    }
    if (status != Status::OK) {
        ALOGE("%s: Unable to register callbacks with camera provider '%s'",
                __FUNCTION__, mProviderName.c_str());
        return mapToStatusT(status);
    }

    hardware::Return<bool> linked = interface->linkToDeath(this, /*cookie*/ mId);
    if (!linked.isOk()) {
        ALOGE("%s: Transaction error in linking to camera provider '%s' death: %s",
                __FUNCTION__, mProviderName.c_str(), linked.description().c_str());
        return DEAD_OBJECT;
    } else if (!linked) {
        ALOGW("%s: Unable to link to provider '%s' death notifications",
                __FUNCTION__, mProviderName.c_str());
    }

    if (!kEnableLazyHal) {
        // Save HAL reference indefinitely
        mSavedInterface = interface;
    } else {
        mActiveInterface = interface;
    }

    ALOGV("%s: Setting device state for %s: 0x%" PRIx64,
            __FUNCTION__, mProviderName.c_str(), mDeviceState);
    notifyDeviceStateChange(currentDeviceState);

    res = setUpVendorTags();
    if (res != OK) {
        ALOGE("%s: Unable to set up vendor tags from provider '%s'",
                __FUNCTION__, mProviderName.c_str());
        return res;
    }

    // Get initial list of camera devices, if any
    std::vector<std::string> devices;
    hardware::Return<void> ret = interface->getCameraIdList([&status, this, &devices](
            Status idStatus,
            const hardware::hidl_vec<hardware::hidl_string>& cameraDeviceNames) {
        status = idStatus;
        if (status == Status::OK) {
            for (auto& name : cameraDeviceNames) {
                uint16_t major, minor;
                std::string type, id;
                status_t res = parseDeviceName(name, &major, &minor, &type, &id);
                if (res != OK) {
                    ALOGE("%s: Error parsing deviceName: %s: %d", __FUNCTION__, name.c_str(), res);
                    status = Status::INTERNAL_ERROR;
                } else {
                    devices.push_back(name);
                    mProviderPublicCameraIds.push_back(id);
                }
            }
        } });
    if (!ret.isOk()) {
        ALOGE("%s: Transaction error in getting camera ID list from provider '%s': %s",
                __FUNCTION__, mProviderName.c_str(), linked.description().c_str());
        return DEAD_OBJECT;
    }
    if (status != Status::OK) {
        ALOGE("%s: Unable to query for camera devices from provider '%s'",
                __FUNCTION__, mProviderName.c_str());
        return mapToStatusT(status);
    }

    ret = interface->isSetTorchModeSupported(
        [this](auto status, bool supported) {
            if (status == Status::OK) {
                mSetTorchModeSupported = supported;
            }
        });
    if (!ret.isOk()) {
        ALOGE("%s: Transaction error checking torch mode support '%s': %s",
                __FUNCTION__, mProviderName.c_str(), ret.description().c_str());
        return DEAD_OBJECT;
    }

    mIsRemote = interface->isRemote();

    sp<StatusListener> listener = mManager->getStatusListener();
    for (auto& device : devices) {
        std::string id;
        status_t res = addDevice(device, common::V1_0::CameraDeviceStatus::PRESENT, &id);
        if (res != OK) {
            ALOGE("%s: Unable to enumerate camera device '%s': %s (%d)",
                    __FUNCTION__, device.c_str(), strerror(-res), res);
            continue;
        }
    }

    ALOGI("Camera provider %s ready with %zu camera devices",
            mProviderName.c_str(), mDevices.size());

    mInitialized = true;
    return OK;
}

const sp<provider::V2_4::ICameraProvider>
CameraProviderManager::ProviderInfo::startProviderInterface() {
    ATRACE_CALL();
    ALOGV("Request to start camera provider: %s", mProviderName.c_str());
    if (mSavedInterface != nullptr) {
        return mSavedInterface;
    }
    if (!kEnableLazyHal) {
        ALOGE("Bad provider state! Should not be here on a non-lazy HAL!");
        return nullptr;
    }

    auto interface = mActiveInterface.promote();
    if (interface == nullptr) {
        ALOGI("Camera HAL provider needs restart, calling getService(%s)", mProviderName.c_str());
        interface = mManager->mServiceProxy->getService(mProviderName);
        interface->setCallback(this);
        hardware::Return<bool> linked = interface->linkToDeath(this, /*cookie*/ mId);
        if (!linked.isOk()) {
            ALOGE("%s: Transaction error in linking to camera provider '%s' death: %s",
                    __FUNCTION__, mProviderName.c_str(), linked.description().c_str());
            mManager->removeProvider(mProviderName);
            return nullptr;
        } else if (!linked) {
            ALOGW("%s: Unable to link to provider '%s' death notifications",
                    __FUNCTION__, mProviderName.c_str());
        }
        // Send current device state
        if (mMinorVersion >= 5) {
            auto castResult = provider::V2_5::ICameraProvider::castFrom(interface);
            if (castResult.isOk()) {
                sp<provider::V2_5::ICameraProvider> interface_2_5 = castResult;
                if (interface_2_5 != nullptr) {
                    ALOGV("%s: Initial device state for %s: 0x %" PRIx64,
                            __FUNCTION__, mProviderName.c_str(), mDeviceState);
                    interface_2_5->notifyDeviceStateChange(mDeviceState);
                }
            }
        }

        mActiveInterface = interface;
    } else {
        ALOGV("Camera provider (%s) already in use. Re-using instance.", mProviderName.c_str());
    }
    return interface;
}

const std::string& CameraProviderManager::ProviderInfo::getType() const {
    return mType;
}

status_t CameraProviderManager::ProviderInfo::addDevice(const std::string& name,
        CameraDeviceStatus initialStatus, /*out*/ std::string* parsedId) {

    ALOGI("Enumerating new camera device: %s", name.c_str());

    uint16_t major, minor;
    std::string type, id;

    status_t res = parseDeviceName(name, &major, &minor, &type, &id);
    if (res != OK) {
        return res;
    }
    if (type != mType) {
        ALOGE("%s: Device type %s does not match provider type %s", __FUNCTION__,
                type.c_str(), mType.c_str());
        return BAD_VALUE;
    }
    if (mManager->isValidDeviceLocked(id, major)) {
        ALOGE("%s: Device %s: ID %s is already in use for device major version %d", __FUNCTION__,
                name.c_str(), id.c_str(), major);
        return BAD_VALUE;
    }

    std::unique_ptr<DeviceInfo> deviceInfo;
    switch (major) {
        case 1:
            deviceInfo = initializeDeviceInfo<DeviceInfo1>(name, mProviderTagid,
                    id, minor);
            break;
        case 3:
            deviceInfo = initializeDeviceInfo<DeviceInfo3>(name, mProviderTagid,
                    id, minor);
            break;
        default:
            ALOGE("%s: Device %s: Unknown HIDL device HAL major version %d:", __FUNCTION__,
                    name.c_str(), major);
            return BAD_VALUE;
    }
    if (deviceInfo == nullptr) return BAD_VALUE;
    deviceInfo->mStatus = initialStatus;
    bool isAPI1Compatible = deviceInfo->isAPI1Compatible();

    mDevices.push_back(std::move(deviceInfo));

    mUniqueCameraIds.insert(id);
    if (isAPI1Compatible) {
        // addDevice can be called more than once for the same camera id if HAL
        // supports openLegacy.
        if (std::find(mUniqueAPI1CompatibleCameraIds.begin(), mUniqueAPI1CompatibleCameraIds.end(),
                id) == mUniqueAPI1CompatibleCameraIds.end()) {
            mUniqueAPI1CompatibleCameraIds.push_back(id);
        }
    }

    if (parsedId != nullptr) {
        *parsedId = id;
    }
    return OK;
}

void CameraProviderManager::ProviderInfo::removeDevice(std::string id) {
    for (auto it = mDevices.begin(); it != mDevices.end(); it++) {
        if ((*it)->mId == id) {
            mUniqueCameraIds.erase(id);
            if ((*it)->isAPI1Compatible()) {
                mUniqueAPI1CompatibleCameraIds.erase(std::remove(
                        mUniqueAPI1CompatibleCameraIds.begin(),
                        mUniqueAPI1CompatibleCameraIds.end(), id));
            }
            mDevices.erase(it);
            break;
        }
    }
}

status_t CameraProviderManager::ProviderInfo::dump(int fd, const Vector<String16>&) const {
    dprintf(fd, "== Camera Provider HAL %s (v2.%d, %s) static info: %zu devices: ==\n",
            mProviderName.c_str(),
            mMinorVersion,
            mIsRemote ? "remote" : "passthrough",
            mDevices.size());

    for (auto& device : mDevices) {
        dprintf(fd, "== Camera HAL device %s (v%d.%d) static information: ==\n", device->mName.c_str(),
                device->mVersion.get_major(), device->mVersion.get_minor());
        dprintf(fd, "  Resource cost: %d\n", device->mResourceCost.resourceCost);
        if (device->mResourceCost.conflictingDevices.size() == 0) {
            dprintf(fd, "  Conflicting devices: None\n");
        } else {
            dprintf(fd, "  Conflicting devices:\n");
            for (size_t i = 0; i < device->mResourceCost.conflictingDevices.size(); i++) {
                dprintf(fd, "    %s\n",
                        device->mResourceCost.conflictingDevices[i].c_str());
            }
        }
        dprintf(fd, "  API1 info:\n");
        dprintf(fd, "    Has a flash unit: %s\n",
                device->hasFlashUnit() ? "true" : "false");
        hardware::CameraInfo info;
        status_t res = device->getCameraInfo(&info);
        if (res != OK) {
            dprintf(fd, "   <Error reading camera info: %s (%d)>\n",
                    strerror(-res), res);
        } else {
            dprintf(fd, "    Facing: %s\n",
                    info.facing == hardware::CAMERA_FACING_BACK ? "Back" : "Front");
            dprintf(fd, "    Orientation: %d\n", info.orientation);
        }
        CameraMetadata info2;
        res = device->getCameraCharacteristics(&info2);
        if (res == INVALID_OPERATION) {
            dprintf(fd, "  API2 not directly supported\n");
        } else if (res != OK) {
            dprintf(fd, "  <Error reading camera characteristics: %s (%d)>\n",
                    strerror(-res), res);
        } else {
            dprintf(fd, "  API2 camera characteristics:\n");
            info2.dump(fd, /*verbosity*/ 2, /*indentation*/ 4);
        }

        // Dump characteristics of non-standalone physical camera
        if (device->mIsLogicalCamera) {
            for (auto& id : device->mPhysicalIds) {
                // Skip if physical id is an independent camera
                if (std::find(mProviderPublicCameraIds.begin(), mProviderPublicCameraIds.end(), id)
                        != mProviderPublicCameraIds.end()) {
                    continue;
                }

                CameraMetadata physicalInfo;
                status_t status = device->getPhysicalCameraCharacteristics(id, &physicalInfo);
                if (status == OK) {
                    dprintf(fd, "  Physical camera %s characteristics:\n", id.c_str());
                    physicalInfo.dump(fd, /*verbosity*/ 2, /*indentation*/ 4);
                }
            }
        }

        dprintf(fd, "== Camera HAL device %s (v%d.%d) dumpState: ==\n", device->mName.c_str(),
                device->mVersion.get_major(), device->mVersion.get_minor());
        res = device->dumpState(fd);
        if (res != OK) {
            dprintf(fd, "   <Error dumping device %s state: %s (%d)>\n",
                    device->mName.c_str(), strerror(-res), res);
        }
    }
    return OK;
}

hardware::Return<void> CameraProviderManager::ProviderInfo::cameraDeviceStatusChange(
        const hardware::hidl_string& cameraDeviceName,
        CameraDeviceStatus newStatus) {
    sp<StatusListener> listener;
    std::string id;
    bool initialized = false;
    {
        std::lock_guard<std::mutex> lock(mLock);
        bool known = false;
        for (auto& deviceInfo : mDevices) {
            if (deviceInfo->mName == cameraDeviceName) {
                ALOGI("Camera device %s status is now %s, was %s", cameraDeviceName.c_str(),
                        deviceStatusToString(newStatus), deviceStatusToString(deviceInfo->mStatus));
                deviceInfo->mStatus = newStatus;
                // TODO: Handle device removal (NOT_PRESENT)
                id = deviceInfo->mId;
                known = true;
                break;
            }
        }
        // Previously unseen device; status must not be NOT_PRESENT
        if (!known) {
            if (newStatus == CameraDeviceStatus::NOT_PRESENT) {
                ALOGW("Camera provider %s says an unknown camera device %s is not present. Curious.",
                    mProviderName.c_str(), cameraDeviceName.c_str());
                return hardware::Void();
            }
            addDevice(cameraDeviceName, newStatus, &id);
        } else if (newStatus == CameraDeviceStatus::NOT_PRESENT) {
            removeDevice(id);
        }
        listener = mManager->getStatusListener();
        initialized = mInitialized;
    }
    // Call without lock held to allow reentrancy into provider manager
    // Don't send the callback if providerInfo hasn't been initialized.
    // CameraService will initialize device status after provider is
    // initialized
    if (listener != nullptr && initialized) {
        listener->onDeviceStatusChanged(String8(id.c_str()), newStatus);
    }
    return hardware::Void();
}

hardware::Return<void> CameraProviderManager::ProviderInfo::torchModeStatusChange(
        const hardware::hidl_string& cameraDeviceName,
        TorchModeStatus newStatus) {
    sp<StatusListener> listener;
    std::string id;
    {
        std::lock_guard<std::mutex> lock(mManager->mStatusListenerMutex);
        bool known = false;
        for (auto& deviceInfo : mDevices) {
            if (deviceInfo->mName == cameraDeviceName) {
                ALOGI("Camera device %s torch status is now %s", cameraDeviceName.c_str(),
                        torchStatusToString(newStatus));
                id = deviceInfo->mId;
                known = true;
                if (TorchModeStatus::AVAILABLE_ON != newStatus) {
                    mManager->removeRef(DeviceMode::TORCH, id);
                }
                break;
            }
        }
        if (!known) {
            ALOGW("Camera provider %s says an unknown camera %s now has torch status %d. Curious.",
                    mProviderName.c_str(), cameraDeviceName.c_str(), newStatus);
            return hardware::Void();
        }
        listener = mManager->getStatusListener();
    }
    // Call without lock held to allow reentrancy into provider manager
    if (listener != nullptr) {
        listener->onTorchStatusChanged(String8(id.c_str()), newStatus);
    }
    return hardware::Void();
}

void CameraProviderManager::ProviderInfo::serviceDied(uint64_t cookie,
        const wp<hidl::base::V1_0::IBase>& who) {
    (void) who;
    ALOGI("Camera provider '%s' has died; removing it", mProviderName.c_str());
    if (cookie != mId) {
        ALOGW("%s: Unexpected serviceDied cookie %" PRIu64 ", expected %" PRIu32,
                __FUNCTION__, cookie, mId);
    }
    mManager->removeProvider(mProviderName);
}

status_t CameraProviderManager::ProviderInfo::setUpVendorTags() {
    if (mVendorTagDescriptor != nullptr)
        return OK;

    hardware::hidl_vec<VendorTagSection> vts;
    Status status;
    hardware::Return<void> ret;
    const sp<provider::V2_4::ICameraProvider> interface = startProviderInterface();
    if (interface == nullptr) {
        return DEAD_OBJECT;
    }
    ret = interface->getVendorTags(
        [&](auto s, const auto& vendorTagSecs) {
            status = s;
            if (s == Status::OK) {
                vts = vendorTagSecs;
            }
    });
    if (!ret.isOk()) {
        ALOGE("%s: Transaction error getting vendor tags from provider '%s': %s",
                __FUNCTION__, mProviderName.c_str(), ret.description().c_str());
        return DEAD_OBJECT;
    }
    if (status != Status::OK) {
        return mapToStatusT(status);
    }

    // Read all vendor tag definitions into a descriptor
    status_t res;
    if ((res = HidlVendorTagDescriptor::createDescriptorFromHidl(vts, /*out*/mVendorTagDescriptor))
            != OK) {
        ALOGE("%s: Could not generate descriptor from vendor tag operations,"
                "received error %s (%d). Camera clients will not be able to use"
                "vendor tags", __FUNCTION__, strerror(res), res);
        return res;
    }

    return OK;
}

status_t CameraProviderManager::ProviderInfo::notifyDeviceStateChange(
        hardware::hidl_bitfield<provider::V2_5::DeviceState> newDeviceState) {
    mDeviceState = newDeviceState;
    if (mMinorVersion >= 5) {
        // Check if the provider is currently active - not going to start it up for this notification
        auto interface = mSavedInterface != nullptr ? mSavedInterface : mActiveInterface.promote();
        if (interface != nullptr) {
            // Send current device state
            auto castResult = provider::V2_5::ICameraProvider::castFrom(interface);
            if (castResult.isOk()) {
                sp<provider::V2_5::ICameraProvider> interface_2_5 = castResult;
                if (interface_2_5 != nullptr) {
                    interface_2_5->notifyDeviceStateChange(mDeviceState);
                }
            }
        }
    }
    return OK;
}

template<class DeviceInfoT>
std::unique_ptr<CameraProviderManager::ProviderInfo::DeviceInfo>
    CameraProviderManager::ProviderInfo::initializeDeviceInfo(
        const std::string &name, const metadata_vendor_id_t tagId,
        const std::string &id, uint16_t minorVersion) {
    Status status;

    auto cameraInterface =
            startDeviceInterface<typename DeviceInfoT::InterfaceT>(name);
    if (cameraInterface == nullptr) return nullptr;

    CameraResourceCost resourceCost;
    cameraInterface->getResourceCost([&status, &resourceCost](
        Status s, CameraResourceCost cost) {
                status = s;
                resourceCost = cost;
            });
    if (status != Status::OK) {
        ALOGE("%s: Unable to obtain resource costs for camera device %s: %s", __FUNCTION__,
                name.c_str(), statusToString(status));
        return nullptr;
    }

    for (auto& conflictName : resourceCost.conflictingDevices) {
        uint16_t major, minor;
        std::string type, id;
        status_t res = parseDeviceName(conflictName, &major, &minor, &type, &id);
        if (res != OK) {
            ALOGE("%s: Failed to parse conflicting device %s", __FUNCTION__, conflictName.c_str());
            return nullptr;
        }
        conflictName = id;
    }

    return std::unique_ptr<DeviceInfo>(
        new DeviceInfoT(name, tagId, id, minorVersion, resourceCost, this,
                mProviderPublicCameraIds, cameraInterface));
}

template<class InterfaceT>
sp<InterfaceT>
CameraProviderManager::ProviderInfo::startDeviceInterface(const std::string &name) {
    ALOGE("%s: Device %s: Unknown HIDL device HAL major version %d:", __FUNCTION__,
            name.c_str(), InterfaceT::version.get_major());
    return nullptr;
}

template<>
sp<device::V1_0::ICameraDevice>
CameraProviderManager::ProviderInfo::startDeviceInterface
        <device::V1_0::ICameraDevice>(const std::string &name) {
    Status status;
    sp<device::V1_0::ICameraDevice> cameraInterface;
    hardware::Return<void> ret;
    const sp<provider::V2_4::ICameraProvider> interface = startProviderInterface();
    if (interface == nullptr) {
        return nullptr;
    }
    ret = interface->getCameraDeviceInterface_V1_x(name, [&status, &cameraInterface](
        Status s, sp<device::V1_0::ICameraDevice> interface) {
                status = s;
                cameraInterface = interface;
            });
    if (!ret.isOk()) {
        ALOGE("%s: Transaction error trying to obtain interface for camera device %s: %s",
                __FUNCTION__, name.c_str(), ret.description().c_str());
        return nullptr;
    }
    if (status != Status::OK) {
        ALOGE("%s: Unable to obtain interface for camera device %s: %s", __FUNCTION__,
                name.c_str(), statusToString(status));
        return nullptr;
    }
    return cameraInterface;
}

template<>
sp<device::V3_2::ICameraDevice>
CameraProviderManager::ProviderInfo::startDeviceInterface
        <device::V3_2::ICameraDevice>(const std::string &name) {
    Status status;
    sp<device::V3_2::ICameraDevice> cameraInterface;
    hardware::Return<void> ret;
    const sp<provider::V2_4::ICameraProvider> interface = startProviderInterface();
    if (interface == nullptr) {
        return nullptr;
    }
    ret = interface->getCameraDeviceInterface_V3_x(name, [&status, &cameraInterface](
        Status s, sp<device::V3_2::ICameraDevice> interface) {
                status = s;
                cameraInterface = interface;
            });
    if (!ret.isOk()) {
        ALOGE("%s: Transaction error trying to obtain interface for camera device %s: %s",
                __FUNCTION__, name.c_str(), ret.description().c_str());
        return nullptr;
    }
    if (status != Status::OK) {
        ALOGE("%s: Unable to obtain interface for camera device %s: %s", __FUNCTION__,
                name.c_str(), statusToString(status));
        return nullptr;
    }
    return cameraInterface;
}

CameraProviderManager::ProviderInfo::DeviceInfo::~DeviceInfo() {}

template<class InterfaceT>
sp<InterfaceT> CameraProviderManager::ProviderInfo::DeviceInfo::startDeviceInterface() {
    sp<InterfaceT> device;
    ATRACE_CALL();
    if (mSavedInterface == nullptr) {
        device = mParentProvider->startDeviceInterface<InterfaceT>(mName);
    } else {
        device = (InterfaceT *) mSavedInterface.get();
    }
    return device;
}

template<class InterfaceT>
status_t CameraProviderManager::ProviderInfo::DeviceInfo::setTorchMode(InterfaceT& interface,
        bool enabled) {
    Status s = interface->setTorchMode(enabled ? TorchMode::ON : TorchMode::OFF);
    return mapToStatusT(s);
}

CameraProviderManager::ProviderInfo::DeviceInfo1::DeviceInfo1(const std::string& name,
        const metadata_vendor_id_t tagId, const std::string &id,
        uint16_t minorVersion,
        const CameraResourceCost& resourceCost,
        sp<ProviderInfo> parentProvider,
        const std::vector<std::string>& publicCameraIds,
        sp<InterfaceT> interface) :
        DeviceInfo(name, tagId, id, hardware::hidl_version{1, minorVersion},
                   publicCameraIds, resourceCost, parentProvider) {
    // Get default parameters and initialize flash unit availability
    // Requires powering on the camera device
    hardware::Return<Status> status = interface->open(nullptr);
    if (!status.isOk()) {
        ALOGE("%s: Transaction error opening camera device %s to check for a flash unit: %s",
                __FUNCTION__, id.c_str(), status.description().c_str());
        return;
    }
    if (status != Status::OK) {
        ALOGE("%s: Unable to open camera device %s to check for a flash unit: %s", __FUNCTION__,
                id.c_str(), CameraProviderManager::statusToString(status));
        return;
    }
    hardware::Return<void> ret;
    ret = interface->getParameters([this](const hardware::hidl_string& parms) {
                mDefaultParameters.unflatten(String8(parms.c_str()));
            });
    if (!ret.isOk()) {
        ALOGE("%s: Transaction error reading camera device %s params to check for a flash unit: %s",
                __FUNCTION__, id.c_str(), status.description().c_str());
        return;
    }
    const char *flashMode =
            mDefaultParameters.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES);
    if (flashMode && strstr(flashMode, CameraParameters::FLASH_MODE_TORCH)) {
        mHasFlashUnit = true;
    }

    status_t res = cacheCameraInfo(interface);
    if (res != OK) {
        ALOGE("%s: Could not cache CameraInfo", __FUNCTION__);
        return;
    }

    ret = interface->close();
    if (!ret.isOk()) {
        ALOGE("%s: Transaction error closing camera device %s after check for a flash unit: %s",
                __FUNCTION__, id.c_str(), status.description().c_str());
    }

    if (!kEnableLazyHal) {
        // Save HAL reference indefinitely
        mSavedInterface = interface;
    }
}

CameraProviderManager::ProviderInfo::DeviceInfo1::~DeviceInfo1() {}

status_t CameraProviderManager::ProviderInfo::DeviceInfo1::setTorchMode(bool enabled) {
    return setTorchModeForDevice<InterfaceT>(enabled);
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo1::getCameraInfo(
        hardware::CameraInfo *info) const {
    if (info == nullptr) return BAD_VALUE;
    *info = mInfo;
    return OK;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo1::cacheCameraInfo(
        sp<CameraProviderManager::ProviderInfo::DeviceInfo1::InterfaceT> interface) {
    Status status;
    device::V1_0::CameraInfo cInfo;
    hardware::Return<void> ret;
    ret = interface->getCameraInfo([&status, &cInfo](Status s, device::V1_0::CameraInfo camInfo) {
                status = s;
                cInfo = camInfo;
            });
    if (!ret.isOk()) {
        ALOGE("%s: Transaction error reading camera info from device %s: %s",
                __FUNCTION__, mId.c_str(), ret.description().c_str());
        return DEAD_OBJECT;
    }
    if (status != Status::OK) {
        return mapToStatusT(status);
    }

    switch(cInfo.facing) {
        case device::V1_0::CameraFacing::BACK:
            mInfo.facing = hardware::CAMERA_FACING_BACK;
            break;
        case device::V1_0::CameraFacing::EXTERNAL:
            // Map external to front for legacy API
        case device::V1_0::CameraFacing::FRONT:
            mInfo.facing = hardware::CAMERA_FACING_FRONT;
            break;
        default:
            ALOGW("%s: Device %s: Unknown camera facing: %d",
                    __FUNCTION__, mId.c_str(), cInfo.facing);
            mInfo.facing = hardware::CAMERA_FACING_BACK;
    }
    mInfo.orientation = cInfo.orientation;

    return OK;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo1::dumpState(int fd) {
    native_handle_t* handle = native_handle_create(1,0);
    handle->data[0] = fd;
    const sp<InterfaceT> interface = startDeviceInterface<InterfaceT>();
    if (interface == nullptr) {
        return DEAD_OBJECT;
    }
    hardware::Return<Status> s = interface->dumpState(handle);
    native_handle_delete(handle);
    if (!s.isOk()) {
        return INVALID_OPERATION;
    }
    return mapToStatusT(s);
}

CameraProviderManager::ProviderInfo::DeviceInfo3::DeviceInfo3(const std::string& name,
        const metadata_vendor_id_t tagId, const std::string &id,
        uint16_t minorVersion,
        const CameraResourceCost& resourceCost,
        sp<ProviderInfo> parentProvider,
        const std::vector<std::string>& publicCameraIds,
        sp<InterfaceT> interface) :
        DeviceInfo(name, tagId, id, hardware::hidl_version{3, minorVersion},
                   publicCameraIds, resourceCost, parentProvider) {
    // Get camera characteristics and initialize flash unit availability
    Status status;
    hardware::Return<void> ret;
    ret = interface->getCameraCharacteristics([&status, this](Status s,
                    device::V3_2::CameraMetadata metadata) {
                status = s;
                if (s == Status::OK) {
                    camera_metadata_t *buffer =
                            reinterpret_cast<camera_metadata_t*>(metadata.data());
                    size_t expectedSize = metadata.size();
                    int res = validate_camera_metadata_structure(buffer, &expectedSize);
                    if (res == OK || res == CAMERA_METADATA_VALIDATION_SHIFTED) {
                        set_camera_metadata_vendor_id(buffer, mProviderTagid);
                        mCameraCharacteristics = buffer;
                    } else {
                        ALOGE("%s: Malformed camera metadata received from HAL", __FUNCTION__);
                        status = Status::INTERNAL_ERROR;
                    }
                }
            });
    if (!ret.isOk()) {
        ALOGE("%s: Transaction error getting camera characteristics for device %s"
                " to check for a flash unit: %s", __FUNCTION__, id.c_str(),
                ret.description().c_str());
        return;
    }
    if (status != Status::OK) {
        ALOGE("%s: Unable to get camera characteristics for device %s: %s (%d)",
                __FUNCTION__, id.c_str(), CameraProviderManager::statusToString(status), status);
        return;
    }

    mIsPublicallyHiddenSecureCamera = isPublicallyHiddenSecureCamera();

    status_t res = fixupMonochromeTags();
    if (OK != res) {
        ALOGE("%s: Unable to fix up monochrome tags based for older HAL version: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return;
    }
    auto stat = addDynamicDepthTags();
    if (OK != stat) {
        ALOGE("%s: Failed appending dynamic depth tags: %s (%d)", __FUNCTION__, strerror(-stat),
                stat);
    }
    res = deriveHeicTags();
    if (OK != res) {
        ALOGE("%s: Unable to derive HEIC tags based on camera and media capabilities: %s (%d)",
                __FUNCTION__, strerror(-res), res);
    }

    camera_metadata_entry flashAvailable =
            mCameraCharacteristics.find(ANDROID_FLASH_INFO_AVAILABLE);
    if (flashAvailable.count == 1 &&
            flashAvailable.data.u8[0] == ANDROID_FLASH_INFO_AVAILABLE_TRUE) {
        mHasFlashUnit = true;
    } else {
        mHasFlashUnit = false;
    }

    queryPhysicalCameraIds();

    // Get physical camera characteristics if applicable
    auto castResult = device::V3_5::ICameraDevice::castFrom(interface);
    if (!castResult.isOk()) {
        ALOGV("%s: Unable to convert ICameraDevice instance to version 3.5", __FUNCTION__);
        return;
    }
    sp<device::V3_5::ICameraDevice> interface_3_5 = castResult;
    if (interface_3_5 == nullptr) {
        ALOGE("%s: Converted ICameraDevice instance to nullptr", __FUNCTION__);
        return;
    }

    if (mIsLogicalCamera) {
        for (auto& id : mPhysicalIds) {
            if (std::find(mPublicCameraIds.begin(), mPublicCameraIds.end(), id) !=
                    mPublicCameraIds.end()) {
                continue;
            }

            hardware::hidl_string hidlId(id);
            ret = interface_3_5->getPhysicalCameraCharacteristics(hidlId,
                    [&status, &id, this](Status s, device::V3_2::CameraMetadata metadata) {
                status = s;
                if (s == Status::OK) {
                    camera_metadata_t *buffer =
                            reinterpret_cast<camera_metadata_t*>(metadata.data());
                    size_t expectedSize = metadata.size();
                    int res = validate_camera_metadata_structure(buffer, &expectedSize);
                    if (res == OK || res == CAMERA_METADATA_VALIDATION_SHIFTED) {
                        set_camera_metadata_vendor_id(buffer, mProviderTagid);
                        mPhysicalCameraCharacteristics[id] = buffer;
                    } else {
                        ALOGE("%s: Malformed camera metadata received from HAL", __FUNCTION__);
                        status = Status::INTERNAL_ERROR;
                    }
                }
            });

            if (!ret.isOk()) {
                ALOGE("%s: Transaction error getting physical camera %s characteristics for %s: %s",
                        __FUNCTION__, id.c_str(), id.c_str(), ret.description().c_str());
                return;
            }
            if (status != Status::OK) {
                ALOGE("%s: Unable to get physical camera %s characteristics for device %s: %s (%d)",
                        __FUNCTION__, id.c_str(), mId.c_str(),
                        CameraProviderManager::statusToString(status), status);
                return;
            }
        }
    }

    if (!kEnableLazyHal) {
        // Save HAL reference indefinitely
        mSavedInterface = interface;
    }
}

CameraProviderManager::ProviderInfo::DeviceInfo3::~DeviceInfo3() {}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::setTorchMode(bool enabled) {
    return setTorchModeForDevice<InterfaceT>(enabled);
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::getCameraInfo(
        hardware::CameraInfo *info) const {
    if (info == nullptr) return BAD_VALUE;

    camera_metadata_ro_entry facing =
            mCameraCharacteristics.find(ANDROID_LENS_FACING);
    if (facing.count == 1) {
        switch (facing.data.u8[0]) {
            case ANDROID_LENS_FACING_BACK:
                info->facing = hardware::CAMERA_FACING_BACK;
                break;
            case ANDROID_LENS_FACING_EXTERNAL:
                // Map external to front for legacy API
            case ANDROID_LENS_FACING_FRONT:
                info->facing = hardware::CAMERA_FACING_FRONT;
                break;
        }
    } else {
        ALOGE("%s: Unable to find android.lens.facing static metadata", __FUNCTION__);
        return NAME_NOT_FOUND;
    }

    camera_metadata_ro_entry orientation =
            mCameraCharacteristics.find(ANDROID_SENSOR_ORIENTATION);
    if (orientation.count == 1) {
        info->orientation = orientation.data.i32[0];
    } else {
        ALOGE("%s: Unable to find android.sensor.orientation static metadata", __FUNCTION__);
        return NAME_NOT_FOUND;
    }

    return OK;
}
bool CameraProviderManager::ProviderInfo::DeviceInfo3::isAPI1Compatible() const {
    // Do not advertise NIR cameras to API1 camera app.
    camera_metadata_ro_entry cfa = mCameraCharacteristics.find(
            ANDROID_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT);
    if (cfa.count == 1 && cfa.data.u8[0] == ANDROID_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_NIR) {
        return false;
    }

    bool isBackwardCompatible = false;
    camera_metadata_ro_entry_t caps = mCameraCharacteristics.find(
            ANDROID_REQUEST_AVAILABLE_CAPABILITIES);
    for (size_t i = 0; i < caps.count; i++) {
        if (caps.data.u8[i] ==
                ANDROID_REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE) {
            isBackwardCompatible = true;
            break;
        }
    }

    return isBackwardCompatible;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::dumpState(int fd) {
    native_handle_t* handle = native_handle_create(1,0);
    handle->data[0] = fd;
    const sp<InterfaceT> interface = startDeviceInterface<InterfaceT>();
    if (interface == nullptr) {
        return DEAD_OBJECT;
    }
    auto ret = interface->dumpState(handle);
    native_handle_delete(handle);
    if (!ret.isOk()) {
        return INVALID_OPERATION;
    }
    return OK;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::getCameraCharacteristics(
        CameraMetadata *characteristics) const {
    if (characteristics == nullptr) return BAD_VALUE;

    *characteristics = mCameraCharacteristics;
    return OK;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::getPhysicalCameraCharacteristics(
        const std::string& physicalCameraId, CameraMetadata *characteristics) const {
    if (characteristics == nullptr) return BAD_VALUE;
    if (mPhysicalCameraCharacteristics.find(physicalCameraId) ==
            mPhysicalCameraCharacteristics.end()) {
        return NAME_NOT_FOUND;
    }

    *characteristics = mPhysicalCameraCharacteristics.at(physicalCameraId);
    return OK;
}

status_t CameraProviderManager::ProviderInfo::DeviceInfo3::isSessionConfigurationSupported(
        const hardware::camera::device::V3_4::StreamConfiguration &configuration,
        bool *status /*out*/) {

    const sp<CameraProviderManager::ProviderInfo::DeviceInfo3::InterfaceT> interface =
            this->startDeviceInterface<CameraProviderManager::ProviderInfo::DeviceInfo3::InterfaceT>();
    if (interface == nullptr) {
        return DEAD_OBJECT;
    }
    auto castResult = device::V3_5::ICameraDevice::castFrom(interface);
    sp<hardware::camera::device::V3_5::ICameraDevice> interface_3_5 = castResult;
    if (interface_3_5 == nullptr) {
        return INVALID_OPERATION;
    }

    status_t res;
    Status callStatus;
    auto ret =  interface_3_5->isStreamCombinationSupported(configuration,
            [&callStatus, &status] (Status s, bool combStatus) {
                callStatus = s;
                *status = combStatus;
            });
    if (ret.isOk()) {
        switch (callStatus) {
            case Status::OK:
                // Expected case, do nothing.
                res = OK;
                break;
            case Status::METHOD_NOT_SUPPORTED:
                res = INVALID_OPERATION;
                break;
            default:
                ALOGE("%s: Session configuration query failed: %d", __FUNCTION__, callStatus);
                res = UNKNOWN_ERROR;
        }
    } else {
        ALOGE("%s: Unexpected binder error: %s", __FUNCTION__, ret.description().c_str());
        res = UNKNOWN_ERROR;
    }

    return res;
}

status_t CameraProviderManager::ProviderInfo::parseProviderName(const std::string& name,
        std::string *type, uint32_t *id) {
    // Format must be "<type>/<id>"
#define ERROR_MSG_PREFIX "%s: Invalid provider name '%s'. "       \
    "Should match '<type>/<id>' - "

    if (!type || !id) return INVALID_OPERATION;

    std::string::size_type slashIdx = name.find('/');
    if (slashIdx == std::string::npos || slashIdx == name.size() - 1) {
        ALOGE(ERROR_MSG_PREFIX
                "does not have / separator between type and id",
                __FUNCTION__, name.c_str());
        return BAD_VALUE;
    }

    std::string typeVal = name.substr(0, slashIdx);

    char *endPtr;
    errno = 0;
    long idVal = strtol(name.c_str() + slashIdx + 1, &endPtr, 10);
    if (errno != 0) {
        ALOGE(ERROR_MSG_PREFIX
                "cannot parse provider id as an integer: %s (%d)",
                __FUNCTION__, name.c_str(), strerror(errno), errno);
        return BAD_VALUE;
    }
    if (endPtr != name.c_str() + name.size()) {
        ALOGE(ERROR_MSG_PREFIX
                "provider id has unexpected length",
                __FUNCTION__, name.c_str());
        return BAD_VALUE;
    }
    if (idVal < 0) {
        ALOGE(ERROR_MSG_PREFIX
                "id is negative: %ld",
                __FUNCTION__, name.c_str(), idVal);
        return BAD_VALUE;
    }

#undef ERROR_MSG_PREFIX

    *type = typeVal;
    *id = static_cast<uint32_t>(idVal);

    return OK;
}

metadata_vendor_id_t CameraProviderManager::ProviderInfo::generateVendorTagId(
        const std::string &name) {
    metadata_vendor_id_t ret = std::hash<std::string> {} (name);
    // CAMERA_METADATA_INVALID_VENDOR_ID is not a valid hash value
    if (CAMERA_METADATA_INVALID_VENDOR_ID == ret) {
        ret = 0;
    }

    return ret;
}

status_t CameraProviderManager::ProviderInfo::parseDeviceName(const std::string& name,
        uint16_t *major, uint16_t *minor, std::string *type, std::string *id) {

    // Format must be "device@<major>.<minor>/<type>/<id>"

#define ERROR_MSG_PREFIX "%s: Invalid device name '%s'. " \
    "Should match 'device@<major>.<minor>/<type>/<id>' - "

    if (!major || !minor || !type || !id) return INVALID_OPERATION;

    // Verify starting prefix
    const char expectedPrefix[] = "device@";

    if (name.find(expectedPrefix) != 0) {
        ALOGE(ERROR_MSG_PREFIX
                "does not start with '%s'",
                __FUNCTION__, name.c_str(), expectedPrefix);
        return BAD_VALUE;
    }

    // Extract major/minor versions
    constexpr std::string::size_type atIdx = sizeof(expectedPrefix) - 2;
    std::string::size_type dotIdx = name.find('.', atIdx);
    if (dotIdx == std::string::npos) {
        ALOGE(ERROR_MSG_PREFIX
                "does not have @<major>. version section",
                __FUNCTION__, name.c_str());
        return BAD_VALUE;
    }
    std::string::size_type typeSlashIdx = name.find('/', dotIdx);
    if (typeSlashIdx == std::string::npos) {
        ALOGE(ERROR_MSG_PREFIX
                "does not have .<minor>/ version section",
                __FUNCTION__, name.c_str());
        return BAD_VALUE;
    }

    char *endPtr;
    errno = 0;
    long majorVal = strtol(name.c_str() + atIdx + 1, &endPtr, 10);
    if (errno != 0) {
        ALOGE(ERROR_MSG_PREFIX
                "cannot parse major version: %s (%d)",
                __FUNCTION__, name.c_str(), strerror(errno), errno);
        return BAD_VALUE;
    }
    if (endPtr != name.c_str() + dotIdx) {
        ALOGE(ERROR_MSG_PREFIX
                "major version has unexpected length",
                __FUNCTION__, name.c_str());
        return BAD_VALUE;
    }
    long minorVal = strtol(name.c_str() + dotIdx + 1, &endPtr, 10);
    if (errno != 0) {
        ALOGE(ERROR_MSG_PREFIX
                "cannot parse minor version: %s (%d)",
                __FUNCTION__, name.c_str(), strerror(errno), errno);
        return BAD_VALUE;
    }
    if (endPtr != name.c_str() + typeSlashIdx) {
        ALOGE(ERROR_MSG_PREFIX
                "minor version has unexpected length",
                __FUNCTION__, name.c_str());
        return BAD_VALUE;
    }
    if (majorVal < 0 || majorVal > UINT16_MAX || minorVal < 0 || minorVal > UINT16_MAX) {
        ALOGE(ERROR_MSG_PREFIX
                "major/minor version is out of range of uint16_t: %ld.%ld",
                __FUNCTION__, name.c_str(), majorVal, minorVal);
        return BAD_VALUE;
    }

    // Extract type and id

    std::string::size_type instanceSlashIdx = name.find('/', typeSlashIdx + 1);
    if (instanceSlashIdx == std::string::npos) {
        ALOGE(ERROR_MSG_PREFIX
                "does not have /<type>/ component",
                __FUNCTION__, name.c_str());
        return BAD_VALUE;
    }
    std::string typeVal = name.substr(typeSlashIdx + 1, instanceSlashIdx - typeSlashIdx - 1);

    if (instanceSlashIdx == name.size() - 1) {
        ALOGE(ERROR_MSG_PREFIX
                "does not have an /<id> component",
                __FUNCTION__, name.c_str());
        return BAD_VALUE;
    }
    std::string idVal = name.substr(instanceSlashIdx + 1);

#undef ERROR_MSG_PREFIX

    *major = static_cast<uint16_t>(majorVal);
    *minor = static_cast<uint16_t>(minorVal);
    *type = typeVal;
    *id = idVal;

    return OK;
}



CameraProviderManager::ProviderInfo::~ProviderInfo() {
    // Destruction of ProviderInfo is only supposed to happen when the respective
    // CameraProvider interface dies, so do not unregister callbacks.

}

status_t CameraProviderManager::mapToStatusT(const Status& s)  {
    switch(s) {
        case Status::OK:
            return OK;
        case Status::ILLEGAL_ARGUMENT:
            return BAD_VALUE;
        case Status::CAMERA_IN_USE:
            return -EBUSY;
        case Status::MAX_CAMERAS_IN_USE:
            return -EUSERS;
        case Status::METHOD_NOT_SUPPORTED:
            return UNKNOWN_TRANSACTION;
        case Status::OPERATION_NOT_SUPPORTED:
            return INVALID_OPERATION;
        case Status::CAMERA_DISCONNECTED:
            return DEAD_OBJECT;
        case Status::INTERNAL_ERROR:
            return INVALID_OPERATION;
    }
    ALOGW("Unexpected HAL status code %d", s);
    return INVALID_OPERATION;
}

const char* CameraProviderManager::statusToString(const Status& s) {
    switch(s) {
        case Status::OK:
            return "OK";
        case Status::ILLEGAL_ARGUMENT:
            return "ILLEGAL_ARGUMENT";
        case Status::CAMERA_IN_USE:
            return "CAMERA_IN_USE";
        case Status::MAX_CAMERAS_IN_USE:
            return "MAX_CAMERAS_IN_USE";
        case Status::METHOD_NOT_SUPPORTED:
            return "METHOD_NOT_SUPPORTED";
        case Status::OPERATION_NOT_SUPPORTED:
            return "OPERATION_NOT_SUPPORTED";
        case Status::CAMERA_DISCONNECTED:
            return "CAMERA_DISCONNECTED";
        case Status::INTERNAL_ERROR:
            return "INTERNAL_ERROR";
    }
    ALOGW("Unexpected HAL status code %d", s);
    return "UNKNOWN_ERROR";
}

const char* CameraProviderManager::deviceStatusToString(const CameraDeviceStatus& s) {
    switch(s) {
        case CameraDeviceStatus::NOT_PRESENT:
            return "NOT_PRESENT";
        case CameraDeviceStatus::PRESENT:
            return "PRESENT";
        case CameraDeviceStatus::ENUMERATING:
            return "ENUMERATING";
    }
    ALOGW("Unexpected HAL device status code %d", s);
    return "UNKNOWN_STATUS";
}

const char* CameraProviderManager::torchStatusToString(const TorchModeStatus& s) {
    switch(s) {
        case TorchModeStatus::NOT_AVAILABLE:
            return "NOT_AVAILABLE";
        case TorchModeStatus::AVAILABLE_OFF:
            return "AVAILABLE_OFF";
        case TorchModeStatus::AVAILABLE_ON:
            return "AVAILABLE_ON";
    }
    ALOGW("Unexpected HAL torch mode status code %d", s);
    return "UNKNOWN_STATUS";
}


status_t HidlVendorTagDescriptor::createDescriptorFromHidl(
        const hardware::hidl_vec<common::V1_0::VendorTagSection>& vts,
        /*out*/
        sp<VendorTagDescriptor>& descriptor) {

    int tagCount = 0;

    for (size_t s = 0; s < vts.size(); s++) {
        tagCount += vts[s].tags.size();
    }

    if (tagCount < 0 || tagCount > INT32_MAX) {
        ALOGE("%s: tag count %d from vendor tag sections is invalid.", __FUNCTION__, tagCount);
        return BAD_VALUE;
    }

    Vector<uint32_t> tagArray;
    LOG_ALWAYS_FATAL_IF(tagArray.resize(tagCount) != tagCount,
            "%s: too many (%u) vendor tags defined.", __FUNCTION__, tagCount);


    sp<HidlVendorTagDescriptor> desc = new HidlVendorTagDescriptor();
    desc->mTagCount = tagCount;

    SortedVector<String8> sections;
    KeyedVector<uint32_t, String8> tagToSectionMap;

    int idx = 0;
    for (size_t s = 0; s < vts.size(); s++) {
        const common::V1_0::VendorTagSection& section = vts[s];
        const char *sectionName = section.sectionName.c_str();
        if (sectionName == NULL) {
            ALOGE("%s: no section name defined for vendor tag section %zu.", __FUNCTION__, s);
            return BAD_VALUE;
        }
        String8 sectionString(sectionName);
        sections.add(sectionString);

        for (size_t j = 0; j < section.tags.size(); j++) {
            uint32_t tag = section.tags[j].tagId;
            if (tag < CAMERA_METADATA_VENDOR_TAG_BOUNDARY) {
                ALOGE("%s: vendor tag %d not in vendor tag section.", __FUNCTION__, tag);
                return BAD_VALUE;
            }

            tagArray.editItemAt(idx++) = section.tags[j].tagId;

            const char *tagName = section.tags[j].tagName.c_str();
            if (tagName == NULL) {
                ALOGE("%s: no tag name defined for vendor tag %d.", __FUNCTION__, tag);
                return BAD_VALUE;
            }
            desc->mTagToNameMap.add(tag, String8(tagName));
            tagToSectionMap.add(tag, sectionString);

            int tagType = (int) section.tags[j].tagType;
            if (tagType < 0 || tagType >= NUM_TYPES) {
                ALOGE("%s: tag type %d from vendor ops does not exist.", __FUNCTION__, tagType);
                return BAD_VALUE;
            }
            desc->mTagToTypeMap.add(tag, tagType);
        }
    }

    desc->mSections = sections;

    for (size_t i = 0; i < tagArray.size(); ++i) {
        uint32_t tag = tagArray[i];
        String8 sectionString = tagToSectionMap.valueFor(tag);

        // Set up tag to section index map
        ssize_t index = sections.indexOf(sectionString);
        LOG_ALWAYS_FATAL_IF(index < 0, "index %zd must be non-negative", index);
        desc->mTagToSectionMap.add(tag, static_cast<uint32_t>(index));

        // Set up reverse mapping
        ssize_t reverseIndex = -1;
        if ((reverseIndex = desc->mReverseMapping.indexOfKey(sectionString)) < 0) {
            KeyedVector<String8, uint32_t>* nameMapper = new KeyedVector<String8, uint32_t>();
            reverseIndex = desc->mReverseMapping.add(sectionString, nameMapper);
        }
        desc->mReverseMapping[reverseIndex]->add(desc->mTagToNameMap.valueFor(tag), tag);
    }

    descriptor = std::move(desc);
    return OK;
}

status_t CameraProviderManager::getCameraCharacteristicsLocked(const std::string &id,
        CameraMetadata* characteristics) const {
    auto deviceInfo = findDeviceInfoLocked(id, /*minVersion*/ {3,0}, /*maxVersion*/ {5,0});
    if (deviceInfo != nullptr) {
        return deviceInfo->getCameraCharacteristics(characteristics);
    }

    // Find hidden physical camera characteristics
    for (auto& provider : mProviders) {
        for (auto& deviceInfo : provider->mDevices) {
            status_t res = deviceInfo->getPhysicalCameraCharacteristics(id, characteristics);
            if (res != NAME_NOT_FOUND) return res;
        }
    }

    return NAME_NOT_FOUND;
}

void CameraProviderManager::filterLogicalCameraIdsLocked(
        std::vector<std::string>& deviceIds) const
{
    // Map between camera facing and camera IDs related to logical camera.
    std::map<int, std::unordered_set<std::string>> idCombos;

    // Collect all logical and its underlying physical camera IDs for each
    // facing.
    for (auto& deviceId : deviceIds) {
        auto deviceInfo = findDeviceInfoLocked(deviceId);
        if (deviceInfo == nullptr) continue;

        if (!deviceInfo->mIsLogicalCamera) {
            continue;
        }

        // combo contains the ids of a logical camera and its physical cameras
        std::vector<std::string> combo = deviceInfo->mPhysicalIds;
        combo.push_back(deviceId);

        hardware::CameraInfo info;
        status_t res = deviceInfo->getCameraInfo(&info);
        if (res != OK) {
            ALOGE("%s: Error reading camera info: %s (%d)", __FUNCTION__, strerror(-res), res);
            continue;
        }
        idCombos[info.facing].insert(combo.begin(), combo.end());
    }

    // Only expose one camera ID per facing for all logical and underlying
    // physical camera IDs.
    for (auto& r : idCombos) {
        auto& removedIds = r.second;
        for (auto& id : deviceIds) {
            auto foundId = std::find(removedIds.begin(), removedIds.end(), id);
            if (foundId == removedIds.end()) {
                continue;
            }

            removedIds.erase(foundId);
            break;
        }
        deviceIds.erase(std::remove_if(deviceIds.begin(), deviceIds.end(),
                [&removedIds](const std::string& s) {
                return removedIds.find(s) != removedIds.end();}),
                deviceIds.end());
    }
}

} // namespace android
