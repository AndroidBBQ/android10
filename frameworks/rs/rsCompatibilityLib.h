/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef _RS_RSCOMPATIBILITYLIB_H
#define _RS_RSCOMPATIBILITYLIB_H

#ifndef RS_COMPATIBILITY_LIB
#error "THIS HEADER IS ONLY VALID FOR BUILDING OUR COMPATIBILITY LIBRARY"
#endif

// from system/graphics.h
enum {
    HAL_PIXEL_FORMAT_YV12   = 0x32315659, // YCrCb 4:2:0 Planar
    HAL_PIXEL_FORMAT_YCrCb_420_SP       = 0x11, // NV21
};

#endif  // _RS_RSCOMPATIBILITYLIB_H
