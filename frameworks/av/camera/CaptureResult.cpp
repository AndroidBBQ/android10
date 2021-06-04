/*
 * Copyright (C) 2014 The Android Open Source Project
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

#define LOG_TAG "Camera-CaptureResult"
#include <utils/Log.h>

#include <camera/CaptureResult.h>
#include <binder/Parcel.h>

namespace android {

bool CaptureResultExtras::isValid() {
    return requestId >= 0;
}

status_t CaptureResultExtras::readFromParcel(const android::Parcel *parcel) {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return BAD_VALUE;
    }

    parcel->readInt32(&requestId);
    parcel->readInt32(&burstId);
    parcel->readInt32(&afTriggerId);
    parcel->readInt32(&precaptureTriggerId);
    parcel->readInt64(&frameNumber);
    parcel->readInt32(&partialResultCount);
    parcel->readInt32(&errorStreamId);
    auto physicalCameraIdPresent = parcel->readBool();
    if (physicalCameraIdPresent) {
        String16 cameraId;
        status_t res = OK;
        if ((res = parcel->readString16(&cameraId)) != OK) {
            ALOGE("%s: Failed to read camera id: %d", __FUNCTION__, res);
            return res;
        }
        errorPhysicalCameraId = cameraId;
    }

    return OK;
}

status_t CaptureResultExtras::writeToParcel(android::Parcel *parcel) const {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return BAD_VALUE;
    }

    parcel->writeInt32(requestId);
    parcel->writeInt32(burstId);
    parcel->writeInt32(afTriggerId);
    parcel->writeInt32(precaptureTriggerId);
    parcel->writeInt64(frameNumber);
    parcel->writeInt32(partialResultCount);
    parcel->writeInt32(errorStreamId);
    if (errorPhysicalCameraId.size() > 0) {
        parcel->writeBool(true);
        status_t res = OK;
        if ((res = parcel->writeString16(errorPhysicalCameraId)) != OK) {
            ALOGE("%s: Failed to write physical camera ID to parcel: %d", __FUNCTION__, res);
            return res;
        }
    } else {
        parcel->writeBool(false);
    }

    return OK;
}

status_t PhysicalCaptureResultInfo::readFromParcel(const android::Parcel* parcel) {
    status_t res;

    mPhysicalCameraId.remove(mPhysicalCameraId.size());
    mPhysicalCameraMetadata.clear();

    if ((res = parcel->readString16(&mPhysicalCameraId)) != OK) {
        ALOGE("%s: Failed to read camera id: %d", __FUNCTION__, res);
        return res;
    }

    if ((res = mPhysicalCameraMetadata.readFromParcel(parcel)) != OK) {
        ALOGE("%s: Failed to read metadata from parcel: %d", __FUNCTION__, res);
        return res;
    }
    return OK;
}

status_t PhysicalCaptureResultInfo::writeToParcel(android::Parcel* parcel) const {
    status_t res;
    if ((res = parcel->writeString16(mPhysicalCameraId)) != OK) {
        ALOGE("%s: Failed to write physical camera ID to parcel: %d",
                __FUNCTION__, res);
        return res;
    }
    if ((res = mPhysicalCameraMetadata.writeToParcel(parcel)) != OK) {
        ALOGE("%s: Failed to write physical camera metadata to parcel: %d",
                __FUNCTION__, res);
        return res;
    }
    return OK;
}

CaptureResult::CaptureResult() :
        mMetadata(), mResultExtras() {
}

CaptureResult::CaptureResult(const CaptureResult &otherResult) {
    mResultExtras = otherResult.mResultExtras;
    mMetadata = otherResult.mMetadata;
    mPhysicalMetadatas = otherResult.mPhysicalMetadatas;
}

status_t CaptureResult::readFromParcel(android::Parcel *parcel) {

    ALOGV("%s: parcel = %p", __FUNCTION__, parcel);

    if (parcel == NULL) {
        ALOGE("%s: parcel is null", __FUNCTION__);
        return BAD_VALUE;
    }

    mMetadata.clear();
    mPhysicalMetadatas.clear();

    status_t res = OK;
    res = mMetadata.readFromParcel(parcel);
    if (res != OK) {
        ALOGE("%s: Failed to read metadata from parcel.",
              __FUNCTION__);
        return res;
    }
    ALOGV("%s: Read metadata from parcel", __FUNCTION__);

    int32_t physicalMetadataCount;
    if ((res = parcel->readInt32(&physicalMetadataCount)) != OK) {
        ALOGE("%s: Failed to read the physical metadata count from parcel: %d", __FUNCTION__, res);
        return res;
    }
    if (physicalMetadataCount < 0) {
        ALOGE("%s: Invalid physical metadata count from parcel: %d",
                __FUNCTION__, physicalMetadataCount);
        return BAD_VALUE;
    }

    for (int32_t i = 0; i < physicalMetadataCount; i++) {
        String16 cameraId;
        if ((res = parcel->readString16(&cameraId)) != OK) {
            ALOGE("%s: Failed to read camera id: %d", __FUNCTION__, res);
            return res;
        }

        CameraMetadata physicalMetadata;
        if ((res = physicalMetadata.readFromParcel(parcel)) != OK) {
            ALOGE("%s: Failed to read metadata from parcel: %d", __FUNCTION__, res);
            return res;
        }

        mPhysicalMetadatas.emplace(mPhysicalMetadatas.end(), cameraId, physicalMetadata);
    }
    ALOGV("%s: Read physical metadata from parcel", __FUNCTION__);

    res = mResultExtras.readFromParcel(parcel);
    if (res != OK) {
        ALOGE("%s: Failed to read result extras from parcel.",
                __FUNCTION__);
        return res;
    }
    ALOGV("%s: Read result extras from parcel", __FUNCTION__);

    return OK;
}

status_t CaptureResult::writeToParcel(android::Parcel *parcel) const {

    ALOGV("%s: parcel = %p", __FUNCTION__, parcel);

    if (parcel == NULL) {
        ALOGE("%s: parcel is null", __FUNCTION__);
        return BAD_VALUE;
    }

    status_t res;

    res = mMetadata.writeToParcel(parcel);
    if (res != OK) {
        ALOGE("%s: Failed to write metadata to parcel", __FUNCTION__);
        return res;
    }
    ALOGV("%s: Wrote metadata to parcel", __FUNCTION__);

    int32_t physicalMetadataCount = static_cast<int32_t>(mPhysicalMetadatas.size());
    res = parcel->writeInt32(physicalMetadataCount);
    if (res != OK) {
        ALOGE("%s: Failed to write physical metadata count to parcel: %d",
                __FUNCTION__, res);
        return BAD_VALUE;
    }
    for (const auto& physicalMetadata : mPhysicalMetadatas) {
        if ((res = parcel->writeString16(physicalMetadata.mPhysicalCameraId)) != OK) {
            ALOGE("%s: Failed to write physical camera ID to parcel: %d",
                    __FUNCTION__, res);
            return res;
        }
        if ((res = physicalMetadata.mPhysicalCameraMetadata.writeToParcel(parcel)) != OK) {
            ALOGE("%s: Failed to write physical camera metadata to parcel: %d",
                    __FUNCTION__, res);
            return res;
        }
    }
    ALOGV("%s: Wrote physical camera metadata to parcel", __FUNCTION__);

    res = mResultExtras.writeToParcel(parcel);
    if (res != OK) {
        ALOGE("%s: Failed to write result extras to parcel", __FUNCTION__);
        return res;
    }
    ALOGV("%s: Wrote result extras to parcel", __FUNCTION__);

    return OK;
}

}
