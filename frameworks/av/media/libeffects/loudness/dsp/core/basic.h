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

#ifndef LE_FX_ENGINE_DSP_CORE_BASIC_H_
#define LE_FX_ENGINE_DSP_CORE_BASIC_H_

#include <limits.h>
#include "common/core/math.h"
#include "common/core/types.h"

namespace le_fx {

namespace sigmod {

// Searchs for the interval that contains <x> using a divide-and-conquer
// algorithm.
// X[]: a vector of sorted values (X[i+1] > X[i])
// x:   a value
// StartIndex: the minimum searched index
// EndIndex: the maximum searched index
// returns: the index <i> that satisfies: X[i] <= x <= X[i+1] &&
//          StartIndex <= i <= (EndIndex-1)
template <typename T>
int SearchIndex(const T x_data[],
                T x,
                int start_index,
                int end_index);

}  // namespace sigmod

}  // namespace le_fx

#include "dsp/core/basic-inl.h"

#endif  // LE_FX_ENGINE_DSP_CORE_BASIC_H_
