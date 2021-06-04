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

#ifndef ANDROID_SERVERS_CAMERA_EXIF_UTILS_H
#define ANDROID_SERVERS_CAMERA_EXIF_UTILS_H

#include "CameraMetadata.h"

namespace android {
namespace camera3 {

/*
 * Orientation value:
 *  1      2      3      4      5          6          7          8
 *
 *  888888 888888     88 88     8888888888 88                 88 8888888888
 *  88         88     88 88     88  88     88  88         88  88     88  88
 *  8888     8888   8888 8888   88         8888888888 8888888888         88
 *  88         88     88 88
 *  88         88 888888 888888
 */
enum ExifOrientation : uint16_t {
    ORIENTATION_UNDEFINED   = 0x0,
    ORIENTATION_0_DEGREES   = 0x1,
    ORIENTATION_90_DEGREES  = 0x6,
    ORIENTATION_180_DEGREES = 0x3,
    ORIENTATION_270_DEGREES = 0x8,
};

// This is based on the camera HIDL shim implementation, which was in turned
// based on original ChromeOS ARC implementation of a V4L2 HAL

// ExifUtils can override APP1 segment with tags which caller set. ExifUtils can
// also add a thumbnail in the APP1 segment if thumbnail size is specified.
// ExifUtils can be reused with different images by calling initialize().
//
// Example of using this class :
//  std::unique_ptr<ExifUtils> utils(ExifUtils::Create());
//  utils->initialize(const unsigned char* app1Segment, size_t app1SegmentSize);
//  ...
//  // Call ExifUtils functions to set Exif tags.
//  ...
//  utils->GenerateApp1();
//  unsigned int app1Length = utils->GetApp1Length();
//  uint8_t* app1Buffer = new uint8_t[app1Length];
//  memcpy(app1Buffer, utils->GetApp1Buffer(), app1Length);
class ExifUtils {

public:
    virtual ~ExifUtils();

    static ExifUtils* create();

    // Initialize() can be called multiple times. The setting of Exif tags will be
    // cleared.
    virtual bool initialize(const unsigned char *app1Segment, size_t app1SegmentSize) = 0;
    virtual bool initializeEmpty() = 0;

    // Set all known fields from a metadata structure
    virtual bool setFromMetadata(const CameraMetadata& metadata,
            const CameraMetadata& staticInfo,
            const size_t imageWidth, const size_t imageHeight) = 0;

    // Sets the len aperture.
    // Returns false if memory allocation fails.
    virtual bool setAperture(float aperture) = 0;

    // sets the color space.
    // Returns false if memory allocation fails.
    virtual bool setColorSpace(uint16_t color_space) = 0;

    // Sets the date and time of image last modified. It takes local time. The
    // name of the tag is DateTime in IFD0.
    // Returns false if memory allocation fails.
    virtual bool setDateTime(const struct tm& t) = 0;

    // Sets the digital zoom ratio. If the numerator is 0, it means digital zoom
    // was not used.
    // Returns false if memory allocation fails.
    virtual bool setDigitalZoomRatio(uint32_t crop_width, uint32_t crop_height,
            uint32_t sensor_width, uint32_t sensor_height) = 0;

    // Sets the exposure bias.
    // Returns false if memory allocation fails.
    virtual bool setExposureBias(int32_t ev,
            uint32_t ev_step_numerator, uint32_t ev_step_denominator) = 0;

    // Sets the exposure mode set when the image was shot.
    // Returns false if memory allocation fails.
    virtual bool setExposureMode(uint8_t exposure_mode) = 0;

    // Sets the exposure time, given in seconds.
    // Returns false if memory allocation fails.
    virtual bool setExposureTime(float exposure_time) = 0;

    // Sets the status of flash.
    // Returns false if memory allocation fails.
    virtual bool setFlash(uint8_t flash_available, uint8_t flash_state, uint8_t ae_mode) = 0;

    // Sets the F number.
    // Returns false if memory allocation fails.
    virtual bool setFNumber(float f_number) = 0;

    // Sets the focal length of lens used to take the image in millimeters.
    // Returns false if memory allocation fails.
    virtual bool setFocalLength(float focal_length) = 0;

    // Sets the focal length of lens for 35mm film used to take the image in millimeters.
    // Returns false if memory allocation fails.
    virtual bool setFocalLengthIn35mmFilm(float focal_length,
            float sensor_size_x, float sensor_size_y) = 0;

    // Sets the altitude in meters.
    // Returns false if memory allocation fails.
    virtual bool setGpsAltitude(double altitude) = 0;

    // Sets the latitude with degrees minutes seconds format.
    // Returns false if memory allocation fails.
    virtual bool setGpsLatitude(double latitude) = 0;

    // Sets the longitude with degrees minutes seconds format.
    // Returns false if memory allocation fails.
    virtual bool setGpsLongitude(double longitude) = 0;

    // Sets GPS processing method.
    // Returns false if memory allocation fails.
    virtual bool setGpsProcessingMethod(const std::string& method) = 0;

    // Sets GPS date stamp and time stamp (atomic clock). It takes UTC time.
    // Returns false if memory allocation fails.
    virtual bool setGpsTimestamp(const struct tm& t) = 0;

    // Sets the height (number of rows) of main image.
    // Returns false if memory allocation fails.
    virtual bool setImageHeight(uint32_t length) = 0;

    // Sets the width (number of columns) of main image.
    // Returns false if memory allocation fails.
    virtual bool setImageWidth(uint32_t width) = 0;

    // Sets the ISO speed.
    // Returns false if memory allocation fails.
    virtual bool setIsoSpeedRating(uint16_t iso_speed_ratings) = 0;

    // Sets the smallest F number of the lens.
    // Returns false if memory allocation fails.
    virtual bool setMaxAperture(float aperture) = 0;

    // Sets image orientation.
    // Returns false if memory allocation fails.
    virtual bool setOrientation(uint16_t degrees) = 0;

    // Sets image orientation.
    // Returns false if memory allocation fails.
    virtual bool setOrientationValue(ExifOrientation orientationValue) = 0;

    // Sets the shutter speed.
    // Returns false if memory allocation fails.
    virtual bool setShutterSpeed(float exposure_time) = 0;

    // Sets the distance to the subject, given in meters.
    // Returns false if memory allocation fails.
    virtual bool setSubjectDistance(float diopters) = 0;

    // Sets the fractions of seconds for the <DateTime> tag.
    // Returns false if memory allocation fails.
    virtual bool setSubsecTime(const std::string& subsec_time) = 0;

    // Sets the white balance mode set when the image was shot.
    // Returns false if memory allocation fails.
    virtual bool setWhiteBalance(uint8_t white_blanace) = 0;

    // Generates APP1 segment.
    // Returns false if generating APP1 segment fails.
    virtual bool generateApp1() = 0;

    // Gets buffer of APP1 segment. This method must be called only after calling
    // GenerateAPP1().
    virtual const uint8_t* getApp1Buffer() = 0;

    // Gets length of APP1 segment. This method must be called only after calling
    // GenerateAPP1().
    virtual unsigned int getApp1Length() = 0;
};

} // namespace camera3
} // namespace android

#endif  // ANDROID_SERVERS_CAMERA_EXIF_UTILS_H
