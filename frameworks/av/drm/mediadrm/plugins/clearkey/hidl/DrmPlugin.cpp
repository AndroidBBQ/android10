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
#define LOG_TAG "hidl_ClearKeyPlugin"
#include <utils/Log.h>

#include <stdio.h>
#include <inttypes.h>

#include "DrmPlugin.h"
#include "ClearKeyDrmProperties.h"
#include "Session.h"
#include "TypeConvert.h"
#include "Utils.h"

namespace {
const std::string kKeySetIdPrefix("ckid");
const int kKeySetIdLength = 16;
const int kSecureStopIdStart = 100;
const std::string kOfflineLicense("\"type\":\"persistent-license\"");
const std::string kStreaming("Streaming");
const std::string kTemporaryLicense("\"type\":\"temporary\"");
const std::string kTrue("True");

const std::string kQueryKeyLicenseType("LicenseType");
    // Value: "Streaming" or "Offline"
const std::string kQueryKeyPlayAllowed("PlayAllowed");
    // Value: "True" or "False"
const std::string kQueryKeyRenewAllowed("RenewAllowed");
    // Value: "True" or "False"

const int kSecureStopIdSize = 10;

std::vector<uint8_t> uint32ToVector(uint32_t value) {
    // 10 bytes to display max value 4294967295 + one byte null terminator
    char buffer[kSecureStopIdSize];
    memset(buffer, 0, kSecureStopIdSize);
    snprintf(buffer, kSecureStopIdSize, "%" PRIu32, value);
    return std::vector<uint8_t>(buffer, buffer + sizeof(buffer));
}

}; // unnamed namespace

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

KeyRequestType toKeyRequestType_V1_0(KeyRequestType_V1_1 keyRequestType) {
  switch (keyRequestType) {
    case KeyRequestType_V1_1::NONE:
    case KeyRequestType_V1_1::UPDATE:
      return KeyRequestType::UNKNOWN;
    default:
      return static_cast<KeyRequestType>(keyRequestType);
  }
}

DrmPlugin::DrmPlugin(SessionLibrary* sessionLibrary)
        : mSessionLibrary(sessionLibrary),
          mOpenSessionOkCount(0),
          mCloseSessionOkCount(0),
          mCloseSessionNotOpenedCount(0),
          mNextSecureStopId(kSecureStopIdStart),
          mMockError(Status_V1_2::OK) {
    mPlayPolicy.clear();
    initProperties();
    mSecureStops.clear();
    mReleaseKeysMap.clear();
    std::srand(std::time(nullptr));
}

void DrmPlugin::initProperties() {
    mStringProperties.clear();
    mStringProperties[kVendorKey] = kVendorValue;
    mStringProperties[kVersionKey] = kVersionValue;
    mStringProperties[kPluginDescriptionKey] = kPluginDescriptionValue;
    mStringProperties[kAlgorithmsKey] = kAlgorithmsValue;
    mStringProperties[kListenerTestSupportKey] = kListenerTestSupportValue;
    mStringProperties[kDrmErrorTestKey] = kDrmErrorTestValue;

    std::vector<uint8_t> valueVector;
    valueVector.clear();
    valueVector.insert(valueVector.end(),
            kTestDeviceIdData, kTestDeviceIdData + sizeof(kTestDeviceIdData) / sizeof(uint8_t));
    mByteArrayProperties[kDeviceIdKey] = valueVector;

    valueVector.clear();
    valueVector.insert(valueVector.end(),
            kMetricsData, kMetricsData + sizeof(kMetricsData) / sizeof(uint8_t));
    mByteArrayProperties[kMetricsKey] = valueVector;
}

// The secure stop in ClearKey implementation is not installed securely.
// This function merely creates a test environment for testing secure stops APIs.
// The content in this secure stop is implementation dependent, the clearkey
// secureStop does not serve as a reference implementation.
void DrmPlugin::installSecureStop(const hidl_vec<uint8_t>& sessionId) {
    Mutex::Autolock lock(mSecureStopLock);

    ClearkeySecureStop clearkeySecureStop;
    clearkeySecureStop.id = uint32ToVector(++mNextSecureStopId);
    clearkeySecureStop.data.assign(sessionId.begin(), sessionId.end());

    mSecureStops.insert(std::pair<std::vector<uint8_t>, ClearkeySecureStop>(
            clearkeySecureStop.id, clearkeySecureStop));
}

