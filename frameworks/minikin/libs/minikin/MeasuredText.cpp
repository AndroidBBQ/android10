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

#define LOG_TAG "Minikin"
#include "minikin/MeasuredText.h"

#include "minikin/Layout.h"

#include "BidiUtils.h"
#include "LayoutSplitter.h"
#include "LayoutUtils.h"
#include "LineBreakerUtil.h"

namespace minikin {

// Helper class for composing character advances.
class AdvancesCompositor {
public:
    AdvancesCompositor(std::vector<float>* outAdvances, LayoutPieces* outPieces)
            : mOutAdvances(outAdvances), mOutPieces(outPieces) {}

    void setNextRange(const Range& range, bool dir) {
        mRange = range;
        mDir = dir;
    }

    void operator()(const LayoutPiece& layoutPiece, const MinikinPaint& paint) {
        const std::vector<float>& advances = layoutPiece.advances();
        std::copy(advances.begin(), advances.end(), mOutAdvances->begin() + mRange.getStart());

        if (mOutPieces != nullptr) {
            mOutPieces->insert(mRange, 0 /* no edit */, layoutPiece, mDir, paint);
        }
    }

private:
    Range mRange;
    bool mDir;
    std::vector<float>* mOutAdvances;
    LayoutPieces* mOutPieces;
};

void StyleRun::getMetrics(const U16StringPiece& textBuf, std::vector<float>* advances,
                          LayoutPieces* precomputed, LayoutPieces* outPieces) const {
    AdvancesCompositor compositor(advances, outPieces);
    const Bidi bidiFlag = mIsRtl ? Bidi::FORCE_RTL : Bidi::FORCE_LTR;
    const uint32_t paintId =
            (precomputed == nullptr) ? LayoutPieces::kNoPaintId : precomputed->findPaintId(mPaint);
    for (const BidiText::RunInfo info : BidiText(textBuf, mRange, bidiFlag)) {
        for (const auto[context, piece] : LayoutSplitter(textBuf, info.range, info.isRtl)) {
            compositor.setNextRange(piece, info.isRtl);
            if (paintId == LayoutPieces::kNoPaintId) {
                LayoutCache::getInstance().getOrCreate(
                        textBuf.substr(context), piece - context.getStart(), mPaint, info.isRtl,
                        StartHyphenEdit::NO_EDIT, EndHyphenEdit::NO_EDIT, compositor);
            } else {
                precomputed->getOrCreate(textBuf, piece, context, mPaint, info.isRtl,
                                         StartHyphenEdit::NO_EDIT, EndHyphenEdit::NO_EDIT, paintId,
                                         compositor);
            }
        }
    }
}

// Helper class for composing total amount of advance
class TotalAdvanceCompositor {
public:
    TotalAdvanceCompositor(LayoutPieces* outPieces) : mTotalAdvance(0), mOutPieces(outPieces) {}

    void setNextContext(const Range& range, HyphenEdit edit, bool dir) {
        mRange = range;
        mEdit = edit;
        mDir = dir;
    }

    void operator()(const LayoutPiece& layoutPiece, const MinikinPaint& paint) {
        mTotalAdvance += layoutPiece.advance();
        if (mOutPieces != nullptr) {
            mOutPieces->insert(mRange, mEdit, layoutPiece, mDir, paint);
        }
    }

