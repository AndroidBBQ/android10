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
#define LOG_TAG "NdkMediaDrm"

#include <inttypes.h>
#include <unistd.h>

#include <media/NdkMediaDrm.h>

#include <cutils/properties.h>
#include <utils/Log.h>
#include <utils/StrongPointer.h>
#include <gui/Surface.h>

#include <android-base/properties.h>
#include <binder/PermissionController.h>
#include <media/IDrm.h>
#include <media/IDrmClient.h>
#include <media/stagefright/MediaErrors.h>
#include <binder/IServiceManager.h>
#include <media/IMediaDrmService.h>
#include <media/NdkMediaCrypto.h>


using namespace android;

typedef Vector<uint8_t> idvec_t;

struct DrmListener: virtual public BnDrmClient
{
private:
    AMediaDrm *mObj;
    AMediaDrmEventListener mEventListener;
    AMediaDrmExpirationUpdateListener mExpirationUpdateListener;
    AMediaDrmKeysChangeListener mKeysChangeListener;

public:
    DrmListener(AMediaDrm *obj, AMediaDrmEventListener listener) : mObj(obj),
            mEventListener(listener), mExpirationUpdateListener(NULL), mKeysChangeListener(NULL) {}

    DrmListener(AMediaDrm *obj, AMediaDrmExpirationUpdateListener listener) : mObj(obj),
            mEventListener(NULL), mExpirationUpdateListener(listener), mKeysChangeListener(NULL) {}

    DrmListener(AMediaDrm *obj, AMediaDrmKeysChangeListener listener) : mObj(obj),
            mEventListener(NULL), mExpirationUpdateListener(NULL), mKeysChangeListener(listener) {}

    void setEventListener(AMediaDrmEventListener listener) {
        mEventListener = listener;
    }

    void setExpirationUpdateListener(AMediaDrmExpirationUpdateListener listener) {
        mExpirationUpdateListener = listener;
    }

    void setKeysChangeListener(AMediaDrmKeysChangeListener listener) {
        mKeysChangeListener = listener;
    }

    void notify(DrmPlugin::EventType eventType, int extra, const Parcel *obj);
};

struct AMediaDrm {
    sp<IDrm> mDrm;
    sp<IDrmClient> mDrmClient;
    List<idvec_t> mIds;
    KeyedVector<String8, String8> mQueryResults;
    Vector<uint8_t> mKeyRequest;
    Vector<uint8_t> mProvisionRequest;
    String8 mProvisionUrl;
    String8 mPropertyString;
    Vector<uint8_t> mPropertyByteArray;
    List<Vector<uint8_t> > mSecureStops;
    sp<DrmListener> mListener;
};

