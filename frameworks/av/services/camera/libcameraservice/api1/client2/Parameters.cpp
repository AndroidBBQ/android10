/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "Camera2-Parameters"
#define ATRACE_TAG ATRACE_TAG_CAMERA
// #define LOG_NDEBUG 0

#include <utils/Log.h>
#include <utils/Trace.h>
#include <utils/Vector.h>
#include <utils/SortedVector.h>

#include <math.h>
#include <stdlib.h>
#include <cutils/properties.h>

#include "Parameters.h"
#include "system/camera.h"
#include "hardware/camera_common.h"
#include <android/hardware/ICamera.h>
#include <media/MediaProfiles.h>
#include <media/mediarecorder.h>

namespace android {
namespace camera2 {

Parameters::Parameters(int cameraId,
        int cameraFacing) :
        cameraId(cameraId),
        cameraFacing(cameraFacing),
        info(NULL),
        mDefaultSceneMode(ANDROID_CONTROL_SCENE_MODE_DISABLED) {
}

Parameters::~Parameters() {
}

status_t Parameters::initialize(CameraDeviceBase *device, int deviceVersion) {
    status_t res;
    if (device == nullptr) {
        ALOGE("%s: device is null!", __FUNCTION__);
        return BAD_VALUE;
    }

    const CameraMetadata& info = device->info();
    if (info.entryCount() == 0) {
        ALOGE("%s: No static information provided!", __FUNCTION__);
        return BAD_VALUE;
    }
    Parameters::info = &info;
    mDeviceVersion = deviceVersion;

    res = buildFastInfo(device);
    if (res != OK) return res;

    res = buildQuirks();
    if (res != OK) return res;

    Size maxPreviewSize = { MAX_PREVIEW_WIDTH, MAX_PREVIEW_HEIGHT };
    // Treat the H.264 max size as the max supported video size.
    MediaProfiles *videoEncoderProfiles = MediaProfiles::getInstance();
    Vector<video_encoder> encoders = videoEncoderProfiles->getVideoEncoders();
    int32_t maxVideoWidth = 0;
    int32_t maxVideoHeight = 0;
    for (size_t i = 0; i < encoders.size(); i++) {
        int width = videoEncoderProfiles->getVideoEncoderParamByName(
                "enc.vid.width.max", encoders[i]);
        int height = videoEncoderProfiles->getVideoEncoderParamByName(
                "enc.vid.height.max", encoders[i]);
        // Treat width/height separately here to handle the case where different
        // profile might report max size of different aspect ratio
        if (width > maxVideoWidth) {
            maxVideoWidth = width;
        }
        if (height > maxVideoHeight) {
            maxVideoHeight = height;
        }
    }
    // This is just an upper bound and may not be an actually valid video size
    Size videoSizeUpperBound = {maxVideoWidth, maxVideoHeight};

    if (fastInfo.supportsPreferredConfigs) {
        maxPreviewSize = getMaxSize(getPreferredPreviewSizes());
        videoSizeUpperBound = getMaxSize(getPreferredVideoSizes());
    }

    res = getFilteredSizes(maxPreviewSize, &availablePreviewSizes);
    if (res != OK) return res;
    res = getFilteredSizes(videoSizeUpperBound, &availableVideoSizes);
    if (res != OK) return res;

    // Select initial preview and video size that's under the initial bound and
    // on the list of both preview and recording sizes
    previewWidth = 0;
    previewHeight = 0;
    for (size_t i = 0 ; i < availablePreviewSizes.size(); i++) {
        int newWidth = availablePreviewSizes[i].width;
        int newHeight = availablePreviewSizes[i].height;
        if (newWidth >= previewWidth && newHeight >= previewHeight &&
                newWidth <= MAX_INITIAL_PREVIEW_WIDTH &&
                newHeight <= MAX_INITIAL_PREVIEW_HEIGHT) {
            for (size_t j = 0; j < availableVideoSizes.size(); j++) {
                if (availableVideoSizes[j].width == newWidth &&
                        availableVideoSizes[j].height == newHeight) {
                    previewWidth = newWidth;
                    previewHeight = newHeight;
                }
            }
        }
    }
    if (previewWidth == 0) {
        ALOGE("%s: No initial preview size can be found!", __FUNCTION__);
        return BAD_VALUE;
    }
    videoWidth = previewWidth;
    videoHeight = previewHeight;

    params.setPreviewSize(previewWidth, previewHeight);
    params.setVideoSize(videoWidth, videoHeight);
    params.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO,
            String8::format("%dx%d",
                    previewWidth, previewHeight));
    {
        String8 supportedPreviewSizes;
        for (size_t i = 0; i < availablePreviewSizes.size(); i++) {
            if (i != 0) supportedPreviewSizes += ",";
            supportedPreviewSizes += String8::format("%dx%d",
                    availablePreviewSizes[i].width,
                    availablePreviewSizes[i].height);
        }
        ALOGV("Supported preview sizes are: %s", supportedPreviewSizes.string());
        params.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES,
                supportedPreviewSizes);