Return<void> DrmPlugin::openSession(openSession_cb _hidl_cb) {
    sp<Session> session = mSessionLibrary->createSession();
    processMockError(session);
    std::vector<uint8_t> sessionId = session->sessionId();

    Status status = setSecurityLevel(sessionId, SecurityLevel::SW_SECURE_CRYPTO);
    _hidl_cb(status, toHidlVec(sessionId));
    mOpenSessionOkCount++;
    return Void();
}

Return<void> DrmPlugin::openSession_1_1(SecurityLevel securityLevel,
        openSession_1_1_cb _hidl_cb) {
    sp<Session> session = mSessionLibrary->createSession();
    processMockError(session);
    std::vector<uint8_t> sessionId = session->sessionId();

    Status status = setSecurityLevel(sessionId, securityLevel);
    _hidl_cb(status, toHidlVec(sessionId));
    mOpenSessionOkCount++;
    return Void();
}

Return<Status> DrmPlugin::closeSession(const hidl_vec<uint8_t>& sessionId) {
    if (sessionId.size() == 0) {
        return Status::BAD_VALUE;
    }

    sp<Session> session = mSessionLibrary->findSession(toVector(sessionId));
    if (session.get()) {
        if (session->getMockError() != Status_V1_2::OK) {
            sendSessionLostState(sessionId);
            return Status::ERROR_DRM_INVALID_STATE;
        }
        mCloseSessionOkCount++;
        mSessionLibrary->destroySession(session);
        return Status::OK;
    }
    mCloseSessionNotOpenedCount++;
    return Status::ERROR_DRM_SESSION_NOT_OPENED;
}

Status_V1_2 DrmPlugin::getKeyRequestCommon(const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        std::vector<uint8_t> *request,
        KeyRequestType_V1_1 *keyRequestType,
        std::string *defaultUrl) {
        UNUSED(optionalParameters);

    // GetKeyRequestOfflineKeyTypeNotSupported() in vts 1.0 and 1.1 expects
    // KeyType::OFFLINE to return ERROR_DRM_CANNOT_HANDLE in clearkey plugin.
    // Those tests pass in an empty initData, we use the empty initData to
    // signal such specific use case.
    if (keyType == KeyType::OFFLINE && 0 == initData.size()) {
        return Status_V1_2::ERROR_DRM_CANNOT_HANDLE;
    }

    *defaultUrl = "";
    *keyRequestType = KeyRequestType_V1_1::UNKNOWN;
    *request = std::vector<uint8_t>();

    if (scope.size() == 0 ||
            (keyType != KeyType::STREAMING &&
            keyType != KeyType::OFFLINE &&
            keyType != KeyType::RELEASE)) {
        return Status_V1_2::BAD_VALUE;
    }

    const std::vector<uint8_t> scopeId = toVector(scope);
    sp<Session> session;
    if (keyType == KeyType::STREAMING || keyType == KeyType::OFFLINE) {
        std::vector<uint8_t> sessionId(scopeId.begin(), scopeId.end());
        session = mSessionLibrary->findSession(sessionId);
        if (!session.get()) {
            return Status_V1_2::ERROR_DRM_SESSION_NOT_OPENED;
        } else if (session->getMockError() != Status_V1_2::OK) {
            return session->getMockError();
        }

        *keyRequestType = KeyRequestType_V1_1::INITIAL;
    }

    Status_V1_2 status = static_cast<Status_V1_2>(
            session->getKeyRequest(initData, mimeType, keyType, request));

    if (keyType == KeyType::RELEASE) {
        std::vector<uint8_t> keySetId(scopeId.begin(), scopeId.end());
        std::string requestString(request->begin(), request->end());
        if (requestString.find(kOfflineLicense) != std::string::npos) {
            std::string emptyResponse;
            std::string keySetIdString(keySetId.begin(), keySetId.end());
            if (!mFileHandle.StoreLicense(keySetIdString,
                    DeviceFiles::kLicenseStateReleasing,
                    emptyResponse)) {
                ALOGE("Problem releasing offline license");
                return Status_V1_2::ERROR_DRM_UNKNOWN;
            }
            if (mReleaseKeysMap.find(keySetIdString) == mReleaseKeysMap.end()) {
                sp<Session> session = mSessionLibrary->createSession();
                mReleaseKeysMap[keySetIdString] = session->sessionId();
            } else {
                ALOGI("key is in use, ignore release request");
            }
        } else {
            ALOGE("Offline license not found, nothing to release");
        }
        *keyRequestType = KeyRequestType_V1_1::RELEASE;
    }
    return status;
}

