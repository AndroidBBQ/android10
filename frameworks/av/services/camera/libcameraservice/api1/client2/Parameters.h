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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA2PARAMETERS_H
#define ANDROID_SERVERS_CAMERA_CAMERA2PARAMETERS_H

#include <system/graphics.h>

#include <utils/Compat.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include <camera/CameraParameters.h>
#include <camera/CameraParameters2.h>
#include <camera/CameraMetadata.h>

#include "common/CameraDeviceBase.h"

namespace android {
namespace camera2 {

/**
 * Current camera state; this is the full state of the Camera under the old
 * camera API (contents of the CameraParameters2 object in a more-efficient
 * format, plus other state). The enum values are mostly based off the
 * corresponding camera2 enums, not the camera1 strings. A few are defined here
 * if they don't cleanly map to camera2 values.
 */
struct Parameters {
    /**
     * Parameters and other state
     */
    int cameraId;
    int cameraFacing;

    int previewWidth, previewHeight;
    int32_t previewFpsRange[2];
    int previewFormat;

    int previewTransform; // set by CAMERA_CMD_SET_DISPLAY_ORIENTATION

    int pictureWidth, pictureHeight;
    // Store the picture size before they are overriden by video snapshot
    int pictureWidthLastSet, pictureHeightLastSet;
    bool pictureSizeOverriden;

    int32_t jpegThumbSize[2];
    uint8_t jpegQuality, jpegThumbQuality;
    int32_t jpegRotation;

    bool gpsEnabled;
    double gpsCoordinates[3];
    int64_t gpsTimestamp;
    String8 gpsProcessingMethod;

    uint8_t wbMode;
    uint8_t effectMode;
    uint8_t antibandingMode;
    uint8_t sceneMode;

    enum flashMode_t {
        FLASH_MODE_OFF = 0,
        FLASH_MODE_AUTO,
        FLASH_MODE_ON,
        FLASH_MODE_TORCH,
        FLASH_MODE_RED_EYE = ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE,
        FLASH_MODE_INVALID = -1
    } flashMode;

    enum focusMode_t {
        FOCUS_MODE_AUTO = ANDROID_CONTROL_AF_MODE_AUTO,
        FOCUS_MODE_MACRO = ANDROID_CONTROL_AF_MODE_MACRO,
        FOCUS_MODE_CONTINUOUS_VIDEO = ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO,
        FOCUS_MODE_CONTINUOUS_PICTURE = ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE,
        FOCUS_MODE_EDOF = ANDROID_CONTROL_AF_MODE_EDOF,
        FOCUS_MODE_INFINITY,
        FOCUS_MODE_FIXED,
        FOCUS_MODE_INVALID = -1
    } focusMode;

    uint8_t focusState; // Latest focus state from HAL

    // For use with triggerAfWithAuto quirk
    focusMode_t shadowFocusMode;

    struct Area {
        int left, top, right, bottom;
        int weight;
        Area() {}
        Area(int left, int top, int right, int bottom, int weight):
                left(left), top(top), right(right), bottom(bottom),
                weight(weight) {}
        bool isEmpty() const {
            return (left == 0) && (top == 0) && (right == 0) && (bottom == 0);
        }
    };
    Vector<Area> focusingAreas;

    struct Size {
        int32_t width;
        int32_t height;
    };

    struct FpsRange {
        int32_t low;
        int32_t high;
    };

    uint8_t aeState; //latest AE state from Hal
    int32_t exposureCompensation;
    bool autoExposureLock;
    bool autoExposureLockAvailable;
    bool autoWhiteBalanceLock;
    bool autoWhiteBalanceLockAvailable;

    // 3A region types, for use with ANDROID_CONTROL_MAX_REGIONS
    enum region_t {
        REGION_AE = 0,
        REGION_AWB,
        REGION_AF,
        NUM_REGION // Number of region types
    } region;

    Vector<Area> meteringAreas;

    int zoom;
    bool zoomAvailable;

    int videoWidth, videoHeight, videoFormat;
    android_dataspace videoDataSpace;

    bool recordingHint;
    bool videoStabilization;

    CameraParameters2 params;
    String8 paramsFlattened;

    // These parameters are also part of the camera API-visible state, but not
    // directly listed in Camera.Parameters
    // One of ICamera::VIDEO_BUFFER_MODE_*
    int32_t videoBufferMode;
    bool playShutterSound;
    bool enableFaceDetect;

