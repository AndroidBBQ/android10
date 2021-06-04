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

#ifndef MINIKIN_MINIKIN_PAINT_H
#define MINIKIN_MINIKIN_PAINT_H

#include <memory>
#include <string>

#include "minikin/FamilyVariant.h"
#include "minikin/FontCollection.h"
#include "minikin/FontFamily.h"
#include "minikin/Hasher.h"

namespace minikin {

class FontCollection;

// These describe what is stored in MinikinPaint.fontFlags.
enum MinikinFontFlags {
    Embolden_Shift = 0,
    LinearMetrics_Shift = 1,
    Subpixel_Shift = 2,
    EmbeddedBitmaps_Shift = 3,
    ForceAutoHinting_Shift = 4,

    Embolden_Flag = 1 << Embolden_Shift,
    LinearMetrics_Flag = 1 << LinearMetrics_Shift,
    Subpixel_Flag = 1 << Subpixel_Shift,
    EmbeddedBitmaps_Flag = 1 << EmbeddedBitmaps_Shift,
    ForceAutoHinting_Flag = 1 << ForceAutoHinting_Shift,
};

// Possibly move into own .h file?
// Note: if you add a field here, either add it to LayoutCacheKey or to skipCache()
struct MinikinPaint {
    MinikinPaint(const std::shared_ptr<FontCollection>& font)
            : size(0),
              scaleX(0),
              skewX(0),
              letterSpacing(0),
              wordSpacing(0),
              fontFlags(0),
              localeListId(0),
              familyVariant(FamilyVariant::DEFAULT),
              fontFeatureSettings(),
              font(font) {}

    bool skipCache() const { return !fontFeatureSettings.empty(); }

    float size;
    float scaleX;
    float skewX;
    float letterSpacing;
    float wordSpacing;
    uint32_t fontFlags;
    uint32_t localeListId;
    FontStyle fontStyle;
    FamilyVariant familyVariant;
    std::string fontFeatureSettings;
    std::shared_ptr<FontCollection> font;

    void copyFrom(const MinikinPaint& paint) { *this = paint; }

    MinikinPaint(const MinikinPaint&) = default;
    MinikinPaint& operator=(const MinikinPaint&) = default;

    MinikinPaint(MinikinPaint&&) = default;
    MinikinPaint& operator=(MinikinPaint&&) = default;

    inline bool operator==(const MinikinPaint& paint) const {
        return size == paint.size && scaleX == paint.scaleX && skewX == paint.skewX &&
               letterSpacing == paint.letterSpacing && wordSpacing == paint.wordSpacing &&
               fontFlags == paint.fontFlags && localeListId == paint.localeListId &&
               fontStyle == paint.fontStyle && familyVariant == paint.familyVariant &&
               fontFeatureSettings == paint.fontFeatureSettings && font.get() == paint.font.get();
    }

    uint32_t hash() const {
        return Hasher()
                .update(size)
                .update(scaleX)
                .update(skewX)
                .update(letterSpacing)
                .update(wordSpacing)
                .update(fontFlags)
                .update(localeListId)
                .update(fontStyle.identifier())
                .update(static_cast<uint8_t>(familyVariant))
                .updateString(fontFeatureSettings)
                .update(font->getId())
                .hash();
    }
};

}  // namespace minikin

#endif  // MINIKIN_MINIKIN_PAINT_H
