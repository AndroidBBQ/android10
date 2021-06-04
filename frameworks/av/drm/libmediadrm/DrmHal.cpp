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

//#define LOG_NDEBUG 0
#define LOG_TAG "DrmHal"
#include <iomanip>

#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <android/hardware/drm/1.2/types.h>
#include <android/hidl/manager/1.2/IServiceManager.h>
#include <hidl/ServiceManagement.h>

#include <media/EventMetric.h>
#include <media/PluginMetricsReporting.h>
#include <media/drm/DrmAPI.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/base64.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaErrors.h>
#include <mediadrm/DrmHal.h>
#include <mediadrm/DrmSessionClientInterface.h>
#include <mediadrm/DrmSessionManager.h>

using drm::V1_0::KeyedVector;
using drm::V1_0::KeyRequestType;
using drm::V1_0::KeyType;
using drm::V1_0::KeyValue;
using drm::V1_0::SecureStop;
using drm::V1_0::SecureStopId;
using drm::V1_0::Status;
using drm::V1_1::HdcpLevel;
using drm::V1_1::SecureStopRelease;
using drm::V1_1::SecurityLevel;
using drm::V1_2::KeySetId;
using drm::V1_2::KeyStatusType;
using ::android::hardware::drm::V1_1::DrmMetricGroup;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hidl::manager::V1_0::IServiceManager;
using ::android::os::PersistableBundle;
using ::android::sp;

typedef drm::V1_1::KeyRequestType KeyRequestType_V1_1;
typedef drm::V1_2::Status Status_V1_2;
typedef drm::V1_2::HdcpLevel HdcpLevel_V1_2;

namespace {

// This constant corresponds to the PROPERTY_DEVICE_UNIQUE_ID constant
// in the MediaDrm API.
constexpr char kPropertyDeviceUniqueId[] = "deviceUniqueId";
constexpr char kEqualsSign[] = "=";

template<typename T>
std::string toBase64StringNoPad(const T* data, size_t size) {
    // Note that the base 64 conversion only works with arrays of single-byte
    // values. If the source is empty or is not an array of single-byte values,
    // return empty string.
    if (size == 0 || sizeof(data[0]) != 1) {
      return "";
    }

    android::AString outputString;
    encodeBase64(data, size, &outputString);
    // Remove trailing equals padding if it exists.
    while (outputString.size() > 0 && outputString.endsWith(kEqualsSign)) {
        outputString.erase(outputString.size() - 1, 1);
    }

    return std::string(outputString.c_str(), outputString.size());
}

}  // anonymous namespace

