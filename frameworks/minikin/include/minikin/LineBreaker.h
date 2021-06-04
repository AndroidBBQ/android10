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

/**
 * A module for breaking paragraphs into lines, supporting high quality
 * hyphenation and justification.
 */

#ifndef MINIKIN_LINE_BREAKER_H
#define MINIKIN_LINE_BREAKER_H

#include <deque>
#include <vector>

#include "minikin/FontCollection.h"
#include "minikin/Layout.h"
#include "minikin/Macros.h"
#include "minikin/MeasuredText.h"
#include "minikin/MinikinFont.h"
#include "minikin/Range.h"
#include "minikin/U16StringPiece.h"

namespace minikin {

enum class BreakStrategy : uint8_t {
    Greedy = 0,
    HighQuality = 1,
    Balanced = 2,
};

enum class HyphenationFrequency : uint8_t {
    None = 0,
    Normal = 1,
    Full = 2,
};

class Hyphenator;
class WordBreaker;

class TabStops {
public:
    // Caller must free stops. stops can be nullprt.
    TabStops(const float* stops, size_t nStops, float tabWidth)
            : mStops(stops), mStopsSize(nStops), mTabWidth(tabWidth) {}

    float nextTab(float widthSoFar) const {
        for (size_t i = 0; i < mStopsSize; i++) {
            if (mStops[i] > widthSoFar) {
                return mStops[i];
            }
        }
        return floor(widthSoFar / mTabWidth + 1) * mTabWidth;
    }

private:
    const float* mStops;
    size_t mStopsSize;
    float mTabWidth;
};

// Implement this for the additional information during line breaking.
// The functions in this class's interface may be called several times. The implementation
// must return the same value for the same input.
class LineWidth {
public:
    virtual ~LineWidth() {}

    // Called to find out the width for the line. This must not return negative values.
    virtual float getAt(size_t lineNo) const = 0;

    // Called to find out the minimum line width. This mut not return negative values.
    virtual float getMin() const = 0;
};

struct LineBreakResult {
public:
    LineBreakResult() = default;

    // Following five vectors have the same length.
    // TODO: Introduce individual line info struct if copy cost in JNI is negligible.
    std::vector<int> breakPoints;
    std::vector<float> widths;
    std::vector<float> ascents;
    std::vector<float> descents;
    std::vector<int> flags;

    LineBreakResult(LineBreakResult&&) = default;
    LineBreakResult& operator=(LineBreakResult&&) = default;

    void reverse() {
        std::reverse(breakPoints.begin(), breakPoints.end());
        std::reverse(widths.begin(), widths.end());
        std::reverse(ascents.begin(), ascents.end());
        std::reverse(descents.begin(), descents.end());
        std::reverse(flags.begin(), flags.end());
    }

private:
    MINIKIN_PREVENT_COPY_AND_ASSIGN(LineBreakResult);
};

LineBreakResult breakIntoLines(const U16StringPiece& textBuffer, BreakStrategy strategy,
                               HyphenationFrequency frequency, bool justified,
                               const MeasuredText& measuredText, const LineWidth& lineWidth,
                               const TabStops& tabStops);

}  // namespace minikin

#endif  // MINIKIN_LINE_BREAKER_H
