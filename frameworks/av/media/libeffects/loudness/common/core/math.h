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

#ifndef LE_FX_ENGINE_COMMON_CORE_MATH_H_
#define LE_FX_ENGINE_COMMON_CORE_MATH_H_

#include <math.h>
#include <algorithm>
using ::std::min;
using ::std::max;
using ::std::fill;
using ::std::fill_n;using ::std::lower_bound;
#include <cmath>
#include <math.h>
//using ::std::fpclassify;

#include "common/core/os.h"
#include "common/core/types.h"

namespace le_fx {
namespace math {

// A fast approximation to log2(.)
inline float fast_log2(float val) {
  int* const exp_ptr = reinterpret_cast <int *> (&val);
  int x = *exp_ptr;
  const int log_2 = ((x >> 23) & 255) - 128;
  x &= ~(255 << 23);
  x += 127 << 23;
  *exp_ptr = x;
  val = ((-1.0f / 3) * val + 2) * val - 2.0f / 3;
  return static_cast<float>(val + log_2);
}

// A fast approximation to log(.)
inline float fast_log(float val) {
  return fast_log2(val) *
      0.693147180559945286226763982995180413126945495605468750f;
}

// An approximation of the exp(.) function using a 5-th order Taylor expansion.
// It's pretty accurate between +-0.1 and accurate to 10e-3 between +-1
template <typename T>
inline T ExpApproximationViaTaylorExpansionOrder5(T x) {
  const T x2 = x * x;
  const T x3 = x2 * x;
  const T x4 = x2 * x2;
  const T x5 = x3 * x2;
  return 1.0f + x + 0.5f * x2 +
      0.16666666666666665741480812812369549646973609924316406250f * x3 +
      0.0416666666666666643537020320309238741174340248107910156250f * x4 +
      0.008333333333333333217685101601546193705871701240539550781250f * x5;
}

}  // namespace math
}  // namespace le_fx

// Math functions missing in Android NDK:
#if defined(LE_FX_OS_ANDROID)

namespace std {

//
// Round to the nearest integer: We need this implementation
// since std::round is missing on android.
//
template <typename T>
inline T round(const T &x) {
  return static_cast<T>(std::floor(static_cast<double>(x) + 0.5));
}

}  // namespace std

#endif  // LE_FX_OS_ANDROID

#endif  // LE_FX_ENGINE_COMMON_CORE_MATH_H_
