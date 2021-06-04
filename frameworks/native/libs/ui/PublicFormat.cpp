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

#include <ui/GraphicTypes.h> // ui::Dataspace
#include <ui/PublicFormat.h>

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

using ui::Dataspace;

int mapPublicFormatToHalFormat(PublicFormat f) {
    switch (f) {
        case PublicFormat::JPEG:
        case PublicFormat::DEPTH_POINT_CLOUD:
        case PublicFormat::DEPTH_JPEG:
        case PublicFormat::HEIC:
            return HAL_PIXEL_FORMAT_BLOB;
        case PublicFormat::DEPTH16:
            return HAL_PIXEL_FORMAT_Y16;
        case PublicFormat::RAW_SENSOR:
        case PublicFormat::RAW_DEPTH:
            return HAL_PIXEL_FORMAT_RAW16;
        default:
            // Most formats map 1:1
            return static_cast<int>(f);
    }
}

android_dataspace mapPublicFormatToHalDataspace(PublicFormat f) {
    Dataspace dataspace;
    switch (f) {
        case PublicFormat::JPEG:
            dataspace = Dataspace::V0_JFIF;
            break;
        case PublicFormat::DEPTH_POINT_CLOUD:
        case PublicFormat::DEPTH16:
        case PublicFormat::RAW_DEPTH:
            dataspace = Dataspace::DEPTH;
            break;
        case PublicFormat::RAW_SENSOR:
        case PublicFormat::RAW_PRIVATE:
        case PublicFormat::RAW10:
        case PublicFormat::RAW12:
            dataspace = Dataspace::ARBITRARY;
            break;
        case PublicFormat::YUV_420_888:
        case PublicFormat::NV21:
        case PublicFormat::YV12:
            dataspace = Dataspace::V0_JFIF;
            break;
        case PublicFormat::DEPTH_JPEG:
            dataspace = Dataspace::DYNAMIC_DEPTH;
            break;
        case PublicFormat::HEIC:
            dataspace = Dataspace::HEIF;
            break;
        default:
            // Most formats map to UNKNOWN
            dataspace = Dataspace::UNKNOWN;
            break;
    }
    return static_cast<android_dataspace>(dataspace);
}

PublicFormat mapHalFormatDataspaceToPublicFormat(int format, android_dataspace dataSpace) {
    Dataspace ds = static_cast<Dataspace>(dataSpace);
    switch (format) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGBA_FP16:
        case HAL_PIXEL_FORMAT_RGBA_1010102:
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_Y8:
        case HAL_PIXEL_FORMAT_RAW10:
        case HAL_PIXEL_FORMAT_RAW12:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
        case HAL_PIXEL_FORMAT_YV12:
            // Enums overlap in both name and value
            return static_cast<PublicFormat>(format);
        case HAL_PIXEL_FORMAT_RAW16:
            switch (ds) {
                case Dataspace::DEPTH:
                    return PublicFormat::RAW_DEPTH;
                default:
                    return PublicFormat::RAW_SENSOR;
            }
        case HAL_PIXEL_FORMAT_RAW_OPAQUE:
            // Name differs, though value is the same
            return PublicFormat::RAW_PRIVATE;
        case HAL_PIXEL_FORMAT_YCbCr_422_SP:
            // Name differs, though the value is the same
            return PublicFormat::NV16;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            // Name differs, though the value is the same
            return PublicFormat::NV21;
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            // Name differs, though the value is the same
            return PublicFormat::YUY2;
        case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
            // Name differs, though the value is the same
            return PublicFormat::PRIVATE;
        case HAL_PIXEL_FORMAT_Y16:
            // Dataspace-dependent
            switch (ds) {
                case Dataspace::DEPTH:
                    return PublicFormat::DEPTH16;
                default:
                    // Assume non-depth Y16 is just Y16.
                    return PublicFormat::Y16;
            }
        case HAL_PIXEL_FORMAT_BLOB:
            // Dataspace-dependent
            switch (ds) {
                case Dataspace::DEPTH:
                    return PublicFormat::DEPTH_POINT_CLOUD;
                case Dataspace::V0_JFIF:
                    return PublicFormat::JPEG;
                case Dataspace::HEIF:
                    return PublicFormat::HEIC;
                default:
                    if (dataSpace == static_cast<android_dataspace>(HAL_DATASPACE_DYNAMIC_DEPTH)) {
                        return PublicFormat::DEPTH_JPEG;
                    } else {
                        // Assume otherwise-marked blobs are also JPEG
                        return PublicFormat::JPEG;
                    }
            }
        case HAL_PIXEL_FORMAT_BGRA_8888:
            // Not defined in public API
            return PublicFormat::UNKNOWN;

        default:
            return PublicFormat::UNKNOWN;
    }
}

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------
