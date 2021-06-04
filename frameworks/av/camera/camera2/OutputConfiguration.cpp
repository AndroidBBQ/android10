/*
**
** Copyright 2015-2018, The Android Open Source Project
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

#define LOG_TAG "OutputConfiguration"
//#define LOG_NDEBUG 0

#include <utils/Log.h>

#include <camera/camera2/OutputConfiguration.h>
#include <binder/Parcel.h>
#include <gui/view/Surface.h>
#include <utils/String8.h>

namespace android {


const int OutputConfiguration::INVALID_ROTATION = -1;
const int OutputConfiguration::INVALID_SET_ID = -1;

const std::vector<sp<IGraphicBufferProducer>>&
        OutputConfiguration::getGraphicBufferProducers() const {
    return mGbps;
}

int OutputConfiguration::getRotation() const {
    return mRotation;
}

int OutputConfiguration::getSurfaceSetID() const {
    return mSurfaceSetID;
}

int OutputConfiguration::getSurfaceType() const {
    return mSurfaceType;
}

int OutputConfiguration::getWidth() const {
    return mWidth;
}

int OutputConfiguration::getHeight() const {
    return mHeight;
}

bool OutputConfiguration::isDeferred() const {
    return mIsDeferred;
}

bool OutputConfiguration::isShared() const {
    return mIsShared;
}

String16 OutputConfiguration::getPhysicalCameraId() const {
    return mPhysicalCameraId;
}

OutputConfiguration::OutputConfiguration() :
        mRotation(INVALID_ROTATION),
        mSurfaceSetID(INVALID_SET_ID),
        mSurfaceType(SURFACE_TYPE_UNKNOWN),
        mWidth(0),
        mHeight(0),
        mIsDeferred(false),
        mIsShared(false) {
}

OutputConfiguration::OutputConfiguration(const android::Parcel& parcel) :
        mRotation(INVALID_ROTATION),
        mSurfaceSetID(INVALID_SET_ID) {
    readFromParcel(&parcel);
}

status_t OutputConfiguration::readFromParcel(const android::Parcel* parcel) {
    status_t err = OK;
    int rotation = 0;

    if (parcel == nullptr) return BAD_VALUE;

    if ((err = parcel->readInt32(&rotation)) != OK) {
        ALOGE("%s: Failed to read rotation from parcel", __FUNCTION__);
        return err;
    }

    int setID = INVALID_SET_ID;
    if ((err = parcel->readInt32(&setID)) != OK) {
        ALOGE("%s: Failed to read surface set ID from parcel", __FUNCTION__);
        return err;
    }

    int surfaceType = SURFACE_TYPE_UNKNOWN;
    if ((err = parcel->readInt32(&surfaceType)) != OK) {
        ALOGE("%s: Failed to read surface type from parcel", __FUNCTION__);
        return err;
    }

    int width = 0;
    if ((err = parcel->readInt32(&width)) != OK) {
        ALOGE("%s: Failed to read surface width from parcel", __FUNCTION__);
        return err;
    }

    int height = 0;
    if ((err = parcel->readInt32(&height)) != OK) {
        ALOGE("%s: Failed to read surface height from parcel", __FUNCTION__);
        return err;
    }

    int isDeferred = 0;
    if ((err = parcel->readInt32(&isDeferred)) != OK) {
        ALOGE("%s: Failed to read surface isDeferred flag from parcel", __FUNCTION__);
        return err;
    }

    int isShared = 0;
    if ((err = parcel->readInt32(&isShared)) != OK) {
        ALOGE("%s: Failed to read surface isShared flag from parcel", __FUNCTION__);
        return err;
    }

    if (isDeferred && surfaceType != SURFACE_TYPE_SURFACE_VIEW &&
            surfaceType != SURFACE_TYPE_SURFACE_TEXTURE) {
        ALOGE("%s: Invalid surface type for deferred configuration", __FUNCTION__);
        return BAD_VALUE;
    }

    std::vector<view::Surface> surfaceShims;
    if ((err = parcel->readParcelableVector(&surfaceShims)) != OK) {
        ALOGE("%s: Failed to read surface(s) from parcel", __FUNCTION__);
        return err;
    }

    parcel->readString16(&mPhysicalCameraId);

    mRotation = rotation;
    mSurfaceSetID = setID;
    mSurfaceType = surfaceType;
    mWidth = width;
    mHeight = height;
    mIsDeferred = isDeferred != 0;
    mIsShared = isShared != 0;
    for (auto& surface : surfaceShims) {
        ALOGV("%s: OutputConfiguration: %p, name %s", __FUNCTION__,
                surface.graphicBufferProducer.get(),
                String8(surface.name).string());
        mGbps.push_back(surface.graphicBufferProducer);
    }

    ALOGV("%s: OutputConfiguration: rotation = %d, setId = %d, surfaceType = %d,"
          " physicalCameraId = %s", __FUNCTION__, mRotation, mSurfaceSetID,
          mSurfaceType, String8(mPhysicalCameraId).string());

    return err;
}

OutputConfiguration::OutputConfiguration(sp<IGraphicBufferProducer>& gbp, int rotation,
        const String16& physicalId,
        int surfaceSetID, bool isShared) {
    mGbps.push_back(gbp);
    mRotation = rotation;
    mSurfaceSetID = surfaceSetID;
    mIsDeferred = false;
    mIsShared = isShared;
    mPhysicalCameraId = physicalId;
}

OutputConfiguration::OutputConfiguration(
        const std::vector<sp<IGraphicBufferProducer>>& gbps,
    int rotation, const String16& physicalCameraId, int surfaceSetID,  int surfaceType,
    int width, int height, bool isShared)
  : mGbps(gbps), mRotation(rotation), mSurfaceSetID(surfaceSetID), mSurfaceType(surfaceType),
    mWidth(width), mHeight(height), mIsDeferred(false), mIsShared(isShared),
    mPhysicalCameraId(physicalCameraId) { }

status_t OutputConfiguration::writeToParcel(android::Parcel* parcel) const {

    if (parcel == nullptr) return BAD_VALUE;
    status_t err = OK;

    err = parcel->writeInt32(mRotation);
    if (err != OK) return err;

    err = parcel->writeInt32(mSurfaceSetID);
    if (err != OK) return err;

    err = parcel->writeInt32(mSurfaceType);
    if (err != OK) return err;

    err = parcel->writeInt32(mWidth);
    if (err != OK) return err;

    err = parcel->writeInt32(mHeight);
    if (err != OK) return err;

    err = parcel->writeInt32(mIsDeferred ? 1 : 0);
    if (err != OK) return err;

    err = parcel->writeInt32(mIsShared ? 1 : 0);
    if (err != OK) return err;

    std::vector<view::Surface> surfaceShims;
    for (auto& gbp : mGbps) {
        view::Surface surfaceShim;
        surfaceShim.name = String16("unknown_name"); // name of surface
        surfaceShim.graphicBufferProducer = gbp;
        surfaceShims.push_back(surfaceShim);
    }
    err = parcel->writeParcelableVector(surfaceShims);
    if (err != OK) return err;

    err = parcel->writeString16(mPhysicalCameraId);
    if (err != OK) return err;

    return OK;
}

bool OutputConfiguration::gbpsEqual(const OutputConfiguration& other) const {
    const std::vector<sp<IGraphicBufferProducer> >& otherGbps =
            other.getGraphicBufferProducers();

    if (mGbps.size() != otherGbps.size()) {
        return false;
    }

    for (size_t i = 0; i < mGbps.size(); i++) {
        if (mGbps[i] != otherGbps[i]) {
            return false;
        }
    }

    return true;
}

bool OutputConfiguration::gbpsLessThan(const OutputConfiguration& other) const {
    const std::vector<sp<IGraphicBufferProducer> >& otherGbps =
            other.getGraphicBufferProducers();

    if (mGbps.size() !=  otherGbps.size()) {
        return mGbps.size() < otherGbps.size();
    }

    for (size_t i = 0; i < mGbps.size(); i++) {
        if (mGbps[i] != otherGbps[i]) {
            return mGbps[i] < otherGbps[i];
        }
    }

    return false;
}
}; // namespace android
