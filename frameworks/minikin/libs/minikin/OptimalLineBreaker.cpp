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

#include "OptimalLineBreaker.h"

#include <algorithm>
#include <limits>

#include "minikin/Characters.h"
#include "minikin/Layout.h"
#include "minikin/Range.h"
#include "minikin/U16StringPiece.h"

#include "HyphenatorMap.h"
#include "LayoutUtils.h"
#include "LineBreakerUtil.h"
#include "Locale.h"
#include "LocaleListCache.h"
#include "MinikinInternal.h"
#include "WordBreaker.h"

namespace minikin {

namespace {

// Large scores in a hierarchy; we prefer desperate breaks to an overfull line. All these
// constants are larger than any reasonable actual width score.
constexpr float SCORE_INFTY = std::numeric_limits<float>::max();
constexpr float SCORE_OVERFULL = 1e12f;
constexpr float SCORE_DESPERATE = 1e10f;

// Multiplier for hyphen penalty on last line.
constexpr float LAST_LINE_PENALTY_MULTIPLIER = 4.0f;
// Penalty assigned to each line break (to try to minimize number of lines)
// TODO: when we implement full justification (so spaces can shrink and stretch), this is
// probably not the most appropriate method.
constexpr float LINE_PENALTY_MULTIPLIER = 2.0f;

// Penalty assigned to shrinking the whitepsace.
constexpr float SHRINK_PENALTY_MULTIPLIER = 4.0f;

// Maximum amount that spaces can shrink, in justified text.
constexpr float SHRINKABILITY = 1.0 / 3.0;

// A single candidate break
struct Candidate {
    uint32_t offset;  // offset to text buffer, in code units

    ParaWidth preBreak;       // width of text until this point, if we decide to not break here:
                              // preBreak is used as an optimized way to calculate the width
                              // between two candidates. The line width between two line break
                              // candidates i and j is calculated as postBreak(j) - preBreak(i).
    ParaWidth postBreak;      // width of text until this point, if we decide to break here
    float penalty;            // penalty of this break (for example, hyphen penalty)
    uint32_t preSpaceCount;   // preceding space count before breaking
    uint32_t postSpaceCount;  // preceding space count after breaking
    HyphenationType hyphenType;
    bool isRtl;  // The direction of the bidi run containing or ending in this candidate

    Candidate(uint32_t offset, ParaWidth preBreak, ParaWidth postBreak, float penalty,
              uint32_t preSpaceCount, uint32_t postSpaceCount, HyphenationType hyphenType,
              bool isRtl)
            : offset(offset),
              preBreak(preBreak),
              postBreak(postBreak),
              penalty(penalty),
              preSpaceCount(preSpaceCount),
              postSpaceCount(postSpaceCount),
              hyphenType(hyphenType),
              isRtl(isRtl) {}
};

// A context of line break optimization.
struct OptimizeContext {
    // The break candidates.
    std::vector<Candidate> candidates;

    // The penalty for the number of lines.
    float linePenalty = 0.0f;

    // The width of a space. May be 0 if there are no spaces.
    // Note: if there are multiple different widths for spaces (for example, because of mixing of
    // fonts), it's only guaranteed to pick one.
    float spaceWidth = 0.0f;

    // Append desperate break point to the candidates.
    inline void pushDesperate(uint32_t offset, ParaWidth sumOfCharWidths, uint32_t spaceCount,
                              bool isRtl) {
        candidates.emplace_back(offset, sumOfCharWidths, sumOfCharWidths, SCORE_DESPERATE,
                                spaceCount, spaceCount,
                                HyphenationType::BREAK_AND_DONT_INSERT_HYPHEN, isRtl);
    }

    // Append hyphenation break point to the candidates.
    inline void pushHyphenation(uint32_t offset, ParaWidth preBreak, ParaWidth postBreak,
                                float penalty, uint32_t spaceCount, HyphenationType type,
                                bool isRtl) {
        candidates.emplace_back(offset, preBreak, postBreak, penalty, spaceCount, spaceCount, type,
                                isRtl);
    }