namespace android {

#define INIT_CHECK() {if (mInitCheck != OK) return mInitCheck;}

static inline int getCallingPid() {
    return IPCThreadState::self()->getCallingPid();
}

static bool checkPermission(const char* permissionString) {
    if (getpid() == IPCThreadState::self()->getCallingPid()) return true;
    bool ok = checkCallingPermission(String16(permissionString));
    if (!ok) ALOGE("Request requires %s", permissionString);
    return ok;
}

static const Vector<uint8_t> toVector(const hidl_vec<uint8_t> &vec) {
    Vector<uint8_t> vector;
    vector.appendArray(vec.data(), vec.size());
    return *const_cast<const Vector<uint8_t> *>(&vector);
}

static hidl_vec<uint8_t> toHidlVec(const Vector<uint8_t> &vector) {
    hidl_vec<uint8_t> vec;
    vec.setToExternal(const_cast<uint8_t *>(vector.array()), vector.size());
    return vec;
}

static String8 toString8(const hidl_string &string) {
    return String8(string.c_str());
}

static hidl_string toHidlString(const String8& string) {
    return hidl_string(string.string());
}

static DrmPlugin::SecurityLevel toSecurityLevel(SecurityLevel level) {
    switch(level) {
    case SecurityLevel::SW_SECURE_CRYPTO:
        return DrmPlugin::kSecurityLevelSwSecureCrypto;
    case SecurityLevel::SW_SECURE_DECODE:
        return DrmPlugin::kSecurityLevelSwSecureDecode;
    case SecurityLevel::HW_SECURE_CRYPTO:
        return DrmPlugin::kSecurityLevelHwSecureCrypto;
    case SecurityLevel::HW_SECURE_DECODE:
        return DrmPlugin::kSecurityLevelHwSecureDecode;
    case SecurityLevel::HW_SECURE_ALL:
        return DrmPlugin::kSecurityLevelHwSecureAll;
    default:
        return DrmPlugin::kSecurityLevelUnknown;
    }
}

static SecurityLevel toHidlSecurityLevel(DrmPlugin::SecurityLevel level) {
    switch(level) {
    case DrmPlugin::kSecurityLevelSwSecureCrypto:
        return SecurityLevel::SW_SECURE_CRYPTO;
    case DrmPlugin::kSecurityLevelSwSecureDecode:
        return SecurityLevel::SW_SECURE_DECODE;
    case DrmPlugin::kSecurityLevelHwSecureCrypto:
        return SecurityLevel::HW_SECURE_CRYPTO;
    case DrmPlugin::kSecurityLevelHwSecureDecode:
        return SecurityLevel::HW_SECURE_DECODE;
    case DrmPlugin::kSecurityLevelHwSecureAll:
        return SecurityLevel::HW_SECURE_ALL;
    default:
        return SecurityLevel::UNKNOWN;
    }
}

static DrmPlugin::OfflineLicenseState toOfflineLicenseState(
        OfflineLicenseState licenseState) {
    switch(licenseState) {
    case OfflineLicenseState::USABLE:
        return DrmPlugin::kOfflineLicenseStateUsable;
    case OfflineLicenseState::INACTIVE:
        return DrmPlugin::kOfflineLicenseStateReleased;
    default:
        return DrmPlugin::kOfflineLicenseStateUnknown;
    }
}

static DrmPlugin::HdcpLevel toHdcpLevel(HdcpLevel_V1_2 level) {
    switch(level) {
    case HdcpLevel_V1_2::HDCP_NONE:
        return DrmPlugin::kHdcpNone;
    case HdcpLevel_V1_2::HDCP_V1:
        return DrmPlugin::kHdcpV1;
    case HdcpLevel_V1_2::HDCP_V2:
        return DrmPlugin::kHdcpV2;
    case HdcpLevel_V1_2::HDCP_V2_1:
        return DrmPlugin::kHdcpV2_1;
    case HdcpLevel_V1_2::HDCP_V2_2:
        return DrmPlugin::kHdcpV2_2;
    case HdcpLevel_V1_2::HDCP_V2_3:
        return DrmPlugin::kHdcpV2_3;
    case HdcpLevel_V1_2::HDCP_NO_OUTPUT:
        return DrmPlugin::kHdcpNoOutput;
    default:
        return DrmPlugin::kHdcpLevelUnknown;
    }
}
static ::KeyedVector toHidlKeyedVector(const KeyedVector<String8, String8>&
        keyedVector) {
    std::vector<KeyValue> stdKeyedVector;
    for (size_t i = 0; i < keyedVector.size(); i++) {
        KeyValue keyValue;
        keyValue.key = toHidlString(keyedVector.keyAt(i));
        keyValue.value = toHidlString(keyedVector.valueAt(i));
        stdKeyedVector.push_back(keyValue);
    }
    return ::KeyedVector(stdKeyedVector);
}

static KeyedVector<String8, String8> toKeyedVector(const ::KeyedVector&
        hKeyedVector) {
    KeyedVector<String8, String8> keyedVector;
    for (size_t i = 0; i < hKeyedVector.size(); i++) {
        keyedVector.add(toString8(hKeyedVector[i].key),
                toString8(hKeyedVector[i].value));
    }
    return keyedVector;
}

static List<Vector<uint8_t>> toSecureStops(const hidl_vec<SecureStop>&
        hSecureStops) {
    List<Vector<uint8_t>> secureStops;
    for (size_t i = 0; i < hSecureStops.size(); i++) {
        secureStops.push_back(toVector(hSecureStops[i].opaqueData));
    }
    return secureStops;
}

static List<Vector<uint8_t>> toSecureStopIds(const hidl_vec<SecureStopId>&
        hSecureStopIds) {
    List<Vector<uint8_t>> secureStopIds;
    for (size_t i = 0; i < hSecureStopIds.size(); i++) {
        secureStopIds.push_back(toVector(hSecureStopIds[i]));
    }
    return secureStopIds;
}

static List<Vector<uint8_t>> toKeySetIds(const hidl_vec<KeySetId>&
        hKeySetIds) {
    List<Vector<uint8_t>> keySetIds;
    for (size_t i = 0; i < hKeySetIds.size(); i++) {
        keySetIds.push_back(toVector(hKeySetIds[i]));
    }
    return keySetIds;
}

static status_t toStatusT(Status status) {
    switch (status) {
    case Status::OK:
        return OK;
        break;
    case Status::ERROR_DRM_NO_LICENSE:
        return ERROR_DRM_NO_LICENSE;
        break;
    case Status::ERROR_DRM_LICENSE_EXPIRED:
        return ERROR_DRM_LICENSE_EXPIRED;
        break;
    case Status::ERROR_DRM_SESSION_NOT_OPENED:
        return ERROR_DRM_SESSION_NOT_OPENED;
        break;
    case Status::ERROR_DRM_CANNOT_HANDLE:
        return ERROR_DRM_CANNOT_HANDLE;
        break;
    case Status::ERROR_DRM_INVALID_STATE:
        return ERROR_DRM_INVALID_STATE;
        break;
    case Status::BAD_VALUE:
        return BAD_VALUE;
        break;
    case Status::ERROR_DRM_NOT_PROVISIONED:
        return ERROR_DRM_NOT_PROVISIONED;
        break;
    case Status::ERROR_DRM_RESOURCE_BUSY:
        return ERROR_DRM_RESOURCE_BUSY;
        break;
    case Status::ERROR_DRM_DEVICE_REVOKED:
        return ERROR_DRM_DEVICE_REVOKED;
        break;
    case Status::ERROR_DRM_UNKNOWN:
    default:
        return ERROR_DRM_UNKNOWN;
        break;
    }
}

static status_t toStatusT_1_2(Status_V1_2 status) {
    switch (status) {
    case Status_V1_2::ERROR_DRM_RESOURCE_CONTENTION:
        return ERROR_DRM_RESOURCE_CONTENTION;
    case Status_V1_2::ERROR_DRM_FRAME_TOO_LARGE:
        return ERROR_DRM_FRAME_TOO_LARGE;
    case Status_V1_2::ERROR_DRM_INSUFFICIENT_SECURITY:
        return ERROR_DRM_INSUFFICIENT_SECURITY;
    default:
        return toStatusT(static_cast<Status>(status));
    }
}


Mutex DrmHal::mLock;

struct DrmSessionClient : public DrmSessionClientInterface {
    explicit DrmSessionClient(DrmHal* drm) : mDrm(drm) {}

    virtual bool reclaimSession(const Vector<uint8_t>& sessionId) {
        sp<DrmHal> drm = mDrm.promote();
        if (drm == NULL) {
            return true;
        }
        status_t err = drm->closeSession(sessionId);
        if (err != OK) {
            return false;
        }
        drm->sendEvent(EventType::SESSION_RECLAIMED,
                toHidlVec(sessionId), hidl_vec<uint8_t>());
        return true;
    }

protected:
    virtual ~DrmSessionClient() {}

private:
    wp<DrmHal> mDrm;

