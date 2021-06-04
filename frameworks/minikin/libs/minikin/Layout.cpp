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

#include "minikin/Layout.h"

#include <cmath>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#include <hb-icu.h>
#include <hb-ot.h>
#include <log/log.h>
#include <unicode/ubidi.h>
#include <unicode/utf16.h>
#include <utils/LruCache.h>

#include "minikin/Emoji.h"
#include "minikin/HbUtils.h"
#include "minikin/LayoutCache.h"
#include "minikin/LayoutPieces.h"
#include "minikin/Macros.h"

#include "BidiUtils.h"
#include "LayoutSplitter.h"
#include "LayoutUtils.h"
#include "LocaleListCache.h"
#include "MinikinInternal.h"

namespace minikin {

void Layout::doLayout(const U16StringPiece& textBuf, const Range& range, Bidi bidiFlags,
                      const MinikinPaint& paint, StartHyphenEdit startHyphen,
                      EndHyphenEdit endHyphen) {
    const uint32_t count = range.getLength();
    mAdvances.resize(count, 0);
    mGlyphs.reserve(count);
    for (const BidiText::RunInfo& runInfo : BidiText(textBuf, range, bidiFlags)) {
        doLayoutRunCached(textBuf, runInfo.range, runInfo.isRtl, paint, range.getStart(),
                          startHyphen, endHyphen, this, nullptr);
    }
}

float Layout::measureText(const U16StringPiece& textBuf, const Range& range, Bidi bidiFlags,
                          const MinikinPaint& paint, StartHyphenEdit startHyphen,
                          EndHyphenEdit endHyphen, float* advances) {
    float advance = 0;
    for (const BidiText::RunInfo& runInfo : BidiText(textBuf, range, bidiFlags)) {
        const size_t offset = range.toRangeOffset(runInfo.range.getStart());
        float* advancesForRun = advances ? advances + offset : nullptr;
        advance += doLayoutRunCached(textBuf, runInfo.range, runInfo.isRtl, paint, 0, startHyphen,
                                     endHyphen, nullptr, advancesForRun);
    }
    return advance;
}

float Layout::doLayoutRunCached(const U16StringPiece& textBuf, const Range& range, bool isRtl,
                                const MinikinPaint& paint, size_t dstStart,
                                StartHyphenEdit startHyphen, EndHyphenEdit endHyphen,
                                Layout* layout, float* advances) {
    if (!range.isValid()) {
        return 0.0f;  // ICU failed to retrieve the bidi run?
    }
    float advance = 0;
    for (const auto[context, piece] : LayoutSplitter(textBuf, range, isRtl)) {
        // Hyphenation only applies to the start/end of run.
        const StartHyphenEdit pieceStartHyphen =
                (piece.getStart() == range.getStart()) ? startHyphen : StartHyphenEdit::NO_EDIT;
        const EndHyphenEdit pieceEndHyphen =
                (piece.getEnd() == range.getEnd()) ? endHyphen : EndHyphenEdit::NO_EDIT;
        float* advancesForRun =
                advances ? advances + (piece.getStart() - range.getStart()) : nullptr;
        advance += doLayoutWord(textBuf.data() + context.getStart(),
                                piece.getStart() - context.getStart(), piece.getLength(),
                                context.getLength(), isRtl, paint, piece.getStart() - dstStart,
                                pieceStartHyphen, pieceEndHyphen, layout, advancesForRun);
    }
    return advance;
}

class LayoutAppendFunctor {
public:
    LayoutAppendFunctor(Layout* layout, float* advances, float* totalAdvance, uint32_t outOffset,
                        float wordSpacing)
            : mLayout(layout),
              mAdvances(advances),
              mTotalAdvance(totalAdvance),
              mOutOffset(outOffset),
              mWordSpacing(wordSpacing) {}

    void operator()(const LayoutPiece& layoutPiece, const MinikinPaint& /* paint */) {
        if (mLayout) {
            mLayout->appendLayout(layoutPiece, mOutOffset, mWordSpacing);
        }
        if (mAdvances) {
            const std::vector<float>& advances = layoutPiece.advances();
            std::copy(advances.begin(), advances.end(), mAdvances);
        }
        if (mTotalAdvance) {
            *mTotalAdvance = layoutPiece.advance();
        }
    }

private:
    Layout* mLayout;
    float* mAdvances;
    float* mTotalAdvance;
    const uint32_t mOutOffset;
    const float mWordSpacing;
};

float Layout::doLayoutWord(const uint16_t* buf, size_t start, size_t count, size_t bufSize,
                           bool isRtl, const MinikinPaint& paint, size_t bufStart,
                           StartHyphenEdit startHyphen, EndHyphenEdit endHyphen, Layout* layout,
                           float* advances) {
    float wordSpacing = count == 1 && isWordSpace(buf[start]) ? paint.wordSpacing : 0;
    float totalAdvance = 0;

    const U16StringPiece textBuf(buf, bufSize);
    const Range range(start, start + count);
    LayoutAppendFunctor f(layout, advances, &totalAdvance, bufStart, wordSpacing);
    LayoutCache::getInstance().getOrCreate(textBuf, range, paint, isRtl, startHyphen, endHyphen, f);

    if (wordSpacing != 0) {
        totalAdvance += wordSpacing;
        if (advances) {
            advances[0] += wordSpacing;
        }
    }
    return totalAdvance;
}

void Layout::appendLayout(const LayoutPiece& src, size_t start, float extraAdvance) {
    for (size_t i = 0; i < src.glyphCount(); i++) {
        mGlyphs.emplace_back(src.fontAt(i), src.glyphIdAt(i), mAdvance + src.pointAt(i).x,
                             src.pointAt(i).y);
    }
    const std::vector<float>& advances = src.advances();
    for (size_t i = 0; i < advances.size(); i++) {
        mAdvances[i + start] = advances[i];
        if (i == 0) {
            mAdvances[start] += extraAdvance;
        }
    }
    MinikinRect srcBounds(src.bounds());
    srcBounds.offset(mAdvance, 0);
    mBounds.join(srcBounds);
    mAdvance += src.advance() + extraAdvance;
}

void Layout::purgeCaches() {
    LayoutCache::getInstance().clear();
}

void Layout::dumpMinikinStats(int fd) {
    LayoutCache::getInstance().dumpStats(fd);
}

}  // namespace minikin