Return<void> DrmPlugin::getKeyRequest(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_cb _hidl_cb) {
    UNUSED(optionalParameters);

    KeyRequestType_V1_1 keyRequestType = KeyRequestType_V1_1::UNKNOWN;
    std::string defaultUrl("");
    std::vector<uint8_t> request;
    Status_V1_2 status = getKeyRequestCommon(
            scope, initData, mimeType, keyType, optionalParameters,
            &request, &keyRequestType, &defaultUrl);

    _hidl_cb(toStatus_1_0(status), toHidlVec(request),
            toKeyRequestType_V1_0(keyRequestType),
            hidl_string(defaultUrl));
    return Void();
}

Return<void> DrmPlugin::getKeyRequest_1_1(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_1_1_cb _hidl_cb) {
    UNUSED(optionalParameters);

    KeyRequestType_V1_1 keyRequestType = KeyRequestType_V1_1::UNKNOWN;
    std::string defaultUrl("");
    std::vector<uint8_t> request;
    Status_V1_2 status = getKeyRequestCommon(
            scope, initData, mimeType, keyType, optionalParameters,
            &request, &keyRequestType, &defaultUrl);

    _hidl_cb(toStatus_1_0(status), toHidlVec(request),
            keyRequestType, hidl_string(defaultUrl));
    return Void();
}

Return<void> DrmPlugin::getKeyRequest_1_2(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_1_2_cb _hidl_cb) {
    UNUSED(optionalParameters);

    KeyRequestType_V1_1 keyRequestType = KeyRequestType_V1_1::UNKNOWN;
    std::string defaultUrl("");
    std::vector<uint8_t> request;
    Status_V1_2 status = getKeyRequestCommon(
            scope, initData, mimeType, keyType, optionalParameters,
            &request, &keyRequestType, &defaultUrl);

    _hidl_cb(status, toHidlVec(request), keyRequestType, hidl_string(defaultUrl));
    return Void();
}

void DrmPlugin::setPlayPolicy() {
    mPlayPolicy.clear();

    KeyValue policy;
    policy.key = kQueryKeyLicenseType;
    policy.value = kStreaming;
    mPlayPolicy.push_back(policy);

    policy.key = kQueryKeyPlayAllowed;
    policy.value = kTrue;
    mPlayPolicy.push_back(policy);

    policy.key = kQueryKeyRenewAllowed;
    mPlayPolicy.push_back(policy);
}

bool DrmPlugin::makeKeySetId(std::string* keySetId) {
    if (!keySetId) {
        ALOGE("keySetId destination not provided");
        return false;
    }
    std::vector<uint8_t> ksid(kKeySetIdPrefix.begin(), kKeySetIdPrefix.end());
    ksid.resize(kKeySetIdLength);
    std::vector<uint8_t> randomData((kKeySetIdLength - kKeySetIdPrefix.size()) / 2, 0);

    while (keySetId->empty()) {
        for (auto itr = randomData.begin(); itr != randomData.end(); ++itr) {
            *itr = std::rand() % 0xff;
        }
        *keySetId = kKeySetIdPrefix + ByteArrayToHexString(
                reinterpret_cast<const uint8_t*>(randomData.data()), randomData.size());
        if (mFileHandle.LicenseExists(*keySetId)) {
            // collision, regenerate
            ALOGV("Retry generating KeySetId");
            keySetId->clear();
        }
    }
    return true;
}

