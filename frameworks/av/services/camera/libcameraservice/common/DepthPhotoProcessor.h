/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA3_DEPTH_PROCESSOR_H
#define ANDROID_SERVERS_CAMERA_CAMERA3_DEPTH_PROCESSOR_H

#include <stddef.h>
#include <stdint.h>

namespace android {
namespace camera3 {

enum DepthPhotoOrientation {
    DEPTH_ORIENTATION_0_DEGREES   = 0,
    DEPTH_ORIENTATION_90_DEGREES  = 90,
    DEPTH_ORIENTATION_180_DEGREES = 180,
    DEPTH_ORIENTATION_270_DEGREES = 270,
};

struct DepthPhotoInputFrame {
    const char*           mMainJpegBuffer;
    size_t                mMainJpegSize;
    size_t                mMainJpegWidth, mMainJpegHeight;
    uint16_t*             mDepthMapBuffer;
    size_t                mDepthMapWidth, mDepthMapHeight, mDepthMapStride;
    size_t                mMaxJpegSize;
    uint8_t               mJpegQuality;
    uint8_t               mIsLogical;
    float                 mIntrinsicCalibration[5];
    uint8_t               mIsIntrinsicCalibrationValid;
    float                 mLensDistortion[5];
    uint8_t               mIsLensDistortionValid;
    DepthPhotoOrientation mOrientation;

    DepthPhotoInputFrame() :
            mMainJpegBuffer(nullptr),
            mMainJpegSize(0),
            mMainJpegWidth(0),
            mMainJpegHeight(0),
            mDepthMapBuffer(nullptr),
            mDepthMapWidth(0),
            mDepthMapHeight(0),
            mDepthMapStride(0),
            mMaxJpegSize(0),
            mJpegQuality(100),
            mIsLogical(0),
            mIntrinsicCalibration{0.f},
            mIsIntrinsicCalibrationValid(0),
            mLensDistortion{0.f},
            mIsLensDistortionValid(0),
            mOrientation(DepthPhotoOrientation::DEPTH_ORIENTATION_0_DEGREES) {}
};

static const char *kDepthPhotoLibrary = "libdepthphoto.so";
static const char *kDepthPhotoProcessFunction = "processDepthPhotoFrame";
typedef int (*process_depth_photo_frame) (DepthPhotoInputFrame /*inputFrame*/,
        size_t /*depthPhotoBufferSize*/, void* /*depthPhotoBuffer out*/,
        size_t* /*depthPhotoActualSize out*/);

}; // namespace camera3
}; // namespace android

#endif