        String8 supportedVideoSizes;
        for (size_t i = 0; i < availableVideoSizes.size(); i++) {
            if (i != 0) supportedVideoSizes += ",";
            supportedVideoSizes += String8::format("%dx%d",
                    availableVideoSizes[i].width,
                    availableVideoSizes[i].height);
        }
        ALOGV("Supported video sizes are: %s", supportedVideoSizes.string());
        params.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES,
                supportedVideoSizes);
    }

    camera_metadata_ro_entry_t availableFpsRanges =
        staticInfo(ANDROID_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES, 2);
    if (!availableFpsRanges.count) return NO_INIT;

    previewFormat = HAL_PIXEL_FORMAT_YCrCb_420_SP;
    params.set(CameraParameters::KEY_PREVIEW_FORMAT,
            formatEnumToString(previewFormat)); // NV21

    previewTransform = degToTransform(0,
            cameraFacing == CAMERA_FACING_FRONT);

    {
        String8 supportedPreviewFormats;
        SortedVector<int32_t> outputFormats = getAvailableOutputFormats();
        bool addComma = false;
        for (size_t i=0; i < outputFormats.size(); i++) {
            if (addComma) supportedPreviewFormats += ",";
            addComma = true;
            switch (outputFormats[i]) {
            case HAL_PIXEL_FORMAT_YCbCr_422_SP:
                supportedPreviewFormats +=
                    CameraParameters::PIXEL_FORMAT_YUV422SP;
                break;
            case HAL_PIXEL_FORMAT_YCrCb_420_SP:
                supportedPreviewFormats +=
                    CameraParameters::PIXEL_FORMAT_YUV420SP;
                break;
            case HAL_PIXEL_FORMAT_YCbCr_422_I:
                supportedPreviewFormats +=
                    CameraParameters::PIXEL_FORMAT_YUV422I;
                break;
            case HAL_PIXEL_FORMAT_YV12:
                supportedPreviewFormats +=
                    CameraParameters::PIXEL_FORMAT_YUV420P;
                break;
            case HAL_PIXEL_FORMAT_RGB_565:
                supportedPreviewFormats +=
                    CameraParameters::PIXEL_FORMAT_RGB565;
                break;
            case HAL_PIXEL_FORMAT_RGBA_8888:
                supportedPreviewFormats +=
                    CameraParameters::PIXEL_FORMAT_RGBA8888;
                break;
            case HAL_PIXEL_FORMAT_YCbCr_420_888:
                // Flexible YUV allows both YV12 and NV21
                supportedPreviewFormats +=
                    CameraParameters::PIXEL_FORMAT_YUV420P;
                supportedPreviewFormats += ",";
                supportedPreviewFormats +=
                    CameraParameters::PIXEL_FORMAT_YUV420SP;
                break;
            // Not advertizing JPEG, RAW16, etc, for preview formats
            case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
            case HAL_PIXEL_FORMAT_RAW16:
            case HAL_PIXEL_FORMAT_BLOB:
                addComma = false;
                break;

            default:
                ALOGW("%s: Camera %d: Unknown preview format: %x",
                        __FUNCTION__, cameraId, outputFormats[i]);
                addComma = false;
                break;
            }
        }
        params.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS,
                supportedPreviewFormats);
    }

    previewFpsRange[0] = fastInfo.bestStillCaptureFpsRange[0];
    previewFpsRange[1] = fastInfo.bestStillCaptureFpsRange[1];

    // PREVIEW_FRAME_RATE / SUPPORTED_PREVIEW_FRAME_RATES are deprecated, but
    // still have to do something sane for them

    // NOTE: Not scaled like FPS range values are.
    int previewFps = fpsFromRange(previewFpsRange[0], previewFpsRange[1]);
    params.set(CameraParameters::KEY_PREVIEW_FRAME_RATE,
            previewFps);

    // PREVIEW_FPS_RANGE
    // -- Order matters. Set range after single value to so that a roundtrip
    //    of setParameters(getParameters()) would keep the FPS range in higher
    //    order.
    params.set(CameraParameters::KEY_PREVIEW_FPS_RANGE,
            String8::format("%d,%d",
                    previewFpsRange[0] * kFpsToApiScale,
                    previewFpsRange[1] * kFpsToApiScale));

    {
        String8 supportedPreviewFpsRange;
        for (size_t i=0; i < availableFpsRanges.count; i += 2) {
            if (!isFpsSupported(availablePreviewSizes,
                HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, availableFpsRanges.data.i32[i+1])) {
                continue;
            }
            if (supportedPreviewFpsRange.length() > 0) {
                supportedPreviewFpsRange += ",";
            }
            supportedPreviewFpsRange += String8::format("(%d,%d)",
                    availableFpsRanges.data.i32[i] * kFpsToApiScale,
                    availableFpsRanges.data.i32[i+1] * kFpsToApiScale);
        }
        params.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE,
                supportedPreviewFpsRange);
    }

    {
        SortedVector<int32_t> sortedPreviewFrameRates;

        String8 supportedPreviewFrameRates;
        for (size_t i=0; i < availableFpsRanges.count; i += 2) {
            // from the [min, max] fps range use the max value
            int fps = fpsFromRange(availableFpsRanges.data.i32[i],
                                   availableFpsRanges.data.i32[i+1]);
            if (!isFpsSupported(availablePreviewSizes,
                    HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, fps)) {
                continue;
            }
            // de-dupe frame rates
            if (sortedPreviewFrameRates.indexOf(fps) == NAME_NOT_FOUND) {
                sortedPreviewFrameRates.add(fps);
            }
            else {
                continue;
            }

            if (sortedPreviewFrameRates.size() > 1) {
                supportedPreviewFrameRates += ",";
            }

            supportedPreviewFrameRates += String8::format("%d",
                    fps);

            ALOGV("%s: Supported preview frame rates: %s",
                    __FUNCTION__, supportedPreviewFrameRates.string());
        }
        params.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES,
                supportedPreviewFrameRates);
    }

    Vector<Size> availableJpegSizes = getAvailableJpegSizes();
    if (!availableJpegSizes.size()) return NO_INIT;

    pictureWidth = availableJpegSizes[0].width;
    pictureHeight = availableJpegSizes[0].height;
    if (fastInfo.supportsPreferredConfigs) {
        Size suggestedJpegSize = getMaxSize(getPreferredJpegSizes());
        pictureWidth = suggestedJpegSize.width;
        pictureHeight = suggestedJpegSize.height;
    }
    pictureWidthLastSet = pictureWidth;
    pictureHeightLastSet = pictureHeight;
    pictureSizeOverriden = false;

    params.setPictureSize(pictureWidth,
            pictureHeight);

    {
        String8 supportedPictureSizes;
        for (size_t i=0; i < availableJpegSizes.size(); i++) {
            if (i != 0) supportedPictureSizes += ",";
            supportedPictureSizes += String8::format("%dx%d",
                    availableJpegSizes[i].width,
                    availableJpegSizes[i].height);
        }
        params.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES,
                supportedPictureSizes);
    }

    params.setPictureFormat(CameraParameters::PIXEL_FORMAT_JPEG);
    params.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS,
            CameraParameters::PIXEL_FORMAT_JPEG);

    camera_metadata_ro_entry_t availableJpegThumbnailSizes =
        staticInfo(ANDROID_JPEG_AVAILABLE_THUMBNAIL_SIZES, 4);
    if (!availableJpegThumbnailSizes.count) return NO_INIT;

    // Pick the largest thumbnail size that matches still image aspect ratio.
    ALOG_ASSERT(pictureWidth > 0 && pictureHeight > 0,
            "Invalid picture size, %d x %d", pictureWidth, pictureHeight);
    float picAspectRatio = static_cast<float>(pictureWidth) / pictureHeight;
    Size thumbnailSize =
            getMaxSizeForRatio(
                    picAspectRatio,
                    &availableJpegThumbnailSizes.data.i32[0],
                    availableJpegThumbnailSizes.count);
    jpegThumbSize[0] = thumbnailSize.width;
    jpegThumbSize[1] = thumbnailSize.height;

    params.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH,
            jpegThumbSize[0]);
    params.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT,
            jpegThumbSize[1]);

    {
        String8 supportedJpegThumbSizes;
        for (size_t i=0; i < availableJpegThumbnailSizes.count; i += 2) {
            if (i != 0) supportedJpegThumbSizes += ",";
            supportedJpegThumbSizes += String8::format("%dx%d",
                    availableJpegThumbnailSizes.data.i32[i],
                    availableJpegThumbnailSizes.data.i32[i+1]);
        }
        params.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES,
                supportedJpegThumbSizes);
    }

    jpegThumbQuality = 90;
    params.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY,
            jpegThumbQuality);
    jpegQuality = 90;
    params.set(CameraParameters::KEY_JPEG_QUALITY,
            jpegQuality);
    jpegRotation = 0;
    params.set(CameraParameters::KEY_ROTATION,
            jpegRotation);

    gpsEnabled = false;
    gpsCoordinates[0] = 0.0;
    gpsCoordinates[1] = 0.0;
    gpsCoordinates[2] = 0.0;
    gpsTimestamp = 0;
    gpsProcessingMethod = "unknown";
    // GPS fields in CameraParameters are not set by implementation

    wbMode = ANDROID_CONTROL_AWB_MODE_AUTO;
    params.set(CameraParameters::KEY_WHITE_BALANCE,
            CameraParameters::WHITE_BALANCE_AUTO);

    camera_metadata_ro_entry_t availableWhiteBalanceModes =
        staticInfo(ANDROID_CONTROL_AWB_AVAILABLE_MODES, 0, 0, false);
    if (!availableWhiteBalanceModes.count) {
        params.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE,
                CameraParameters::WHITE_BALANCE_AUTO);
    } else {
        String8 supportedWhiteBalance;
        bool addComma = false;
        for (size_t i=0; i < availableWhiteBalanceModes.count; i++) {
            if (addComma) supportedWhiteBalance += ",";
            addComma = true;
            switch (availableWhiteBalanceModes.data.u8[i]) {
            case ANDROID_CONTROL_AWB_MODE_AUTO:
                supportedWhiteBalance +=
                    CameraParameters::WHITE_BALANCE_AUTO;
                break;
            case ANDROID_CONTROL_AWB_MODE_INCANDESCENT:
                supportedWhiteBalance +=
                    CameraParameters::WHITE_BALANCE_INCANDESCENT;
                break;
            case ANDROID_CONTROL_AWB_MODE_FLUORESCENT:
                supportedWhiteBalance +=
                    CameraParameters::WHITE_BALANCE_FLUORESCENT;
                break;
            case ANDROID_CONTROL_AWB_MODE_WARM_FLUORESCENT:
                supportedWhiteBalance +=
                    CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT;
                break;
            case ANDROID_CONTROL_AWB_MODE_DAYLIGHT:
                supportedWhiteBalance +=
                    CameraParameters::WHITE_BALANCE_DAYLIGHT;
                break;
            case ANDROID_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT:
                supportedWhiteBalance +=
                    CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT;
                break;
            case ANDROID_CONTROL_AWB_MODE_TWILIGHT:
                supportedWhiteBalance +=
                    CameraParameters::WHITE_BALANCE_TWILIGHT;
                break;
            case ANDROID_CONTROL_AWB_MODE_SHADE:
                supportedWhiteBalance +=
                    CameraParameters::WHITE_BALANCE_SHADE;
                break;
            // Skipping values not mappable to v1 API
            case ANDROID_CONTROL_AWB_MODE_OFF:
                addComma = false;
                break;
            default:
                ALOGW("%s: Camera %d: Unknown white balance value: %d",
                        __FUNCTION__, cameraId,
                        availableWhiteBalanceModes.data.u8[i]);
                addComma = false;
                break;
            }
        }
        params.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE,
                supportedWhiteBalance);
    }

    effectMode = ANDROID_CONTROL_EFFECT_MODE_OFF;
    params.set(CameraParameters::KEY_EFFECT,
            CameraParameters::EFFECT_NONE);

    camera_metadata_ro_entry_t availableEffects =
        staticInfo(ANDROID_CONTROL_AVAILABLE_EFFECTS, 0, 0, false);
    if (!availableEffects.count) {
        params.set(CameraParameters::KEY_SUPPORTED_EFFECTS,
                CameraParameters::EFFECT_NONE);
    } else {
        String8 supportedEffects;
        bool addComma = false;
        for (size_t i=0; i < availableEffects.count; i++) {
            if (addComma) supportedEffects += ",";
            addComma = true;
            switch (availableEffects.data.u8[i]) {
                case ANDROID_CONTROL_EFFECT_MODE_OFF:
                    supportedEffects +=
                        CameraParameters::EFFECT_NONE;
                    break;
                case ANDROID_CONTROL_EFFECT_MODE_MONO:
                    supportedEffects +=
                        CameraParameters::EFFECT_MONO;
                    break;
                case ANDROID_CONTROL_EFFECT_MODE_NEGATIVE:
                    supportedEffects +=
                        CameraParameters::EFFECT_NEGATIVE;
                    break;
                case ANDROID_CONTROL_EFFECT_MODE_SOLARIZE:
                    supportedEffects +=
                        CameraParameters::EFFECT_SOLARIZE;
                    break;
                case ANDROID_CONTROL_EFFECT_MODE_SEPIA:
                    supportedEffects +=
                        CameraParameters::EFFECT_SEPIA;
                    break;
                case ANDROID_CONTROL_EFFECT_MODE_POSTERIZE:
                    supportedEffects +=
                        CameraParameters::EFFECT_POSTERIZE;
                    break;
                case ANDROID_CONTROL_EFFECT_MODE_WHITEBOARD:
                    supportedEffects +=
                        CameraParameters::EFFECT_WHITEBOARD;
                    break;
                case ANDROID_CONTROL_EFFECT_MODE_BLACKBOARD:
                    supportedEffects +=
                        CameraParameters::EFFECT_BLACKBOARD;
                    break;
                case ANDROID_CONTROL_EFFECT_MODE_AQUA:
                    supportedEffects +=
                        CameraParameters::EFFECT_AQUA;
                    break;
                default:
                    ALOGW("%s: Camera %d: Unknown effect value: %d",
                        __FUNCTION__, cameraId, availableEffects.data.u8[i]);
                    addComma = false;
                    break;
            }
        }
        params.set(CameraParameters::KEY_SUPPORTED_EFFECTS, supportedEffects);
    }

    antibandingMode = ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO;
    params.set(CameraParameters::KEY_ANTIBANDING,
            CameraParameters::ANTIBANDING_AUTO);

    camera_metadata_ro_entry_t availableAntibandingModes =
        staticInfo(ANDROID_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES, 0, 0, false);
    if (!availableAntibandingModes.count) {
        params.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING,
                CameraParameters::ANTIBANDING_OFF);
    } else {
        String8 supportedAntibanding;
        bool addComma = false;
        for (size_t i=0; i < availableAntibandingModes.count; i++) {
            if (addComma) supportedAntibanding += ",";
            addComma = true;
            switch (availableAntibandingModes.data.u8[i]) {
                case ANDROID_CONTROL_AE_ANTIBANDING_MODE_OFF:
                    supportedAntibanding +=
                        CameraParameters::ANTIBANDING_OFF;
                    break;
                case ANDROID_CONTROL_AE_ANTIBANDING_MODE_50HZ:
                    supportedAntibanding +=
                        CameraParameters::ANTIBANDING_50HZ;
                    break;
                case ANDROID_CONTROL_AE_ANTIBANDING_MODE_60HZ:
                    supportedAntibanding +=
                        CameraParameters::ANTIBANDING_60HZ;
                    break;
                case ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO:
                    supportedAntibanding +=
                        CameraParameters::ANTIBANDING_AUTO;
                    break;
                default:
                    ALOGW("%s: Camera %d: Unknown antibanding value: %d",
                        __FUNCTION__, cameraId,
                            availableAntibandingModes.data.u8[i]);
                    addComma = false;
                    break;
            }
        }
        params.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING,
                supportedAntibanding);
    }

    sceneMode = ANDROID_CONTROL_SCENE_MODE_DISABLED;
    params.set(CameraParameters::KEY_SCENE_MODE,
            CameraParameters::SCENE_MODE_AUTO);

    camera_metadata_ro_entry_t availableSceneModes =
        staticInfo(ANDROID_CONTROL_AVAILABLE_SCENE_MODES, 0, 0, false);
    if (!availableSceneModes.count) {
        params.remove(CameraParameters::KEY_SCENE_MODE);
    } else {
        String8 supportedSceneModes(CameraParameters::SCENE_MODE_AUTO);
        bool addComma = true;
        bool noSceneModes = false;
        for (size_t i=0; i < availableSceneModes.count; i++) {
            if (addComma) supportedSceneModes += ",";
            addComma = true;
            switch (availableSceneModes.data.u8[i]) {
                case ANDROID_CONTROL_SCENE_MODE_DISABLED:
                    noSceneModes = true;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_FACE_PRIORITY:
                    // Face priority can be used as alternate default if supported.
                    // Per API contract it shouldn't override the user set flash,
                    // white balance and focus modes.
                    mDefaultSceneMode = availableSceneModes.data.u8[i];
                    // Not in old API
                    addComma = false;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_ACTION:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_ACTION;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_PORTRAIT:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_PORTRAIT;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_LANDSCAPE:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_LANDSCAPE;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_NIGHT:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_NIGHT;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_NIGHT_PORTRAIT:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_NIGHT_PORTRAIT;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_THEATRE:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_THEATRE;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_BEACH:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_BEACH;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_SNOW:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_SNOW;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_SUNSET:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_SUNSET;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_STEADYPHOTO:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_STEADYPHOTO;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_FIREWORKS:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_FIREWORKS;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_SPORTS:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_SPORTS;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_PARTY:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_PARTY;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_CANDLELIGHT:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_CANDLELIGHT;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_BARCODE:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_BARCODE;
                    break;
                case ANDROID_CONTROL_SCENE_MODE_HDR:
                    supportedSceneModes +=
                        CameraParameters::SCENE_MODE_HDR;
                    break;
                default:
                    ALOGW("%s: Camera %d: Unknown scene mode value: %d",
                        __FUNCTION__, cameraId,
                            availableSceneModes.data.u8[i]);
                    addComma = false;
                    break;
            }
        }
        if (!noSceneModes) {
            params.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES,
                    supportedSceneModes);
        } else {
            params.remove(CameraParameters::KEY_SCENE_MODE);
        }
    }

    bool isFlashAvailable = false;
    camera_metadata_ro_entry_t flashAvailable =
        staticInfo(ANDROID_FLASH_INFO_AVAILABLE, 0, 1, false);
    if (flashAvailable.count) {
        isFlashAvailable = flashAvailable.data.u8[0];
    }

    camera_metadata_ro_entry_t availableAeModes =
        staticInfo(ANDROID_CONTROL_AE_AVAILABLE_MODES, 0, 0, false);

    flashMode = Parameters::FLASH_MODE_OFF;
    if (isFlashAvailable) {
        params.set(CameraParameters::KEY_FLASH_MODE,
                CameraParameters::FLASH_MODE_OFF);

        String8 supportedFlashModes(CameraParameters::FLASH_MODE_OFF);
        supportedFlashModes = supportedFlashModes +
            "," + CameraParameters::FLASH_MODE_AUTO +
            "," + CameraParameters::FLASH_MODE_ON +
            "," + CameraParameters::FLASH_MODE_TORCH;
        for (size_t i=0; i < availableAeModes.count; i++) {
            if (availableAeModes.data.u8[i] ==
                    ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE) {
                supportedFlashModes = supportedFlashModes + "," +
                    CameraParameters::FLASH_MODE_RED_EYE;
                break;
            }
        }
        params.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES,
                supportedFlashModes);
    } else {
        // No flash means null flash mode and supported flash modes keys, so
        // remove them just to be safe
        params.remove(CameraParameters::KEY_FLASH_MODE);
        params.remove(CameraParameters::KEY_SUPPORTED_FLASH_MODES);
    }

    camera_metadata_ro_entry_t minFocusDistance =
        staticInfo(ANDROID_LENS_INFO_MINIMUM_FOCUS_DISTANCE, 0, 1, false);

    camera_metadata_ro_entry_t availableAfModes =
        staticInfo(ANDROID_CONTROL_AF_AVAILABLE_MODES, 0, 0, false);

    if (!minFocusDistance.count || minFocusDistance.data.f[0] == 0) {
        // Fixed-focus lens
        focusMode = Parameters::FOCUS_MODE_FIXED;
        params.set(CameraParameters::KEY_FOCUS_MODE,
                CameraParameters::FOCUS_MODE_FIXED);
        params.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,
                CameraParameters::FOCUS_MODE_FIXED);
    } else {
        focusMode = Parameters::FOCUS_MODE_AUTO;
        params.set(CameraParameters::KEY_FOCUS_MODE,
                CameraParameters::FOCUS_MODE_AUTO);
        String8 supportedFocusModes;
        bool addComma = false;
        camera_metadata_ro_entry_t focusDistanceCalibration =
            staticInfo(ANDROID_LENS_INFO_FOCUS_DISTANCE_CALIBRATION, 0, 0, false);

        if (focusDistanceCalibration.count &&
                focusDistanceCalibration.data.u8[0] !=
                ANDROID_LENS_INFO_FOCUS_DISTANCE_CALIBRATION_UNCALIBRATED) {
            supportedFocusModes += CameraParameters::FOCUS_MODE_INFINITY;
            addComma = true;
        }

        for (size_t i=0; i < availableAfModes.count; i++) {
            if (addComma) supportedFocusModes += ",";
            addComma = true;
            switch (availableAfModes.data.u8[i]) {
                case ANDROID_CONTROL_AF_MODE_AUTO:
                    supportedFocusModes +=
                        CameraParameters::FOCUS_MODE_AUTO;
                    break;
                case ANDROID_CONTROL_AF_MODE_MACRO:
                    supportedFocusModes +=
                        CameraParameters::FOCUS_MODE_MACRO;
                    break;
                case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
                    supportedFocusModes +=
                        CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO;
                    break;
                case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
                    supportedFocusModes +=
                        CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE;
                    break;
                case ANDROID_CONTROL_AF_MODE_EDOF:
                    supportedFocusModes +=
                        CameraParameters::FOCUS_MODE_EDOF;
                    break;
                // Not supported in old API
                case ANDROID_CONTROL_AF_MODE_OFF:
                    addComma = false;
                    break;
                default:
                    ALOGW("%s: Camera %d: Unknown AF mode value: %d",
                        __FUNCTION__, cameraId, availableAfModes.data.u8[i]);
                    addComma = false;
                    break;
            }
        }
        params.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,
                supportedFocusModes);
    }
    focusState = ANDROID_CONTROL_AF_STATE_INACTIVE;
    shadowFocusMode = FOCUS_MODE_INVALID;

    aeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
    camera_metadata_ro_entry_t max3aRegions = staticInfo(ANDROID_CONTROL_MAX_REGIONS,
            Parameters::NUM_REGION, Parameters::NUM_REGION);
    if (max3aRegions.count != Parameters::NUM_REGION) return NO_INIT;

    int32_t maxNumFocusAreas = 0;
    if (focusMode != Parameters::FOCUS_MODE_FIXED) {
        maxNumFocusAreas = max3aRegions.data.i32[Parameters::REGION_AF];
    }
    params.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS, maxNumFocusAreas);
    params.set(CameraParameters::KEY_FOCUS_AREAS,
            "(0,0,0,0,0)");
    focusingAreas.clear();
    focusingAreas.add(Parameters::Area(0,0,0,0,0));

    params.setFloat(CameraParameters::KEY_FOCAL_LENGTH, fastInfo.defaultFocalLength);

    float horizFov, vertFov;
    res = calculatePictureFovs(&horizFov, &vertFov);
    if (res != OK) {
        ALOGE("%s: Can't calculate field of views!", __FUNCTION__);
        return res;
    }

    params.setFloat(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE, horizFov);
    params.setFloat(CameraParameters::KEY_VERTICAL_VIEW_ANGLE, vertFov);

    exposureCompensation = 0;
    params.set(CameraParameters::KEY_EXPOSURE_COMPENSATION,
                exposureCompensation);

    camera_metadata_ro_entry_t exposureCompensationRange =
        staticInfo(ANDROID_CONTROL_AE_COMPENSATION_RANGE, 2, 2);
    if (!exposureCompensationRange.count) return NO_INIT;

    params.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION,
            exposureCompensationRange.data.i32[1]);
    params.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION,
            exposureCompensationRange.data.i32[0]);

    camera_metadata_ro_entry_t exposureCompensationStep =
        staticInfo(ANDROID_CONTROL_AE_COMPENSATION_STEP, 1, 1);
    if (!exposureCompensationStep.count) return NO_INIT;

    params.setFloat(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP,
            (float)exposureCompensationStep.data.r[0].numerator /
            exposureCompensationStep.data.r[0].denominator);

    autoExposureLock = false;
    autoExposureLockAvailable = false;
    camera_metadata_ro_entry_t exposureLockAvailable =
        staticInfo(ANDROID_CONTROL_AE_LOCK_AVAILABLE, 1, 1);
    if ((0 < exposureLockAvailable.count) &&
            (ANDROID_CONTROL_AE_LOCK_AVAILABLE_TRUE ==
                    exposureLockAvailable.data.u8[0])) {
        params.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK,
                CameraParameters::FALSE);
        params.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED,
                   CameraParameters::TRUE);
        autoExposureLockAvailable = true;
    } else {
        params.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED,
                   CameraParameters::FALSE);
    }

    autoWhiteBalanceLock = false;
    autoWhiteBalanceLockAvailable = false;
    camera_metadata_ro_entry_t whitebalanceLockAvailable =
        staticInfo(ANDROID_CONTROL_AWB_LOCK_AVAILABLE, 1, 1);
    if ((0 < whitebalanceLockAvailable.count) &&
            (ANDROID_CONTROL_AWB_LOCK_AVAILABLE_TRUE ==
                    whitebalanceLockAvailable.data.u8[0])) {
        params.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK,
                CameraParameters::FALSE);
        params.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED,
                CameraParameters::TRUE);
        autoWhiteBalanceLockAvailable = true;
    } else {
        params.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED,
                CameraParameters::FALSE);
    }

    meteringAreas.add(Parameters::Area(0, 0, 0, 0, 0));
    params.set(CameraParameters::KEY_MAX_NUM_METERING_AREAS,
            max3aRegions.data.i32[Parameters::REGION_AE]);
    params.set(CameraParameters::KEY_METERING_AREAS,
            "(0,0,0,0,0)");

    zoom = 0;
    zoomAvailable = false;
    camera_metadata_ro_entry_t maxDigitalZoom =
        staticInfo(ANDROID_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM, /*minCount*/1, /*maxCount*/1);
    if (!maxDigitalZoom.count) return NO_INIT;

    if (fabs(maxDigitalZoom.data.f[0] - 1.f) > 0.00001f) {
        params.set(CameraParameters::KEY_ZOOM, zoom);
        params.set(CameraParameters::KEY_MAX_ZOOM, NUM_ZOOM_STEPS - 1);

        {
            String8 zoomRatios;
            float zoom = 1.f;
            float zoomIncrement = (maxDigitalZoom.data.f[0] - zoom) /
                    (NUM_ZOOM_STEPS-1);
            bool addComma = false;
            for (size_t i=0; i < NUM_ZOOM_STEPS; i++) {
                if (addComma) zoomRatios += ",";
                addComma = true;
                zoomRatios += String8::format("%d", static_cast<int>(zoom * 100));
                zoom += zoomIncrement;
            }
            params.set(CameraParameters::KEY_ZOOM_RATIOS, zoomRatios);
        }

        params.set(CameraParameters::KEY_ZOOM_SUPPORTED,
                CameraParameters::TRUE);
        zoomAvailable = true;
    } else {
        params.set(CameraParameters::KEY_ZOOM_SUPPORTED,
                CameraParameters::FALSE);
    }
    params.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED,
            CameraParameters::FALSE);

    params.set(CameraParameters::KEY_FOCUS_DISTANCES,
            "Infinity,Infinity,Infinity");

    params.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW,
            fastInfo.maxFaces);
    params.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_SW,
            0);

    params.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT,
            CameraParameters::PIXEL_FORMAT_ANDROID_OPAQUE);

    recordingHint = false;
    params.set(CameraParameters::KEY_RECORDING_HINT,
            CameraParameters::FALSE);

    params.set(CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED,
            CameraParameters::TRUE);

    videoStabilization = false;
    params.set(CameraParameters::KEY_VIDEO_STABILIZATION,
            CameraParameters::FALSE);

    camera_metadata_ro_entry_t availableVideoStabilizationModes =
        staticInfo(ANDROID_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES, 0, 0,
                false);

    if (availableVideoStabilizationModes.count > 1) {
        params.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED,
                CameraParameters::TRUE);
    } else {
        params.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED,
                CameraParameters::FALSE);
    }

    // Set up initial state for non-Camera.Parameters state variables
    videoFormat = HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;
    videoDataSpace = HAL_DATASPACE_V0_BT709;
    videoBufferMode = hardware::ICamera::VIDEO_BUFFER_MODE_DATA_CALLBACK_YUV;
    playShutterSound = true;
    enableFaceDetect = false;

    enableFocusMoveMessages = false;
    afTriggerCounter = 1;
    afStateCounter = 0;
    currentAfTriggerId = -1;
    afInMotion = false;

    precaptureTriggerCounter = 1;

    takePictureCounter = 0;

    previewCallbackFlags = 0;
    previewCallbackOneShot = false;
    previewCallbackSurface = false;

    Size maxJpegSize = getMaxSize(getAvailableJpegSizes());
    int64_t minFrameDurationNs = getJpegStreamMinFrameDurationNs(maxJpegSize);

    slowJpegMode = false;
    if (minFrameDurationNs > kSlowJpegModeThreshold) {
        slowJpegMode = true;
        // Slow jpeg devices does not support video snapshot without
        // slowing down preview.
        // TODO: support video size video snapshot only?
        params.set(CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED,
            CameraParameters::FALSE);
    }

    isZslReprocessPresent = false;
    camera_metadata_ro_entry_t availableCapabilities =
        staticInfo(ANDROID_REQUEST_AVAILABLE_CAPABILITIES);
    if (0 < availableCapabilities.count) {
        const uint8_t *caps = availableCapabilities.data.u8;
        for (size_t i = 0; i < availableCapabilities.count; i++) {
            if (ANDROID_REQUEST_AVAILABLE_CAPABILITIES_PRIVATE_REPROCESSING ==
                    caps[i]) {
                isZslReprocessPresent = true;
                break;
            }
        }
    }

    isDistortionCorrectionSupported = false;
    camera_metadata_ro_entry_t distortionCorrectionModes =
            staticInfo(ANDROID_DISTORTION_CORRECTION_AVAILABLE_MODES);
    for (size_t i = 0; i < distortionCorrectionModes.count; i++) {
        if (distortionCorrectionModes.data.u8[i] !=
                ANDROID_DISTORTION_CORRECTION_MODE_OFF) {
            isDistortionCorrectionSupported = true;
            break;
        }
    }

    if (isDeviceZslSupported || slowJpegMode ||
            property_get_bool("camera.disable_zsl_mode", false)) {
        ALOGI("Camera %d: Disabling ZSL mode", cameraId);
        allowZslMode = false;
    } else {
        allowZslMode = isZslReprocessPresent;
    }

    ALOGI("%s: allowZslMode: %d slowJpegMode %d", __FUNCTION__, allowZslMode, slowJpegMode);

    state = STOPPED;

    paramsFlattened = params.flatten();

    return OK;
}

