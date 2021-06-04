/*
 * Copyright (C) 2015-2018 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_CAMERA2_OUTPUTCONFIGURATION_H
#define ANDROID_HARDWARE_CAMERA2_OUTPUTCONFIGURATION_H

#include <gui/IGraphicBufferProducer.h>
#include <binder/Parcelable.h>

namespace android {

class Surface;

namespace hardware {
namespace camera2 {
namespace params {

class OutputConfiguration : public android::Parcelable {
public:

    static const int INVALID_ROTATION;
    static const int INVALID_SET_ID;
    enum SurfaceType{
        SURFACE_TYPE_UNKNOWN = -1,
        SURFACE_TYPE_SURFACE_VIEW = 0,
        SURFACE_TYPE_SURFACE_TEXTURE = 1
    };
    const std::vector<sp<IGraphicBufferProducer>>& getGraphicBufferProducers() const;
    int                        getRotation() const;
    int                        getSurfaceSetID() const;
    int                        getSurfaceType() const;
    int                        getWidth() const;
    int                        getHeight() const;
    bool                       isDeferred() const;
    bool                       isShared() const;
    String16                   getPhysicalCameraId() const;
    /**
     * Keep impl up-to-date with OutputConfiguration.java in frameworks/base
     */
    virtual status_t           writeToParcel(android::Parcel* parcel) const override;

    virtual status_t           readFromParcel(const android::Parcel* parcel) override;

    // getGraphicBufferProducer will be NULL
    // getRotation will be INVALID_ROTATION
    // getSurfaceSetID will be INVALID_SET_ID
    OutputConfiguration();

    // getGraphicBufferProducer will be NULL if error occurred
    // getRotation will be INVALID_ROTATION if error occurred
    // getSurfaceSetID will be INVALID_SET_ID if error occurred
    OutputConfiguration(const android::Parcel& parcel);

    OutputConfiguration(sp<IGraphicBufferProducer>& gbp, int rotation,
            const String16& physicalCameraId,
            int surfaceSetID = INVALID_SET_ID, bool isShared = false);

    OutputConfiguration(const std::vector<sp<IGraphicBufferProducer>>& gbps,
                        int rotation, const String16& physicalCameraId,
                        int surfaceSetID = INVALID_SET_ID,
                        int surfaceType = OutputConfiguration::SURFACE_TYPE_UNKNOWN, int width = 0,
                        int height = 0, bool isShared = false);

    bool operator == (const OutputConfiguration& other) const {
        return ( mRotation == other.mRotation &&
                mSurfaceSetID == other.mSurfaceSetID &&
                mSurfaceType == other.mSurfaceType &&
                mWidth == other.mWidth &&
                mHeight == other.mHeight &&
                mIsDeferred == other.mIsDeferred &&
                mIsShared == other.mIsShared &&
                gbpsEqual(other) &&
                mPhysicalCameraId == other.mPhysicalCameraId );
    }
    bool operator != (const OutputConfiguration& other) const {
        return !(*this == other);
    }
    bool operator < (const OutputConfiguration& other) const {
        if (*this == other) return false;
        if (mSurfaceSetID != other.mSurfaceSetID) {
            return mSurfaceSetID < other.mSurfaceSetID;
        }
        if (mSurfaceType != other.mSurfaceType) {
            return mSurfaceType < other.mSurfaceType;
        }
        if (mWidth != other.mWidth) {
            return mWidth < other.mWidth;
        }
        if (mHeight != other.mHeight) {
            return mHeight < other.mHeight;
        }
        if (mRotation != other.mRotation) {
            return mRotation < other.mRotation;
        }
        if (mIsDeferred != other.mIsDeferred) {
            return mIsDeferred < other.mIsDeferred;
        }
        if (mIsShared != other.mIsShared) {
            return mIsShared < other.mIsShared;
        }
        if (mPhysicalCameraId != other.mPhysicalCameraId) {
            return mPhysicalCameraId < other.mPhysicalCameraId;
        }
        return gbpsLessThan(other);
    }
    bool operator > (const OutputConfiguration& other) const {
        return (*this != other && !(*this < other));
    }

    bool gbpsEqual(const OutputConfiguration& other) const;
    bool gbpsLessThan(const OutputConfiguration& other) const;
    void addGraphicProducer(sp<IGraphicBufferProducer> gbp) {mGbps.push_back(gbp);}
private:
    std::vector<sp<IGraphicBufferProducer>> mGbps;
    int                        mRotation;
    int                        mSurfaceSetID;
    int                        mSurfaceType;
    int                        mWidth;
    int                        mHeight;
    bool                       mIsDeferred;
    bool                       mIsShared;
    String16                   mPhysicalCameraId;
};
} // namespace params
} // namespace camera2
} // namespace hardware


using hardware::camera2::params::OutputConfiguration;

}; // namespace android

#endif