    DISALLOW_EVIL_CONSTRUCTORS(DrmSessionClient);
};

DrmHal::DrmHal()
   : mDrmSessionClient(new DrmSessionClient(this)),
     mFactories(makeDrmFactories()),
     mInitCheck((mFactories.size() == 0) ? ERROR_UNSUPPORTED : NO_INIT) {
}

void DrmHal::closeOpenSessions() {
    Mutex::Autolock autoLock(mLock);
    auto openSessions = mOpenSessions;
    for (size_t i = 0; i < openSessions.size(); i++) {
        mLock.unlock();
        closeSession(openSessions[i]);
        mLock.lock();
    }
    mOpenSessions.clear();
}

DrmHal::~DrmHal() {
    DrmSessionManager::Instance()->removeDrm(mDrmSessionClient);
}

void DrmHal::cleanup() {
    closeOpenSessions();

    Mutex::Autolock autoLock(mLock);
    reportPluginMetrics();
    reportFrameworkMetrics();

    setListener(NULL);
    mInitCheck = NO_INIT;
    if (mPluginV1_2 != NULL) {
        if (!mPluginV1_2->setListener(NULL).isOk()) {
            mInitCheck = DEAD_OBJECT;
        }
    } else if (mPlugin != NULL) {
        if (!mPlugin->setListener(NULL).isOk()) {
            mInitCheck = DEAD_OBJECT;
        }
    }
    mPlugin.clear();
    mPluginV1_1.clear();
    mPluginV1_2.clear();
}

Vector<sp<IDrmFactory>> DrmHal::makeDrmFactories() {
    Vector<sp<IDrmFactory>> factories;

    auto manager = hardware::defaultServiceManager1_2();

    if (manager != NULL) {
        manager->listManifestByInterface(drm::V1_0::IDrmFactory::descriptor,
                [&factories](const hidl_vec<hidl_string> &registered) {
                    for (const auto &instance : registered) {
                        auto factory = drm::V1_0::IDrmFactory::getService(instance);
                        if (factory != NULL) {
                            factories.push_back(factory);
                        }
                    }
                }
            );
        manager->listManifestByInterface(drm::V1_1::IDrmFactory::descriptor,
                [&factories](const hidl_vec<hidl_string> &registered) {
                    for (const auto &instance : registered) {
                        auto factory = drm::V1_1::IDrmFactory::getService(instance);
                        if (factory != NULL) {
                            factories.push_back(factory);
                        }
                    }
                }
            );
        manager->listByInterface(drm::V1_2::IDrmFactory::descriptor,
                [&factories](const hidl_vec<hidl_string> &registered) {
                    for (const auto &instance : registered) {
                        auto factory = drm::V1_2::IDrmFactory::getService(instance);
                        if (factory != NULL) {
                            factories.push_back(factory);
                        }
                    }
                }
            );
    }

    if (factories.size() == 0) {
        // must be in passthrough mode, load the default passthrough service
        auto passthrough = IDrmFactory::getService();
        if (passthrough != NULL) {
            ALOGI("makeDrmFactories: using default passthrough drm instance");
            factories.push_back(passthrough);
        } else {
            ALOGE("Failed to find any drm factories");
        }
    }
    return factories;
}

sp<IDrmPlugin> DrmHal::makeDrmPlugin(const sp<IDrmFactory>& factory,
        const uint8_t uuid[16], const String8& appPackageName) {
    mAppPackageName = appPackageName;
    mMetrics.SetAppPackageName(appPackageName);

    sp<IDrmPlugin> plugin;
    Return<void> hResult = factory->createPlugin(uuid, appPackageName.string(),
            [&](Status status, const sp<IDrmPlugin>& hPlugin) {
                if (status != Status::OK) {
                    ALOGE("Failed to make drm plugin");
                    return;
                }
                plugin = hPlugin;
            }
        );

    if (!hResult.isOk()) {
        ALOGE("createPlugin remote call failed");
    }

    return plugin;
}

status_t DrmHal::initCheck() const {
    return mInitCheck;
}

status_t DrmHal::setListener(const sp<IDrmClient>& listener)
{
    Mutex::Autolock lock(mEventLock);
    if (mListener != NULL){
        IInterface::asBinder(mListener)->unlinkToDeath(this);
    }
    if (listener != NULL) {
        IInterface::asBinder(listener)->linkToDeath(this);
    }
    mListener = listener;
    return NO_ERROR;
}

Return<void> DrmHal::sendEvent(EventType hEventType,
        const hidl_vec<uint8_t>& sessionId, const hidl_vec<uint8_t>& data) {
    mMetrics.mEventCounter.Increment(hEventType);

    mEventLock.lock();
    sp<IDrmClient> listener = mListener;
    mEventLock.unlock();

    if (listener != NULL) {
        Parcel obj;
        writeByteArray(obj, sessionId);
        writeByteArray(obj, data);

        Mutex::Autolock lock(mNotifyLock);
        DrmPlugin::EventType eventType;
        switch(hEventType) {
        case EventType::PROVISION_REQUIRED:
            eventType = DrmPlugin::kDrmPluginEventProvisionRequired;
            break;
        case EventType::KEY_NEEDED:
            eventType = DrmPlugin::kDrmPluginEventKeyNeeded;
            break;
        case EventType::KEY_EXPIRED:
            eventType = DrmPlugin::kDrmPluginEventKeyExpired;
            break;
        case EventType::VENDOR_DEFINED:
            eventType = DrmPlugin::kDrmPluginEventVendorDefined;
            break;
        case EventType::SESSION_RECLAIMED:
            eventType = DrmPlugin::kDrmPluginEventSessionReclaimed;
            break;
        default:
            return Void();
        }
        listener->notify(eventType, 0, &obj);
    }
    return Void();
}

Return<void> DrmHal::sendExpirationUpdate(const hidl_vec<uint8_t>& sessionId,
        int64_t expiryTimeInMS) {

    mEventLock.lock();
    sp<IDrmClient> listener = mListener;
    mEventLock.unlock();

    if (listener != NULL) {
        Parcel obj;
        writeByteArray(obj, sessionId);
        obj.writeInt64(expiryTimeInMS);

        Mutex::Autolock lock(mNotifyLock);
        listener->notify(DrmPlugin::kDrmPluginEventExpirationUpdate, 0, &obj);
    }
    return Void();
}

Return<void> DrmHal::sendKeysChange(const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<KeyStatus_V1_0>& keyStatusList_V1_0, bool hasNewUsableKey) {
    std::vector<KeyStatus> keyStatusVec;
    for (const auto &keyStatus_V1_0 : keyStatusList_V1_0) {
        keyStatusVec.push_back({keyStatus_V1_0.keyId,
                static_cast<KeyStatusType>(keyStatus_V1_0.type)});
    }
    hidl_vec<KeyStatus> keyStatusList_V1_2(keyStatusVec);
    return sendKeysChange_1_2(sessionId, keyStatusList_V1_2, hasNewUsableKey);
}

Return<void> DrmHal::sendKeysChange_1_2(const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<KeyStatus>& keyStatusList, bool hasNewUsableKey) {

    mEventLock.lock();
    sp<IDrmClient> listener = mListener;
    mEventLock.unlock();

    if (listener != NULL) {
        Parcel obj;
        writeByteArray(obj, sessionId);

        size_t nKeys = keyStatusList.size();
        obj.writeInt32(nKeys);
        for (size_t i = 0; i < nKeys; ++i) {
            const KeyStatus &keyStatus = keyStatusList[i];
            writeByteArray(obj, keyStatus.keyId);
            uint32_t type;
            switch(keyStatus.type) {
            case KeyStatusType::USABLE:
                type = DrmPlugin::kKeyStatusType_Usable;
                break;
            case KeyStatusType::EXPIRED:
                type = DrmPlugin::kKeyStatusType_Expired;
                break;
            case KeyStatusType::OUTPUTNOTALLOWED:
                type = DrmPlugin::kKeyStatusType_OutputNotAllowed;
                break;
            case KeyStatusType::STATUSPENDING:
                type = DrmPlugin::kKeyStatusType_StatusPending;
                break;
            case KeyStatusType::USABLEINFUTURE:
                type = DrmPlugin::kKeyStatusType_UsableInFuture;
                break;
            case KeyStatusType::INTERNALERROR:
            default:
                type = DrmPlugin::kKeyStatusType_InternalError;
                break;
            }
            obj.writeInt32(type);
            mMetrics.mKeyStatusChangeCounter.Increment(keyStatus.type);
        }
        obj.writeInt32(hasNewUsableKey);

        Mutex::Autolock lock(mNotifyLock);
        listener->notify(DrmPlugin::kDrmPluginEventKeysChange, 0, &obj);
    } else {
        // There's no listener. But we still want to count the key change
        // events.
        size_t nKeys = keyStatusList.size();
        for (size_t i = 0; i < nKeys; i++) {
            mMetrics.mKeyStatusChangeCounter.Increment(keyStatusList[i].type);
        }
    }

    return Void();
}

Return<void> DrmHal::sendSessionLostState(
        const hidl_vec<uint8_t>& sessionId) {

    mEventLock.lock();
    sp<IDrmClient> listener = mListener;
    mEventLock.unlock();

    if (listener != NULL) {
        Parcel obj;
        writeByteArray(obj, sessionId);
        Mutex::Autolock lock(mNotifyLock);
        listener->notify(DrmPlugin::kDrmPluginEventSessionLostState, 0, &obj);
    }
    return Void();
}

status_t DrmHal::matchMimeTypeAndSecurityLevel(const sp<IDrmFactory> &factory,
                                               const uint8_t uuid[16],
                                               const String8 &mimeType,
                                               DrmPlugin::SecurityLevel level,
                                               bool *isSupported) {
    *isSupported = false;

    // handle default value cases
    if (level == DrmPlugin::kSecurityLevelUnknown) {
        if (mimeType == "") {
            // isCryptoSchemeSupported(uuid)
            *isSupported = true;
        } else {
            // isCryptoSchemeSupported(uuid, mimeType)
            *isSupported = factory->isContentTypeSupported(mimeType.string());
        }
        return OK;
    } else if (mimeType == "") {
        return BAD_VALUE;
    }

    sp<drm::V1_2::IDrmFactory> factoryV1_2 = drm::V1_2::IDrmFactory::castFrom(factory);
    if (factoryV1_2 == NULL) {
        return ERROR_UNSUPPORTED;
    } else {
        *isSupported = factoryV1_2->isCryptoSchemeSupported_1_2(uuid,
                mimeType.string(), toHidlSecurityLevel(level));
        return OK;
    }
}

status_t DrmHal::isCryptoSchemeSupported(const uint8_t uuid[16],
                                         const String8 &mimeType,
                                         DrmPlugin::SecurityLevel level,
                                         bool *isSupported) {
    Mutex::Autolock autoLock(mLock);
    *isSupported = false;
    for (ssize_t i = mFactories.size() - 1; i >= 0; i--) {
        if (mFactories[i]->isCryptoSchemeSupported(uuid)) {
            return matchMimeTypeAndSecurityLevel(mFactories[i],
                    uuid, mimeType, level, isSupported);
        }
    }
    return OK;
}

status_t DrmHal::createPlugin(const uint8_t uuid[16],
        const String8& appPackageName) {
    Mutex::Autolock autoLock(mLock);

    for (ssize_t i = mFactories.size() - 1; i >= 0; i--) {
        if (mFactories[i]->isCryptoSchemeSupported(uuid)) {
            auto plugin = makeDrmPlugin(mFactories[i], uuid, appPackageName);
            if (plugin != NULL) {
                mPlugin = plugin;
                mPluginV1_1 = drm::V1_1::IDrmPlugin::castFrom(mPlugin);
                mPluginV1_2 = drm::V1_2::IDrmPlugin::castFrom(mPlugin);
                break;
            }
        }
    }

    if (mPlugin == NULL) {
        mInitCheck = ERROR_UNSUPPORTED;
    } else {
        mInitCheck = OK;
        if (mPluginV1_2 != NULL) {
            if (!mPluginV1_2->setListener(this).isOk()) {
                mInitCheck = DEAD_OBJECT;
            }
        } else if (!mPlugin->setListener(this).isOk()) {
            mInitCheck = DEAD_OBJECT;
        }
        if (mInitCheck != OK) {
            mPlugin.clear();
            mPluginV1_1.clear();
            mPluginV1_2.clear();
        }
    }


    return mInitCheck;
}

status_t DrmHal::destroyPlugin() {
    cleanup();
    return OK;
}

status_t DrmHal::openSession(DrmPlugin::SecurityLevel level,
        Vector<uint8_t> &sessionId) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    SecurityLevel hSecurityLevel = toHidlSecurityLevel(level);
    bool setSecurityLevel = true;