    float advance() const { return mTotalAdvance; }

private:
    float mTotalAdvance;
    Range mRange;
    HyphenEdit mEdit;
    bool mDir;
    LayoutPieces* mOutPieces;
};

float StyleRun::measureHyphenPiece(const U16StringPiece& textBuf, const Range& range,
                                   StartHyphenEdit startHyphen, EndHyphenEdit endHyphen,
                                   LayoutPieces* pieces) const {
    TotalAdvanceCompositor compositor(pieces);
    const Bidi bidiFlag = mIsRtl ? Bidi::FORCE_RTL : Bidi::FORCE_LTR;
    for (const BidiText::RunInfo info : BidiText(textBuf, range, bidiFlag)) {
        for (const auto[context, piece] : LayoutSplitter(textBuf, info.range, info.isRtl)) {
            const StartHyphenEdit startEdit =
                    piece.getStart() == range.getStart() ? startHyphen : StartHyphenEdit::NO_EDIT;
            const EndHyphenEdit endEdit =
                    piece.getEnd() == range.getEnd() ? endHyphen : EndHyphenEdit::NO_EDIT;

            compositor.setNextContext(piece, packHyphenEdit(startEdit, endEdit), info.isRtl);
            LayoutCache::getInstance().getOrCreate(textBuf.substr(context),
                                                   piece - context.getStart(), mPaint, info.isRtl,
                                                   startEdit, endEdit, compositor);
        }
    }
    return compositor.advance();
}

void MeasuredText::measure(const U16StringPiece& textBuf, bool computeHyphenation,
                           bool computeLayout, MeasuredText* hint) {
    if (textBuf.size() == 0) {
        return;
    }

    LayoutPieces* piecesOut = computeLayout ? &layoutPieces : nullptr;
    CharProcessor proc(textBuf);
    for (const auto& run : runs) {
        const Range& range = run->getRange();
        run->getMetrics(textBuf, &widths, hint ? &hint->layoutPieces : nullptr, piecesOut);

        if (!computeHyphenation || !run->canBreak()) {
            continue;
        }

        proc.updateLocaleIfNecessary(*run);
        for (uint32_t i = range.getStart(); i < range.getEnd(); ++i) {
            proc.feedChar(i, textBuf[i], widths[i], run->canBreak());

            const uint32_t nextCharOffset = i + 1;
            if (nextCharOffset != proc.nextWordBreak) {
                continue;  // Wait until word break point.
            }

            populateHyphenationPoints(textBuf, *run, *proc.hyphenator, proc.contextRange(),
                                      proc.wordRange(), &hyphenBreaks, piecesOut);
        }
    }
}

// Helper class for composing Layout object.
class LayoutCompositor {
public:
    LayoutCompositor(Layout* outLayout, float extraAdvance)
            : mOutLayout(outLayout), mExtraAdvance(extraAdvance) {}

    void setOutOffset(uint32_t outOffset) { mOutOffset = outOffset; }

    void operator()(const LayoutPiece& layoutPiece, const MinikinPaint& /* paint */) {
        mOutLayout->appendLayout(layoutPiece, mOutOffset, mExtraAdvance);
    }

    uint32_t mOutOffset;
    Layout* mOutLayout;
    float mExtraAdvance;
};

void StyleRun::appendLayout(const U16StringPiece& textBuf, const Range& range,
                            const Range& /* context */, const LayoutPieces& pieces,
                            const MinikinPaint& paint, uint32_t outOrigin,
                            StartHyphenEdit startHyphen, EndHyphenEdit endHyphen,
                            Layout* outLayout) const {
    float wordSpacing = range.getLength() == 1 && isWordSpace(textBuf[range.getStart()])
                                ? mPaint.wordSpacing
                                : 0;
    bool canUsePrecomputedResult = mPaint == paint;

    LayoutCompositor compositor(outLayout, wordSpacing);
    const Bidi bidiFlag = mIsRtl ? Bidi::FORCE_RTL : Bidi::FORCE_LTR;
    const uint32_t paintId = pieces.findPaintId(mPaint);
    for (const BidiText::RunInfo info : BidiText(textBuf, range, bidiFlag)) {
        for (const auto[context, piece] : LayoutSplitter(textBuf, info.range, info.isRtl)) {
            compositor.setOutOffset(piece.getStart() - outOrigin);
            const StartHyphenEdit startEdit =
                    range.getStart() == piece.getStart() ? startHyphen : StartHyphenEdit::NO_EDIT;
            const EndHyphenEdit endEdit =
                    range.getEnd() == piece.getEnd() ? endHyphen : EndHyphenEdit::NO_EDIT;

            if (canUsePrecomputedResult) {
                pieces.getOrCreate(textBuf, piece, context, mPaint, info.isRtl, startEdit, endEdit,
                                   paintId, compositor);
            } else {
                LayoutCache::getInstance().getOrCreate(textBuf.substr(context),
                                                       piece - context.getStart(), paint,
                                                       info.isRtl, startEdit, endEdit, compositor);
            }
        }
    }
}

// Helper class for composing bounding box.
class BoundsCompositor {
public:
    BoundsCompositor() : mAdvance(0) {}

    void operator()(const LayoutPiece& layoutPiece, const MinikinPaint& /* paint */) {
        MinikinRect tmpBounds = layoutPiece.bounds();
        tmpBounds.offset(mAdvance, 0);
        mBounds.join(tmpBounds);
        mAdvance += layoutPiece.advance();
    }