String8 Parameters::get() const {
    return paramsFlattened;
}

status_t Parameters::buildFastInfo(CameraDeviceBase *device) {

    camera_metadata_ro_entry_t activeArraySize =
        staticInfo(ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE, 2, 4);
    if (!activeArraySize.count) return NO_INIT;
    int32_t arrayWidth;
    int32_t arrayHeight;
    if (activeArraySize.count == 2) {
        ALOGW("%s: Camera %d: activeArraySize is missing xmin/ymin!",
                __FUNCTION__, cameraId);
        arrayWidth = activeArraySize.data.i32[0];
        arrayHeight = activeArraySize.data.i32[1];
    } else if (activeArraySize.count == 4) {
        arrayWidth = activeArraySize.data.i32[2];
        arrayHeight = activeArraySize.data.i32[3];
    } else return NO_INIT;

    fastInfo.supportsPreferredConfigs =
        info->exists(ANDROID_SCALER_AVAILABLE_RECOMMENDED_STREAM_CONFIGURATIONS);

    // We'll set the target FPS range for still captures to be as wide
    // as possible to give the HAL maximum latitude for exposure selection
    camera_metadata_ro_entry_t availableFpsRanges =
        staticInfo(ANDROID_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES, 2);
    if (availableFpsRanges.count < 2 || availableFpsRanges.count % 2 != 0) {
        return NO_INIT;
    }

    // Get supported preview fps ranges, up to default maximum.
    Vector<Size> supportedPreviewSizes;
    Vector<FpsRange> supportedPreviewFpsRanges;
    Size previewSizeBound = { MAX_PREVIEW_WIDTH, MAX_PREVIEW_HEIGHT };
    if (fastInfo.supportsPreferredConfigs) {
        previewSizeBound = getMaxSize(getPreferredPreviewSizes());
    }
    status_t res = getFilteredSizes(previewSizeBound, &supportedPreviewSizes);
    if (res != OK) return res;
    for (size_t i=0; i < availableFpsRanges.count; i += 2) {
        if (!isFpsSupported(supportedPreviewSizes,
                HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, availableFpsRanges.data.i32[i+1]) ||
                availableFpsRanges.data.i32[i+1] > MAX_DEFAULT_FPS) {
            continue;
        }
        FpsRange fpsRange = {availableFpsRanges.data.i32[i], availableFpsRanges.data.i32[i+1]};
        supportedPreviewFpsRanges.add(fpsRange);
    }
    if (supportedPreviewFpsRanges.size() == 0) {
        ALOGE("Supported preview fps range is empty");
        return NO_INIT;
    }

    int32_t bestStillCaptureFpsRange[2] = {
        supportedPreviewFpsRanges[0].low, supportedPreviewFpsRanges[0].high
    };
    int32_t curRange =
            bestStillCaptureFpsRange[1] - bestStillCaptureFpsRange[0];
    for (size_t i = 1; i < supportedPreviewFpsRanges.size(); i ++) {
        int32_t nextRange =
                supportedPreviewFpsRanges[i].high -
                supportedPreviewFpsRanges[i].low;
        if ( (nextRange > curRange) ||       // Maximize size of FPS range first
                (nextRange == curRange &&    // Then minimize low-end FPS
                 bestStillCaptureFpsRange[0] > supportedPreviewFpsRanges[i].low)) {

            bestStillCaptureFpsRange[0] = supportedPreviewFpsRanges[i].low;
            bestStillCaptureFpsRange[1] = supportedPreviewFpsRanges[i].high;
            curRange = nextRange;
        }
    }

    camera_metadata_ro_entry_t availableFaceDetectModes =
        staticInfo(ANDROID_STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES, 0, 0,
                false);

    uint8_t bestFaceDetectMode =
        ANDROID_STATISTICS_FACE_DETECT_MODE_OFF;
    for (size_t i = 0 ; i < availableFaceDetectModes.count; i++) {
        switch (availableFaceDetectModes.data.u8[i]) {
            case ANDROID_STATISTICS_FACE_DETECT_MODE_OFF:
                break;
            case ANDROID_STATISTICS_FACE_DETECT_MODE_SIMPLE:
                if (bestFaceDetectMode !=
                        ANDROID_STATISTICS_FACE_DETECT_MODE_FULL) {
                    bestFaceDetectMode =
                        ANDROID_STATISTICS_FACE_DETECT_MODE_SIMPLE;
                }
                break;
            case ANDROID_STATISTICS_FACE_DETECT_MODE_FULL:
                bestFaceDetectMode =
                    ANDROID_STATISTICS_FACE_DETECT_MODE_FULL;
                break;
            default:
                ALOGE("%s: Camera %d: Unknown face detect mode %d:",
                        __FUNCTION__, cameraId,
                        availableFaceDetectModes.data.u8[i]);
                return NO_INIT;
        }
    }

    int32_t maxFaces = 0;
    camera_metadata_ro_entry_t maxFacesDetected =
        staticInfo(ANDROID_STATISTICS_INFO_MAX_FACE_COUNT, 0, 1, false);
    if (maxFacesDetected.count) {
        maxFaces = maxFacesDetected.data.i32[0];
    }

    camera_metadata_ro_entry_t availableSceneModes =
        staticInfo(ANDROID_CONTROL_AVAILABLE_SCENE_MODES, 0, 0, false);
    camera_metadata_ro_entry_t sceneModeOverrides =
        staticInfo(ANDROID_CONTROL_SCENE_MODE_OVERRIDES, 0, 0, false);
    camera_metadata_ro_entry_t minFocusDistance =
        staticInfo(ANDROID_LENS_INFO_MINIMUM_FOCUS_DISTANCE, 0, 0, false);
    bool fixedLens = minFocusDistance.count == 0 ||
        minFocusDistance.data.f[0] == 0;

    camera_metadata_ro_entry_t focusDistanceCalibration =
            staticInfo(ANDROID_LENS_INFO_FOCUS_DISTANCE_CALIBRATION, 0, 0,
                    false);
    bool canFocusInfinity = (focusDistanceCalibration.count &&
            focusDistanceCalibration.data.u8[0] !=
            ANDROID_LENS_INFO_FOCUS_DISTANCE_CALIBRATION_UNCALIBRATED);

    res = getDefaultFocalLength(device);
    if (res != OK) return res;

    SortedVector<int32_t> availableFormats = getAvailableOutputFormats();
    if (!availableFormats.size()) return NO_INIT;

    if (sceneModeOverrides.count > 0) {
        // sceneModeOverrides is defined to have 3 entries for each scene mode,
        // which are AE, AWB, and AF override modes the HAL wants for that scene
        // mode.
        const size_t kModesPerSceneMode = 3;
        if (sceneModeOverrides.count !=
                availableSceneModes.count * kModesPerSceneMode) {
            ALOGE("%s: Camera %d: Scene mode override list is an "
                    "unexpected size: %zu (expected %zu)", __FUNCTION__,
                    cameraId, sceneModeOverrides.count,
                    availableSceneModes.count * kModesPerSceneMode);
            return NO_INIT;
        }
        for (size_t i = 0; i < availableSceneModes.count; i++) {
            DeviceInfo::OverrideModes modes;
            uint8_t aeMode =
                    sceneModeOverrides.data.u8[i * kModesPerSceneMode + 0];
            switch(aeMode) {
                case ANDROID_CONTROL_AE_MODE_ON:
                    modes.flashMode = FLASH_MODE_OFF;
                    break;
                case ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH:
                    modes.flashMode = FLASH_MODE_AUTO;
                    break;
                case ANDROID_CONTROL_AE_MODE_ON_ALWAYS_FLASH:
                    modes.flashMode = FLASH_MODE_ON;
                    break;
                case ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE:
                    modes.flashMode = FLASH_MODE_RED_EYE;
                    break;
                default:
                    ALOGE("%s: Unknown override AE mode: %d", __FUNCTION__,
                            aeMode);
                    modes.flashMode = FLASH_MODE_INVALID;
                    break;
            }
            modes.wbMode =
                    sceneModeOverrides.data.u8[i * kModesPerSceneMode + 1];
            uint8_t afMode =
                    sceneModeOverrides.data.u8[i * kModesPerSceneMode + 2];
            switch(afMode) {
                case ANDROID_CONTROL_AF_MODE_OFF:
                    if (!fixedLens && !canFocusInfinity) {
                        ALOGE("%s: Camera %d: Scene mode override lists asks for"
                                " fixed focus on a device with focuser but not"
                                " calibrated for infinity focus", __FUNCTION__,
                                cameraId);
                        return NO_INIT;
                    }
                    modes.focusMode = fixedLens ?
                            FOCUS_MODE_FIXED : FOCUS_MODE_INFINITY;
                    break;
                case ANDROID_CONTROL_AF_MODE_AUTO:
                case ANDROID_CONTROL_AF_MODE_MACRO:
                case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
                case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
                case ANDROID_CONTROL_AF_MODE_EDOF:
                    modes.focusMode = static_cast<focusMode_t>(afMode);
                    break;
                default:
                    ALOGE("%s: Unknown override AF mode: %d", __FUNCTION__,
                            afMode);
                    modes.focusMode = FOCUS_MODE_INVALID;
                    break;
            }
            fastInfo.sceneModeOverrides.add(availableSceneModes.data.u8[i],
                    modes);
        }
    }

    fastInfo.arrayWidth = arrayWidth;
    fastInfo.arrayHeight = arrayHeight;
    fastInfo.bestStillCaptureFpsRange[0] = bestStillCaptureFpsRange[0];
    fastInfo.bestStillCaptureFpsRange[1] = bestStillCaptureFpsRange[1];
    fastInfo.bestFaceDetectMode = bestFaceDetectMode;
    fastInfo.maxFaces = maxFaces;

    // Check if the HAL supports HAL_PIXEL_FORMAT_YCbCr_420_888
    fastInfo.useFlexibleYuv = false;
    for (size_t i = 0; i < availableFormats.size(); i++) {
        if (availableFormats[i] == HAL_PIXEL_FORMAT_YCbCr_420_888) {
            fastInfo.useFlexibleYuv = true;
            break;
        }
    }
    ALOGV("Camera %d: Flexible YUV %s supported",
            cameraId, fastInfo.useFlexibleYuv ? "is" : "is not");

    fastInfo.maxJpegSize = getMaxSize(getAvailableJpegSizes());

    isZslReprocessPresent = false;
    camera_metadata_ro_entry_t availableCapabilities =
        staticInfo(ANDROID_REQUEST_AVAILABLE_CAPABILITIES);
    if (0 < availableCapabilities.count) {
        const uint8_t *caps = availableCapabilities.data.u8;
        for (size_t i = 0; i < availableCapabilities.count; i++) {
            if (ANDROID_REQUEST_AVAILABLE_CAPABILITIES_PRIVATE_REPROCESSING ==
                    caps[i]) {
                isZslReprocessPresent = true;
                break;
            }
        }
    }
    if (isZslReprocessPresent) {
        Vector<StreamConfiguration> scs = getStreamConfigurations();
        Size maxPrivInputSize = {0, 0};
        for (const auto& sc : scs) {
            if (sc.isInput == ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_INPUT &&
                    sc.format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED) {
                if (sc.width * sc.height > maxPrivInputSize.width * maxPrivInputSize.height) {
                    maxPrivInputSize = {sc.width, sc.height};
                }
            }
        }
        fastInfo.maxZslSize = maxPrivInputSize;
    } else {
        fastInfo.maxZslSize = {0, 0};
    }

    return OK;
}

status_t Parameters::buildQuirks() {
    camera_metadata_ro_entry_t entry;
    entry = info->find(ANDROID_QUIRKS_TRIGGER_AF_WITH_AUTO);
    quirks.triggerAfWithAuto = (entry.count != 0 && entry.data.u8[0] == 1);
    ALOGV_IF(quirks.triggerAfWithAuto, "Camera %d: Quirk triggerAfWithAuto enabled",
            cameraId);

    entry = info->find(ANDROID_QUIRKS_USE_ZSL_FORMAT);
    quirks.useZslFormat = (entry.count != 0 && entry.data.u8[0] == 1);
    ALOGV_IF(quirks.useZslFormat, "Camera %d: Quirk useZslFormat enabled",
            cameraId);

    entry = info->find(ANDROID_QUIRKS_METERING_CROP_REGION);
    quirks.meteringCropRegion = (entry.count != 0 && entry.data.u8[0] == 1);
    ALOGV_IF(quirks.meteringCropRegion, "Camera %d: Quirk meteringCropRegion"
                " enabled", cameraId);

    entry = info->find(ANDROID_QUIRKS_USE_PARTIAL_RESULT);
    quirks.partialResults = (entry.count != 0 && entry.data.u8[0] == 1);
    ALOGV_IF(quirks.partialResults, "Camera %d: Quirk usePartialResult"
                " enabled", cameraId);

    return OK;
}

camera_metadata_ro_entry_t Parameters::staticInfo(uint32_t tag,
        size_t minCount, size_t maxCount, bool required) const {
    camera_metadata_ro_entry_t entry = info->find(tag);
    const camera_metadata_t *metaBuffer = info->getAndLock();

    if (CC_UNLIKELY( entry.count == 0 ) && required) {
        const char* tagSection = get_local_camera_metadata_section_name(tag,
                metaBuffer);
        if (tagSection == NULL) tagSection = "<unknown>";
        const char* tagName = get_local_camera_metadata_tag_name(tag,
                metaBuffer);
        if (tagName == NULL) tagName = "<unknown>";

        ALOGE("Error finding static metadata entry '%s.%s' (%x)",
                tagSection, tagName, tag);
    } else if (CC_UNLIKELY(
            (minCount != 0 && entry.count < minCount) ||
            (maxCount != 0 && entry.count > maxCount) ) ) {
        const char* tagSection = get_local_camera_metadata_section_name(tag,
                metaBuffer);
        if (tagSection == NULL) tagSection = "<unknown>";
        const char* tagName = get_local_camera_metadata_tag_name(tag,
                metaBuffer);
        if (tagName == NULL) tagName = "<unknown>";
        ALOGE("Malformed static metadata entry '%s.%s' (%x):"
                "Expected between %zu and %zu values, but got %zu values",
                tagSection, tagName, tag, minCount, maxCount, entry.count);
    }
    info->unlock(metaBuffer);

    return entry;
}

