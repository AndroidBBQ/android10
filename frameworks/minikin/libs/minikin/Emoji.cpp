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

#include "minikin/Emoji.h"

namespace minikin {

bool isNewEmoji(uint32_t c) {
    // Emoji characters new in Unicode emoji 12
    // From https://www.unicode.org/Public/emoji/12.0/emoji-data.txt
    // TODO: Remove once emoji-data.text 12 is in ICU or update to 12.
    if (c < 0x1F6D5 || c > 0x1FA95) {
        // Optimization for characters outside the new emoji range.
        return false;
    }
    return c == 0x1F6D5 || c == 0x1F6FA || c == 0x1F93F || c == 0x1F971 || c == 0x1F97B ||
           (0x1F7E0 <= c && c <= 0x1F7EB) || (0x1F90D <= c && c <= 0x1F90F) ||
           (0x1F9A5 <= c && c <= 0x1F9AA) || (0x1F9AE <= c && c <= 0x1F9AF) ||
           (0x1F9BA <= c && c <= 0x1F9BF) || (0x1F9C3 <= c && c <= 0x1F9CA) ||
           (0x1F9CD <= c && c <= 0x1F9CF) || (0x1FA70 <= c && c <= 0x1FA73) ||
           (0x1FA78 <= c && c <= 0x1FA7A) || (0x1FA80 <= c && c <= 0x1FA82) ||
           (0x1FA90 <= c && c <= 0x1FA95);
}

bool isEmoji(uint32_t c) {
    return isNewEmoji(c) || u_hasBinaryProperty(c, UCHAR_EMOJI);
}

bool isEmojiModifier(uint32_t c) {
    // Emoji modifier are not expected to change, so there's a small change we need to customize
    // this.
    return u_hasBinaryProperty(c, UCHAR_EMOJI_MODIFIER);
}

bool isEmojiBase(uint32_t c) {
    // These two characters were removed from Emoji_Modifier_Base in Emoji 4.0, but we need to keep
    // them as emoji modifier bases since there are fonts and user-generated text out there that
    // treats these as potential emoji bases.
    if (c == 0x1F91D || c == 0x1F93C) {
        return true;
    }
    // Emoji Modifier Base characters new in Unicode emoji 11
    // From https://www.unicode.org/Public/emoji/11.0/emoji-data.txt
    // TODO: Remove once emoji-data.text 11 is in ICU or update to 11.
    if ((0x1F9B5 <= c && c <= 0x1F9B6) || (0x1F9B8 <= c && c <= 0x1F9B9)) {
        return true;
    }
    return u_hasBinaryProperty(c, UCHAR_EMOJI_MODIFIER_BASE);
}

UCharDirection emojiBidiOverride(const void* /* context */, UChar32 c) {
    if (isNewEmoji(c)) {
        // All new emoji characters in Unicode 10.0 are of the bidi class ON.
        return U_OTHER_NEUTRAL;
    } else {
        return u_charDirection(c);
    }
}

}  // namespace minikin
