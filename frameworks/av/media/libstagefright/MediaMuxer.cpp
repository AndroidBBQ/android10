/*
 * Copyright 2013, The Android Open Source Project
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
#define LOG_TAG "MediaMuxer"

#include "webm/WebmWriter.h"

#include <utils/Log.h>

#include <media/stagefright/MediaMuxer.h>

#include <media/mediarecorder.h>
#include <media/MediaSource.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaAdapter.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MPEG4Writer.h>
#include <media/stagefright/OggWriter.h>
#include <media/stagefright/Utils.h>

namespace android {

static bool isMp4Format(MediaMuxer::OutputFormat format) {
    return format == MediaMuxer::OUTPUT_FORMAT_MPEG_4 ||
           format == MediaMuxer::OUTPUT_FORMAT_THREE_GPP ||
           format == MediaMuxer::OUTPUT_FORMAT_HEIF;
}

MediaMuxer::MediaMuxer(int fd, OutputFormat format)
    : mFormat(format),
      mState(UNINITIALIZED) {
    if (isMp4Format(format)) {
        mWriter = new MPEG4Writer(fd);
    } else if (format == OUTPUT_FORMAT_WEBM) {
        mWriter = new WebmWriter(fd);
    } else if (format == OUTPUT_FORMAT_OGG) {
        mWriter = new OggWriter(fd);
    }

    if (mWriter != NULL) {
        mFileMeta = new MetaData;
        if (format == OUTPUT_FORMAT_HEIF) {
            // Note that the key uses recorder file types.
            mFileMeta->setInt32(kKeyFileType, output_format::OUTPUT_FORMAT_HEIF);
        } else if (format == OUTPUT_FORMAT_OGG) {
            mFileMeta->setInt32(kKeyFileType, output_format::OUTPUT_FORMAT_OGG);
        }
        mState = INITIALIZED;
    }
}

MediaMuxer::~MediaMuxer() {
    Mutex::Autolock autoLock(mMuxerLock);

    // Clean up all the internal resources.
    mFileMeta.clear();
    mWriter.clear();
    mTrackList.clear();
}

ssize_t MediaMuxer::addTrack(const sp<AMessage> &format) {
    Mutex::Autolock autoLock(mMuxerLock);

    if (format.get() == NULL) {
        ALOGE("addTrack() get a null format");
        return -EINVAL;
    }

    if (mState != INITIALIZED) {
        ALOGE("addTrack() must be called after constructor and before start().");
        return INVALID_OPERATION;
    }

    sp<MetaData> trackMeta = new MetaData;
    convertMessageToMetaData(format, trackMeta);

    sp<MediaAdapter> newTrack = new MediaAdapter(trackMeta);
    status_t result = mWriter->addSource(newTrack);
    if (result != OK) {
        return -1;
    }
    float captureFps = -1.0;
    if (format->findAsFloat("time-lapse-fps", &captureFps)) {
        ALOGV("addTrack() time-lapse-fps: %f", captureFps);
        result = mWriter->setCaptureRate(captureFps);
        if (result != OK) {
            ALOGW("addTrack() setCaptureRate failed :%d", result);
        }
    }
    return mTrackList.add(newTrack);
}

status_t MediaMuxer::setOrientationHint(int degrees) {
    Mutex::Autolock autoLock(mMuxerLock);
    if (mState != INITIALIZED) {
        ALOGE("setOrientationHint() must be called before start().");
        return INVALID_OPERATION;
    }

    if (degrees != 0 && degrees != 90 && degrees != 180 && degrees != 270) {
        ALOGE("setOrientationHint() get invalid degrees");
        return -EINVAL;
    }

    mFileMeta->setInt32(kKeyRotation, degrees);
    return OK;
}

status_t MediaMuxer::setLocation(int latitude, int longitude) {
    Mutex::Autolock autoLock(mMuxerLock);
    if (mState != INITIALIZED) {
        ALOGE("setLocation() must be called before start().");
        return INVALID_OPERATION;
    }
    if (!isMp4Format(mFormat)) {
        ALOGE("setLocation() is only supported for .mp4, .3gp or .heic output.");
        return INVALID_OPERATION;
    }

    ALOGV("Setting location: latitude = %d, longitude = %d", latitude, longitude);
    return static_cast<MPEG4Writer*>(mWriter.get())->setGeoData(latitude, longitude);
}

status_t MediaMuxer::start() {
    Mutex::Autolock autoLock(mMuxerLock);
    if (mState == INITIALIZED) {
        mState = STARTED;
        mFileMeta->setInt32(kKeyRealTimeRecording, false);
        return mWriter->start(mFileMeta.get());
    } else {
        ALOGE("start() is called in invalid state %d", mState);
        return INVALID_OPERATION;
    }
}

status_t MediaMuxer::stop() {
    Mutex::Autolock autoLock(mMuxerLock);

    if (mState == STARTED) {
        mState = STOPPED;
        for (size_t i = 0; i < mTrackList.size(); i++) {
            if (mTrackList[i]->stop() != OK) {
                return INVALID_OPERATION;
            }
        }
        return mWriter->stop();
    } else {
        ALOGE("stop() is called in invalid state %d", mState);
        return INVALID_OPERATION;
    }
}

status_t MediaMuxer::writeSampleData(const sp<ABuffer> &buffer, size_t trackIndex,
                                     int64_t timeUs, uint32_t flags) {
    Mutex::Autolock autoLock(mMuxerLock);

    if (buffer.get() == NULL) {
        ALOGE("WriteSampleData() get an NULL buffer.");
        return -EINVAL;
    }

    if (mState != STARTED) {
        ALOGE("WriteSampleData() is called in invalid state %d", mState);
        return INVALID_OPERATION;
    }

    if (trackIndex >= mTrackList.size()) {
        ALOGE("WriteSampleData() get an invalid index %zu", trackIndex);
        return -EINVAL;
    }

    MediaBuffer* mediaBuffer = new MediaBuffer(buffer);

    mediaBuffer->add_ref(); // Released in MediaAdapter::signalBufferReturned().
    mediaBuffer->set_range(buffer->offset(), buffer->size());

    MetaDataBase &sampleMetaData = mediaBuffer->meta_data();
    sampleMetaData.setInt64(kKeyTime, timeUs);
    // Just set the kKeyDecodingTime as the presentation time for now.
    sampleMetaData.setInt64(kKeyDecodingTime, timeUs);

    if (flags & MediaCodec::BUFFER_FLAG_SYNCFRAME) {
        sampleMetaData.setInt32(kKeyIsSyncFrame, true);
    }

    if (flags & MediaCodec::BUFFER_FLAG_MUXER_DATA) {
        sampleMetaData.setInt32(kKeyIsMuxerData, 1);
    }

    sp<MediaAdapter> currentTrack = mTrackList[trackIndex];
    // This pushBuffer will wait until the mediaBuffer is consumed.
    return currentTrack->pushBuffer(mediaBuffer);
}

}  // namespace android