status_t Parameters::set(const String8& paramString) {
    status_t res;

    CameraParameters2 newParams(paramString);

    // TODO: Currently ignoring any changes to supposedly read-only parameters
    // such as supported preview sizes, etc. Should probably produce an error if
    // they're changed.

    /** Extract and verify new parameters */

    size_t i;

    Parameters validatedParams(*this);

    // PREVIEW_SIZE
    newParams.getPreviewSize(&validatedParams.previewWidth,
            &validatedParams.previewHeight);

    if (validatedParams.previewWidth != previewWidth ||
            validatedParams.previewHeight != previewHeight) {
        if (state >= PREVIEW) {
            ALOGE("%s: Preview size cannot be updated when preview "
                    "is active! (Currently %d x %d, requested %d x %d",
                    __FUNCTION__,
                    previewWidth, previewHeight,
                    validatedParams.previewWidth, validatedParams.previewHeight);
            return BAD_VALUE;
        }
        for (i = 0; i < availablePreviewSizes.size(); i++) {
            if ((availablePreviewSizes[i].width ==
                    validatedParams.previewWidth) &&
                (availablePreviewSizes[i].height ==
                    validatedParams.previewHeight)) break;
        }
        if (i == availablePreviewSizes.size()) {
            ALOGE("%s: Requested preview size %d x %d is not supported",
                    __FUNCTION__, validatedParams.previewWidth,
                    validatedParams.previewHeight);
            return BAD_VALUE;
        }
    }

    // RECORDING_HINT (always supported)
    validatedParams.recordingHint = boolFromString(
        newParams.get(CameraParameters::KEY_RECORDING_HINT) );
    IF_ALOGV() { // Avoid unused variable warning
        bool recordingHintChanged =
                validatedParams.recordingHint != recordingHint;
        if (recordingHintChanged) {
            ALOGV("%s: Recording hint changed to %d",
                  __FUNCTION__, validatedParams.recordingHint);
        }
    }

    // PREVIEW_FPS_RANGE

    /**
     * Use the single FPS value if it was set later than the range.
     * Otherwise, use the range value.
     */
    bool fpsUseSingleValue;
    {
        const char *fpsRange, *fpsSingle;

        fpsSingle = newParams.get(CameraParameters::KEY_PREVIEW_FRAME_RATE);
        fpsRange = newParams.get(CameraParameters::KEY_PREVIEW_FPS_RANGE);

        /**
         * Pick either the range or the single key if only one was set.
         *
         * If both are set, pick the one that has greater set order.
         */
        if (fpsRange == NULL && fpsSingle == NULL) {
            ALOGE("%s: FPS was not set. One of %s or %s must be set.",
                  __FUNCTION__, CameraParameters::KEY_PREVIEW_FRAME_RATE,
                  CameraParameters::KEY_PREVIEW_FPS_RANGE);
            return BAD_VALUE;
        } else if (fpsRange == NULL) {
            fpsUseSingleValue = true;
            ALOGV("%s: FPS range not set, using FPS single value",
                  __FUNCTION__);
        } else if (fpsSingle == NULL) {
            fpsUseSingleValue = false;
            ALOGV("%s: FPS single not set, using FPS range value",
                  __FUNCTION__);
        } else {
            int fpsKeyOrder;
            res = newParams.compareSetOrder(
                    CameraParameters::KEY_PREVIEW_FRAME_RATE,
                    CameraParameters::KEY_PREVIEW_FPS_RANGE,
                    &fpsKeyOrder);
            LOG_ALWAYS_FATAL_IF(res != OK, "Impossibly bad FPS keys");

            fpsUseSingleValue = (fpsKeyOrder > 0);

        }

        ALOGV("%s: Preview FPS value is used from '%s'",
              __FUNCTION__, fpsUseSingleValue ? "single" : "range");
    }
    newParams.getPreviewFpsRange(&validatedParams.previewFpsRange[0],
            &validatedParams.previewFpsRange[1]);

    validatedParams.previewFpsRange[0] /= kFpsToApiScale;
    validatedParams.previewFpsRange[1] /= kFpsToApiScale;

    // Ignore the FPS range if the FPS single has higher precedence
    if (!fpsUseSingleValue) {
        ALOGV("%s: Preview FPS range (%d, %d)", __FUNCTION__,
                validatedParams.previewFpsRange[0],
                validatedParams.previewFpsRange[1]);

        camera_metadata_ro_entry_t availablePreviewFpsRanges =
            staticInfo(ANDROID_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES, 2);
        for (i = 0; i < availablePreviewFpsRanges.count; i += 2) {
            if ((availablePreviewFpsRanges.data.i32[i] ==
                    validatedParams.previewFpsRange[0]) &&
                (availablePreviewFpsRanges.data.i32[i+1] ==
                    validatedParams.previewFpsRange[1]) ) {
                break;
            }
        }
        if (i == availablePreviewFpsRanges.count) {
            ALOGE("%s: Requested preview FPS range %d - %d is not supported",
                __FUNCTION__, validatedParams.previewFpsRange[0],
                    validatedParams.previewFpsRange[1]);
            return BAD_VALUE;
        }
    }

    // PREVIEW_FORMAT
    validatedParams.previewFormat =
            formatStringToEnum(newParams.getPreviewFormat());
    if (validatedParams.previewFormat != previewFormat) {
        if (state >= PREVIEW) {
            ALOGE("%s: Preview format cannot be updated when preview "
                    "is active!", __FUNCTION__);
            return BAD_VALUE;
        }
        SortedVector<int32_t> availableFormats = getAvailableOutputFormats();
        // If using flexible YUV, always support NV21/YV12. Otherwise, check
        // HAL's list.
        if (! (fastInfo.useFlexibleYuv &&
                (validatedParams.previewFormat ==
                        HAL_PIXEL_FORMAT_YCrCb_420_SP ||
                 validatedParams.previewFormat ==
                        HAL_PIXEL_FORMAT_YV12) ) ) {
            // Not using flexible YUV format, so check explicitly
            for (i = 0; i < availableFormats.size(); i++) {
                if (availableFormats[i] == validatedParams.previewFormat) break;
            }
            if (i == availableFormats.size()) {
                ALOGE("%s: Requested preview format %s (0x%x) is not supported",
                        __FUNCTION__, newParams.getPreviewFormat(),
                        validatedParams.previewFormat);
                return BAD_VALUE;
            }
        }
    }

    // PREVIEW_FRAME_RATE Deprecated
    // - Use only if the single FPS value was set later than the FPS range
    if (fpsUseSingleValue) {
        int previewFps = newParams.getPreviewFrameRate();
        ALOGV("%s: Preview FPS single value requested: %d",
              __FUNCTION__, previewFps);
        {
            camera_metadata_ro_entry_t availableFrameRates =
                staticInfo(ANDROID_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
            /**
              * If recording hint is set, find the range that encompasses
              * previewFps with the largest min index.
              *
              * If recording hint is not set, find the range with previewFps
              * with the smallest min index.
              *
              * Either way, in case of multiple ranges, break the tie by
              * selecting the smaller range.
              *
              * Always select range within 30fps if one exists.
              */

            // all ranges which have previewFps
            Vector<Range> candidateRanges;
            Vector<Range> candidateFastRanges;
            for (i = 0; i < availableFrameRates.count; i+=2) {
                Range r = {
                            availableFrameRates.data.i32[i],
                            availableFrameRates.data.i32[i+1]
                };
                if (!isFpsSupported(availablePreviewSizes,
                        HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, r.max)) {
                    continue;
                }

                if (r.min <= previewFps && previewFps <= r.max) {
                    if (r.max <= MAX_DEFAULT_FPS) {
                        candidateRanges.push(r);
                    } else {
                        candidateFastRanges.push(r);
                    }
                }
            }
            if (candidateRanges.isEmpty() && candidateFastRanges.isEmpty()) {
                ALOGE("%s: Requested preview frame rate %d is not supported",
                        __FUNCTION__, previewFps);
                return BAD_VALUE;
            }

            // most applicable range with targetFps
            Vector<Range>& ranges =
                    candidateRanges.size() > 0 ? candidateRanges : candidateFastRanges;
            Range bestRange = ranges[0];
            for (i = 1; i < ranges.size(); ++i) {
                Range r = ranges[i];
                // Find by largest minIndex in recording mode
                if (validatedParams.recordingHint) {
                    if (r.min > bestRange.min) {
                        bestRange = r;
                    }
                    else if (r.min == bestRange.min && r.max < bestRange.max) {
                        bestRange = r;
                    }
                }
                // Find by smallest minIndex in preview mode
                else {
                    if (r.min < bestRange.min) {
                        bestRange = r;
                    }
                    else if (r.min == bestRange.min && r.max < bestRange.max) {
                        bestRange = r;
                    }
                }
            }

            validatedParams.previewFpsRange[0] =
                    bestRange.min;
            validatedParams.previewFpsRange[1] =
                    bestRange.max;

            ALOGV("%s: New preview FPS range: %d, %d, recordingHint = %d",
                __FUNCTION__,
                validatedParams.previewFpsRange[0],
                validatedParams.previewFpsRange[1],
                validatedParams.recordingHint);
        }
    }

    /**
     * Update Preview FPS and Preview FPS ranges based on
     * what we actually set.
     *
     * This updates the API-visible (Camera.Parameters#getParameters) values of
     * the FPS fields, not only the internal versions.
     *
     * Order matters: The value that was set last takes precedence.
     * - If the client does a setParameters(getParameters()) we retain
     *   the same order for preview FPS.
     */
    if (!fpsUseSingleValue) {
        // Set fps single, then fps range (range wins)
        newParams.setPreviewFrameRate(
                fpsFromRange(/*min*/validatedParams.previewFpsRange[0],
                             /*max*/validatedParams.previewFpsRange[1]));
        newParams.setPreviewFpsRange(
                validatedParams.previewFpsRange[0] * kFpsToApiScale,
                validatedParams.previewFpsRange[1] * kFpsToApiScale);
    } else {
        // Set fps range, then fps single (single wins)
        newParams.setPreviewFpsRange(
                validatedParams.previewFpsRange[0] * kFpsToApiScale,
                validatedParams.previewFpsRange[1] * kFpsToApiScale);
        // Set this to the same value, but with higher priority
        newParams.setPreviewFrameRate(
                newParams.getPreviewFrameRate());
    }

    // PICTURE_SIZE
    newParams.getPictureSize(&validatedParams.pictureWidth,
            &validatedParams.pictureHeight);
    if (validatedParams.pictureWidth != pictureWidth ||
            validatedParams.pictureHeight != pictureHeight) {
        Vector<Size> availablePictureSizes = getAvailableJpegSizes();
        for (i = 0; i < availablePictureSizes.size(); i++) {
            if ((availablePictureSizes[i].width ==
                    validatedParams.pictureWidth) &&
                (availablePictureSizes[i].height ==
                    validatedParams.pictureHeight)) break;
        }
        if (i == availablePictureSizes.size()) {
            ALOGE("%s: Requested picture size %d x %d is not supported",
                    __FUNCTION__, validatedParams.pictureWidth,
                    validatedParams.pictureHeight);
            return BAD_VALUE;
        }
    }

    // JPEG_THUMBNAIL_WIDTH/HEIGHT
    validatedParams.jpegThumbSize[0] =
            newParams.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
    validatedParams.jpegThumbSize[1] =
            newParams.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
    if (validatedParams.jpegThumbSize[0] != jpegThumbSize[0] ||
            validatedParams.jpegThumbSize[1] != jpegThumbSize[1]) {
        camera_metadata_ro_entry_t availableJpegThumbSizes =
            staticInfo(ANDROID_JPEG_AVAILABLE_THUMBNAIL_SIZES);
        for (i = 0; i < availableJpegThumbSizes.count; i+=2) {
            if ((availableJpegThumbSizes.data.i32[i] ==
                    validatedParams.jpegThumbSize[0]) &&
                (availableJpegThumbSizes.data.i32[i+1] ==
                    validatedParams.jpegThumbSize[1])) break;
        }
        if (i == availableJpegThumbSizes.count) {
            ALOGE("%s: Requested JPEG thumbnail size %d x %d is not supported",
                    __FUNCTION__, validatedParams.jpegThumbSize[0],
                    validatedParams.jpegThumbSize[1]);
            return BAD_VALUE;
        }
    }

    // JPEG_THUMBNAIL_QUALITY
    int quality = newParams.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY);
    // also makes sure quality fits in uint8_t
    if (quality < 0 || quality > 100) {
        ALOGE("%s: Requested JPEG thumbnail quality %d is not supported",
                __FUNCTION__, quality);
        return BAD_VALUE;
    }
    validatedParams.jpegThumbQuality = quality;

    // JPEG_QUALITY
    quality = newParams.getInt(CameraParameters::KEY_JPEG_QUALITY);
    // also makes sure quality fits in uint8_t
    if (quality < 0 || quality > 100) {
        ALOGE("%s: Requested JPEG quality %d is not supported",
                __FUNCTION__, quality);
        return BAD_VALUE;
    }
    validatedParams.jpegQuality = quality;

    // ROTATION
    validatedParams.jpegRotation =
            newParams.getInt(CameraParameters::KEY_ROTATION);
    if (validatedParams.jpegRotation != 0 &&
            validatedParams.jpegRotation != 90 &&
            validatedParams.jpegRotation != 180 &&
            validatedParams.jpegRotation != 270) {
        ALOGE("%s: Requested picture rotation angle %d is not supported",
                __FUNCTION__, validatedParams.jpegRotation);
        return BAD_VALUE;
    }

    // GPS

    const char *gpsLatStr =
            newParams.get(CameraParameters::KEY_GPS_LATITUDE);
    if (gpsLatStr != NULL) {
        const char *gpsLongStr =
                newParams.get(CameraParameters::KEY_GPS_LONGITUDE);
        const char *gpsAltitudeStr =
                newParams.get(CameraParameters::KEY_GPS_ALTITUDE);
        const char *gpsTimeStr =
                newParams.get(CameraParameters::KEY_GPS_TIMESTAMP);
        const char *gpsProcMethodStr =
                newParams.get(CameraParameters::KEY_GPS_PROCESSING_METHOD);
        if (gpsLongStr == NULL ||
                gpsAltitudeStr == NULL ||
                gpsTimeStr == NULL ||
                gpsProcMethodStr == NULL) {
            ALOGE("%s: Incomplete set of GPS parameters provided",
                    __FUNCTION__);
            return BAD_VALUE;
        }
        char *endPtr;
        errno = 0;
        validatedParams.gpsCoordinates[0] = strtod(gpsLatStr, &endPtr);
        if (errno || endPtr == gpsLatStr) {
            ALOGE("%s: Malformed GPS latitude: %s", __FUNCTION__, gpsLatStr);
            return BAD_VALUE;
        }
        errno = 0;
        validatedParams.gpsCoordinates[1] = strtod(gpsLongStr, &endPtr);
        if (errno || endPtr == gpsLongStr) {
            ALOGE("%s: Malformed GPS longitude: %s", __FUNCTION__, gpsLongStr);
            return BAD_VALUE;
        }
        errno = 0;
        validatedParams.gpsCoordinates[2] = strtod(gpsAltitudeStr, &endPtr);
        if (errno || endPtr == gpsAltitudeStr) {
            ALOGE("%s: Malformed GPS altitude: %s", __FUNCTION__,
                    gpsAltitudeStr);
            return BAD_VALUE;
        }
        errno = 0;
        validatedParams.gpsTimestamp = strtoll(gpsTimeStr, &endPtr, 10);
        if (errno || endPtr == gpsTimeStr) {
            ALOGE("%s: Malformed GPS timestamp: %s", __FUNCTION__, gpsTimeStr);
            return BAD_VALUE;
        }
        validatedParams.gpsProcessingMethod = gpsProcMethodStr;

        validatedParams.gpsEnabled = true;
    } else {
        validatedParams.gpsEnabled = false;
    }

    // EFFECT
    validatedParams.effectMode = effectModeStringToEnum(
        newParams.get(CameraParameters::KEY_EFFECT) );
    if (validatedParams.effectMode != effectMode) {
        camera_metadata_ro_entry_t availableEffectModes =
            staticInfo(ANDROID_CONTROL_AVAILABLE_EFFECTS);
        for (i = 0; i < availableEffectModes.count; i++) {
            if (validatedParams.effectMode == availableEffectModes.data.u8[i]) break;
        }
        if (i == availableEffectModes.count) {
            ALOGE("%s: Requested effect mode \"%s\" is not supported",
                    __FUNCTION__,
                    newParams.get(CameraParameters::KEY_EFFECT) );
            return BAD_VALUE;
        }
    }

    // ANTIBANDING
    validatedParams.antibandingMode = abModeStringToEnum(
        newParams.get(CameraParameters::KEY_ANTIBANDING) );
    if (validatedParams.antibandingMode != antibandingMode) {
        camera_metadata_ro_entry_t availableAbModes =
            staticInfo(ANDROID_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES);
        for (i = 0; i < availableAbModes.count; i++) {
            if (validatedParams.antibandingMode == availableAbModes.data.u8[i])
                break;
        }
        if (i == availableAbModes.count) {
            ALOGE("%s: Requested antibanding mode \"%s\" is not supported",
                    __FUNCTION__,
                    newParams.get(CameraParameters::KEY_ANTIBANDING));
            return BAD_VALUE;
        }
    }

    // SCENE_MODE
    validatedParams.sceneMode = sceneModeStringToEnum(
        newParams.get(CameraParameters::KEY_SCENE_MODE), mDefaultSceneMode);
    if (validatedParams.sceneMode != sceneMode &&
            validatedParams.sceneMode !=
            ANDROID_CONTROL_SCENE_MODE_DISABLED) {
        camera_metadata_ro_entry_t availableSceneModes =
            staticInfo(ANDROID_CONTROL_AVAILABLE_SCENE_MODES);
        for (i = 0; i < availableSceneModes.count; i++) {
            if (validatedParams.sceneMode == availableSceneModes.data.u8[i])
                break;
        }
        if (i == availableSceneModes.count) {
            ALOGE("%s: Requested scene mode \"%s\" is not supported",
                    __FUNCTION__,
                    newParams.get(CameraParameters::KEY_SCENE_MODE));
            return BAD_VALUE;
        }
    }
    bool sceneModeSet =
            validatedParams.sceneMode != mDefaultSceneMode;

    // FLASH_MODE
    if (sceneModeSet) {
        validatedParams.flashMode =
                fastInfo.sceneModeOverrides.
                        valueFor(validatedParams.sceneMode).flashMode;
    } else {
        validatedParams.flashMode = FLASH_MODE_INVALID;
    }
    if (validatedParams.flashMode == FLASH_MODE_INVALID) {
        validatedParams.flashMode = flashModeStringToEnum(
            newParams.get(CameraParameters::KEY_FLASH_MODE) );
    }

    if (validatedParams.flashMode != flashMode) {
        camera_metadata_ro_entry_t flashAvailable =
            staticInfo(ANDROID_FLASH_INFO_AVAILABLE, 1, 1);
        bool isFlashAvailable =
                flashAvailable.data.u8[0] == ANDROID_FLASH_INFO_AVAILABLE_TRUE;
        if (!isFlashAvailable &&
                validatedParams.flashMode != Parameters::FLASH_MODE_OFF) {
            ALOGE("%s: Requested flash mode \"%s\" is not supported: "
                    "No flash on device", __FUNCTION__,
                    newParams.get(CameraParameters::KEY_FLASH_MODE));
            return BAD_VALUE;
        } else if (validatedParams.flashMode == Parameters::FLASH_MODE_RED_EYE) {
            camera_metadata_ro_entry_t availableAeModes =
                staticInfo(ANDROID_CONTROL_AE_AVAILABLE_MODES);
            for (i = 0; i < availableAeModes.count; i++) {
                if (validatedParams.flashMode == availableAeModes.data.u8[i])
                    break;
            }
            if (i == availableAeModes.count) {
                ALOGE("%s: Requested flash mode \"%s\" is not supported",
                        __FUNCTION__,
                        newParams.get(CameraParameters::KEY_FLASH_MODE));
                return BAD_VALUE;
            }
        } else if (validatedParams.flashMode == -1) {
            ALOGE("%s: Requested flash mode \"%s\" is unknown",
                    __FUNCTION__,
                    newParams.get(CameraParameters::KEY_FLASH_MODE));
            return BAD_VALUE;
        }
        // Update in case of override, but only if flash is supported
        if (isFlashAvailable) {
            newParams.set(CameraParameters::KEY_FLASH_MODE,
                    flashModeEnumToString(validatedParams.flashMode));
        }
    }

    // WHITE_BALANCE
    if (sceneModeSet) {
        validatedParams.wbMode =
                fastInfo.sceneModeOverrides.
                        valueFor(validatedParams.sceneMode).wbMode;
    } else {
        validatedParams.wbMode = ANDROID_CONTROL_AWB_MODE_OFF;
    }
    if (validatedParams.wbMode == ANDROID_CONTROL_AWB_MODE_OFF) {
        validatedParams.wbMode = wbModeStringToEnum(
            newParams.get(CameraParameters::KEY_WHITE_BALANCE) );
    }
    if (validatedParams.wbMode != wbMode) {
        camera_metadata_ro_entry_t availableWbModes =
            staticInfo(ANDROID_CONTROL_AWB_AVAILABLE_MODES, 0, 0, false);
        for (i = 0; i < availableWbModes.count; i++) {
            if (validatedParams.wbMode == availableWbModes.data.u8[i]) break;
        }
        if (i == availableWbModes.count) {
            ALOGE("%s: Requested white balance mode %s is not supported",
                    __FUNCTION__,
                    newParams.get(CameraParameters::KEY_WHITE_BALANCE));
            return BAD_VALUE;
        }
        // Update in case of override
        newParams.set(CameraParameters::KEY_WHITE_BALANCE,
                wbModeEnumToString(validatedParams.wbMode));
    }

    // FOCUS_MODE
    if (sceneModeSet) {
        validatedParams.focusMode =
                fastInfo.sceneModeOverrides.
                        valueFor(validatedParams.sceneMode).focusMode;
    } else {
        validatedParams.focusMode = FOCUS_MODE_INVALID;
    }
    if (validatedParams.focusMode == FOCUS_MODE_INVALID) {
        validatedParams.focusMode = focusModeStringToEnum(
                newParams.get(CameraParameters::KEY_FOCUS_MODE) );
    }
    if (validatedParams.focusMode != focusMode) {
        validatedParams.currentAfTriggerId = -1;
        if (validatedParams.focusMode != Parameters::FOCUS_MODE_FIXED) {
            camera_metadata_ro_entry_t minFocusDistance =
                staticInfo(ANDROID_LENS_INFO_MINIMUM_FOCUS_DISTANCE, 0, 0,
                        false);
            if (minFocusDistance.count && minFocusDistance.data.f[0] == 0) {
                ALOGE("%s: Requested focus mode \"%s\" is not available: "
                        "fixed focus lens",
                        __FUNCTION__,
                        newParams.get(CameraParameters::KEY_FOCUS_MODE));
                return BAD_VALUE;
            } else if (validatedParams.focusMode !=
                    Parameters::FOCUS_MODE_INFINITY) {
                camera_metadata_ro_entry_t availableFocusModes =
                    staticInfo(ANDROID_CONTROL_AF_AVAILABLE_MODES);
                for (i = 0; i < availableFocusModes.count; i++) {
                    if (validatedParams.focusMode ==
                            availableFocusModes.data.u8[i]) break;
                }
                if (i == availableFocusModes.count) {
                    ALOGE("%s: Requested focus mode \"%s\" is not supported",
                            __FUNCTION__,
                            newParams.get(CameraParameters::KEY_FOCUS_MODE));
                    return BAD_VALUE;
                }
            }
        }
        validatedParams.focusState = ANDROID_CONTROL_AF_STATE_INACTIVE;
        // Always reset shadow focus mode to avoid reverting settings
        validatedParams.shadowFocusMode = FOCUS_MODE_INVALID;
        // Update in case of override
        newParams.set(CameraParameters::KEY_FOCUS_MODE,
                focusModeEnumToString(validatedParams.focusMode));
    } else {
        validatedParams.currentAfTriggerId = currentAfTriggerId;
    }

    // FOCUS_AREAS
    res = parseAreas(newParams.get(CameraParameters::KEY_FOCUS_AREAS),
            &validatedParams.focusingAreas);
    size_t maxAfRegions = (size_t)staticInfo(ANDROID_CONTROL_MAX_REGIONS,
              Parameters::NUM_REGION, Parameters::NUM_REGION).
              data.i32[Parameters::REGION_AF];
    if (res == OK) res = validateAreas(validatedParams.focusingAreas,
            maxAfRegions, AREA_KIND_FOCUS);
    if (res != OK) {
        ALOGE("%s: Requested focus areas are malformed: %s",
                __FUNCTION__, newParams.get(CameraParameters::KEY_FOCUS_AREAS));
        return BAD_VALUE;
    }

    // EXPOSURE_COMPENSATION
    validatedParams.exposureCompensation =
        newParams.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
    camera_metadata_ro_entry_t exposureCompensationRange =
        staticInfo(ANDROID_CONTROL_AE_COMPENSATION_RANGE);
    if ((validatedParams.exposureCompensation <
            exposureCompensationRange.data.i32[0]) ||
        (validatedParams.exposureCompensation >
            exposureCompensationRange.data.i32[1])) {
        ALOGE("%s: Requested exposure compensation index is out of bounds: %d",
                __FUNCTION__, validatedParams.exposureCompensation);
        return BAD_VALUE;
    }

    if (autoExposureLockAvailable) {
        validatedParams.autoExposureLock = boolFromString(
            newParams.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK));
    } else if (nullptr !=
            newParams.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK)){
        ALOGE("%s: Requested auto exposure lock is not supported",
              __FUNCTION__);
        return BAD_VALUE;
    }

    if (autoWhiteBalanceLockAvailable) {
        validatedParams.autoWhiteBalanceLock = boolFromString(
                newParams.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK));
    } else if (nullptr !=
           newParams.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK)) {
        ALOGE("%s: Requested auto whitebalance lock is not supported",
              __FUNCTION__);
        return BAD_VALUE;
    }

    // METERING_AREAS
    size_t maxAeRegions = (size_t)staticInfo(ANDROID_CONTROL_MAX_REGIONS,
            Parameters::NUM_REGION, Parameters::NUM_REGION).
            data.i32[Parameters::REGION_AE];
    res = parseAreas(newParams.get(CameraParameters::KEY_METERING_AREAS),
            &validatedParams.meteringAreas);
    if (res == OK) {
        res = validateAreas(validatedParams.meteringAreas, maxAeRegions,
                            AREA_KIND_METERING);
    }
    if (res != OK) {
        ALOGE("%s: Requested metering areas are malformed: %s",
                __FUNCTION__,
                newParams.get(CameraParameters::KEY_METERING_AREAS));
        return BAD_VALUE;
    }

    // ZOOM
    if (zoomAvailable) {
        validatedParams.zoom = newParams.getInt(CameraParameters::KEY_ZOOM);
        if (validatedParams.zoom < 0
                    || validatedParams.zoom >= (int)NUM_ZOOM_STEPS) {
            ALOGE("%s: Requested zoom level %d is not supported",
                    __FUNCTION__, validatedParams.zoom);
            return BAD_VALUE;
        }
    }

    // VIDEO_SIZE
    newParams.getVideoSize(&validatedParams.videoWidth,
            &validatedParams.videoHeight);
    if (validatedParams.videoWidth != videoWidth ||
            validatedParams.videoHeight != videoHeight) {
        if (state == RECORD) {
            ALOGW("%s: Video size cannot be updated (from %d x %d to %d x %d)"
                    " when recording is active! Ignore the size update!",
                    __FUNCTION__, videoWidth, videoHeight, validatedParams.videoWidth,
                    validatedParams.videoHeight);
            validatedParams.videoWidth = videoWidth;
            validatedParams.videoHeight = videoHeight;
            newParams.setVideoSize(videoWidth, videoHeight);
        } else {
            for (i = 0; i < availableVideoSizes.size(); i++) {
                if ((availableVideoSizes[i].width ==
                        validatedParams.videoWidth) &&
                    (availableVideoSizes[i].height ==
                        validatedParams.videoHeight)) break;
            }
            if (i == availableVideoSizes.size()) {
                ALOGE("%s: Requested video size %d x %d is not supported",
                        __FUNCTION__, validatedParams.videoWidth,
                        validatedParams.videoHeight);
                return BAD_VALUE;
            }
        }
    }

    // VIDEO_STABILIZATION
    validatedParams.videoStabilization = boolFromString(
        newParams.get(CameraParameters::KEY_VIDEO_STABILIZATION) );
    camera_metadata_ro_entry_t availableVideoStabilizationModes =
        staticInfo(ANDROID_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES, 0, 0,
                false);
    if (validatedParams.videoStabilization &&
            availableVideoStabilizationModes.count == 1) {
        ALOGE("%s: Video stabilization not supported", __FUNCTION__);
    }

    /** Update internal parameters */

    *this = validatedParams;
    updateOverriddenJpegSize();

    /** Update external parameters calculated from the internal ones */

    // HORIZONTAL/VERTICAL FIELD OF VIEW
    float horizFov, vertFov;
    res = calculatePictureFovs(&horizFov, &vertFov);
    if (res != OK) {
        ALOGE("%s: Can't calculate FOVs", __FUNCTION__);
        // continue so parameters are at least consistent
    }
    newParams.setFloat(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE,
            horizFov);
    newParams.setFloat(CameraParameters::KEY_VERTICAL_VIEW_ANGLE,
            vertFov);
    ALOGV("Current still picture FOV: %f x %f deg", horizFov, vertFov);

    // Need to flatten again in case of overrides
    paramsFlattened = newParams.flatten();
    params = newParams;

    slowJpegMode = false;
    Size pictureSize = { pictureWidth, pictureHeight };
    int64_t minFrameDurationNs = getJpegStreamMinFrameDurationNs(pictureSize);
    if (previewFpsRange[1] > 1e9/minFrameDurationNs + FPS_MARGIN) {
        slowJpegMode = true;
    }
    if (isDeviceZslSupported || slowJpegMode ||
            property_get_bool("camera.disable_zsl_mode", false)) {
        allowZslMode = false;
    } else {
        allowZslMode = isZslReprocessPresent;
    }
    ALOGV("%s: allowZslMode: %d slowJpegMode %d", __FUNCTION__, allowZslMode, slowJpegMode);

    return OK;
}

