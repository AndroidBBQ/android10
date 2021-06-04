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

#ifndef MINIKIN_LAYOUT_SPLITTER_H
#define MINIKIN_LAYOUT_SPLITTER_H

#define LOG_TAG "Minikin"

#include "minikin/Layout.h"

#include <memory>

#include <unicode/ubidi.h>

#include "minikin/Macros.h"
#include "minikin/U16StringPiece.h"

#include "LayoutUtils.h"

namespace minikin {

// LayoutSplitter split the input text into recycle-able pieces.
//
// LayoutSplitter basically splits the text before and after space characters.
//
// Here is an example of how the LayoutSplitter split the text into layout pieces.
// Input:
//   Text          : T h i s _ i s _ a n _ e x a m p l e _ t e x t .
//   Range         :            |-------------------|
//
// Output:
//   Context Range :          |---|-|---|-|-------------|
//   Piece Range   :            |-|-|---|-|---------|
//
// Input:
//   Text          : T h i s _ i s _ a n _ e x a m p l e _ t e x t .
//   Range         :                          |-------|
//
// Output:
//   Context Range :                      |-------------|
//   Piece Range   :                          |-------|
class LayoutSplitter {
public:
    LayoutSplitter(const U16StringPiece& textBuf, const Range& range, bool isRtl)
            : mTextBuf(textBuf), mRange(range), mIsRtl(isRtl) {}

    class iterator {
    public:
        bool operator==(const iterator& o) const { return mPos == o.mPos && mParent == o.mParent; }

        bool operator!=(const iterator& o) const { return !(*this == o); }

        std::pair<Range, Range> operator*() const {
            return std::make_pair(mContextRange, mPieceRange);
        }

        iterator& operator++() {
            const U16StringPiece& textBuf = mParent->mTextBuf;
            const Range& range = mParent->mRange;
            if (mParent->mIsRtl) {
                mPos = mPieceRange.getStart();
                mContextRange.setStart(getPrevWordBreakForCache(textBuf, mPos));
                mContextRange.setEnd(mPos);
                mPieceRange.setStart(std::max(mContextRange.getStart(), range.getStart()));
                mPieceRange.setEnd(mPos);
            } else {
                mPos = mPieceRange.getEnd();
                mContextRange.setStart(mPos);
                mContextRange.setEnd(getNextWordBreakForCache(textBuf, mPos));
                mPieceRange.setStart(mPos);
                mPieceRange.setEnd(std::min(mContextRange.getEnd(), range.getEnd()));
            }
            return *this;
        }

    private:
        friend class LayoutSplitter;

        iterator(const LayoutSplitter* parent, uint32_t pos) : mParent(parent), mPos(pos) {
            const U16StringPiece& textBuf = mParent->mTextBuf;
            const Range& range = mParent->mRange;
            if (parent->mIsRtl) {
                mContextRange.setStart(getPrevWordBreakForCache(textBuf, pos));
                mContextRange.setEnd(getNextWordBreakForCache(textBuf, pos == 0 ? 0 : pos - 1));
                mPieceRange.setStart(std::max(mContextRange.getStart(), range.getStart()));
                mPieceRange.setEnd(pos);
            } else {
                mContextRange.setStart(
                        getPrevWordBreakForCache(textBuf, pos == range.getEnd() ? pos : pos + 1));
                mContextRange.setEnd(getNextWordBreakForCache(textBuf, pos));
                mPieceRange.setStart(pos);
                mPieceRange.setEnd(std::min(mContextRange.getEnd(), range.getEnd()));
            }
        }

        const LayoutSplitter* mParent;
        uint32_t mPos;
        Range mContextRange;
        Range mPieceRange;
    };

    iterator begin() const { return iterator(this, mIsRtl ? mRange.getEnd() : mRange.getStart()); }
    iterator end() const { return iterator(this, mIsRtl ? mRange.getStart() : mRange.getEnd()); }

private:
    U16StringPiece mTextBuf;
    Range mRange;  // The range in the original buffer. Used for range check.
    bool mIsRtl;   // The paragraph direction.

    MINIKIN_PREVENT_COPY_AND_ASSIGN(LayoutSplitter);
};

}  // namespace minikin

#endif  // MINIKIN_LAYOUT_SPLITTER_H
