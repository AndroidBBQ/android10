/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef COLOR_CONVERT_H_
#define COLOR_CONVERT_H_

#include <inttypes.h>

namespace android {

void YUVToRGB(
        int32_t y, int32_t u, int32_t v,
        int32_t* r, int32_t* g, int32_t* b);

void convertYUV420spToARGB(
        uint8_t *pY, uint8_t *pUV, int32_t width, int32_t height,
        uint8_t *dest);

void convertYUV420spToRGB888(
        uint8_t *pY, uint8_t *pUV, int32_t width, int32_t height,
        uint8_t *dest);

// TODO: remove when RGBA support is added to SoftwareRenderer
void convertRGBAToARGB(
        uint8_t *src, int32_t width, int32_t height, uint32_t stride,
        uint8_t *dest);

}   // namespace android

#endif  // COLOR_CONVERT_H_
