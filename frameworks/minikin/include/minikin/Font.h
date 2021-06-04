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

#ifndef MINIKIN_FONT_H
#define MINIKIN_FONT_H

#include <memory>
#include <unordered_set>

#include "minikin/FontStyle.h"
#include "minikin/FontVariation.h"
#include "minikin/HbUtils.h"
#include "minikin/Macros.h"
#include "minikin/MinikinFont.h"

namespace minikin {

class Font;

// attributes representing transforms (fake bold, fake italic) to match styles
class FontFakery {
public:
    FontFakery() : mFakeBold(false), mFakeItalic(false) {}
    FontFakery(bool fakeBold, bool fakeItalic) : mFakeBold(fakeBold), mFakeItalic(fakeItalic) {}
    // TODO: want to support graded fake bolding
    bool isFakeBold() { return mFakeBold; }
    bool isFakeItalic() { return mFakeItalic; }
    inline bool operator==(const FontFakery& o) const {
        return mFakeBold == o.mFakeBold && mFakeItalic == o.mFakeItalic;
    }
    inline bool operator!=(const FontFakery& o) const { return !(*this == o); }

private:
    bool mFakeBold;
    bool mFakeItalic;
};

struct FakedFont {
    inline bool operator==(const FakedFont& o) const {
        return font == o.font && fakery == o.fakery;
    }
    inline bool operator!=(const FakedFont& o) const { return !(*this == o); }

    // ownership is the enclosing FontCollection
    const Font* font;
    FontFakery fakery;
};

// Represents a single font file.
class Font {
public:
    class Builder {
    public:
        Builder(const std::shared_ptr<MinikinFont>& typeface) : mTypeface(typeface) {}

        // Override the font style. If not called, info from OS/2 table is used.
        Builder& setStyle(FontStyle style) {
            mWeight = style.weight();
            mSlant = style.slant();
            mIsWeightSet = mIsSlantSet = true;
            return *this;
        }

        // Override the font weight. If not called, info from OS/2 table is used.
        Builder& setWeight(uint16_t weight) {
            mWeight = weight;
            mIsWeightSet = true;
            return *this;
        }

        // Override the font slant. If not called, info from OS/2 table is used.
        Builder& setSlant(FontStyle::Slant slant) {
            mSlant = slant;
            mIsSlantSet = true;
            return *this;
        }

        Font build();

    private:
        std::shared_ptr<MinikinFont> mTypeface;
        uint16_t mWeight = static_cast<uint16_t>(FontStyle::Weight::NORMAL);
        FontStyle::Slant mSlant = FontStyle::Slant::UPRIGHT;
        bool mIsWeightSet = false;
        bool mIsSlantSet = false;
    };

    Font(Font&& o) = default;
    Font& operator=(Font&& o) = default;

    Font& operator=(const Font& o) {
        mTypeface = o.mTypeface;
        mStyle = o.mStyle;
        mBaseFont = HbFontUniquePtr(hb_font_reference(o.mBaseFont.get()));
        return *this;
    }
    Font(const Font& o) { *this = o; }

    inline const std::shared_ptr<MinikinFont>& typeface() const { return mTypeface; }
    inline FontStyle style() const { return mStyle; }
    inline const HbFontUniquePtr& baseFont() const { return mBaseFont; }

    std::unordered_set<AxisTag> getSupportedAxes() const;

private:
    // Use Builder instead.
    Font(std::shared_ptr<MinikinFont>&& typeface, FontStyle style, HbFontUniquePtr&& baseFont)
            : mTypeface(std::move(typeface)), mStyle(style), mBaseFont(std::move(baseFont)) {}

    static HbFontUniquePtr prepareFont(const std::shared_ptr<MinikinFont>& typeface);
    static FontStyle analyzeStyle(const HbFontUniquePtr& font);

    std::shared_ptr<MinikinFont> mTypeface;
    FontStyle mStyle;
    HbFontUniquePtr mBaseFont;
};

}  // namespace minikin

#endif  // MINIKIN_FONT_H
