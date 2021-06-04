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

#define LOG_TAG "Camera3-DepthPhotoProcessor"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0
//

#include "DepthPhotoProcessor.h"

#include <dynamic_depth/camera.h>
#include <dynamic_depth/cameras.h>
#include <dynamic_depth/container.h>
#include <dynamic_depth/device.h>
#include <dynamic_depth/dimension.h>
#include <dynamic_depth/dynamic_depth.h>
#include <dynamic_depth/point.h>
#include <dynamic_depth/pose.h>
#include <dynamic_depth/profile.h>
#include <dynamic_depth/profiles.h>
#include <jpeglib.h>
#include <libexif/exif-data.h>
#include <libexif/exif-system.h>
#include <math.h>
#include <sstream>
#include <utils/Errors.h>
#include <utils/ExifUtils.h>
#include <utils/Log.h>
#include <xmpmeta/xmp_data.h>
#include <xmpmeta/xmp_writer.h>

using dynamic_depth::Camera;
using dynamic_depth::Cameras;
using dynamic_depth::CameraParams;
using dynamic_depth::Container;
using dynamic_depth::DepthFormat;
using dynamic_depth::DepthMap;
using dynamic_depth::DepthMapParams;
using dynamic_depth::DepthUnits;
using dynamic_depth::Device;
using dynamic_depth::DeviceParams;
using dynamic_depth::Dimension;
using dynamic_depth::Image;
using dynamic_depth::ImagingModel;
using dynamic_depth::ImagingModelParams;
using dynamic_depth::Item;
using dynamic_depth::Pose;
using dynamic_depth::Profile;
using dynamic_depth::Profiles;

template<>
struct std::default_delete<jpeg_compress_struct> {
    inline void operator()(jpeg_compress_struct* cinfo) const {
        jpeg_destroy_compress(cinfo);
    }
};

