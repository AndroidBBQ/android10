/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "LayoutUtils.h"

namespace minikin {

/*
 * Determine whether the code unit is a word space for the purposes of justification.
 * TODO: Support NBSP and other stretchable whitespace (b/34013491 and b/68204709).
 */
bool isWordSpace(uint16_t code_unit) {
    return code_unit == ' ';
}

/**
 * For the purpose of layout, a word break is a boundary with no
 * kerning or complex script processing. This is necessarily a
 * heuristic, but should be accurate most of the time.
 */
static bool isWordBreakAfter(uint16_t c) {
    if (c == ' ' || (0x2000 <= c && c <= 0x200A) || c == 0x3000) {
        // spaces
        return true;
    }
    // Note: kana is not included, as sophisticated fonts may kern kana
    return false;
}

static bool isWordBreakBefore(uint16_t c) {
    // CJK ideographs (and yijing hexagram symbols)
    return isWordBreakAfter(c) || (0x3400 <= c && c <= 0x9FFF);
}

/**
 * Return offset of previous word break. It is either < offset or == 0.
 */
uint32_t getPrevWordBreakForCache(const U16StringPiece& textBuf, uint32_t offset) {
    if (offset == 0) return 0;
    if (offset > textBuf.size()) offset = textBuf.size();
    if (isWordBreakBefore(textBuf[offset - 1])) {
        return offset - 1;
    }
    for (uint32_t i = offset - 1; i > 0; i--) {
        if (isWordBreakBefore(textBuf[i]) || isWordBreakAfter(textBuf[i - 1])) {
            return i;
        }
    }
    return 0;
}

/**
 * Return offset of next word break. It is either > offset or == len.
 */
uint32_t getNextWordBreakForCache(const U16StringPiece& textBuf, uint32_t offset) {
    if (offset >= textBuf.size()) return textBuf.size();
    if (isWordBreakAfter(textBuf[offset])) {
        return offset + 1;
    }
    for (uint32_t i = offset + 1; i < textBuf.size(); i++) {
        // No need to check isWordBreakAfter(chars[i - 1]) since it is checked
        // in previous iteration.  Note that isWordBreakBefore returns true
        // whenever isWordBreakAfter returns true.
        if (isWordBreakBefore(textBuf[i])) {
            return i;
        }
    }
    return textBuf.size();
}

}  // namespace minikin