status_t Parameters::updateRequest(CameraMetadata *request) const {
    ATRACE_CALL();
    status_t res;

    /**
     * Mixin default important security values
     * - android.led.transmit = defaulted ON
     */
    camera_metadata_ro_entry_t entry = staticInfo(ANDROID_LED_AVAILABLE_LEDS,
                                                  /*minimumCount*/0,
                                                  /*maximumCount*/0,
                                                  /*required*/false);
    for(size_t i = 0; i < entry.count; ++i) {
        uint8_t led = entry.data.u8[i];

        switch(led) {
            // Transmit LED is unconditionally on when using
            // the android.hardware.Camera API
            case ANDROID_LED_AVAILABLE_LEDS_TRANSMIT: {
                uint8_t transmitDefault = ANDROID_LED_TRANSMIT_ON;
                res = request->update(ANDROID_LED_TRANSMIT,
                                      &transmitDefault, 1);
                if (res != OK) return res;
                break;
            }
        }
    }

    /**
     * Construct metadata from parameters
     */

    uint8_t metadataMode = ANDROID_REQUEST_METADATA_MODE_FULL;
    res = request->update(ANDROID_REQUEST_METADATA_MODE,
            &metadataMode, 1);
    if (res != OK) return res;

    camera_metadata_entry_t intent =
            request->find(ANDROID_CONTROL_CAPTURE_INTENT);

    if (intent.count == 0) return BAD_VALUE;

    uint8_t distortionMode = ANDROID_DISTORTION_CORRECTION_MODE_OFF;
    if (intent.data.u8[0] == ANDROID_CONTROL_CAPTURE_INTENT_STILL_CAPTURE) {
        res = request->update(ANDROID_CONTROL_AE_TARGET_FPS_RANGE,
                fastInfo.bestStillCaptureFpsRange, 2);
        distortionMode = ANDROID_DISTORTION_CORRECTION_MODE_HIGH_QUALITY;
    } else {
        res = request->update(ANDROID_CONTROL_AE_TARGET_FPS_RANGE,
                previewFpsRange, 2);
        distortionMode = ANDROID_DISTORTION_CORRECTION_MODE_FAST;
    }
    if (res != OK) return res;

    if (isDistortionCorrectionSupported) {
        res = request->update(ANDROID_DISTORTION_CORRECTION_MODE,
                &distortionMode, 1);
        if (res != OK) return res;
    }

    if (autoWhiteBalanceLockAvailable) {
        uint8_t reqWbLock = autoWhiteBalanceLock ?
                ANDROID_CONTROL_AWB_LOCK_ON : ANDROID_CONTROL_AWB_LOCK_OFF;
        res = request->update(ANDROID_CONTROL_AWB_LOCK,
                &reqWbLock, 1);
    }

    res = request->update(ANDROID_CONTROL_EFFECT_MODE,
            &effectMode, 1);
    if (res != OK) return res;
    res = request->update(ANDROID_CONTROL_AE_ANTIBANDING_MODE,
            &antibandingMode, 1);
    if (res != OK) return res;

    // android.hardware.Camera requires that when face detect is enabled, the
    // camera is in a face-priority mode. HAL3.x splits this into separate parts
    // (face detection statistics and face priority scene mode). Map from other
    // to the other.
    bool sceneModeActive =
            sceneMode != (uint8_t)ANDROID_CONTROL_SCENE_MODE_DISABLED;
    uint8_t reqControlMode = ANDROID_CONTROL_MODE_AUTO;
    if (enableFaceDetect || sceneModeActive) {
        reqControlMode = ANDROID_CONTROL_MODE_USE_SCENE_MODE;
    }
    res = request->update(ANDROID_CONTROL_MODE,
            &reqControlMode, 1);
    if (res != OK) return res;

    uint8_t reqSceneMode =
            sceneModeActive ? sceneMode :
            enableFaceDetect ? (uint8_t)ANDROID_CONTROL_SCENE_MODE_FACE_PRIORITY :
            mDefaultSceneMode;
    res = request->update(ANDROID_CONTROL_SCENE_MODE,
            &reqSceneMode, 1);
    if (res != OK) return res;

    uint8_t reqFlashMode = ANDROID_FLASH_MODE_OFF;
    uint8_t reqAeMode = ANDROID_CONTROL_AE_MODE_OFF;
    switch (flashMode) {
        case Parameters::FLASH_MODE_OFF:
            reqAeMode = ANDROID_CONTROL_AE_MODE_ON; break;
        case Parameters::FLASH_MODE_AUTO:
            reqAeMode = ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH; break;
        case Parameters::FLASH_MODE_ON:
            reqAeMode = ANDROID_CONTROL_AE_MODE_ON_ALWAYS_FLASH; break;
        case Parameters::FLASH_MODE_TORCH:
            reqAeMode = ANDROID_CONTROL_AE_MODE_ON;
            reqFlashMode = ANDROID_FLASH_MODE_TORCH;
            break;
        case Parameters::FLASH_MODE_RED_EYE:
            reqAeMode = ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE; break;
        default:
            ALOGE("%s: Camera %d: Unknown flash mode %d", __FUNCTION__,
                    cameraId, flashMode);
                return BAD_VALUE;
    }
    res = request->update(ANDROID_FLASH_MODE,
            &reqFlashMode, 1);
    if (res != OK) return res;
    res = request->update(ANDROID_CONTROL_AE_MODE,
            &reqAeMode, 1);
    if (res != OK) return res;

    if (autoExposureLockAvailable) {
        uint8_t reqAeLock = autoExposureLock ?
                ANDROID_CONTROL_AE_LOCK_ON : ANDROID_CONTROL_AE_LOCK_OFF;
        res = request->update(ANDROID_CONTROL_AE_LOCK,
                &reqAeLock, 1);
        if (res != OK) return res;
    }

    res = request->update(ANDROID_CONTROL_AWB_MODE,
            &wbMode, 1);
    if (res != OK) return res;

    float reqFocusDistance = 0; // infinity focus in diopters
    uint8_t reqFocusMode = ANDROID_CONTROL_AF_MODE_OFF;
    switch (focusMode) {
        case Parameters::FOCUS_MODE_AUTO:
        case Parameters::FOCUS_MODE_MACRO:
        case Parameters::FOCUS_MODE_CONTINUOUS_VIDEO:
        case Parameters::FOCUS_MODE_CONTINUOUS_PICTURE:
        case Parameters::FOCUS_MODE_EDOF:
            reqFocusMode = focusMode;
            break;
        case Parameters::FOCUS_MODE_INFINITY:
        case Parameters::FOCUS_MODE_FIXED:
            reqFocusMode = ANDROID_CONTROL_AF_MODE_OFF;
            break;
        default:
                ALOGE("%s: Camera %d: Unknown focus mode %d", __FUNCTION__,
                        cameraId, focusMode);
                return BAD_VALUE;
    }
    res = request->update(ANDROID_LENS_FOCUS_DISTANCE,
            &reqFocusDistance, 1);
    if (res != OK) return res;
    res = request->update(ANDROID_CONTROL_AF_MODE,
            &reqFocusMode, 1);
    if (res != OK) return res;

    size_t reqFocusingAreasSize = focusingAreas.size() * 5;
    int32_t *reqFocusingAreas = new int32_t[reqFocusingAreasSize];
    for (size_t i = 0, j = 0; i < reqFocusingAreasSize; i += 5, j++) {
        if (focusingAreas[j].weight != 0) {
            reqFocusingAreas[i + 0] =
                    normalizedXToArray(focusingAreas[j].left);
            reqFocusingAreas[i + 1] =
                    normalizedYToArray(focusingAreas[j].top);
            reqFocusingAreas[i + 2] =
                    normalizedXToArray(focusingAreas[j].right);
            reqFocusingAreas[i + 3] =
                    normalizedYToArray(focusingAreas[j].bottom);
        } else {
            reqFocusingAreas[i + 0] = 0;
            reqFocusingAreas[i + 1] = 0;
            reqFocusingAreas[i + 2] = 0;
            reqFocusingAreas[i + 3] = 0;
        }
        reqFocusingAreas[i + 4] = focusingAreas[j].weight;
    }
    res = request->update(ANDROID_CONTROL_AF_REGIONS,
            reqFocusingAreas, reqFocusingAreasSize);
    if (res != OK) return res;
    delete[] reqFocusingAreas;

    res = request->update(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION,
            &exposureCompensation, 1);
    if (res != OK) return res;

    size_t reqMeteringAreasSize = meteringAreas.size() * 5;
    int32_t *reqMeteringAreas = new int32_t[reqMeteringAreasSize];
    for (size_t i = 0, j = 0; i < reqMeteringAreasSize; i += 5, j++) {
        if (meteringAreas[j].weight != 0) {
            reqMeteringAreas[i + 0] =
                normalizedXToArray(meteringAreas[j].left);
            reqMeteringAreas[i + 1] =
                normalizedYToArray(meteringAreas[j].top);
            reqMeteringAreas[i + 2] =
                normalizedXToArray(meteringAreas[j].right);
            reqMeteringAreas[i + 3] =
                normalizedYToArray(meteringAreas[j].bottom);
            // Requested size may be zero by rounding error with/without zooming.
            // The ae regions should be at least 1 if metering width/height is not zero.
            if (reqMeteringAreas[i + 0] == reqMeteringAreas[i + 2]) {
                reqMeteringAreas[i + 2]++;
            }
            if (reqMeteringAreas[i + 1] == reqMeteringAreas[i + 3]) {
                reqMeteringAreas[i + 3]++;
            }
        } else {
            reqMeteringAreas[i + 0] = 0;
            reqMeteringAreas[i + 1] = 0;
            reqMeteringAreas[i + 2] = 0;
            reqMeteringAreas[i + 3] = 0;
        }
        reqMeteringAreas[i + 4] = meteringAreas[j].weight;
    }
    res = request->update(ANDROID_CONTROL_AE_REGIONS,
            reqMeteringAreas, reqMeteringAreasSize);
    if (res != OK) return res;

    // Set awb regions to be the same as the metering regions if allowed
    size_t maxAwbRegions = (size_t)staticInfo(ANDROID_CONTROL_MAX_REGIONS,
            Parameters::NUM_REGION, Parameters::NUM_REGION).
            data.i32[Parameters::REGION_AWB];
    if (maxAwbRegions > 0) {
        if (maxAwbRegions >= meteringAreas.size()) {
            res = request->update(ANDROID_CONTROL_AWB_REGIONS,
                    reqMeteringAreas, reqMeteringAreasSize);
        } else {
            // Ensure the awb regions are zeroed if the region count is too high.
            int32_t zeroedAwbAreas[5] = {0, 0, 0, 0, 0};
            res = request->update(ANDROID_CONTROL_AWB_REGIONS,
                    zeroedAwbAreas, sizeof(zeroedAwbAreas)/sizeof(int32_t));
        }
        if (res != OK) return res;
    }

    delete[] reqMeteringAreas;

    CropRegion crop = calculateCropRegion(/*previewOnly*/ false);
    int32_t reqCropRegion[4] = {
        static_cast<int32_t>(crop.left),
        static_cast<int32_t>(crop.top),
        static_cast<int32_t>(crop.width),
        static_cast<int32_t>(crop.height)
    };
    res = request->update(ANDROID_SCALER_CROP_REGION,
            reqCropRegion, 4);
    if (res != OK) return res;

    uint8_t reqVstabMode = videoStabilization ?
            ANDROID_CONTROL_VIDEO_STABILIZATION_MODE_ON :
            ANDROID_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    res = request->update(ANDROID_CONTROL_VIDEO_STABILIZATION_MODE,
            &reqVstabMode, 1);
    if (res != OK) return res;

    uint8_t reqFaceDetectMode = enableFaceDetect ?
            fastInfo.bestFaceDetectMode :
            (uint8_t)ANDROID_STATISTICS_FACE_DETECT_MODE_OFF;
    res = request->update(ANDROID_STATISTICS_FACE_DETECT_MODE,
            &reqFaceDetectMode, 1);
    if (res != OK) return res;

    return OK;
}