void DrmListener::notify(DrmPlugin::EventType eventType, int extra, const Parcel *obj) {
    if (!mEventListener || !mExpirationUpdateListener || !mKeysChangeListener) {
        ALOGE("No listeners are specified");
        return;
    }

    obj->setDataPosition(0);

    AMediaDrmSessionId sessionId = {NULL, 0};
    int32_t sessionIdSize = obj->readInt32();
    if (sessionIdSize <= 0) {
        ALOGE("Invalid session id size");
        return;
    }

    std::unique_ptr<uint8_t[]> sessionIdData(new uint8_t[sessionIdSize]);
    sessionId.ptr = sessionIdData.get();
    sessionId.length = sessionIdSize;
    status_t err = obj->read(sessionIdData.get(), sessionId.length);
    if (err != OK) {
        ALOGE("Failed to read session id, error=%d", err);
        return;
    }

    if (DrmPlugin::kDrmPluginEventExpirationUpdate == eventType) {
        int64_t expiryTimeInMS = obj->readInt64();
        if (expiryTimeInMS >= 0) {
            (*mExpirationUpdateListener)(mObj, &sessionId, expiryTimeInMS);
        } else {
            ALOGE("Failed to read expiry time, status=%" PRId64 "", expiryTimeInMS);
        }
        return;
    } else if (DrmPlugin::kDrmPluginEventKeysChange == eventType) {
        int32_t numKeys = 0;
        err = obj->readInt32(&numKeys);
        if (err != OK) {
            ALOGE("Failed to read number of keys status, error=%d", err);
            return;
        }

        Vector<AMediaDrmKeyStatus> keysStatus;
        std::vector<std::unique_ptr<uint8_t[]> > dataPointers;
        AMediaDrmKeyStatus keyStatus;

        for (size_t i = 0; i < numKeys; ++i) {
            keyStatus.keyId.ptr = nullptr;
            keyStatus.keyId.length = 0;
            int32_t idSize = obj->readInt32();
            if (idSize > 0) {
                std::unique_ptr<uint8_t[]> data(new uint8_t[idSize]);
                err = obj->read(data.get(), idSize);
                if (err != OK) {
                    ALOGE("Failed to read key data, error=%d", err);
                    return;
                }
                keyStatus.keyId.ptr = data.get();
                keyStatus.keyId.length = idSize;
                dataPointers.push_back(std::move(data));
            }
            keyStatus.keyType = static_cast<AMediaDrmKeyStatusType>(obj->readInt32());
            keysStatus.push(keyStatus);
        }

        bool hasNewUsableKey = obj->readInt32();
        (*mKeysChangeListener)(mObj, &sessionId, keysStatus.array(), numKeys, hasNewUsableKey);
        return;
    }

    // Handles AMediaDrmEventListener below:
    //  translates DrmPlugin event types into their NDK equivalents
    AMediaDrmEventType ndkEventType;
    switch(eventType) {
        case DrmPlugin::kDrmPluginEventProvisionRequired:
            ndkEventType = EVENT_PROVISION_REQUIRED;
            break;
        case DrmPlugin::kDrmPluginEventKeyNeeded:
            ndkEventType = EVENT_KEY_REQUIRED;
            break;
        case DrmPlugin::kDrmPluginEventKeyExpired:
            ndkEventType = EVENT_KEY_EXPIRED;
            break;
        case DrmPlugin::kDrmPluginEventVendorDefined:
            ndkEventType = EVENT_VENDOR_DEFINED;
            break;
        case DrmPlugin::kDrmPluginEventSessionReclaimed:
            ndkEventType = EVENT_SESSION_RECLAIMED;
            break;
        default:
            ALOGE("Invalid event DrmPlugin::EventType %d, ignored", (int)eventType);
            return;
    }

    int32_t dataSize = obj->readInt32();
    uint8_t *data = NULL;
    if (dataSize > 0) {
        data = new uint8_t[dataSize];
        err = obj->read(data, dataSize);
        if (err == OK) {
            (*mEventListener)(mObj, &sessionId, ndkEventType, extra, data, dataSize);
        } else {
            ALOGE("Failed to read event data, error=%d", err);
        }
        delete [] data;
    } else {
        ALOGE("Error reading parcel: invalid event data size=%d", dataSize);
    }
}

