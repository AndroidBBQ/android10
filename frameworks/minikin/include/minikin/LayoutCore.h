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

#ifndef MINIKIN_LAYOUT_CORE_H
#define MINIKIN_LAYOUT_CORE_H

#include <vector>

#include <gtest/gtest_prod.h>

#include "minikin/FontFamily.h"
#include "minikin/Hyphenator.h"
#include "minikin/MinikinExtent.h"
#include "minikin/MinikinFont.h"
#include "minikin/MinikinRect.h"
#include "minikin/Range.h"
#include "minikin/U16StringPiece.h"

namespace minikin {

struct MinikinPaint;

struct Point {
    Point() : x(0), y(0) {}
    Point(float x, float y) : x(x), y(y) {}
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    float x;
    float y;
};

// Immutable, recycle-able layout result.
class LayoutPiece {
public:
    LayoutPiece(const U16StringPiece& textBuf, const Range& range, bool isRtl,
                const MinikinPaint& paint, StartHyphenEdit startHyphen, EndHyphenEdit endHyphen);

    // Low level accessors.
    const std::vector<uint8_t>& fontIndices() const { return mFontIndices; }
    const std::vector<uint32_t> glyphIds() const { return mGlyphIds; }
    const std::vector<Point> points() const { return mPoints; }
    const std::vector<float> advances() const { return mAdvances; }
    float advance() const { return mAdvance; }
    const MinikinRect& bounds() const { return mBounds; }
    const MinikinExtent& extent() const { return mExtent; }
    const std::vector<FakedFont>& fonts() const { return mFonts; }

    // Helper accessors
    uint32_t glyphCount() const { return mGlyphIds.size(); }
    const FakedFont& fontAt(int glyphPos) const { return mFonts[mFontIndices[glyphPos]]; }
    uint32_t glyphIdAt(int glyphPos) const { return mGlyphIds[glyphPos]; }
    const Point& pointAt(int glyphPos) const { return mPoints[glyphPos]; }

    uint32_t getMemoryUsage() const {
        return sizeof(uint8_t) * mFontIndices.size() + sizeof(uint32_t) * mGlyphIds.size() +
               sizeof(Point) * mPoints.size() + sizeof(float) * mAdvances.size() + sizeof(float) +
               sizeof(MinikinRect) + sizeof(MinikinExtent);
    }

private:
    FRIEND_TEST(LayoutTest, doLayoutWithPrecomputedPiecesTest);

    std::vector<uint8_t> mFontIndices;  // per glyph
    std::vector<uint32_t> mGlyphIds;    // per glyph
    std::vector<Point> mPoints;         // per glyph

    std::vector<float> mAdvances;  // per code units

    float mAdvance;
    MinikinRect mBounds;
    MinikinExtent mExtent;

    std::vector<FakedFont> mFonts;
};

// For gtest output
inline std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "(" << p.x << ", " << p.y << ")";
}
}  // namespace minikin

#endif  // MINIKIN_LAYOUT_CORE_H