status_t Parameters::updateRequestJpeg(CameraMetadata *request) const {
    status_t res;

    res = request->update(ANDROID_JPEG_THUMBNAIL_SIZE,
            jpegThumbSize, 2);
    if (res != OK) return res;
    res = request->update(ANDROID_JPEG_THUMBNAIL_QUALITY,
            &jpegThumbQuality, 1);
    if (res != OK) return res;
    res = request->update(ANDROID_JPEG_QUALITY,
            &jpegQuality, 1);
    if (res != OK) return res;
    res = request->update(
            ANDROID_JPEG_ORIENTATION,
            &jpegRotation, 1);
    if (res != OK) return res;

    if (gpsEnabled) {
        res = request->update(
                ANDROID_JPEG_GPS_COORDINATES,
                gpsCoordinates, 3);
        if (res != OK) return res;
        res = request->update(
                ANDROID_JPEG_GPS_TIMESTAMP,
                &gpsTimestamp, 1);
        if (res != OK) return res;
        res = request->update(
                ANDROID_JPEG_GPS_PROCESSING_METHOD,
                gpsProcessingMethod);
        if (res != OK) return res;
    } else {
        res = request->erase(ANDROID_JPEG_GPS_COORDINATES);
        if (res != OK) return res;
        res = request->erase(ANDROID_JPEG_GPS_TIMESTAMP);
        if (res != OK) return res;
        res = request->erase(ANDROID_JPEG_GPS_PROCESSING_METHOD);
        if (res != OK) return res;
    }
    return OK;
}

status_t Parameters::overrideJpegSizeByVideoSize() {
    if (pictureSizeOverriden) {
        ALOGV("Picture size has been overridden. Skip overriding");
        return OK;
    }

    pictureSizeOverriden = true;
    pictureWidthLastSet = pictureWidth;
    pictureHeightLastSet = pictureHeight;
    pictureWidth = videoWidth;
    pictureHeight = videoHeight;
    // This change of picture size is invisible to app layer.
    // Do not update app visible params
    return OK;
}

status_t Parameters::updateOverriddenJpegSize() {
    if (!pictureSizeOverriden) {
        ALOGV("Picture size has not been overridden. Skip checking");
        return OK;
    }

    pictureWidthLastSet = pictureWidth;
    pictureHeightLastSet = pictureHeight;

    if (pictureWidth <= videoWidth && pictureHeight <= videoHeight) {
        // Picture size is now smaller than video size. No need to override anymore
        return recoverOverriddenJpegSize();
    }

    pictureWidth = videoWidth;
    pictureHeight = videoHeight;

    return OK;
}

status_t Parameters::recoverOverriddenJpegSize() {
    if (!pictureSizeOverriden) {
        ALOGV("Picture size has not been overridden. Skip recovering");
        return OK;
    }
    pictureSizeOverriden = false;
    pictureWidth = pictureWidthLastSet;
    pictureHeight = pictureHeightLastSet;
    return OK;
}

bool Parameters::isJpegSizeOverridden() {
    return pictureSizeOverriden;
}

bool Parameters::useZeroShutterLag() const {
    // If ZSL mode is disabled, don't use it
    if (!allowZslMode) return false;
    // If recording hint is enabled, don't do ZSL
    if (recordingHint) return false;
    // If still capture size is no bigger than preview or video size,
    // don't do ZSL
    if (pictureWidth <= previewWidth || pictureHeight <= previewHeight ||
            pictureWidth <= videoWidth || pictureHeight <= videoHeight) {
        return false;
    }
    // If still capture size is less than quarter of max, don't do ZSL
    if ((pictureWidth * pictureHeight) <
            (fastInfo.maxJpegSize.width * fastInfo.maxJpegSize.height / 4) ) {
        return false;
    }
    return true;
}

status_t Parameters::getDefaultFocalLength(CameraDeviceBase *device) {
    if (device == nullptr) {
        ALOGE("%s: Camera device is nullptr", __FUNCTION__);
        return BAD_VALUE;
    }

    camera_metadata_ro_entry_t hwLevel = staticInfo(ANDROID_INFO_SUPPORTED_HARDWARE_LEVEL);
    if (!hwLevel.count) return NO_INIT;
    fastInfo.isExternalCamera =
            hwLevel.data.u8[0] == ANDROID_INFO_SUPPORTED_HARDWARE_LEVEL_EXTERNAL;

    camera_metadata_ro_entry_t availableFocalLengths =
        staticInfo(ANDROID_LENS_INFO_AVAILABLE_FOCAL_LENGTHS, 0, 0, /*required*/false);
    if (!availableFocalLengths.count && !fastInfo.isExternalCamera) return NO_INIT;

    // Find focal length in PREVIEW template to use as default focal length.
    if (fastInfo.isExternalCamera) {
        fastInfo.defaultFocalLength = -1.0;
    } else {
        // Find smallest (widest-angle) focal length to use as basis of still
        // picture FOV reporting.
        fastInfo.defaultFocalLength = availableFocalLengths.data.f[0];
        for (size_t i = 1; i < availableFocalLengths.count; i++) {
            if (fastInfo.defaultFocalLength > availableFocalLengths.data.f[i]) {
                fastInfo.defaultFocalLength = availableFocalLengths.data.f[i];
            }
        }

        // Use focal length in preview template if it exists
        CameraMetadata previewTemplate;
        status_t res = device->createDefaultRequest(CAMERA3_TEMPLATE_PREVIEW, &previewTemplate);
        if (res != OK) {
            ALOGE("%s: Failed to create default PREVIEW request: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
            return res;
        }
        camera_metadata_entry entry = previewTemplate.find(ANDROID_LENS_FOCAL_LENGTH);
        if (entry.count != 0) {
            fastInfo.defaultFocalLength = entry.data.f[0];
        }
    }
    return OK;
}

const char* Parameters::getStateName(State state) {
#define CASE_ENUM_TO_CHAR(x) case x: return(#x); break;
    switch(state) {
        CASE_ENUM_TO_CHAR(DISCONNECTED)
        CASE_ENUM_TO_CHAR(STOPPED)
        CASE_ENUM_TO_CHAR(WAITING_FOR_PREVIEW_WINDOW)
        CASE_ENUM_TO_CHAR(PREVIEW)
        CASE_ENUM_TO_CHAR(RECORD)
        CASE_ENUM_TO_CHAR(STILL_CAPTURE)
        CASE_ENUM_TO_CHAR(VIDEO_SNAPSHOT)
        default:
            return "Unknown state!";
            break;
    }
#undef CASE_ENUM_TO_CHAR
}

int Parameters::formatStringToEnum(const char *format) {
    return CameraParameters::previewFormatToEnum(format);
}

const char* Parameters::formatEnumToString(int format) {
    const char *fmt;
    switch(format) {
        case HAL_PIXEL_FORMAT_YCbCr_422_SP: // NV16
            fmt = CameraParameters::PIXEL_FORMAT_YUV422SP;
            break;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP: // NV21
            fmt = CameraParameters::PIXEL_FORMAT_YUV420SP;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_422_I: // YUY2
            fmt = CameraParameters::PIXEL_FORMAT_YUV422I;
            break;
        case HAL_PIXEL_FORMAT_YV12:        // YV12
            fmt = CameraParameters::PIXEL_FORMAT_YUV420P;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:     // RGB565
            fmt = CameraParameters::PIXEL_FORMAT_RGB565;
            break;
        case HAL_PIXEL_FORMAT_RGBA_8888:   // RGBA8888
            fmt = CameraParameters::PIXEL_FORMAT_RGBA8888;
            break;
        case HAL_PIXEL_FORMAT_RAW16:
            ALOGW("Raw sensor preview format requested.");
            fmt = CameraParameters::PIXEL_FORMAT_BAYER_RGGB;
            break;
        default:
            ALOGE("%s: Unknown preview format: %x",
                    __FUNCTION__,  format);
            fmt = NULL;
            break;
    }
    return fmt;
}

int Parameters::wbModeStringToEnum(const char *wbMode) {
    return
        !wbMode ?
            ANDROID_CONTROL_AWB_MODE_AUTO :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_AUTO) ?
            ANDROID_CONTROL_AWB_MODE_AUTO :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_INCANDESCENT) ?
            ANDROID_CONTROL_AWB_MODE_INCANDESCENT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_FLUORESCENT) ?
            ANDROID_CONTROL_AWB_MODE_FLUORESCENT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT) ?
            ANDROID_CONTROL_AWB_MODE_WARM_FLUORESCENT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_DAYLIGHT) ?
            ANDROID_CONTROL_AWB_MODE_DAYLIGHT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT) ?
            ANDROID_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_TWILIGHT) ?
            ANDROID_CONTROL_AWB_MODE_TWILIGHT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_SHADE) ?
            ANDROID_CONTROL_AWB_MODE_SHADE :
        -1;
}