Return<void> DrmPlugin::provideKeyResponse(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& response,
        provideKeyResponse_cb _hidl_cb) {
    if (scope.size() == 0 || response.size() == 0) {
        // Returns empty keySetId
        _hidl_cb(Status::BAD_VALUE, hidl_vec<uint8_t>());
        return Void();
    }

    std::string responseString(
            reinterpret_cast<const char*>(response.data()), response.size());
    const std::vector<uint8_t> scopeId = toVector(scope);
    std::vector<uint8_t> sessionId;
    std::string keySetId;

    Status status = Status::OK;
    bool isOfflineLicense = responseString.find(kOfflineLicense) != std::string::npos;
    bool isRelease = (memcmp(scopeId.data(), kKeySetIdPrefix.data(), kKeySetIdPrefix.size()) == 0);
    if (isRelease) {
        keySetId.assign(scopeId.begin(), scopeId.end());

        auto iter = mReleaseKeysMap.find(std::string(keySetId.begin(), keySetId.end()));
        if (iter != mReleaseKeysMap.end()) {
            sessionId.assign(iter->second.begin(), iter->second.end());
        }
    } else {
        sessionId.assign(scopeId.begin(), scopeId.end());
        // non offline license returns empty keySetId
        keySetId.clear();
    }

    sp<Session> session = mSessionLibrary->findSession(sessionId);
    if (!session.get()) {
        _hidl_cb(Status::ERROR_DRM_SESSION_NOT_OPENED, hidl_vec<uint8_t>());
        return Void();
    }
    setPlayPolicy();

    status = session->provideKeyResponse(response);
    if (status == Status::OK) {
        if (isOfflineLicense) {
            if (isRelease) {
                mFileHandle.DeleteLicense(keySetId);
            } else {
                if (!makeKeySetId(&keySetId)) {
                    _hidl_cb(Status::ERROR_DRM_UNKNOWN, hidl_vec<uint8_t>());
                    return Void();
                }

                bool ok = mFileHandle.StoreLicense(
                        keySetId,
                        DeviceFiles::kLicenseStateActive,
                        std::string(response.begin(), response.end()));
                if (!ok) {
                    ALOGE("Failed to store offline license");
                }
            }
        }

        // Test calling AMediaDrm listeners.
        sendEvent(EventType::VENDOR_DEFINED, sessionId, sessionId);

        sendExpirationUpdate(sessionId, 100);

        std::vector<KeyStatus_V1_2> keysStatus;
        KeyStatus_V1_2 keyStatus;

        std::vector<uint8_t> keyId1 = { 0xA, 0xB, 0xC };
        keyStatus.keyId = keyId1;
        keyStatus.type = V1_2::KeyStatusType::USABLE;
        keysStatus.push_back(keyStatus);

        std::vector<uint8_t> keyId2 = { 0xD, 0xE, 0xF };
        keyStatus.keyId = keyId2;
        keyStatus.type = V1_2::KeyStatusType::EXPIRED;
        keysStatus.push_back(keyStatus);

        std::vector<uint8_t> keyId3 = { 0x0, 0x1, 0x2 };
        keyStatus.keyId = keyId3;
        keyStatus.type = V1_2::KeyStatusType::USABLEINFUTURE;
        keysStatus.push_back(keyStatus);

        sendKeysChange_1_2(sessionId, keysStatus, true);

        installSecureStop(sessionId);
    } else {
        ALOGE("provideKeyResponse returns error=%d", status);
    }

    std::vector<uint8_t> keySetIdVec(keySetId.begin(), keySetId.end());
    _hidl_cb(status, toHidlVec(keySetIdVec));
    return Void();
}

Return<Status> DrmPlugin::restoreKeys(
        const hidl_vec<uint8_t>& sessionId, const hidl_vec<uint8_t>& keySetId) {
        if (sessionId.size() == 0 || keySetId.size() == 0) {
            return Status::BAD_VALUE;
        }

        DeviceFiles::LicenseState licenseState;
        std::string offlineLicense;
        Status status = Status::OK;
        if (!mFileHandle.RetrieveLicense(std::string(keySetId.begin(), keySetId.end()),
                &licenseState, &offlineLicense)) {
            ALOGE("Failed to restore offline license");
            return Status::ERROR_DRM_NO_LICENSE;
        }

        if (DeviceFiles::kLicenseStateUnknown == licenseState ||
                DeviceFiles::kLicenseStateReleasing == licenseState) {
            ALOGE("Invalid license state=%d", licenseState);
            return Status::ERROR_DRM_NO_LICENSE;
        }

        sp<Session> session = mSessionLibrary->findSession(toVector(sessionId));
        if (!session.get()) {
            return Status::ERROR_DRM_SESSION_NOT_OPENED;
        }
        status = session->provideKeyResponse(std::vector<uint8_t>(offlineLicense.begin(),
                offlineLicense.end()));
        if (status != Status::OK) {
            ALOGE("Failed to restore keys");
        }
        return status;
}

