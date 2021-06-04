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

//#define LOG_NDEBUG 0
#define LOG_TAG "ClearKeyCryptoPlugin"
#include <utils/Log.h>

#include <openssl/aes.h>

#include "AesCtrDecryptor.h"

namespace clearkeydrm {

static const size_t kBlockBitCount = kBlockSize * 8;

android::status_t AesCtrDecryptor::decrypt(const android::Vector<uint8_t>& key,
        const Iv iv, const uint8_t* source,
        uint8_t* destination,
        const SubSample* subSamples,
        size_t numSubSamples,
        size_t* bytesDecryptedOut) {
    uint32_t blockOffset = 0;
    uint8_t previousEncryptedCounter[kBlockSize];
    memset(previousEncryptedCounter, 0, kBlockSize);

    if (key.size() != kBlockSize || (sizeof(Iv) / sizeof(uint8_t)) != kBlockSize) {
        android_errorWriteLog(0x534e4554, "63982768");
        return android::ERROR_DRM_DECRYPT;
    }

    size_t offset = 0;
    AES_KEY opensslKey;
    AES_set_encrypt_key(key.array(), kBlockBitCount, &opensslKey);
    Iv opensslIv;
    memcpy(opensslIv, iv, sizeof(opensslIv));

    for (size_t i = 0; i < numSubSamples; ++i) {
        const SubSample& subSample = subSamples[i];

        if (subSample.mNumBytesOfClearData > 0) {
            memcpy(destination + offset, source + offset,
                    subSample.mNumBytesOfClearData);
            offset += subSample.mNumBytesOfClearData;
        }

        if (subSample.mNumBytesOfEncryptedData > 0) {
            AES_ctr128_encrypt(source + offset, destination + offset,
                    subSample.mNumBytesOfEncryptedData, &opensslKey,
                    opensslIv, previousEncryptedCounter,
                    &blockOffset);
            offset += subSample.mNumBytesOfEncryptedData;
        }
    }

    *bytesDecryptedOut = offset;
    return android::OK;
}

} // namespace clearkeydrm
