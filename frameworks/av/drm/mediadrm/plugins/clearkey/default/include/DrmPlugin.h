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

#ifndef CLEARKEY_DRM_PLUGIN_H_
#define CLEARKEY_DRM_PLUGIN_H_

#include <media/drm/DrmAPI.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/List.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include "SessionLibrary.h"
#include "Utils.h"

namespace clearkeydrm {

using android::KeyedVector;
using android::List;
using android::status_t;
using android::String8;
using android::Vector;

class DrmPlugin : public android::DrmPlugin {
public:
    explicit DrmPlugin(SessionLibrary* sessionLibrary);

    virtual ~DrmPlugin() {}

    virtual status_t openSession(Vector<uint8_t>& sessionId);

    virtual status_t closeSession(const Vector<uint8_t>& sessionId);

    virtual status_t getKeyRequest(
            const Vector<uint8_t>& scope,
            const Vector<uint8_t>& mimeType,
            const String8& initDataType,
            KeyType keyType,
            const KeyedVector<String8, String8>& optionalParameters,
            Vector<uint8_t>& request,
            String8& defaultUrl,
            DrmPlugin::KeyRequestType *keyRequestType);

    virtual status_t provideKeyResponse(
            const Vector<uint8_t>& scope,
            const Vector<uint8_t>& response,
            Vector<uint8_t>& keySetId);

    virtual status_t removeKeys(const Vector<uint8_t>& sessionId) {
        if (sessionId.size() == 0) {
            return android::BAD_VALUE;
        }

        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t restoreKeys(
            const Vector<uint8_t>& sessionId,
            const Vector<uint8_t>& keySetId) {
        if (sessionId.size() == 0 || keySetId.size() == 0) {
            return android::BAD_VALUE;
        }
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t queryKeyStatus(
            const Vector<uint8_t>& sessionId,
            KeyedVector<String8, String8>& infoMap) const;

    virtual status_t getProvisionRequest(
            const String8& cert_type,
            const String8& cert_authority,
            Vector<uint8_t>& request,
            String8& defaultUrl) {
        UNUSED(cert_type);
        UNUSED(cert_authority);
        UNUSED(request);
        UNUSED(defaultUrl);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t provideProvisionResponse(
            const Vector<uint8_t>& response,
            Vector<uint8_t>& certificate,
            Vector<uint8_t>& wrappedKey) {
        UNUSED(certificate);
        UNUSED(wrappedKey);
        if (response.size() == 0) {
            // empty response
            return android::BAD_VALUE;
        }
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t getSecureStops(List<Vector<uint8_t> >& secureStops) {
        UNUSED(secureStops);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t getSecureStop(Vector<uint8_t> const &ssid, Vector<uint8_t> &secureStop) {
        if (ssid.size() == 0) {
            return android::BAD_VALUE;
        }

        UNUSED(secureStop);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t releaseSecureStops(const Vector<uint8_t>& ssRelease) {
        if (ssRelease.size() == 0) {
            return android::BAD_VALUE;
        }
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t releaseAllSecureStops() {
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t getHdcpLevels(HdcpLevel *connectedLevel,
            HdcpLevel *maxLevel) const {
        UNUSED(connectedLevel);
        UNUSED(maxLevel);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }


    virtual status_t getNumberOfSessions(uint32_t *currentSessions,
            uint32_t *maxSessions) const {
        UNUSED(currentSessions);
        UNUSED(maxSessions);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t getSecurityLevel(Vector<uint8_t> const &sessionId,
            SecurityLevel *level) const {
        UNUSED(sessionId);
        UNUSED(level);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t setSecurityLevel(Vector<uint8_t> const &sessionId,
            const SecurityLevel& level) {
        UNUSED(sessionId);
        UNUSED(level);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t getPropertyString(
            const String8& name, String8& value) const;

    virtual status_t getPropertyByteArray(
            const String8& name, Vector<uint8_t>& value) const;

    virtual status_t setPropertyString(
            const String8& name, const String8& value);

    virtual status_t setPropertyByteArray(
            const String8& name, const Vector<uint8_t>& value);

    virtual status_t setCipherAlgorithm(
            const Vector<uint8_t>& sessionId, const String8& algorithm) {
        if (sessionId.size() == 0 || algorithm.size() == 0) {
            return android::BAD_VALUE;
        }
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t setMacAlgorithm(
            const Vector<uint8_t>& sessionId, const String8& algorithm) {
        if (sessionId.size() == 0 || algorithm.size() == 0) {
            return android::BAD_VALUE;
        }
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t encrypt(
            const Vector<uint8_t>& sessionId,
            const Vector<uint8_t>& keyId,
            const Vector<uint8_t>& input,
            const Vector<uint8_t>& iv,
            Vector<uint8_t>& output) {
        if (sessionId.size() == 0 || keyId.size() == 0 ||
                input.size() == 0 || iv.size() == 0) {
            return android::BAD_VALUE;
        }
        UNUSED(output);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t decrypt(
            const Vector<uint8_t>& sessionId,
            const Vector<uint8_t>& keyId,
            const Vector<uint8_t>& input,
            const Vector<uint8_t>& iv,
            Vector<uint8_t>& output) {
        if (sessionId.size() == 0 || keyId.size() == 0 ||
                input.size() == 0 || iv.size() == 0) {
            return android::BAD_VALUE;
        }
        UNUSED(output);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t sign(
            const Vector<uint8_t>& sessionId,
            const Vector<uint8_t>& keyId,
            const Vector<uint8_t>& message,
            Vector<uint8_t>& signature) {
        if (sessionId.size() == 0 || keyId.size() == 0 ||
                message.size() == 0) {
            return android::BAD_VALUE;
        }
        UNUSED(signature);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t verify(
            const Vector<uint8_t>& sessionId,
            const Vector<uint8_t>& keyId,
            const Vector<uint8_t>& message,
            const Vector<uint8_t>& signature, bool& match) {
        if (sessionId.size() == 0 || keyId.size() == 0 ||
                message.size() == 0 || signature.size() == 0) {
            return android::BAD_VALUE;
        }
        UNUSED(match);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    virtual status_t signRSA(
            const Vector<uint8_t>& sessionId,
            const String8& algorithm,
            const Vector<uint8_t>& message,
            const Vector<uint8_t>& wrappedKey,
            Vector<uint8_t>& signature) {
        if (sessionId.size() == 0 || algorithm.size() == 0 ||
                message.size() == 0 || wrappedKey.size() == 0) {
            return android::BAD_VALUE;
        }
        UNUSED(signature);
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

private:
    void initProperties();
    void setPlayPolicy();

    android::KeyedVector<String8, String8> mPlayPolicy;
    android::KeyedVector<String8, String8> mStringProperties;
    android::KeyedVector<String8, Vector<uint8_t>> mByteArrayProperties;

    SessionLibrary* mSessionLibrary;

    DISALLOW_EVIL_CONSTRUCTORS(DrmPlugin);
};

} // namespace clearkeydrm

#endif // CLEARKEY_DRM_PLUGIN_H_
