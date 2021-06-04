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

#ifndef MINIKIN_U16STRING_PIECE_H
#define MINIKIN_U16STRING_PIECE_H

#include <vector>

#include "minikin/Range.h"

namespace minikin {

class U16StringPiece {
public:
    U16StringPiece() : mData(nullptr), mLength(0) {}
    U16StringPiece(const uint16_t* data, uint32_t length) : mData(data), mLength(length) {}
    U16StringPiece(const std::vector<uint16_t>& v)  // Intentionally not explicit.
            : mData(v.data()), mLength(static_cast<uint32_t>(v.size())) {}
    template <uint32_t length>
    U16StringPiece(uint16_t const (&data)[length]) : mData(data), mLength(length) {}

    U16StringPiece(const U16StringPiece&) = default;
    U16StringPiece& operator=(const U16StringPiece&) = default;

    inline const uint16_t* data() const { return mData; }
    inline uint32_t size() const { return mLength; }
    inline uint32_t length() const { return mLength; }

    // Undefined behavior if pos is out of range.
    inline const uint16_t& at(uint32_t pos) const { return mData[pos]; }
    inline const uint16_t& operator[](uint32_t pos) const { return mData[pos]; }

    inline U16StringPiece substr(const Range& range) const {
        return U16StringPiece(mData + range.getStart(), range.getLength());
    }

    inline bool hasChar(uint16_t c) const {
        const uint16_t* end = mData + mLength;
        return std::find(mData, end, c) != end;
    }

private:
    const uint16_t* mData;
    uint32_t mLength;
};

}  // namespace minikin

#endif  // MINIKIN_U16STRING_PIECE_H