    bool enableFocusMoveMessages;
    int afTriggerCounter;
    int afStateCounter;
    int currentAfTriggerId;
    bool afInMotion;

    int precaptureTriggerCounter;

    int takePictureCounter;

    uint32_t previewCallbackFlags;
    bool previewCallbackOneShot;
    bool previewCallbackSurface;

    bool allowZslMode;
    // Whether the jpeg stream is slower than 30FPS and can slow down preview.
    // When slowJpegMode is true, allowZslMode must be false to avoid slowing down preview.
    bool slowJpegMode;
    // Whether ZSL reprocess is supported by the device.
    bool isZslReprocessPresent;
    // Whether the device supports enableZsl.
    bool isDeviceZslSupported;
    // Whether the device supports geometric distortion correction
    bool isDistortionCorrectionSupported;

    // Overall camera state
    enum State {
        DISCONNECTED,
        STOPPED,
        WAITING_FOR_PREVIEW_WINDOW,
        PREVIEW,
        RECORD,
        STILL_CAPTURE,
        VIDEO_SNAPSHOT
    } state;

    // Number of zoom steps to simulate
    static const unsigned int NUM_ZOOM_STEPS = 100;
    // Max preview size allowed
    // This is set to a 1:1 value to allow for any aspect ratio that has
    // a max long side of 1920 pixels
    static const unsigned int MAX_PREVIEW_WIDTH = 1920;
    static const unsigned int MAX_PREVIEW_HEIGHT = 1920;
    // Initial max preview/recording size bound
    static const int MAX_INITIAL_PREVIEW_WIDTH = 1920;
    static const int MAX_INITIAL_PREVIEW_HEIGHT = 1080;
    // Aspect ratio tolerance
    static const CONSTEXPR float ASPECT_RATIO_TOLERANCE = 0.001;
    // Threshold for slow jpeg mode
    static const int64_t kSlowJpegModeThreshold = 33400000LL; // 33.4 ms
    // Margin for checking FPS
    static const int32_t FPS_MARGIN = 1;
    // Max FPS for default parameters
    static const int32_t MAX_DEFAULT_FPS = 30;
    // Minimum FPS for a size to be listed in supported preview/video sizes
    // Set to slightly less than 30.0 to have some tolerance margin
    static constexpr double MIN_PREVIEW_RECORD_FPS = 29.97;
    // Maximum frame duration for a size to be listed in supported preview/video sizes
    static constexpr int64_t MAX_PREVIEW_RECORD_DURATION_NS = 1e9 / MIN_PREVIEW_RECORD_FPS;

    // Full static camera info, object owned by someone else, such as
    // Camera2Device.
    const CameraMetadata *info;

    // Fast-access static device information; this is a subset of the
    // information available through the staticInfo() method, used for
    // frequently-accessed values or values that have to be calculated from the
    // static information.
    struct DeviceInfo {
        int32_t arrayWidth;
        int32_t arrayHeight;
        int32_t bestStillCaptureFpsRange[2];
        uint8_t bestFaceDetectMode;
        int32_t maxFaces;
        struct OverrideModes {
            flashMode_t flashMode;
            uint8_t     wbMode;
            focusMode_t focusMode;
            OverrideModes():
                    flashMode(FLASH_MODE_INVALID),
                    wbMode(ANDROID_CONTROL_AWB_MODE_OFF),
                    focusMode(FOCUS_MODE_INVALID) {
            }
        };
        DefaultKeyedVector<uint8_t, OverrideModes> sceneModeOverrides;
        bool isExternalCamera;
        float defaultFocalLength;
        bool useFlexibleYuv;
        Size maxJpegSize;
        Size maxZslSize;
        bool supportsPreferredConfigs;
    } fastInfo;

    // Quirks information; these are short-lived flags to enable workarounds for
    // incomplete HAL implementations
    struct Quirks {
        bool triggerAfWithAuto;
        bool useZslFormat;
        bool meteringCropRegion;
        bool partialResults;
    } quirks;

    /**
     * Parameter manipulation and setup methods
     */

    Parameters(int cameraId, int cameraFacing);
    ~Parameters();

    // Sets up default parameters
    status_t initialize(CameraDeviceBase *device, int deviceVersion);

    // Build fast-access device static info from static info
    status_t buildFastInfo(CameraDeviceBase *device);
    // Query for quirks from static info
    status_t buildQuirks();

