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

#define LOG_TAG "CameraServerExifUtils"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <cutils/log.h>

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "ExifUtils.h"

extern "C" {
#include <libexif/exif-data.h>
}

namespace std {

template <>
struct default_delete<ExifEntry> {
    inline void operator()(ExifEntry* entry) const { exif_entry_unref(entry); }
};

}  // namespace std


namespace android {
namespace camera3 {


class ExifUtilsImpl : public ExifUtils {
public:
    ExifUtilsImpl();

    virtual ~ExifUtilsImpl();

    // Initialize() can be called multiple times. The setting of Exif tags will be
    // cleared.
    virtual bool initialize(const unsigned char *app1Segment, size_t app1SegmentSize);
    virtual bool initializeEmpty();

    // set all known fields from a metadata structure
    virtual bool setFromMetadata(const CameraMetadata& metadata,
            const CameraMetadata& staticInfo,
            const size_t imageWidth,
            const size_t imageHeight);

    // sets the len aperture.
    // Returns false if memory allocation fails.
    virtual bool setAperture(float aperture);

    // sets the color space.
    // Returns false if memory allocation fails.
    virtual bool setColorSpace(uint16_t color_space);

    // sets the date and time of image last modified. It takes local time. The
    // name of the tag is DateTime in IFD0.
    // Returns false if memory allocation fails.
    virtual bool setDateTime(const struct tm& t);

    // sets the digital zoom ratio. If the numerator is 0, it means digital zoom
    // was not used.
    // Returns false if memory allocation fails.
    virtual bool setDigitalZoomRatio(
            uint32_t crop_width, uint32_t crop_height,
            uint32_t sensor_width, uint32_t sensor_height);

    // Sets the exposure bias.
    // Returns false if memory allocation fails.
    virtual bool setExposureBias(int32_t ev,
            uint32_t ev_step_numerator, uint32_t ev_step_denominator);

    // sets the exposure mode set when the image was shot.
    // Returns false if memory allocation fails.
    virtual bool setExposureMode(uint8_t exposure_mode);

    // sets the exposure time, given in seconds.
    // Returns false if memory allocation fails.
    virtual bool setExposureTime(float exposure_time);

    // sets the status of flash.
    // Returns false if memory allocation fails.
    virtual bool setFlash(uint8_t flash_available, uint8_t flash_state, uint8_t ae_mode);

    // sets the F number.
    // Returns false if memory allocation fails.
    virtual bool setFNumber(float f_number);

    // sets the focal length of lens used to take the image in millimeters.
    // Returns false if memory allocation fails.
    virtual bool setFocalLength(float focal_length);

    // sets the focal length of lens for 35mm film used to take the image in millimeters.
    // Returns false if memory allocation fails.
    virtual bool setFocalLengthIn35mmFilm(float focal_length,
            float sensor_size_x, float sensor_size_y);

    // sets the altitude in meters.
    // Returns false if memory allocation fails.
    virtual bool setGpsAltitude(double altitude);

    // sets the latitude with degrees minutes seconds format.
    // Returns false if memory allocation fails.
    virtual bool setGpsLatitude(double latitude);

    // sets the longitude with degrees minutes seconds format.
    // Returns false if memory allocation fails.
    virtual bool setGpsLongitude(double longitude);

    // sets GPS processing method.
    // Returns false if memory allocation fails.
    virtual bool setGpsProcessingMethod(const std::string& method);

    // sets GPS date stamp and time stamp (atomic clock). It takes UTC time.
    // Returns false if memory allocation fails.
    virtual bool setGpsTimestamp(const struct tm& t);

    // sets the length (number of rows) of main image.
    // Returns false if memory allocation fails.
    virtual bool setImageHeight(uint32_t length);

    // sets the width (number of columes) of main image.
    // Returns false if memory allocation fails.
    virtual bool setImageWidth(uint32_t width);

    // sets the ISO speed.
    // Returns false if memory allocation fails.
    virtual bool setIsoSpeedRating(uint16_t iso_speed_ratings);

    // sets the smallest F number of the lens.
    // Returns false if memory allocation fails.
    virtual bool setMaxAperture(float aperture);

    // sets image orientation.
    // Returns false if memory allocation fails.
    virtual bool setOrientation(uint16_t degrees);

    // sets image orientation.
    // Returns false if memory allocation fails.
    virtual bool setOrientationValue(ExifOrientation orientationValue);

    // sets the shutter speed.
    // Returns false if memory allocation fails.
    virtual bool setShutterSpeed(float exposure_time);

    // sets the distance to the subject, given in meters.
    // Returns false if memory allocation fails.
    virtual bool setSubjectDistance(float diopters);

    // sets the fractions of seconds for the <DateTime> tag.
    // Returns false if memory allocation fails.
    virtual bool setSubsecTime(const std::string& subsec_time);

    // sets the white balance mode set when the image was shot.
    // Returns false if memory allocation fails.
    virtual bool setWhiteBalance(uint8_t white_balance);

    // Generates APP1 segment.
    // Returns false if generating APP1 segment fails.
    virtual bool generateApp1();

    // Gets buffer of APP1 segment. This method must be called only after calling
    // GenerateAPP1().
    virtual const uint8_t* getApp1Buffer();

    // Gets length of APP1 segment. This method must be called only after calling
    // GenerateAPP1().
    virtual unsigned int getApp1Length();

  protected:
    // sets the version of this standard supported.
    // Returns false if memory allocation fails.
    virtual bool setExifVersion(const std::string& exif_version);

    // Resets the pointers and memories.
    virtual void reset();