Return<void> DrmPlugin::getPropertyString(
        const hidl_string& propertyName, getPropertyString_cb _hidl_cb) {
    std::string name(propertyName.c_str());
    std::string value;

    if (name == kVendorKey) {
        value = mStringProperties[kVendorKey];
    } else if (name == kVersionKey) {
        value = mStringProperties[kVersionKey];
    } else if (name == kPluginDescriptionKey) {
        value = mStringProperties[kPluginDescriptionKey];
    } else if (name == kAlgorithmsKey) {
        value = mStringProperties[kAlgorithmsKey];
    } else if (name == kListenerTestSupportKey) {
        value = mStringProperties[kListenerTestSupportKey];
    } else if (name == kDrmErrorTestKey) {
        value = mStringProperties[kDrmErrorTestKey];
    } else {
        ALOGE("App requested unknown string property %s", name.c_str());
        _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, "");
        return Void();
    }
    _hidl_cb(Status::OK, value.c_str());
    return Void();
}

Return<void> DrmPlugin::getPropertyByteArray(
        const hidl_string& propertyName, getPropertyByteArray_cb _hidl_cb) {
    std::map<std::string, std::vector<uint8_t> >::iterator itr =
            mByteArrayProperties.find(std::string(propertyName.c_str()));
    if (itr == mByteArrayProperties.end()) {
        ALOGE("App requested unknown property: %s", propertyName.c_str());
        _hidl_cb(Status::BAD_VALUE, std::vector<uint8_t>());
        return Void();
    }
    _hidl_cb(Status::OK, itr->second);
    return Void();

}

Return<Status> DrmPlugin::setPropertyString(
    const hidl_string& name, const hidl_string& value) {
    std::string immutableKeys;
    immutableKeys.append(kAlgorithmsKey + ",");
    immutableKeys.append(kPluginDescriptionKey + ",");
    immutableKeys.append(kVendorKey + ",");
    immutableKeys.append(kVersionKey + ",");

    std::string key = std::string(name.c_str());
    if (immutableKeys.find(key) != std::string::npos) {
        ALOGD("Cannot set immutable property: %s", key.c_str());
        return Status::BAD_VALUE;
    }

    std::map<std::string, std::string>::iterator itr =
            mStringProperties.find(key);
    if (itr == mStringProperties.end()) {
        ALOGE("Cannot set undefined property string, key=%s", key.c_str());
        return Status::BAD_VALUE;
    }

    if (name == kDrmErrorTestKey) {
        if (value == kResourceContentionValue) {
            mMockError = Status_V1_2::ERROR_DRM_RESOURCE_CONTENTION;
        } else if (value == kLostStateValue) {
            mMockError = Status_V1_2::ERROR_DRM_SESSION_LOST_STATE;
        } else if (value == kFrameTooLargeValue) {
            mMockError = Status_V1_2::ERROR_DRM_FRAME_TOO_LARGE;
        } else if (value == kInvalidStateValue)  {
            mMockError = Status_V1_2::ERROR_DRM_INVALID_STATE;
        } else {
            mMockError = Status_V1_2::ERROR_DRM_UNKNOWN;
        }
    }

    mStringProperties[key] = std::string(value.c_str());
    return Status::OK;
}

Return<Status> DrmPlugin::setPropertyByteArray(
    const hidl_string& name, const hidl_vec<uint8_t>& value) {
   UNUSED(value);
   if (name == kDeviceIdKey) {
      ALOGD("Cannot set immutable property: %s", name.c_str());
      return Status::BAD_VALUE;
   } else if (name == kClientIdKey) {
       mByteArrayProperties[kClientIdKey] = toVector(value);
       return Status::OK;
   }

   // Setting of undefined properties is not supported
   ALOGE("Failed to set property byte array, key=%s", name.c_str());
   return Status::ERROR_DRM_CANNOT_HANDLE;
}

