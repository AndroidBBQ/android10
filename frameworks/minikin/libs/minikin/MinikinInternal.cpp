/*
 * Copyright (C) 2014 The Android Open Source Project
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
// Definitions internal to Minikin

#define LOG_TAG "Minikin"

#include <log/log.h>

#include "MinikinInternal.h"

namespace minikin {

inline static bool isBMPVariationSelector(uint32_t codePoint) {
    return VS1 <= codePoint && codePoint <= VS16;
}

inline static bool isVariationSelectorSupplement(uint32_t codePoint) {
    return VS17 <= codePoint && codePoint <= VS256;
}

uint16_t getVsIndex(uint32_t codePoint) {
    if (isBMPVariationSelector(codePoint)) {
        return codePoint - VS1;
    } else if (isVariationSelectorSupplement(codePoint)) {
        return codePoint - VS17 + 16;
    } else {
        return INVALID_VS_INDEX;
    }
}

bool isVariationSelector(uint32_t codePoint) {
    return isBMPVariationSelector(codePoint) || isVariationSelectorSupplement(codePoint);
}

}  // namespace minikin
