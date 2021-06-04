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

#define LOG_TAG "GreedyLineBreak"

#include "minikin/Characters.h"
#include "minikin/LineBreaker.h"
#include "minikin/MeasuredText.h"
#include "minikin/Range.h"
#include "minikin/U16StringPiece.h"

#include "HyphenatorMap.h"
#include "LineBreakerUtil.h"
#include "Locale.h"
#include "LocaleListCache.h"
#include "WordBreaker.h"

namespace minikin {

namespace {

constexpr uint32_t NOWHERE = 0xFFFFFFFF;

class GreedyLineBreaker {
public:
    // User of this class must keep measured, lineWidthLimit, tabStop alive until the instance is
    // destructed.
    GreedyLineBreaker(const U16StringPiece& textBuf, const MeasuredText& measured,
                      const LineWidth& lineWidthLimits, const TabStops& tabStops,
                      bool enableHyphenation)
            : mLineWidthLimit(lineWidthLimits.getAt(0)),
              mTextBuf(textBuf),
              mMeasuredText(measured),
              mLineWidthLimits(lineWidthLimits),
              mTabStops(tabStops),
              mEnableHyphenation(enableHyphenation) {}

    void process();

    LineBreakResult getResult() const;

private:
    struct BreakPoint {
        BreakPoint(uint32_t offset, float lineWidth, StartHyphenEdit startHyphen,
                   EndHyphenEdit endHyphen)
                : offset(offset),
                  lineWidth(lineWidth),
                  hyphenEdit(packHyphenEdit(startHyphen, endHyphen)) {}

        uint32_t offset;
        float lineWidth;
        HyphenEdit hyphenEdit;
    };

    inline uint32_t getPrevLineBreakOffset() {
        return mBreakPoints.empty() ? 0 : mBreakPoints.back().offset;
    }

    // Registers the break point and prepares for next line computation.
    void breakLineAt(uint32_t offset, float lineWidth, float remainingNextLineWidth,
                     float remainingNextSumOfCharWidths, EndHyphenEdit thisLineEndHyphen,
                     StartHyphenEdit nextLineStartHyphen);

    // Update current line width.
    void updateLineWidth(uint16_t c, float width);

    // Break line if current line exceeds the line limit.
    void processLineBreak(uint32_t offset, WordBreaker* breaker, bool doHyphenation);

    // Try to break with previous word boundary.
    // Returns false if unable to break by word boundary.
    bool tryLineBreakWithWordBreak();

    // Try to break with hyphenation.
    // Returns false if unable to hyphenate.
    //
    // This method keeps hyphenation until the line width after line break meets the line width
    // limit.
    bool tryLineBreakWithHyphenation(const Range& range, WordBreaker* breaker);

    // Do line break with each characters.
    //
    // This method only breaks at the first offset which has the longest width for the line width
    // limit. This method don't keep line breaking even if the rest of the word exceeds the line
    // width limit.
    // This method return true if there is no characters to be processed.
    bool doLineBreakWithGraphemeBounds(const Range& range);

    // Info about the line currently processing.
    uint32_t mLineNum = 0;
    double mLineWidth = 0;
    double mSumOfCharWidths = 0;
    double mLineWidthLimit;
    StartHyphenEdit mStartHyphenEdit = StartHyphenEdit::NO_EDIT;

    // Previous word break point info.
    uint32_t mPrevWordBoundsOffset = NOWHERE;
    double mLineWidthAtPrevWordBoundary = 0;
    double mSumOfCharWidthsAtPrevWordBoundary = 0;
    bool mIsPrevWordBreakIsInEmailOrUrl = false;

    // The hyphenator currently used.
    const Hyphenator* mHyphenator = nullptr;

    // Input parameters.
    const U16StringPiece& mTextBuf;
    const MeasuredText& mMeasuredText;
    const LineWidth& mLineWidthLimits;
    const TabStops& mTabStops;
    bool mEnableHyphenation;

    // The result of line breaking.
    std::vector<BreakPoint> mBreakPoints;

