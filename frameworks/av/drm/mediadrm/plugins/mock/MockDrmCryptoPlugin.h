/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <utils/Mutex.h>

#include "media/drm/DrmAPI.h"
#include "media/hardware/CryptoAPI.h"

extern "C" {
      android::DrmFactory *createDrmFactory();
      android::CryptoFactory *createCryptoFactory();
}

namespace android {

    class MockDrmFactory : public DrmFactory {
    public:
        MockDrmFactory() {}
        virtual ~MockDrmFactory() {}

        bool isCryptoSchemeSupported(const uint8_t uuid[16]);
        bool isContentTypeSupported(const String8 &mimeType);
        status_t createDrmPlugin(const uint8_t uuid[16],
                                 DrmPlugin **plugin);
    };

    class MockCryptoFactory : public CryptoFactory {
    public:
        MockCryptoFactory() {}
        virtual ~MockCryptoFactory() {}

        bool isCryptoSchemeSupported(const uint8_t uuid[16]) const;
        status_t createPlugin(
            const uint8_t uuid[16], const void *data, size_t size,
            CryptoPlugin **plugin);
    };



    class MockDrmPlugin : public DrmPlugin {
    public:
        MockDrmPlugin() {}
        virtual ~MockDrmPlugin() {}

        // from DrmPlugin
        status_t openSession(Vector<uint8_t> &sessionId);
        status_t closeSession(Vector<uint8_t> const &sessionId);

        status_t getKeyRequest(Vector<uint8_t> const &sessionId,
                               Vector<uint8_t> const &initData,
                               String8 const &mimeType, KeyType keyType,
                               KeyedVector<String8, String8> const &optionalParameters,
                               Vector<uint8_t> &request, String8 &defaultUrl,
                               KeyRequestType *keyRequestType);

        status_t provideKeyResponse(Vector<uint8_t> const &sessionId,
                                    Vector<uint8_t> const &response,
                                    Vector<uint8_t> &keySetId);

        status_t removeKeys(Vector<uint8_t> const &keySetId);

        status_t restoreKeys(Vector<uint8_t> const &sessionId,
                             Vector<uint8_t> const &keySetId);

        status_t queryKeyStatus(Vector<uint8_t> const &sessionId,
                                KeyedVector<String8, String8> &infoMap) const;

        status_t getProvisionRequest(String8 const &certType,
                                     String8 const &certAuthority,
                                     Vector<uint8_t> &request,
                                     String8 &defaultUrl);

        status_t provideProvisionResponse(Vector<uint8_t> const &response,
                                          Vector<uint8_t> &certificate,
                                          Vector<uint8_t> &wrappedKey);

        status_t getSecureStops(List<Vector<uint8_t> > &secureStops);
        status_t getSecureStop(Vector<uint8_t> const &ssid, Vector<uint8_t> &secureStop);
        status_t releaseSecureStops(Vector<uint8_t> const &ssRelease);
        status_t releaseAllSecureStops();

        status_t getPropertyString(String8 const &name, String8 &value ) const;
        status_t getPropertyByteArray(String8 const &name,
                                              Vector<uint8_t> &value ) const;

        status_t setPropertyString(String8 const &name,
                                   String8 const &value );
        status_t setPropertyByteArray(String8 const &name,
                                      Vector<uint8_t> const &value );

        status_t setCipherAlgorithm(Vector<uint8_t> const &sessionId,
                                    String8 const &algorithm);

        status_t setMacAlgorithm(Vector<uint8_t> const &sessionId,
                                 String8 const &algorithm);

        status_t encrypt(Vector<uint8_t> const &sessionId,
                         Vector<uint8_t> const &keyId,
                         Vector<uint8_t> const &input,
                         Vector<uint8_t> const &iv,
                         Vector<uint8_t> &output);

        status_t decrypt(Vector<uint8_t> const &sessionId,
                         Vector<uint8_t> const &keyId,
                         Vector<uint8_t> const &input,
                         Vector<uint8_t> const &iv,
                         Vector<uint8_t> &output);

        status_t sign(Vector<uint8_t> const &sessionId,
                      Vector<uint8_t> const &keyId,
                      Vector<uint8_t> const &message,
                      Vector<uint8_t> &signature);

        status_t verify(Vector<uint8_t> const &sessionId,
                        Vector<uint8_t> const &keyId,
                        Vector<uint8_t> const &message,
                        Vector<uint8_t> const &signature,
                        bool &match);

        status_t signRSA(Vector<uint8_t> const &sessionId,
                         String8 const &algorithm,
                         Vector<uint8_t> const &message,
                         Vector<uint8_t> const &wrappedKey,
                         Vector<uint8_t> &signature);

    private:
        String8 vectorToString(Vector<uint8_t> const &vector) const;
        String8 arrayToString(uint8_t const *array, size_t len) const;
        String8 stringMapToString(const KeyedVector<String8, String8>& map) const;

        SortedVector<Vector<uint8_t> > mSessions;
        SortedVector<Vector<uint8_t> > mKeySets;

        static const ssize_t kNotFound = -1;
        ssize_t findSession(Vector<uint8_t> const &sessionId) const;
        ssize_t findKeySet(Vector<uint8_t> const &keySetId) const;

        Mutex mLock;
        KeyedVector<String8, String8> mStringProperties;
        KeyedVector<String8, Vector<uint8_t> > mByteArrayProperties;
    };


    class MockCryptoPlugin : public CryptoPlugin {

        bool requiresSecureDecoderComponent(const char *mime) const;

        static constexpr size_t DECRYPT_KEY_SIZE = 16;

        ssize_t decrypt(bool secure,
            const uint8_t key[DECRYPT_KEY_SIZE],
            const uint8_t iv[DECRYPT_KEY_SIZE], Mode mode,
            const Pattern &pattern, const void *srcPtr,
            const SubSample *subSamples, size_t numSubSamples,
            void *dstPtr, AString *errorDetailMsg);
    private:
        String8 subSamplesToString(CryptoPlugin::SubSample const *subSamples, size_t numSubSamples) const;
        String8 arrayToString(uint8_t const *array, size_t len) const;
    };
};