    // Append word break point to the candidates.
    inline void pushWordBreak(uint32_t offset, ParaWidth preBreak, ParaWidth postBreak,
                              float penalty, uint32_t preSpaceCount, uint32_t postSpaceCount,
                              bool isRtl) {
        candidates.emplace_back(offset, preBreak, postBreak, penalty, preSpaceCount, postSpaceCount,
                                HyphenationType::DONT_BREAK, isRtl);
    }

    OptimizeContext() {
        candidates.emplace_back(0, 0.0f, 0.0f, 0.0f, 0, 0, HyphenationType::DONT_BREAK, false);
    }
};

// Compute the penalty for the run and returns penalty for hyphenation and number of lines.
std::pair<float, float> computePenalties(const Run& run, const LineWidth& lineWidth,
                                         HyphenationFrequency frequency, bool justified) {
    float linePenalty = 0.0;
    const MinikinPaint* paint = run.getPaint();
    // a heuristic that seems to perform well
    float hyphenPenalty = 0.5 * paint->size * paint->scaleX * lineWidth.getAt(0);
    if (frequency == HyphenationFrequency::Normal) {
        hyphenPenalty *= 4.0;  // TODO: Replace with a better value after some testing
    }

    if (justified) {
        // Make hyphenation more aggressive for fully justified text (so that "normal" in
        // justified mode is the same as "full" in ragged-right).
        hyphenPenalty *= 0.25;
    } else {
        // Line penalty is zero for justified text.
        linePenalty = hyphenPenalty * LINE_PENALTY_MULTIPLIER;
    }

    return std::make_pair(hyphenPenalty, linePenalty);
}

// Represents a desperate break point.
struct DesperateBreak {
    // The break offset.
    uint32_t offset;

    // The sum of the character width from the beginning of the word.
    ParaWidth sumOfChars;