const char* Parameters::wbModeEnumToString(uint8_t wbMode) {
    switch (wbMode) {
        case ANDROID_CONTROL_AWB_MODE_AUTO:
            return CameraParameters::WHITE_BALANCE_AUTO;
        case ANDROID_CONTROL_AWB_MODE_INCANDESCENT:
            return CameraParameters::WHITE_BALANCE_INCANDESCENT;
        case ANDROID_CONTROL_AWB_MODE_FLUORESCENT:
            return CameraParameters::WHITE_BALANCE_FLUORESCENT;
        case ANDROID_CONTROL_AWB_MODE_WARM_FLUORESCENT:
            return CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT;
        case ANDROID_CONTROL_AWB_MODE_DAYLIGHT:
            return CameraParameters::WHITE_BALANCE_DAYLIGHT;
        case ANDROID_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT:
            return CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT;
        case ANDROID_CONTROL_AWB_MODE_TWILIGHT:
            return CameraParameters::WHITE_BALANCE_TWILIGHT;
        case ANDROID_CONTROL_AWB_MODE_SHADE:
            return CameraParameters::WHITE_BALANCE_SHADE;
        default:
            ALOGE("%s: Unknown AWB mode enum: %d",
                    __FUNCTION__, wbMode);
            return "unknown";
    }
}

int Parameters::effectModeStringToEnum(const char *effectMode) {
    return
        !effectMode ?
            ANDROID_CONTROL_EFFECT_MODE_OFF :
        !strcmp(effectMode, CameraParameters::EFFECT_NONE) ?
            ANDROID_CONTROL_EFFECT_MODE_OFF :
        !strcmp(effectMode, CameraParameters::EFFECT_MONO) ?
            ANDROID_CONTROL_EFFECT_MODE_MONO :
        !strcmp(effectMode, CameraParameters::EFFECT_NEGATIVE) ?
            ANDROID_CONTROL_EFFECT_MODE_NEGATIVE :
        !strcmp(effectMode, CameraParameters::EFFECT_SOLARIZE) ?
            ANDROID_CONTROL_EFFECT_MODE_SOLARIZE :
        !strcmp(effectMode, CameraParameters::EFFECT_SEPIA) ?
            ANDROID_CONTROL_EFFECT_MODE_SEPIA :
        !strcmp(effectMode, CameraParameters::EFFECT_POSTERIZE) ?
            ANDROID_CONTROL_EFFECT_MODE_POSTERIZE :
        !strcmp(effectMode, CameraParameters::EFFECT_WHITEBOARD) ?
            ANDROID_CONTROL_EFFECT_MODE_WHITEBOARD :
        !strcmp(effectMode, CameraParameters::EFFECT_BLACKBOARD) ?
            ANDROID_CONTROL_EFFECT_MODE_BLACKBOARD :
        !strcmp(effectMode, CameraParameters::EFFECT_AQUA) ?
            ANDROID_CONTROL_EFFECT_MODE_AQUA :
        -1;
}

int Parameters::abModeStringToEnum(const char *abMode) {
    return
        !abMode ?
            ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO :
        !strcmp(abMode, CameraParameters::ANTIBANDING_AUTO) ?
            ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO :
        !strcmp(abMode, CameraParameters::ANTIBANDING_OFF) ?
            ANDROID_CONTROL_AE_ANTIBANDING_MODE_OFF :
        !strcmp(abMode, CameraParameters::ANTIBANDING_50HZ) ?
            ANDROID_CONTROL_AE_ANTIBANDING_MODE_50HZ :
        !strcmp(abMode, CameraParameters::ANTIBANDING_60HZ) ?
            ANDROID_CONTROL_AE_ANTIBANDING_MODE_60HZ :
        -1;
}

int Parameters::sceneModeStringToEnum(const char *sceneMode, uint8_t defaultSceneMode) {
    return
        !sceneMode ?
            defaultSceneMode :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_AUTO) ?
            defaultSceneMode :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_ACTION) ?
            ANDROID_CONTROL_SCENE_MODE_ACTION :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_PORTRAIT) ?
            ANDROID_CONTROL_SCENE_MODE_PORTRAIT :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_LANDSCAPE) ?
            ANDROID_CONTROL_SCENE_MODE_LANDSCAPE :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_NIGHT) ?
            ANDROID_CONTROL_SCENE_MODE_NIGHT :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_NIGHT_PORTRAIT) ?
            ANDROID_CONTROL_SCENE_MODE_NIGHT_PORTRAIT :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_THEATRE) ?
            ANDROID_CONTROL_SCENE_MODE_THEATRE :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_BEACH) ?
            ANDROID_CONTROL_SCENE_MODE_BEACH :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_SNOW) ?
            ANDROID_CONTROL_SCENE_MODE_SNOW :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_SUNSET) ?
            ANDROID_CONTROL_SCENE_MODE_SUNSET :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_STEADYPHOTO) ?
            ANDROID_CONTROL_SCENE_MODE_STEADYPHOTO :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_FIREWORKS) ?
            ANDROID_CONTROL_SCENE_MODE_FIREWORKS :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_SPORTS) ?
            ANDROID_CONTROL_SCENE_MODE_SPORTS :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_PARTY) ?
            ANDROID_CONTROL_SCENE_MODE_PARTY :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_CANDLELIGHT) ?
            ANDROID_CONTROL_SCENE_MODE_CANDLELIGHT :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_BARCODE) ?
            ANDROID_CONTROL_SCENE_MODE_BARCODE:
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_HDR) ?
            ANDROID_CONTROL_SCENE_MODE_HDR:
        -1;
}

Parameters::Parameters::flashMode_t Parameters::flashModeStringToEnum(
        const char *flashMode) {
    return
        !flashMode ?
            Parameters::FLASH_MODE_OFF :
        !strcmp(flashMode, CameraParameters::FLASH_MODE_OFF) ?
            Parameters::FLASH_MODE_OFF :
        !strcmp(flashMode, CameraParameters::FLASH_MODE_AUTO) ?
            Parameters::FLASH_MODE_AUTO :
        !strcmp(flashMode, CameraParameters::FLASH_MODE_ON) ?
            Parameters::FLASH_MODE_ON :
        !strcmp(flashMode, CameraParameters::FLASH_MODE_RED_EYE) ?
            Parameters::FLASH_MODE_RED_EYE :
        !strcmp(flashMode, CameraParameters::FLASH_MODE_TORCH) ?
            Parameters::FLASH_MODE_TORCH :
        Parameters::FLASH_MODE_INVALID;
}

const char *Parameters::flashModeEnumToString(flashMode_t flashMode) {
    switch (flashMode) {
        case FLASH_MODE_OFF:
            return CameraParameters::FLASH_MODE_OFF;
        case FLASH_MODE_AUTO:
            return CameraParameters::FLASH_MODE_AUTO;
        case FLASH_MODE_ON:
            return CameraParameters::FLASH_MODE_ON;
        case FLASH_MODE_RED_EYE:
            return CameraParameters::FLASH_MODE_RED_EYE;
        case FLASH_MODE_TORCH:
            return CameraParameters::FLASH_MODE_TORCH;
        default:
            ALOGE("%s: Unknown flash mode enum %d",
                    __FUNCTION__, flashMode);
            return "unknown";
    }
}

Parameters::Parameters::focusMode_t Parameters::focusModeStringToEnum(
        const char *focusMode) {
    return
        !focusMode ?
            Parameters::FOCUS_MODE_INVALID :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_AUTO) ?
            Parameters::FOCUS_MODE_AUTO :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_INFINITY) ?
            Parameters::FOCUS_MODE_INFINITY :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_MACRO) ?
            Parameters::FOCUS_MODE_MACRO :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_FIXED) ?
            Parameters::FOCUS_MODE_FIXED :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_EDOF) ?
            Parameters::FOCUS_MODE_EDOF :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO) ?
            Parameters::FOCUS_MODE_CONTINUOUS_VIDEO :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE) ?
            Parameters::FOCUS_MODE_CONTINUOUS_PICTURE :
        Parameters::FOCUS_MODE_INVALID;
}

const char *Parameters::focusModeEnumToString(focusMode_t focusMode) {
    switch (focusMode) {
        case FOCUS_MODE_AUTO:
            return CameraParameters::FOCUS_MODE_AUTO;
        case FOCUS_MODE_MACRO:
            return CameraParameters::FOCUS_MODE_MACRO;
        case FOCUS_MODE_CONTINUOUS_VIDEO:
            return CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO;
        case FOCUS_MODE_CONTINUOUS_PICTURE:
            return CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE;
        case FOCUS_MODE_EDOF:
            return CameraParameters::FOCUS_MODE_EDOF;
        case FOCUS_MODE_INFINITY:
            return CameraParameters::FOCUS_MODE_INFINITY;
        case FOCUS_MODE_FIXED:
            return CameraParameters::FOCUS_MODE_FIXED;
        default:
            ALOGE("%s: Unknown focus mode enum: %d",
                    __FUNCTION__, focusMode);
            return "unknown";
    }
}

status_t Parameters::parseAreas(const char *areasCStr,
        Vector<Parameters::Area> *areas) {
    static const size_t NUM_FIELDS = 5;
    areas->clear();
    if (areasCStr == NULL) {
        // If no key exists, use default (0,0,0,0,0)
        areas->push();
        return OK;
    }
    String8 areasStr(areasCStr);
    ssize_t areaStart = areasStr.find("(", 0) + 1;
    while (areaStart != 0) {
        const char* area = areasStr.string() + areaStart;
        char *numEnd;
        int vals[NUM_FIELDS];
        for (size_t i = 0; i < NUM_FIELDS; i++) {
            errno = 0;
            vals[i] = strtol(area, &numEnd, 10);
            if (errno || numEnd == area) return BAD_VALUE;
            area = numEnd + 1;
        }
        areas->push(Parameters::Area(
            vals[0], vals[1], vals[2], vals[3], vals[4]) );
        areaStart = areasStr.find("(", areaStart) + 1;
    }
    return OK;
}

status_t Parameters::validateAreas(const Vector<Parameters::Area> &areas,
                                      size_t maxRegions,
                                      AreaKind areaKind) const {
    // Definition of valid area can be found in
    // include/camera/CameraParameters.h
    if (areas.size() == 0) return BAD_VALUE;
    if (areas.size() == 1) {
        if (areas[0].left == 0 &&
                areas[0].top == 0 &&
                areas[0].right == 0 &&
                areas[0].bottom == 0 &&
                areas[0].weight == 0) {
            // Single (0,0,0,0,0) entry is always valid (== driver decides)
            return OK;
        }
    }

    // fixed focus can only set (0,0,0,0,0) focus area
    if (areaKind == AREA_KIND_FOCUS && focusMode == FOCUS_MODE_FIXED) {
        return BAD_VALUE;
    }

    if (areas.size() > maxRegions) {
        ALOGE("%s: Too many areas requested: %zu",
                __FUNCTION__, areas.size());
        return BAD_VALUE;
    }

    for (Vector<Parameters::Area>::const_iterator a = areas.begin();
         a != areas.end(); a++) {
        if (a->weight < 1 || a->weight > 1000) return BAD_VALUE;
        if (a->left < -1000 || a->left > 1000) return BAD_VALUE;
        if (a->top < -1000 || a->top > 1000) return BAD_VALUE;
        if (a->right < -1000 || a->right > 1000) return BAD_VALUE;
        if (a->bottom < -1000 || a->bottom > 1000) return BAD_VALUE;
        if (a->left >= a->right) return BAD_VALUE;
        if (a->top >= a->bottom) return BAD_VALUE;
    }
    return OK;
}

bool Parameters::boolFromString(const char *boolStr) {
    return !boolStr ? false :
        !strcmp(boolStr, CameraParameters::TRUE) ? true :
        false;
}

int Parameters::degToTransform(int degrees, bool mirror) {
    if (!mirror) {
        if (degrees == 0) return 0;
        else if (degrees == 90) return HAL_TRANSFORM_ROT_90;
        else if (degrees == 180) return HAL_TRANSFORM_ROT_180;
        else if (degrees == 270) return HAL_TRANSFORM_ROT_270;
    } else {  // Do mirror (horizontal flip)
        if (degrees == 0) {           // FLIP_H and ROT_0
            return HAL_TRANSFORM_FLIP_H;
        } else if (degrees == 90) {   // FLIP_H and ROT_90
            return HAL_TRANSFORM_FLIP_H | HAL_TRANSFORM_ROT_90;
        } else if (degrees == 180) {  // FLIP_H and ROT_180
            return HAL_TRANSFORM_FLIP_V;
        } else if (degrees == 270) {  // FLIP_H and ROT_270
            return HAL_TRANSFORM_FLIP_V | HAL_TRANSFORM_ROT_90;
        }
    }
    ALOGE("%s: Bad input: %d", __FUNCTION__, degrees);
    return -1;
}

int Parameters::cropXToArray(int x) const {
    ALOG_ASSERT(x >= 0, "Crop-relative X coordinate = '%d' is out of bounds"
                         "(lower = 0)", x);

    CropRegion previewCrop = calculateCropRegion(/*previewOnly*/ true);
    ALOG_ASSERT(x < previewCrop.width, "Crop-relative X coordinate = '%d' "
                    "is out of bounds (upper = %f)", x, previewCrop.width);

    int ret = x + previewCrop.left;

    ALOG_ASSERT( (ret >= 0 && ret < fastInfo.arrayWidth),
        "Calculated pixel array value X = '%d' is out of bounds (upper = %d)",
        ret, fastInfo.arrayWidth);
    return ret;
}

int Parameters::cropYToArray(int y) const {
    ALOG_ASSERT(y >= 0, "Crop-relative Y coordinate = '%d' is out of bounds "
        "(lower = 0)", y);

    CropRegion previewCrop = calculateCropRegion(/*previewOnly*/ true);
    ALOG_ASSERT(y < previewCrop.height, "Crop-relative Y coordinate = '%d' is "
                "out of bounds (upper = %f)", y, previewCrop.height);

    int ret = y + previewCrop.top;

    ALOG_ASSERT( (ret >= 0 && ret < fastInfo.arrayHeight),
        "Calculated pixel array value Y = '%d' is out of bounds (upper = %d)",
        ret, fastInfo.arrayHeight);

    return ret;

}

int Parameters::normalizedXToCrop(int x) const {
    CropRegion previewCrop = calculateCropRegion(/*previewOnly*/ true);
    return (x + 1000) * (previewCrop.width - 1) / 2000;
}

int Parameters::normalizedYToCrop(int y) const {
    CropRegion previewCrop = calculateCropRegion(/*previewOnly*/ true);
    return (y + 1000) * (previewCrop.height - 1) / 2000;
}

int Parameters::normalizedXToArray(int x) const {

    // Work-around for HAL pre-scaling the coordinates themselves
    if (quirks.meteringCropRegion) {
        return (x + 1000) * (fastInfo.arrayWidth - 1) / 2000;
    }

    return cropXToArray(normalizedXToCrop(x));
}

int Parameters::normalizedYToArray(int y) const {
    // Work-around for HAL pre-scaling the coordinates themselves
    if (quirks.meteringCropRegion) {
        return (y + 1000) * (fastInfo.arrayHeight - 1) / 2000;
    }

    return cropYToArray(normalizedYToCrop(y));
}


Parameters::CropRegion Parameters::calculatePreviewCrop(
        const CropRegion &scalerCrop) const {
    float left, top, width, height;
    float previewAspect = static_cast<float>(previewWidth) / previewHeight;
    float cropAspect = scalerCrop.width / scalerCrop.height;

    if (previewAspect > cropAspect) {
        width = scalerCrop.width;
        height = cropAspect * scalerCrop.height / previewAspect;

        left = scalerCrop.left;
        top = scalerCrop.top + (scalerCrop.height - height) / 2;
    } else {
        width = previewAspect * scalerCrop.width / cropAspect;
        height = scalerCrop.height;

        left = scalerCrop.left + (scalerCrop.width - width) / 2;
        top = scalerCrop.top;
    }

    CropRegion previewCrop = {left, top, width, height};

    return previewCrop;
}

int Parameters::arrayXToNormalizedWithCrop(int x,
        const CropRegion &scalerCrop) const {
    // Work-around for HAL pre-scaling the coordinates themselves
    if (quirks.meteringCropRegion) {
        return x * 2000 / (fastInfo.arrayWidth - 1) - 1000;
    } else {
        CropRegion previewCrop = calculatePreviewCrop(scalerCrop);
        return (x - previewCrop.left) * 2000 / (previewCrop.width - 1) - 1000;
    }
}

int Parameters::arrayYToNormalizedWithCrop(int y,
        const CropRegion &scalerCrop) const {
    // Work-around for HAL pre-scaling the coordinates themselves
    if (quirks.meteringCropRegion) {
        return y * 2000 / (fastInfo.arrayHeight - 1) - 1000;
    } else {
        CropRegion previewCrop = calculatePreviewCrop(scalerCrop);
        return (y - previewCrop.top) * 2000 / (previewCrop.height - 1) - 1000;
    }
}