extern "C" {

static media_status_t translateStatus(status_t status) {
    media_status_t result = AMEDIA_ERROR_UNKNOWN;
    switch (status) {
        case OK:
            result = AMEDIA_OK;
            break;
        case android::ERROR_DRM_NOT_PROVISIONED:
            result = AMEDIA_DRM_NOT_PROVISIONED;
            break;
        case android::ERROR_DRM_RESOURCE_BUSY:
            result = AMEDIA_DRM_RESOURCE_BUSY;
            break;
        case android::ERROR_DRM_DEVICE_REVOKED:
            result = AMEDIA_DRM_DEVICE_REVOKED;
            break;
        case android::ERROR_DRM_CANNOT_HANDLE:
            result = AMEDIA_ERROR_INVALID_PARAMETER;
            break;
        case android::ERROR_DRM_TAMPER_DETECTED:
            result = AMEDIA_DRM_TAMPER_DETECTED;
            break;
        case android::ERROR_DRM_SESSION_NOT_OPENED:
            result = AMEDIA_DRM_SESSION_NOT_OPENED;
            break;
        case android::ERROR_DRM_NO_LICENSE:
            result = AMEDIA_DRM_NEED_KEY;
            break;
        case android::ERROR_DRM_LICENSE_EXPIRED:
            result = AMEDIA_DRM_LICENSE_EXPIRED;
            break;
        default:
            break;
    }
    return result;
}

static bool ShouldGetAppPackageName(void) {
    // Check what this device's first API level was.
    int32_t firstApiLevel = android::base::GetIntProperty<int32_t>("ro.product.first_api_level", 0);
    if (firstApiLevel == 0) {
        // First API Level is 0 on factory ROMs, but we can assume the current SDK
        // version is the first if it's a factory ROM.
        firstApiLevel = android::base::GetIntProperty<int32_t>("ro.build.version.sdk", 0);
    }
    return firstApiLevel >= 29;  // Android Q
}

static status_t GetAppPackageName(String8 *packageName) {
    sp<IServiceManager> serviceManager = defaultServiceManager();
    sp<IBinder> binder = serviceManager->getService(String16("permission"));

    sp<IPermissionController> permissionContol = interface_cast<IPermissionController>(binder);
    if (permissionContol == NULL) {
        ALOGE("Failed to get permission service");
        return UNKNOWN_ERROR;
    }

    Vector<String16> packages;
    permissionContol->getPackagesForUid(getuid(), packages);

    if (packages.isEmpty()) {
        ALOGE("Unable to get package name for current UID");
        return UNKNOWN_ERROR;
    }

    *packageName = String8(packages[0]);
    return OK;
}

static sp<IDrm> CreateDrm() {
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("media.drm"));

    sp<IMediaDrmService> service = interface_cast<IMediaDrmService>(binder);
    if (service == NULL) {
        return NULL;
    }

    sp<IDrm> drm = service->makeDrm();
    if (drm == NULL || (drm->initCheck() != OK && drm->initCheck() != NO_INIT)) {
        return NULL;
    }
    return drm;
}


static sp<IDrm> CreateDrmFromUUID(const AMediaUUID uuid) {
    sp<IDrm> drm = CreateDrm();

    if (drm == NULL) {
        return NULL;
    }

    String8 packageName;
    if (ShouldGetAppPackageName()) {
        status_t err = GetAppPackageName(&packageName);

        if (err != OK) {
            return NULL;
        }
    }

    status_t err = drm->createPlugin(uuid, packageName);

    if (err != OK) {
        return NULL;
    }

    return drm;
}

EXPORT
bool AMediaDrm_isCryptoSchemeSupported(const AMediaUUID uuid, const char *mimeType) {
    sp<IDrm> drm = CreateDrm();

    if (drm == NULL) {
        return false;
    }

    String8 mimeStr = mimeType ? String8(mimeType) : String8("");
    bool isSupported = false;
    status_t status = drm->isCryptoSchemeSupported(uuid, mimeStr,
            DrmPlugin::kSecurityLevelUnknown, &isSupported);
    return (status == OK) && isSupported;
}

EXPORT
AMediaDrm* AMediaDrm_createByUUID(const AMediaUUID uuid) {
    AMediaDrm *mObj = new AMediaDrm();
    mObj->mDrm = CreateDrmFromUUID(uuid);

    mObj->mListener.clear();
    return mObj;
}

EXPORT
void AMediaDrm_release(AMediaDrm *mObj) {
    if (mObj->mDrm != NULL) {
        mObj->mDrm->setListener(NULL);
        mObj->mDrm->destroyPlugin();
        mObj->mDrm.clear();
    }
    delete mObj;
}

EXPORT
media_status_t AMediaDrm_setOnEventListener(AMediaDrm *mObj, AMediaDrmEventListener listener) {
    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }

    if (mObj->mListener.get()) {
        mObj->mListener->setEventListener(listener);
    } else {
        mObj->mListener = new DrmListener(mObj, listener);
    }
    mObj->mDrm->setListener(mObj->mListener);
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaDrm_setOnExpirationUpdateListener(AMediaDrm *mObj,
        AMediaDrmExpirationUpdateListener listener) {
    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }

    if (mObj->mListener.get()) {
        mObj->mListener->setExpirationUpdateListener(listener);
    } else {
        mObj->mListener = new DrmListener(mObj, listener);
    }
    mObj->mDrm->setListener(mObj->mListener);
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaDrm_setOnKeysChangeListener(AMediaDrm *mObj,
        AMediaDrmKeysChangeListener listener) {
    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }

    if (mObj->mListener.get()) {
        mObj->mListener->setKeysChangeListener(listener);
    } else {
        mObj->mListener = new DrmListener(mObj, listener);
    }
    mObj->mDrm->setListener(mObj->mListener);
    return AMEDIA_OK;
}