    DesperateBreak(uint32_t offset, ParaWidth sumOfChars)
            : offset(offset), sumOfChars(sumOfChars){};
};

// Retrieves desperate break points from a word.
std::vector<DesperateBreak> populateDesperatePoints(const MeasuredText& measured,
                                                    const Range& range) {
    std::vector<DesperateBreak> out;
    ParaWidth width = measured.widths[range.getStart()];
    for (uint32_t i = range.getStart() + 1; i < range.getEnd(); ++i) {
        const float w = measured.widths[i];
        if (w == 0) {
            continue;  // w == 0 means here is not a grapheme bounds. Don't break here.
        }
        out.emplace_back(i, width);
        width += w;
    }
    return out;
}

// Append hyphenation break points and desperate break points.
// If an offset is a both candidate for hyphenation and desperate break points, place desperate
// break candidate first and hyphenation break points second since the result width of the desperate
// break is shorter than hyphenation break.
// This is important since DP in computeBreaksOptimal assumes that the result line width is
// increased by break offset.
void appendWithMerging(std::vector<HyphenBreak>::const_iterator hyIter,
                       std::vector<HyphenBreak>::const_iterator endHyIter,
                       const std::vector<DesperateBreak>& desperates, const CharProcessor& proc,
                       float hyphenPenalty, bool isRtl, OptimizeContext* out) {
    auto d = desperates.begin();
    while (hyIter != endHyIter || d != desperates.end()) {
        // If both hyphen breaks and desperate breaks point to the same offset, push desperate
        // breaks first.
        if (d != desperates.end() && (hyIter == endHyIter || d->offset <= hyIter->offset)) {
            out->pushDesperate(d->offset, proc.sumOfCharWidthsAtPrevWordBreak + d->sumOfChars,
                               proc.effectiveSpaceCount, isRtl);
            d++;
        } else {
            out->pushHyphenation(hyIter->offset, proc.sumOfCharWidths - hyIter->second,
                                 proc.sumOfCharWidthsAtPrevWordBreak + hyIter->first, hyphenPenalty,
                                 proc.effectiveSpaceCount, hyIter->type, isRtl);
            hyIter++;
        }
    }
}

// Enumerate all line break candidates.
OptimizeContext populateCandidates(const U16StringPiece& textBuf, const MeasuredText& measured,
                                   const LineWidth& lineWidth, HyphenationFrequency frequency,
                                   bool isJustified) {
    const ParaWidth minLineWidth = lineWidth.getMin();
    CharProcessor proc(textBuf);

    OptimizeContext result;

    const bool doHyphenation = frequency != HyphenationFrequency::None;
    auto hyIter = std::begin(measured.hyphenBreaks);

    for (const auto& run : measured.runs) {
        const bool isRtl = run->isRtl();
        const Range& range = run->getRange();

        // Compute penalty parameters.
        float hyphenPenalty = 0.0f;
        if (run->canBreak()) {
            auto penalties = computePenalties(*run, lineWidth, frequency, isJustified);
            hyphenPenalty = penalties.first;
            result.linePenalty = std::max(penalties.second, result.linePenalty);
        }

        proc.updateLocaleIfNecessary(*run);

        for (uint32_t i = range.getStart(); i < range.getEnd(); ++i) {
            MINIKIN_ASSERT(textBuf[i] != CHAR_TAB, "TAB is not supported in optimal line breaker");
            proc.feedChar(i, textBuf[i], measured.widths[i], run->canBreak());

            const uint32_t nextCharOffset = i + 1;
            if (nextCharOffset != proc.nextWordBreak) {
                continue;  // Wait until word break point.
            }

            // Add hyphenation and desperate break points.
            std::vector<HyphenBreak> hyphenedBreaks;
            std::vector<DesperateBreak> desperateBreaks;
            const Range contextRange = proc.contextRange();

            auto beginHyIter = hyIter;
            while (hyIter != std::end(measured.hyphenBreaks) &&
                   hyIter->offset < contextRange.getEnd()) {
                hyIter++;
            }
            if (proc.widthFromLastWordBreak() > minLineWidth) {
                desperateBreaks = populateDesperatePoints(measured, contextRange);
            }
            appendWithMerging(beginHyIter, doHyphenation ? hyIter : beginHyIter, desperateBreaks,
                              proc, hyphenPenalty, isRtl, &result);

            // We skip breaks for zero-width characters inside replacement spans.
            if (run->getPaint() != nullptr || nextCharOffset == range.getEnd() ||
                measured.widths[nextCharOffset] > 0) {
                const float penalty = hyphenPenalty * proc.wordBreakPenalty();
                result.pushWordBreak(nextCharOffset, proc.sumOfCharWidths, proc.effectiveWidth,
                                     penalty, proc.rawSpaceCount, proc.effectiveSpaceCount, isRtl);
            }
        }
    }
    result.spaceWidth = proc.spaceWidth;
    return result;
}

class LineBreakOptimizer {
public:
    LineBreakOptimizer() {}

