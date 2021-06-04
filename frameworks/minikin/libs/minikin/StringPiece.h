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

#ifndef MINIKIN_STRING_PIECE_H
#define MINIKIN_STRING_PIECE_H

#include <cstdint>
#include <string>
#include <vector>

namespace minikin {

class StringPiece {
public:
    StringPiece() : mData(nullptr), mLength(0) {}
    StringPiece(const char* data) : mData(data), mLength(data == nullptr ? 0 : strlen(data)) {}
    StringPiece(const char* data, size_t length) : mData(data), mLength(length) {}
    StringPiece(const std::string& str) : mData(str.data()), mLength(str.size()) {}

    inline const char* data() const { return mData; }
    inline size_t length() const { return mLength; }
    inline size_t size() const { return mLength; }
    inline bool empty() const { return mLength == 0; }

    inline char operator[](size_t i) const { return mData[i]; }

    inline StringPiece substr(size_t from, size_t length) const {
        return StringPiece(mData + from, length);
    }

    inline size_t find(size_t from, char c) const {
        if (from >= mLength) {
            return mLength;
        }
        const char* p = static_cast<const char*>(memchr(mData + from, c, mLength - from));
        return p == nullptr ? mLength : p - mData;
    }

    std::string toString() const { return std::string(mData, mData + mLength); }

private:
    const char* mData;
    size_t mLength;
};

inline bool operator==(const StringPiece& l, const StringPiece& r) {
    const size_t len = l.size();
    if (len != r.size()) {
        return false;
    }
    const char* lData = l.data();
    const char* rData = r.data();
    if (lData == rData) {
        return true;
    }
    return memcmp(lData, rData, len) == 0;
}

inline bool operator==(const StringPiece& l, const char* s) {
    const size_t len = l.size();
    if (len != strlen(s)) {
        return false;
    }
    return memcmp(l.data(), s, len) == 0;
}

inline bool operator!=(const StringPiece& l, const StringPiece& r) {
    return !(l == r);
}

inline bool operator!=(const StringPiece& l, const char* s) {
    return !(l == s);
}

class SplitIterator {
public:
    SplitIterator(const StringPiece& string, char delimiter)
            : mStarted(false), mCurrent(0), mString(string), mDelimiter(delimiter) {}

    inline StringPiece next() {
        if (!hasNext()) {
            return StringPiece();
        }
        const size_t searchFrom = mStarted ? mCurrent + 1 : 0;
        mStarted = true;
        mCurrent = mString.find(searchFrom, mDelimiter);
        return mString.substr(searchFrom, mCurrent - searchFrom);
    }
    inline bool hasNext() const { return mCurrent < mString.size(); }

private:
    bool mStarted;
    size_t mCurrent;
    StringPiece mString;
    char mDelimiter;
};

}  // namespace minikin

#endif  // MINIKIN_STRING_PIECE_H
