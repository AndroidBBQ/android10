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

#ifndef LE_FX_ENGINE_COMMON_CORE_BYTE_SWAPPER_H_
#define LE_FX_ENGINE_COMMON_CORE_BYTE_SWAPPER_H_

#include <stdio.h>
#include <string.h>

#include "common/core/basic_types.h"
#include "common/core/os.h"

namespace le_fx {

namespace arch {

inline bool IsLittleEndian(void) {
  int16 word = 1;
  char *cp = reinterpret_cast<char *>(&word);
  return cp[0] != 0;
}

inline bool IsBigEndian(void) {
  return !IsLittleEndian();
}

template <typename T, unsigned int kValSize>
struct ByteSwapper {
  static T Swap(const T &val) {
    T new_val = val;
    char *first = &new_val, *last = first + kValSize - 1, x;
    for (; first < last; ++first, --last) {
      x = *last;
      *last = *first;
      *first = x;
    }
    return new_val;
  }
};

template <typename T>
struct ByteSwapper<T, 1> {
  static T Swap(const T &val) {
    return val;
  }
};

template <typename T>
struct ByteSwapper<T, 2> {
  static T Swap(const T &val) {
    T new_val;
    const char *o = (const char *)&val;
    char *p = reinterpret_cast<char *>(&new_val);
    p[0] = o[1];
    p[1] = o[0];
    return new_val;
  }
};

template <typename T>
struct ByteSwapper<T, 4> {
  static T Swap(const T &val) {
    T new_val;
    const char *o = (const char *)&val;
    char *p = reinterpret_cast<char *>(&new_val);
    p[0] = o[3];
    p[1] = o[2];
    p[2] = o[1];
    p[3] = o[0];
    return new_val;
  }
};

template <typename T>
struct ByteSwapper<T, 8> {
  static T Swap(const T &val) {
    T new_val = val;
    const char *o = (const char *)&val;
    char *p = reinterpret_cast<char *>(&new_val);
    p[0] = o[7];
    p[1] = o[6];
    p[2] = o[5];
    p[3] = o[4];
    p[4] = o[3];
    p[5] = o[2];
    p[6] = o[1];
    p[7] = o[0];
    return new_val;
  }
};

template <typename T>
T SwapBytes(const T &val, bool force_swap) {
  if (force_swap) {
#if !defined(LE_FX__NEED_BYTESWAP)
    return ByteSwapper<T, sizeof(T)>::Swap(val);
#else
    return val;
#endif  // !LE_FX_NEED_BYTESWAP
  } else {
#if !defined(LE_FX_NEED_BYTESWAP)
    return val;
#else
    return ByteSwapper<T, sizeof(T)>::Swap(val);
#endif  // !LE_FX_NEED_BYTESWAP
  }
}

template <typename T>
const T *SwapBytes(const T *vals, unsigned int num_items, bool force_swap) {
  if (force_swap) {
#if !defined(LE_FX_NEED_BYTESWAP)
    T *writeable_vals = const_cast<T *>(vals);
    for (unsigned int i = 0; i < num_items; i++) {
      writeable_vals[i] = ByteSwapper<T, sizeof(T)>::Swap(vals[i]);
    }
    return writeable_vals;
#else
    return vals;
#endif  // !LE_FX_NEED_BYTESWAP
  } else {
#if !defined(LE_FX_NEED_BYTESWAP)
    return vals;
#else
    T *writeable_vals = const_cast<T *>(vals);
    for (unsigned int i = 0; i < num_items; i++) {
      writeable_vals[i] = ByteSwapper<T, sizeof(T)>::Swap(vals[i]);
    }
    return writeable_vals;
#endif  // !LE_FX_NEED_BYTESWAP
  }
}

}  // namespace arch

}  // namespace le_fx

#endif  // LE_FX_ENGINE_COMMON_CORE_BYTE_SWAPPER_H_
