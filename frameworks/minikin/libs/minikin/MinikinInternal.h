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

// Definitions internal to Minikin

#ifndef MINIKIN_INTERNAL_H
#define MINIKIN_INTERNAL_H

#include <hb.h>
#include <utils/Log.h>
#include <utils/Mutex.h>

#include "minikin/HbUtils.h"
#include "minikin/MinikinFont.h"

namespace minikin {

#ifdef ENABLE_ASSERTION
#define MINIKIN_ASSERT(cond, ...) LOG_ALWAYS_FATAL_IF(!(cond), __VA_ARGS__)
#else
#define MINIKIN_ASSERT(cond, ...) ((void)0)
#endif

#define MINIKIN_NOT_REACHED(...) MINIKIN_ASSERT(false, __VA_ARGS__);

constexpr uint32_t MAX_UNICODE_CODE_POINT = 0x10FFFF;

constexpr uint32_t VS1 = 0xFE00;
constexpr uint32_t VS16 = 0xFE0F;
constexpr uint32_t VS17 = 0xE0100;
constexpr uint32_t VS256 = 0xE01EF;

// Returns variation selector index. This is one unit less than the variation selector number. For
// example, VARIATION SELECTOR-25 maps to 24.
// [0x00-0x0F] for U+FE00..U+FE0F
// [0x10-0xFF] for U+E0100..U+E01EF
// INVALID_VS_INDEX for other input.
constexpr uint16_t INVALID_VS_INDEX = 0xFFFF;
uint16_t getVsIndex(uint32_t codePoint);

// Returns true if the code point is a variation selector.
// Note that this function returns false for Mongolian free variation selectors.
bool isVariationSelector(uint32_t codePoint);

// An RAII accessor for hb_blob_t
class HbBlob {
public:
    HbBlob(const HbFaceUniquePtr& face, uint32_t tag)
            : mBlob(hb_face_reference_table(face.get(), tag)) {}
    HbBlob(const HbFontUniquePtr& font, uint32_t tag)
            : mBlob(hb_face_reference_table(hb_font_get_face(font.get()), tag)) {}

    inline const uint8_t* get() const {
        return reinterpret_cast<const uint8_t*>(hb_blob_get_data(mBlob.get(), nullptr));
    }

    inline size_t size() const { return (size_t)hb_blob_get_length(mBlob.get()); }

    inline operator bool() const { return size() > 0; }

private:
    HbBlobUniquePtr mBlob;
};

}  // namespace minikin

#endif  // MINIKIN_INTERNAL_H
