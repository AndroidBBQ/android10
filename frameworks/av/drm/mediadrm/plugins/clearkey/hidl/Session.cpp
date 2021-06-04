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
#define LOG_TAG "hidl_ClearKeySession"
#include <utils/Log.h>

#include "Session.h"
#include "Utils.h"

#include "AesCtrDecryptor.h"
#include "InitDataParser.h"
#include "JsonWebKey.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

using ::android::hardware::drm::V1_0::KeyValue;
using ::android::hardware::drm::V1_0::Status;
using ::android::hardware::drm::V1_0::SubSample;
using ::android::hardware::Return;
using ::android::sp;

using android::Mutex;

Status Session::getKeyRequest(
        const std::vector<uint8_t>& initData,
        const std::string& mimeType,
        V1_0::KeyType keyType,
        std::vector<uint8_t>* keyRequest) const {
    InitDataParser parser;
    return parser.parse(initData, mimeType, keyType, keyRequest);
}

Status Session::provideKeyResponse(const std::vector<uint8_t>& response) {
    std::string responseString(
            reinterpret_cast<const char*>(response.data()), response.size());
    KeyMap keys;

    Mutex::Autolock lock(mMapLock);
    JsonWebKey parser;
    if (parser.extractKeysFromJsonWebKeySet(responseString, &keys)) {
        for (auto &key : keys) {
            std::string first(key.first.begin(), key.first.end());
            std::string second(key.second.begin(), key.second.end());
            mKeyMap.insert(std::pair<std::vector<uint8_t>,
                    std::vector<uint8_t> >(key.first, key.second));
        }
        return Status::OK;
    } else {
        return Status::ERROR_DRM_UNKNOWN;
    }
}

Status_V1_2 Session::decrypt(
        const KeyId keyId, const Iv iv, const uint8_t* srcPtr,
        uint8_t* destPtr, const std::vector<SubSample> subSamples,
        size_t* bytesDecryptedOut) {
    Mutex::Autolock lock(mMapLock);

    if (getMockError() != Status_V1_2::OK) {
        return getMockError();
    }

    std::vector<uint8_t> keyIdVector;
    keyIdVector.clear();
    keyIdVector.insert(keyIdVector.end(), keyId, keyId + kBlockSize);
    std::map<std::vector<uint8_t>, std::vector<uint8_t> >::iterator itr;
    itr = mKeyMap.find(keyIdVector);
    if (itr == mKeyMap.end()) {
        return Status_V1_2::ERROR_DRM_NO_LICENSE;
    }

    AesCtrDecryptor decryptor;
    Status status = decryptor.decrypt(
            itr->second /*key*/, iv, srcPtr, destPtr, subSamples,
            subSamples.size(), bytesDecryptedOut);
    return static_cast<Status_V1_2>(status);
}

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android