static bool findId(AMediaDrm *mObj, const AMediaDrmByteArray &id, List<idvec_t>::iterator &iter) {
    for (iter = mObj->mIds.begin(); iter != mObj->mIds.end(); ++iter) {
        if (id.length == iter->size() && memcmp(iter->array(), id.ptr, iter->size()) == 0) {
            return true;
        }
    }
    return false;
}

EXPORT
media_status_t AMediaDrm_openSession(AMediaDrm *mObj, AMediaDrmSessionId *sessionId) {
    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!sessionId) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    Vector<uint8_t> session;
    status_t status = mObj->mDrm->openSession(DrmPlugin::kSecurityLevelMax, session);
    if (status == OK) {
        mObj->mIds.push_front(session);
        List<idvec_t>::iterator iter = mObj->mIds.begin();
        sessionId->ptr = iter->array();
        sessionId->length = iter->size();
    }
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaDrm_closeSession(AMediaDrm *mObj, const AMediaDrmSessionId *sessionId) {
    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!sessionId) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    List<idvec_t>::iterator iter;
    if (!findId(mObj, *sessionId, iter)) {
        return AMEDIA_DRM_SESSION_NOT_OPENED;
    }
    mObj->mDrm->closeSession(*iter);
    mObj->mIds.erase(iter);
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaDrm_getKeyRequest(AMediaDrm *mObj, const AMediaDrmScope *scope,
        const uint8_t *init, size_t initSize, const char *mimeType, AMediaDrmKeyType keyType,
        const AMediaDrmKeyValue *optionalParameters, size_t numOptionalParameters,
        const uint8_t **keyRequest, size_t *keyRequestSize) {

    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!mimeType || !scope || !keyRequest || !keyRequestSize) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    List<idvec_t>::iterator iter;
    if (!findId(mObj, *scope, iter)) {
        return AMEDIA_DRM_SESSION_NOT_OPENED;
    }

    Vector<uint8_t> mdInit;
    mdInit.appendArray(init, initSize);
    DrmPlugin::KeyType mdKeyType;
    switch (keyType) {
        case KEY_TYPE_STREAMING:
            mdKeyType = DrmPlugin::kKeyType_Streaming;
            break;
        case KEY_TYPE_OFFLINE:
            mdKeyType = DrmPlugin::kKeyType_Offline;
            break;
        case KEY_TYPE_RELEASE:
            mdKeyType = DrmPlugin::kKeyType_Release;
            break;
        default:
            return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    KeyedVector<String8, String8> mdOptionalParameters;
    for (size_t i = 0; i < numOptionalParameters; i++) {
        mdOptionalParameters.add(String8(optionalParameters[i].mKey),
                String8(optionalParameters[i].mValue));
    }
    String8 defaultUrl;
    DrmPlugin::KeyRequestType keyRequestType;
    mObj->mKeyRequest.clear();
    status_t status = mObj->mDrm->getKeyRequest(*iter, mdInit, String8(mimeType),
            mdKeyType, mdOptionalParameters, mObj->mKeyRequest, defaultUrl,
            &keyRequestType);
    if (status != OK) {
        return translateStatus(status);
    } else {
        *keyRequest = mObj->mKeyRequest.array();
        *keyRequestSize = mObj->mKeyRequest.size();
    }
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaDrm_provideKeyResponse(AMediaDrm *mObj, const AMediaDrmScope *scope,
        const uint8_t *response, size_t responseSize, AMediaDrmKeySetId *keySetId) {

    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!scope || !response || !responseSize || !keySetId) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    List<idvec_t>::iterator iter;
    if (!findId(mObj, *scope, iter)) {
        return AMEDIA_DRM_SESSION_NOT_OPENED;
    }
    Vector<uint8_t> mdResponse;
    mdResponse.appendArray(response, responseSize);

    Vector<uint8_t> mdKeySetId;
    status_t status = mObj->mDrm->provideKeyResponse(*iter, mdResponse, mdKeySetId);
    if (status == OK) {
        mObj->mIds.push_front(mdKeySetId);
        List<idvec_t>::iterator iter = mObj->mIds.begin();
        keySetId->ptr = iter->array();
        keySetId->length = iter->size();
    } else {
        keySetId->ptr = NULL;
        keySetId->length = 0;
    }
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaDrm_restoreKeys(AMediaDrm *mObj, const AMediaDrmSessionId *sessionId,
        const AMediaDrmKeySetId *keySetId) {

    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!sessionId || !keySetId) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    List<idvec_t>::iterator iter;
    if (!findId(mObj, *sessionId, iter)) {
        return AMEDIA_DRM_SESSION_NOT_OPENED;
    }
    Vector<uint8_t> keySet;
    keySet.appendArray(keySetId->ptr, keySetId->length);
    return translateStatus(mObj->mDrm->restoreKeys(*iter, keySet));
}

