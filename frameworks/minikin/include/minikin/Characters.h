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

#ifndef MINIKIN_CHARACTERS_H
#define MINIKIN_CHARACTERS_H

#include <cstdint>

namespace minikin {

// Code point order
constexpr uint32_t CHAR_LINE_FEED = 0x000A;
constexpr uint32_t CHAR_CARRIAGE_RETURN = 0x000D;
constexpr uint32_t CHAR_TAB = 0x0009;
constexpr uint32_t CHAR_HYPHEN_MINUS = 0x002D;
constexpr uint32_t CHAR_NBSP = 0x00A0;
constexpr uint32_t CHAR_SOFT_HYPHEN = 0x00AD;
constexpr uint32_t CHAR_MIDDLE_DOT = 0x00B7;
constexpr uint32_t CHAR_ARMENIAN_HYPHEN = 0x058A;
constexpr uint32_t CHAR_MAQAF = 0x05BE;
constexpr uint32_t CHAR_UCAS_HYPHEN = 0x1400;
constexpr uint32_t CHAR_ZWJ = 0x200D;
constexpr uint32_t CHAR_HYPHEN = 0x2010;

}  // namespace minikin

#endif  // MINIKIN_CHARACTERS_H