    if (level == DrmPlugin::kSecurityLevelMax) {
        setSecurityLevel = false;
    } else {
        if (hSecurityLevel == SecurityLevel::UNKNOWN) {
            return ERROR_DRM_CANNOT_HANDLE;
        }
    }

    status_t  err = UNKNOWN_ERROR;
    bool retry = true;
    do {
        hidl_vec<uint8_t> hSessionId;

        Return<void> hResult;
        if (mPluginV1_1 == NULL || !setSecurityLevel) {
            hResult = mPlugin->openSession(
                    [&](Status status,const hidl_vec<uint8_t>& id) {
                        if (status == Status::OK) {
                            sessionId = toVector(id);
                        }
                        err = toStatusT(status);
                    }
                );
        } else {
            hResult = mPluginV1_1->openSession_1_1(hSecurityLevel,
                    [&](Status status, const hidl_vec<uint8_t>& id) {
                        if (status == Status::OK) {
                            sessionId = toVector(id);
                        }
                        err = toStatusT(status);
                    }
                );
        }

        if (!hResult.isOk()) {
            err = DEAD_OBJECT;
        }

        if (err == ERROR_DRM_RESOURCE_BUSY && retry) {
            mLock.unlock();
            // reclaimSession may call back to closeSession, since mLock is
            // shared between Drm instances, we should unlock here to avoid
            // deadlock.
            retry = DrmSessionManager::Instance()->reclaimSession(getCallingPid());
            mLock.lock();
        } else {
            retry = false;
        }
    } while (retry);

    if (err == OK) {
        DrmSessionManager::Instance()->addSession(getCallingPid(),
                mDrmSessionClient, sessionId);
        mOpenSessions.push(sessionId);
        mMetrics.SetSessionStart(sessionId);
    }

    mMetrics.mOpenSessionCounter.Increment(err);
    return err;
}

