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

#ifndef MINIKIN_LAYOUT_H
#define MINIKIN_LAYOUT_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <gtest/gtest_prod.h>

#include "minikin/FontCollection.h"
#include "minikin/LayoutCore.h"
#include "minikin/Range.h"
#include "minikin/U16StringPiece.h"

namespace minikin {

class Layout;
struct LayoutPieces;

struct LayoutGlyph {
    LayoutGlyph(FakedFont font, uint32_t glyph_id, float x, float y)
            : font(font), glyph_id(glyph_id), x(x), y(y) {}
    FakedFont font;

    uint32_t glyph_id;
    float x;
    float y;
};

// Must be the same value with Paint.java
enum class Bidi : uint8_t {
    LTR = 0b0000,          // Must be same with Paint.BIDI_LTR
    RTL = 0b0001,          // Must be same with Paint.BIDI_RTL
    DEFAULT_LTR = 0b0010,  // Must be same with Paint.BIDI_DEFAULT_LTR
    DEFAULT_RTL = 0b0011,  // Must be same with Paint.BIDI_DEFAULT_RTL
    FORCE_LTR = 0b0100,    // Must be same with Paint.BIDI_FORCE_LTR
    FORCE_RTL = 0b0101,    // Must be same with Paint.BIDI_FORCE_RTL
};

inline bool isRtl(Bidi bidi) {
    return static_cast<uint8_t>(bidi) & 0b0001;
}
inline bool isOverride(Bidi bidi) {
    return static_cast<uint8_t>(bidi) & 0b0100;
}

// Lifecycle and threading assumptions for Layout:
// The object is assumed to be owned by a single thread; multiple threads
// may not mutate it at the same time.
class Layout {
public:
    Layout(const U16StringPiece& str, const Range& range, Bidi bidiFlags, const MinikinPaint& paint,
           StartHyphenEdit startHyphen, EndHyphenEdit endHyphen)
            : mAdvance(0) {
        doLayout(str, range, bidiFlags, paint, startHyphen, endHyphen);
    }

    Layout(uint32_t count) : mAdvance(0) {
        mAdvances.resize(count, 0);
        mGlyphs.reserve(count);
    }

    static float measureText(const U16StringPiece& str, const Range& range, Bidi bidiFlags,
                             const MinikinPaint& paint, StartHyphenEdit startHyphen,
                             EndHyphenEdit endHyphen, float* advances);

    const std::vector<float>& advances() const { return mAdvances; }

    // public accessors
    size_t nGlyphs() const { return mGlyphs.size(); }
    const MinikinFont* getFont(int i) const { return mGlyphs[i].font.font->typeface().get(); }
    FontFakery getFakery(int i) const { return mGlyphs[i].font.fakery; }
    unsigned int getGlyphId(int i) const { return mGlyphs[i].glyph_id; }
    float getX(int i) const { return mGlyphs[i].x; }
    float getY(int i) const { return mGlyphs[i].y; }
    float getAdvance() const { return mAdvance; }
    float getCharAdvance(size_t i) const { return mAdvances[i]; }
    const std::vector<float>& getAdvances() const { return mAdvances; }
    void getBounds(MinikinRect* rect) const { rect->set(mBounds); }
    const MinikinRect& getBounds() const { return mBounds; }

    // Purge all caches, useful in low memory conditions
    static void purgeCaches();

    // Dump minikin internal statistics, cache usage, cache hit ratio, etc.
    static void dumpMinikinStats(int fd);

    // Append another layout (for example, cached value) into this one
    void appendLayout(const LayoutPiece& src, size_t start, float extraAdvance);

private:
    FRIEND_TEST(LayoutTest, doLayoutWithPrecomputedPiecesTest);

    void doLayout(const U16StringPiece& str, const Range& range, Bidi bidiFlags,
                  const MinikinPaint& paint, StartHyphenEdit startHyphen, EndHyphenEdit endHyphen);

    // Lay out a single bidi run
    // When layout is not null, layout info will be stored in the object.
    // When advances is not null, measurement results will be stored in the array.
    static float doLayoutRunCached(const U16StringPiece& textBuf, const Range& range, bool isRtl,
                                   const MinikinPaint& paint, size_t dstStart,
                                   StartHyphenEdit startHyphen, EndHyphenEdit endHyphen,
                                   Layout* layout, float* advances);

    // Lay out a single word
    static float doLayoutWord(const uint16_t* buf, size_t start, size_t count, size_t bufSize,
                              bool isRtl, const MinikinPaint& paint, size_t bufStart,
                              StartHyphenEdit startHyphen, EndHyphenEdit endHyphen, Layout* layout,
                              float* advances);

    // Lay out a single bidi run
    void doLayoutRun(const uint16_t* buf, size_t start, size_t count, size_t bufSize, bool isRtl,
                     const MinikinPaint& paint, StartHyphenEdit startHyphen,
                     EndHyphenEdit endHyphen);

    std::vector<LayoutGlyph> mGlyphs;

    // This vector defined per code unit, so their length is identical to the input text.
    std::vector<float> mAdvances;

    float mAdvance;
    MinikinRect mBounds;
};

}  // namespace minikin

#endif  // MINIKIN_LAYOUT_H
