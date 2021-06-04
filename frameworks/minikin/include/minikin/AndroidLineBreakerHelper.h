/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef MINIKIN_ANDROID_LINE_BREAKER_HELPERS_H
#define MINIKIN_ANDROID_LINE_BREAKER_HELPERS_H

#include <algorithm>

#include "minikin/LineBreaker.h"

namespace minikin {
namespace android {

class AndroidLineWidth : public LineWidth {
public:
    AndroidLineWidth(float firstWidth, int32_t firstLineCount, float restWidth,
                     const std::vector<float>& indents, int32_t indentsOffset)
            : mFirstWidth(firstWidth),
              mFirstLineCount(firstLineCount),
              mRestWidth(restWidth),
              mIndents(indents),
              mOffset(indentsOffset) {}

    float getAt(size_t lineNo) const override {
        const float width = ((ssize_t)lineNo < (ssize_t)mFirstLineCount) ? mFirstWidth : mRestWidth;
        return std::max(0.0f, width - get(mIndents, lineNo));
    }

    float getMin() const override {
        // A simpler algorithm would have been simply looping until the larger of
        // mFirstLineCount and mIndents.size()-mOffset, but that does unnecessary calculations
        // when mFirstLineCount is large. Instead, we measure the first line, all the lines that
        // have an indent, and the first line after firstWidth ends and restWidth starts.
        float minWidth = std::min(getAt(0), getAt(mFirstLineCount));
        for (size_t lineNo = 1; lineNo + mOffset < mIndents.size(); lineNo++) {
            minWidth = std::min(minWidth, getAt(lineNo));
        }
        return minWidth;
    }

private:
    float get(const std::vector<float>& vec, size_t lineNo) const {
        if (vec.empty()) {
            return 0;
        }
        const size_t index = lineNo + mOffset;
        if (index < vec.size()) {
            return vec[index];
        } else {
            return vec.back();
        }
    }

    const float mFirstWidth;
    const int32_t mFirstLineCount;
    const float mRestWidth;
    const std::vector<float>& mIndents;
    const int32_t mOffset;
};

class StaticLayoutNative {
public:
    StaticLayoutNative(BreakStrategy strategy, HyphenationFrequency frequency, bool isJustified,
                       std::vector<float>&& indents)
            : mStrategy(strategy),
              mFrequency(frequency),
              mIsJustified(isJustified),
              mIndents(std::move(indents)) {}

    LineBreakResult computeBreaks(const U16StringPiece& textBuf, const MeasuredText& measuredText,
                                  // Line width arguments
                                  float firstWidth, int32_t firstWidthLineCount, float restWidth,
                                  int32_t indentsOffset,
                                  // Tab stop arguments
                                  const float* tabStops, int32_t tabStopSize,
                                  float defaultTabStopWidth) const {
        AndroidLineWidth lineWidth(firstWidth, firstWidthLineCount, restWidth, mIndents,
                                   indentsOffset);
        return breakIntoLines(textBuf, mStrategy, mFrequency, mIsJustified, measuredText, lineWidth,
                              TabStops(tabStops, tabStopSize, defaultTabStopWidth));
    }

    inline BreakStrategy getStrategy() const { return mStrategy; }
    inline HyphenationFrequency getFrequency() const { return mFrequency; }
    inline bool isJustified() const { return mIsJustified; }

private:
    const BreakStrategy mStrategy;
    const HyphenationFrequency mFrequency;
    const bool mIsJustified;
    const std::vector<float> mIndents;
    const std::vector<float> mLeftPaddings;
    const std::vector<float> mRightPaddings;
};

}  // namespace android
}  // namespace minikin

#endif  // MINIKIN_ANDROID_LINE_BREAKER_HELPERS_H
