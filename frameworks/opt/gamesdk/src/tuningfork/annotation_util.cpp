/*
 * Copyright 2019 The Android Open Source Project
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

#include "annotation_util.h"

#include <cstdlib>

#define LOG_TAG "TuningFork"
#include "Log.h"

namespace annotation_util {

typedef uint64_t AnnotationId;

// This is a protobuf 1-based index
int GetKeyIndex(uint8_t b) {
    int type = b & 0x7;
    if (type != 0) return kKeyError;
    return b >> 3;
}

uint64_t GetBase128IntegerFromByteStream(const std::vector<uint8_t> &bytes, int &index) {
    uint64_t m = 0;
    uint64_t r = 0;
    while (index < bytes.size() && m <= (64 - 7)) {
        auto b = bytes[index];
        r |= (((uint64_t) b) & 0x7f) << m;
        if ((b & 0x80) != 0) m += 7;
        else return r;
        ++index;
    }
    return kStreamError;
}

void WriteBase128IntToStream(uint64_t x, std::vector<uint8_t> &bytes) {
    do {
        uint8_t a = x & 0x7f;
        int b = x & 0xffffffffffffff80;
        if (b) {
            bytes.push_back(a | 0x80);
            x >>= 7;
        } else {
            bytes.push_back(a);
            return;
        }
    } while(x);
}

AnnotationId DecodeAnnotationSerialization(const SerializedAnnotation &ser,
                                           const std::vector<int>& radix_mult) {
    AnnotationId result = 0;
    for (int i = 0; i < ser.size(); ++i) {
        int key = GetKeyIndex(ser[i]);
        if (key == kKeyError)
            return kAnnotationError;
        // Convert to 0-based index
        --key;
        if (key >= radix_mult.size())
            return kAnnotationError;
        ++i;
        if (i >= ser.size())
            return kAnnotationError;
        uint64_t value = GetBase128IntegerFromByteStream(ser, i);
        if (value == kStreamError)
            return kAnnotationError;
        // Check the range of the value
        if (value == 0 || value >= radix_mult[key])
            return kAnnotationError;
        // We don't allow enums with more that 255 values
        if (value > 0xff)
            return kAnnotationError;
        if (key > 0)
            result += radix_mult[key - 1] * value;
        else
            result += value;
    }
    return result;
}

int SerializeAnnotationId(uint64_t id, SerializedAnnotation& ser,
                          const std::vector<int>& radix_mult) {
  int err = 0;
  uint64_t x = id;
  for (int i = 0; i < radix_mult.size(); ++i) {
    auto r = ::div(x, radix_mult[i]);
    int value = r.rem;
    if (value > 0) {
      int key = (i + 1) << 3;
      ser.push_back(key);
      WriteBase128IntToStream(value, ser);
    }
    x = r.quot;
  }
  return err;
}

void SetUpAnnotationRadixes( std::vector<int>& radix_mult,
                             const std::vector<int>& enum_sizes) {
    ALOGV("Settings::annotation_enum_size");
    for(int i=0; i< enum_sizes.size();++i) {
      ALOGV("%d", enum_sizes[i]);
    }
    int n = enum_sizes.size();
    if (n == 0) {
        // With no annotations, we just have 1 possible prong per key
        radix_mult.resize(1);
        radix_mult[0] = 1;
    } else {
        radix_mult.resize(n);
        int r = 1;
        for (int i = 0; i < n; ++i) {
            r *= enum_sizes[i] + 1;
            radix_mult[i] = r;
        }
    }
}

} // namespace annotation_util