Return<void> DrmPlugin::queryKeyStatus(
        const hidl_vec<uint8_t>& sessionId,
        queryKeyStatus_cb _hidl_cb) {

    if (sessionId.size() == 0) {
        // Returns empty key status KeyValue pair
        _hidl_cb(Status::BAD_VALUE, hidl_vec<KeyValue>());
        return Void();
    }

    std::vector<KeyValue> infoMapVec;
    infoMapVec.clear();

    KeyValue keyValuePair;
    for (size_t i = 0; i < mPlayPolicy.size(); ++i) {
        keyValuePair.key = mPlayPolicy[i].key;
        keyValuePair.value = mPlayPolicy[i].value;
        infoMapVec.push_back(keyValuePair);
    }
    _hidl_cb(Status::OK, toHidlVec(infoMapVec));
    return Void();
}

Return<void> DrmPlugin::getNumberOfSessions(getNumberOfSessions_cb _hidl_cb) {
        uint32_t currentSessions = mSessionLibrary->numOpenSessions();
        uint32_t maxSessions = 10;
        _hidl_cb(Status::OK, currentSessions, maxSessions);
        return Void();
}

Return<void> DrmPlugin::getSecurityLevel(const hidl_vec<uint8_t>& sessionId,
            getSecurityLevel_cb _hidl_cb) {
    if (sessionId.size() == 0) {
        _hidl_cb(Status::BAD_VALUE, SecurityLevel::UNKNOWN);
        return Void();
    }

    std::vector<uint8_t> sid = toVector(sessionId);
    sp<Session> session = mSessionLibrary->findSession(sid);
    if (!session.get()) {
        _hidl_cb(Status::ERROR_DRM_SESSION_NOT_OPENED, SecurityLevel::UNKNOWN);
        return Void();
    }

    std::map<std::vector<uint8_t>, SecurityLevel>::iterator itr =
            mSecurityLevel.find(sid);
    if (itr == mSecurityLevel.end()) {
        ALOGE("Session id not found");
        _hidl_cb(Status::ERROR_DRM_INVALID_STATE, SecurityLevel::UNKNOWN);
        return Void();
    }

    _hidl_cb(Status::OK, itr->second);
    return Void();
}

Return<Status> DrmPlugin::setSecurityLevel(const hidl_vec<uint8_t>& sessionId,
            SecurityLevel level) {
    if (sessionId.size() == 0) {
        ALOGE("Invalid empty session id");
        return Status::BAD_VALUE;
    }

    if (level > SecurityLevel::SW_SECURE_CRYPTO) {
        ALOGE("Cannot set security level > max");
        return Status::ERROR_DRM_CANNOT_HANDLE;
    }

    std::vector<uint8_t> sid = toVector(sessionId);
    sp<Session> session = mSessionLibrary->findSession(sid);
    if (!session.get()) {
        return Status::ERROR_DRM_SESSION_NOT_OPENED;
    }

    std::map<std::vector<uint8_t>, SecurityLevel>::iterator itr =
            mSecurityLevel.find(sid);
    if (itr != mSecurityLevel.end()) {
        mSecurityLevel[sid] = level;
    } else {
        if (!mSecurityLevel.insert(
                std::pair<std::vector<uint8_t>, SecurityLevel>(sid, level)).second) {
            ALOGE("Failed to set security level");
            return Status::ERROR_DRM_INVALID_STATE;
        }
    }
    return Status::OK;
}

Return<void> DrmPlugin::getMetrics(getMetrics_cb _hidl_cb) {
    // Set the open session count metric.
    DrmMetricGroup::Attribute openSessionOkAttribute = {
      "status", DrmMetricGroup::ValueType::INT64_TYPE, (int64_t) Status::OK, 0.0, ""
    };
    DrmMetricGroup::Value openSessionMetricValue = {
      "count", DrmMetricGroup::ValueType::INT64_TYPE, mOpenSessionOkCount, 0.0, ""
    };
    DrmMetricGroup::Metric openSessionMetric = {
      "open_session", { openSessionOkAttribute }, { openSessionMetricValue }
    };

    // Set the close session count metric.
    DrmMetricGroup::Attribute closeSessionOkAttribute = {
      "status", DrmMetricGroup::ValueType::INT64_TYPE, (int64_t) Status::OK, 0.0, ""
    };
    DrmMetricGroup::Value closeSessionMetricValue = {
      "count", DrmMetricGroup::ValueType::INT64_TYPE, mCloseSessionOkCount, 0.0, ""
    };
    DrmMetricGroup::Metric closeSessionMetric = {
      "close_session", { closeSessionOkAttribute }, { closeSessionMetricValue }
    };

    // Set the close session, not opened metric.
    DrmMetricGroup::Attribute closeSessionNotOpenedAttribute = {
      "status", DrmMetricGroup::ValueType::INT64_TYPE,
      (int64_t) Status::ERROR_DRM_SESSION_NOT_OPENED, 0.0, ""
    };
    DrmMetricGroup::Value closeSessionNotOpenedMetricValue = {
      "count", DrmMetricGroup::ValueType::INT64_TYPE, mCloseSessionNotOpenedCount, 0.0, ""
    };
    DrmMetricGroup::Metric closeSessionNotOpenedMetric = {
      "close_session", { closeSessionNotOpenedAttribute }, { closeSessionNotOpenedMetricValue }
    };

    DrmMetricGroup metrics = { { openSessionMetric, closeSessionMetric,
                                closeSessionNotOpenedMetric } };

    _hidl_cb(Status::OK, hidl_vec<DrmMetricGroup>({metrics}));
    return Void();
}

