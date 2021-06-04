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

#ifndef UI_PUBLICFORMAT_H
#define UI_PUBLICFORMAT_H

#include <system/graphics.h>

namespace android {

/**
 * Enum mirroring the public API definitions for image and pixel formats.
 * Some of these are hidden in the public API
 *
 * Keep up to date with android.graphics.ImageFormat and
 * android.graphics.PixelFormat
 *
 * TODO: PublicFormat is going to be deprecated(b/126594675)
 */
enum class PublicFormat {
    UNKNOWN = 0x0,
    RGBA_8888 = 0x1,
    RGBX_8888 = 0x2,
    RGB_888 = 0x3,
    RGB_565 = 0x4,
    NV16 = 0x10,
    NV21 = 0x11,
    YUY2 = 0x14,
    RGBA_FP16 = 0x16,
    RAW_SENSOR = 0x20,
    PRIVATE = 0x22,
    YUV_420_888 = 0x23,
    RAW_PRIVATE = 0x24,
    RAW10 = 0x25,
    RAW12 = 0x26,
    RGBA_1010102 = 0x2b,
    JPEG = 0x100,
    DEPTH_POINT_CLOUD = 0x101,
    RAW_DEPTH = 0x1002, // @hide
    YV12 = 0x32315659,
    Y8 = 0x20203859,
    Y16 = 0x20363159, // @hide
    DEPTH16 = 0x44363159,
    DEPTH_JPEG = 0x69656963,
    HEIC = 0x48454946,
};

/* Convert from android.graphics.ImageFormat/PixelFormat enums to graphics.h HAL
 * format */
extern int mapPublicFormatToHalFormat(PublicFormat f);

/* Convert from android.graphics.ImageFormat/PixelFormat enums to graphics.h HAL
 * dataspace */
extern android_dataspace mapPublicFormatToHalDataspace(PublicFormat f);

/* Convert from HAL format, dataspace pair to
 * android.graphics.ImageFormat/PixelFormat.
 * For unknown/unspecified pairs, returns PublicFormat::UNKNOWN */
extern PublicFormat mapHalFormatDataspaceToPublicFormat(int format, android_dataspace dataSpace);

}; // namespace android

#endif // UI_PUBLICFORMAT_H