    // Adds a variable length tag to |exif_data_|. It will remove the original one
    // if the tag exists.
    // Returns the entry of the tag. The reference count of returned ExifEntry is
    // two.
    virtual std::unique_ptr<ExifEntry> addVariableLengthEntry(ExifIfd ifd,
            ExifTag tag, ExifFormat format, uint64_t components, unsigned int size);

    // Adds a entry of |tag| in |exif_data_|. It won't remove the original one if
    // the tag exists.
    // Returns the entry of the tag. It adds one reference count to returned
    // ExifEntry.
    virtual std::unique_ptr<ExifEntry> addEntry(ExifIfd ifd, ExifTag tag);

    // Helpe functions to add exif data with different types.
    virtual bool setShort(ExifIfd ifd, ExifTag tag, uint16_t value, const std::string& msg);

    virtual bool setLong(ExifIfd ifd, ExifTag tag, uint32_t value, const std::string& msg);

    virtual bool setRational(ExifIfd ifd, ExifTag tag, uint32_t numerator,
            uint32_t denominator, const std::string& msg);

    virtual bool setSRational(ExifIfd ifd, ExifTag tag, int32_t numerator,
            int32_t denominator, const std::string& msg);

    virtual bool setString(ExifIfd ifd, ExifTag tag, ExifFormat format,
            const std::string& buffer, const std::string& msg);

    float convertToApex(float val) {
        return 2.0f * log2f(val);
    }

    // Destroys the buffer of APP1 segment if exists.
    virtual void destroyApp1();

    // The Exif data (APP1). Owned by this class.
    ExifData* exif_data_;
    // The raw data of APP1 segment. It's allocated by ExifMem in |exif_data_| but
    // owned by this class.
    uint8_t* app1_buffer_;
    // The length of |app1_buffer_|.
    unsigned int app1_length_;