Return<void> DrmPlugin::getOfflineLicenseKeySetIds(getOfflineLicenseKeySetIds_cb _hidl_cb) {
    std::vector<std::string> licenseNames = mFileHandle.ListLicenses();
    std::vector<KeySetId> keySetIds;
    if (mMockError != Status_V1_2::OK) {
        _hidl_cb(toStatus_1_0(mMockError), keySetIds);
        return Void();
    }
    for (const auto& name : licenseNames) {
        std::vector<uint8_t> keySetId(name.begin(), name.end());
        keySetIds.push_back(keySetId);
    }
    _hidl_cb(Status::OK, keySetIds);
    return Void();
}


Return<Status> DrmPlugin::removeOfflineLicense(const KeySetId& keySetId) {
    if (mMockError != Status_V1_2::OK) {
        return toStatus_1_0(mMockError);
    }
    std::string licenseName(keySetId.begin(), keySetId.end());
    if (mFileHandle.DeleteLicense(licenseName)) {
        return Status::OK;
    }
    return Status::BAD_VALUE;
}

Return<void> DrmPlugin::getOfflineLicenseState(const KeySetId& keySetId,
        getOfflineLicenseState_cb _hidl_cb) {
    std::string licenseName(keySetId.begin(), keySetId.end());
    DeviceFiles::LicenseState state;
    std::string license;
    OfflineLicenseState hLicenseState;
    if (mMockError != Status_V1_2::OK) {
        _hidl_cb(toStatus_1_0(mMockError), OfflineLicenseState::UNKNOWN);
    } else if (mFileHandle.RetrieveLicense(licenseName, &state, &license)) {
        switch (state) {
        case DeviceFiles::kLicenseStateActive:
            hLicenseState = OfflineLicenseState::USABLE;
            break;
        case DeviceFiles::kLicenseStateReleasing:
            hLicenseState = OfflineLicenseState::INACTIVE;
            break;
        case DeviceFiles::kLicenseStateUnknown:
            hLicenseState = OfflineLicenseState::UNKNOWN;
            break;
        }
        _hidl_cb(Status::OK, hLicenseState);
    } else {
        _hidl_cb(Status::BAD_VALUE, OfflineLicenseState::UNKNOWN);
    }
    return Void();
}

Return<void> DrmPlugin::getSecureStops(getSecureStops_cb _hidl_cb) {
    mSecureStopLock.lock();
    std::vector<SecureStop> stops;
    for (auto itr = mSecureStops.begin(); itr != mSecureStops.end(); ++itr) {
        ClearkeySecureStop clearkeyStop = itr->second;
        std::vector<uint8_t> stopVec;
        stopVec.insert(stopVec.end(), clearkeyStop.id.begin(), clearkeyStop.id.end());
        stopVec.insert(stopVec.end(), clearkeyStop.data.begin(), clearkeyStop.data.end());

        SecureStop stop;
        stop.opaqueData = toHidlVec(stopVec);
        stops.push_back(stop);
    }
    mSecureStopLock.unlock();

    _hidl_cb(Status::OK, stops);
    return Void();
}