    // Get entry from camera static characteristics information. min/maxCount
    // are used for error checking the number of values in the entry. 0 for
    // max/minCount means to do no bounds check in that direction. In case of
    // error, the entry data pointer is null and the count is 0.
    camera_metadata_ro_entry_t staticInfo(uint32_t tag,
            size_t minCount=0, size_t maxCount=0, bool required=true) const;

    // Validate and update camera parameters based on new settings
    status_t set(const String8 &paramString);

    // Retrieve the current settings
    String8 get() const;

    // Update passed-in request for common parameters
    status_t updateRequest(CameraMetadata *request) const;

    // Add/update JPEG entries in metadata
    status_t updateRequestJpeg(CameraMetadata *request) const;

    /* Helper functions to override jpeg size for video snapshot */
    // Override jpeg size by video size. Called during startRecording.
    status_t overrideJpegSizeByVideoSize();
    // Recover overridden jpeg size.  Called during stopRecording.
    status_t recoverOverriddenJpegSize();
    // if video snapshot size is currently overridden
    bool isJpegSizeOverridden();
    // whether zero shutter lag should be used for non-recording operation
    bool useZeroShutterLag() const;

    // Get default focal length
    status_t getDefaultFocalLength(CameraDeviceBase *camera);

    // Calculate the crop region rectangle, either tightly about the preview
    // resolution, or a region just based on the active array; both take
    // into account the current zoom level.
    struct CropRegion {
        float left;
        float top;
        float width;
        float height;
    };
    CropRegion calculateCropRegion(bool previewOnly) const;

    // Calculate the field of view of the high-resolution JPEG capture
    status_t calculatePictureFovs(float *horizFov, float *vertFov) const;

    // Static methods for debugging and converting between camera1 and camera2
    // parameters

    static const char *getStateName(State state);

    static int formatStringToEnum(const char *format);
    static const char *formatEnumToString(int format);

    static int wbModeStringToEnum(const char *wbMode);
    static const char* wbModeEnumToString(uint8_t wbMode);
    static int effectModeStringToEnum(const char *effectMode);
    static int abModeStringToEnum(const char *abMode);
    static int sceneModeStringToEnum(const char *sceneMode, uint8_t defaultScene);
    static flashMode_t flashModeStringToEnum(const char *flashMode);
    static const char* flashModeEnumToString(flashMode_t flashMode);
    static focusMode_t focusModeStringToEnum(const char *focusMode);
    static const char* focusModeEnumToString(focusMode_t focusMode);

    static status_t parseAreas(const char *areasCStr,
            Vector<Area> *areas);

    enum AreaKind
    {
        AREA_KIND_FOCUS,
        AREA_KIND_METERING
    };
    status_t validateAreas(const Vector<Area> &areas,
                                  size_t maxRegions,
                                  AreaKind areaKind) const;
    static bool boolFromString(const char *boolStr);

    // Map from camera orientation + facing to gralloc transform enum
    static int degToTransform(int degrees, bool mirror);

    // API specifies FPS ranges are done in fixed point integer, with LSB = 0.001.
    // Note that this doesn't apply to the (deprecated) single FPS value.
    static const int kFpsToApiScale = 1000;

    // Transform from (-1000,-1000)-(1000,1000) normalized coords from camera
    // API to HAL3 (0,0)-(activePixelArray.width/height) coordinates
    int normalizedXToArray(int x) const;
    int normalizedYToArray(int y) const;

    // Transform from HAL3 (0,0)-(activePixelArray.width/height) coordinates to
    // (-1000,-1000)-(1000,1000) normalized coordinates given a scaler crop
    // region.
    int arrayXToNormalizedWithCrop(int x, const CropRegion &scalerCrop) const;
    int arrayYToNormalizedWithCrop(int y, const CropRegion &scalerCrop) const;

    struct Range {
        int min;
        int max;
    };

    int32_t fpsFromRange(int32_t min, int32_t max) const;

private:

    // Convert from viewfinder crop-region relative array coordinates
    // to HAL3 sensor array coordinates
    int cropXToArray(int x) const;
    int cropYToArray(int y) const;

    // Convert from camera API (-1000,1000)-(1000,1000) normalized coords
    // to viewfinder crop-region relative array coordinates
    int normalizedXToCrop(int x) const;
    int normalizedYToCrop(int y) const;

