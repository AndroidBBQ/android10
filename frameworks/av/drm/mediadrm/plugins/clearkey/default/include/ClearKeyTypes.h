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

#ifndef CLEARKEY_TYPES_H_
#define CLEARKEY_TYPES_H_

#include <media/hardware/CryptoAPI.h>
#include <openssl/aes.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>

namespace clearkeydrm {

const uint8_t kBlockSize = AES_BLOCK_SIZE;
typedef uint8_t KeyId[kBlockSize];
typedef uint8_t Iv[kBlockSize];

typedef android::CryptoPlugin::SubSample SubSample;

typedef android::KeyedVector<android::Vector<uint8_t>,
        android::Vector<uint8_t> > KeyMap;

} // namespace clearkeydrm

#endif // CLEARKEY_TYPES_H_