EXPORT
media_status_t AMediaDrm_removeKeys(AMediaDrm *mObj, const AMediaDrmSessionId *keySetId) {
    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!keySetId) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    List<idvec_t>::iterator iter;
    status_t status;
    if (!findId(mObj, *keySetId, iter)) {
        Vector<uint8_t> keySet;
        keySet.appendArray(keySetId->ptr, keySetId->length);
        status = mObj->mDrm->removeKeys(keySet);
    } else {
        status = mObj->mDrm->removeKeys(*iter);
        mObj->mIds.erase(iter);
    }
    return translateStatus(status);
}

EXPORT
media_status_t AMediaDrm_queryKeyStatus(AMediaDrm *mObj, const AMediaDrmSessionId *sessionId,
        AMediaDrmKeyValue *keyValuePairs, size_t *numPairs) {

    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!sessionId || !numPairs) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    List<idvec_t>::iterator iter;
    if (!findId(mObj, *sessionId, iter)) {
        return AMEDIA_DRM_SESSION_NOT_OPENED;
    }

    status_t status = mObj->mDrm->queryKeyStatus(*iter, mObj->mQueryResults);
    if (status != OK) {
        *numPairs = 0;
        return translateStatus(status);
    }

    if (mObj->mQueryResults.size() > *numPairs) {
        *numPairs = mObj->mQueryResults.size();
        return AMEDIA_DRM_SHORT_BUFFER;
    }

    for (size_t i = 0; i < mObj->mQueryResults.size(); i++) {
        keyValuePairs[i].mKey = mObj->mQueryResults.keyAt(i).string();
        keyValuePairs[i].mValue = mObj->mQueryResults.valueAt(i).string();
    }
    *numPairs = mObj->mQueryResults.size();
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaDrm_getProvisionRequest(AMediaDrm *mObj, const uint8_t **provisionRequest,
        size_t *provisionRequestSize, const char **serverUrl) {
    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!provisionRequest || !provisionRequestSize || !*provisionRequestSize || !serverUrl) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    status_t status = mObj->mDrm->getProvisionRequest(String8(""), String8(""),
            mObj->mProvisionRequest, mObj->mProvisionUrl);
    if (status != OK) {
        return translateStatus(status);
    } else {
        *provisionRequest = mObj->mProvisionRequest.array();
        *provisionRequestSize = mObj->mProvisionRequest.size();
        *serverUrl = mObj->mProvisionUrl.string();
    }
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaDrm_provideProvisionResponse(AMediaDrm *mObj,
        const uint8_t *response, size_t responseSize) {
    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!response || !responseSize) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    Vector<uint8_t> mdResponse;
    mdResponse.appendArray(response, responseSize);

    Vector<uint8_t> unused;
    return translateStatus(mObj->mDrm->provideProvisionResponse(mdResponse, unused, unused));
}