status_t DrmHal::closeSession(Vector<uint8_t> const &sessionId) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    Return<Status> status = mPlugin->closeSession(toHidlVec(sessionId));
    if (status.isOk()) {
        if (status == Status::OK) {
            DrmSessionManager::Instance()->removeSession(sessionId);
            for (size_t i = 0; i < mOpenSessions.size(); i++) {
                if (mOpenSessions[i] == sessionId) {
                    mOpenSessions.removeAt(i);
                    break;
                }
            }
        }
        status_t response = toStatusT(status);
        mMetrics.SetSessionEnd(sessionId);
        mMetrics.mCloseSessionCounter.Increment(response);
        return response;
    }
    mMetrics.mCloseSessionCounter.Increment(DEAD_OBJECT);
    return DEAD_OBJECT;
}

static DrmPlugin::KeyRequestType toKeyRequestType(
        KeyRequestType keyRequestType) {
    switch (keyRequestType) {
        case KeyRequestType::INITIAL:
            return DrmPlugin::kKeyRequestType_Initial;
            break;
        case KeyRequestType::RENEWAL:
            return DrmPlugin::kKeyRequestType_Renewal;
            break;
        case KeyRequestType::RELEASE:
            return DrmPlugin::kKeyRequestType_Release;
            break;
        default:
            return DrmPlugin::kKeyRequestType_Unknown;
            break;
    }
}

static DrmPlugin::KeyRequestType toKeyRequestType_1_1(
        KeyRequestType_V1_1 keyRequestType) {
    switch (keyRequestType) {
        case KeyRequestType_V1_1::NONE:
            return DrmPlugin::kKeyRequestType_None;
            break;
        case KeyRequestType_V1_1::UPDATE:
            return DrmPlugin::kKeyRequestType_Update;
            break;
        default:
            return toKeyRequestType(static_cast<KeyRequestType>(keyRequestType));
            break;
    }
}

status_t DrmHal::getKeyRequest(Vector<uint8_t> const &sessionId,
        Vector<uint8_t> const &initData, String8 const &mimeType,
        DrmPlugin::KeyType keyType, KeyedVector<String8,
        String8> const &optionalParameters, Vector<uint8_t> &request,
        String8 &defaultUrl, DrmPlugin::KeyRequestType *keyRequestType) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();
    EventTimer<status_t> keyRequestTimer(&mMetrics.mGetKeyRequestTimeUs);

    DrmSessionManager::Instance()->useSession(sessionId);

    KeyType hKeyType;
    if (keyType == DrmPlugin::kKeyType_Streaming) {
        hKeyType = KeyType::STREAMING;
    } else if (keyType == DrmPlugin::kKeyType_Offline) {
        hKeyType = KeyType::OFFLINE;
    } else if (keyType == DrmPlugin::kKeyType_Release) {
        hKeyType = KeyType::RELEASE;
    } else {
        keyRequestTimer.SetAttribute(BAD_VALUE);
        return BAD_VALUE;
    }

    ::KeyedVector hOptionalParameters = toHidlKeyedVector(optionalParameters);

    status_t err = UNKNOWN_ERROR;
    Return<void> hResult;

    if (mPluginV1_2 != NULL) {
        hResult = mPluginV1_2->getKeyRequest_1_2(
                toHidlVec(sessionId), toHidlVec(initData),
                toHidlString(mimeType), hKeyType, hOptionalParameters,
                [&](Status_V1_2 status, const hidl_vec<uint8_t>& hRequest,
                        KeyRequestType_V1_1 hKeyRequestType,
                        const hidl_string& hDefaultUrl) {
                    if (status == Status_V1_2::OK) {
                        request = toVector(hRequest);
                        defaultUrl = toString8(hDefaultUrl);
                        *keyRequestType = toKeyRequestType_1_1(hKeyRequestType);
                    }
                    err = toStatusT_1_2(status);
                });
    } else if (mPluginV1_1 != NULL) {
        hResult = mPluginV1_1->getKeyRequest_1_1(
                toHidlVec(sessionId), toHidlVec(initData),
                toHidlString(mimeType), hKeyType, hOptionalParameters,
                [&](Status status, const hidl_vec<uint8_t>& hRequest,
                        KeyRequestType_V1_1 hKeyRequestType,
                        const hidl_string& hDefaultUrl) {
                    if (status == Status::OK) {
                        request = toVector(hRequest);
                        defaultUrl = toString8(hDefaultUrl);
                        *keyRequestType = toKeyRequestType_1_1(hKeyRequestType);
                    }
                    err = toStatusT(status);
                });
    } else {
        hResult = mPlugin->getKeyRequest(
                toHidlVec(sessionId), toHidlVec(initData),
                toHidlString(mimeType), hKeyType, hOptionalParameters,
                [&](Status status, const hidl_vec<uint8_t>& hRequest,
                        KeyRequestType hKeyRequestType,
                        const hidl_string& hDefaultUrl) {
                    if (status == Status::OK) {
                        request = toVector(hRequest);
                        defaultUrl = toString8(hDefaultUrl);
                        *keyRequestType = toKeyRequestType(hKeyRequestType);
                    }
                    err = toStatusT(status);
                });
    }

    err = hResult.isOk() ? err : DEAD_OBJECT;
    keyRequestTimer.SetAttribute(err);
    return err;
}

status_t DrmHal::provideKeyResponse(Vector<uint8_t> const &sessionId,
        Vector<uint8_t> const &response, Vector<uint8_t> &keySetId) {
    Mutex::Autolock autoLock(mLock);
    EventTimer<status_t> keyResponseTimer(&mMetrics.mProvideKeyResponseTimeUs);

    INIT_CHECK();

    DrmSessionManager::Instance()->useSession(sessionId);

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->provideKeyResponse(toHidlVec(sessionId),
            toHidlVec(response),
            [&](Status status, const hidl_vec<uint8_t>& hKeySetId) {
                if (status == Status::OK) {
                    keySetId = toVector(hKeySetId);
                }
                err = toStatusT(status);
            }
        );
    err = hResult.isOk() ? err : DEAD_OBJECT;
    keyResponseTimer.SetAttribute(err);
    return err;
}

status_t DrmHal::removeKeys(Vector<uint8_t> const &keySetId) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    Return<Status> status = mPlugin->removeKeys(toHidlVec(keySetId));
    return status.isOk() ? toStatusT(status) : DEAD_OBJECT;
}

status_t DrmHal::restoreKeys(Vector<uint8_t> const &sessionId,
        Vector<uint8_t> const &keySetId) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    DrmSessionManager::Instance()->useSession(sessionId);

    Return<Status> status = mPlugin->restoreKeys(toHidlVec(sessionId),
            toHidlVec(keySetId));
    return status.isOk() ? toStatusT(status) : DEAD_OBJECT;
}

