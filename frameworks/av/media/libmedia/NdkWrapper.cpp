/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "NdkWrapper"

#include <media/NdkWrapper.h>

#include <android/native_window.h>
#include <log/log.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaCrypto.h>
#include <media/NdkMediaDrm.h>
#include <media/NdkMediaFormat.h>
#include <media/NdkMediaExtractor.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <utils/Errors.h>

#include "NdkMediaDataSourceCallbacksPriv.h"

namespace android {

static const size_t kAESBlockSize = 16;  // AES_BLOCK_SIZE

static const char *AMediaFormatKeyGroupInt32[] = {
    AMEDIAFORMAT_KEY_AAC_DRC_ATTENUATION_FACTOR,
    AMEDIAFORMAT_KEY_AAC_DRC_BOOST_FACTOR,
    AMEDIAFORMAT_KEY_AAC_DRC_HEAVY_COMPRESSION,
    AMEDIAFORMAT_KEY_AAC_DRC_TARGET_REFERENCE_LEVEL,
    AMEDIAFORMAT_KEY_AAC_ENCODED_TARGET_LEVEL,
    AMEDIAFORMAT_KEY_AAC_MAX_OUTPUT_CHANNEL_COUNT,
    AMEDIAFORMAT_KEY_AAC_PROFILE,
    AMEDIAFORMAT_KEY_AAC_SBR_MODE,
    AMEDIAFORMAT_KEY_AUDIO_SESSION_ID,
    AMEDIAFORMAT_KEY_BITRATE_MODE,
    AMEDIAFORMAT_KEY_BIT_RATE,
    AMEDIAFORMAT_KEY_CAPTURE_RATE,
    AMEDIAFORMAT_KEY_CHANNEL_COUNT,
    AMEDIAFORMAT_KEY_CHANNEL_MASK,
    AMEDIAFORMAT_KEY_COLOR_FORMAT,
    AMEDIAFORMAT_KEY_COLOR_RANGE,
    AMEDIAFORMAT_KEY_COLOR_STANDARD,
    AMEDIAFORMAT_KEY_COLOR_TRANSFER,
    AMEDIAFORMAT_KEY_COMPLEXITY,
    AMEDIAFORMAT_KEY_CREATE_INPUT_SURFACE_SUSPENDED,
    AMEDIAFORMAT_KEY_CRYPTO_DEFAULT_IV_SIZE,
    AMEDIAFORMAT_KEY_CRYPTO_ENCRYPTED_BYTE_BLOCK,
    AMEDIAFORMAT_KEY_CRYPTO_MODE,
    AMEDIAFORMAT_KEY_CRYPTO_SKIP_BYTE_BLOCK,
    AMEDIAFORMAT_KEY_FLAC_COMPRESSION_LEVEL,
    AMEDIAFORMAT_KEY_GRID_COLUMNS,
    AMEDIAFORMAT_KEY_GRID_ROWS,
    AMEDIAFORMAT_KEY_HAPTIC_CHANNEL_COUNT,
    AMEDIAFORMAT_KEY_HEIGHT,
    AMEDIAFORMAT_KEY_INTRA_REFRESH_PERIOD,
    AMEDIAFORMAT_KEY_IS_ADTS,
    AMEDIAFORMAT_KEY_IS_AUTOSELECT,
    AMEDIAFORMAT_KEY_IS_DEFAULT,
    AMEDIAFORMAT_KEY_IS_FORCED_SUBTITLE,
    AMEDIAFORMAT_KEY_LATENCY,
    AMEDIAFORMAT_KEY_LEVEL,
    AMEDIAFORMAT_KEY_MAX_HEIGHT,
    AMEDIAFORMAT_KEY_MAX_INPUT_SIZE,
    AMEDIAFORMAT_KEY_MAX_WIDTH,
    AMEDIAFORMAT_KEY_PCM_ENCODING,
    AMEDIAFORMAT_KEY_PRIORITY,
    AMEDIAFORMAT_KEY_PROFILE,
    AMEDIAFORMAT_KEY_PUSH_BLANK_BUFFERS_ON_STOP,
    AMEDIAFORMAT_KEY_ROTATION,
    AMEDIAFORMAT_KEY_SAMPLE_RATE,
    AMEDIAFORMAT_KEY_SLICE_HEIGHT,
    AMEDIAFORMAT_KEY_STRIDE,
    AMEDIAFORMAT_KEY_TRACK_ID,
    AMEDIAFORMAT_KEY_WIDTH,
    AMEDIAFORMAT_KEY_DISPLAY_HEIGHT,
    AMEDIAFORMAT_KEY_DISPLAY_WIDTH,
    AMEDIAFORMAT_KEY_TEMPORAL_LAYER_ID,
    AMEDIAFORMAT_KEY_TILE_HEIGHT,
    AMEDIAFORMAT_KEY_TILE_WIDTH,
    AMEDIAFORMAT_KEY_TRACK_INDEX,
};

static const char *AMediaFormatKeyGroupInt64[] = {
    AMEDIAFORMAT_KEY_DURATION,
    AMEDIAFORMAT_KEY_MAX_PTS_GAP_TO_ENCODER,
    AMEDIAFORMAT_KEY_REPEAT_PREVIOUS_FRAME_AFTER,
    AMEDIAFORMAT_KEY_TIME_US,
};

static const char *AMediaFormatKeyGroupString[] = {
    AMEDIAFORMAT_KEY_LANGUAGE,
    AMEDIAFORMAT_KEY_MIME,
    AMEDIAFORMAT_KEY_TEMPORAL_LAYERING,
};

static const char *AMediaFormatKeyGroupBuffer[] = {
    AMEDIAFORMAT_KEY_CRYPTO_IV,
    AMEDIAFORMAT_KEY_CRYPTO_KEY,
    AMEDIAFORMAT_KEY_HDR_STATIC_INFO,
    AMEDIAFORMAT_KEY_SEI,
    AMEDIAFORMAT_KEY_MPEG_USER_DATA,
};

static const char *AMediaFormatKeyGroupCsd[] = {
    AMEDIAFORMAT_KEY_CSD_0,
    AMEDIAFORMAT_KEY_CSD_1,
    AMEDIAFORMAT_KEY_CSD_2,
};

static const char *AMediaFormatKeyGroupRect[] = {
    AMEDIAFORMAT_KEY_DISPLAY_CROP,
};

static const char *AMediaFormatKeyGroupFloatInt32[] = {
    AMEDIAFORMAT_KEY_FRAME_RATE,
    AMEDIAFORMAT_KEY_I_FRAME_INTERVAL,
    AMEDIAFORMAT_KEY_MAX_FPS_TO_ENCODER,
    AMEDIAFORMAT_KEY_OPERATING_RATE,
};

static status_t translateErrorCode(media_status_t err) {
    if (err == AMEDIA_OK) {
        return OK;
    } else if (err == AMEDIA_ERROR_END_OF_STREAM) {
        return ERROR_END_OF_STREAM;
    } else if (err == AMEDIA_ERROR_IO) {
        return ERROR_IO;
    } else if (err == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
        return -EAGAIN;
    }

    ALOGE("ndk error code: %d", err);
    return UNKNOWN_ERROR;
}

static int32_t translateActionCode(int32_t actionCode) {
    if (AMediaCodecActionCode_isTransient(actionCode)) {
        return ACTION_CODE_TRANSIENT;
    } else if (AMediaCodecActionCode_isRecoverable(actionCode)) {
        return ACTION_CODE_RECOVERABLE;
    }
    return ACTION_CODE_FATAL;
}

static CryptoPlugin::Mode translateToCryptoPluginMode(cryptoinfo_mode_t mode) {
    CryptoPlugin::Mode ret = CryptoPlugin::kMode_Unencrypted;
    switch (mode) {
        case AMEDIACODECRYPTOINFO_MODE_AES_CTR: {
            ret = CryptoPlugin::kMode_AES_CTR;
            break;
        }

        case AMEDIACODECRYPTOINFO_MODE_AES_WV: {
            ret = CryptoPlugin::kMode_AES_WV;
            break;
        }

        case AMEDIACODECRYPTOINFO_MODE_AES_CBC: {
            ret = CryptoPlugin::kMode_AES_CBC;
            break;
        }

        default:
            break;
    }

    return ret;
}

static cryptoinfo_mode_t translateToCryptoInfoMode(CryptoPlugin::Mode mode) {
    cryptoinfo_mode_t ret = AMEDIACODECRYPTOINFO_MODE_CLEAR;
    switch (mode) {
        case CryptoPlugin::kMode_AES_CTR: {
            ret = AMEDIACODECRYPTOINFO_MODE_AES_CTR;
            break;
        }

        case CryptoPlugin::kMode_AES_WV: {
            ret = AMEDIACODECRYPTOINFO_MODE_AES_WV;
            break;
        }

        case CryptoPlugin::kMode_AES_CBC: {
            ret = AMEDIACODECRYPTOINFO_MODE_AES_CBC;
            break;
        }

        default:
            break;
    }

    return ret;
}

//////////// AMediaFormatWrapper
// static
sp<AMediaFormatWrapper> AMediaFormatWrapper::Create(const sp<AMessage> &message) {
    sp<AMediaFormatWrapper> aMediaFormat = new AMediaFormatWrapper();

    for (size_t i = 0; i < message->countEntries(); ++i) {
        AMessage::Type valueType;
        const char *key = message->getEntryNameAt(i, &valueType);

        switch (valueType) {
            case AMessage::kTypeInt32: {
                int32_t val;
                if (!message->findInt32(key, &val)) {
                    ALOGE("AMediaFormatWrapper::Create: error at item %zu", i);
                    continue;
                }
                aMediaFormat->setInt32(key, val);
                break;
            }

            case AMessage::kTypeInt64: {
                int64_t val;
                if (!message->findInt64(key, &val)) {
                    ALOGE("AMediaFormatWrapper::Create: error at item %zu", i);
                    continue;
                }
                aMediaFormat->setInt64(key, val);
                break;
            }

            case AMessage::kTypeFloat: {
                float val;
                if (!message->findFloat(key, &val)) {
                    ALOGE("AMediaFormatWrapper::Create: error at item %zu", i);
                    continue;
                }
                aMediaFormat->setFloat(key, val);
                break;
            }

            case AMessage::kTypeDouble: {
                double val;
                if (!message->findDouble(key, &val)) {
                    ALOGE("AMediaFormatWrapper::Create: error at item %zu", i);
                    continue;
                }
                aMediaFormat->setDouble(key, val);
                break;
            }

            case AMessage::kTypeSize: {
                size_t val;
                if (!message->findSize(key, &val)) {
                    ALOGE("AMediaFormatWrapper::Create: error at item %zu", i);
                    continue;
                }
                aMediaFormat->setSize(key, val);
                break;
            }

            case AMessage::kTypeRect: {
                int32_t left, top, right, bottom;
                if (!message->findRect(key, &left, &top, &right, &bottom)) {
                    ALOGE("AMediaFormatWrapper::Create: error at item %zu", i);
                    continue;
                }
                aMediaFormat->setRect(key, left, top, right, bottom);
                break;
            }

            case AMessage::kTypeString: {
                AString val;
                if (!message->findString(key, &val)) {
                    ALOGE("AMediaFormatWrapper::Create: error at item %zu", i);
                    continue;
                }
                aMediaFormat->setString(key, val);
                break;
            }

            case AMessage::kTypeBuffer: {
                sp<ABuffer> val;
                if (!message->findBuffer(key, &val)) {
                    ALOGE("AMediaFormatWrapper::Create: error at item %zu", i);
                    continue;
                }
                aMediaFormat->setBuffer(key, val->data(), val->size());
                break;
            }

            default: {
                break;
            }
        }
    }

    return aMediaFormat;
}

AMediaFormatWrapper::AMediaFormatWrapper() {
    mAMediaFormat = AMediaFormat_new();
}

AMediaFormatWrapper::AMediaFormatWrapper(AMediaFormat *aMediaFormat)
    : mAMediaFormat(aMediaFormat) {
}

AMediaFormatWrapper::~AMediaFormatWrapper() {
    release();
}

status_t AMediaFormatWrapper::release() {
    if (mAMediaFormat != NULL) {
        media_status_t err = AMediaFormat_delete(mAMediaFormat);
        mAMediaFormat = NULL;
        return translateErrorCode(err);
    }
    return OK;
}

AMediaFormat *AMediaFormatWrapper::getAMediaFormat() const {
    return mAMediaFormat;
}

sp<AMessage> AMediaFormatWrapper::toAMessage() const {
  sp<AMessage> msg;
  writeToAMessage(msg);
  return msg;
}

void AMediaFormatWrapper::writeToAMessage(sp<AMessage> &msg) const {
    if (mAMediaFormat == NULL) {
        msg = NULL;
    }

    if (msg == NULL) {
        msg = new AMessage;
    }
    for (auto& key : AMediaFormatKeyGroupInt32) {
        int32_t val;
        if (getInt32(key, &val)) {
            msg->setInt32(key, val);
        }
    }
    for (auto& key : AMediaFormatKeyGroupInt64) {
        int64_t val;
        if (getInt64(key, &val)) {
            msg->setInt64(key, val);
        }
    }
    for (auto& key : AMediaFormatKeyGroupString) {
        AString val;
        if (getString(key, &val)) {
            msg->setString(key, val);
        }
    }
    for (auto& key : AMediaFormatKeyGroupBuffer) {
        void *data;
        size_t size;
        if (getBuffer(key, &data, &size)) {
            sp<ABuffer> buffer = ABuffer::CreateAsCopy(data, size);
            msg->setBuffer(key, buffer);
        }
    }
    for (auto& key : AMediaFormatKeyGroupCsd) {
        void *data;
        size_t size;
        if (getBuffer(key, &data, &size)) {
            sp<ABuffer> buffer = ABuffer::CreateAsCopy(data, size);
            buffer->meta()->setInt32(AMEDIAFORMAT_KEY_CSD, 1);
            buffer->meta()->setInt64(AMEDIAFORMAT_KEY_TIME_US, 0);
            msg->setBuffer(key, buffer);
        }
    }
    for (auto& key : AMediaFormatKeyGroupRect) {
        int32_t left, top, right, bottom;
        if (getRect(key, &left, &top, &right, &bottom)) {
            msg->setRect(key, left, top, right, bottom);
        }
    }
    for (auto& key : AMediaFormatKeyGroupFloatInt32) {
        float valFloat;
        if (getFloat(key, &valFloat)) {
            msg->setFloat(key, valFloat);
        } else {
            int32_t valInt32;
            if (getInt32(key, &valInt32)) {
                msg->setFloat(key, (float)valInt32);
            }
        }
    }
}

const char* AMediaFormatWrapper::toString() const {
    if (mAMediaFormat == NULL) {
        return NULL;
    }
    return AMediaFormat_toString(mAMediaFormat);
}

bool AMediaFormatWrapper::getInt32(const char *name, int32_t *out) const {
    if (mAMediaFormat == NULL) {
        return false;
    }
    return AMediaFormat_getInt32(mAMediaFormat, name, out);
}

bool AMediaFormatWrapper::getInt64(const char *name, int64_t *out) const {
    if (mAMediaFormat == NULL) {
        return false;
    }
    return AMediaFormat_getInt64(mAMediaFormat, name, out);
}

bool AMediaFormatWrapper::getFloat(const char *name, float *out) const {
    if (mAMediaFormat == NULL) {
        return false;
    }
    return AMediaFormat_getFloat(mAMediaFormat, name, out);
}

bool AMediaFormatWrapper::getDouble(const char *name, double *out) const {
    if (mAMediaFormat == NULL) {
        return false;
    }
    return AMediaFormat_getDouble(mAMediaFormat, name, out);
}

bool AMediaFormatWrapper::getSize(const char *name, size_t *out) const {
    if (mAMediaFormat == NULL) {
        return false;
    }
    return AMediaFormat_getSize(mAMediaFormat, name, out);
}

bool AMediaFormatWrapper::getRect(
        const char *name, int32_t *left, int32_t *top, int32_t *right, int32_t *bottom) const {
    if (mAMediaFormat == NULL) {
        return false;
    }
    return AMediaFormat_getRect(mAMediaFormat, name, left, top, right, bottom);
}

bool AMediaFormatWrapper::getBuffer(const char *name, void** data, size_t *outSize) const {
    if (mAMediaFormat == NULL) {
        return false;
    }
    return AMediaFormat_getBuffer(mAMediaFormat, name, data, outSize);
}

bool AMediaFormatWrapper::getString(const char *name, AString *out) const {
    if (mAMediaFormat == NULL) {
        return false;
    }
    const char *outChar = NULL;
    bool ret = AMediaFormat_getString(mAMediaFormat, name, &outChar);
    if (ret) {
        *out = AString(outChar);
    }
    return ret;
}

void AMediaFormatWrapper::setInt32(const char* name, int32_t value) {
    if (mAMediaFormat != NULL) {
        AMediaFormat_setInt32(mAMediaFormat, name, value);
    }
}

void AMediaFormatWrapper::setInt64(const char* name, int64_t value) {
    if (mAMediaFormat != NULL) {
        AMediaFormat_setInt64(mAMediaFormat, name, value);
    }
}

void AMediaFormatWrapper::setFloat(const char* name, float value) {
    if (mAMediaFormat != NULL) {
        AMediaFormat_setFloat(mAMediaFormat, name, value);
    }
}

void AMediaFormatWrapper::setDouble(const char* name, double value) {
    if (mAMediaFormat != NULL) {
        AMediaFormat_setDouble(mAMediaFormat, name, value);
    }
}

void AMediaFormatWrapper::setSize(const char* name, size_t value) {
    if (mAMediaFormat != NULL) {
        AMediaFormat_setSize(mAMediaFormat, name, value);
    }
}

void AMediaFormatWrapper::setRect(
        const char* name, int32_t left, int32_t top, int32_t right, int32_t bottom) {
    if (mAMediaFormat != NULL) {
        AMediaFormat_setRect(mAMediaFormat, name, left, top, right, bottom);
    }
}

void AMediaFormatWrapper::setString(const char* name, const AString &value) {
    if (mAMediaFormat != NULL) {
        AMediaFormat_setString(mAMediaFormat, name, value.c_str());
    }
}

void AMediaFormatWrapper::setBuffer(const char* name, void* data, size_t size) {
    if (mAMediaFormat != NULL) {
        AMediaFormat_setBuffer(mAMediaFormat, name, data, size);
    }
}


//////////// ANativeWindowWrapper
ANativeWindowWrapper::ANativeWindowWrapper(ANativeWindow *aNativeWindow)
    : mANativeWindow(aNativeWindow) {
    if (aNativeWindow != NULL) {
        ANativeWindow_acquire(aNativeWindow);
    }
}

ANativeWindowWrapper::~ANativeWindowWrapper() {
    release();
}

status_t ANativeWindowWrapper::release() {
    if (mANativeWindow != NULL) {
        ANativeWindow_release(mANativeWindow);
        mANativeWindow = NULL;
    }
    return OK;
}

ANativeWindow *ANativeWindowWrapper::getANativeWindow() const {
    return mANativeWindow;
}


//////////// AMediaDrmWrapper
AMediaDrmWrapper::AMediaDrmWrapper(const uint8_t uuid[16]) {
    mAMediaDrm = AMediaDrm_createByUUID(uuid);
}

AMediaDrmWrapper::AMediaDrmWrapper(AMediaDrm *aMediaDrm)
    : mAMediaDrm(aMediaDrm) {
}

AMediaDrmWrapper::~AMediaDrmWrapper() {
    release();
}

status_t AMediaDrmWrapper::release() {
    if (mAMediaDrm != NULL) {
        AMediaDrm_release(mAMediaDrm);
        mAMediaDrm = NULL;
    }
    return OK;
}

AMediaDrm *AMediaDrmWrapper::getAMediaDrm() const {
    return mAMediaDrm;
}

// static
bool AMediaDrmWrapper::isCryptoSchemeSupported(
        const uint8_t uuid[16],
        const char *mimeType) {
    return AMediaDrm_isCryptoSchemeSupported(uuid, mimeType);
}


//////////// AMediaCryptoWrapper
AMediaCryptoWrapper::AMediaCryptoWrapper(
        const uint8_t uuid[16], const void *initData, size_t initDataSize) {
    mAMediaCrypto = AMediaCrypto_new(uuid, initData, initDataSize);
}

AMediaCryptoWrapper::AMediaCryptoWrapper(AMediaCrypto *aMediaCrypto)
    : mAMediaCrypto(aMediaCrypto) {
}

AMediaCryptoWrapper::~AMediaCryptoWrapper() {
    release();
}

status_t AMediaCryptoWrapper::release() {
    if (mAMediaCrypto != NULL) {
        AMediaCrypto_delete(mAMediaCrypto);
        mAMediaCrypto = NULL;
    }
    return OK;
}

AMediaCrypto *AMediaCryptoWrapper::getAMediaCrypto() const {
    return mAMediaCrypto;
}

bool AMediaCryptoWrapper::isCryptoSchemeSupported(const uint8_t uuid[16]) {
    if (mAMediaCrypto == NULL) {
        return false;
    }
    return AMediaCrypto_isCryptoSchemeSupported(uuid);
}

bool AMediaCryptoWrapper::requiresSecureDecoderComponent(const char *mime) {
    if (mAMediaCrypto == NULL) {
        return false;
    }
    return AMediaCrypto_requiresSecureDecoderComponent(mime);
}


//////////// AMediaCodecCryptoInfoWrapper
// static
sp<AMediaCodecCryptoInfoWrapper> AMediaCodecCryptoInfoWrapper::Create(MetaDataBase &meta) {

    uint32_t type;
    const void *crypteddata;
    size_t cryptedsize;

    if (!meta.findData(kKeyEncryptedSizes, &type, &crypteddata, &cryptedsize)) {
        return NULL;
    }

    int numSubSamples = cryptedsize / sizeof(size_t);

    if (numSubSamples <= 0) {
        ALOGE("Create: INVALID numSubSamples: %d", numSubSamples);
        return NULL;
    }

    const void *cleardata;
    size_t clearsize;
    if (meta.findData(kKeyPlainSizes, &type, &cleardata, &clearsize)) {
        if (clearsize != cryptedsize) {
            // The two must be of the same length.
            ALOGE("Create: mismatch cryptedsize: %zu != clearsize: %zu", cryptedsize, clearsize);
            return NULL;
        }
    }

    const void *key;
    size_t keysize;
    if (meta.findData(kKeyCryptoKey, &type, &key, &keysize)) {
        if (keysize != kAESBlockSize) {
            // Keys must be 16 bytes in length.
            ALOGE("Create: Keys must be %zu bytes in length: %zu", kAESBlockSize, keysize);
            return NULL;
        }
    }

    const void *iv;
    size_t ivsize;
    if (meta.findData(kKeyCryptoIV, &type, &iv, &ivsize)) {
        if (ivsize != kAESBlockSize) {
            // IVs must be 16 bytes in length.
            ALOGE("Create: IV must be %zu bytes in length: %zu", kAESBlockSize, ivsize);
            return NULL;
        }
    }

    int32_t mode;
    if (!meta.findInt32(kKeyCryptoMode, &mode)) {
        mode = CryptoPlugin::kMode_AES_CTR;
    }

    return new AMediaCodecCryptoInfoWrapper(
            numSubSamples,
            (uint8_t*) key,
            (uint8_t*) iv,
            (CryptoPlugin::Mode)mode,
            (size_t*) cleardata,
            (size_t*) crypteddata);
}

AMediaCodecCryptoInfoWrapper::AMediaCodecCryptoInfoWrapper(
        int numsubsamples,
        uint8_t key[16],
        uint8_t iv[16],
        CryptoPlugin::Mode mode,
        size_t *clearbytes,
        size_t *encryptedbytes) {
    mAMediaCodecCryptoInfo =
        AMediaCodecCryptoInfo_new(numsubsamples,
                                  key,
                                  iv,
                                  translateToCryptoInfoMode(mode),
                                  clearbytes,
                                  encryptedbytes);
}

AMediaCodecCryptoInfoWrapper::AMediaCodecCryptoInfoWrapper(
        AMediaCodecCryptoInfo *aMediaCodecCryptoInfo)
    : mAMediaCodecCryptoInfo(aMediaCodecCryptoInfo) {
}

AMediaCodecCryptoInfoWrapper::~AMediaCodecCryptoInfoWrapper() {
    release();
}

status_t AMediaCodecCryptoInfoWrapper::release() {
    if (mAMediaCodecCryptoInfo != NULL) {
        media_status_t err = AMediaCodecCryptoInfo_delete(mAMediaCodecCryptoInfo);
        mAMediaCodecCryptoInfo = NULL;
        return translateErrorCode(err);
    }
    return OK;
}

AMediaCodecCryptoInfo *AMediaCodecCryptoInfoWrapper::getAMediaCodecCryptoInfo() const {
    return mAMediaCodecCryptoInfo;
}

void AMediaCodecCryptoInfoWrapper::setPattern(CryptoPlugin::Pattern *pattern) {
    if (mAMediaCodecCryptoInfo == NULL || pattern == NULL) {
        return;
    }
    cryptoinfo_pattern_t ndkPattern = {(int32_t)pattern->mEncryptBlocks,
                                       (int32_t)pattern->mSkipBlocks };
    return AMediaCodecCryptoInfo_setPattern(mAMediaCodecCryptoInfo, &ndkPattern);
}

size_t AMediaCodecCryptoInfoWrapper::getNumSubSamples() {
    if (mAMediaCodecCryptoInfo == NULL) {
        return 0;
    }
    return AMediaCodecCryptoInfo_getNumSubSamples(mAMediaCodecCryptoInfo);
}

status_t AMediaCodecCryptoInfoWrapper::getKey(uint8_t *dst) {
    if (mAMediaCodecCryptoInfo == NULL) {
        return DEAD_OBJECT;
    }
    if (dst == NULL) {
        return BAD_VALUE;
    }
    return translateErrorCode(
        AMediaCodecCryptoInfo_getKey(mAMediaCodecCryptoInfo, dst));
}

status_t AMediaCodecCryptoInfoWrapper::getIV(uint8_t *dst) {
    if (mAMediaCodecCryptoInfo == NULL) {
        return DEAD_OBJECT;
    }
    if (dst == NULL) {
        return BAD_VALUE;
    }
    return translateErrorCode(
        AMediaCodecCryptoInfo_getIV(mAMediaCodecCryptoInfo, dst));
}

CryptoPlugin::Mode AMediaCodecCryptoInfoWrapper::getMode() {
    if (mAMediaCodecCryptoInfo == NULL) {
        return CryptoPlugin::kMode_Unencrypted;
    }
    return translateToCryptoPluginMode(
        AMediaCodecCryptoInfo_getMode(mAMediaCodecCryptoInfo));
}

status_t AMediaCodecCryptoInfoWrapper::getClearBytes(size_t *dst) {
    if (mAMediaCodecCryptoInfo == NULL) {
        return DEAD_OBJECT;
    }
    if (dst == NULL) {
        return BAD_VALUE;
    }
    return translateErrorCode(
        AMediaCodecCryptoInfo_getClearBytes(mAMediaCodecCryptoInfo, dst));
}

status_t AMediaCodecCryptoInfoWrapper::getEncryptedBytes(size_t *dst) {
    if (mAMediaCodecCryptoInfo == NULL) {
        return DEAD_OBJECT;
    }
    if (dst == NULL) {
        return BAD_VALUE;
    }
    return translateErrorCode(
        AMediaCodecCryptoInfo_getEncryptedBytes(mAMediaCodecCryptoInfo, dst));
}


//////////// AMediaCodecWrapper
// static
sp<AMediaCodecWrapper> AMediaCodecWrapper::CreateCodecByName(const AString &name) {
    AMediaCodec *aMediaCodec = AMediaCodec_createCodecByName(name.c_str());
    return new AMediaCodecWrapper(aMediaCodec);
}

// static
sp<AMediaCodecWrapper> AMediaCodecWrapper::CreateDecoderByType(const AString &mimeType) {
    AMediaCodec *aMediaCodec = AMediaCodec_createDecoderByType(mimeType.c_str());
    return new AMediaCodecWrapper(aMediaCodec);
}

// static
void AMediaCodecWrapper::OnInputAvailableCB(
        AMediaCodec * /* aMediaCodec */,
        void *userdata,
        int32_t index) {
    ALOGV("OnInputAvailableCB: index(%d)", index);
    sp<AMessage> msg = sp<AMessage>((AMessage *)userdata)->dup();
    msg->setInt32("callbackID", CB_INPUT_AVAILABLE);
    msg->setInt32("index", index);
    msg->post();
}

// static
void AMediaCodecWrapper::OnOutputAvailableCB(
        AMediaCodec * /* aMediaCodec */,
        void *userdata,
        int32_t index,
        AMediaCodecBufferInfo *bufferInfo) {
    ALOGV("OnOutputAvailableCB: index(%d), (%d, %d, %lld, 0x%x)",
          index, bufferInfo->offset, bufferInfo->size,
          (long long)bufferInfo->presentationTimeUs, bufferInfo->flags);
    sp<AMessage> msg = sp<AMessage>((AMessage *)userdata)->dup();
    msg->setInt32("callbackID", CB_OUTPUT_AVAILABLE);
    msg->setInt32("index", index);
    msg->setSize("offset", (size_t)(bufferInfo->offset));
    msg->setSize("size", (size_t)(bufferInfo->size));
    msg->setInt64("timeUs", bufferInfo->presentationTimeUs);
    msg->setInt32("flags", (int32_t)(bufferInfo->flags));
    msg->post();
}

// static
void AMediaCodecWrapper::OnFormatChangedCB(
        AMediaCodec * /* aMediaCodec */,
        void *userdata,
        AMediaFormat *format) {
    sp<AMediaFormatWrapper> formatWrapper = new AMediaFormatWrapper(format);
    sp<AMessage> outputFormat = formatWrapper->toAMessage();
    ALOGV("OnFormatChangedCB: format(%s)", outputFormat->debugString().c_str());

    sp<AMessage> msg = sp<AMessage>((AMessage *)userdata)->dup();
    msg->setInt32("callbackID", CB_OUTPUT_FORMAT_CHANGED);
    msg->setMessage("format", outputFormat);
    msg->post();
}

// static
void AMediaCodecWrapper::OnErrorCB(
        AMediaCodec * /* aMediaCodec */,
        void *userdata,
        media_status_t err,
        int32_t actionCode,
        const char *detail) {
    ALOGV("OnErrorCB: err(%d), actionCode(%d), detail(%s)", err, actionCode, detail);
    sp<AMessage> msg = sp<AMessage>((AMessage *)userdata)->dup();
    msg->setInt32("callbackID", CB_ERROR);
    msg->setInt32("err", translateErrorCode(err));
    msg->setInt32("actionCode", translateActionCode(actionCode));
    msg->setString("detail", detail);
    msg->post();
}

AMediaCodecWrapper::AMediaCodecWrapper(AMediaCodec *aMediaCodec)
    : mAMediaCodec(aMediaCodec) {
}

AMediaCodecWrapper::~AMediaCodecWrapper() {
    release();
}

status_t AMediaCodecWrapper::release() {
    if (mAMediaCodec != NULL) {
        AMediaCodecOnAsyncNotifyCallback aCB = {};
        AMediaCodec_setAsyncNotifyCallback(mAMediaCodec, aCB, NULL);
        mCallback = NULL;

        media_status_t err = AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = NULL;
        return translateErrorCode(err);
    }
    return OK;
}

AMediaCodec *AMediaCodecWrapper::getAMediaCodec() const {
    return mAMediaCodec;
}

status_t AMediaCodecWrapper::getName(AString *outComponentName) const {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    char *name = NULL;
    media_status_t err = AMediaCodec_getName(mAMediaCodec, &name);
    if (err != AMEDIA_OK) {
        return translateErrorCode(err);
    }

    *outComponentName = AString(name);
    AMediaCodec_releaseName(mAMediaCodec, name);
    return OK;
}

status_t AMediaCodecWrapper::configure(
    const sp<AMediaFormatWrapper> &format,
    const sp<ANativeWindowWrapper> &nww,
    const sp<AMediaCryptoWrapper> &crypto,
    uint32_t flags) {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }

