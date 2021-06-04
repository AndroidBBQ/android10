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

#ifndef DRM_HAL_H_

#define DRM_HAL_H_

#include <android/hardware/drm/1.0/IDrmFactory.h>
#include <android/hardware/drm/1.0/IDrmPlugin.h>
#include <android/hardware/drm/1.1/IDrmFactory.h>
#include <android/hardware/drm/1.1/IDrmPlugin.h>
#include <android/hardware/drm/1.2/IDrmFactory.h>
#include <android/hardware/drm/1.2/IDrmPlugin.h>
#include <android/hardware/drm/1.2/IDrmPluginListener.h>

#include <media/MediaAnalyticsItem.h>
#include <mediadrm/DrmMetrics.h>
#include <mediadrm/IDrm.h>
#include <mediadrm/IDrmClient.h>
#include <utils/threads.h>

namespace drm = ::android::hardware::drm;
using drm::V1_0::EventType;
using drm::V1_0::IDrmFactory;
using drm::V1_0::IDrmPlugin;
using drm::V1_0::IDrmPluginListener;
using drm::V1_1::SecurityLevel;
using drm::V1_2::KeyStatus;
using drm::V1_2::OfflineLicenseState;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

typedef drm::V1_2::IDrmPluginListener IDrmPluginListener_V1_2;
typedef drm::V1_0::KeyStatus KeyStatus_V1_0;

