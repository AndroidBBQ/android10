/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include <mutex>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/Utils.h>

#include <media/MediaTrack.h>
#include <media/MediaExtractorPluginApi.h>
#include <media/NdkMediaErrorPriv.h>
#include <media/NdkMediaFormatPriv.h>

namespace android {

MediaTrack::MediaTrack() {}

MediaTrack::~MediaTrack() {}

////////////////////////////////////////////////////////////////////////////////

void MediaTrack::ReadOptions::setNonBlocking() {
    mNonBlocking = true;
}

void MediaTrack::ReadOptions::clearNonBlocking() {
    mNonBlocking = false;
}

bool MediaTrack::ReadOptions::getNonBlocking() const {
    return mNonBlocking;
}

void MediaTrack::ReadOptions::setSeekTo(int64_t time_us, SeekMode mode) {
    mOptions |= kSeekTo_Option;
    mSeekTimeUs = time_us;
    mSeekMode = mode;
}

bool MediaTrack::ReadOptions::getSeekTo(
        int64_t *time_us, SeekMode *mode) const {
    *time_us = mSeekTimeUs;
    *mode = mSeekMode;
    return (mOptions & kSeekTo_Option) != 0;
}

/* -------------- unwrapper --------------- */

MediaTrackCUnwrapper::MediaTrackCUnwrapper(CMediaTrack *cmediatrack) {
    wrapper = cmediatrack;
    bufferGroup = nullptr;
}

MediaTrackCUnwrapper *MediaTrackCUnwrapper::create(CMediaTrack *cmediatrack) {
    if (cmediatrack == nullptr) {
        return nullptr;
    }
    return new MediaTrackCUnwrapper(cmediatrack);
}

MediaTrackCUnwrapper::~MediaTrackCUnwrapper() {
    wrapper->free(wrapper->data);
    free(wrapper);
    delete bufferGroup;
}

status_t MediaTrackCUnwrapper::start() {
    if (bufferGroup == nullptr) {
        bufferGroup = new MediaBufferGroup();
    }
    return reverse_translate_error(wrapper->start(wrapper->data, bufferGroup->wrap()));
}

status_t MediaTrackCUnwrapper::stop() {
    return reverse_translate_error(wrapper->stop(wrapper->data));
}

status_t MediaTrackCUnwrapper::getFormat(MetaDataBase& format) {
    sp<AMessage> msg = new AMessage();
    AMediaFormat *tmpFormat =  AMediaFormat_fromMsg(&msg);
    media_status_t ret = wrapper->getFormat(wrapper->data, tmpFormat);
    sp<MetaData> newMeta = new MetaData();
    convertMessageToMetaData(msg, newMeta);
    delete tmpFormat;
    format = *newMeta;
    return reverse_translate_error(ret);
}

status_t MediaTrackCUnwrapper::read(MediaBufferBase **buffer, const ReadOptions *options) {

    uint32_t opts = 0;

    if (options && options->getNonBlocking()) {
        opts |= CMediaTrackReadOptions::NONBLOCKING;
    }

    int64_t seekPosition = 0;
    MediaTrack::ReadOptions::SeekMode seekMode;
    if (options && options->getSeekTo(&seekPosition, &seekMode)) {
        opts |= SEEK;
        opts |= (uint32_t) seekMode;
    }
    CMediaBuffer *buf = nullptr;
    media_status_t ret = wrapper->read(wrapper->data, &buf, opts, seekPosition);
    if (ret == AMEDIA_OK && buf != nullptr) {
        *buffer = (MediaBufferBase*)buf->handle;
        MetaDataBase &meta = (*buffer)->meta_data();
        AMediaFormat *format = buf->meta_data(buf->handle);
        // only convert the keys we're actually expecting, as doing
        // the full convertMessageToMetadata() for every buffer is
        // too expensive
        int64_t val64;
        if (format->mFormat->findInt64("timeUs", &val64)) {
            meta.setInt64(kKeyTime, val64);
        }
        if (format->mFormat->findInt64("durationUs", &val64)) {
            meta.setInt64(kKeyDuration, val64);
        }
        if (format->mFormat->findInt64("target-time", &val64)) {
            meta.setInt64(kKeyTargetTime, val64);
        }
        int32_t val32;
        if (format->mFormat->findInt32("is-sync-frame", &val32)) {
            meta.setInt32(kKeyIsSyncFrame, val32);
        }
        if (format->mFormat->findInt32("temporal-layer-id", &val32)) {
            meta.setInt32(kKeyTemporalLayerId, val32);
        }
        if (format->mFormat->findInt32("temporal-layer-count", &val32)) {
            meta.setInt32(kKeyTemporalLayerCount, val32);
        }
        if (format->mFormat->findInt32("crypto-default-iv-size", &val32)) {
            meta.setInt32(kKeyCryptoDefaultIVSize, val32);
        }
        if (format->mFormat->findInt32("crypto-mode", &val32)) {
            meta.setInt32(kKeyCryptoMode, val32);
        }
        if (format->mFormat->findInt32("crypto-encrypted-byte-block", &val32)) {
            meta.setInt32(kKeyEncryptedByteBlock, val32);
        }
        if (format->mFormat->findInt32("crypto-skip-byte-block", &val32)) {
            meta.setInt32(kKeySkipByteBlock, val32);
        }
        if (format->mFormat->findInt32("valid-samples", &val32)) {
            meta.setInt32(kKeyValidSamples, val32);
        }
        sp<ABuffer> valbuf;
        if (format->mFormat->findBuffer("crypto-plain-sizes", &valbuf)) {
            meta.setData(kKeyPlainSizes,
                    MetaDataBase::Type::TYPE_NONE, valbuf->data(), valbuf->size());
        }
        if (format->mFormat->findBuffer("crypto-encrypted-sizes", &valbuf)) {
            meta.setData(kKeyEncryptedSizes,
                    MetaDataBase::Type::TYPE_NONE, valbuf->data(), valbuf->size());
        }
        if (format->mFormat->findBuffer("crypto-key", &valbuf)) {
            meta.setData(kKeyCryptoKey,
                    MetaDataBase::Type::TYPE_NONE, valbuf->data(), valbuf->size());
        }
        if (format->mFormat->findBuffer("crypto-iv", &valbuf)) {
            meta.setData(kKeyCryptoIV,
                    MetaDataBase::Type::TYPE_NONE, valbuf->data(), valbuf->size());
        }
        if (format->mFormat->findBuffer("sei", &valbuf)) {
            meta.setData(kKeySEI,
                    MetaDataBase::Type::TYPE_NONE, valbuf->data(), valbuf->size());
        }
        if (format->mFormat->findBuffer("audio-presentation-info", &valbuf)) {
            meta.setData(kKeyAudioPresentationInfo,
                    MetaDataBase::Type::TYPE_NONE, valbuf->data(), valbuf->size());
        }
    } else {
        *buffer = nullptr;
    }

    return reverse_translate_error(ret);
}

bool MediaTrackCUnwrapper::supportNonblockingRead() {
    return wrapper->supportsNonBlockingRead(wrapper->data);
}

}  // namespace android
