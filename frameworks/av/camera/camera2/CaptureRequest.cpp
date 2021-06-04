/*
**
** Copyright 2013, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

// #define LOG_NDEBUG 0
#define LOG_TAG "CameraRequest"
#include <utils/Log.h>
#include <utils/String16.h>

#include <camera/camera2/CaptureRequest.h>

#include <binder/Parcel.h>
#include <gui/Surface.h>
#include <gui/view/Surface.h>

namespace android {
namespace hardware {
namespace camera2 {

// These must be in the .cpp (to avoid inlining)
CaptureRequest::CaptureRequest() = default;
CaptureRequest::~CaptureRequest() = default;
CaptureRequest::CaptureRequest(const CaptureRequest& rhs) = default;
CaptureRequest::CaptureRequest(CaptureRequest&& rhs) noexcept = default;


status_t CaptureRequest::readFromParcel(const android::Parcel* parcel) {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return BAD_VALUE;
    }

    mSurfaceList.clear();
    mStreamIdxList.clear();
    mSurfaceIdxList.clear();
    mPhysicalCameraSettings.clear();

    status_t err = OK;

    int32_t settingsCount;
    if ((err = parcel->readInt32(&settingsCount)) != OK) {
        ALOGE("%s: Failed to read the settings count from parcel: %d", __FUNCTION__, err);
        return err;
    }

    if (settingsCount <= 0) {
        ALOGE("%s: Settings count %d should always be positive!", __FUNCTION__, settingsCount);
        return BAD_VALUE;
    }

    for (int32_t i = 0; i < settingsCount; i++) {
        String16 id;
        if ((err = parcel->readString16(&id)) != OK) {
            ALOGE("%s: Failed to read camera id!", __FUNCTION__);
            return BAD_VALUE;
        }

        CameraMetadata settings;
        if ((err = settings.readFromParcel(parcel)) != OK) {
            ALOGE("%s: Failed to read metadata from parcel", __FUNCTION__);
            return err;
        }
        ALOGV("%s: Read metadata from parcel", __FUNCTION__);
        mPhysicalCameraSettings.push_back({std::string(String8(id).string()), settings});
    }

    int isReprocess = 0;
    if ((err = parcel->readInt32(&isReprocess)) != OK) {
        ALOGE("%s: Failed to read reprocessing from parcel", __FUNCTION__);
        return err;
    }
    mIsReprocess = (isReprocess != 0);

    int32_t size;
    if ((err = parcel->readInt32(&size)) != OK) {
        ALOGE("%s: Failed to read surface list size from parcel", __FUNCTION__);
        return err;
    }
    ALOGV("%s: Read surface list size = %d", __FUNCTION__, size);

    // Do not distinguish null arrays from 0-sized arrays.
    for (int32_t i = 0; i < size; ++i) {
        // Parcel.writeParcelableArray
        size_t len;
        const char16_t* className = parcel->readString16Inplace(&len);
        ALOGV("%s: Read surface class = %s", __FUNCTION__,
              className != NULL ? String8(className).string() : "<null>");

        if (className == NULL) {
            continue;
        }

        // Surface.writeToParcel
        view::Surface surfaceShim;
        if ((err = surfaceShim.readFromParcel(parcel)) != OK) {
            ALOGE("%s: Failed to read output target Surface %d from parcel: %s (%d)",
                    __FUNCTION__, i, strerror(-err), err);
            return err;
        }

        sp<Surface> surface;
        if (surfaceShim.graphicBufferProducer != NULL) {
            surface = new Surface(surfaceShim.graphicBufferProducer);
        }

        mSurfaceList.push_back(surface);
    }

    int32_t streamSurfaceSize;
    if ((err = parcel->readInt32(&streamSurfaceSize)) != OK) {
        ALOGE("%s: Failed to read streamSurfaceSize from parcel", __FUNCTION__);
        return err;
    }

    if (streamSurfaceSize < 0) {
        ALOGE("%s: Bad streamSurfaceSize %d from parcel", __FUNCTION__, streamSurfaceSize);
        return BAD_VALUE;
    }

    for (int32_t i = 0; i < streamSurfaceSize; ++i) {
        int streamIdx;
        if ((err = parcel->readInt32(&streamIdx)) != OK) {
            ALOGE("%s: Failed to read stream index from parcel", __FUNCTION__);
            return err;
        }
        mStreamIdxList.push_back(streamIdx);

        int surfaceIdx;
        if ((err = parcel->readInt32(&surfaceIdx)) != OK) {
            ALOGE("%s: Failed to read surface index from parcel", __FUNCTION__);
            return err;
        }
        mSurfaceIdxList.push_back(surfaceIdx);
    }

    return OK;
}

status_t CaptureRequest::writeToParcel(android::Parcel* parcel) const {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return BAD_VALUE;
    }

    status_t err = OK;

    int32_t settingsCount = static_cast<int32_t>(mPhysicalCameraSettings.size());

    if ((err = parcel->writeInt32(settingsCount)) != OK) {
        ALOGE("%s: Failed to write settings count!", __FUNCTION__);
        return err;
    }

    for (const auto &it : mPhysicalCameraSettings) {
        if ((err = parcel->writeString16(String16(it.id.c_str()))) != OK) {
            ALOGE("%s: Failed to camera id!", __FUNCTION__);
            return err;
        }

        if ((err = it.settings.writeToParcel(parcel)) != OK) {
            ALOGE("%s: Failed to write settings!", __FUNCTION__);
            return err;
        }
    }

    parcel->writeInt32(mIsReprocess ? 1 : 0);

    if (mSurfaceConverted) {
        parcel->writeInt32(0); // 0-sized array
    } else {
        int32_t size = static_cast<int32_t>(mSurfaceList.size());

        // Send 0-sized arrays when it's empty. Do not send null arrays.
        parcel->writeInt32(size);

        for (int32_t i = 0; i < size; ++i) {
            // not sure if readParcelableArray does this, hard to tell from source
            parcel->writeString16(String16("android.view.Surface"));

            // Surface.writeToParcel
            view::Surface surfaceShim;
            surfaceShim.name = String16("unknown_name");
            surfaceShim.graphicBufferProducer = mSurfaceList[i]->getIGraphicBufferProducer();
            if ((err = surfaceShim.writeToParcel(parcel)) != OK) {
                ALOGE("%s: Failed to write output target Surface %d to parcel: %s (%d)",
                        __FUNCTION__, i, strerror(-err), err);
                return err;
            }
        }
    }

    parcel->writeInt32(mStreamIdxList.size());
    for (size_t i = 0; i < mStreamIdxList.size(); ++i) {
        if ((err = parcel->writeInt32(mStreamIdxList[i])) != OK) {
            ALOGE("%s: Failed to write stream index to parcel", __FUNCTION__);
            return err;
        }
        if ((err = parcel->writeInt32(mSurfaceIdxList[i])) != OK) {
            ALOGE("%s: Failed to write surface index to parcel", __FUNCTION__);
            return err;
        }
    }
    return OK;
}

} // namespace camera2
} // namespace hardware
} // namespace android
