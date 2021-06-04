/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <hidl/Convert.h>

#include <hidl/HidlCameraService.h>

#include <hidl/HidlCameraDeviceUser.h>
#include <hidl/AidlCameraDeviceCallbacks.h>
#include <hidl/AidlCameraServiceListener.h>

#include <hidl/HidlTransportSupport.h>

namespace android {
namespace frameworks {
namespace cameraservice {
namespace service {
namespace V2_0 {
namespace implementation {

using frameworks::cameraservice::service::V2_0::implementation::HidlCameraService;
using hardware::hidl_vec;
using hardware::cameraservice::utils::conversion::convertToHidl;
using hardware::cameraservice::utils::conversion::B2HStatus;
using hardware::Void;

using device::V2_0::implementation::H2BCameraDeviceCallbacks;
using device::V2_0::implementation::HidlCameraDeviceUser;
using service::V2_0::implementation::H2BCameraServiceListener;
using HCameraMetadataType = frameworks::cameraservice::common::V2_0::CameraMetadataType;
using HVendorTag = frameworks::cameraservice::common::V2_0::VendorTag;
using HVendorTagSection = frameworks::cameraservice::common::V2_0::VendorTagSection;
using HProviderIdAndVendorTagSections =
        frameworks::cameraservice::common::V2_0::ProviderIdAndVendorTagSections;

sp<HidlCameraService> gHidlCameraService;

sp<HidlCameraService> HidlCameraService::getInstance(android::CameraService *cs) {
    gHidlCameraService = new HidlCameraService(cs);
    return gHidlCameraService;
}

Return<void>
HidlCameraService::getCameraCharacteristics(const hidl_string& cameraId,
                                            getCameraCharacteristics_cb _hidl_cb) {
    android::CameraMetadata cameraMetadata;
    HStatus status = HStatus::NO_ERROR;
    binder::Status serviceRet =
        mAidlICameraService->getCameraCharacteristics(String16(cameraId.c_str()), &cameraMetadata);
    HCameraMetadata hidlMetadata;
    if (!serviceRet.isOk()) {
        switch(serviceRet.serviceSpecificErrorCode()) {
            // No ERROR_CAMERA_DISCONNECTED since we're in the same process.
            case hardware::ICameraService::ERROR_ILLEGAL_ARGUMENT:
                ALOGE("%s: Camera ID %s does not exist!", __FUNCTION__, cameraId.c_str());
                status = HStatus::ILLEGAL_ARGUMENT;
                break;
            default:
                ALOGE("Get camera characteristics from camera service failed: %s",
                      serviceRet.toString8().string());
                status = B2HStatus(serviceRet);
          }
        _hidl_cb(status, hidlMetadata);
        return Void();
    }
    const camera_metadata_t *rawMetadata = cameraMetadata.getAndLock();
    convertToHidl(rawMetadata, &hidlMetadata);
    _hidl_cb(status, hidlMetadata);
    cameraMetadata.unlock(rawMetadata);
    return Void();
}

Return<void> HidlCameraService::connectDevice(const sp<HCameraDeviceCallback>& hCallback,
                                              const hidl_string& cameraId,
                                              connectDevice_cb _hidl_cb) {
    // Here, we first get ICameraDeviceUser from mAidlICameraService, then save
    // that interface in the newly created HidlCameraDeviceUser impl class.
    if (mAidlICameraService == nullptr) {
        _hidl_cb(HStatus::UNKNOWN_ERROR, nullptr);
        return Void();
    }
    sp<hardware::camera2::ICameraDeviceUser> deviceRemote = nullptr;
    // Create a hardware::camera2::ICameraDeviceCallback object which internally
    // calls callback functions passed through hCallback.
    sp<H2BCameraDeviceCallbacks> hybridCallbacks = new H2BCameraDeviceCallbacks(hCallback);
    if (!hybridCallbacks->initializeLooper()) {
        ALOGE("Unable to handle callbacks on device, cannot connect");
        _hidl_cb(HStatus::UNKNOWN_ERROR, nullptr);
        return Void();
    }
    sp<hardware::camera2::ICameraDeviceCallbacks> callbacks = hybridCallbacks;
    binder::Status serviceRet = mAidlICameraService->connectDevice(
            callbacks, String16(cameraId.c_str()), String16(""),
            hardware::ICameraService::USE_CALLING_UID, /*out*/&deviceRemote);
    HStatus status = HStatus::NO_ERROR;
    if (!serviceRet.isOk()) {
        ALOGE("%s: Unable to connect to camera device", __FUNCTION__);
        status = B2HStatus(serviceRet);
        _hidl_cb(status, nullptr);
        return Void();
    }
    // Now we create a HidlCameraDeviceUser class, store the deviceRemote in it,
    // and return that back. All calls on that interface will be forwarded to
    // the AIDL interface.
    sp<HidlCameraDeviceUser> hDeviceRemote = new HidlCameraDeviceUser(deviceRemote);
    if (!hDeviceRemote->initStatus()) {
        ALOGE("%s: Unable to initialize camera device HIDL wrapper", __FUNCTION__);
        _hidl_cb(HStatus::UNKNOWN_ERROR, nullptr);
        return Void();
    }
    hybridCallbacks->setCaptureResultMetadataQueue(hDeviceRemote->getCaptureResultMetadataQueue());
    _hidl_cb(status, hDeviceRemote);
    return Void();
}

void HidlCameraService::addToListenerCacheLocked(sp<HCameraServiceListener> hListener,
                                                 sp<hardware::ICameraServiceListener> csListener) {
        mListeners.emplace_back(std::make_pair(hListener, csListener));
}

sp<hardware::ICameraServiceListener>
HidlCameraService::searchListenerCacheLocked(sp<HCameraServiceListener> hListener,
                                             bool shouldRemove) {
    // Go through the mListeners list and compare the listener with the HIDL
    // listener registered.
    auto it = mListeners.begin();
    sp<ICameraServiceListener> csListener = nullptr;
    for (;it != mListeners.end(); it++) {
        if (hardware::interfacesEqual(it->first, hListener)) {
            break;
        }
    }
    if (it != mListeners.end()) {
        csListener = it->second;
        if (shouldRemove) {
          mListeners.erase(it);
        }
    }
    return csListener;
}

Return<void> HidlCameraService::addListener(const sp<HCameraServiceListener>& hCsListener,
                                            addListener_cb _hidl_cb) {
    if (mAidlICameraService == nullptr) {
        _hidl_cb(HStatus::UNKNOWN_ERROR, {});
        return Void();
    }
    if (hCsListener == nullptr) {
        ALOGE("%s listener must not be NULL", __FUNCTION__);
        _hidl_cb(HStatus::ILLEGAL_ARGUMENT, {});
        return Void();
    }
    sp<hardware::ICameraServiceListener> csListener = nullptr;
    // Check the cache for previously registered callbacks
    {
        Mutex::Autolock l(mListenerListLock);
        csListener = searchListenerCacheLocked(hCsListener);
        if (csListener == nullptr) {
            // Wrap an hCsListener with AidlCameraServiceListener and pass it to
            // CameraService.
            csListener = new H2BCameraServiceListener(hCsListener);
            // Add to cache
            addToListenerCacheLocked(hCsListener, csListener);
        } else {
            ALOGE("%s: Trying to add a listener %p already registered",
                  __FUNCTION__, hCsListener.get());
            _hidl_cb(HStatus::ILLEGAL_ARGUMENT, {});
            return Void();
        }
    }
    std::vector<hardware::CameraStatus> cameraStatusAndIds{};
    binder::Status serviceRet =
        mAidlICameraService->addListenerHelper(csListener, &cameraStatusAndIds, true);
    HStatus status = HStatus::NO_ERROR;
    if (!serviceRet.isOk()) {
      ALOGE("%s: Unable to add camera device status listener", __FUNCTION__);
      status = B2HStatus(serviceRet);
      _hidl_cb(status, {});
      return Void();
    }
    hidl_vec<HCameraStatusAndId> hCameraStatusAndIds;
    //Convert cameraStatusAndIds to HIDL and call callback
    convertToHidl(cameraStatusAndIds, &hCameraStatusAndIds);
    _hidl_cb(status, hCameraStatusAndIds);
    return Void();
}

Return<HStatus> HidlCameraService::removeListener(const sp<HCameraServiceListener>& hCsListener) {
    if (hCsListener == nullptr) {
        ALOGE("%s listener must not be NULL", __FUNCTION__);
        return HStatus::ILLEGAL_ARGUMENT;
    }
    sp<ICameraServiceListener> csListener = nullptr;
    {
        Mutex::Autolock l(mListenerListLock);
        csListener = searchListenerCacheLocked(hCsListener, /*removeIfFound*/true);
    }
    if (csListener != nullptr) {
          mAidlICameraService->removeListener(csListener);
    } else {
        ALOGE("%s Removing unregistered listener %p", __FUNCTION__, hCsListener.get());
        return HStatus::ILLEGAL_ARGUMENT;
    }
    return HStatus::NO_ERROR;
}

Return<void> HidlCameraService::getCameraVendorTagSections(getCameraVendorTagSections_cb _hidl_cb) {
    sp<VendorTagDescriptorCache> gCache = VendorTagDescriptorCache::getGlobalVendorTagCache();
    if (gCache == nullptr) {
        _hidl_cb(HStatus::UNKNOWN_ERROR, {});
        return Void();
    }
    const std::unordered_map<metadata_vendor_id_t, sp<android::VendorTagDescriptor>>
            &vendorIdsAndTagDescs = gCache->getVendorIdsAndTagDescriptors();
    if (vendorIdsAndTagDescs.size() == 0) {
        _hidl_cb(HStatus::UNKNOWN_ERROR, {});
        return Void();
    }

    hidl_vec<HProviderIdAndVendorTagSections> hTagIdsAndVendorTagSections;
    hTagIdsAndVendorTagSections.resize(vendorIdsAndTagDescs.size());
    size_t j = 0;
    for (auto &vendorIdAndTagDescs : vendorIdsAndTagDescs) {
        hidl_vec<HVendorTagSection> hVendorTagSections;
        sp<VendorTagDescriptor> desc = vendorIdAndTagDescs.second;
        const SortedVector<String8>* sectionNames = desc->getAllSectionNames();
        size_t numSections = sectionNames->size();
        std::vector<std::vector<HVendorTag>> tagsBySection(numSections);
        int tagCount = desc->getTagCount();
        std::vector<uint32_t> tags(tagCount);
        desc->getTagArray(tags.data());
        for (int i = 0; i < tagCount; i++) {
            HVendorTag vt;
            vt.tagId = tags[i];
            vt.tagName = desc->getTagName(tags[i]);
            vt.tagType = (HCameraMetadataType) desc->getTagType(tags[i]);
            ssize_t sectionIdx = desc->getSectionIndex(tags[i]);
            tagsBySection[sectionIdx].push_back(vt);
        }
        hVendorTagSections.resize(numSections);
        for (size_t s = 0; s < numSections; s++) {
            hVendorTagSections[s].sectionName = (*sectionNames)[s].string();
            hVendorTagSections[s].tags = tagsBySection[s];
        }
        HProviderIdAndVendorTagSections &hProviderIdAndVendorTagSections =
                hTagIdsAndVendorTagSections[j];
        hProviderIdAndVendorTagSections.providerId = vendorIdAndTagDescs.first;
        hProviderIdAndVendorTagSections.vendorTagSections = std::move(hVendorTagSections);
        j++;
    }
    _hidl_cb(HStatus::NO_ERROR, hTagIdsAndVendorTagSections);
    return Void();
}

} // implementation
} // V2_0
} // service
} // cameraservice
} // frameworks
} // android

