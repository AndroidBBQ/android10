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

#include <media/stagefright/MediaErrors.h>

#include "CryptoPlugin.h"
#include "SessionLibrary.h"

namespace clearkeydrm {

using android::Vector;
using android::AString;
using android::status_t;

// Returns negative values for error code and positive values for the size of
// decrypted data.  In theory, the output size can be larger than the input
// size, but in practice this will never happen for AES-CTR.
ssize_t CryptoPlugin::decrypt(bool secure, const KeyId keyId, const Iv iv,
                              Mode mode, const Pattern &/* pattern */, const void* srcPtr,
                              const SubSample* subSamples, size_t numSubSamples,
                              void* dstPtr, AString* errorDetailMsg) {
    if (secure) {
        errorDetailMsg->setTo("Secure decryption is not supported with "
                              "ClearKey.");
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    if (mode == kMode_Unencrypted) {
        size_t offset = 0;
        for (size_t i = 0; i < numSubSamples; ++i) {
            const SubSample& subSample = subSamples[i];

            if (subSample.mNumBytesOfEncryptedData != 0) {
                errorDetailMsg->setTo(
                        "Encrypted subsamples found in allegedly unencrypted "
                        "data.");
                return android::ERROR_DRM_DECRYPT;
            }

            if (subSample.mNumBytesOfClearData != 0) {
                memcpy(reinterpret_cast<uint8_t*>(dstPtr) + offset,
                       reinterpret_cast<const uint8_t*>(srcPtr) + offset,
                       subSample.mNumBytesOfClearData);
                offset += subSample.mNumBytesOfClearData;
            }
        }
        return static_cast<ssize_t>(offset);
    } else if (mode == kMode_AES_CTR) {
        size_t bytesDecrypted;
        status_t res = mSession->decrypt(keyId, iv, srcPtr, dstPtr, subSamples,
                                         numSubSamples, &bytesDecrypted);
        if (res == android::OK) {
            return static_cast<ssize_t>(bytesDecrypted);
        } else {
            errorDetailMsg->setTo("Decryption Error");
            return static_cast<ssize_t>(res);
        }
    } else {
        errorDetailMsg->setTo(
                "Selected encryption mode is not supported by the ClearKey DRM "
                "Plugin.");
        return android::ERROR_DRM_CANNOT_HANDLE;
    }
}

android::status_t CryptoPlugin::setMediaDrmSession(
        const android::Vector<uint8_t>& sessionId) {
    if (!sessionId.size()) {
        mSession.clear();
    } else {
        mSession = SessionLibrary::get()->findSession(sessionId);
        if (!mSession.get()) {
            return android::ERROR_DRM_SESSION_NOT_OPENED;
        }
    }
    return android::OK;
}


}  // namespace clearkeydrm