EXPORT
media_status_t AMediaDrm_getSecureStops(AMediaDrm *mObj,
        AMediaDrmSecureStop *secureStops, size_t *numSecureStops) {

    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!numSecureStops) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    status_t status = mObj->mDrm->getSecureStops(mObj->mSecureStops);
    if (status != OK) {
        *numSecureStops = 0;
        return translateStatus(status);
    }
    if (*numSecureStops < mObj->mSecureStops.size()) {
        return AMEDIA_DRM_SHORT_BUFFER;
    }
    List<Vector<uint8_t> >::iterator iter = mObj->mSecureStops.begin();
    size_t i = 0;
    while (iter != mObj->mSecureStops.end()) {
        secureStops[i].ptr = iter->array();
        secureStops[i].length = iter->size();
        ++iter;
        ++i;
    }
    *numSecureStops = mObj->mSecureStops.size();
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaDrm_releaseSecureStops(AMediaDrm *mObj,
        const AMediaDrmSecureStop *ssRelease) {

    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!ssRelease) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    Vector<uint8_t> release;
    release.appendArray(ssRelease->ptr, ssRelease->length);
    return translateStatus(mObj->mDrm->releaseSecureStops(release));
}


EXPORT
media_status_t AMediaDrm_getPropertyString(AMediaDrm *mObj, const char *propertyName,
        const char **propertyValue) {

    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!propertyName || !propertyValue) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    status_t status = mObj->mDrm->getPropertyString(String8(propertyName),
            mObj->mPropertyString);

    if (status == OK) {
        *propertyValue = mObj->mPropertyString.string();
    } else {
        *propertyValue = NULL;
    }
    return translateStatus(status);
}

EXPORT
media_status_t AMediaDrm_getPropertyByteArray(AMediaDrm *mObj,
        const char *propertyName, AMediaDrmByteArray *propertyValue) {
    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!propertyName || !propertyValue) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    status_t status = mObj->mDrm->getPropertyByteArray(String8(propertyName),
            mObj->mPropertyByteArray);

    if (status == OK) {
        propertyValue->ptr = mObj->mPropertyByteArray.array();
        propertyValue->length = mObj->mPropertyByteArray.size();
    } else {
        propertyValue->ptr = NULL;
        propertyValue->length = 0;
    }
    return translateStatus(status);
}

EXPORT
media_status_t AMediaDrm_setPropertyString(AMediaDrm *mObj,
        const char *propertyName, const char *value) {
    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }

    return translateStatus(mObj->mDrm->setPropertyString(String8(propertyName),
                    String8(value)));
}

EXPORT
media_status_t AMediaDrm_setPropertyByteArray(AMediaDrm *mObj,
        const char *propertyName, const uint8_t *value, size_t valueSize) {

    Vector<uint8_t> byteArray;
    byteArray.appendArray(value, valueSize);

    return translateStatus(mObj->mDrm->setPropertyByteArray(String8(propertyName),
                    byteArray));
}


static media_status_t encrypt_decrypt_common(AMediaDrm *mObj,
        const AMediaDrmSessionId &sessionId,
        const char *cipherAlgorithm, uint8_t *keyId, uint8_t *iv,
        const uint8_t *input, uint8_t *output, size_t dataSize, bool encrypt) {

    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    List<idvec_t>::iterator iter;
    if (!findId(mObj, sessionId, iter)) {
        return AMEDIA_DRM_SESSION_NOT_OPENED;
    }

    status_t status = mObj->mDrm->setCipherAlgorithm(*iter, String8(cipherAlgorithm));
    if (status != OK) {
        return translateStatus(status);
    }

    Vector<uint8_t> keyIdVec;
    const size_t kKeyIdSize = 16;
    keyIdVec.appendArray(keyId, kKeyIdSize);

    Vector<uint8_t> inputVec;
    inputVec.appendArray(input, dataSize);

    Vector<uint8_t> ivVec;
    const size_t kIvSize = 16;
    ivVec.appendArray(iv, kIvSize);

    Vector<uint8_t> outputVec;
    if (encrypt) {
        status = mObj->mDrm->encrypt(*iter, keyIdVec, inputVec, ivVec, outputVec);
    } else {
        status = mObj->mDrm->decrypt(*iter, keyIdVec, inputVec, ivVec, outputVec);
    }
    if (status == OK) {
        memcpy(output, outputVec.array(), outputVec.size());
    }
    return translateStatus(status);
}