    MINIKIN_PREVENT_COPY_ASSIGN_AND_MOVE(GreedyLineBreaker);
};

void GreedyLineBreaker::breakLineAt(uint32_t offset, float lineWidth, float remainingNextLineWidth,
                                    float remainingNextSumOfCharWidths,
                                    EndHyphenEdit thisLineEndHyphen,
                                    StartHyphenEdit nextLineStartHyphen) {
    // First, push the break to result.
    mBreakPoints.emplace_back(offset, lineWidth, mStartHyphenEdit, thisLineEndHyphen);

    // Update the current line info.
    mLineWidthLimit = mLineWidthLimits.getAt(++mLineNum);
    mLineWidth = remainingNextLineWidth;
    mSumOfCharWidths = remainingNextSumOfCharWidths;
    mStartHyphenEdit = nextLineStartHyphen;
    mPrevWordBoundsOffset = NOWHERE;
    mLineWidthAtPrevWordBoundary = 0;
    mSumOfCharWidthsAtPrevWordBoundary = 0;
    mIsPrevWordBreakIsInEmailOrUrl = false;
}

bool GreedyLineBreaker::tryLineBreakWithWordBreak() {
    if (mPrevWordBoundsOffset == NOWHERE) {
        return false;  // No word break point before..
    }

    breakLineAt(mPrevWordBoundsOffset,                            // break offset
                mLineWidthAtPrevWordBoundary,                     // line width
                mLineWidth - mSumOfCharWidthsAtPrevWordBoundary,  // remaining next line width
                // remaining next sum of char widths.
                mSumOfCharWidths - mSumOfCharWidthsAtPrevWordBoundary, EndHyphenEdit::NO_EDIT,
                StartHyphenEdit::NO_EDIT);  // No hyphen modification.
    return true;
}

bool GreedyLineBreaker::tryLineBreakWithHyphenation(const Range& range, WordBreaker* breaker) {
    if (!mEnableHyphenation || mHyphenator == nullptr) {
        return false;
    }

    Run* targetRun = nullptr;
    for (const auto& run : mMeasuredText.runs) {
        if (run->getRange().contains(range)) {
            targetRun = run.get();
        }
    }

    if (targetRun == nullptr) {
        return false;  // The target range may lay on multiple run. Unable to hyphenate.
    }

    const Range targetRange = breaker->wordRange();
    if (!range.contains(targetRange)) {
        return false;
    }

    const std::vector<HyphenationType> hyphenResult =
            hyphenate(mTextBuf.substr(targetRange), *mHyphenator);
    Range contextRange = range;
    uint32_t prevOffset = NOWHERE;
    float prevWidth = 0;

    // Look up the hyphenation point from the begining.
    for (uint32_t i = targetRange.getStart(); i < targetRange.getEnd(); ++i) {
        const HyphenationType hyph = hyphenResult[targetRange.toRangeOffset(i)];
        if (hyph == HyphenationType::DONT_BREAK) {
            continue;  // Not a hyphenation point.
        }

        const float width =
                targetRun->measureHyphenPiece(mTextBuf, contextRange.split(i).first,
                                              mStartHyphenEdit, editForThisLine(hyph), nullptr);

        if (width <= mLineWidthLimit) {
            // There are still space, remember current offset and look up next hyphenation point.
            prevOffset = i;
            prevWidth = width;
            continue;
        }

        if (prevOffset == NOWHERE) {
            // Even with hyphenation, the piece is too long for line. Give up and break in
            // character bounds.
            doLineBreakWithGraphemeBounds(contextRange);
        } else {
            // Previous offset is the longest hyphenation piece. Break with it.
            const HyphenationType hyph = hyphenResult[targetRange.toRangeOffset(prevOffset)];
            const StartHyphenEdit nextLineStartHyphenEdit = editForNextLine(hyph);
            const float remainingCharWidths = targetRun->measureHyphenPiece(
                    mTextBuf, contextRange.split(prevOffset).second, nextLineStartHyphenEdit,
                    EndHyphenEdit::NO_EDIT, nullptr);
            breakLineAt(prevOffset, prevWidth,
                        remainingCharWidths - (mSumOfCharWidths - mLineWidth), remainingCharWidths,
                        editForThisLine(hyph), nextLineStartHyphenEdit);
        }

        if (mLineWidth <= mLineWidthLimit) {
            // The remaining hyphenation piece is less than line width. No more hyphenation is
            // needed. Go to next word.
            return true;
        }

        // Even after line break, the remaining hyphenation piece is still too long for the limit.
        // Keep hyphenating for the rest.
        i = getPrevLineBreakOffset();
        contextRange.setStart(i);  // Update the hyphenation start point.
        prevOffset = NOWHERE;
    }

    // Do the same line break at the end of text.
    // TODO: Remove code duplication. This is the same as in the for loop but extracting function
    //       may not clear.
    if (prevOffset == NOWHERE) {
        doLineBreakWithGraphemeBounds(contextRange);
    } else {
        const HyphenationType hyph = hyphenResult[targetRange.toRangeOffset(prevOffset)];
        const StartHyphenEdit nextLineStartHyphenEdit = editForNextLine(hyph);
        const float remainingCharWidths = targetRun->measureHyphenPiece(
                mTextBuf, contextRange.split(prevOffset).second, nextLineStartHyphenEdit,
                EndHyphenEdit::NO_EDIT, nullptr);

        breakLineAt(prevOffset, prevWidth, remainingCharWidths - (mSumOfCharWidths - mLineWidth),
                    remainingCharWidths, editForThisLine(hyph), nextLineStartHyphenEdit);
    }

    return true;
}

// TODO: Respect trailing line end spaces.
bool GreedyLineBreaker::doLineBreakWithGraphemeBounds(const Range& range) {
    double width = mMeasuredText.widths[range.getStart()];

    // Starting from + 1 since at least one character needs to be assigned to a line.
    for (uint32_t i = range.getStart() + 1; i < range.getEnd(); ++i) {
        const float w = mMeasuredText.widths[i];
        if (w == 0) {
            continue;  // w == 0 means here is not a grapheme bounds. Don't break here.
        }
        if (width + w > mLineWidthLimit) {
            // Okay, here is the longest position.
            breakLineAt(i, width, mLineWidth - width, mSumOfCharWidths - width,
                        EndHyphenEdit::NO_EDIT, StartHyphenEdit::NO_EDIT);

            // This method only breaks at the first longest offset, since we may want to hyphenate
            // the rest of the word.
            return false;
        } else {
            width += w;
        }
    }

    // Reaching here means even one character (or cluster) doesn't fit the line.
    // Give up and break at the end of this range.
    breakLineAt(range.getEnd(), mLineWidth, 0, 0, EndHyphenEdit::NO_EDIT, StartHyphenEdit::NO_EDIT);
    return true;
}

void GreedyLineBreaker::updateLineWidth(uint16_t c, float width) {
    if (c == CHAR_TAB) {
        mSumOfCharWidths = mTabStops.nextTab(mSumOfCharWidths);
        mLineWidth = mSumOfCharWidths;
    } else {
        mSumOfCharWidths += width;
        if (!isLineEndSpace(c)) {
            mLineWidth = mSumOfCharWidths;
        }
    }
}

void GreedyLineBreaker::processLineBreak(uint32_t offset, WordBreaker* breaker,
                                         bool doHyphenation) {
    while (mLineWidth > mLineWidthLimit) {
        const Range lineRange(getPrevLineBreakOffset(), offset);  // The range we need to address.
        if (tryLineBreakWithWordBreak()) {
            continue;  // The word in the new line may still be too long for the line limit.
        } else if (doHyphenation && tryLineBreakWithHyphenation(lineRange, breaker)) {
            continue;  // TODO: we may be able to return here.
        } else {
            if (doLineBreakWithGraphemeBounds(lineRange)) {
                return;
            }
        }
    }

    // There is still spaces, remember current word break point as a candidate and wait next word.
    const bool isInEmailOrUrl = breaker->breakBadness() != 0;
    if (mPrevWordBoundsOffset == NOWHERE || mIsPrevWordBreakIsInEmailOrUrl | !isInEmailOrUrl) {
        mPrevWordBoundsOffset = offset;
        mLineWidthAtPrevWordBoundary = mLineWidth;
        mSumOfCharWidthsAtPrevWordBoundary = mSumOfCharWidths;
        mIsPrevWordBreakIsInEmailOrUrl = isInEmailOrUrl;
    }
}

void GreedyLineBreaker::process() {
    WordBreaker wordBreaker;
    wordBreaker.setText(mTextBuf.data(), mTextBuf.size());

    // Following two will be initialized after the first iteration.
    uint32_t localeListId = LocaleListCache::kInvalidListId;
    uint32_t nextWordBoundaryOffset = 0;
    for (const auto& run : mMeasuredText.runs) {
        const Range range = run->getRange();

        // Update locale if necessary.
        uint32_t newLocaleListId = run->getLocaleListId();
        if (localeListId != newLocaleListId) {
            Locale locale = getEffectiveLocale(newLocaleListId);
            nextWordBoundaryOffset = wordBreaker.followingWithLocale(locale, range.getStart());
            mHyphenator = HyphenatorMap::lookup(locale);
            localeListId = newLocaleListId;
        }

        for (uint32_t i = range.getStart(); i < range.getEnd(); ++i) {
            updateLineWidth(mTextBuf[i], mMeasuredText.widths[i]);

            if ((i + 1) == nextWordBoundaryOffset) {
                // Only process line break at word boundary and the run can break into some pieces.
                if (run->canBreak() || nextWordBoundaryOffset == range.getEnd()) {
                    processLineBreak(i + 1, &wordBreaker, run->canBreak());
                }
                nextWordBoundaryOffset = wordBreaker.next();
            }
        }
    }

    if (getPrevLineBreakOffset() != mTextBuf.size() && mPrevWordBoundsOffset != NOWHERE) {
        // The remaining words in the last line.
        breakLineAt(mPrevWordBoundsOffset, mLineWidth, 0, 0, EndHyphenEdit::NO_EDIT,
                    StartHyphenEdit::NO_EDIT);
    }
}

LineBreakResult GreedyLineBreaker::getResult() const {
    constexpr int TAB_BIT = 1 << 29;  // Must be the same in StaticLayout.java

    LineBreakResult out;
    uint32_t prevBreakOffset = 0;
    for (const auto& breakPoint : mBreakPoints) {
        // TODO: compute these during line breaking if these takes longer time.
        bool hasTabChar = false;
        for (uint32_t i = prevBreakOffset; i < breakPoint.offset; ++i) {
            hasTabChar |= mTextBuf[i] == CHAR_TAB;
        }

        MinikinExtent extent =
                mMeasuredText.getExtent(mTextBuf, Range(prevBreakOffset, breakPoint.offset));
        out.breakPoints.push_back(breakPoint.offset);
        out.widths.push_back(breakPoint.lineWidth);
        out.ascents.push_back(extent.ascent);
        out.descents.push_back(extent.descent);
        out.flags.push_back((hasTabChar ? TAB_BIT : 0) | static_cast<int>(breakPoint.hyphenEdit));

        prevBreakOffset = breakPoint.offset;
    }
    return out;
}

}  // namespace

LineBreakResult breakLineGreedy(const U16StringPiece& textBuf, const MeasuredText& measured,
                                const LineWidth& lineWidthLimits, const TabStops& tabStops,
                                bool enableHyphenation) {
    if (textBuf.size() == 0) {
        return LineBreakResult();
    }
    GreedyLineBreaker lineBreaker(textBuf, measured, lineWidthLimits, tabStops, enableHyphenation);
    lineBreaker.process();
    return lineBreaker.getResult();
}

}  // namespace minikin