    media_status_t err = AMediaCodec_configure(
            mAMediaCodec,
            format->getAMediaFormat(),
            (nww == NULL ? NULL : nww->getANativeWindow()),
            crypto == NULL ? NULL : crypto->getAMediaCrypto(),
            flags);

    return translateErrorCode(err);
}

status_t AMediaCodecWrapper::setCallback(const sp<AMessage> &callback) {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }

    mCallback = callback;

    AMediaCodecOnAsyncNotifyCallback aCB = {
        OnInputAvailableCB,
        OnOutputAvailableCB,
        OnFormatChangedCB,
        OnErrorCB
    };

    return translateErrorCode(
            AMediaCodec_setAsyncNotifyCallback(mAMediaCodec, aCB, callback.get()));
}

status_t AMediaCodecWrapper::releaseCrypto() {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(AMediaCodec_releaseCrypto(mAMediaCodec));
}

status_t AMediaCodecWrapper::start() {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(AMediaCodec_start(mAMediaCodec));
}

status_t AMediaCodecWrapper::stop() {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(AMediaCodec_stop(mAMediaCodec));
}

status_t AMediaCodecWrapper::flush() {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(AMediaCodec_flush(mAMediaCodec));
}

uint8_t* AMediaCodecWrapper::getInputBuffer(size_t idx, size_t *out_size) {
    if (mAMediaCodec == NULL) {
        return NULL;
    }
    return AMediaCodec_getInputBuffer(mAMediaCodec, idx, out_size);
}

