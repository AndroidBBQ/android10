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
#define LOG_TAG "NdkMediaExtractor"


#include <media/NdkMediaError.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaErrorPriv.h>
#include <media/NdkMediaFormatPriv.h>
#include "NdkMediaDataSourcePriv.h"


#include <inttypes.h>
#include <utils/Log.h>
#include <utils/StrongPointer.h>
#include <media/hardware/CryptoAPI.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/NuMediaExtractor.h>
#include <media/IMediaHTTPService.h>
#include <android_util_Binder.h>

#include <jni.h>

using namespace android;

struct AMediaExtractor {
    sp<NuMediaExtractor> mImpl;
    sp<ABuffer> mPsshBuf;
};

sp<ABuffer> U32ArrayToSizeBuf(size_t numSubSamples, uint32_t *data) {
    if (numSubSamples >  SIZE_MAX / sizeof(size_t)) {
        return NULL;
    }
    sp<ABuffer> sizebuf = new ABuffer(numSubSamples * sizeof(size_t));
    size_t *sizes = (size_t *)sizebuf->data();
    for (size_t i = 0; sizes != NULL && i < numSubSamples; i++) {
        sizes[i] = data[i];
    }
    return sizebuf;
}

extern "C" {

EXPORT
AMediaExtractor* AMediaExtractor_new() {
    ALOGV("ctor");
    AMediaExtractor *mData = new AMediaExtractor();
    mData->mImpl = new NuMediaExtractor();
    return mData;
}

EXPORT
media_status_t AMediaExtractor_delete(AMediaExtractor *mData) {
    ALOGV("dtor");
    delete mData;
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaExtractor_setDataSourceFd(AMediaExtractor *mData, int fd, off64_t offset,
        off64_t length) {
    ALOGV("setDataSource(%d, %" PRId64 ", %" PRId64 ")", fd, offset, length);
    return translate_error(mData->mImpl->setDataSource(fd, offset, length));
}

media_status_t AMediaExtractor_setDataSourceWithHeaders(AMediaExtractor *mData,
        const char *uri,
        int numheaders,
        const char * const *keys,
        const char * const *values) {

    ALOGV("setDataSource(%s)", uri);

    sp<MediaHTTPService> httpService = createMediaHttpService(uri, /* version = */ 1);
    if (httpService == NULL) {
        ALOGE("can't create http service");
        return AMEDIA_ERROR_UNSUPPORTED;
    }

    KeyedVector<String8, String8> headers;
    for (int i = 0; i < numheaders; ++i) {
        String8 key8(keys[i]);
        String8 value8(values[i]);
        headers.add(key8, value8);
    }

    status_t err;
    err = mData->mImpl->setDataSource(httpService, uri, numheaders > 0 ? &headers : NULL);
    return translate_error(err);
}

EXPORT
media_status_t AMediaExtractor_setDataSource(AMediaExtractor *mData, const char *location) {
    return AMediaExtractor_setDataSourceWithHeaders(mData, location, 0, NULL, NULL);
}

EXPORT
media_status_t AMediaExtractor_setDataSourceCustom(AMediaExtractor* mData, AMediaDataSource *src) {
    return translate_error(mData->mImpl->setDataSource(new NdkDataSource(src)));
}

EXPORT
AMediaFormat* AMediaExtractor_getFileFormat(AMediaExtractor *mData) {
    sp<AMessage> format;
    mData->mImpl->getFileFormat(&format);
    return AMediaFormat_fromMsg(&format);
}

EXPORT
size_t AMediaExtractor_getTrackCount(AMediaExtractor *mData) {
    return mData->mImpl->countTracks();
}

EXPORT
AMediaFormat* AMediaExtractor_getTrackFormat(AMediaExtractor *mData, size_t idx) {
    sp<AMessage> format;
    mData->mImpl->getTrackFormat(idx, &format);
    return AMediaFormat_fromMsg(&format);
}

EXPORT
media_status_t AMediaExtractor_selectTrack(AMediaExtractor *mData, size_t idx) {
    ALOGV("selectTrack(%zu)", idx);
    return translate_error(mData->mImpl->selectTrack(idx));
}

EXPORT
media_status_t AMediaExtractor_unselectTrack(AMediaExtractor *mData, size_t idx) {
    ALOGV("unselectTrack(%zu)", idx);
    return translate_error(mData->mImpl->unselectTrack(idx));
}

EXPORT
bool AMediaExtractor_advance(AMediaExtractor *mData) {
    //ALOGV("advance");
    status_t err = mData->mImpl->advance();
    if (err == ERROR_END_OF_STREAM) {
        return false;
    } else if (err != OK) {
        ALOGE("sf error code: %d", err);
        return false;
    }
    return true;
}

EXPORT
media_status_t AMediaExtractor_seekTo(AMediaExtractor *ex, int64_t seekPosUs, SeekMode mode) {
    android::MediaSource::ReadOptions::SeekMode sfmode;
    if (mode == AMEDIAEXTRACTOR_SEEK_PREVIOUS_SYNC) {
        sfmode = android::MediaSource::ReadOptions::SEEK_PREVIOUS_SYNC;
    } else if (mode == AMEDIAEXTRACTOR_SEEK_CLOSEST_SYNC) {
        sfmode = android::MediaSource::ReadOptions::SEEK_CLOSEST_SYNC;
    } else {
        sfmode = android::MediaSource::ReadOptions::SEEK_NEXT_SYNC;
    }

    return translate_error(ex->mImpl->seekTo(seekPosUs, sfmode));
}

EXPORT
ssize_t AMediaExtractor_readSampleData(AMediaExtractor *mData, uint8_t *buffer, size_t capacity) {
    //ALOGV("readSampleData");
    sp<ABuffer> tmp = new ABuffer(buffer, capacity);
    if (mData->mImpl->readSampleData(tmp) == OK) {
        return tmp->size();
    }
    return -1;
}

EXPORT
ssize_t AMediaExtractor_getSampleSize(AMediaExtractor *mData) {
    size_t sampleSize;
    status_t err = mData->mImpl->getSampleSize(&sampleSize);
    if (err != OK) {
        return -1;
    }
    return sampleSize;
}

EXPORT
uint32_t AMediaExtractor_getSampleFlags(AMediaExtractor *mData) {
    int sampleFlags = 0;
    sp<MetaData> meta;
    status_t err = mData->mImpl->getSampleMeta(&meta);
    if (err != OK) {
        return -1;
    }
    int32_t val;
    if (meta->findInt32(kKeyIsSyncFrame, &val) && val != 0) {
        sampleFlags |= AMEDIAEXTRACTOR_SAMPLE_FLAG_SYNC;
    }

    uint32_t type;
    const void *data;
    size_t size;
    if (meta->findData(kKeyEncryptedSizes, &type, &data, &size)) {
        sampleFlags |= AMEDIAEXTRACTOR_SAMPLE_FLAG_ENCRYPTED;
    }
    return sampleFlags;
}

EXPORT
int AMediaExtractor_getSampleTrackIndex(AMediaExtractor *mData) {
    size_t idx;
    if (mData->mImpl->getSampleTrackIndex(&idx) != OK) {
        return -1;
    }
    return idx;
}

EXPORT
int64_t AMediaExtractor_getSampleTime(AMediaExtractor *mData) {
    int64_t time;
    if (mData->mImpl->getSampleTime(&time) != OK) {
        return -1;
    }
    return time;
}

EXPORT
PsshInfo* AMediaExtractor_getPsshInfo(AMediaExtractor *ex) {

    if (ex->mPsshBuf != NULL) {
        return (PsshInfo*) ex->mPsshBuf->data();
    }

    sp<AMessage> format;
    ex->mImpl->getFileFormat(&format);
    sp<ABuffer> buffer;
    if(!format->findBuffer("pssh", &buffer)) {
        return NULL;
    }

    // the format of the buffer is 1 or more of:
    //    {
    //        16 byte uuid
    //        4 byte data length N
    //        N bytes of data
    //    }

    // Determine the number of entries in the source data.
    // Since we got the data from stagefright, we trust it is valid and properly formatted.
    const uint8_t* data = buffer->data();
    size_t len = buffer->size();
    size_t numentries = 0;
    while (len > 0) {
        numentries++;

        if (len < 16) {
            ALOGE("invalid PSSH data");
            return NULL;
        }
        // skip uuid
        data += 16;
        len -= 16;

        // get data length
        if (len < 4) {
            ALOGE("invalid PSSH data");
            return NULL;
        }
        uint32_t datalen = *((uint32_t*)data);
        data += 4;
        len -= 4;

        if (len < datalen) {
            ALOGE("invalid PSSH data");
            return NULL;
        }
        // skip the data
        data += datalen;
        len -= datalen;
    }

    // there are <numentries> in the source buffer, we need
    // (source buffer size) - (sizeof(uint32_t) * numentries) + sizeof(size_t)
    //  + ((sizeof(void*) + sizeof(size_t)) * numentries) bytes for the PsshInfo structure
    // Or in other words, the data lengths in the source structure are replaced by size_t
    // (which may be the same size or larger, for 64 bit), and in addition there is an
    // extra pointer for each entry, and an extra size_t for the entire PsshInfo.
    size_t newsize = buffer->size() - (sizeof(uint32_t) * numentries) + sizeof(size_t)
            + ((sizeof(void*) + sizeof(size_t)) * numentries);
    if (newsize <= buffer->size()) {
        ALOGE("invalid PSSH data");
        return NULL;
    }
    ex->mPsshBuf = new ABuffer(newsize);
    ex->mPsshBuf->setRange(0, newsize);

    // copy data
    const uint8_t* src = buffer->data();
    uint8_t* dst = ex->mPsshBuf->data();
    uint8_t* dstdata = dst + sizeof(size_t) + numentries * sizeof(PsshEntry);
    *((size_t*)dst) = numentries;
    dst += sizeof(size_t);
    for (size_t i = 0; i < numentries; i++) {
        // copy uuid
        memcpy(dst, src, 16);
        src += 16;
        dst += 16;

        // get/copy data length
        uint32_t datalen = *((uint32_t*)src);
        *((size_t*)dst) = datalen;
        src += sizeof(uint32_t);
        dst += sizeof(size_t);

        // the next entry in the destination is a pointer to the actual data, which we store
        // after the array of PsshEntry
        *((void**)dst) = dstdata;
        dst += sizeof(void*);

        // copy the actual data
        memcpy(dstdata, src, datalen);
        dstdata += datalen;
        src += datalen;
    }

    return (PsshInfo*) ex->mPsshBuf->data();
}

EXPORT
AMediaCodecCryptoInfo *AMediaExtractor_getSampleCryptoInfo(AMediaExtractor *ex) {
    sp<MetaData> meta;
    if(ex->mImpl->getSampleMeta(&meta) != 0) {
        return NULL;
    }

    uint32_t type;
    const void *crypteddata;
    size_t cryptedsize;
    if (!meta->findData(kKeyEncryptedSizes, &type, &crypteddata, &cryptedsize)) {
        return NULL;
    }
    size_t numSubSamples = cryptedsize / sizeof(uint32_t);

    const void *cleardata;
    size_t clearsize;
    if (meta->findData(kKeyPlainSizes, &type, &cleardata, &clearsize)) {
        if (clearsize != cryptedsize) {
            // The two must be of the same length.
            return NULL;
        }
    }

    const void *key;
    size_t keysize;
    if (meta->findData(kKeyCryptoKey, &type, &key, &keysize)) {
        if (keysize != 16) {
            // Keys must be 16 bytes in length.
            return NULL;
        }
    }

    const void *iv;
    size_t ivsize;
    if (meta->findData(kKeyCryptoIV, &type, &iv, &ivsize)) {
        if (ivsize != 16) {
            // IVs must be 16 bytes in length.
            return NULL;
        }
    }

    int32_t mode;
    if (!meta->findInt32(kKeyCryptoMode, &mode)) {
        mode = CryptoPlugin::kMode_AES_CTR;
    }

    if (sizeof(uint32_t) != sizeof(size_t)) {
        sp<ABuffer> clearbuf   = U32ArrayToSizeBuf(numSubSamples, (uint32_t *)cleardata);
        sp<ABuffer> cryptedbuf = U32ArrayToSizeBuf(numSubSamples, (uint32_t *)crypteddata);
        cleardata   = clearbuf    == NULL ? NULL : clearbuf->data();
        crypteddata = crypteddata == NULL ? NULL : cryptedbuf->data();
        if(crypteddata == NULL || cleardata == NULL) {
            return NULL;
        }
    }

    return AMediaCodecCryptoInfo_new(
            numSubSamples,
            (uint8_t*) key,
            (uint8_t*) iv,
            (cryptoinfo_mode_t) mode,
            (size_t*) cleardata,
            (size_t*) crypteddata);
}

EXPORT
int64_t AMediaExtractor_getCachedDuration(AMediaExtractor *ex) {
    bool eos;
    int64_t durationUs;
    if (ex->mImpl->getCachedDuration(&durationUs, &eos)) {
        return durationUs;
    }
    return -1;
}

EXPORT
media_status_t AMediaExtractor_getSampleFormat(AMediaExtractor *ex, AMediaFormat *fmt) {
    if (fmt == NULL) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    sp<MetaData> sampleMeta;
    status_t err = ex->mImpl->getSampleMeta(&sampleMeta);
    if (err != OK) {
        return translate_error(err);
    }

    sp<AMessage> meta;
    AMediaFormat_getFormat(fmt, &meta);
    meta->clear();

    int32_t layerId;
    if (sampleMeta->findInt32(kKeyTemporalLayerId, &layerId)) {
        meta->setInt32(AMEDIAFORMAT_KEY_TEMPORAL_LAYER_ID, layerId);
    }

    size_t trackIndex;
    err = ex->mImpl->getSampleTrackIndex(&trackIndex);
    if (err == OK) {
        meta->setInt32(AMEDIAFORMAT_KEY_TRACK_INDEX, trackIndex);
        sp<AMessage> trackFormat;
        AString mime;
        err = ex->mImpl->getTrackFormat(trackIndex, &trackFormat);
        if (err == OK
                && trackFormat != NULL
                && trackFormat->findString(AMEDIAFORMAT_KEY_MIME, &mime)) {
            meta->setString(AMEDIAFORMAT_KEY_MIME, mime);
        }
    }

    int64_t durationUs;
    if (sampleMeta->findInt64(kKeyDuration, &durationUs)) {
        meta->setInt64(AMEDIAFORMAT_KEY_DURATION, durationUs);
    }

    uint32_t dataType; // unused
    const void *seiData;
    size_t seiLength;
    if (sampleMeta->findData(kKeySEI, &dataType, &seiData, &seiLength)) {
        sp<ABuffer> sei = ABuffer::CreateAsCopy(seiData, seiLength);;
        meta->setBuffer(AMEDIAFORMAT_KEY_SEI, sei);
    }

    const void *mpegUserDataPointer;
    size_t mpegUserDataLength;
    if (sampleMeta->findData(
            kKeyMpegUserData, &dataType, &mpegUserDataPointer, &mpegUserDataLength)) {
        sp<ABuffer> mpegUserData = ABuffer::CreateAsCopy(mpegUserDataPointer, mpegUserDataLength);
        meta->setBuffer(AMEDIAFORMAT_KEY_MPEG_USER_DATA, mpegUserData);
    }

    const void *audioPresentationsPointer;
    size_t audioPresentationsLength;
    if (sampleMeta->findData(
            kKeyAudioPresentationInfo, &dataType,
            &audioPresentationsPointer, &audioPresentationsLength)) {
        sp<ABuffer> audioPresentationsData = ABuffer::CreateAsCopy(
                audioPresentationsPointer, audioPresentationsLength);
        meta->setBuffer(AMEDIAFORMAT_KEY_AUDIO_PRESENTATION_INFO, audioPresentationsData);
    }

    return AMEDIA_OK;
}

} // extern "C"