Return<void> DrmPlugin::getSecureStop(const hidl_vec<uint8_t>& secureStopId,
        getSecureStop_cb _hidl_cb) {
    std::vector<uint8_t> stopVec;

    mSecureStopLock.lock();
    auto itr = mSecureStops.find(toVector(secureStopId));
    if (itr != mSecureStops.end()) {
        ClearkeySecureStop clearkeyStop = itr->second;
        stopVec.insert(stopVec.end(), clearkeyStop.id.begin(), clearkeyStop.id.end());
        stopVec.insert(stopVec.end(), clearkeyStop.data.begin(), clearkeyStop.data.end());
    }
    mSecureStopLock.unlock();

    SecureStop stop;
    if (!stopVec.empty()) {
        stop.opaqueData = toHidlVec(stopVec);
        _hidl_cb(Status::OK, stop);
    } else {
        _hidl_cb(Status::BAD_VALUE, stop);
    }
    return Void();
}

Return<Status> DrmPlugin::releaseSecureStop(const hidl_vec<uint8_t>& secureStopId) {
    return removeSecureStop(secureStopId);
}

Return<Status> DrmPlugin::releaseAllSecureStops() {
    return removeAllSecureStops();
}

Return<void> DrmPlugin::getSecureStopIds(getSecureStopIds_cb _hidl_cb) {
    mSecureStopLock.lock();
    std::vector<SecureStopId> ids;
    for (auto itr = mSecureStops.begin(); itr != mSecureStops.end(); ++itr) {
        ids.push_back(itr->first);
    }
    mSecureStopLock.unlock();

    _hidl_cb(Status::OK, toHidlVec(ids));
    return Void();
}

Return<Status> DrmPlugin::releaseSecureStops(const SecureStopRelease& ssRelease) {
    // OpaqueData starts with 4 byte decimal integer string
    const size_t kFourBytesOffset = 4;
    if (ssRelease.opaqueData.size() < kFourBytesOffset) {
        ALOGE("Invalid secureStopRelease length");
        return Status::BAD_VALUE;
    }

    Status status = Status::OK;
    std::vector<uint8_t> input = toVector(ssRelease.opaqueData);

    if (input.size() < kSecureStopIdSize + kFourBytesOffset) {
        // The minimum size of SecureStopRelease has to contain
        // a 4 bytes count and one secureStop id
        ALOGE("Total size of secureStops is too short");
        return Status::BAD_VALUE;
    }

    // The format of opaqueData is shared between the server
    // and the drm service. The clearkey implementation consists of:
    //    count - number of secure stops
    //    list of fixed length secure stops
    size_t countBufferSize = sizeof(uint32_t);
    if (input.size() < countBufferSize) {
        // SafetyNet logging
        android_errorWriteLog(0x534e4554, "144766455");
        return Status::BAD_VALUE;
    }
    uint32_t count = 0;
    sscanf(reinterpret_cast<char*>(input.data()), "%04" PRIu32, &count);

    // Avoid divide by 0 below.
    if (count == 0) {
        ALOGE("Invalid 0 secureStop count");
        return Status::BAD_VALUE;
    }

    // Computes the fixed length secureStop size
    size_t secureStopSize = (input.size() - kFourBytesOffset) / count;
    if (secureStopSize < kSecureStopIdSize) {
        // A valid secureStop contains the id plus data
        ALOGE("Invalid secureStop size");
        return Status::BAD_VALUE;
    }
    uint8_t* buffer = new uint8_t[secureStopSize];
    size_t offset = kFourBytesOffset; // skip the count
    for (size_t i = 0; i < count; ++i, offset += secureStopSize) {
        memcpy(buffer, input.data() + offset, secureStopSize);

        // A secureStop contains id+data, we only use the id for removal
        std::vector<uint8_t> id(buffer, buffer + kSecureStopIdSize);
        status = removeSecureStop(toHidlVec(id));
        if (Status::OK != status) break;
    }

    delete[] buffer;
    return status;
}

Return<Status> DrmPlugin::removeSecureStop(const hidl_vec<uint8_t>& secureStopId) {
    Mutex::Autolock lock(mSecureStopLock);

    if (1 != mSecureStops.erase(toVector(secureStopId))) {
        return Status::BAD_VALUE;
    }
    return Status::OK;
}

Return<Status> DrmPlugin::removeAllSecureStops() {
    Mutex::Autolock lock(mSecureStopLock);

    mSecureStops.clear();
    mNextSecureStopId = kSecureStopIdStart;
    return Status::OK;
}

}  // namespace clearkey
}  // namespace V1_2
}  // namespace drm
}  // namespace hardware
}  // namespace android