    // How precise the float-to-rational conversion for EXIF tags would be.
    const static int kRationalPrecision = 10000;
};

#define SET_SHORT(ifd, tag, value)                      \
    do {                                                \
        if (setShort(ifd, tag, value, #tag) == false)   \
            return false;                               \
    } while (0);

#define SET_LONG(ifd, tag, value)                       \
    do {                                                \
        if (setLong(ifd, tag, value, #tag) == false)    \
            return false;                               \
    } while (0);

#define SET_RATIONAL(ifd, tag, numerator, denominator)                      \
    do {                                                                    \
        if (setRational(ifd, tag, numerator, denominator, #tag) == false)   \
            return false;                                                   \
    } while (0);

#define SET_SRATIONAL(ifd, tag, numerator, denominator)                       \
    do {                                                                      \
        if (setSRational(ifd, tag, numerator, denominator, #tag) == false)    \
            return false;                                                     \
    } while (0);

#define SET_STRING(ifd, tag, format, buffer)                                  \
    do {                                                                      \
        if (setString(ifd, tag, format, buffer, #tag) == false)               \
            return false;                                                     \
    } while (0);

// This comes from the Exif Version 2.2 standard table 6.
const char gExifAsciiPrefix[] = {0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0};

static void setLatitudeOrLongitudeData(unsigned char* data, double num) {
    // Take the integer part of |num|.
    ExifLong degrees = static_cast<ExifLong>(num);
    ExifLong minutes = static_cast<ExifLong>(60 * (num - degrees));
    ExifLong microseconds =
            static_cast<ExifLong>(3600000000u * (num - degrees - minutes / 60.0));
    exif_set_rational(data, EXIF_BYTE_ORDER_INTEL, {degrees, 1});
    exif_set_rational(data + sizeof(ExifRational), EXIF_BYTE_ORDER_INTEL, {minutes, 1});
    exif_set_rational(data + 2 * sizeof(ExifRational), EXIF_BYTE_ORDER_INTEL,
            {microseconds, 1000000});
}

ExifUtils *ExifUtils::create() {
    return new ExifUtilsImpl();
}

ExifUtils::~ExifUtils() {
}

ExifUtilsImpl::ExifUtilsImpl()
        : exif_data_(nullptr), app1_buffer_(nullptr), app1_length_(0) {}

ExifUtilsImpl::~ExifUtilsImpl() {
    reset();
}


bool ExifUtilsImpl::initialize(const unsigned char *app1Segment, size_t app1SegmentSize) {
    reset();
    exif_data_ = exif_data_new_from_data(app1Segment, app1SegmentSize);
    if (exif_data_ == nullptr) {
        ALOGE("%s: allocate memory for exif_data_ failed", __FUNCTION__);
        return false;
    }
    // set the image options.
    exif_data_set_option(exif_data_, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
    exif_data_set_data_type(exif_data_, EXIF_DATA_TYPE_COMPRESSED);
    exif_data_set_byte_order(exif_data_, EXIF_BYTE_ORDER_INTEL);

    // set exif version to 2.2.
    if (!setExifVersion("0220")) {
        return false;
    }

    return true;
}

bool ExifUtilsImpl::initializeEmpty() {
    reset();
    exif_data_ = exif_data_new();
    if (exif_data_ == nullptr) {
        ALOGE("%s: allocate memory for exif_data_ failed", __FUNCTION__);
        return false;
    }
    // set the image options.
    exif_data_set_option(exif_data_, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
    exif_data_set_data_type(exif_data_, EXIF_DATA_TYPE_COMPRESSED);
    exif_data_set_byte_order(exif_data_, EXIF_BYTE_ORDER_INTEL);

    // set exif version to 2.2.
    if (!setExifVersion("0220")) {
        return false;
    }

    return true;
}

bool ExifUtilsImpl::setAperture(float aperture) {
    float apexValue = convertToApex(aperture);
    SET_RATIONAL(EXIF_IFD_EXIF, EXIF_TAG_APERTURE_VALUE,
            static_cast<uint32_t>(std::round(apexValue * kRationalPrecision)),
            kRationalPrecision);
    return true;
}

bool ExifUtilsImpl::setColorSpace(uint16_t color_space) {
    SET_SHORT(EXIF_IFD_EXIF, EXIF_TAG_COLOR_SPACE, color_space);
    return true;
}

bool ExifUtilsImpl::setDateTime(const struct tm& t) {
    // The length is 20 bytes including NULL for termination in Exif standard.
    char str[20];
    int result = snprintf(str, sizeof(str), "%04i:%02i:%02i %02i:%02i:%02i",
            t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    if (result != sizeof(str) - 1) {
        ALOGW("%s: Input time is invalid", __FUNCTION__);
        return false;
    }
    std::string buffer(str);
    SET_STRING(EXIF_IFD_0, EXIF_TAG_DATE_TIME, EXIF_FORMAT_ASCII, buffer);
    SET_STRING(EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL, EXIF_FORMAT_ASCII, buffer);
    SET_STRING(EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_DIGITIZED, EXIF_FORMAT_ASCII, buffer);
    return true;
}

bool ExifUtilsImpl::setDigitalZoomRatio(
        uint32_t crop_width, uint32_t crop_height,
        uint32_t sensor_width, uint32_t sensor_height) {
    float zoomRatioX = (crop_width == 0) ? 1.0 : 1.0 * sensor_width / crop_width;
    float zoomRatioY = (crop_height == 0) ? 1.0 : 1.0 * sensor_height / crop_height;
    float zoomRatio = std::max(zoomRatioX, zoomRatioY);
    const static float noZoomThreshold = 1.02f;

    if (zoomRatio <= noZoomThreshold) {
        SET_RATIONAL(EXIF_IFD_EXIF, EXIF_TAG_DIGITAL_ZOOM_RATIO, 0, 1);
    } else {
        SET_RATIONAL(EXIF_IFD_EXIF, EXIF_TAG_DIGITAL_ZOOM_RATIO,
                static_cast<uint32_t>(std::round(zoomRatio * kRationalPrecision)),
                kRationalPrecision);
    }
    return true;
}

bool ExifUtilsImpl::setExposureMode(uint8_t exposure_mode) {
    uint16_t exposureMode = (exposure_mode == ANDROID_CONTROL_AE_MODE_OFF) ? 1 : 0;
    SET_SHORT(EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE, exposureMode);
    return true;
}

bool ExifUtilsImpl::setExposureTime(float exposure_time) {
    SET_RATIONAL(EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME,
            static_cast<uint32_t>(std::round(exposure_time * kRationalPrecision)),
            kRationalPrecision);
    return true;
}

bool ExifUtilsImpl::setFlash(uint8_t flash_available, uint8_t flash_state, uint8_t ae_mode) {
    // EXIF_TAG_FLASH bits layout per EXIF standard:
    // Bit 0:    0 - did not fire
    //           1 - fired
    // Bit 1-2:  status of return light
    // Bit 3-4:  0 - unknown
    //           1 - compulsory flash firing
    //           2 - compulsory flash suppression
    //           3 - auto mode
    // Bit 5:    0 - flash function present
    //           1 - no flash function
    // Bit 6:    0 - no red-eye reduction mode or unknown
    //           1 - red-eye reduction supported
    uint16_t flash = 0x20;

    if (flash_available == ANDROID_FLASH_INFO_AVAILABLE_TRUE) {
        flash = 0x00;

        if (flash_state == ANDROID_FLASH_STATE_FIRED) {
            flash |= 0x1;
        }
        if (ae_mode == ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE) {
            flash |= 0x40;
        }

        uint16_t flashMode = 0;
        switch (ae_mode) {
            case ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH:
            case ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE:
               flashMode = 3; // AUTO
               break;
            case ANDROID_CONTROL_AE_MODE_ON_ALWAYS_FLASH:
            case ANDROID_CONTROL_AE_MODE_ON_EXTERNAL_FLASH:
               flashMode = 1; // ON
               break;
            case ANDROID_CONTROL_AE_MODE_OFF:
            case ANDROID_CONTROL_AE_MODE_ON:
               flashMode = 2; // OFF
               break;
            default:
               flashMode = 0; // UNKNOWN
               break;
        }
        flash |= (flashMode << 3);
    }
    SET_SHORT(EXIF_IFD_EXIF, EXIF_TAG_FLASH, flash);
    return true;
}

bool ExifUtilsImpl::setFNumber(float f_number) {
    SET_RATIONAL(EXIF_IFD_EXIF, EXIF_TAG_FNUMBER,
            static_cast<uint32_t>(std::round(f_number * kRationalPrecision)),
            kRationalPrecision);
    return true;
}

bool ExifUtilsImpl::setFocalLength(float focal_length) {
    uint32_t numerator = static_cast<uint32_t>(std::round(focal_length * kRationalPrecision));
    SET_RATIONAL(EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH, numerator, kRationalPrecision);
    return true;
}

bool ExifUtilsImpl::setFocalLengthIn35mmFilm(
        float focal_length, float sensor_size_x, float sensor_size_y) {
    static const float filmDiagonal = 43.27; // diagonal of 35mm film
    static const float minSensorDiagonal = 0.01;
    float sensorDiagonal = std::sqrt(
            sensor_size_x * sensor_size_x + sensor_size_y * sensor_size_y);
    sensorDiagonal = std::max(sensorDiagonal, minSensorDiagonal);
    float focalLength35mmFilm = std::round(focal_length * filmDiagonal / sensorDiagonal);
    focalLength35mmFilm = std::min(1.0f * 65535, focalLength35mmFilm);

    SET_SHORT(EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH_IN_35MM_FILM,
            static_cast<uint16_t>(focalLength35mmFilm));
    return true;
}

bool ExifUtilsImpl::setGpsAltitude(double altitude) {
    ExifTag refTag = static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE_REF);
    std::unique_ptr<ExifEntry> refEntry =
            addVariableLengthEntry(EXIF_IFD_GPS, refTag, EXIF_FORMAT_BYTE, 1, 1);
    if (!refEntry) {
        ALOGE("%s: Adding GPSAltitudeRef exif entry failed", __FUNCTION__);
        return false;
    }
    if (altitude >= 0) {
        *refEntry->data = 0;
    } else {
        *refEntry->data = 1;
        altitude *= -1;
    }

    ExifTag tag = static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE);
    std::unique_ptr<ExifEntry> entry = addVariableLengthEntry(
            EXIF_IFD_GPS, tag, EXIF_FORMAT_RATIONAL, 1, sizeof(ExifRational));
    if (!entry) {
        exif_content_remove_entry(exif_data_->ifd[EXIF_IFD_GPS], refEntry.get());
        ALOGE("%s: Adding GPSAltitude exif entry failed", __FUNCTION__);
        return false;
    }
    exif_set_rational(entry->data, EXIF_BYTE_ORDER_INTEL,
            {static_cast<ExifLong>(altitude * 1000), 1000});

    return true;
}

bool ExifUtilsImpl::setGpsLatitude(double latitude) {
    const ExifTag refTag = static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE_REF);
    std::unique_ptr<ExifEntry> refEntry =
            addVariableLengthEntry(EXIF_IFD_GPS, refTag, EXIF_FORMAT_ASCII, 2, 2);
    if (!refEntry) {
        ALOGE("%s: Adding GPSLatitudeRef exif entry failed", __FUNCTION__);
        return false;
    }
    if (latitude >= 0) {
        memcpy(refEntry->data, "N", sizeof("N"));
    } else {
        memcpy(refEntry->data, "S", sizeof("S"));
        latitude *= -1;
    }

    const ExifTag tag = static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE);
    std::unique_ptr<ExifEntry> entry = addVariableLengthEntry(
            EXIF_IFD_GPS, tag, EXIF_FORMAT_RATIONAL, 3, 3 * sizeof(ExifRational));
    if (!entry) {
        exif_content_remove_entry(exif_data_->ifd[EXIF_IFD_GPS], refEntry.get());
        ALOGE("%s: Adding GPSLatitude exif entry failed", __FUNCTION__);
        return false;
    }
    setLatitudeOrLongitudeData(entry->data, latitude);

    return true;
}

bool ExifUtilsImpl::setGpsLongitude(double longitude) {
    ExifTag refTag = static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE_REF);
    std::unique_ptr<ExifEntry> refEntry =
            addVariableLengthEntry(EXIF_IFD_GPS, refTag, EXIF_FORMAT_ASCII, 2, 2);
    if (!refEntry) {
        ALOGE("%s: Adding GPSLongitudeRef exif entry failed", __FUNCTION__);
        return false;
    }
    if (longitude >= 0) {
        memcpy(refEntry->data, "E", sizeof("E"));
    } else {
        memcpy(refEntry->data, "W", sizeof("W"));
        longitude *= -1;
    }

    ExifTag tag = static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE);
    std::unique_ptr<ExifEntry> entry = addVariableLengthEntry(
            EXIF_IFD_GPS, tag, EXIF_FORMAT_RATIONAL, 3, 3 * sizeof(ExifRational));
    if (!entry) {
        exif_content_remove_entry(exif_data_->ifd[EXIF_IFD_GPS], refEntry.get());
        ALOGE("%s: Adding GPSLongitude exif entry failed", __FUNCTION__);
        return false;
    }
    setLatitudeOrLongitudeData(entry->data, longitude);

    return true;
}

bool ExifUtilsImpl::setGpsProcessingMethod(const std::string& method) {
    std::string buffer =
            std::string(gExifAsciiPrefix, sizeof(gExifAsciiPrefix)) + method;
    SET_STRING(EXIF_IFD_GPS, static_cast<ExifTag>(EXIF_TAG_GPS_PROCESSING_METHOD),
            EXIF_FORMAT_UNDEFINED, buffer);
    return true;
}

bool ExifUtilsImpl::setGpsTimestamp(const struct tm& t) {
    const ExifTag dateTag = static_cast<ExifTag>(EXIF_TAG_GPS_DATE_STAMP);
    const size_t kGpsDateStampSize = 11;
    std::unique_ptr<ExifEntry> entry = addVariableLengthEntry(EXIF_IFD_GPS,
            dateTag, EXIF_FORMAT_ASCII, kGpsDateStampSize, kGpsDateStampSize);
    if (!entry) {
        ALOGE("%s: Adding GPSDateStamp exif entry failed", __FUNCTION__);
        return false;
    }
    int result = snprintf(reinterpret_cast<char*>(entry->data), kGpsDateStampSize,
            "%04i:%02i:%02i", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    if (result != kGpsDateStampSize - 1) {
        ALOGW("%s: Input time is invalid", __FUNCTION__);
        return false;
    }

    const ExifTag timeTag = static_cast<ExifTag>(EXIF_TAG_GPS_TIME_STAMP);
    entry = addVariableLengthEntry(EXIF_IFD_GPS, timeTag, EXIF_FORMAT_RATIONAL, 3,
            3 * sizeof(ExifRational));
    if (!entry) {
        ALOGE("%s: Adding GPSTimeStamp exif entry failed", __FUNCTION__);
        return false;
    }
    exif_set_rational(entry->data, EXIF_BYTE_ORDER_INTEL,
            {static_cast<ExifLong>(t.tm_hour), 1});
    exif_set_rational(entry->data + sizeof(ExifRational), EXIF_BYTE_ORDER_INTEL,
            {static_cast<ExifLong>(t.tm_min), 1});
    exif_set_rational(entry->data + 2 * sizeof(ExifRational), EXIF_BYTE_ORDER_INTEL,
            {static_cast<ExifLong>(t.tm_sec), 1});

    return true;
}

bool ExifUtilsImpl::setImageHeight(uint32_t length) {
    SET_SHORT(EXIF_IFD_0, EXIF_TAG_IMAGE_LENGTH, length);
    SET_LONG(EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION, length);
    return true;
}

bool ExifUtilsImpl::setImageWidth(uint32_t width) {
    SET_SHORT(EXIF_IFD_0, EXIF_TAG_IMAGE_WIDTH, width);
    SET_LONG(EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION, width);
    return true;
}

bool ExifUtilsImpl::setIsoSpeedRating(uint16_t iso_speed_ratings) {
    SET_SHORT(EXIF_IFD_EXIF, EXIF_TAG_ISO_SPEED_RATINGS, iso_speed_ratings);
    return true;
}

bool ExifUtilsImpl::setMaxAperture(float aperture) {
    float maxAperture = convertToApex(aperture);
    SET_RATIONAL(EXIF_IFD_EXIF, EXIF_TAG_MAX_APERTURE_VALUE,
            static_cast<uint32_t>(std::round(maxAperture * kRationalPrecision)),
            kRationalPrecision);
    return true;
}

bool ExifUtilsImpl::setExposureBias(int32_t ev,
        uint32_t ev_step_numerator, uint32_t ev_step_denominator) {
    SET_RATIONAL(EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_BIAS_VALUE,
            ev * ev_step_numerator, ev_step_denominator);
    return true;
}

bool ExifUtilsImpl::setOrientation(uint16_t degrees) {
    ExifOrientation value = ExifOrientation::ORIENTATION_0_DEGREES;
    switch (degrees) {
        case 90:
            value = ExifOrientation::ORIENTATION_90_DEGREES;
            break;
        case 180:
            value = ExifOrientation::ORIENTATION_180_DEGREES;
            break;
        case 270:
            value = ExifOrientation::ORIENTATION_270_DEGREES;
            break;
        default:
            break;
    }
    return setOrientationValue(value);
}

bool ExifUtilsImpl::setOrientationValue(ExifOrientation orientationValue) {
    SET_SHORT(EXIF_IFD_0, EXIF_TAG_ORIENTATION, orientationValue);
    return true;
}

bool ExifUtilsImpl::setShutterSpeed(float exposure_time) {
    float shutterSpeed = -log2f(exposure_time);
    SET_SRATIONAL(EXIF_IFD_EXIF, EXIF_TAG_SHUTTER_SPEED_VALUE,
            static_cast<uint32_t>(shutterSpeed * kRationalPrecision), kRationalPrecision);
    return true;
}

bool ExifUtilsImpl::setSubjectDistance(float diopters) {
    const static float kInfinityDiopters = 1.0e-6;
    uint32_t numerator, denominator;
    uint16_t distanceRange;
    if (diopters > kInfinityDiopters) {
        float focusDistance = 1.0f / diopters;
        numerator = static_cast<uint32_t>(std::round(focusDistance * kRationalPrecision));
        denominator = kRationalPrecision;

        if (focusDistance < 1.0f) {
            distanceRange = 1; // Macro
        } else if (focusDistance < 3.0f) {
            distanceRange = 2; // Close
        } else {
            distanceRange = 3; // Distant
        }
    } else {
        numerator = 0xFFFFFFFF;
        denominator = 1;
        distanceRange = 3; // Distant
    }
    SET_RATIONAL(EXIF_IFD_EXIF, EXIF_TAG_SUBJECT_DISTANCE, numerator, denominator);
    SET_SHORT(EXIF_IFD_EXIF, EXIF_TAG_SUBJECT_DISTANCE_RANGE, distanceRange);
    return true;
}

bool ExifUtilsImpl::setSubsecTime(const std::string& subsec_time) {
    SET_STRING(EXIF_IFD_EXIF, EXIF_TAG_SUB_SEC_TIME, EXIF_FORMAT_ASCII, subsec_time);
    SET_STRING(EXIF_IFD_EXIF, EXIF_TAG_SUB_SEC_TIME_ORIGINAL, EXIF_FORMAT_ASCII, subsec_time);
    SET_STRING(EXIF_IFD_EXIF, EXIF_TAG_SUB_SEC_TIME_DIGITIZED, EXIF_FORMAT_ASCII, subsec_time);
    return true;
}

bool ExifUtilsImpl::setWhiteBalance(uint8_t white_balance) {
    uint16_t whiteBalance = (white_balance == ANDROID_CONTROL_AWB_MODE_AUTO) ? 0 : 1;
    SET_SHORT(EXIF_IFD_EXIF, EXIF_TAG_WHITE_BALANCE, whiteBalance);
    return true;
}

bool ExifUtilsImpl::generateApp1() {
    destroyApp1();
    // Save the result into |app1_buffer_|.
    exif_data_save_data(exif_data_, &app1_buffer_, &app1_length_);
    if (!app1_length_) {
        ALOGE("%s: Allocate memory for app1_buffer_ failed", __FUNCTION__);
        return false;
    }
    /*
     * The JPEG segment size is 16 bits in spec. The size of APP1 segment should
     * be smaller than 65533 because there are two bytes for segment size field.
     */
    if (app1_length_ > 65533) {
        destroyApp1();
        ALOGE("%s: The size of APP1 segment is too large", __FUNCTION__);
        return false;
    }
    return true;
}

const uint8_t* ExifUtilsImpl::getApp1Buffer() {
    return app1_buffer_;
}

unsigned int ExifUtilsImpl::getApp1Length() {
    return app1_length_;
}

bool ExifUtilsImpl::setExifVersion(const std::string& exif_version) {
    SET_STRING(EXIF_IFD_EXIF, EXIF_TAG_EXIF_VERSION, EXIF_FORMAT_UNDEFINED, exif_version);
    return true;
}

void ExifUtilsImpl::reset() {
    destroyApp1();
    if (exif_data_) {
        /*
         * Since we decided to ignore the original APP1, we are sure that there is
         * no thumbnail allocated by libexif. |exif_data_->data| is actually
         * allocated by JpegCompressor. sets |exif_data_->data| to nullptr to
         * prevent exif_data_unref() destroy it incorrectly.
         */
        exif_data_->data = nullptr;
        exif_data_->size = 0;
        exif_data_unref(exif_data_);
        exif_data_ = nullptr;
    }
}

std::unique_ptr<ExifEntry> ExifUtilsImpl::addVariableLengthEntry(ExifIfd ifd,
        ExifTag tag, ExifFormat format, uint64_t components, unsigned int size) {
    // Remove old entry if exists.
    exif_content_remove_entry(exif_data_->ifd[ifd],
            exif_content_get_entry(exif_data_->ifd[ifd], tag));
    ExifMem* mem = exif_mem_new_default();
    if (!mem) {
        ALOGE("%s: Allocate memory for exif entry failed", __FUNCTION__);
        return nullptr;
    }
    std::unique_ptr<ExifEntry> entry(exif_entry_new_mem(mem));
    if (!entry) {
        ALOGE("%s: Allocate memory for exif entry failed", __FUNCTION__);
        exif_mem_unref(mem);
        return nullptr;
    }
    void* tmpBuffer = exif_mem_alloc(mem, size);
    if (!tmpBuffer) {
        ALOGE("%s: Allocate memory for exif entry failed", __FUNCTION__);
        exif_mem_unref(mem);
        return nullptr;
    }

    entry->data = static_cast<unsigned char*>(tmpBuffer);
    entry->tag = tag;
    entry->format = format;
    entry->components = components;
    entry->size = size;

    exif_content_add_entry(exif_data_->ifd[ifd], entry.get());
    exif_mem_unref(mem);

    return entry;
}

std::unique_ptr<ExifEntry> ExifUtilsImpl::addEntry(ExifIfd ifd, ExifTag tag) {
    std::unique_ptr<ExifEntry> entry(exif_content_get_entry(exif_data_->ifd[ifd], tag));
    if (entry) {
        // exif_content_get_entry() won't ref the entry, so we ref here.
        exif_entry_ref(entry.get());
        return entry;
    }
    entry.reset(exif_entry_new());
    if (!entry) {
        ALOGE("%s: Allocate memory for exif entry failed", __FUNCTION__);
        return nullptr;
    }
    entry->tag = tag;
    exif_content_add_entry(exif_data_->ifd[ifd], entry.get());
    exif_entry_initialize(entry.get(), tag);
    return entry;
}

bool ExifUtilsImpl::setShort(ExifIfd ifd, ExifTag tag, uint16_t value, const std::string& msg) {
    std::unique_ptr<ExifEntry> entry = addEntry(ifd, tag);
    if (!entry) {
        ALOGE("%s: Adding '%s' entry failed", __FUNCTION__, msg.c_str());
        return false;
    }
    exif_set_short(entry->data, EXIF_BYTE_ORDER_INTEL, value);
    return true;
}

bool ExifUtilsImpl::setLong(ExifIfd ifd, ExifTag tag, uint32_t value, const std::string& msg) {
    std::unique_ptr<ExifEntry> entry = addEntry(ifd, tag);
    if (!entry) {
        ALOGE("%s: Adding '%s' entry failed", __FUNCTION__, msg.c_str());
        return false;
    }
    exif_set_long(entry->data, EXIF_BYTE_ORDER_INTEL, value);
    return true;
}

bool ExifUtilsImpl::setRational(ExifIfd ifd, ExifTag tag, uint32_t numerator,
        uint32_t denominator, const std::string& msg) {
    std::unique_ptr<ExifEntry> entry = addEntry(ifd, tag);
    if (!entry) {
        ALOGE("%s: Adding '%s' entry failed", __FUNCTION__, msg.c_str());
        return false;
    }
    exif_set_rational(entry->data, EXIF_BYTE_ORDER_INTEL, {numerator, denominator});
    return true;
}

bool ExifUtilsImpl::setSRational(ExifIfd ifd, ExifTag tag, int32_t numerator,
        int32_t denominator, const std::string& msg) {
    std::unique_ptr<ExifEntry> entry = addEntry(ifd, tag);
    if (!entry) {
        ALOGE("%s: Adding '%s' entry failed", __FUNCTION__, msg.c_str());
        return false;
    }
    exif_set_srational(entry->data, EXIF_BYTE_ORDER_INTEL, {numerator, denominator});
    return true;
}

bool ExifUtilsImpl::setString(ExifIfd ifd, ExifTag tag, ExifFormat format,
        const std::string& buffer, const std::string& msg) {
    size_t entry_size = buffer.length();
    // Since the exif format is undefined, NULL termination is not necessary.
    if (format == EXIF_FORMAT_ASCII) {
        entry_size++;
    }
    std::unique_ptr<ExifEntry> entry =
            addVariableLengthEntry(ifd, tag, format, entry_size, entry_size);
    if (!entry) {
        ALOGE("%s: Adding '%s' entry failed", __FUNCTION__, msg.c_str());
        return false;
    }
    memcpy(entry->data, buffer.c_str(), entry_size);
    return true;
}

void ExifUtilsImpl::destroyApp1() {
    /*
     * Since there is no API to access ExifMem in ExifData->priv, we use free
     * here, which is the default free function in libexif. See
     * exif_data_save_data() for detail.
     */
    free(app1_buffer_);
    app1_buffer_ = nullptr;
    app1_length_ = 0;
}

bool ExifUtilsImpl::setFromMetadata(const CameraMetadata& metadata,
        const CameraMetadata& staticInfo,
        const size_t imageWidth, const size_t imageHeight) {
    if (!setImageWidth(imageWidth) ||
            !setImageHeight(imageHeight)) {
        ALOGE("%s: setting image resolution failed.", __FUNCTION__);
        return false;
    }

    struct timespec tp;
    struct tm time_info;
    bool time_available = clock_gettime(CLOCK_REALTIME, &tp) != -1;
    localtime_r(&tp.tv_sec, &time_info);
    if (!setDateTime(time_info)) {
        ALOGE("%s: setting data time failed.", __FUNCTION__);
        return false;
    }

    float focal_length;
    camera_metadata_ro_entry entry = metadata.find(ANDROID_LENS_FOCAL_LENGTH);
    if (entry.count) {
        focal_length = entry.data.f[0];

        if (!setFocalLength(focal_length)) {
            ALOGE("%s: setting focal length failed.", __FUNCTION__);
            return false;
        }

        camera_metadata_ro_entry sensorSizeEntry =
                staticInfo.find(ANDROID_SENSOR_INFO_PHYSICAL_SIZE);
        if (sensorSizeEntry.count == 2) {
            if (!setFocalLengthIn35mmFilm(
                    focal_length, sensorSizeEntry.data.f[0], sensorSizeEntry.data.f[1])) {
                ALOGE("%s: setting focal length in 35mm failed.", __FUNCTION__);
                return false;
            }
        }
    } else {
        ALOGV("%s: Cannot find focal length in metadata.", __FUNCTION__);
    }

    if (metadata.exists(ANDROID_SCALER_CROP_REGION) &&
            staticInfo.exists(ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE)) {
        entry = metadata.find(ANDROID_SCALER_CROP_REGION);
        camera_metadata_ro_entry activeArrayEntry =
                staticInfo.find(ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE);

        if (!setDigitalZoomRatio(entry.data.i32[2], entry.data.i32[3],
                activeArrayEntry.data.i32[2], activeArrayEntry.data.i32[3])) {
            ALOGE("%s: setting digital zoom ratio failed.", __FUNCTION__);
            return false;
        }
    }

    if (metadata.exists(ANDROID_JPEG_GPS_COORDINATES)) {
        entry = metadata.find(ANDROID_JPEG_GPS_COORDINATES);
        if (entry.count < 3) {
            ALOGE("%s: Gps coordinates in metadata is not complete.", __FUNCTION__);
            return false;
        }
        if (!setGpsLatitude(entry.data.d[0])) {
            ALOGE("%s: setting gps latitude failed.", __FUNCTION__);
            return false;
        }
        if (!setGpsLongitude(entry.data.d[1])) {
            ALOGE("%s: setting gps longitude failed.", __FUNCTION__);
            return false;
        }
        if (!setGpsAltitude(entry.data.d[2])) {
            ALOGE("%s: setting gps altitude failed.", __FUNCTION__);
            return false;
        }
    }

    if (metadata.exists(ANDROID_JPEG_GPS_PROCESSING_METHOD)) {
        entry = metadata.find(ANDROID_JPEG_GPS_PROCESSING_METHOD);
        std::string method_str(reinterpret_cast<const char*>(entry.data.u8));
        if (!setGpsProcessingMethod(method_str)) {
            ALOGE("%s: setting gps processing method failed.", __FUNCTION__);
            return false;
        }
    }

    if (time_available && metadata.exists(ANDROID_JPEG_GPS_TIMESTAMP)) {
        entry = metadata.find(ANDROID_JPEG_GPS_TIMESTAMP);
        time_t timestamp = static_cast<time_t>(entry.data.i64[0]);
        if (gmtime_r(&timestamp, &time_info)) {
            if (!setGpsTimestamp(time_info)) {
                ALOGE("%s: setting gps timestamp failed.", __FUNCTION__);
                return false;
            }
        } else {
            ALOGE("%s: Time tranformation failed.", __FUNCTION__);
            return false;
        }
    }

    if (staticInfo.exists(ANDROID_CONTROL_AE_COMPENSATION_STEP) &&
            metadata.exists(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION)) {
        entry = metadata.find(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION);
        camera_metadata_ro_entry stepEntry =
                staticInfo.find(ANDROID_CONTROL_AE_COMPENSATION_STEP);
        if (!setExposureBias(entry.data.i32[0], stepEntry.data.r[0].numerator,
                stepEntry.data.r[0].denominator)) {
            ALOGE("%s: setting exposure bias failed.", __FUNCTION__);
            return false;
        }
    }

    if (metadata.exists(ANDROID_JPEG_ORIENTATION)) {
        entry = metadata.find(ANDROID_JPEG_ORIENTATION);
        if (!setOrientation(entry.data.i32[0])) {
            ALOGE("%s: setting orientation failed.", __FUNCTION__);
            return false;
        }
    }

    if (metadata.exists(ANDROID_SENSOR_EXPOSURE_TIME)) {
        entry = metadata.find(ANDROID_SENSOR_EXPOSURE_TIME);
        float exposure_time = 1.0f * entry.data.i64[0] / 1e9;
        if (!setExposureTime(exposure_time)) {
            ALOGE("%s: setting exposure time failed.", __FUNCTION__);
            return false;
        }

        if (!setShutterSpeed(exposure_time)) {
            ALOGE("%s: setting shutter speed failed.", __FUNCTION__);
            return false;
        }
    }

    if (metadata.exists(ANDROID_LENS_FOCUS_DISTANCE)) {
        entry = metadata.find(ANDROID_LENS_FOCUS_DISTANCE);
        if (!setSubjectDistance(entry.data.f[0])) {
            ALOGE("%s: setting subject distance failed.", __FUNCTION__);
            return false;
        }
    }

    if (metadata.exists(ANDROID_SENSOR_SENSITIVITY)) {
        entry = metadata.find(ANDROID_SENSOR_SENSITIVITY);
        int32_t iso = entry.data.i32[0];
        camera_metadata_ro_entry postRawSensEntry =
                metadata.find(ANDROID_CONTROL_POST_RAW_SENSITIVITY_BOOST);
        if (postRawSensEntry.count > 0) {
            iso = iso * postRawSensEntry.data.i32[0] / 100;
        }

        if (!setIsoSpeedRating(static_cast<uint16_t>(iso))) {
            ALOGE("%s: setting iso rating failed.", __FUNCTION__);
            return false;
        }
    }

    if (metadata.exists(ANDROID_LENS_APERTURE)) {
        entry = metadata.find(ANDROID_LENS_APERTURE);
        if (!setFNumber(entry.data.f[0])) {
            ALOGE("%s: setting F number failed.", __FUNCTION__);
            return false;
        }
        if (!setAperture(entry.data.f[0])) {
            ALOGE("%s: setting aperture failed.", __FUNCTION__);
            return false;
        }
    }

    static const uint16_t kSRGBColorSpace = 1;
    if (!setColorSpace(kSRGBColorSpace)) {
        ALOGE("%s: setting color space failed.", __FUNCTION__);
        return false;
    }

    if (staticInfo.exists(ANDROID_LENS_INFO_AVAILABLE_APERTURES)) {
        entry = staticInfo.find(ANDROID_LENS_INFO_AVAILABLE_APERTURES);
        if (!setMaxAperture(entry.data.f[0])) {
            ALOGE("%s: setting max aperture failed.", __FUNCTION__);
            return false;
        }
    }

    if (staticInfo.exists(ANDROID_FLASH_INFO_AVAILABLE)) {
        entry = staticInfo.find(ANDROID_FLASH_INFO_AVAILABLE);
        camera_metadata_ro_entry flashStateEntry = metadata.find(ANDROID_FLASH_STATE);
        camera_metadata_ro_entry aeModeEntry = metadata.find(ANDROID_CONTROL_AE_MODE);
        uint8_t flashState = flashStateEntry.count > 0 ?
                flashStateEntry.data.u8[0] : ANDROID_FLASH_STATE_UNAVAILABLE;
        uint8_t aeMode = aeModeEntry.count > 0 ?
                aeModeEntry.data.u8[0] : ANDROID_CONTROL_AE_MODE_OFF;

        if (!setFlash(entry.data.u8[0], flashState, aeMode)) {
            ALOGE("%s: setting flash failed.", __FUNCTION__);
            return false;
        }
    }

    if (metadata.exists(ANDROID_CONTROL_AWB_MODE)) {
        entry = metadata.find(ANDROID_CONTROL_AWB_MODE);
        if (!setWhiteBalance(entry.data.u8[0])) {
            ALOGE("%s: setting white balance failed.", __FUNCTION__);
            return false;
        }
    }

    if (metadata.exists(ANDROID_CONTROL_AE_MODE)) {
        entry = metadata.find(ANDROID_CONTROL_AE_MODE);
        if (!setExposureMode(entry.data.u8[0])) {
            ALOGE("%s: setting exposure mode failed.", __FUNCTION__);
            return false;
        }
    }
    if (time_available) {
        char str[4];
        if (snprintf(str, sizeof(str), "%03ld", tp.tv_nsec / 1000000) < 0) {
            ALOGE("%s: Subsec is invalid: %ld", __FUNCTION__, tp.tv_nsec);
            return false;
        }
        if (!setSubsecTime(std::string(str))) {
            ALOGE("%s: setting subsec time failed.", __FUNCTION__);
            return false;
        }
    }

    return true;
}

} // namespace camera3
} // namespace android