uint8_t* AMediaCodecWrapper::getOutputBuffer(size_t idx, size_t *out_size) {
    if (mAMediaCodec == NULL) {
        return NULL;
    }
    return AMediaCodec_getOutputBuffer(mAMediaCodec, idx, out_size);
}

status_t AMediaCodecWrapper::queueInputBuffer(
        size_t idx,
        size_t offset,
        size_t size,
        uint64_t time,
        uint32_t flags) {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(
        AMediaCodec_queueInputBuffer(mAMediaCodec, idx, offset, size, time, flags));
}

status_t AMediaCodecWrapper::queueSecureInputBuffer(
        size_t idx,
        size_t offset,
        sp<AMediaCodecCryptoInfoWrapper> &codecCryptoInfo,
        uint64_t time,
        uint32_t flags) {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(
        AMediaCodec_queueSecureInputBuffer(
            mAMediaCodec,
            idx,
            offset,
            codecCryptoInfo->getAMediaCodecCryptoInfo(),
            time,
            flags));
}

sp<AMediaFormatWrapper> AMediaCodecWrapper::getOutputFormat() {
    if (mAMediaCodec == NULL) {
        return NULL;
    }
    return new AMediaFormatWrapper(AMediaCodec_getOutputFormat(mAMediaCodec));
}