namespace android {
namespace camera3 {

// Depth samples with low confidence can skew the
// near/far values and impact the range inverse coding.
static const float CONFIDENCE_THRESHOLD = .15f;

ExifOrientation getExifOrientation(const unsigned char *jpegBuffer, size_t jpegBufferSize) {
    if ((jpegBuffer == nullptr) || (jpegBufferSize == 0)) {
        return ExifOrientation::ORIENTATION_UNDEFINED;
    }

    auto exifData = exif_data_new();
    exif_data_load_data(exifData, jpegBuffer, jpegBufferSize);
    ExifEntry *orientation = exif_content_get_entry(exifData->ifd[EXIF_IFD_0],
            EXIF_TAG_ORIENTATION);
    if ((orientation == nullptr) || (orientation->size != sizeof(ExifShort))) {
        ALOGV("%s: Orientation EXIF entry invalid!", __FUNCTION__);
        exif_data_unref(exifData);
        return ExifOrientation::ORIENTATION_0_DEGREES;
    }

    auto orientationValue = exif_get_short(orientation->data, exif_data_get_byte_order(exifData));
    ExifOrientation ret;
    switch (orientationValue) {
        case ExifOrientation::ORIENTATION_0_DEGREES:
        case ExifOrientation::ORIENTATION_90_DEGREES:
        case ExifOrientation::ORIENTATION_180_DEGREES:
        case ExifOrientation::ORIENTATION_270_DEGREES:
            ret = static_cast<ExifOrientation> (orientationValue);
            break;
        default:
            ALOGE("%s: Unexpected EXIF orientation value: %d, defaulting to 0 degrees",
                    __FUNCTION__, orientationValue);
            ret = ExifOrientation::ORIENTATION_0_DEGREES;
    }

    exif_data_unref(exifData);

    return ret;
}

status_t encodeGrayscaleJpeg(size_t width, size_t height, uint8_t *in, void *out,
        const size_t maxOutSize, uint8_t jpegQuality, ExifOrientation exifOrientation,
        size_t &actualSize) {
    status_t ret;
    // libjpeg is a C library so we use C-style "inheritance" by
    // putting libjpeg's jpeg_destination_mgr first in our custom
    // struct. This allows us to cast jpeg_destination_mgr* to
    // CustomJpegDestMgr* when we get it passed to us in a callback.
    struct CustomJpegDestMgr : public jpeg_destination_mgr {
        JOCTET *mBuffer;
        size_t mBufferSize;
        size_t mEncodedSize;
        bool mSuccess;
    } dmgr;

    std::unique_ptr<jpeg_compress_struct> cinfo = std::make_unique<jpeg_compress_struct>();
    jpeg_error_mgr jerr;

    // Initialize error handling with standard callbacks, but
    // then override output_message (to print to ALOG) and
    // error_exit to set a flag and print a message instead
    // of killing the whole process.
    cinfo->err = jpeg_std_error(&jerr);

    cinfo->err->output_message = [](j_common_ptr cinfo) {
        char buffer[JMSG_LENGTH_MAX];

        /* Create the message */
        (*cinfo->err->format_message)(cinfo, buffer);
        ALOGE("libjpeg error: %s", buffer);
    };

    cinfo->err->error_exit = [](j_common_ptr cinfo) {
        (*cinfo->err->output_message)(cinfo);
        if(cinfo->client_data) {
            auto & dmgr = *static_cast<CustomJpegDestMgr*>(cinfo->client_data);
            dmgr.mSuccess = false;
        }
    };

    // Now that we initialized some callbacks, let's create our compressor
    jpeg_create_compress(cinfo.get());
    dmgr.mBuffer = static_cast<JOCTET*>(out);
    dmgr.mBufferSize = maxOutSize;
    dmgr.mEncodedSize = 0;
    dmgr.mSuccess = true;
    cinfo->client_data = static_cast<void*>(&dmgr);

    // These lambdas become C-style function pointers and as per C++11 spec
    // may not capture anything.
    dmgr.init_destination = [](j_compress_ptr cinfo) {
        auto & dmgr = static_cast<CustomJpegDestMgr&>(*cinfo->dest);
        dmgr.next_output_byte = dmgr.mBuffer;
        dmgr.free_in_buffer = dmgr.mBufferSize;
        ALOGV("%s:%d jpeg start: %p [%zu]",
              __FUNCTION__, __LINE__, dmgr.mBuffer, dmgr.mBufferSize);
    };

    dmgr.empty_output_buffer = [](j_compress_ptr cinfo __unused) {
        ALOGV("%s:%d Out of buffer", __FUNCTION__, __LINE__);
        return 0;
    };

    dmgr.term_destination = [](j_compress_ptr cinfo) {
        auto & dmgr = static_cast<CustomJpegDestMgr&>(*cinfo->dest);
        dmgr.mEncodedSize = dmgr.mBufferSize - dmgr.free_in_buffer;
        ALOGV("%s:%d Done with jpeg: %zu", __FUNCTION__, __LINE__, dmgr.mEncodedSize);
    };
    cinfo->dest = static_cast<struct jpeg_destination_mgr*>(&dmgr);
    cinfo->image_width = width;
    cinfo->image_height = height;
    cinfo->input_components = 1;
    cinfo->in_color_space = JCS_GRAYSCALE;

    // Initialize defaults and then override what we want
    jpeg_set_defaults(cinfo.get());

    jpeg_set_quality(cinfo.get(), jpegQuality, 1);
    jpeg_set_colorspace(cinfo.get(), JCS_GRAYSCALE);
    cinfo->raw_data_in = 0;
    cinfo->dct_method = JDCT_IFAST;

    cinfo->comp_info[0].h_samp_factor = 1;
    cinfo->comp_info[1].h_samp_factor = 1;
    cinfo->comp_info[2].h_samp_factor = 1;
    cinfo->comp_info[0].v_samp_factor = 1;
    cinfo->comp_info[1].v_samp_factor = 1;
    cinfo->comp_info[2].v_samp_factor = 1;

    jpeg_start_compress(cinfo.get(), TRUE);

    if (exifOrientation != ExifOrientation::ORIENTATION_UNDEFINED) {
        std::unique_ptr<ExifUtils> utils(ExifUtils::create());
        utils->initializeEmpty();
        utils->setImageWidth(width);
        utils->setImageHeight(height);
        utils->setOrientationValue(exifOrientation);

        if (utils->generateApp1()) {
            const uint8_t* exifBuffer = utils->getApp1Buffer();
            size_t exifBufferSize = utils->getApp1Length();
            jpeg_write_marker(cinfo.get(), JPEG_APP0 + 1, static_cast<const JOCTET*>(exifBuffer),
                    exifBufferSize);
        } else {
            ALOGE("%s: Unable to generate App1 buffer", __FUNCTION__);
        }
    }

    for (size_t i = 0; i < cinfo->image_height; i++) {
        auto currentRow  = static_cast<JSAMPROW>(in + i*width);
        jpeg_write_scanlines(cinfo.get(), &currentRow, /*num_lines*/1);
    }

    jpeg_finish_compress(cinfo.get());

    actualSize = dmgr.mEncodedSize;
    if (dmgr.mSuccess) {
        ret = NO_ERROR;
    } else {
        ret = UNKNOWN_ERROR;
    }

    return ret;
}

inline void unpackDepth16(uint16_t value, std::vector<float> *points /*out*/,
        std::vector<float> *confidence /*out*/, float *near /*out*/, float *far /*out*/) {
    // Android densely packed depth map. The units for the range are in
    // millimeters and need to be scaled to meters.
    // The confidence value is encoded in the 3 most significant bits.
    // The confidence data needs to be additionally normalized with
    // values 1.0f, 0.0f representing maximum and minimum confidence
    // respectively.
    auto point = static_cast<float>(value & 0x1FFF) / 1000.f;
    points->push_back(point);

    auto conf = (value >> 13) & 0x7;
    float normConfidence = (conf == 0) ? 1.f : (static_cast<float>(conf) - 1) / 7.f;
    confidence->push_back(normConfidence);
    if (normConfidence < CONFIDENCE_THRESHOLD) {
        return;
    }

    if (*near > point) {
        *near = point;
    }
    if (*far < point) {
        *far = point;
    }
}

// Trivial case, read forward from top,left corner.
void rotate0AndUnpack(DepthPhotoInputFrame inputFrame, std::vector<float> *points /*out*/,
        std::vector<float> *confidence /*out*/, float *near /*out*/, float *far /*out*/) {
    for (size_t i = 0; i < inputFrame.mDepthMapHeight; i++) {
        for (size_t j = 0; j < inputFrame.mDepthMapWidth; j++) {
            unpackDepth16(inputFrame.mDepthMapBuffer[i*inputFrame.mDepthMapStride + j], points,
                    confidence, near, far);
        }
    }
}

// 90 degrees CW rotation can be applied by starting to read from bottom, left corner
// transposing rows and columns.
void rotate90AndUnpack(DepthPhotoInputFrame inputFrame, std::vector<float> *points /*out*/,
        std::vector<float> *confidence /*out*/, float *near /*out*/, float *far /*out*/) {
    for (size_t i = 0; i < inputFrame.mDepthMapWidth; i++) {
        for (ssize_t j = inputFrame.mDepthMapHeight-1; j >= 0; j--) {
            unpackDepth16(inputFrame.mDepthMapBuffer[j*inputFrame.mDepthMapStride + i], points,
                    confidence, near, far);
        }
    }
}

// 180 CW degrees rotation can be applied by starting to read backwards from bottom, right corner.
void rotate180AndUnpack(DepthPhotoInputFrame inputFrame, std::vector<float> *points /*out*/,
        std::vector<float> *confidence /*out*/, float *near /*out*/, float *far /*out*/) {
    for (ssize_t i = inputFrame.mDepthMapHeight-1; i >= 0; i--) {
        for (ssize_t j = inputFrame.mDepthMapWidth-1; j >= 0; j--) {
            unpackDepth16(inputFrame.mDepthMapBuffer[i*inputFrame.mDepthMapStride + j], points,
                    confidence, near, far);
        }
    }
}

// 270 degrees CW rotation can be applied by starting to read from top, right corner
// transposing rows and columns.
void rotate270AndUnpack(DepthPhotoInputFrame inputFrame, std::vector<float> *points /*out*/,
        std::vector<float> *confidence /*out*/, float *near /*out*/, float *far /*out*/) {
    for (ssize_t i = inputFrame.mDepthMapWidth-1; i >= 0; i--) {
        for (size_t j = 0; j < inputFrame.mDepthMapHeight; j++) {
            unpackDepth16(inputFrame.mDepthMapBuffer[j*inputFrame.mDepthMapStride + i], points,
                    confidence, near, far);
        }
    }
}

bool rotateAndUnpack(DepthPhotoInputFrame inputFrame, std::vector<float> *points /*out*/,
        std::vector<float> *confidence /*out*/, float *near /*out*/, float *far /*out*/) {
    switch (inputFrame.mOrientation) {
        case DepthPhotoOrientation::DEPTH_ORIENTATION_0_DEGREES:
            rotate0AndUnpack(inputFrame, points, confidence, near, far);
            return false;
        case DepthPhotoOrientation::DEPTH_ORIENTATION_90_DEGREES:
            rotate90AndUnpack(inputFrame, points, confidence, near, far);
            return true;
        case DepthPhotoOrientation::DEPTH_ORIENTATION_180_DEGREES:
            rotate180AndUnpack(inputFrame, points, confidence, near, far);
            return false;
        case DepthPhotoOrientation::DEPTH_ORIENTATION_270_DEGREES:
            rotate270AndUnpack(inputFrame, points, confidence, near, far);
            return true;
        default:
            ALOGE("%s: Unsupported depth photo rotation: %d, default to 0", __FUNCTION__,
                    inputFrame.mOrientation);
            rotate0AndUnpack(inputFrame, points, confidence, near, far);
    }

    return false;
}

std::unique_ptr<dynamic_depth::DepthMap> processDepthMapFrame(DepthPhotoInputFrame inputFrame,
        ExifOrientation exifOrientation, std::vector<std::unique_ptr<Item>> *items /*out*/,
        bool *switchDimensions /*out*/) {
    if ((items == nullptr) || (switchDimensions == nullptr)) {
        return nullptr;
    }

    std::vector<float> points, confidence;

    size_t pointCount = inputFrame.mDepthMapWidth * inputFrame.mDepthMapHeight;
    points.reserve(pointCount);
    confidence.reserve(pointCount);
    float near = UINT16_MAX;
    float far = .0f;
    *switchDimensions = false;
    // Physical rotation of depth and confidence maps may be needed in case
    // the EXIF orientation is set to 0 degrees and the depth photo orientation
    // (source color image) has some different value.
    if (exifOrientation == ExifOrientation::ORIENTATION_0_DEGREES) {
        *switchDimensions = rotateAndUnpack(inputFrame, &points, &confidence, &near, &far);
    } else {
        rotate0AndUnpack(inputFrame, &points, &confidence, &near, &far);
    }

    size_t width = inputFrame.mDepthMapWidth;
    size_t height = inputFrame.mDepthMapHeight;
    if (*switchDimensions) {
        width = inputFrame.mDepthMapHeight;
        height = inputFrame.mDepthMapWidth;
    }

    if (near == far) {
        ALOGE("%s: Near and far range values must not match!", __FUNCTION__);
        return nullptr;
    }

    std::vector<uint8_t> pointsQuantized, confidenceQuantized;
    pointsQuantized.reserve(pointCount); confidenceQuantized.reserve(pointCount);
    auto pointIt = points.begin();
    auto confidenceIt = confidence.begin();
    while ((pointIt != points.end()) && (confidenceIt != confidence.end())) {
        auto point = *pointIt;
        if ((*confidenceIt) < CONFIDENCE_THRESHOLD) {
            point = std::clamp(point, near, far);
        }
        pointsQuantized.push_back(floorf(((far * (point - near)) /
                (point * (far - near))) * 255.0f));
        confidenceQuantized.push_back(floorf(*confidenceIt * 255.0f));
        confidenceIt++; pointIt++;
    }

    DepthMapParams depthParams(DepthFormat::kRangeInverse, near, far, DepthUnits::kMeters,
            "android/depthmap");
    depthParams.confidence_uri = "android/confidencemap";
    depthParams.mime = "image/jpeg";
    depthParams.depth_image_data.resize(inputFrame.mMaxJpegSize);
    depthParams.confidence_data.resize(inputFrame.mMaxJpegSize);
    size_t actualJpegSize;
    auto ret = encodeGrayscaleJpeg(width, height, pointsQuantized.data(),
            depthParams.depth_image_data.data(), inputFrame.mMaxJpegSize,
            inputFrame.mJpegQuality, exifOrientation, actualJpegSize);
    if (ret != NO_ERROR) {
        ALOGE("%s: Depth map compression failed!", __FUNCTION__);
        return nullptr;
    }
    depthParams.depth_image_data.resize(actualJpegSize);

    ret = encodeGrayscaleJpeg(width, height, confidenceQuantized.data(),
            depthParams.confidence_data.data(), inputFrame.mMaxJpegSize,
            inputFrame.mJpegQuality, exifOrientation, actualJpegSize);
    if (ret != NO_ERROR) {
        ALOGE("%s: Confidence map compression failed!", __FUNCTION__);
        return nullptr;
    }
    depthParams.confidence_data.resize(actualJpegSize);

    return DepthMap::FromData(depthParams, items);
}

extern "C" int processDepthPhotoFrame(DepthPhotoInputFrame inputFrame, size_t depthPhotoBufferSize,
        void* depthPhotoBuffer /*out*/, size_t* depthPhotoActualSize /*out*/) {
    if ((inputFrame.mMainJpegBuffer == nullptr) || (inputFrame.mDepthMapBuffer == nullptr) ||
            (depthPhotoBuffer == nullptr) || (depthPhotoActualSize == nullptr)) {
        return BAD_VALUE;
    }

    std::vector<std::unique_ptr<Item>> items;
    std::vector<std::unique_ptr<Camera>> cameraList;
    auto image = Image::FromDataForPrimaryImage("image/jpeg", &items);
    std::unique_ptr<CameraParams> cameraParams(new CameraParams(std::move(image)));
    if (cameraParams == nullptr) {
        ALOGE("%s: Failed to initialize camera parameters", __FUNCTION__);
        return BAD_VALUE;
    }

    ExifOrientation exifOrientation = getExifOrientation(
            reinterpret_cast<const unsigned char*> (inputFrame.mMainJpegBuffer),
            inputFrame.mMainJpegSize);
    bool switchDimensions;
    cameraParams->depth_map = processDepthMapFrame(inputFrame, exifOrientation, &items,
            &switchDimensions);
    if (cameraParams->depth_map == nullptr) {
        ALOGE("%s: Depth map processing failed!", __FUNCTION__);
        return BAD_VALUE;
    }

    // It is not possible to generate an imaging model without intrinsic calibration.
    if (inputFrame.mIsIntrinsicCalibrationValid) {
        // The camera intrinsic calibration layout is as follows:
        // [focalLengthX, focalLengthY, opticalCenterX, opticalCenterY, skew]
        const dynamic_depth::Point<double> focalLength(inputFrame.mIntrinsicCalibration[0],
                inputFrame.mIntrinsicCalibration[1]);
        size_t width = inputFrame.mMainJpegWidth;
        size_t height = inputFrame.mMainJpegHeight;
        if (switchDimensions) {
            width = inputFrame.mMainJpegHeight;
            height = inputFrame.mMainJpegWidth;
        }
        const Dimension imageSize(width, height);
        ImagingModelParams imagingParams(focalLength, imageSize);
        imagingParams.principal_point.x = inputFrame.mIntrinsicCalibration[2];
        imagingParams.principal_point.y = inputFrame.mIntrinsicCalibration[3];
        imagingParams.skew = inputFrame.mIntrinsicCalibration[4];

        // The camera lens distortion contains the following lens correction coefficients.
        // [kappa_1, kappa_2, kappa_3 kappa_4, kappa_5]
        if (inputFrame.mIsLensDistortionValid) {
            // According to specification the lens distortion coefficients should be ordered
            // as [1, kappa_4, kappa_1, kappa_5, kappa_2, 0, kappa_3, 0]
            float distortionData[] = {1.f, inputFrame.mLensDistortion[3],
                    inputFrame.mLensDistortion[0], inputFrame.mLensDistortion[4],
                    inputFrame.mLensDistortion[1], 0.f, inputFrame.mLensDistortion[2], 0.f};
            auto distortionDataLength = sizeof(distortionData) / sizeof(distortionData[0]);
            imagingParams.distortion.reserve(distortionDataLength);
            imagingParams.distortion.insert(imagingParams.distortion.end(), distortionData,
                    distortionData + distortionDataLength);
        }

        cameraParams->imaging_model = ImagingModel::FromData(imagingParams);
    }

    if (inputFrame.mIsLogical) {
        cameraParams->trait = dynamic_depth::CameraTrait::LOGICAL;
    } else {
        cameraParams->trait = dynamic_depth::CameraTrait::PHYSICAL;
    }

    cameraList.emplace_back(Camera::FromData(std::move(cameraParams)));

    auto deviceParams = std::make_unique<DeviceParams> (Cameras::FromCameraArray(&cameraList));
    deviceParams->container = Container::FromItems(&items);
    std::vector<std::unique_ptr<Profile>> profileList;
    profileList.emplace_back(Profile::FromData("DepthPhoto", {0}));
    deviceParams->profiles = Profiles::FromProfileArray(&profileList);
    std::unique_ptr<Device> device = Device::FromData(std::move(deviceParams));
    if (device == nullptr) {
        ALOGE("%s: Failed to initialize camera device", __FUNCTION__);
        return BAD_VALUE;
    }

    std::istringstream inputJpegStream(
            std::string(inputFrame.mMainJpegBuffer, inputFrame.mMainJpegSize));
    std::ostringstream outputJpegStream;
    if (!WriteImageAndMetadataAndContainer(&inputJpegStream, device.get(), &outputJpegStream)) {
        ALOGE("%s: Failed writing depth output", __FUNCTION__);
        return BAD_VALUE;
    }

    *depthPhotoActualSize = static_cast<size_t> (outputJpegStream.tellp());
    if (*depthPhotoActualSize > depthPhotoBufferSize) {
        ALOGE("%s: Depth photo output buffer not sufficient, needed %zu actual %zu", __FUNCTION__,
                *depthPhotoActualSize, depthPhotoBufferSize);
        return NO_MEMORY;
    }

    memcpy(depthPhotoBuffer, outputJpegStream.str().c_str(), *depthPhotoActualSize);

    return 0;
}

}; // namespace camera3
}; // namespace android