status_t DrmHal::queryKeyStatus(Vector<uint8_t> const &sessionId,
        KeyedVector<String8, String8> &infoMap) const {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    DrmSessionManager::Instance()->useSession(sessionId);

    ::KeyedVector hInfoMap;

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->queryKeyStatus(toHidlVec(sessionId),
            [&](Status status, const hidl_vec<KeyValue>& map) {
                if (status == Status::OK) {
                    infoMap = toKeyedVector(map);
                }
                err = toStatusT(status);
            }
        );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::getProvisionRequest(String8 const &certType,
        String8 const &certAuthority, Vector<uint8_t> &request,
        String8 &defaultUrl) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    status_t err = UNKNOWN_ERROR;
    Return<void> hResult;

    if (mPluginV1_2 != NULL) {
        Return<void> hResult = mPluginV1_2->getProvisionRequest_1_2(
                toHidlString(certType), toHidlString(certAuthority),
                [&](Status_V1_2 status, const hidl_vec<uint8_t>& hRequest,
                        const hidl_string& hDefaultUrl) {
                    if (status == Status_V1_2::OK) {
                        request = toVector(hRequest);
                        defaultUrl = toString8(hDefaultUrl);
                    }
                    err = toStatusT_1_2(status);
                }
            );
    } else {
        Return<void> hResult = mPlugin->getProvisionRequest(
                toHidlString(certType), toHidlString(certAuthority),
                [&](Status status, const hidl_vec<uint8_t>& hRequest,
                        const hidl_string& hDefaultUrl) {
                    if (status == Status::OK) {
                        request = toVector(hRequest);
                        defaultUrl = toString8(hDefaultUrl);
                    }
                    err = toStatusT(status);
                }
            );
    }

    err = hResult.isOk() ? err : DEAD_OBJECT;
    mMetrics.mGetProvisionRequestCounter.Increment(err);
    return err;
}

status_t DrmHal::provideProvisionResponse(Vector<uint8_t> const &response,
        Vector<uint8_t> &certificate, Vector<uint8_t> &wrappedKey) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->provideProvisionResponse(toHidlVec(response),
            [&](Status status, const hidl_vec<uint8_t>& hCertificate,
                    const hidl_vec<uint8_t>& hWrappedKey) {
                if (status == Status::OK) {
                    certificate = toVector(hCertificate);
                    wrappedKey = toVector(hWrappedKey);
                }
                err = toStatusT(status);
            }
        );

    err = hResult.isOk() ? err : DEAD_OBJECT;
    mMetrics.mProvideProvisionResponseCounter.Increment(err);
    return err;
}

