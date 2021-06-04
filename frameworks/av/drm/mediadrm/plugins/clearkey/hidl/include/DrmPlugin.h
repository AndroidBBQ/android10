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

#ifndef CLEARKEY_DRM_PLUGIN_H_
#define CLEARKEY_DRM_PLUGIN_H_

#include <android/hardware/drm/1.2/IDrmPlugin.h>
#include <android/hardware/drm/1.2/IDrmPluginListener.h>

#include <map>
#include <stdio.h>

#include <utils/List.h>

#include "DeviceFiles.h"
#include "SessionLibrary.h"
#include "Utils.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

namespace drm = ::android::hardware::drm;
using drm::V1_0::EventType;
using drm::V1_0::IDrmPluginListener;
using drm::V1_0::KeyRequestType;
using drm::V1_0::KeyStatus;
using drm::V1_0::KeyType;
using drm::V1_0::KeyValue;
using drm::V1_0::SecureStop;
using drm::V1_0::SecureStopId;
using drm::V1_0::SessionId;
using drm::V1_0::Status;
using drm::V1_1::DrmMetricGroup;
using drm::V1_1::HdcpLevel;
using drm::V1_1::SecureStopRelease;
using drm::V1_1::SecurityLevel;
using drm::V1_2::IDrmPlugin;
using drm::V1_2::KeySetId;
using drm::V1_2::OfflineLicenseState;

using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

typedef drm::V1_1::KeyRequestType KeyRequestType_V1_1;
typedef drm::V1_2::IDrmPluginListener IDrmPluginListener_V1_2;
typedef drm::V1_2::KeyStatus KeyStatus_V1_2;
typedef drm::V1_2::Status Status_V1_2;
typedef drm::V1_2::HdcpLevel HdcpLevel_V1_2;

struct DrmPlugin : public IDrmPlugin {
    explicit DrmPlugin(SessionLibrary* sessionLibrary);

    virtual ~DrmPlugin() { mFileHandle.DeleteAllLicenses(); }

    Return<void> openSession(openSession_cb _hidl_cb) override;
    Return<void> openSession_1_1(SecurityLevel securityLevel,
            openSession_cb _hidl_cb) override;

    Return<Status> closeSession(const hidl_vec<uint8_t>& sessionId) override;

    Return<void> getKeyRequest(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_cb _hidl_cb) override;

    Return<void> getKeyRequest_1_1(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_1_1_cb _hidl_cb) override;

    Return<void> getKeyRequest_1_2(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_1_2_cb _hidl_cb) override;

    Return<void> provideKeyResponse(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& response,
        provideKeyResponse_cb _hidl_cb) override;

    Return<Status> removeKeys(const hidl_vec<uint8_t>& sessionId) {
        if (sessionId.size() == 0) {
            return Status::BAD_VALUE;
        }
        return Status::ERROR_DRM_CANNOT_HANDLE;
    }

    Return<Status> restoreKeys(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keySetId) override;

    Return<void> queryKeyStatus(
        const hidl_vec<uint8_t>& sessionId,
        queryKeyStatus_cb _hidl_cb) override;

