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
#define LOG_TAG "ClearKeyCasPlugin"

#include "ClearKeyFetcher.h"
#include "ecm.h"
#include "ClearKeyLicenseFetcher.h"
#include "ClearKeyCasPlugin.h"
#include "ClearKeySessionLibrary.h"
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/Log.h>

android::CasFactory* createCasFactory() {
    return new android::clearkeycas::ClearKeyCasFactory();
}

android::DescramblerFactory *createDescramblerFactory()
{
    return new android::clearkeycas::ClearKeyDescramblerFactory();
}

namespace android {
namespace clearkeycas {

static const int32_t sClearKeySystemId = 0xF6D8;

bool ClearKeyCasFactory::isSystemIdSupported(int32_t CA_system_id) const {
    return CA_system_id == sClearKeySystemId;
}

status_t ClearKeyCasFactory::queryPlugins(
        std::vector<CasPluginDescriptor> *descriptors) const {
    descriptors->clear();
    descriptors->push_back({sClearKeySystemId, String8("Clear Key CAS")});
    return OK;
}

status_t ClearKeyCasFactory::createPlugin(
        int32_t CA_system_id,
        void *appData,
        CasPluginCallback callback,
        CasPlugin **plugin) {
    if (!isSystemIdSupported(CA_system_id)) {
        return BAD_VALUE;
    }

    *plugin = new ClearKeyCasPlugin(appData, callback);
    return OK;
}

status_t ClearKeyCasFactory::createPlugin(
        int32_t CA_system_id,
        void *appData,
        CasPluginCallbackExt callback,
        CasPlugin **plugin) {
    if (!isSystemIdSupported(CA_system_id)) {
        return BAD_VALUE;
    }

    *plugin = new ClearKeyCasPlugin(appData, callback);
    return OK;
}
////////////////////////////////////////////////////////////////////////////////
bool ClearKeyDescramblerFactory::isSystemIdSupported(
        int32_t CA_system_id) const {
    return CA_system_id == sClearKeySystemId;
}

status_t ClearKeyDescramblerFactory::createPlugin(
        int32_t CA_system_id, DescramblerPlugin** plugin) {
    if (!isSystemIdSupported(CA_system_id)) {
        return BAD_VALUE;
    }

    *plugin = new ClearKeyDescramblerPlugin();
    return OK;
}

///////////////////////////////////////////////////////////////////////////////
ClearKeyCasPlugin::ClearKeyCasPlugin(
        void *appData, CasPluginCallback callback)
    : mCallback(callback), mCallbackExt(NULL), mAppData(appData) {
    ALOGV("CTOR");
}

ClearKeyCasPlugin::ClearKeyCasPlugin(
        void *appData, CasPluginCallbackExt callback)
    : mCallback(NULL), mCallbackExt(callback), mAppData(appData) {
    ALOGV("CTOR");
}

ClearKeyCasPlugin::~ClearKeyCasPlugin() {
    ALOGV("DTOR");
    ClearKeySessionLibrary::get()->destroyPlugin(this);
}

status_t ClearKeyCasPlugin::setPrivateData(const CasData &/*data*/) {
    ALOGV("setPrivateData");

    return OK;
}

static String8 sessionIdToString(const std::vector<uint8_t> &array) {
    String8 result;
    for (size_t i = 0; i < array.size(); i++) {
        result.appendFormat("%02x ", array[i]);
    }
    if (result.isEmpty()) {
        result.append("(null)");
    }
    return result;
}

status_t ClearKeyCasPlugin::openSession(CasSessionId* sessionId) {
    ALOGV("openSession");

    return ClearKeySessionLibrary::get()->addSession(this, sessionId);
}

status_t ClearKeyCasPlugin::closeSession(const CasSessionId &sessionId) {
    ALOGV("closeSession: sessionId=%s", sessionIdToString(sessionId).string());
    std::shared_ptr<ClearKeyCasSession> session =
            ClearKeySessionLibrary::get()->findSession(sessionId);
    if (session.get() == nullptr) {
        return ERROR_CAS_SESSION_NOT_OPENED;
    }

    ClearKeySessionLibrary::get()->destroySession(sessionId);
    return OK;
}

status_t ClearKeyCasPlugin::setSessionPrivateData(
        const CasSessionId &sessionId, const CasData & /*data*/) {
    ALOGV("setSessionPrivateData: sessionId=%s",
            sessionIdToString(sessionId).string());
    std::shared_ptr<ClearKeyCasSession> session =
            ClearKeySessionLibrary::get()->findSession(sessionId);
    if (session.get() == nullptr) {
        return ERROR_CAS_SESSION_NOT_OPENED;
    }
    return OK;
}

status_t ClearKeyCasPlugin::processEcm(
        const CasSessionId &sessionId, const CasEcm& ecm) {
    ALOGV("processEcm: sessionId=%s", sessionIdToString(sessionId).string());
    std::shared_ptr<ClearKeyCasSession> session =
            ClearKeySessionLibrary::get()->findSession(sessionId);
    if (session.get() == nullptr) {
        return ERROR_CAS_SESSION_NOT_OPENED;
    }

    Mutex::Autolock lock(mKeyFetcherLock);

    return session->updateECM(mKeyFetcher.get(), (void*)ecm.data(), ecm.size());
}

status_t ClearKeyCasPlugin::processEmm(const CasEmm& /*emm*/) {
    ALOGV("processEmm");
    Mutex::Autolock lock(mKeyFetcherLock);

    return OK;
}

status_t ClearKeyCasPlugin::sendEvent(
        int32_t event, int32_t arg, const CasData &eventData) {
    ALOGV("sendEvent: event=%d, arg=%d", event, arg);
    // Echo the received event to the callback.
    // Clear key plugin doesn't use any event, echo'ing for testing only.
    if (mCallback != NULL) {
        mCallback((void*)mAppData, event, arg, (uint8_t*)eventData.data(),
                    eventData.size());
    } else if (mCallbackExt != NULL) {
        mCallbackExt((void*)mAppData, event, arg, (uint8_t*)eventData.data(),
                    eventData.size(), NULL);
    }
    return OK;
}

status_t ClearKeyCasPlugin::sendSessionEvent(
        const CasSessionId &sessionId, int32_t event,
        int arg, const CasData &eventData) {
    ALOGV("sendSessionEvent: sessionId=%s, event=%d, arg=%d",
          sessionIdToString(sessionId).string(), event, arg);
    // Echo the received event to the callback.
    // Clear key plugin doesn't use any event, echo'ing for testing only.
    if (mCallbackExt != NULL) {
        mCallbackExt((void*)mAppData, event, arg, (uint8_t*)eventData.data(),
                    eventData.size(), &sessionId);
    }

    return OK;
}

status_t ClearKeyCasPlugin::provision(const String8 &str) {
    ALOGV("provision: provisionString=%s", str.string());
    Mutex::Autolock lock(mKeyFetcherLock);

    std::unique_ptr<ClearKeyLicenseFetcher> license_fetcher;
    license_fetcher.reset(new ClearKeyLicenseFetcher());
    status_t err = license_fetcher->Init(str.string());
    if (err != OK) {
        ALOGE("provision: failed to init ClearKeyLicenseFetcher (err=%d)", err);
        return err;
    }

    std::unique_ptr<ClearKeyFetcher> key_fetcher;
    key_fetcher.reset(new ClearKeyFetcher(std::move(license_fetcher)));
    err = key_fetcher->Init();
    if (err != OK) {
        ALOGE("provision: failed to init ClearKeyFetcher (err=%d)", err);
        return err;
    }

    ALOGV("provision: using ClearKeyFetcher");
    mKeyFetcher = std::move(key_fetcher);

    return OK;
}

status_t ClearKeyCasPlugin::refreshEntitlements(
        int32_t refreshType, const CasData &/*refreshData*/) {
    ALOGV("refreshEntitlements: refreshType=%d", refreshType);
    Mutex::Autolock lock(mKeyFetcherLock);

    return OK;
}

///////////////////////////////////////////////////////////////////////

// AES-128 CBC-CTS decrypt optimized for Transport Packets. |key| is the AES
// key (odd key or even key), |length| is the data size, and |buffer| is the
// ciphertext to be decrypted in place.
status_t TpBlockCtsDecrypt(const AES_KEY& key, size_t length, char* buffer) {
    CHECK(buffer);

    // Invariant: Packet must be at least 16 bytes.
    CHECK(length >= AES_BLOCK_SIZE);

    // OpenSSL uses unsigned char.
    unsigned char* data = reinterpret_cast<unsigned char*>(buffer);

    // Start with zero-filled initialization vector.
    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0, AES_BLOCK_SIZE);