namespace android {

struct DrmSessionClientInterface;

inline bool operator==(const Vector<uint8_t> &l, const Vector<uint8_t> &r) {
    if (l.size() != r.size()) return false;
    return memcmp(l.array(), r.array(), l.size()) == 0;
}

struct DrmHal : public BnDrm,
                public IBinder::DeathRecipient,
                public IDrmPluginListener_V1_2 {
    DrmHal();
    virtual ~DrmHal();

    virtual status_t initCheck() const;

    virtual status_t isCryptoSchemeSupported(const uint8_t uuid[16],
                                             const String8& mimeType,
                                             DrmPlugin::SecurityLevel level,
                                             bool *isSupported);

    virtual status_t createPlugin(const uint8_t uuid[16],
                                  const String8 &appPackageName);

    virtual status_t destroyPlugin();

    virtual status_t openSession(DrmPlugin::SecurityLevel level,
            Vector<uint8_t> &sessionId);

    virtual status_t closeSession(Vector<uint8_t> const &sessionId);

    virtual status_t
        getKeyRequest(Vector<uint8_t> const &sessionId,
                      Vector<uint8_t> const &initData,
                      String8 const &mimeType, DrmPlugin::KeyType keyType,
                      KeyedVector<String8, String8> const &optionalParameters,
                      Vector<uint8_t> &request, String8 &defaultUrl,
                      DrmPlugin::KeyRequestType *keyRequestType);

    virtual status_t provideKeyResponse(Vector<uint8_t> const &sessionId,
                                        Vector<uint8_t> const &response,
                                        Vector<uint8_t> &keySetId);

    virtual status_t removeKeys(Vector<uint8_t> const &keySetId);

    virtual status_t restoreKeys(Vector<uint8_t> const &sessionId,
                                 Vector<uint8_t> const &keySetId);

    virtual status_t queryKeyStatus(Vector<uint8_t> const &sessionId,
                                    KeyedVector<String8, String8> &infoMap) const;

    virtual status_t getProvisionRequest(String8 const &certType,
                                         String8 const &certAuthority,
                                         Vector<uint8_t> &request,
                                         String8 &defaulUrl);

    virtual status_t provideProvisionResponse(Vector<uint8_t> const &response,
                                              Vector<uint8_t> &certificate,
                                              Vector<uint8_t> &wrappedKey);

    virtual status_t getSecureStops(List<Vector<uint8_t>> &secureStops);
    virtual status_t getSecureStopIds(List<Vector<uint8_t>> &secureStopIds);
    virtual status_t getSecureStop(Vector<uint8_t> const &ssid, Vector<uint8_t> &secureStop);

    virtual status_t releaseSecureStops(Vector<uint8_t> const &ssRelease);
    virtual status_t removeSecureStop(Vector<uint8_t> const &ssid);
    virtual status_t removeAllSecureStops();

    virtual status_t getHdcpLevels(DrmPlugin::HdcpLevel *connectedLevel,
            DrmPlugin::HdcpLevel *maxLevel) const;
    virtual status_t getNumberOfSessions(uint32_t *currentSessions,
            uint32_t *maxSessions) const;
    virtual status_t getSecurityLevel(Vector<uint8_t> const &sessionId,
            DrmPlugin::SecurityLevel *level) const;

    virtual status_t getOfflineLicenseKeySetIds(List<Vector<uint8_t>> &keySetIds) const;
    virtual status_t removeOfflineLicense(Vector<uint8_t> const &keySetId);
    virtual status_t getOfflineLicenseState(Vector<uint8_t> const &keySetId,
            DrmPlugin::OfflineLicenseState *licenseState) const;

    virtual status_t getPropertyString(String8 const &name, String8 &value ) const;
    virtual status_t getPropertyByteArray(String8 const &name,
                                          Vector<uint8_t> &value ) const;
    virtual status_t setPropertyString(String8 const &name, String8 const &value ) const;
    virtual status_t setPropertyByteArray(String8 const &name,
                                          Vector<uint8_t> const &value ) const;
    virtual status_t getMetrics(os::PersistableBundle *metrics);

    virtual status_t setCipherAlgorithm(Vector<uint8_t> const &sessionId,
                                        String8 const &algorithm);

    virtual status_t setMacAlgorithm(Vector<uint8_t> const &sessionId,
                                     String8 const &algorithm);

    virtual status_t encrypt(Vector<uint8_t> const &sessionId,
                             Vector<uint8_t> const &keyId,
                             Vector<uint8_t> const &input,
                             Vector<uint8_t> const &iv,
                             Vector<uint8_t> &output);

    virtual status_t decrypt(Vector<uint8_t> const &sessionId,
                             Vector<uint8_t> const &keyId,
                             Vector<uint8_t> const &input,
                             Vector<uint8_t> const &iv,
                             Vector<uint8_t> &output);

    virtual status_t sign(Vector<uint8_t> const &sessionId,
                          Vector<uint8_t> const &keyId,
                          Vector<uint8_t> const &message,
                          Vector<uint8_t> &signature);

    virtual status_t verify(Vector<uint8_t> const &sessionId,
                            Vector<uint8_t> const &keyId,
                            Vector<uint8_t> const &message,
                            Vector<uint8_t> const &signature,
                            bool &match);

    virtual status_t signRSA(Vector<uint8_t> const &sessionId,
                             String8 const &algorithm,
                             Vector<uint8_t> const &message,
                             Vector<uint8_t> const &wrappedKey,
                             Vector<uint8_t> &signature);

    virtual status_t setListener(const sp<IDrmClient>& listener);

    // Methods of IDrmPluginListener
    Return<void> sendEvent(EventType eventType,
            const hidl_vec<uint8_t>& sessionId, const hidl_vec<uint8_t>& data);

    Return<void> sendExpirationUpdate(const hidl_vec<uint8_t>& sessionId,
            int64_t expiryTimeInMS);

    Return<void> sendKeysChange(const hidl_vec<uint8_t>& sessionId,
            const hidl_vec<KeyStatus_V1_0>& keyStatusList, bool hasNewUsableKey);

    Return<void> sendKeysChange_1_2(const hidl_vec<uint8_t>& sessionId,
            const hidl_vec<KeyStatus>& keyStatusList, bool hasNewUsableKey);

    Return<void> sendSessionLostState(const hidl_vec<uint8_t>& sessionId);

    virtual void binderDied(const wp<IBinder> &the_late_who);

private:
    static Mutex mLock;

    sp<DrmSessionClientInterface> mDrmSessionClient;

    sp<IDrmClient> mListener;
    mutable Mutex mEventLock;
    mutable Mutex mNotifyLock;

    const Vector<sp<IDrmFactory>> mFactories;
    sp<IDrmPlugin> mPlugin;
    sp<drm::V1_1::IDrmPlugin> mPluginV1_1;
    sp<drm::V1_2::IDrmPlugin> mPluginV1_2;
    String8 mAppPackageName;

    // Mutable to allow modification within GetPropertyByteArray.
    mutable MediaDrmMetrics mMetrics;

    Vector<Vector<uint8_t>> mOpenSessions;
    void closeOpenSessions();
    void cleanup();

    /**
     * mInitCheck is:
     *   NO_INIT if a plugin hasn't been created yet
     *   ERROR_UNSUPPORTED if a plugin can't be created for the uuid
     *   OK after a plugin has been created and mPlugin is valid
     */
    status_t mInitCheck;

    Vector<sp<IDrmFactory>> makeDrmFactories();
    sp<IDrmPlugin> makeDrmPlugin(const sp<IDrmFactory>& factory,
            const uint8_t uuid[16], const String8& appPackageName);

    void writeByteArray(Parcel &obj, const hidl_vec<uint8_t>& array);

    void reportPluginMetrics() const;
    void reportFrameworkMetrics() const;
    status_t getPropertyStringInternal(String8 const &name, String8 &value) const;
    status_t getPropertyByteArrayInternal(String8 const &name,
                                          Vector<uint8_t> &value) const;
    status_t matchMimeTypeAndSecurityLevel(const sp<IDrmFactory> &factory,
                                           const uint8_t uuid[16],
                                           const String8 &mimeType,
                                           DrmPlugin::SecurityLevel level,
                                           bool *isSupported);

    DISALLOW_EVIL_CONSTRUCTORS(DrmHal);
};

}  // namespace android

#endif  // DRM_HAL_H_