    Return<void> getProvisionRequest(
        const hidl_string& certificateType,
        const hidl_string& certificateAuthority,
        getProvisionRequest_cb _hidl_cb) {
        UNUSED(certificateType);
        UNUSED(certificateAuthority);

        hidl_string defaultUrl;
        _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, hidl_vec<uint8_t>(), defaultUrl);
        return Void();
    }

    Return<void> getProvisionRequest_1_2(
        const hidl_string& certificateType,
        const hidl_string& certificateAuthority,
        getProvisionRequest_1_2_cb _hidl_cb) {
        UNUSED(certificateType);
        UNUSED(certificateAuthority);

        hidl_string defaultUrl;
        _hidl_cb(Status_V1_2::ERROR_DRM_CANNOT_HANDLE, hidl_vec<uint8_t>(), defaultUrl);
        return Void();
    }

    Return<void> provideProvisionResponse(
        const hidl_vec<uint8_t>& response,
        provideProvisionResponse_cb _hidl_cb) {

        if (response.size() == 0) {
            _hidl_cb(Status::BAD_VALUE, hidl_vec<uint8_t>(), hidl_vec<uint8_t>());
            return Void();
        }
        _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, hidl_vec<uint8_t>(), hidl_vec<uint8_t>());
        return Void();
    }

    Return<void> getHdcpLevels(getHdcpLevels_cb _hidl_cb) {
        HdcpLevel connectedLevel = HdcpLevel::HDCP_NONE;
        HdcpLevel maxLevel = HdcpLevel::HDCP_NO_OUTPUT;
        _hidl_cb(Status::OK, connectedLevel, maxLevel);
        return Void();
    }

    Return<void> getHdcpLevels_1_2(getHdcpLevels_1_2_cb _hidl_cb) {
        HdcpLevel_V1_2 connectedLevel = HdcpLevel_V1_2::HDCP_NONE;
        HdcpLevel_V1_2 maxLevel = HdcpLevel_V1_2::HDCP_NO_OUTPUT;
        _hidl_cb(Status_V1_2::OK, connectedLevel, maxLevel);
        return Void();
    }

    Return<void> getNumberOfSessions(getNumberOfSessions_cb _hidl_cb) override;

    Return<void> getSecurityLevel(const hidl_vec<uint8_t>& sessionId,
            getSecurityLevel_cb _hidl_cb) override;

    Return<void> getMetrics(getMetrics_cb _hidl_cb) override;

    Return<void> getOfflineLicenseKeySetIds(getOfflineLicenseKeySetIds_cb _hidl_cb) override;

    Return<Status> removeOfflineLicense(const KeySetId &keySetId) override;

    Return<void> getOfflineLicenseState(const KeySetId &keySetId,
            getOfflineLicenseState_cb _hidl_cb) override;

    Return<void> getPropertyString(
        const hidl_string& name,
        getPropertyString_cb _hidl_cb) override;

    Return<void> getPropertyByteArray(
        const hidl_string& name,
        getPropertyByteArray_cb _hidl_cb) override;

    Return<Status> setPropertyString(
            const hidl_string& name, const hidl_string& value) override;

    Return<Status> setPropertyByteArray(
            const hidl_string& name, const hidl_vec<uint8_t>& value) override;

    Return<Status> setCipherAlgorithm(
            const hidl_vec<uint8_t>& sessionId, const hidl_string& algorithm) {
        if (sessionId.size() == 0 || algorithm.size() == 0) {
            return Status::BAD_VALUE;
        }
        return Status::ERROR_DRM_CANNOT_HANDLE;
    }

    Return<Status> setMacAlgorithm(
            const hidl_vec<uint8_t>& sessionId, const hidl_string& algorithm) {
        if (sessionId.size() == 0 || algorithm.size() == 0) {
            return Status::BAD_VALUE;
        }
        return Status::ERROR_DRM_CANNOT_HANDLE;
    }

    Return<void> encrypt(
            const hidl_vec<uint8_t>& sessionId,
            const hidl_vec<uint8_t>& keyId,
            const hidl_vec<uint8_t>& input,
            const hidl_vec<uint8_t>& iv,
            encrypt_cb _hidl_cb) {
        if (sessionId.size() == 0 || keyId.size() == 0 ||
                input.size() == 0 || iv.size() == 0) {
            _hidl_cb(Status::BAD_VALUE, hidl_vec<uint8_t>());
            return Void();
        }
        _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, hidl_vec<uint8_t>());
        return Void();
    }

    Return<void> decrypt(
            const hidl_vec<uint8_t>& sessionId,
            const hidl_vec<uint8_t>& keyId,
            const hidl_vec<uint8_t>& input,
            const hidl_vec<uint8_t>& iv,
            decrypt_cb _hidl_cb) {
        if (sessionId.size() == 0 || keyId.size() == 0 ||
                input.size() == 0 || iv.size() == 0) {
            _hidl_cb(Status::BAD_VALUE, hidl_vec<uint8_t>());
            return Void();
        }
        _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, hidl_vec<uint8_t>());
        return Void();
    }

    Return<void> sign(
            const hidl_vec<uint8_t>& sessionId,
            const hidl_vec<uint8_t>& keyId,
            const hidl_vec<uint8_t>& message,
            sign_cb _hidl_cb) {
        if (sessionId.size() == 0 || keyId.size() == 0 ||
                message.size() == 0) {
            _hidl_cb(Status::BAD_VALUE, hidl_vec<uint8_t>());
            return Void();
        }
        _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, hidl_vec<uint8_t>());
        return Void();
    }

    Return<void> verify(
            const hidl_vec<uint8_t>& sessionId,
            const hidl_vec<uint8_t>& keyId,
            const hidl_vec<uint8_t>& message,
            const hidl_vec<uint8_t>& signature,
            verify_cb _hidl_cb) {

        if (sessionId.size() == 0 || keyId.size() == 0 ||
                message.size() == 0 || signature.size() == 0) {
            _hidl_cb(Status::BAD_VALUE, false);
            return Void();
        }
        _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, false);
        return Void();
    }

    Return<void> signRSA(
            const hidl_vec<uint8_t>& sessionId,
            const hidl_string& algorithm,
            const hidl_vec<uint8_t>& message,
            const hidl_vec<uint8_t>& wrappedKey,
            signRSA_cb _hidl_cb) {
        if (sessionId.size() == 0 || algorithm.size() == 0 ||
                message.size() == 0 || wrappedKey.size() == 0) {
             _hidl_cb(Status::BAD_VALUE, hidl_vec<uint8_t>());
             return Void();
         }
         _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, hidl_vec<uint8_t>());
         return Void();
    }

    Return<void> setListener(const sp<IDrmPluginListener>& listener) {
        mListener = listener;
        mListenerV1_2 = IDrmPluginListener_V1_2::castFrom(listener);
        return Void();
    };

    Return<void> sendEvent(
            EventType eventType,
            const hidl_vec<uint8_t>& sessionId,
            const hidl_vec<uint8_t>& data) {
        if (mListenerV1_2 != NULL) {
            mListenerV1_2->sendEvent(eventType, sessionId, data);
        } else if (mListener != NULL) {
            mListener->sendEvent(eventType, sessionId, data);
        } else {
            ALOGE("Null event listener, event not sent");
        }
        return Void();
    }

    Return<void> sendExpirationUpdate(
            const hidl_vec<uint8_t>& sessionId,
            int64_t expiryTimeInMS) {
        if (mListenerV1_2 != NULL) {
            mListenerV1_2->sendExpirationUpdate(sessionId, expiryTimeInMS);
        } else if (mListener != NULL) {
            mListener->sendExpirationUpdate(sessionId, expiryTimeInMS);
        } else {
            ALOGE("Null event listener, event not sent");
        }
        return Void();
    }

    Return<void> sendKeysChange(
            const hidl_vec<uint8_t>& sessionId,
            const hidl_vec<KeyStatus>& keyStatusList, bool hasNewUsableKey) {
        if (mListenerV1_2 != NULL) {
            mListenerV1_2->sendKeysChange(sessionId, keyStatusList, hasNewUsableKey);
        } else if (mListener != NULL) {
            mListener->sendKeysChange(sessionId, keyStatusList, hasNewUsableKey);
        } else {
            ALOGE("Null event listener, event not sent");
        }
        return Void();
    }

    Return<void> sendKeysChange_1_2(
            const hidl_vec<uint8_t>& sessionId,
            const hidl_vec<KeyStatus_V1_2>& keyStatusList, bool hasNewUsableKey) {
        if (mListenerV1_2 != NULL) {
            mListenerV1_2->sendKeysChange_1_2(sessionId, keyStatusList, hasNewUsableKey);
        }
        return Void();
    }

    Return<void> sendSessionLostState(
            const hidl_vec<uint8_t>& sessionId) {
        if (mListenerV1_2 != NULL) {
            mListenerV1_2->sendSessionLostState(sessionId);
        }
        return Void();
    }

    Return<void> getSecureStops(getSecureStops_cb _hidl_cb);

    Return<void> getSecureStop(const hidl_vec<uint8_t>& secureStopId,
            getSecureStop_cb _hidl_cb);

    Return<Status> releaseSecureStop(const hidl_vec<uint8_t>& ssRelease);

    Return<Status> releaseAllSecureStops();

    Return<void> getSecureStopIds(getSecureStopIds_cb _hidl_cb);

    Return<Status> releaseSecureStops(const SecureStopRelease& ssRelease);

    Return<Status> removeSecureStop(const hidl_vec<uint8_t>& secureStopId);

    Return<Status> removeAllSecureStops();

