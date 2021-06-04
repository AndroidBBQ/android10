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

//#define LOG_NDEBUG 0
#define LOG_TAG "hidl_ClearkeyDecryptor"
#include <utils/Log.h>

#include <openssl/aes.h>

#include "AesCtrDecryptor.h"
#include "ClearKeyTypes.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

using ::android::hardware::drm::V1_0::SubSample;
using ::android::hardware::drm::V1_0::Status;

static const size_t kBlockBitCount = kBlockSize * 8;

Status AesCtrDecryptor::decrypt(
        const std::vector<uint8_t>& key,
        const Iv iv, const uint8_t* source,
        uint8_t* destination,
        const std::vector<SubSample> subSamples,
        size_t numSubSamples,
        size_t* bytesDecryptedOut) {
    uint32_t blockOffset = 0;
    uint8_t previousEncryptedCounter[kBlockSize];
    memset(previousEncryptedCounter, 0, kBlockSize);

    if (key.size() != kBlockSize || (sizeof(Iv) / sizeof(uint8_t)) != kBlockSize) {
        android_errorWriteLog(0x534e4554, "63982768");
        return Status::ERROR_DRM_DECRYPT;
    }

    size_t offset = 0;
    AES_KEY opensslKey;
    AES_set_encrypt_key(key.data(), kBlockBitCount, &opensslKey);
    Iv opensslIv;
    memcpy(opensslIv, iv, sizeof(opensslIv));

    for (size_t i = 0; i < numSubSamples; ++i) {
        const SubSample& subSample = subSamples[i];

        if (subSample.numBytesOfClearData > 0) {
            memcpy(destination + offset, source + offset,
                    subSample.numBytesOfClearData);
            offset += subSample.numBytesOfClearData;
        }

        if (subSample.numBytesOfEncryptedData > 0) {
            AES_ctr128_encrypt(source + offset, destination + offset,
                    subSample.numBytesOfEncryptedData, &opensslKey,
                    opensslIv, previousEncryptedCounter,
                    &blockOffset);
            offset += subSample.numBytesOfEncryptedData;
        }
    }

    *bytesDecryptedOut = offset;
    return Status::OK;
}

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

