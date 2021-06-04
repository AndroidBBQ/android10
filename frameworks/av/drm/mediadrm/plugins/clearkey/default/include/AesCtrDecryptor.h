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

#ifndef CLEARKEY_AES_CTR_DECRYPTOR_H_
#define CLEARKEY_AES_CTR_DECRYPTOR_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/MediaErrors.h>
#include <Utils.h>
#include <utils/Errors.h>
#include <utils/Vector.h>

#include "ClearKeyTypes.h"

namespace clearkeydrm {

class AesCtrDecryptor {
public:
    AesCtrDecryptor() {}

    android::status_t decrypt(const android::Vector<uint8_t>& key, const Iv iv,
            const uint8_t* source, uint8_t* destination,
            const SubSample* subSamples, size_t numSubSamples,
            size_t* bytesDecryptedOut);

private:
    DISALLOW_EVIL_CONSTRUCTORS(AesCtrDecryptor);
};

} // namespace clearkeydrm

#endif // CLEARKEY_AES_CTR_DECRYPTOR_H_
