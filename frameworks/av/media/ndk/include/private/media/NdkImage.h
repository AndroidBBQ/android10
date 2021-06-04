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

#ifndef _PRIVATE_MEDIA_NDKIMAGE_H_
#define _PRIVATE_MEDIA_NDKIMAGE_H_
// Formats not listed in the public API, but still available to AImageReader
enum AIMAGE_PRIVATE_FORMATS {
    /**
     * Unprocessed implementation-dependent raw
     * depth measurements, opaque with 16 bit
     * samples.
     *
     */

    AIMAGE_FORMAT_RAW_DEPTH = 0x1002,
};
#endif // _PRIVATE_MEDIA_NDKIMAGE
