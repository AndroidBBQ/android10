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

#ifndef MINIKIN_FONT_FAMILY_H
#define MINIKIN_FONT_FAMILY_H

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "minikin/FamilyVariant.h"
#include "minikin/Font.h"
#include "minikin/FontStyle.h"
#include "minikin/HbUtils.h"
#include "minikin/Macros.h"
#include "minikin/SparseBitSet.h"

namespace minikin {

class FontFamily {
public:
    explicit FontFamily(std::vector<Font>&& fonts);
    FontFamily(FamilyVariant variant, std::vector<Font>&& fonts);
    FontFamily(uint32_t localeListId, FamilyVariant variant, std::vector<Font>&& fonts,
               bool isCustomFallback);

    FakedFont getClosestMatch(FontStyle style) const;

    uint32_t localeListId() const { return mLocaleListId; }
    FamilyVariant variant() const { return mVariant; }

    // API's for enumerating the fonts in a family. These don't guarantee any particular order
    size_t getNumFonts() const { return mFonts.size(); }
    const Font* getFont(size_t index) const { return &mFonts[index]; }
    FontStyle getStyle(size_t index) const { return mFonts[index].style(); }
    bool isColorEmojiFamily() const { return mIsColorEmoji; }
    const std::unordered_set<AxisTag>& supportedAxes() const { return mSupportedAxes; }
    bool isCustomFallback() const { return mIsCustomFallback; }

    // Get Unicode coverage.
    const SparseBitSet& getCoverage() const { return mCoverage; }

    // Returns true if the font has a glyph for the code point and variation selector pair.
    // Caller should acquire a lock before calling the method.
    bool hasGlyph(uint32_t codepoint, uint32_t variationSelector) const;

    // Returns true if this font family has a variaion sequence table (cmap format 14 subtable).
    bool hasVSTable() const { return !mCmapFmt14Coverage.empty(); }

    // Creates new FontFamily based on this family while applying font variations. Returns nullptr
    // if none of variations apply to this family.
    std::shared_ptr<FontFamily> createFamilyWithVariation(
            const std::vector<FontVariation>& variations) const;

private:
    void computeCoverage();

    uint32_t mLocaleListId;
    FamilyVariant mVariant;
    std::vector<Font> mFonts;
    std::unordered_set<AxisTag> mSupportedAxes;
    bool mIsColorEmoji;
    bool mIsCustomFallback;

    SparseBitSet mCoverage;
    std::vector<std::unique_ptr<SparseBitSet>> mCmapFmt14Coverage;

    MINIKIN_PREVENT_COPY_AND_ASSIGN(FontFamily);
};

}  // namespace minikin

#endif  // MINIKIN_FONT_FAMILY_H
