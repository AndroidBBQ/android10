/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef CLEARKEY_SESSION_LIBRARY_H_
#define CLEARKEY_SESSION_LIBRARY_H_

#include <media/cas/CasAPI.h>
#include <media/cas/DescramblerAPI.h>
#include <openssl/aes.h>
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

namespace android {
struct ABuffer;

namespace clearkeycas {
class KeyFetcher;

class ClearKeyCasSession : public RefBase {
public:
    explicit ClearKeyCasSession(CasPlugin *plugin);

    virtual ~ClearKeyCasSession();

    ssize_t decrypt(
            bool secure,
            DescramblerPlugin::ScramblingControl scramblingControl,
            size_t numSubSamples,
            const DescramblerPlugin::SubSample *subSamples,
            const void *srcPtr,
            void *dstPtr,
            AString * /* errorDetailMsg */);

    status_t updateECM(KeyFetcher *keyFetcher, void *ecm, size_t size);

private:
    enum {
        kNumKeys = 2,
    };
    struct KeyInfo {
        bool valid;
        AES_KEY contentKey;
    };
    sp<ABuffer> mEcmBuffer;
    Mutex mKeyLock;
    CasPlugin* mPlugin;
    KeyInfo mKeyInfo[kNumKeys];

    friend class ClearKeySessionLibrary;

    CasPlugin* getPlugin() const { return mPlugin; }
    status_t decryptPayload(
            const AES_KEY& key, size_t length, size_t offset, char* buffer) const;

    DISALLOW_EVIL_CONSTRUCTORS(ClearKeyCasSession);
};

class ClearKeySessionLibrary {
public:
    static ClearKeySessionLibrary* get();

    status_t addSession(CasPlugin *plugin, CasSessionId *sessionId);

    std::shared_ptr<ClearKeyCasSession> findSession(const CasSessionId& sessionId);

    void destroySession(const CasSessionId& sessionId);

    void destroyPlugin(CasPlugin *plugin);

private:
    static Mutex sSingletonLock;
    static ClearKeySessionLibrary* sSingleton;

    Mutex mSessionsLock;
    uint32_t mNextSessionId;
    KeyedVector<CasSessionId, std::shared_ptr<ClearKeyCasSession>> mIDToSessionMap;

    ClearKeySessionLibrary();
    DISALLOW_EVIL_CONSTRUCTORS(ClearKeySessionLibrary);
};

} // namespace clearkeycas
} // namespace android

#endif // CLEARKEY_SESSION_LIBRARY_H_