EXPORT
media_status_t AMediaDrm_encrypt(AMediaDrm *mObj, const AMediaDrmSessionId *sessionId,
        const char *cipherAlgorithm, uint8_t *keyId, uint8_t *iv,
        const uint8_t *input, uint8_t *output, size_t dataSize) {
    if (!sessionId) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return encrypt_decrypt_common(mObj, *sessionId, cipherAlgorithm, keyId, iv,
            input, output, dataSize, true);
}

EXPORT
media_status_t AMediaDrm_decrypt(AMediaDrm *mObj, const AMediaDrmSessionId *sessionId,
        const char *cipherAlgorithm, uint8_t *keyId, uint8_t *iv,
        const uint8_t *input, uint8_t *output, size_t dataSize) {
    if (!sessionId) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    return encrypt_decrypt_common(mObj, *sessionId, cipherAlgorithm, keyId, iv,
            input, output, dataSize, false);
}

EXPORT
media_status_t AMediaDrm_sign(AMediaDrm *mObj, const AMediaDrmSessionId *sessionId,
        const char *macAlgorithm, uint8_t *keyId, uint8_t *message, size_t messageSize,
        uint8_t *signature, size_t *signatureSize) {

    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!sessionId) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    List<idvec_t>::iterator iter;
    if (!findId(mObj, *sessionId, iter)) {
        return AMEDIA_DRM_SESSION_NOT_OPENED;
    }

    status_t status = mObj->mDrm->setMacAlgorithm(*iter, String8(macAlgorithm));
    if (status != OK) {
        return translateStatus(status);
    }

    Vector<uint8_t> keyIdVec;
    const size_t kKeyIdSize = 16;
    keyIdVec.appendArray(keyId, kKeyIdSize);

    Vector<uint8_t> messageVec;
    messageVec.appendArray(message, messageSize);

    Vector<uint8_t> signatureVec;
    status = mObj->mDrm->sign(*iter, keyIdVec, messageVec, signatureVec);
    if (signatureVec.size() > *signatureSize) {
        return AMEDIA_DRM_SHORT_BUFFER;
    }
    if (status == OK) {
        memcpy(signature, signatureVec.array(), signatureVec.size());
    }
    return translateStatus(status);
}

EXPORT
media_status_t AMediaDrm_verify(AMediaDrm *mObj, const AMediaDrmSessionId *sessionId,
        const char *macAlgorithm, uint8_t *keyId, const uint8_t *message, size_t messageSize,
        const uint8_t *signature, size_t signatureSize) {

    if (!mObj || mObj->mDrm == NULL) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!sessionId) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    List<idvec_t>::iterator iter;
    if (!findId(mObj, *sessionId, iter)) {
        return AMEDIA_DRM_SESSION_NOT_OPENED;
    }

    status_t status = mObj->mDrm->setMacAlgorithm(*iter, String8(macAlgorithm));
    if (status != OK) {
        return translateStatus(status);
    }

    Vector<uint8_t> keyIdVec;
    const size_t kKeyIdSize = 16;
    keyIdVec.appendArray(keyId, kKeyIdSize);

    Vector<uint8_t> messageVec;
    messageVec.appendArray(message, messageSize);

    Vector<uint8_t> signatureVec;
    signatureVec.appendArray(signature, signatureSize);

    bool match;
    status = mObj->mDrm->verify(*iter, keyIdVec, messageVec, signatureVec, match);
    if (status == OK) {
        return match ? AMEDIA_OK : AMEDIA_DRM_VERIFY_FAILED;
    }
    return translateStatus(status);
}

} // extern "C"
