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

#ifndef CLEARKEY_AES_CTR_DECRYPTOR_H_
#define CLEARKEY_AES_CTR_DECRYPTOR_H_

#include "ClearKeyTypes.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

using ::android::hardware::drm::V1_0::Status;
using ::android::hardware::drm::V1_0::SubSample;

class AesCtrDecryptor {
public:
    AesCtrDecryptor() {}

    Status decrypt(const std::vector<uint8_t>& key, const Iv iv,
            const uint8_t* source, uint8_t* destination,
            const std::vector<SubSample> subSamples, size_t numSubSamples,
            size_t* bytesDecryptedOut);

private:
    CLEARKEY_DISALLOW_COPY_AND_ASSIGN(AesCtrDecryptor);
};

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

#endif // CLEARKEY_AES_CTR_DECRYPTOR_H_
