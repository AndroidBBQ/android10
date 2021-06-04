/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef MINIKIN_HB_UTILS_H
#define MINIKIN_HB_UTILS_H

#include <hb.h>
#include <cmath>
#include <memory>

namespace minikin {

inline float HBFixedToFloat(hb_position_t v) {
    return scalbnf(v, -8);
}

inline hb_position_t HBFloatToFixed(float v) {
    return scalbnf(v, +8);
}

struct HbBlobDeleter {
    void operator()(hb_blob_t* v) { hb_blob_destroy(v); }
};

struct HbFaceDeleter {
    void operator()(hb_face_t* v) { hb_face_destroy(v); }
};

struct HbFontDeleter {
    void operator()(hb_font_t* v) { hb_font_destroy(v); }
};

struct HbBufferDeleter {
    void operator()(hb_buffer_t* v) { hb_buffer_destroy(v); }
};

using HbBlobUniquePtr = std::unique_ptr<hb_blob_t, HbBlobDeleter>;
using HbFaceUniquePtr = std::unique_ptr<hb_face_t, HbFaceDeleter>;
using HbFontUniquePtr = std::unique_ptr<hb_font_t, HbFontDeleter>;
using HbBufferUniquePtr = std::unique_ptr<hb_buffer_t, HbBufferDeleter>;

}  // namespace minikin

#endif  // MINIKIN_HB_UTILS_H
