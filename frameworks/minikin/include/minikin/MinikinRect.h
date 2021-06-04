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

#ifndef MINIKIN_MINIKIN_RECT_H
#define MINIKIN_MINIKIN_RECT_H

#include <ostream>

namespace minikin {

struct MinikinRect {
    MinikinRect() : mLeft(0), mTop(0), mRight(0), mBottom(0) {}
    MinikinRect(float left, float top, float right, float bottom)
            : mLeft(left), mTop(top), mRight(right), mBottom(bottom) {}
    bool operator==(const MinikinRect& o) const {
        return mLeft == o.mLeft && mTop == o.mTop && mRight == o.mRight && mBottom == o.mBottom;
    }
    float mLeft;
    float mTop;
    float mRight;
    float mBottom;

    bool isEmpty() const { return mLeft == mRight || mTop == mBottom; }
    void set(const MinikinRect& r) {
        mLeft = r.mLeft;
        mTop = r.mTop;
        mRight = r.mRight;
        mBottom = r.mBottom;
    }
    void offset(float dx, float dy) {
        mLeft += dx;
        mTop += dy;
        mRight += dx;
        mBottom += dy;
    }
    void setEmpty() { mLeft = mTop = mRight = mBottom = 0.0; }
    void join(const MinikinRect& r) {
        if (isEmpty()) {
            set(r);
        } else if (!r.isEmpty()) {
            mLeft = std::min(mLeft, r.mLeft);
            mTop = std::min(mTop, r.mTop);
            mRight = std::max(mRight, r.mRight);
            mBottom = std::max(mBottom, r.mBottom);
        }
    }
};

// For gtest output
inline std::ostream& operator<<(std::ostream& os, const MinikinRect& r) {
    return os << "(" << r.mLeft << ", " << r.mTop << ")-(" << r.mRight << ", " << r.mBottom << ")";
}

}  // namespace minikin

#endif  // MINIKIN_MINIKIN_RECT_H