    // Size of partial last block handled via CTS.
    int cts_byte_count = length % AES_BLOCK_SIZE;

    // If there no is no partial last block, then process using normal CBC.
    if (cts_byte_count == 0) {
        AES_cbc_encrypt(data, data, length, &key, iv, 0);
        return OK;
    }

    // Cipher text stealing (CTS) - Schneier Figure 9.5 p 196.
    // In CTS mode, the last two blocks have been swapped. Block[n-1] is really
    // the original block[n] combined with the low-order bytes of the original
    // block[n-1], while block[n] is the high-order bytes of the original
    // block[n-1] padded with zeros.

    // Block[0] - block[n-2] are handled with normal CBC.
    int cbc_byte_count = length - cts_byte_count - AES_BLOCK_SIZE;
    if (cbc_byte_count > 0) {
        AES_cbc_encrypt(data, data, cbc_byte_count, &key, iv, 0);
        // |data| points to block[n-1].
        data += cbc_byte_count;
    }

    // Save block[n] to use as IV when decrypting block[n-1].
    unsigned char block_n[AES_BLOCK_SIZE];
    memset(block_n, 0, AES_BLOCK_SIZE);
    memcpy(block_n, data + AES_BLOCK_SIZE, cts_byte_count);

    // Decrypt block[n-1] using block[n] as IV, consistent with the original
    // block order.
    AES_cbc_encrypt(data, data, AES_BLOCK_SIZE, &key, block_n, 0);

