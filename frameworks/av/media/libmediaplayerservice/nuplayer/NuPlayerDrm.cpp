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
#define LOG_TAG "NuPlayerDrm"

#include "NuPlayerDrm.h"

#include <binder/IServiceManager.h>
#include <mediadrm/IMediaDrmService.h>
#include <utils/Log.h>


namespace android {

// static helpers - internal

sp<IDrm> NuPlayerDrm::CreateDrm(status_t *pstatus)
{
    status_t &status = *pstatus;
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("media.drm"));
    ALOGV("CreateDrm binder %p", (binder != NULL ? binder.get() : 0));

    sp<IMediaDrmService> service = interface_cast<IMediaDrmService>(binder);
    if (service == NULL) {
        ALOGE("CreateDrm failed at IMediaDrmService");
        return NULL;
    }

    sp<IDrm> drm = service->makeDrm();
    if (drm == NULL) {
        ALOGE("CreateDrm failed at makeDrm");
        return NULL;
    }

    // this is before plugin creation so NO_INIT is fine
    status = drm->initCheck();
    if (status != OK && status != NO_INIT) {
        ALOGE("CreateDrm failed drm->initCheck(): %d", status);
        return NULL;
    }
    return drm;
}

sp<ICrypto> NuPlayerDrm::createCrypto(status_t *pstatus)
{
    status_t &status = *pstatus;
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("media.drm"));

    sp<IMediaDrmService> service = interface_cast<IMediaDrmService>(binder);
    if (service == NULL) {
        status = UNKNOWN_ERROR;
        ALOGE("CreateCrypto failed at IMediaDrmService");
        return NULL;
    }

    sp<ICrypto> crypto = service->makeCrypto();
    if (crypto == NULL) {
        status = UNKNOWN_ERROR;
        ALOGE("createCrypto failed");
        return NULL;
    }

    // this is before plugin creation so NO_INIT is fine
    status = crypto->initCheck();
    if (status != OK && status != NO_INIT) {
        ALOGE("createCrypto failed crypto->initCheck(): %d", status);
        return NULL;
    }

    return crypto;
}

Vector<DrmUUID> NuPlayerDrm::parsePSSH(const void *pssh, size_t psshsize)
{
    Vector<DrmUUID> drmSchemes, empty;
    const int DATALEN_SIZE = 4;

    // the format of the buffer is 1 or more of:
    //    {
    //        16 byte uuid
    //        4 byte data length N
    //        N bytes of data
    //    }
    // Determine the number of entries in the source data.
    // Since we got the data from stagefright, we trust it is valid and properly formatted.

    const uint8_t *data = (const uint8_t*)pssh;
    size_t len = psshsize;
    size_t numentries = 0;
    while (len > 0) {
        if (len < DrmUUID::UUID_SIZE) {
            ALOGE("ParsePSSH: invalid PSSH data");
            return empty;
        }

        const uint8_t *uuidPtr = data;

        // skip uuid
        data += DrmUUID::UUID_SIZE;
        len -= DrmUUID::UUID_SIZE;

        // get data length
        if (len < DATALEN_SIZE) {
            ALOGE("ParsePSSH: invalid PSSH data");
            return empty;
        }

        uint32_t datalen = *((uint32_t*)data);
        data += DATALEN_SIZE;
        len -= DATALEN_SIZE;

        if (len < datalen) {
            ALOGE("ParsePSSH: invalid PSSH data");
            return empty;
        }

        // skip the data
        data += datalen;
        len -= datalen;

        DrmUUID _uuid(uuidPtr);
        drmSchemes.add(_uuid);

        ALOGV("ParsePSSH[%zu]: %s: %s", numentries,
                _uuid.toHexString().string(),
                DrmUUID::arrayToHex(data, datalen).string()
             );

        numentries++;
    }

    return drmSchemes;
}

Vector<DrmUUID> NuPlayerDrm::getSupportedDrmSchemes(const void *pssh, size_t psshsize)
{
    Vector<DrmUUID> psshDRMs = parsePSSH(pssh, psshsize);

    Vector<DrmUUID> supportedDRMs;
     // temporary DRM object for crypto Scheme enquiry (without creating a plugin)
    status_t status = OK;
    sp<IDrm> drm = CreateDrm(&status);
    if (drm != NULL) {
        for (size_t i = 0; i < psshDRMs.size(); i++) {
            DrmUUID uuid = psshDRMs[i];
            bool isSupported = false;
            status = drm->isCryptoSchemeSupported(uuid.ptr(), String8(),
                    DrmPlugin::kSecurityLevelUnknown, &isSupported);
            if (status == OK && isSupported) {
                supportedDRMs.add(uuid);
            }
        }

        drm.clear();
    } else {
        ALOGE("getSupportedDrmSchemes: Can't create Drm obj: %d", status);
    }

    ALOGV("getSupportedDrmSchemes: psshDRMs: %zu supportedDRMs: %zu",
            psshDRMs.size(), supportedDRMs.size());

    return supportedDRMs;
}

// static helpers - public

sp<ICrypto> NuPlayerDrm::createCryptoAndPlugin(const uint8_t uuid[16],
        const Vector<uint8_t> &drmSessionId, status_t &status)
{
    // Extra check
    if (drmSessionId.isEmpty()) {
        status = INVALID_OPERATION;
        ALOGE("createCryptoAndPlugin: Failed. Empty drmSessionId. status: %d", status);
        return NULL;
    }

    status = OK;
    sp<ICrypto> crypto = createCrypto(&status);
    if (crypto == NULL) {
        ALOGE("createCryptoAndPlugin: createCrypto failed. status: %d", status);
        return NULL;
    }
    ALOGV("createCryptoAndPlugin: createCrypto succeeded");

    status = crypto->createPlugin(uuid, drmSessionId.array(), drmSessionId.size());
    if (status != OK) {
        ALOGE("createCryptoAndPlugin: createCryptoPlugin failed. status: %d", status);
        // crypto will clean itself when leaving the current scope
        return NULL;
    }

    return crypto;
}

