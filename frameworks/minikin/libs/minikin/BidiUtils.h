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

#ifndef MINIKIN_BIDI_UTILS_H
#define MINIKIN_BIDI_UTILS_H

#define LOG_TAG "Minikin"

#include "minikin/Layout.h"

#include <memory>

#include <unicode/ubidi.h>

#include "minikin/Macros.h"
#include "minikin/U16StringPiece.h"

namespace minikin {

struct UBiDiDeleter {
    void operator()(UBiDi* v) { ubidi_close(v); }
};

using UBiDiUniquePtr = std::unique_ptr<UBiDi, UBiDiDeleter>;

// A helper class for iterating the bidi run transitions.
class BidiText {
public:
    struct RunInfo {
        Range range;
        bool isRtl;
    };

    BidiText(const U16StringPiece& textBuf, const Range& range, Bidi bidiFlags);

    RunInfo getRunInfoAt(uint32_t runOffset) const;

    class iterator {
    public:
        inline bool operator==(const iterator& o) const {
            return mRunOffset == o.mRunOffset && mParent == o.mParent;
        }

        inline bool operator!=(const iterator& o) const { return !(*this == o); }

        inline RunInfo operator*() const { return mParent->getRunInfoAt(mRunOffset); }

        inline iterator& operator++() {
            mRunOffset++;
            return *this;
        }

    private:
        friend class BidiText;

        iterator(const BidiText* parent, uint32_t runOffset)
                : mParent(parent), mRunOffset(runOffset) {}

        const BidiText* mParent;
        uint32_t mRunOffset;
    };

    inline iterator begin() const { return iterator(this, 0); }
    inline iterator end() const { return iterator(this, mRunCount); }

private:
    UBiDiUniquePtr mBidi;  // Maybe null for single run.
    const Range mRange;    // The range in the original buffer. Used for range check.
    bool mIsRtl;           // The paragraph direction.
    uint32_t mRunCount;    // The number of the bidi run in this text.

    MINIKIN_PREVENT_COPY_AND_ASSIGN(BidiText);
};

}  // namespace minikin

#endif  // MINIKIN_BIDI_UTILS_H