    const MinikinRect& bounds() const { return mBounds; }
    float advance() const { return mAdvance; }

private:
    float mAdvance;
    MinikinRect mBounds;
};

std::pair<float, MinikinRect> StyleRun::getBounds(const U16StringPiece& textBuf, const Range& range,
                                                  const LayoutPieces& pieces) const {
    BoundsCompositor compositor;
    const Bidi bidiFlag = mIsRtl ? Bidi::FORCE_RTL : Bidi::FORCE_LTR;
    const uint32_t paintId = pieces.findPaintId(mPaint);
    for (const BidiText::RunInfo info : BidiText(textBuf, range, bidiFlag)) {
        for (const auto[context, piece] : LayoutSplitter(textBuf, info.range, info.isRtl)) {
            pieces.getOrCreate(textBuf, piece, context, mPaint, info.isRtl,
                               StartHyphenEdit::NO_EDIT, EndHyphenEdit::NO_EDIT, paintId,
                               compositor);
        }
    }
    return std::make_pair(compositor.advance(), compositor.bounds());
}

// Helper class for composing total extent.
class ExtentCompositor {
public:
    ExtentCompositor() {}

    void operator()(const LayoutPiece& layoutPiece, const MinikinPaint& /* paint */) {
        mExtent.extendBy(layoutPiece.extent());
    }

    const MinikinExtent& extent() const { return mExtent; }

private:
    MinikinExtent mExtent;
};

MinikinExtent StyleRun::getExtent(const U16StringPiece& textBuf, const Range& range,
                                  const LayoutPieces& pieces) const {
    ExtentCompositor compositor;
    Bidi bidiFlag = mIsRtl ? Bidi::FORCE_RTL : Bidi::FORCE_LTR;
    const uint32_t paintId = pieces.findPaintId(mPaint);
    for (const BidiText::RunInfo info : BidiText(textBuf, range, bidiFlag)) {
        for (const auto[context, piece] : LayoutSplitter(textBuf, info.range, info.isRtl)) {
            pieces.getOrCreate(textBuf, piece, context, mPaint, info.isRtl,
                               StartHyphenEdit::NO_EDIT, EndHyphenEdit::NO_EDIT, paintId,
                               compositor);
        }
    }
    return compositor.extent();
}

Layout MeasuredText::buildLayout(const U16StringPiece& textBuf, const Range& range,
                                 const Range& contextRange, const MinikinPaint& paint,
                                 StartHyphenEdit startHyphen, EndHyphenEdit endHyphen) {
    Layout outLayout(range.getLength());
    for (const auto& run : runs) {
        const Range& runRange = run->getRange();
        if (!Range::intersects(range, runRange)) {
            continue;
        }
        const Range targetRange = Range::intersection(runRange, range);
        StartHyphenEdit startEdit =
                targetRange.getStart() == range.getStart() ? startHyphen : StartHyphenEdit::NO_EDIT;
        EndHyphenEdit endEdit =
                targetRange.getEnd() == range.getEnd() ? endHyphen : EndHyphenEdit::NO_EDIT;
        run->appendLayout(textBuf, targetRange, contextRange, layoutPieces, paint, range.getStart(),
                          startEdit, endEdit, &outLayout);
    }
    return outLayout;
}

MinikinRect MeasuredText::getBounds(const U16StringPiece& textBuf, const Range& range) const {
    MinikinRect rect;
    float totalAdvance = 0.0f;

    for (const auto& run : runs) {
        const Range& runRange = run->getRange();
        if (!Range::intersects(range, runRange)) {
            continue;
        }
        auto[advance, bounds] =
                run->getBounds(textBuf, Range::intersection(runRange, range), layoutPieces);
        bounds.offset(totalAdvance, 0);
        rect.join(bounds);
        totalAdvance += advance;
    }
    return rect;
}

MinikinExtent MeasuredText::getExtent(const U16StringPiece& textBuf, const Range& range) const {
    MinikinExtent extent;
    for (const auto& run : runs) {
        const Range& runRange = run->getRange();
        if (!Range::intersects(range, runRange)) {
            continue;
        }
        MinikinExtent runExtent =
                run->getExtent(textBuf, Range::intersection(runRange, range), layoutPieces);
        extent.extendBy(runExtent);
    }
    return extent;
}

}  // namespace minikin
