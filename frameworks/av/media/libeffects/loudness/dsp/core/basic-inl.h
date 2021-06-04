/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef LE_FX_ENGINE_DSP_CORE_BASIC_INL_H_
#define LE_FX_ENGINE_DSP_CORE_BASIC_INL_H_

#include <math.h>

namespace le_fx {

namespace sigmod {

template <typename T>
int SearchIndex(const T x_data[],
                T x,
                int start_index,
                int end_index) {
  int start = start_index;
  int end = end_index;
  while (end > start + 1) {
    int i = (end + start) / 2;
    if (x_data[i] > x) {
      end = i;
    } else {
      start = i;
    }
  }
  return start;
}

}  // namespace sigmod

}  // namespace le_fx

#endif  // LE_FX_ENGINE_DSP_CORE_BASIC_INL_H_