sp<AMediaFormatWrapper> AMediaCodecWrapper::getInputFormat() {
    if (mAMediaCodec == NULL) {
        return NULL;
    }
    return new AMediaFormatWrapper(AMediaCodec_getInputFormat(mAMediaCodec));
}

status_t AMediaCodecWrapper::releaseOutputBuffer(size_t idx, bool render) {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(
        AMediaCodec_releaseOutputBuffer(mAMediaCodec, idx, render));
}

status_t AMediaCodecWrapper::setOutputSurface(const sp<ANativeWindowWrapper> &nww) {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(
        AMediaCodec_setOutputSurface(mAMediaCodec,
                                     (nww == NULL ? NULL : nww->getANativeWindow())));
}

status_t AMediaCodecWrapper::releaseOutputBufferAtTime(size_t idx, int64_t timestampNs) {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(
        AMediaCodec_releaseOutputBufferAtTime(mAMediaCodec, idx, timestampNs));
}

status_t AMediaCodecWrapper::setParameters(const sp<AMediaFormatWrapper> &params) {
    if (mAMediaCodec == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(
        AMediaCodec_setParameters(mAMediaCodec, params->getAMediaFormat()));
}

//////////// AMediaExtractorWrapper

AMediaExtractorWrapper::AMediaExtractorWrapper(AMediaExtractor *aMediaExtractor)
    : mAMediaExtractor(aMediaExtractor) {
}

AMediaExtractorWrapper::~AMediaExtractorWrapper() {
    release();
}

status_t AMediaExtractorWrapper::release() {
    if (mAMediaExtractor != NULL) {
        media_status_t err = AMediaExtractor_delete(mAMediaExtractor);
        mAMediaExtractor = NULL;
        return translateErrorCode(err);
    }
    return OK;
}

AMediaExtractor *AMediaExtractorWrapper::getAMediaExtractor() const {
    return mAMediaExtractor;
}

status_t AMediaExtractorWrapper::setDataSource(int fd, off64_t offset, off64_t length) {
    if (mAMediaExtractor == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(AMediaExtractor_setDataSourceFd(
            mAMediaExtractor, fd, offset, length));
}

status_t AMediaExtractorWrapper::setDataSource(const char *location) {
    if (mAMediaExtractor == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(AMediaExtractor_setDataSource(mAMediaExtractor, location));
}

status_t AMediaExtractorWrapper::setDataSource(AMediaDataSource *source) {
    if (mAMediaExtractor == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(AMediaExtractor_setDataSourceCustom(mAMediaExtractor, source));
}

size_t AMediaExtractorWrapper::getTrackCount() {
    if (mAMediaExtractor == NULL) {
        return 0;
    }
    return AMediaExtractor_getTrackCount(mAMediaExtractor);
}

sp<AMediaFormatWrapper> AMediaExtractorWrapper::getFormat() {
    if (mAMediaExtractor == NULL) {
        return NULL;
    }
    return new AMediaFormatWrapper(AMediaExtractor_getFileFormat(mAMediaExtractor));
}

sp<AMediaFormatWrapper> AMediaExtractorWrapper::getTrackFormat(size_t idx) {
    if (mAMediaExtractor == NULL) {
        return NULL;
    }
    return new AMediaFormatWrapper(AMediaExtractor_getTrackFormat(mAMediaExtractor, idx));
}

status_t AMediaExtractorWrapper::selectTrack(size_t idx) {
    if (mAMediaExtractor == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(AMediaExtractor_selectTrack(mAMediaExtractor, idx));
}

status_t AMediaExtractorWrapper::unselectTrack(size_t idx) {
    if (mAMediaExtractor == NULL) {
        return DEAD_OBJECT;
    }
    return translateErrorCode(AMediaExtractor_unselectTrack(mAMediaExtractor, idx));
}

status_t AMediaExtractorWrapper::selectSingleTrack(size_t idx) {
    if (mAMediaExtractor == NULL) {
        return DEAD_OBJECT;
    }
    for (size_t i = 0; i < AMediaExtractor_getTrackCount(mAMediaExtractor); ++i) {
        if (i == idx) {
            media_status_t err = AMediaExtractor_selectTrack(mAMediaExtractor, i);
            if (err != AMEDIA_OK) {
                return translateErrorCode(err);
            }
        } else {
            media_status_t err = AMediaExtractor_unselectTrack(mAMediaExtractor, i);
            if (err != AMEDIA_OK) {
                return translateErrorCode(err);
            }
        }
    }
    return OK;
}

ssize_t AMediaExtractorWrapper::readSampleData(const sp<ABuffer> &buffer) {
    if (mAMediaExtractor == NULL) {
        return -1;
    }
    return AMediaExtractor_readSampleData(mAMediaExtractor, buffer->data(), buffer->capacity());
}

ssize_t AMediaExtractorWrapper::getSampleSize() {
    if (mAMediaExtractor == NULL) {
        return 0;
    }
    return AMediaExtractor_getSampleSize(mAMediaExtractor);
}

uint32_t AMediaExtractorWrapper::getSampleFlags() {
    if (mAMediaExtractor == NULL) {
        return 0;
    }
    return AMediaExtractor_getSampleFlags(mAMediaExtractor);
}

int AMediaExtractorWrapper::getSampleTrackIndex() {
    if (mAMediaExtractor == NULL) {
        return -1;
    }
    return AMediaExtractor_getSampleTrackIndex(mAMediaExtractor);
}

int64_t AMediaExtractorWrapper::getSampleTime() {
    if (mAMediaExtractor == NULL) {
        return -1;
    }
    return AMediaExtractor_getSampleTime(mAMediaExtractor);
}

status_t AMediaExtractorWrapper::getSampleFormat(sp<AMediaFormatWrapper> &formatWrapper) {
    if (mAMediaExtractor == NULL) {
        return DEAD_OBJECT;
    }
    AMediaFormat *format = AMediaFormat_new();
    formatWrapper = new AMediaFormatWrapper(format);
    return translateErrorCode(AMediaExtractor_getSampleFormat(mAMediaExtractor, format));
}

int64_t AMediaExtractorWrapper::getCachedDuration() {
    if (mAMediaExtractor == NULL) {
        return -1;
    }
    return AMediaExtractor_getCachedDuration(mAMediaExtractor);
}

bool AMediaExtractorWrapper::advance() {
    if (mAMediaExtractor == NULL) {
        return false;
    }
    return AMediaExtractor_advance(mAMediaExtractor);
}

status_t AMediaExtractorWrapper::seekTo(int64_t seekPosUs, MediaSource::ReadOptions::SeekMode mode) {
    if (mAMediaExtractor == NULL) {
        return DEAD_OBJECT;
    }

    SeekMode aMode;
    switch (mode) {
        case MediaSource::ReadOptions::SEEK_PREVIOUS_SYNC: {
            aMode = AMEDIAEXTRACTOR_SEEK_PREVIOUS_SYNC;
            break;
        }
        case MediaSource::ReadOptions::SEEK_NEXT_SYNC: {
            aMode = AMEDIAEXTRACTOR_SEEK_NEXT_SYNC;
            break;
        }
        default: {
            aMode = AMEDIAEXTRACTOR_SEEK_CLOSEST_SYNC;
            break;
        }
    }
    return AMediaExtractor_seekTo(mAMediaExtractor, seekPosUs, aMode);
}

PsshInfo* AMediaExtractorWrapper::getPsshInfo() {
    if (mAMediaExtractor == NULL) {
        return NULL;
    }
    return AMediaExtractor_getPsshInfo(mAMediaExtractor);
}

sp<AMediaCodecCryptoInfoWrapper> AMediaExtractorWrapper::getSampleCryptoInfo() {
    if (mAMediaExtractor == NULL) {
        return NULL;
    }
    AMediaCodecCryptoInfo *cryptoInfo = AMediaExtractor_getSampleCryptoInfo(mAMediaExtractor);
    if (cryptoInfo == NULL) {
        return NULL;
    }
    return new AMediaCodecCryptoInfoWrapper(cryptoInfo);
}

AMediaDataSourceWrapper::AMediaDataSourceWrapper(const sp<DataSource> &dataSource)
    : mDataSource(dataSource),
      mAMediaDataSource(convertDataSourceToAMediaDataSource(dataSource)) {
}

AMediaDataSourceWrapper::AMediaDataSourceWrapper(AMediaDataSource *aDataSource)
    : mDataSource(NULL),
      mAMediaDataSource(aDataSource) {
}

AMediaDataSourceWrapper::~AMediaDataSourceWrapper() {
    if (mAMediaDataSource == NULL) {
        return;
    }
    AMediaDataSource_close(mAMediaDataSource);
    AMediaDataSource_delete(mAMediaDataSource);
    mAMediaDataSource = NULL;
}

AMediaDataSource* AMediaDataSourceWrapper::getAMediaDataSource() {
    return mAMediaDataSource;
}

void AMediaDataSourceWrapper::close() {
    AMediaDataSource_close(mAMediaDataSource);
}

}  // namespace android