status_t DrmHal::getSecureStops(List<Vector<uint8_t>> &secureStops) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->getSecureStops(
            [&](Status status, const hidl_vec<SecureStop>& hSecureStops) {
                if (status == Status::OK) {
                    secureStops = toSecureStops(hSecureStops);
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}


status_t DrmHal::getSecureStopIds(List<Vector<uint8_t>> &secureStopIds) {
    Mutex::Autolock autoLock(mLock);

    if (mInitCheck != OK) {
        return mInitCheck;
    }

    if (mPluginV1_1 == NULL) {
        return ERROR_DRM_CANNOT_HANDLE;
    }

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPluginV1_1->getSecureStopIds(
            [&](Status status, const hidl_vec<SecureStopId>& hSecureStopIds) {
                if (status == Status::OK) {
                    secureStopIds = toSecureStopIds(hSecureStopIds);
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}


status_t DrmHal::getSecureStop(Vector<uint8_t> const &ssid, Vector<uint8_t> &secureStop) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->getSecureStop(toHidlVec(ssid),
            [&](Status status, const SecureStop& hSecureStop) {
                if (status == Status::OK) {
                    secureStop = toVector(hSecureStop.opaqueData);
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::releaseSecureStops(Vector<uint8_t> const &ssRelease) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    Return<Status> status(Status::ERROR_DRM_UNKNOWN);
    if (mPluginV1_1 != NULL) {
        SecureStopRelease secureStopRelease;
        secureStopRelease.opaqueData = toHidlVec(ssRelease);
        status = mPluginV1_1->releaseSecureStops(secureStopRelease);
    } else {
        status = mPlugin->releaseSecureStop(toHidlVec(ssRelease));
    }
    return status.isOk() ? toStatusT(status) : DEAD_OBJECT;
}

status_t DrmHal::removeSecureStop(Vector<uint8_t> const &ssid) {
    Mutex::Autolock autoLock(mLock);

    if (mInitCheck != OK) {
        return mInitCheck;
    }

    if (mPluginV1_1 == NULL) {
        return ERROR_DRM_CANNOT_HANDLE;
    }

    Return<Status> status = mPluginV1_1->removeSecureStop(toHidlVec(ssid));
    return status.isOk() ? toStatusT(status) : DEAD_OBJECT;
}

status_t DrmHal::removeAllSecureStops() {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    Return<Status> status(Status::ERROR_DRM_UNKNOWN);
    if (mPluginV1_1 != NULL) {
        status = mPluginV1_1->removeAllSecureStops();
    } else {
        status = mPlugin->releaseAllSecureStops();
    }
    return status.isOk() ? toStatusT(status) : DEAD_OBJECT;
}

status_t DrmHal::getHdcpLevels(DrmPlugin::HdcpLevel *connected,
            DrmPlugin::HdcpLevel *max) const {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    if (connected == NULL || max == NULL) {
        return BAD_VALUE;
    }
    status_t err = UNKNOWN_ERROR;

    *connected = DrmPlugin::kHdcpLevelUnknown;
    *max = DrmPlugin::kHdcpLevelUnknown;

    Return<void> hResult;
    if (mPluginV1_2 != NULL) {
        hResult = mPluginV1_2->getHdcpLevels_1_2(
                [&](Status_V1_2 status, const HdcpLevel_V1_2& hConnected, const HdcpLevel_V1_2& hMax) {
                    if (status == Status_V1_2::OK) {
                        *connected = toHdcpLevel(hConnected);
                        *max = toHdcpLevel(hMax);
                    }
                    err = toStatusT_1_2(status);
                });
    } else if (mPluginV1_1 != NULL) {
        hResult = mPluginV1_1->getHdcpLevels(
                [&](Status status, const HdcpLevel& hConnected, const HdcpLevel& hMax) {
                    if (status == Status::OK) {
                        *connected = toHdcpLevel(static_cast<HdcpLevel_V1_2>(hConnected));
                        *max = toHdcpLevel(static_cast<HdcpLevel_V1_2>(hMax));
                    }
                    err = toStatusT(status);
                });
    } else {
        return ERROR_DRM_CANNOT_HANDLE;
    }

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::getNumberOfSessions(uint32_t *open, uint32_t *max) const {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    if (open == NULL || max == NULL) {
        return BAD_VALUE;
    }
    status_t err = UNKNOWN_ERROR;

    *open = 0;
    *max = 0;

    if (mPluginV1_1 == NULL) {
        return ERROR_DRM_CANNOT_HANDLE;
    }

    Return<void> hResult = mPluginV1_1->getNumberOfSessions(
            [&](Status status, uint32_t hOpen, uint32_t hMax) {
                if (status == Status::OK) {
                    *open = hOpen;
                    *max = hMax;
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::getSecurityLevel(Vector<uint8_t> const &sessionId,
        DrmPlugin::SecurityLevel *level) const {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    if (level == NULL) {
        return BAD_VALUE;
    }
    status_t err = UNKNOWN_ERROR;

    if (mPluginV1_1 == NULL) {
        return ERROR_DRM_CANNOT_HANDLE;
    }

    *level = DrmPlugin::kSecurityLevelUnknown;

    Return<void> hResult = mPluginV1_1->getSecurityLevel(toHidlVec(sessionId),
            [&](Status status, SecurityLevel hLevel) {
                if (status == Status::OK) {
                    *level = toSecurityLevel(hLevel);
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::getOfflineLicenseKeySetIds(List<Vector<uint8_t>> &keySetIds) const {
    Mutex::Autolock autoLock(mLock);

    if (mInitCheck != OK) {
        return mInitCheck;
    }

    if (mPluginV1_2 == NULL) {
        return ERROR_UNSUPPORTED;
    }

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPluginV1_2->getOfflineLicenseKeySetIds(
            [&](Status status, const hidl_vec<KeySetId>& hKeySetIds) {
                if (status == Status::OK) {
                    keySetIds = toKeySetIds(hKeySetIds);
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::removeOfflineLicense(Vector<uint8_t> const &keySetId) {
    Mutex::Autolock autoLock(mLock);

    if (mInitCheck != OK) {
        return mInitCheck;
    }

    if (mPluginV1_2 == NULL) {
        return ERROR_UNSUPPORTED;
    }

    Return<Status> status = mPluginV1_2->removeOfflineLicense(toHidlVec(keySetId));
    return status.isOk() ? toStatusT(status) : DEAD_OBJECT;
}

status_t DrmHal::getOfflineLicenseState(Vector<uint8_t> const &keySetId,
        DrmPlugin::OfflineLicenseState *licenseState) const {
    Mutex::Autolock autoLock(mLock);

    if (mInitCheck != OK) {
        return mInitCheck;
    }

    if (mPluginV1_2 == NULL) {
        return ERROR_UNSUPPORTED;
    }
    *licenseState = DrmPlugin::kOfflineLicenseStateUnknown;

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPluginV1_2->getOfflineLicenseState(toHidlVec(keySetId),
            [&](Status status, OfflineLicenseState hLicenseState) {
                if (status == Status::OK) {
                    *licenseState = toOfflineLicenseState(hLicenseState);
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::getPropertyString(String8 const &name, String8 &value ) const {
    Mutex::Autolock autoLock(mLock);
    return getPropertyStringInternal(name, value);
}

status_t DrmHal::getPropertyStringInternal(String8 const &name, String8 &value) const {
    // This function is internal to the class and should only be called while
    // mLock is already held.
    INIT_CHECK();

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->getPropertyString(toHidlString(name),
            [&](Status status, const hidl_string& hValue) {
                if (status == Status::OK) {
                    value = toString8(hValue);
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::getPropertyByteArray(String8 const &name, Vector<uint8_t> &value ) const {
    Mutex::Autolock autoLock(mLock);
    return getPropertyByteArrayInternal(name, value);
}

status_t DrmHal::getPropertyByteArrayInternal(String8 const &name, Vector<uint8_t> &value ) const {
    // This function is internal to the class and should only be called while
    // mLock is already held.
    INIT_CHECK();

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->getPropertyByteArray(toHidlString(name),
            [&](Status status, const hidl_vec<uint8_t>& hValue) {
                if (status == Status::OK) {
                    value = toVector(hValue);
                }
                err = toStatusT(status);
            }
    );

    err = hResult.isOk() ? err : DEAD_OBJECT;
    if (name == kPropertyDeviceUniqueId) {
        mMetrics.mGetDeviceUniqueIdCounter.Increment(err);
    }
    return err;
}

status_t DrmHal::setPropertyString(String8 const &name, String8 const &value ) const {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    Return<Status> status = mPlugin->setPropertyString(toHidlString(name),
            toHidlString(value));
    return status.isOk() ? toStatusT(status) : DEAD_OBJECT;
}

status_t DrmHal::setPropertyByteArray(String8 const &name,
                                   Vector<uint8_t> const &value ) const {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    Return<Status> status = mPlugin->setPropertyByteArray(toHidlString(name),
            toHidlVec(value));
    return status.isOk() ? toStatusT(status) : DEAD_OBJECT;
}

status_t DrmHal::getMetrics(PersistableBundle* metrics) {
    if (metrics == nullptr) {
        return UNEXPECTED_NULL;
    }
    mMetrics.Export(metrics);

    // Append vendor metrics if they are supported.
    if (mPluginV1_1 != NULL) {
        String8 vendor;
        String8 description;
        if (getPropertyStringInternal(String8("vendor"), vendor) != OK
            || vendor.isEmpty()) {
          ALOGE("Get vendor failed or is empty");
          vendor = "NONE";
        }
        if (getPropertyStringInternal(String8("description"), description) != OK
            || description.isEmpty()) {
          ALOGE("Get description failed or is empty.");
          description = "NONE";
        }
        vendor += ".";
        vendor += description;

        hidl_vec<DrmMetricGroup> pluginMetrics;
        status_t err = UNKNOWN_ERROR;

        Return<void> status = mPluginV1_1->getMetrics(
                [&](Status status, hidl_vec<DrmMetricGroup> pluginMetrics) {
                    if (status != Status::OK) {
                      ALOGV("Error getting plugin metrics: %d", status);
                    } else {
                        PersistableBundle pluginBundle;
                        if (MediaDrmMetrics::HidlMetricsToBundle(
                                pluginMetrics, &pluginBundle) == OK) {
                            metrics->putPersistableBundle(String16(vendor), pluginBundle);
                        }
                    }
                    err = toStatusT(status);
                });
        return status.isOk() ? err : DEAD_OBJECT;
    }

    return OK;
}

status_t DrmHal::setCipherAlgorithm(Vector<uint8_t> const &sessionId,
                                 String8 const &algorithm) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    DrmSessionManager::Instance()->useSession(sessionId);

    Return<Status> status = mPlugin->setCipherAlgorithm(toHidlVec(sessionId),
            toHidlString(algorithm));
    return status.isOk() ? toStatusT(status) : DEAD_OBJECT;
}

status_t DrmHal::setMacAlgorithm(Vector<uint8_t> const &sessionId,
                              String8 const &algorithm) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    DrmSessionManager::Instance()->useSession(sessionId);

    Return<Status> status = mPlugin->setMacAlgorithm(toHidlVec(sessionId),
            toHidlString(algorithm));
    return status.isOk() ? toStatusT(status) : DEAD_OBJECT;
}

status_t DrmHal::encrypt(Vector<uint8_t> const &sessionId,
        Vector<uint8_t> const &keyId, Vector<uint8_t> const &input,
        Vector<uint8_t> const &iv, Vector<uint8_t> &output) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    DrmSessionManager::Instance()->useSession(sessionId);

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->encrypt(toHidlVec(sessionId),
            toHidlVec(keyId), toHidlVec(input), toHidlVec(iv),
            [&](Status status, const hidl_vec<uint8_t>& hOutput) {
                if (status == Status::OK) {
                    output = toVector(hOutput);
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::decrypt(Vector<uint8_t> const &sessionId,
        Vector<uint8_t> const &keyId, Vector<uint8_t> const &input,
        Vector<uint8_t> const &iv, Vector<uint8_t> &output) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    DrmSessionManager::Instance()->useSession(sessionId);

    status_t  err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->decrypt(toHidlVec(sessionId),
            toHidlVec(keyId), toHidlVec(input), toHidlVec(iv),
            [&](Status status, const hidl_vec<uint8_t>& hOutput) {
                if (status == Status::OK) {
                    output = toVector(hOutput);
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::sign(Vector<uint8_t> const &sessionId,
        Vector<uint8_t> const &keyId, Vector<uint8_t> const &message,
        Vector<uint8_t> &signature) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    DrmSessionManager::Instance()->useSession(sessionId);

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->sign(toHidlVec(sessionId),
            toHidlVec(keyId), toHidlVec(message),
            [&](Status status, const hidl_vec<uint8_t>& hSignature)  {
                if (status == Status::OK) {
                    signature = toVector(hSignature);
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::verify(Vector<uint8_t> const &sessionId,
        Vector<uint8_t> const &keyId, Vector<uint8_t> const &message,
        Vector<uint8_t> const &signature, bool &match) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    DrmSessionManager::Instance()->useSession(sessionId);

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->verify(toHidlVec(sessionId),toHidlVec(keyId),
            toHidlVec(message), toHidlVec(signature),
            [&](Status status, bool hMatch) {
                if (status == Status::OK) {
                    match = hMatch;
                } else {
                    match = false;
                }
                err = toStatusT(status);
            }
    );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

status_t DrmHal::signRSA(Vector<uint8_t> const &sessionId,
        String8 const &algorithm, Vector<uint8_t> const &message,
        Vector<uint8_t> const &wrappedKey, Vector<uint8_t> &signature) {
    Mutex::Autolock autoLock(mLock);
    INIT_CHECK();

    if (!checkPermission("android.permission.ACCESS_DRM_CERTIFICATES")) {
        return -EPERM;
    }

    DrmSessionManager::Instance()->useSession(sessionId);

    status_t err = UNKNOWN_ERROR;

    Return<void> hResult = mPlugin->signRSA(toHidlVec(sessionId),
            toHidlString(algorithm), toHidlVec(message), toHidlVec(wrappedKey),
            [&](Status status, const hidl_vec<uint8_t>& hSignature) {
                if (status == Status::OK) {
                    signature = toVector(hSignature);
                }
                err = toStatusT(status);
            }
        );

    return hResult.isOk() ? err : DEAD_OBJECT;
}

void DrmHal::binderDied(const wp<IBinder> &the_late_who __unused)
{
    cleanup();
}

void DrmHal::writeByteArray(Parcel &obj, hidl_vec<uint8_t> const &vec)
{
    if (vec.size()) {
        obj.writeInt32(vec.size());
        obj.write(vec.data(), vec.size());
    } else {
        obj.writeInt32(0);
    }
}

void DrmHal::reportFrameworkMetrics() const
{
    std::unique_ptr<MediaAnalyticsItem> item(MediaAnalyticsItem::create("mediadrm"));
    item->generateSessionID();
    item->setPkgName(mMetrics.GetAppPackageName().c_str());
    String8 vendor;
    String8 description;
    status_t result = getPropertyStringInternal(String8("vendor"), vendor);
    if (result != OK) {
        ALOGE("Failed to get vendor from drm plugin: %d", result);
    } else {
        item->setCString("vendor", vendor.c_str());
    }
    result = getPropertyStringInternal(String8("description"), description);
    if (result != OK) {
        ALOGE("Failed to get description from drm plugin: %d", result);
    } else {
        item->setCString("description", description.c_str());
    }

    std::string serializedMetrics;
    result = mMetrics.GetSerializedMetrics(&serializedMetrics);
    if (result != OK) {
        ALOGE("Failed to serialize framework metrics: %d", result);
    }
    std::string b64EncodedMetrics = toBase64StringNoPad(serializedMetrics.data(),
                                                        serializedMetrics.size());
    if (!b64EncodedMetrics.empty()) {
        item->setCString("serialized_metrics", b64EncodedMetrics.c_str());
    }
    if (!item->selfrecord()) {
        ALOGE("Failed to self record framework metrics");
    }
}

void DrmHal::reportPluginMetrics() const
{
    Vector<uint8_t> metricsVector;
    String8 vendor;
    String8 description;
    if (getPropertyStringInternal(String8("vendor"), vendor) == OK &&
            getPropertyStringInternal(String8("description"), description) == OK &&
            getPropertyByteArrayInternal(String8("metrics"), metricsVector) == OK) {
        std::string metricsString = toBase64StringNoPad(metricsVector.array(),
                                                        metricsVector.size());
        status_t res = android::reportDrmPluginMetrics(metricsString, vendor,
                                                       description, mAppPackageName);
        if (res != OK) {
            ALOGE("Metrics were retrieved but could not be reported: %d", res);
        }
    }
}

}  // namespace android
