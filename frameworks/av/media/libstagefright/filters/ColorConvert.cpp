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

#include "ColorConvert.h"

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

namespace android {

void YUVToRGB(
        int32_t y, int32_t u, int32_t v,
        int32_t* r, int32_t* g, int32_t* b) {
    y -= 16;
    u -= 128;
    v -= 128;

    *b = 1192 * y + 2066 * u;
    *g = 1192 * y - 833 * v - 400 * u;
    *r = 1192 * y + 1634 * v;

    *r = min(262143, max(0, *r));
    *g = min(262143, max(0, *g));
    *b = min(262143, max(0, *b));

    *r >>= 10;
    *g >>= 10;
    *b >>= 10;
}

void convertYUV420spToARGB(
        uint8_t *pY, uint8_t *pUV, int32_t width, int32_t height,
        uint8_t *dest) {
    const int32_t bytes_per_pixel = 2;

    for (int32_t i = 0; i < height; i++) {
        for (int32_t j = 0; j < width; j++) {
            int32_t y = *(pY + i * width + j);
            int32_t u = *(pUV + (i/2) * width + bytes_per_pixel * (j/2));
            int32_t v = *(pUV + (i/2) * width + bytes_per_pixel * (j/2) + 1);

            int32_t r, g, b;
            YUVToRGB(y, u, v, &r, &g, &b);

            *dest++ = 0xFF;
            *dest++ = r;
            *dest++ = g;
            *dest++ = b;
        }
    }
}

void convertYUV420spToRGB888(
        uint8_t *pY, uint8_t *pUV, int32_t width, int32_t height,
        uint8_t *dest) {
    const int32_t bytes_per_pixel = 2;

    for (int32_t i = 0; i < height; i++) {
        for (int32_t j = 0; j < width; j++) {
            int32_t y = *(pY + i * width + j);
            int32_t u = *(pUV + (i/2) * width + bytes_per_pixel * (j/2));
            int32_t v = *(pUV + (i/2) * width + bytes_per_pixel * (j/2) + 1);

            int32_t r, g, b;
            YUVToRGB(y, u, v, &r, &g, &b);

            *dest++ = r;
            *dest++ = g;
            *dest++ = b;
        }
    }
}

// HACK - not even slightly optimized
// TODO: remove when RGBA support is added to SoftwareRenderer
void convertRGBAToARGB(
        uint8_t *src, int32_t width, int32_t height, uint32_t stride,
        uint8_t *dest) {
    for (int32_t i = 0; i < height; ++i) {
        for (int32_t j = 0; j < width; ++j) {
            uint8_t r = *src++;
            uint8_t g = *src++;
            uint8_t b = *src++;
            uint8_t a = *src++;
            *dest++ = a;
            *dest++ = r;
            *dest++ = g;
            *dest++ = b;
        }
        src += (stride - width) * 4;
    }
}

}   // namespace android
