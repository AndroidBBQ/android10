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
#define LOG_TAG "ClearKeySession"
#include <utils/Log.h>

#include <media/stagefright/MediaErrors.h>
#include <utils/String8.h>

#include "Session.h"

#include "AesCtrDecryptor.h"
#include "InitDataParser.h"
#include "JsonWebKey.h"

namespace clearkeydrm {

using android::Mutex;
using android::String8;
using android::Vector;
using android::status_t;

status_t Session::getKeyRequest(
        const Vector<uint8_t>& initData,
        const String8& mimeType,
        Vector<uint8_t>* keyRequest) const {
    InitDataParser parser;
    return parser.parse(initData, mimeType, keyRequest);
}

status_t Session::provideKeyResponse(const Vector<uint8_t>& response) {
    String8 responseString(
            reinterpret_cast<const char*>(response.array()), response.size());
    KeyMap keys;

    Mutex::Autolock lock(mMapLock);
    JsonWebKey parser;
    if (parser.extractKeysFromJsonWebKeySet(responseString, &keys)) {
        for (size_t i = 0; i < keys.size(); ++i) {
            const KeyMap::key_type& keyId = keys.keyAt(i);
            const KeyMap::value_type& key = keys.valueAt(i);
            mKeyMap.add(keyId, key);
        }
        return android::OK;
    } else {
        return android::ERROR_DRM_UNKNOWN;
    }
}

status_t Session::decrypt(
        const KeyId keyId, const Iv iv, const void* source,
        void* destination, const SubSample* subSamples,
        size_t numSubSamples, size_t* bytesDecryptedOut) {
    Mutex::Autolock lock(mMapLock);

    Vector<uint8_t> keyIdVector;
    keyIdVector.appendArray(keyId, kBlockSize);
    if (mKeyMap.indexOfKey(keyIdVector) < 0) {
        return android::ERROR_DRM_NO_LICENSE;
    }

    const Vector<uint8_t>& key = mKeyMap.valueFor(keyIdVector);
    AesCtrDecryptor decryptor;
    return decryptor.decrypt(
            key, iv,
            reinterpret_cast<const uint8_t*>(source),
            reinterpret_cast<uint8_t*>(destination), subSamples,
            numSubSamples, bytesDecryptedOut);
}

} // namespace clearkeydrm