    LineBreakResult computeBreaks(const OptimizeContext& context, const U16StringPiece& textBuf,
                                  const MeasuredText& measuredText, const LineWidth& lineWidth,
                                  BreakStrategy strategy, bool justified);

private:
    // Data used to compute optimal line breaks
    struct OptimalBreaksData {
        float score;          // best score found for this break
        uint32_t prev;        // index to previous break
        uint32_t lineNumber;  // the computed line number of the candidate
    };
    LineBreakResult finishBreaksOptimal(const U16StringPiece& textBuf, const MeasuredText& measured,
                                        const std::vector<OptimalBreaksData>& breaksData,
                                        const std::vector<Candidate>& candidates);
};

// Follow "prev" links in candidates array, and copy to result arrays.
LineBreakResult LineBreakOptimizer::finishBreaksOptimal(
        const U16StringPiece& textBuf, const MeasuredText& measured,
        const std::vector<OptimalBreaksData>& breaksData,
        const std::vector<Candidate>& candidates) {
    LineBreakResult result;
    const uint32_t nCand = candidates.size();
    uint32_t prevIndex;
    for (uint32_t i = nCand - 1; i > 0; i = prevIndex) {
        prevIndex = breaksData[i].prev;
        const Candidate& cand = candidates[i];
        const Candidate& prev = candidates[prevIndex];

        result.breakPoints.push_back(cand.offset);
        result.widths.push_back(cand.postBreak - prev.preBreak);
        MinikinExtent extent = measured.getExtent(textBuf, Range(prev.offset, cand.offset));
        result.ascents.push_back(extent.ascent);
        result.descents.push_back(extent.descent);

        const HyphenEdit edit =
                packHyphenEdit(editForNextLine(prev.hyphenType), editForThisLine(cand.hyphenType));
        result.flags.push_back(static_cast<int>(edit));
    }
    result.reverse();
    return result;
}

LineBreakResult LineBreakOptimizer::computeBreaks(const OptimizeContext& context,
                                                  const U16StringPiece& textBuf,
                                                  const MeasuredText& measured,
                                                  const LineWidth& lineWidth,
                                                  BreakStrategy strategy, bool justified) {
    const std::vector<Candidate>& candidates = context.candidates;
    uint32_t active = 0;
    const uint32_t nCand = candidates.size();
    const float maxShrink = justified ? SHRINKABILITY * context.spaceWidth : 0.0f;

    std::vector<OptimalBreaksData> breaksData;
    breaksData.reserve(nCand);
    breaksData.push_back({0.0, 0, 0});  // The first candidate is always at the first line.

    // "i" iterates through candidates for the end of the line.
    for (uint32_t i = 1; i < nCand; i++) {
        const bool atEnd = i == nCand - 1;
        float best = SCORE_INFTY;
        uint32_t bestPrev = 0;

        uint32_t lineNumberLast = breaksData[active].lineNumber;
        float width = lineWidth.getAt(lineNumberLast);

        ParaWidth leftEdge = candidates[i].postBreak - width;
        float bestHope = 0;

        // "j" iterates through candidates for the beginning of the line.
        for (uint32_t j = active; j < i; j++) {
            const uint32_t lineNumber = breaksData[j].lineNumber;
            if (lineNumber != lineNumberLast) {
                const float widthNew = lineWidth.getAt(lineNumber);
                if (widthNew != width) {
                    leftEdge = candidates[i].postBreak - width;
                    bestHope = 0;
                    width = widthNew;
                }
                lineNumberLast = lineNumber;
            }
            const float jScore = breaksData[j].score;
            if (jScore + bestHope >= best) continue;
            const float delta = candidates[j].preBreak - leftEdge;

            // compute width score for line

            // Note: the "bestHope" optimization makes the assumption that, when delta is
            // non-negative, widthScore will increase monotonically as successive candidate
            // breaks are considered.
            float widthScore = 0.0f;
            float additionalPenalty = 0.0f;
            if ((atEnd || !justified) && delta < 0) {
                widthScore = SCORE_OVERFULL;
            } else if (atEnd && strategy != BreakStrategy::Balanced) {
                // increase penalty for hyphen on last line
                additionalPenalty = LAST_LINE_PENALTY_MULTIPLIER * candidates[j].penalty;
            } else {
                widthScore = delta * delta;
                if (delta < 0) {
                    if (-delta <
                        maxShrink * (candidates[i].postSpaceCount - candidates[j].preSpaceCount)) {
                        widthScore *= SHRINK_PENALTY_MULTIPLIER;
                    } else {
                        widthScore = SCORE_OVERFULL;
                    }
                }
            }

            if (delta < 0) {
                active = j + 1;
            } else {
                bestHope = widthScore;
            }

            const float score = jScore + widthScore + additionalPenalty;
            if (score <= best) {
                best = score;
                bestPrev = j;
            }
        }
        breaksData.push_back({best + candidates[i].penalty + context.linePenalty,  // score
                              bestPrev,                                            // prev
                              breaksData[bestPrev].lineNumber + 1});               // lineNumber
    }
    return finishBreaksOptimal(textBuf, measured, breaksData, candidates);
}

}  // namespace

LineBreakResult breakLineOptimal(const U16StringPiece& textBuf, const MeasuredText& measured,
                                 const LineWidth& lineWidth, BreakStrategy strategy,
                                 HyphenationFrequency frequency, bool justified) {
    if (textBuf.size() == 0) {
        return LineBreakResult();
    }
    const OptimizeContext context =
            populateCandidates(textBuf, measured, lineWidth, frequency, justified);
    LineBreakOptimizer optimizer;
    return optimizer.computeBreaks(context, textBuf, measured, lineWidth, strategy, justified);
}

}  // namespace minikin