    // Return the stolen ciphertext: swap the high-order bytes of block[n]
    // and block[n-1].
    for (int i = 0; i < cts_byte_count; i++) {
        unsigned char temp = *(data + i);
        *(data + i) = *(data + AES_BLOCK_SIZE + i);
        *(data + AES_BLOCK_SIZE + i) = temp;
    }

    // Decrypt block[n-1] using previous IV.
    AES_cbc_encrypt(data, data, AES_BLOCK_SIZE, &key, iv, 0);
    return OK;
}

// PES header and ECM stream header layout
//
// processECM() receives the data_byte portion from the transport packet.
// Below is the layout of the first 16 bytes of the ECM PES packet. Here
// we don't parse them, we skip them and go to the ECM container directly.
// The layout is included here only for reference.
//
// 0-2:   0x00 00 01 = start code prefix.
// 3:     0xf0 = stream type (90 = ECM).
// 4-5:   0x00 00 = PES length (filled in later, this is the length of the
//                  PES header (16) plus the length of the ECM container).
// 6-7:   0x00 00 = ECM major version.
// 8-9:   0x00 01 = ECM minor version.
// 10-11: 0x00 00 = Crypto period ID (filled in later).
// 12-13: 0x00 00 = ECM container length (filled in later, either 84 or
// 166).
// 14-15: 0x00 00 = offset = 0.

const static size_t kEcmHeaderLength = 16;
const static size_t kUserKeyLength = 16;

status_t ClearKeyCasSession::updateECM(
        KeyFetcher *keyFetcher, void *ecm, size_t size) {
    if (keyFetcher == nullptr) {
        return ERROR_CAS_NOT_PROVISIONED;
    }

    if (size < kEcmHeaderLength) {
        ALOGE("updateECM: invalid ecm size %zu", size);
        return BAD_VALUE;
    }

    Mutex::Autolock _lock(mKeyLock);

    if (mEcmBuffer != NULL && mEcmBuffer->capacity() == size
            && !memcmp(mEcmBuffer->base(), ecm, size)) {
        return OK;
    }

    mEcmBuffer = ABuffer::CreateAsCopy(ecm, size);
    mEcmBuffer->setRange(kEcmHeaderLength, size - kEcmHeaderLength);

    uint64_t asset_id;
    std::vector<KeyFetcher::KeyInfo> keys;
    status_t err = keyFetcher->ObtainKey(mEcmBuffer, &asset_id, &keys);
    if (err != OK) {
        ALOGE("updateECM: failed to obtain key (err=%d)", err);
        return err;
    }

    ALOGV("updateECM: %zu key(s) found", keys.size());
    for (size_t keyIndex = 0; keyIndex < keys.size(); keyIndex++) {
        String8 str;

        const sp<ABuffer>& keyBytes = keys[keyIndex].key_bytes;
        CHECK(keyBytes->size() == kUserKeyLength);

        int result = AES_set_decrypt_key(
                reinterpret_cast<const uint8_t*>(keyBytes->data()),
                AES_BLOCK_SIZE * 8, &mKeyInfo[keyIndex].contentKey);
        mKeyInfo[keyIndex].valid = (result == 0);
        if (!mKeyInfo[keyIndex].valid) {
            ALOGE("updateECM: failed to set key %zu, key_id=%d",
                    keyIndex, keys[keyIndex].key_id);
        }
    }
    return OK;
}

// Decryption of a set of sub-samples
ssize_t ClearKeyCasSession::decrypt(
        bool secure, DescramblerPlugin::ScramblingControl scramblingControl,
        size_t numSubSamples, const DescramblerPlugin::SubSample *subSamples,
        const void *srcPtr, void *dstPtr, AString * /* errorDetailMsg */) {
    if (secure) {
        return ERROR_CAS_CANNOT_HANDLE;
    }

    scramblingControl = (DescramblerPlugin::ScramblingControl)
        (scramblingControl & DescramblerPlugin::kScrambling_Mask_Key);

    AES_KEY contentKey;

    if (scramblingControl != DescramblerPlugin::kScrambling_Unscrambled) {
        // Hold lock to get the key only to avoid contention for decryption
        Mutex::Autolock _lock(mKeyLock);

        int32_t keyIndex = (scramblingControl & 1);
        if (!mKeyInfo[keyIndex].valid) {
            ALOGE("decrypt: key %d is invalid", keyIndex);
            return ERROR_CAS_DECRYPT;
        }
        contentKey = mKeyInfo[keyIndex].contentKey;
    }

    uint8_t *src = (uint8_t*)srcPtr;
    uint8_t *dst = (uint8_t*)dstPtr;

    for (size_t i = 0; i < numSubSamples; i++) {
        size_t numBytesinSubSample = subSamples[i].mNumBytesOfClearData
                + subSamples[i].mNumBytesOfEncryptedData;
        if (src != dst) {
            memcpy(dst, src, numBytesinSubSample);
        }
        status_t err = OK;
        // Don't decrypt if len < AES_BLOCK_SIZE.
        // The last chunk shorter than AES_BLOCK_SIZE is not encrypted.
        if (scramblingControl != DescramblerPlugin::kScrambling_Unscrambled
                && subSamples[i].mNumBytesOfEncryptedData >= AES_BLOCK_SIZE) {
            err = decryptPayload(
                    contentKey,
                    numBytesinSubSample,
                    subSamples[i].mNumBytesOfClearData,
                    (char *)dst);
        }

        dst += numBytesinSubSample;
        src += numBytesinSubSample;
    }
    return dst - (uint8_t *)dstPtr;
}

// Decryption of a TS payload
status_t ClearKeyCasSession::decryptPayload(
        const AES_KEY& key, size_t length, size_t offset, char* buffer) const {
    CHECK(buffer);

    // Invariant: only call decryptPayload with TS packets with at least 16
    // bytes of payload (AES_BLOCK_SIZE).

    CHECK(length >= offset + AES_BLOCK_SIZE);

    return TpBlockCtsDecrypt(key, length - offset, buffer + offset);
}

///////////////////////////////////////////////////////////////////////////
#undef LOG_TAG
#define LOG_TAG "ClearKeyDescramblerPlugin"

bool ClearKeyDescramblerPlugin::requiresSecureDecoderComponent(
        const char *mime) const {
    ALOGV("requiresSecureDecoderComponent: mime=%s", mime);
    return false;
}

status_t ClearKeyDescramblerPlugin::setMediaCasSession(
        const CasSessionId &sessionId) {
    ALOGV("setMediaCasSession: sessionId=%s", sessionIdToString(sessionId).string());

    std::shared_ptr<ClearKeyCasSession> session =
            ClearKeySessionLibrary::get()->findSession(sessionId);

    if (session.get() == nullptr) {
        ALOGE("ClearKeyDescramblerPlugin: session not found");
        return ERROR_CAS_SESSION_NOT_OPENED;
    }

    std::atomic_store(&mCASSession, session);
    return OK;
}

ssize_t ClearKeyDescramblerPlugin::descramble(
        bool secure,
        ScramblingControl scramblingControl,
        size_t numSubSamples,
        const SubSample *subSamples,
        const void *srcPtr,
        int32_t srcOffset,
        void *dstPtr,
        int32_t dstOffset,
        AString *errorDetailMsg) {

    ALOGV("descramble: secure=%d, sctrl=%d, subSamples=%s, "
            "srcPtr=%p, dstPtr=%p, srcOffset=%d, dstOffset=%d",
          (int)secure, (int)scramblingControl,
          subSamplesToString(subSamples, numSubSamples).string(),
          srcPtr, dstPtr, srcOffset, dstOffset);

    std::shared_ptr<ClearKeyCasSession> session = std::atomic_load(&mCASSession);

    if (session.get() == nullptr) {
        ALOGE("Uninitialized CAS session!");
        return ERROR_CAS_DECRYPT_UNIT_NOT_INITIALIZED;
    }

    return session->decrypt(
            secure, scramblingControl,
            numSubSamples, subSamples,
            (uint8_t*)srcPtr + srcOffset,
            dstPtr == NULL ? NULL : ((uint8_t*)dstPtr + dstOffset),
            errorDetailMsg);
}

// Conversion utilities
String8 ClearKeyDescramblerPlugin::arrayToString(
        uint8_t const *array, size_t len) const
{
    String8 result("{ ");
    for (size_t i = 0; i < len; i++) {
        result.appendFormat("0x%02x ", array[i]);
    }
    result += "}";
    return result;
}

String8 ClearKeyDescramblerPlugin::subSamplesToString(
        SubSample const *subSamples, size_t numSubSamples) const
{
    String8 result;
    for (size_t i = 0; i < numSubSamples; i++) {
        result.appendFormat("[%zu] {clear:%u, encrypted:%u} ", i,
                            subSamples[i].mNumBytesOfClearData,
                            subSamples[i].mNumBytesOfEncryptedData);
    }
    return result;
}

} // namespace clearkeycas
} // namespace android
