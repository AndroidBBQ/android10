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

#ifndef LE_FX_ENGINE_COMMON_CORE_BASIC_TYPES_H_
#define LE_FX_ENGINE_COMMON_CORE_BASIC_TYPES_H_

#include <stddef.h>
#include <stdlib.h>
#include <string>
using ::std::string;
using ::std::basic_string;
#include <vector>
using ::std::vector;

#include "common/core/os.h"

// -----------------------------------------------------------------------------
// Definitions of common basic types:
// -----------------------------------------------------------------------------

#if !defined(G_COMPILE) && !defined(BASE_INTEGRAL_TYPES_H_)

namespace le_fx {

typedef signed char         schar;
typedef signed char         int8;
typedef short               int16;
typedef int                 int32;
typedef long long           int64;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

}  // namespace le_fx

#endif

namespace le_fx {

struct FloatArray {
  int length;
  float *data;

  FloatArray(void) {
    data = NULL;
    length = 0;
  }
};

struct Int16Array {
  int length;
  int16 *data;

  Int16Array(void) {
    data = NULL;
    length = 0;
  }
};

struct Int32Array {
  int length;
  int32 *data;

  Int32Array(void) {
    data = NULL;
    length = 0;
  }
};

struct Int8Array {
  int length;
  uint8 *data;

  Int8Array(void) {
    data = NULL;
    length = 0;
  }
};

//
// Simple wrapper for waveform data:
//
class WaveData : public vector<int16> {
 public:
  WaveData();
  ~WaveData();

  void Set(int number_samples, int sampling_rate, int16 *data);
  int sample_rate(void) const;
  void set_sample_rate(int sample_rate);
  bool Equals(const WaveData &wave_data, int threshold = 0) const;

 private:
  int sample_rate_;
};

}  // namespace le_fx

#endif  // LE_FX_ENGINE_COMMON_CORE_BASIC_TYPES_H_