private:
    void initProperties();
    void installSecureStop(const hidl_vec<uint8_t>& sessionId);
    bool makeKeySetId(std::string* keySetId);
    void setPlayPolicy();

    Return<Status> setSecurityLevel(const hidl_vec<uint8_t>& sessionId,
            SecurityLevel level);

    Status_V1_2 getKeyRequestCommon(const hidl_vec<uint8_t>& scope,
            const hidl_vec<uint8_t>& initData,
            const hidl_string& mimeType,
            KeyType keyType,
            const hidl_vec<KeyValue>& optionalParameters,
            std::vector<uint8_t> *request,
            KeyRequestType_V1_1 *getKeyRequestType,
            std::string *defaultUrl);

    struct ClearkeySecureStop {
        std::vector<uint8_t> id;
        std::vector<uint8_t> data;
    };

    std::map<std::vector<uint8_t>, ClearkeySecureStop> mSecureStops;
    std::vector<KeyValue> mPlayPolicy;
    std::map<std::string, std::string> mStringProperties;
    std::map<std::string, std::vector<uint8_t> > mByteArrayProperties;
    std::map<std::string, std::vector<uint8_t> > mReleaseKeysMap;
    std::map<std::vector<uint8_t>, SecurityLevel> mSecurityLevel;
    sp<IDrmPluginListener> mListener;
    sp<IDrmPluginListener_V1_2> mListenerV1_2;
    SessionLibrary *mSessionLibrary;
    int64_t mOpenSessionOkCount;
    int64_t mCloseSessionOkCount;
    int64_t mCloseSessionNotOpenedCount;
    uint32_t mNextSecureStopId;

    // set by property to mock error scenarios
    Status_V1_2 mMockError;

    void processMockError(const sp<Session> &session) {
        session->setMockError(mMockError);
        mMockError = Status_V1_2::OK;
    }

    DeviceFiles mFileHandle;
    Mutex mSecureStopLock;

    CLEARKEY_DISALLOW_COPY_AND_ASSIGN_AND_NEW(DrmPlugin);
};

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

#endif // CLEARKEY_DRM_PLUGIN_H_
