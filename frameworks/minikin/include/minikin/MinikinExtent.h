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

#ifndef MINIKIN_MINIKIN_EXTENT_H
#define MINIKIN_MINIKIN_EXTENT_H

#include <ostream>

namespace minikin {

// For holding vertical extents.
struct MinikinExtent {
    MinikinExtent() : ascent(0), descent(0) {}
    MinikinExtent(float ascent, float descent) : ascent(ascent), descent(descent) {}
    bool operator==(const MinikinExtent& o) const {
        return ascent == o.ascent && descent == o.descent;
    }
    float ascent;   // negative
    float descent;  // positive

    void reset() { ascent = descent = 0.0; }

    void extendBy(const MinikinExtent& e) {
        ascent = std::min(ascent, e.ascent);
        descent = std::max(descent, e.descent);
    }
};

// For gtest output
inline std::ostream& operator<<(std::ostream& os, const MinikinExtent& e) {
    return os << e.ascent << ", " << e.descent;
}
}  // namespace minikin

#endif  // MINIKIN_MINIKIN_EXTENT_H