// Parcel has only private copy constructor so passing it in rather than returning
void NuPlayerDrm::retrieveDrmInfo(const void *pssh, size_t psshsize, Parcel *parcel)
{
    // 1) PSSH bytes
    parcel->writeUint32(psshsize);
    parcel->writeByteArray(psshsize, (const uint8_t*)pssh);

    ALOGV("retrieveDrmInfo: MEDIA_DRM_INFO  PSSH: size: %zu %s", psshsize,
            DrmUUID::arrayToHex((uint8_t*)pssh, psshsize).string());

    // 2) supportedDRMs
    Vector<DrmUUID> supportedDRMs = getSupportedDrmSchemes(pssh, psshsize);
    parcel->writeUint32(supportedDRMs.size());
    for (size_t i = 0; i < supportedDRMs.size(); i++) {
        DrmUUID uuid = supportedDRMs[i];
        parcel->writeByteArray(DrmUUID::UUID_SIZE, uuid.ptr());

        ALOGV("retrieveDrmInfo: MEDIA_DRM_INFO  supportedScheme[%zu] %s", i,
                uuid.toHexString().string());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Helpers for NuPlayerDecoder
////////////////////////////////////////////////////////////////////////////////////////////

NuPlayerDrm::CryptoInfo *NuPlayerDrm::makeCryptoInfo(
        int numSubSamples,
        uint8_t key[kBlockSize],
        uint8_t iv[kBlockSize],
        CryptoPlugin::Mode mode,
        size_t *clearbytes,
        size_t *encryptedbytes)
{
    // size needed to store all the crypto data
    size_t cryptosize;
    // sizeof(CryptoInfo) + sizeof(CryptoPlugin::SubSample) * numSubSamples;
    if (__builtin_mul_overflow(sizeof(CryptoPlugin::SubSample), numSubSamples, &cryptosize) ||
            __builtin_add_overflow(cryptosize, sizeof(CryptoInfo), &cryptosize)) {
        ALOGE("crypto size overflow");
        return NULL;
    }

    CryptoInfo *ret = (CryptoInfo*) malloc(cryptosize);
    if (ret == NULL) {
        ALOGE("couldn't allocate %zu bytes", cryptosize);
        return NULL;
    }
    ret->numSubSamples = numSubSamples;
    memcpy(ret->key, key, kBlockSize);
    memcpy(ret->iv, iv, kBlockSize);
    ret->mode = mode;
    ret->pattern.mEncryptBlocks = 0;
    ret->pattern.mSkipBlocks = 0;
    ret->subSamples = (CryptoPlugin::SubSample*)(ret + 1);
    CryptoPlugin::SubSample *subSamples = ret->subSamples;

    for (int i = 0; i < numSubSamples; i++) {
        subSamples[i].mNumBytesOfClearData = (clearbytes == NULL) ? 0 : clearbytes[i];
        subSamples[i].mNumBytesOfEncryptedData = (encryptedbytes == NULL) ?
                                                  0 :
                                                  encryptedbytes[i];
    }

    return ret;
}

NuPlayerDrm::CryptoInfo *NuPlayerDrm::getSampleCryptoInfo(MetaDataBase &meta)
{
    uint32_t type;
    const void *crypteddata;
    size_t cryptedsize;

    if (!meta.findData(kKeyEncryptedSizes, &type, &crypteddata, &cryptedsize)) {
        return NULL;
    }
    size_t numSubSamples = cryptedsize / sizeof(size_t);

    if (numSubSamples <= 0) {
        ALOGE("getSampleCryptoInfo INVALID numSubSamples: %zu", numSubSamples);
        return NULL;
    }

    const void *cleardata;
    size_t clearsize;
    if (meta.findData(kKeyPlainSizes, &type, &cleardata, &clearsize)) {
        if (clearsize != cryptedsize) {
            // The two must be of the same length.
            ALOGE("getSampleCryptoInfo mismatch cryptedsize: %zu != clearsize: %zu",
                    cryptedsize, clearsize);
            return NULL;
        }
    }

    const void *key;
    size_t keysize;
    if (meta.findData(kKeyCryptoKey, &type, &key, &keysize)) {
        if (keysize != kBlockSize) {
            ALOGE("getSampleCryptoInfo Keys must be %d bytes in length: %zu",
                    kBlockSize, keysize);
            // Keys must be 16 bytes in length.
            return NULL;
        }
    }

    const void *iv;
    size_t ivsize;
    if (meta.findData(kKeyCryptoIV, &type, &iv, &ivsize)) {
        if (ivsize != kBlockSize) {
            ALOGE("getSampleCryptoInfo IV must be %d bytes in length: %zu",
                    kBlockSize, ivsize);
            // IVs must be 16 bytes in length.
            return NULL;
        }
    }

    int32_t mode;
    if (!meta.findInt32(kKeyCryptoMode, &mode)) {
        mode = CryptoPlugin::kMode_AES_CTR;
    }

    return makeCryptoInfo(numSubSamples,
            (uint8_t*) key,
            (uint8_t*) iv,
            (CryptoPlugin::Mode)mode,
            (size_t*) cleardata,
            (size_t*) crypteddata);
}

}   // namespace android

