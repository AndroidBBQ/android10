/*
 * Copyright (C) 2015 The Android Open Source Project
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
#ifndef _ACAMERA_METADATA_H
#define _ACAMERA_METADATA_H

#include <unordered_set>
#include <vector>

#include <sys/types.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>

#ifdef __ANDROID_VNDK__
#include <CameraMetadata.h>
using CameraMetadata = android::hardware::camera::common::V1_0::helper::CameraMetadata;
#else
#include <camera/CameraMetadata.h>
#endif

#include <camera/NdkCameraMetadata.h>

using namespace android;

/**
 * ACameraMetadata opaque struct definition
 * Leave outside of android namespace because it's NDK struct
 */
struct ACameraMetadata : public RefBase {
  public:
    typedef enum {
        ACM_CHARACTERISTICS, // Read only
        ACM_REQUEST,         // Read/Write
        ACM_RESULT,          // Read only
    } ACAMERA_METADATA_TYPE;

    // Takes ownership of pass-in buffer
    ACameraMetadata(camera_metadata_t *buffer, ACAMERA_METADATA_TYPE type);
    // Clone
    ACameraMetadata(const ACameraMetadata& other) :
            mData(other.mData), mType(other.mType) {};

    camera_status_t getConstEntry(uint32_t tag, ACameraMetadata_const_entry* entry) const;

    camera_status_t update(uint32_t tag, uint32_t count, const uint8_t* data);
    camera_status_t update(uint32_t tag, uint32_t count, const int32_t* data);
    camera_status_t update(uint32_t tag, uint32_t count, const float* data);
    camera_status_t update(uint32_t tag, uint32_t count, const double* data);
    camera_status_t update(uint32_t tag, uint32_t count, const int64_t* data);
    camera_status_t update(uint32_t tag, uint32_t count, const ACameraMetadata_rational* data);

    camera_status_t getTags(/*out*/int32_t* numTags,
                            /*out*/const uint32_t** tags) const;

    const CameraMetadata& getInternalData() const;
    bool isLogicalMultiCamera(size_t* count, const char* const** physicalCameraIds) const;

  private:

    // This function does not check whether the capability passed to it is valid.
    // The caller must make sure that it is.
    bool isNdkSupportedCapability(const int32_t capability);
    static inline bool isVendorTag(const uint32_t tag);
    static bool isCaptureRequestTag(const uint32_t tag);
    void filterUnsupportedFeatures(); // Hide features not yet supported by NDK
    void filterStreamConfigurations(); // Hide input streams, translate hal format to NDK formats
    void filterDurations(uint32_t tag); // translate hal format to NDK formats
    void derivePhysicalCameraIds(); // Derive array of physical ids.

    template<typename INTERNAL_T, typename NDK_T>
    camera_status_t updateImpl(uint32_t tag, uint32_t count, const NDK_T* data) {
        if (mType != ACM_REQUEST) {
            ALOGE("Error: Write to metadata is only allowed for capture request!");
            return ACAMERA_ERROR_INVALID_PARAMETER;
        }
        if (!isCaptureRequestTag(tag)) {
            ALOGE("Error: tag %d is not writable!", tag);
            return ACAMERA_ERROR_INVALID_PARAMETER;
        }

        Mutex::Autolock _l(mLock);

        status_t ret = OK;
        if (count == 0 && data == nullptr) {
            ret = mData.erase(tag);
        } else {
            // Here we have to use reinterpret_cast because the NDK data type is
            // exact copy of internal data type but they do not inherit from each other
            ret = mData.update(tag, reinterpret_cast<const INTERNAL_T*>(data), count);
        }

        if (ret == OK) {
            mTags.clear();
            return ACAMERA_OK;
        } else {
            return ACAMERA_ERROR_INVALID_PARAMETER;
        }
    }

    // guard access of public APIs: get/update/getTags
    mutable Mutex    mLock;
    CameraMetadata   mData;
    mutable Vector<uint32_t> mTags; // updated in getTags, cleared by update
    const ACAMERA_METADATA_TYPE mType;

    static std::unordered_set<uint32_t> sSystemTags;

    std::vector<const char*> mStaticPhysicalCameraIds;
    std::vector<String8> mStaticPhysicalCameraIdValues;
};

#endif // _ACAMERA_METADATA_H
