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

#ifndef MINIKIN_FONT_VARIATION_H
#define MINIKIN_FONT_VARIATION_H

#include <cstdint>

namespace minikin {

typedef uint32_t AxisTag;

struct FontVariation {
    FontVariation(AxisTag axisTag, float value) : axisTag(axisTag), value(value) {}
    AxisTag axisTag;
    float value;
};

}  // namespace minikin

#endif  // MINIKIN_FONT_VARIATION_H