status_t Parameters::getFilteredSizes(Size limit, Vector<Size> *sizes) {
    if (info == NULL) {
        ALOGE("%s: Static metadata is not initialized", __FUNCTION__);
        return NO_INIT;
    }
    if (sizes == NULL) {
        ALOGE("%s: Input size is null", __FUNCTION__);
        return BAD_VALUE;
    }
    sizes->clear();

    Vector<StreamConfiguration> scs = getStreamConfigurations();
    for (size_t i=0; i < scs.size(); i++) {
        const StreamConfiguration &sc = scs[i];
        if (sc.isInput == ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT &&
                sc.format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED &&
                sc.width <= limit.width && sc.height <= limit.height) {
            int64_t minFrameDuration = getMinFrameDurationNs(
                    {sc.width, sc.height}, HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED);
            if (minFrameDuration > MAX_PREVIEW_RECORD_DURATION_NS) {
                // Filter slow sizes from preview/record
                continue;
            }
            sizes->push({sc.width, sc.height});
        }
    }

    if (sizes->isEmpty()) {
        ALOGE("generated preview size list is empty!!");
        return BAD_VALUE;
    }
    return OK;
}

Parameters::Size Parameters::getMaxSizeForRatio(
        float ratio, const int32_t* sizeArray, size_t count) {
    ALOG_ASSERT(sizeArray != NULL, "size array shouldn't be NULL");
    ALOG_ASSERT(count >= 2 && count % 2 == 0, "count must be a positive even number");

    Size maxSize = {0, 0};
    for (size_t i = 0; i < count; i += 2) {
        if (sizeArray[i] > 0 && sizeArray[i+1] > 0) {
            float curRatio = static_cast<float>(sizeArray[i]) / sizeArray[i+1];
            if (fabs(curRatio - ratio) < ASPECT_RATIO_TOLERANCE && maxSize.width < sizeArray[i]) {
                maxSize.width = sizeArray[i];
                maxSize.height = sizeArray[i+1];
            }
        }
    }

    if (maxSize.width == 0 || maxSize.height == 0) {
        maxSize.width = sizeArray[0];
        maxSize.height = sizeArray[1];
        ALOGW("Unable to find the size to match the given aspect ratio %f."
                "Fall back to %d x %d", ratio, maxSize.width, maxSize.height);
    }

    return maxSize;
}

Parameters::Size Parameters::getMaxSize(const Vector<Parameters::Size> &sizes) {
    Size maxSize = {-1, -1};
    for (size_t i = 0; i < sizes.size(); i++) {
        if (sizes[i].width > maxSize.width ||
                (sizes[i].width == maxSize.width && sizes[i].height > maxSize.height )) {
            maxSize = sizes[i];
        }
    }
    return maxSize;
}

Vector<Parameters::StreamConfiguration> Parameters::getStreamConfigurations() {
    const int STREAM_CONFIGURATION_SIZE = 4;
    const int STREAM_FORMAT_OFFSET = 0;
    const int STREAM_WIDTH_OFFSET = 1;
    const int STREAM_HEIGHT_OFFSET = 2;
    const int STREAM_IS_INPUT_OFFSET = 3;
    Vector<StreamConfiguration> scs;

    camera_metadata_ro_entry_t availableStreamConfigs =
                staticInfo(ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);
    for (size_t i = 0; i < availableStreamConfigs.count; i+= STREAM_CONFIGURATION_SIZE) {
        int32_t format = availableStreamConfigs.data.i32[i + STREAM_FORMAT_OFFSET];
        int32_t width = availableStreamConfigs.data.i32[i + STREAM_WIDTH_OFFSET];
        int32_t height = availableStreamConfigs.data.i32[i + STREAM_HEIGHT_OFFSET];
        int32_t isInput = availableStreamConfigs.data.i32[i + STREAM_IS_INPUT_OFFSET];
        StreamConfiguration sc = {format, width, height, isInput};
        scs.add(sc);
    }
    return scs;
}

int64_t Parameters::getJpegStreamMinFrameDurationNs(Parameters::Size size) {
    return getMinFrameDurationNs(size, HAL_PIXEL_FORMAT_BLOB);
}

int64_t Parameters::getMinFrameDurationNs(Parameters::Size size, int fmt) {
    const int STREAM_DURATION_SIZE = 4;
    const int STREAM_FORMAT_OFFSET = 0;
    const int STREAM_WIDTH_OFFSET = 1;
    const int STREAM_HEIGHT_OFFSET = 2;
    const int STREAM_DURATION_OFFSET = 3;
    camera_metadata_ro_entry_t availableStreamMinDurations =
                staticInfo(ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS);
    for (size_t i = 0; i < availableStreamMinDurations.count; i+= STREAM_DURATION_SIZE) {
        int64_t format = availableStreamMinDurations.data.i64[i + STREAM_FORMAT_OFFSET];
        int64_t width = availableStreamMinDurations.data.i64[i + STREAM_WIDTH_OFFSET];
        int64_t height = availableStreamMinDurations.data.i64[i + STREAM_HEIGHT_OFFSET];
        int64_t duration = availableStreamMinDurations.data.i64[i + STREAM_DURATION_OFFSET];
        if (format == fmt && width == size.width && height == size.height) {
            return duration;
        }
    }

    return -1;
}

bool Parameters::isFpsSupported(const Vector<Size> &sizes, int format, int32_t fps) {
    // Get min frame duration for each size and check if the given fps range can be supported.
    for (size_t i = 0 ; i < sizes.size(); i++) {
        int64_t minFrameDuration = getMinFrameDurationNs(sizes[i], format);
        if (minFrameDuration <= 0) {
            ALOGE("Min frame duration (%" PRId64") for size (%dx%d) and format 0x%x is wrong!",
                minFrameDuration, sizes[i].width, sizes[i].height, format);
            return false;
        }
        int32_t maxSupportedFps = 1e9 / minFrameDuration;
        // Add some margin here for the case where the hal supports 29.xxxfps.
        maxSupportedFps += FPS_MARGIN;
        if (fps > maxSupportedFps) {
            return false;
        }
    }
    return true;
}

SortedVector<int32_t> Parameters::getAvailableOutputFormats() {
    SortedVector<int32_t> outputFormats; // Non-duplicated output formats
    Vector<StreamConfiguration> scs = getStreamConfigurations();
    for (size_t i = 0; i < scs.size(); i++) {
        const StreamConfiguration &sc = scs[i];
        if (sc.isInput == ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT) {
            outputFormats.add(sc.format);
        }
    }

    return outputFormats;
}

Vector<Parameters::Size> Parameters::getAvailableJpegSizes() {
    Vector<Parameters::Size> jpegSizes;
    Vector<StreamConfiguration> scs = getStreamConfigurations();
    for (size_t i = 0; i < scs.size(); i++) {
        const StreamConfiguration &sc = scs[i];
        if (sc.isInput == ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT &&
                sc.format == HAL_PIXEL_FORMAT_BLOB) {
            Size sz = {sc.width, sc.height};
            jpegSizes.add(sz);
        }
    }

    return jpegSizes;
}

Vector<Parameters::StreamConfiguration> Parameters::getPreferredStreamConfigurations(
        int32_t usecaseId) const {
    const size_t STREAM_CONFIGURATION_SIZE = 5;
    const size_t STREAM_WIDTH_OFFSET = 0;
    const size_t STREAM_HEIGHT_OFFSET = 1;
    const size_t STREAM_FORMAT_OFFSET = 2;
    const size_t STREAM_IS_INPUT_OFFSET = 3;
    const size_t STREAM_USECASE_BITMAP_OFFSET = 4;
    Vector<StreamConfiguration> scs;

    if (fastInfo.supportsPreferredConfigs) {
        camera_metadata_ro_entry_t availableStreamConfigs = staticInfo(
                ANDROID_SCALER_AVAILABLE_RECOMMENDED_STREAM_CONFIGURATIONS);
        for (size_t i = 0; i < availableStreamConfigs.count; i+= STREAM_CONFIGURATION_SIZE) {
            int32_t width = availableStreamConfigs.data.i32[i + STREAM_WIDTH_OFFSET];
            int32_t height = availableStreamConfigs.data.i32[i + STREAM_HEIGHT_OFFSET];
            int32_t format = availableStreamConfigs.data.i32[i + STREAM_FORMAT_OFFSET];
            int32_t isInput = availableStreamConfigs.data.i32[i + STREAM_IS_INPUT_OFFSET];
            int32_t supportedUsecases =
                    availableStreamConfigs.data.i32[i + STREAM_USECASE_BITMAP_OFFSET];
            if (supportedUsecases & (1 << usecaseId)) {
                StreamConfiguration sc = {format, width, height, isInput};
                scs.add(sc);
            }
        }
    }

    return scs;
}

Vector<Parameters::Size> Parameters::getPreferredFilteredSizes(int32_t usecaseId,
        int32_t format) const {
    Vector<Parameters::Size> sizes;
    Vector<StreamConfiguration> scs = getPreferredStreamConfigurations(usecaseId);
    for (const auto &it : scs) {
        if (it.format == format) {
            sizes.add({it.width, it.height});
        }
    }

    return sizes;
}

Vector<Parameters::Size> Parameters::getPreferredJpegSizes() const {
    return getPreferredFilteredSizes(
            ANDROID_SCALER_AVAILABLE_RECOMMENDED_STREAM_CONFIGURATIONS_SNAPSHOT,
            HAL_PIXEL_FORMAT_BLOB);
}

Vector<Parameters::Size> Parameters::getPreferredPreviewSizes() const {
    return getPreferredFilteredSizes(
            ANDROID_SCALER_AVAILABLE_RECOMMENDED_STREAM_CONFIGURATIONS_PREVIEW,
            HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED);
}

Vector<Parameters::Size> Parameters::getPreferredVideoSizes() const {
    return getPreferredFilteredSizes(
            ANDROID_SCALER_AVAILABLE_RECOMMENDED_STREAM_CONFIGURATIONS_RECORD,
            HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED);
}

Parameters::CropRegion Parameters::calculateCropRegion(bool previewOnly) const {

    float zoomLeft, zoomTop, zoomWidth, zoomHeight;

    // Need to convert zoom index into a crop rectangle. The rectangle is
    // chosen to maximize its area on the sensor

    camera_metadata_ro_entry_t maxDigitalZoom =
            staticInfo(ANDROID_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM);
    // For each zoom step by how many pixels more do we change the zoom
    float zoomIncrement = (maxDigitalZoom.data.f[0] - 1) /
            (NUM_ZOOM_STEPS-1);
    // The desired activeAreaWidth/cropAreaWidth ratio (or height if h>w)
    // via interpolating zoom step into a zoom ratio
    float zoomRatio = 1 + zoomIncrement * zoom;
    ALOG_ASSERT( (zoomRatio >= 1.f && zoomRatio <= maxDigitalZoom.data.f[0]),
        "Zoom ratio calculated out of bounds. Expected 1 - %f, actual: %f",
        maxDigitalZoom.data.f[0], zoomRatio);

    ALOGV("Zoom maxDigital=%f, increment=%f, ratio=%f, previewWidth=%d, "
          "previewHeight=%d, activeWidth=%d, activeHeight=%d",
          maxDigitalZoom.data.f[0], zoomIncrement, zoomRatio, previewWidth,
          previewHeight, fastInfo.arrayWidth, fastInfo.arrayHeight);

    if (previewOnly) {
        // Calculate a tight crop region for the preview stream only
        float previewRatio = static_cast<float>(previewWidth) / previewHeight;

        /* Ensure that the width/height never go out of bounds
         * by scaling across a diffent dimension if an out-of-bounds
         * possibility exists.
         *
         * e.g. if the previewratio < arrayratio and e.g. zoomratio = 1.0, then by
         * calculating the zoomWidth from zoomHeight we'll actually get a
         * zoomheight > arrayheight
         */
        float arrayRatio = 1.f * fastInfo.arrayWidth / fastInfo.arrayHeight;
        if (previewRatio >= arrayRatio) {
            // Adjust the height based on the width
            zoomWidth =  fastInfo.arrayWidth / zoomRatio;
            zoomHeight = zoomWidth *
                    previewHeight / previewWidth;

        } else {
            // Adjust the width based on the height
            zoomHeight = fastInfo.arrayHeight / zoomRatio;
            zoomWidth = zoomHeight *
                    previewWidth / previewHeight;
        }
    } else {
        // Calculate the global crop region with a shape matching the active
        // array.
        zoomWidth = fastInfo.arrayWidth / zoomRatio;
        zoomHeight = fastInfo.arrayHeight / zoomRatio;
    }

    // center the zoom area within the active area
    zoomLeft = (fastInfo.arrayWidth - zoomWidth) / 2;
    zoomTop = (fastInfo.arrayHeight - zoomHeight) / 2;

    ALOGV("Crop region calculated (x=%d,y=%d,w=%f,h=%f) for zoom=%d",
        (int32_t)zoomLeft, (int32_t)zoomTop, zoomWidth, zoomHeight, this->zoom);

    CropRegion crop = { zoomLeft, zoomTop, zoomWidth, zoomHeight };
    return crop;
}

status_t Parameters::calculatePictureFovs(float *horizFov, float *vertFov)
        const {
    if (fastInfo.isExternalCamera) {
        if (horizFov != NULL) {
            *horizFov = -1.0;
        }
        if (vertFov != NULL) {
            *vertFov = -1.0;
        }
        return OK;
    }

    camera_metadata_ro_entry_t sensorSize =
            staticInfo(ANDROID_SENSOR_INFO_PHYSICAL_SIZE, 2, 2);
    if (!sensorSize.count) return NO_INIT;

    camera_metadata_ro_entry_t pixelArraySize =
            staticInfo(ANDROID_SENSOR_INFO_PIXEL_ARRAY_SIZE, 2, 2);
    if (!pixelArraySize.count) return NO_INIT;

    float arrayAspect = static_cast<float>(fastInfo.arrayWidth) /
            fastInfo.arrayHeight;
    float stillAspect = static_cast<float>(pictureWidth) / pictureHeight;
    ALOGV("Array aspect: %f, still aspect: %f", arrayAspect, stillAspect);

    // The crop factors from the full sensor array to the still picture crop
    // region
    float horizCropFactor = 1.f;
    float vertCropFactor = 1.f;

    /**
     * Need to calculate the still image field of view based on the total pixel
     * array field of view, and the relative aspect ratios of the pixel array
     * and output streams.
     *
     * Special treatment for quirky definition of crop region and relative
     * stream cropping.
     */
    if (quirks.meteringCropRegion) {
        // Use max of preview and video as first crop
        float previewAspect = static_cast<float>(previewWidth) / previewHeight;
        float videoAspect = static_cast<float>(videoWidth) / videoHeight;
        if (videoAspect > previewAspect) {
            previewAspect = videoAspect;
        }
        // First crop sensor to preview aspect ratio
        if (arrayAspect < previewAspect) {
            vertCropFactor = arrayAspect / previewAspect;
        } else {
            horizCropFactor = previewAspect / arrayAspect;
        }
        // Second crop to still aspect ratio
        if (stillAspect < previewAspect) {
            horizCropFactor *= stillAspect / previewAspect;
        } else {
            vertCropFactor *= previewAspect / stillAspect;
        }
    } else {
        /**
         * Crop are just a function of just the still/array relative aspect
         * ratios. Since each stream will maximize its area within the crop
         * region, and for FOV we assume a full-sensor crop region, we only ever
         * crop the FOV either vertically or horizontally, never both.
         */
        horizCropFactor = (arrayAspect > stillAspect) ?
                (stillAspect / arrayAspect) : 1.f;
        vertCropFactor = (arrayAspect < stillAspect) ?
                (arrayAspect / stillAspect) : 1.f;
    }

    /**
     * Convert the crop factors w.r.t the active array size to the crop factors
     * w.r.t the pixel array size.
     */
    horizCropFactor *= (static_cast<float>(fastInfo.arrayWidth) /
                            pixelArraySize.data.i32[0]);
    vertCropFactor *= (static_cast<float>(fastInfo.arrayHeight) /
                            pixelArraySize.data.i32[1]);

    ALOGV("Horiz crop factor: %f, vert crop fact: %f",
            horizCropFactor, vertCropFactor);
    /**
     * Basic field of view formula is:
     *   angle of view = 2 * arctangent ( d / 2f )
     * where d is the physical sensor dimension of interest, and f is
     * the focal length. This only applies to rectilinear sensors, for focusing
     * at distances >> f, etc.
     */
    if (horizFov != NULL) {
        *horizFov = 180 / M_PI * 2 *
                atanf(horizCropFactor * sensorSize.data.f[0] /
                        (2 * fastInfo.defaultFocalLength));
    }
    if (vertFov != NULL) {
        *vertFov = 180 / M_PI * 2 *
                atanf(vertCropFactor * sensorSize.data.f[1] /
                        (2 * fastInfo.defaultFocalLength));
    }
    return OK;
}

int32_t Parameters::fpsFromRange(int32_t /*min*/, int32_t max) const {
    return max;
}

}; // namespace camera2
}; // namespace android
