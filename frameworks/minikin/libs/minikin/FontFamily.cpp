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

#define LOG_TAG "Minikin"

#include "minikin/FontFamily.h"

#include <cstdint>
#include <vector>

#include <hb-ot.h>
#include <hb.h>
#include <log/log.h>

#include "minikin/CmapCoverage.h"
#include "minikin/FamilyVariant.h"
#include "minikin/HbUtils.h"
#include "minikin/MinikinFont.h"

#include "FontUtils.h"
#include "Locale.h"
#include "LocaleListCache.h"
#include "MinikinInternal.h"

namespace minikin {

Font Font::Builder::build() {
    if (mIsWeightSet && mIsSlantSet) {
        // No need to read OS/2 header of the font file.
        return Font(std::move(mTypeface), FontStyle(mWeight, mSlant), prepareFont(mTypeface));
    }

    HbFontUniquePtr font = prepareFont(mTypeface);
    FontStyle styleFromFont = analyzeStyle(font);
    if (!mIsWeightSet) {
        mWeight = styleFromFont.weight();
    }
    if (!mIsSlantSet) {
        mSlant = styleFromFont.slant();
    }
    return Font(std::move(mTypeface), FontStyle(mWeight, mSlant), std::move(font));
}

// static
HbFontUniquePtr Font::prepareFont(const std::shared_ptr<MinikinFont>& typeface) {
    const char* buf = reinterpret_cast<const char*>(typeface->GetFontData());
    size_t size = typeface->GetFontSize();
    uint32_t ttcIndex = typeface->GetFontIndex();

    HbBlobUniquePtr blob(hb_blob_create(buf, size, HB_MEMORY_MODE_READONLY, nullptr, nullptr));
    HbFaceUniquePtr face(hb_face_create(blob.get(), ttcIndex));
    HbFontUniquePtr parent(hb_font_create(face.get()));
    hb_ot_font_set_funcs(parent.get());

    uint32_t upem = hb_face_get_upem(face.get());
    hb_font_set_scale(parent.get(), upem, upem);

    HbFontUniquePtr font(hb_font_create_sub_font(parent.get()));
    std::vector<hb_variation_t> variations;
    variations.reserve(typeface->GetAxes().size());
    for (const FontVariation& variation : typeface->GetAxes()) {
        variations.push_back({variation.axisTag, variation.value});
    }
    hb_font_set_variations(font.get(), variations.data(), variations.size());
    return font;
}

// static
FontStyle Font::analyzeStyle(const HbFontUniquePtr& font) {
    HbBlob os2Table(font, MinikinFont::MakeTag('O', 'S', '/', '2'));
    if (!os2Table) {
        return FontStyle();
    }

    int weight;
    bool italic;
    if (!::minikin::analyzeStyle(os2Table.get(), os2Table.size(), &weight, &italic)) {
        return FontStyle();
    }
    // TODO: Update weight/italic based on fvar value.
    return FontStyle(static_cast<uint16_t>(weight), static_cast<FontStyle::Slant>(italic));
}

std::unordered_set<AxisTag> Font::getSupportedAxes() const {
    HbBlob fvarTable(mBaseFont, MinikinFont::MakeTag('f', 'v', 'a', 'r'));
    if (!fvarTable) {
        return std::unordered_set<AxisTag>();
    }
    std::unordered_set<AxisTag> supportedAxes;
    analyzeAxes(fvarTable.get(), fvarTable.size(), &supportedAxes);
    return supportedAxes;
}

FontFamily::FontFamily(std::vector<Font>&& fonts)
        : FontFamily(FamilyVariant::DEFAULT, std::move(fonts)) {}

FontFamily::FontFamily(FamilyVariant variant, std::vector<Font>&& fonts)
        : FontFamily(LocaleListCache::kEmptyListId, variant, std::move(fonts),
                     false /* isCustomFallback */) {}

FontFamily::FontFamily(uint32_t localeListId, FamilyVariant variant, std::vector<Font>&& fonts,
                       bool isCustomFallback)
        : mLocaleListId(localeListId),
          mVariant(variant),
          mFonts(std::move(fonts)),
          mIsColorEmoji(LocaleListCache::getById(localeListId).getEmojiStyle() ==
                        EmojiStyle::EMOJI),
          mIsCustomFallback(isCustomFallback) {
    MINIKIN_ASSERT(!mFonts.empty(), "FontFamily must contain at least one font.");
    computeCoverage();
}

// Compute a matching metric between two styles - 0 is an exact match
static int computeMatch(FontStyle style1, FontStyle style2) {
    if (style1 == style2) return 0;
    int score = abs(style1.weight() / 100 - style2.weight() / 100);
    if (style1.slant() != style2.slant()) {
        score += 2;
    }
    return score;
}

static FontFakery computeFakery(FontStyle wanted, FontStyle actual) {
    // If desired weight is semibold or darker, and 2 or more grades
    // higher than actual (for example, medium 500 -> bold 700), then
    // select fake bold.
    bool isFakeBold = wanted.weight() >= 600 && (wanted.weight() - actual.weight()) >= 200;
    bool isFakeItalic = wanted.slant() == FontStyle::Slant::ITALIC &&
                        actual.slant() == FontStyle::Slant::UPRIGHT;
    return FontFakery(isFakeBold, isFakeItalic);
}

FakedFont FontFamily::getClosestMatch(FontStyle style) const {
    const Font* bestFont = &mFonts[0];
    int bestMatch = computeMatch(bestFont->style(), style);
    for (size_t i = 1; i < mFonts.size(); i++) {
        const Font& font = mFonts[i];
        int match = computeMatch(font.style(), style);
        if (i == 0 || match < bestMatch) {
            bestFont = &font;
            bestMatch = match;
        }
    }
    return FakedFont{bestFont, computeFakery(style, bestFont->style())};
}

void FontFamily::computeCoverage() {
    const Font* font = getClosestMatch(FontStyle()).font;
    HbBlob cmapTable(font->baseFont(), MinikinFont::MakeTag('c', 'm', 'a', 'p'));
    if (cmapTable.get() == nullptr) {
        ALOGE("Could not get cmap table size!\n");
        return;
    }

    mCoverage = CmapCoverage::getCoverage(cmapTable.get(), cmapTable.size(), &mCmapFmt14Coverage);

    for (size_t i = 0; i < mFonts.size(); ++i) {
        std::unordered_set<AxisTag> supportedAxes = mFonts[i].getSupportedAxes();
        mSupportedAxes.insert(supportedAxes.begin(), supportedAxes.end());
    }
}

bool FontFamily::hasGlyph(uint32_t codepoint, uint32_t variationSelector) const {
    if (variationSelector == 0) {
        return mCoverage.get(codepoint);
    }

    if (mCmapFmt14Coverage.empty()) {
        return false;
    }

    const uint16_t vsIndex = getVsIndex(variationSelector);

    if (vsIndex >= mCmapFmt14Coverage.size()) {
        // Even if vsIndex is INVALID_VS_INDEX, we reach here since INVALID_VS_INDEX is defined to
        // be at the maximum end of the range.
        return false;
    }

    const std::unique_ptr<SparseBitSet>& bitset = mCmapFmt14Coverage[vsIndex];
    if (bitset.get() == nullptr) {
        return false;
    }

    return bitset->get(codepoint);
}

std::shared_ptr<FontFamily> FontFamily::createFamilyWithVariation(
        const std::vector<FontVariation>& variations) const {
    if (variations.empty() || mSupportedAxes.empty()) {
        return nullptr;
    }

    bool hasSupportedAxis = false;
    for (const FontVariation& variation : variations) {
        if (mSupportedAxes.find(variation.axisTag) != mSupportedAxes.end()) {
            hasSupportedAxis = true;
            break;
        }
    }
    if (!hasSupportedAxis) {
        // None of variation axes are suppored by this family.
        return nullptr;
    }

    std::vector<Font> fonts;
    for (const Font& font : mFonts) {
        bool supportedVariations = false;
        std::unordered_set<AxisTag> supportedAxes = font.getSupportedAxes();
        if (!supportedAxes.empty()) {
            for (const FontVariation& variation : variations) {
                if (supportedAxes.find(variation.axisTag) != supportedAxes.end()) {
                    supportedVariations = true;
                    break;
                }
            }
        }
        std::shared_ptr<MinikinFont> minikinFont;
        if (supportedVariations) {
            minikinFont = font.typeface()->createFontWithVariation(variations);
        }
        if (minikinFont == nullptr) {
            minikinFont = font.typeface();
        }
        fonts.push_back(Font::Builder(minikinFont).setStyle(font.style()).build());
    }

    return std::shared_ptr<FontFamily>(
            new FontFamily(mLocaleListId, mVariant, std::move(fonts), mIsCustomFallback));
}

}  // namespace minikin
