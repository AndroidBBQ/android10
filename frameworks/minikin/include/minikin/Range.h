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

#ifndef MINIKIN_RANGE_H
#define MINIKIN_RANGE_H

#include <algorithm>
#include <limits>
#include <utility>

namespace minikin {

// An undirected range.
class Range {
public:
    static constexpr uint32_t NOWHERE = std::numeric_limits<uint32_t>::max();

    // start must be smaller than or equal to end otherwise the behavior is undefined.
    Range(uint32_t start, uint32_t end) : mStart(start), mEnd(end) {}
    Range() : Range(NOWHERE, NOWHERE) {}

    Range(const Range&) = default;
    Range& operator=(const Range&) = default;

    static Range invalidRange() { return Range(NOWHERE, NOWHERE); }
    inline bool isValid() const { return mStart != NOWHERE && mEnd != NOWHERE; }

    inline uint32_t getStart() const { return mStart; }       // inclusive
    inline void setStart(uint32_t start) { mStart = start; }  // inclusive

    inline uint32_t getEnd() const { return mEnd; }   // exclusive
    inline void setEnd(uint32_t end) { mEnd = end; }  // exclusive

    inline uint32_t getLength() const { return mEnd - mStart; }

    inline bool isEmpty() const { return mStart == mEnd; }

    inline uint32_t toRangeOffset(uint32_t globalPos) const { return globalPos - mStart; }
    inline uint32_t toGlobalOffset(uint32_t rangePos) const { return mStart + rangePos; }

    // The behavior is undefined if pos is out of range.
    inline std::pair<Range, Range> split(uint32_t pos) const {
        return std::make_pair(Range(mStart, pos), Range(pos, mEnd));
    }

    inline bool contains(const Range& other) const {
        return mStart <= other.mStart && other.mEnd <= mEnd;
    }

    // Returns true if the pos is in this range.
    // For example,
    //   const Range range(1, 2);  // 1 is inclusive, 2 is exclusive.
    //   range.contains(0);  // false
    //   range.contains(1);  // true
    //   range.contains(2);  // false
    inline bool contains(uint32_t pos) const { return mStart <= pos && pos < mEnd; }

    // Returns true if left and right intersect.
    inline static bool intersects(const Range& left, const Range& right) {
        return left.isValid() && right.isValid() && left.mStart < right.mEnd &&
               right.mStart < left.mEnd;
    }
    inline static Range intersection(const Range& left, const Range& right) {
        return Range(std::max(left.mStart, right.mStart), std::min(left.mEnd, right.mEnd));
    }

    // Returns merged range. This method assumes left and right are not invalid ranges and they have
    // an intersection.
    static Range merge(const Range& left, const Range& right) {
        return Range({std::min(left.mStart, right.mStart), std::max(left.mEnd, right.mEnd)});
    }

    inline bool operator==(const Range& o) const { return mStart == o.mStart && mEnd == o.mEnd; }

    inline bool operator!=(const Range& o) const { return !(*this == o); }

    inline Range operator+(int32_t shift) const { return Range(mStart + shift, mEnd + shift); }

    inline Range operator-(int32_t shift) const { return Range(mStart - shift, mEnd - shift); }

private:
    // Helper class for "for (uint32_t i : range)" style for-loop.
    class RangeIterator {
    public:
        RangeIterator(uint32_t pos) : mPos(pos) {}

        inline bool operator!=(const RangeIterator& o) const { return o.mPos != mPos; }
        inline uint32_t operator*() const { return mPos; }
        inline RangeIterator& operator++() {
            mPos++;
            return *this;
        }

    private:
        uint32_t mPos;
    };

public:
    inline RangeIterator begin() const { return RangeIterator(mStart); }
    inline RangeIterator end() const { return RangeIterator(mEnd); }

private:
    uint32_t mStart;
    uint32_t mEnd;
};

}  // namespace minikin

#endif  // MINIKIN_RANGE_H