    // Given a scaler crop region, calculate preview crop region based on
    // preview aspect ratio.
    CropRegion calculatePreviewCrop(const CropRegion &scalerCrop) const;

    Vector<Size> availablePreviewSizes;
    Vector<Size> availableVideoSizes;
    // Get size list (that are no larger than limit) from static metadata.
    // This method filtered size with minFrameDuration < MAX_PREVIEW_RECORD_DURATION_NS
    status_t getFilteredSizes(Size limit, Vector<Size> *sizes);
    // Get max size (from the size array) that matches the given aspect ratio.
    Size getMaxSizeForRatio(float ratio, const int32_t* sizeArray, size_t count);

    // Helper function for overriding jpeg size for video snapshot
    // Check if overridden jpeg size needs to be updated after Parameters::set.
    // The behavior of this function is tailored to the implementation of Parameters::set.
    // Do not use this function for other purpose.
    status_t updateOverriddenJpegSize();

    struct StreamConfiguration {
        int32_t format;
        int32_t width;
        int32_t height;
        int32_t isInput;
    };

    // Helper function extract available stream configuration
    // Only valid since device HAL version 3.2
    // returns an empty Vector if device HAL version does support it
    Vector<StreamConfiguration> getStreamConfigurations();

    // Helper function to extract the suggested stream configurations
    Vector<StreamConfiguration> getPreferredStreamConfigurations(int32_t usecaseId) const;

    // Helper function to get minimum frame duration for a jpeg size
    // return -1 if input jpeg size cannot be found in supported size list
    int64_t getJpegStreamMinFrameDurationNs(Parameters::Size size);

    // Helper function to get minimum frame duration for a size/format combination
    // return -1 if input size/format combination cannot be found.
    int64_t getMinFrameDurationNs(Parameters::Size size, int format);

    // Helper function to check if a given fps is supported by all the sizes with
    // the same format.
    // return true if the device doesn't support min frame duration metadata tag.
    bool isFpsSupported(const Vector<Size> &size, int format, int32_t fps);

    // Helper function to get non-duplicated available output formats
    SortedVector<int32_t> getAvailableOutputFormats();
    // Helper function to get available output jpeg sizes
    Vector<Size> getAvailableJpegSizes();
    // Helper function to get maximum size in input Size vector.
    // The maximum size is defined by comparing width first, when width ties comparing height.
    Size getMaxSize(const Vector<Size>& sizes);

    // Helper function to filter and sort suggested sizes
    Vector<Parameters::Size> getPreferredFilteredSizes(int32_t usecaseId, int32_t format) const;
    // Helper function to get the suggested jpeg sizes
    Vector<Size> getPreferredJpegSizes() const;
    // Helper function to get the suggested preview sizes
    Vector<Size> getPreferredPreviewSizes() const;
    // Helper function to get the suggested video sizes
    Vector<Size> getPreferredVideoSizes() const;

    int mDeviceVersion;
    uint8_t mDefaultSceneMode;
};

// This class encapsulates the Parameters class so that it can only be accessed
// by constructing a Lock object, which locks the SharedParameter's mutex.
class SharedParameters {
  public:
    SharedParameters(int cameraId, int cameraFacing):
            mParameters(cameraId, cameraFacing) {
    }

    template<typename S, typename P>
    class BaseLock {
      public:
        explicit BaseLock(S &p):
                mParameters(p.mParameters),
                mSharedParameters(p) {
            mSharedParameters.mLock.lock();
        }

        ~BaseLock() {
            mSharedParameters.mLock.unlock();
        }
        P &mParameters;
      private:
        // Disallow copying, default construction
        BaseLock();
        BaseLock(const BaseLock &);
        BaseLock &operator=(const BaseLock &);
        S &mSharedParameters;
    };
    typedef BaseLock<SharedParameters, Parameters> Lock;
    typedef BaseLock<const SharedParameters, const Parameters> ReadLock;

    // Access static info, read-only and immutable, so no lock needed
    camera_metadata_ro_entry_t staticInfo(uint32_t tag,
            size_t minCount=0, size_t maxCount=0) const {
        return mParameters.staticInfo(tag, minCount, maxCount);
    }

    // Only use for dumping or other debugging
    const Parameters &unsafeAccess() {
        return mParameters;
    }
  private:
    Parameters mParameters;
    mutable Mutex mLock;
};


}; // namespace camera2
}; // namespace android

#endif
