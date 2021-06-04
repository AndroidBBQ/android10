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

#ifndef CLEARKEY_MACROS_H_
#define CLEARKEY_MACROS_H_

#include <android/hardware/drm/1.2/types.h>

#include <map>

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

using ::android::hardware::drm::V1_0::KeyValue;
using ::android::hardware::drm::V1_1::SecurityLevel;
using ::android::hardware::hidl_vec;

const uint8_t kBlockSize = 16; //AES_BLOCK_SIZE;
typedef uint8_t KeyId[kBlockSize];
typedef uint8_t Iv[kBlockSize];

typedef ::android::hardware::drm::V1_0::SubSample SubSample;
typedef std::map<std::vector<uint8_t>, std::vector<uint8_t> > KeyMap;

#define CLEARKEY_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete;

#define CLEARKEY_DISALLOW_COPY_AND_ASSIGN_AND_NEW(TypeName) \
  TypeName() = delete;                     \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete;

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

#endif // CLEARKEY_MACROS_H_
