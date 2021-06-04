/*
 * Copyright 2017 The Android Open Source Project
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
#define LOG_TAG "HTTPLiveSource2"
#include <utils/Log.h>

#include "HTTPLiveSource2.h"

#include "AnotherPacketSource.h"
#include "LiveDataSource.h"

#include <media/MediaHTTPService.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/Utils.h>

// default buffer prepare/ready/underflow marks
static const int kReadyMarkMs     = 5000;  // 5 seconds
static const int kPrepareMarkMs   = 1500;  // 1.5 seconds

namespace android {

NuPlayer2::HTTPLiveSource2::HTTPLiveSource2(
        const sp<AMessage> &notify,
        const sp<MediaHTTPService> &httpService,
        const char *url,
        const KeyedVector<String8, String8> *headers)
    : Source(notify),
      mHTTPService(httpService),
      mURL(url),
      mFlags(0),
      mFinalResult(OK),
      mOffset(0),
      mFetchSubtitleDataGeneration(0),
      mFetchMetaDataGeneration(0),
      mHasMetadata(false),
      mMetadataSelected(false) {
    mBufferingSettings.mInitialMarkMs = kPrepareMarkMs;
    mBufferingSettings.mResumePlaybackMarkMs = kReadyMarkMs;
    if (headers) {
        mExtraHeaders = *headers;

        ssize_t index =
            mExtraHeaders.indexOfKey(String8("x-hide-urls-from-log"));

        if (index >= 0) {
            mFlags |= kFlagIncognito;

            mExtraHeaders.removeItemsAt(index);
        }
    }
}

NuPlayer2::HTTPLiveSource2::~HTTPLiveSource2() {
    if (mLiveSession != NULL) {
        mLiveSession->disconnect();

        mLiveLooper->unregisterHandler(mLiveSession->id());
        mLiveLooper->unregisterHandler(id());
        mLiveLooper->stop();

        mLiveSession.clear();
        mLiveLooper.clear();
    }
}

status_t NuPlayer2::HTTPLiveSource2::getBufferingSettings(
            BufferingSettings* buffering /* nonnull */) {
    *buffering = mBufferingSettings;

    return OK;
}

status_t NuPlayer2::HTTPLiveSource2::setBufferingSettings(const BufferingSettings& buffering) {
    mBufferingSettings = buffering;

    if (mLiveSession != NULL) {
        mLiveSession->setBufferingSettings(mBufferingSettings);
    }

    return OK;
}

// TODO: fetch data starting from |startTimeUs|
void NuPlayer2::HTTPLiveSource2::prepareAsync(int64_t /* startTimeUs */) {
    if (mLiveLooper == NULL) {
        mLiveLooper = new ALooper;
        mLiveLooper->setName("http live2");
        mLiveLooper->start(false, /* runOnCallingThread */
                           true /* canCallJava */);

        mLiveLooper->registerHandler(this);
    }

    sp<AMessage> notify = new AMessage(kWhatSessionNotify, this);

    mLiveSession = new LiveSession(
            notify,
            (mFlags & kFlagIncognito) ? LiveSession::kFlagIncognito : 0,
            mHTTPService);

    mLiveLooper->registerHandler(mLiveSession);

    mLiveSession->setBufferingSettings(mBufferingSettings);
    mLiveSession->connectAsync(
            mURL.c_str(), mExtraHeaders.isEmpty() ? NULL : &mExtraHeaders);
}

void NuPlayer2::HTTPLiveSource2::start() {
}

sp<MetaData> NuPlayer2::HTTPLiveSource2::getFormatMeta(bool audio) {
    sp<MetaData> meta;
    if (mLiveSession != NULL) {
        mLiveSession->getStreamFormatMeta(
                audio ? LiveSession::STREAMTYPE_AUDIO
                      : LiveSession::STREAMTYPE_VIDEO,
                &meta);
    }

    return meta;
}

sp<AMessage> NuPlayer2::HTTPLiveSource2::getFormat(bool audio) {
    sp<MetaData> meta;
    status_t err = -EWOULDBLOCK;
    if (mLiveSession != NULL) {
        err = mLiveSession->getStreamFormatMeta(
                audio ? LiveSession::STREAMTYPE_AUDIO
                      : LiveSession::STREAMTYPE_VIDEO,
                &meta);
    }

    sp<AMessage> format;
    if (err == -EWOULDBLOCK) {
        format = new AMessage();
        format->setInt32("err", err);
        return format;
    }

    if (err != OK || convertMetaDataToMessage(meta, &format) != OK) {
        return NULL;
    }
    return format;
}

status_t NuPlayer2::HTTPLiveSource2::feedMoreTSData() {
    return OK;
}

status_t NuPlayer2::HTTPLiveSource2::dequeueAccessUnit(
        bool audio, sp<ABuffer> *accessUnit) {
    return mLiveSession->dequeueAccessUnit(
            audio ? LiveSession::STREAMTYPE_AUDIO
                  : LiveSession::STREAMTYPE_VIDEO,
            accessUnit);
}

status_t NuPlayer2::HTTPLiveSource2::getDuration(int64_t *durationUs) {
    return mLiveSession->getDuration(durationUs);
}

size_t NuPlayer2::HTTPLiveSource2::getTrackCount() const {
    return mLiveSession->getTrackCount();
}

sp<AMessage> NuPlayer2::HTTPLiveSource2::getTrackInfo(size_t trackIndex) const {
    return mLiveSession->getTrackInfo(trackIndex);
}

ssize_t NuPlayer2::HTTPLiveSource2::getSelectedTrack(media_track_type type) const {
    if (mLiveSession == NULL) {
        return -1;
    } else if (type == MEDIA_TRACK_TYPE_METADATA) {
        // MEDIA_TRACK_TYPE_METADATA is always last track
        // mMetadataSelected can only be true when mHasMetadata is true
        return mMetadataSelected ? (mLiveSession->getTrackCount() - 1) : -1;
    } else {
        return mLiveSession->getSelectedTrack(type);
    }
}

status_t NuPlayer2::HTTPLiveSource2::selectTrack(size_t trackIndex, bool select, int64_t /*timeUs*/) {
    if (mLiveSession == NULL) {
        return INVALID_OPERATION;
    }

    status_t err = INVALID_OPERATION;
    bool postFetchMsg = false, isSub = false;
    if (!mHasMetadata || trackIndex != mLiveSession->getTrackCount() - 1) {
        err = mLiveSession->selectTrack(trackIndex, select);
        postFetchMsg = select;
        isSub = true;
    } else {
        // metadata track; i.e. (mHasMetadata && trackIndex == mLiveSession->getTrackCount() - 1)
        if (mMetadataSelected && !select) {
            err = OK;
        } else if (!mMetadataSelected && select) {
            postFetchMsg = true;
            err = OK;
        } else {
            err = BAD_VALUE; // behave as LiveSession::selectTrack
        }

        mMetadataSelected = select;
    }

    if (err == OK) {
        int32_t &generation = isSub ? mFetchSubtitleDataGeneration : mFetchMetaDataGeneration;
        generation++;
        if (postFetchMsg) {
            int32_t what = isSub ? kWhatFetchSubtitleData : kWhatFetchMetaData;
            sp<AMessage> msg = new AMessage(what, this);
            msg->setInt32("generation", generation);
            msg->post();
        }
    }

    // LiveSession::selectTrack returns BAD_VALUE when selecting the currently
    // selected track, or unselecting a non-selected track. In this case it's an
    // no-op so we return OK.
    return (err == OK || err == BAD_VALUE) ? (status_t)OK : err;
}

status_t NuPlayer2::HTTPLiveSource2::seekTo(int64_t seekTimeUs, MediaPlayer2SeekMode mode) {
    if (mLiveSession->isSeekable()) {
        return mLiveSession->seekTo(seekTimeUs, mode);
    } else {
        return INVALID_OPERATION;
    }
}

void NuPlayer2::HTTPLiveSource2::pollForRawData(
        const sp<AMessage> &msg, int32_t currentGeneration,
        LiveSession::StreamType fetchType, int32_t pushWhat) {

    int32_t generation;
    CHECK(msg->findInt32("generation", &generation));

    if (generation != currentGeneration) {
        return;
    }

    sp<ABuffer> buffer;
    while (mLiveSession->dequeueAccessUnit(fetchType, &buffer) == OK) {

        sp<AMessage> notify = dupNotify();
        notify->setInt32("what", pushWhat);
        notify->setBuffer("buffer", buffer);

        int64_t timeUs, baseUs, delayUs;
        CHECK(buffer->meta()->findInt64("baseUs", &baseUs));
        CHECK(buffer->meta()->findInt64("timeUs", &timeUs));
        delayUs = baseUs + timeUs - ALooper::GetNowUs();

        if (fetchType == LiveSession::STREAMTYPE_SUBTITLES) {
            notify->post();
            msg->post(delayUs > 0LL ? delayUs : 0LL);
            return;
        } else if (fetchType == LiveSession::STREAMTYPE_METADATA) {
            if (delayUs < -1000000LL) { // 1 second
                continue;
            }
            notify->post();
            // push all currently available metadata buffers in each invocation of pollForRawData
            // continue;
        } else {
            TRESPASS();
        }
    }

    // try again in 1 second
    msg->post(1000000LL);
}

void NuPlayer2::HTTPLiveSource2::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatSessionNotify:
        {
            onSessionNotify(msg);
            break;
        }

        case kWhatFetchSubtitleData:
        {
            pollForRawData(
                    msg, mFetchSubtitleDataGeneration,
                    /* fetch */ LiveSession::STREAMTYPE_SUBTITLES,
                    /* push */ kWhatSubtitleData);

            break;
        }

        case kWhatFetchMetaData:
        {
            if (!mMetadataSelected) {
                break;
            }

            pollForRawData(
                    msg, mFetchMetaDataGeneration,
                    /* fetch */ LiveSession::STREAMTYPE_METADATA,
                    /* push */ kWhatTimedMetaData);

            break;
        }

        default:
            Source::onMessageReceived(msg);
            break;
    }
}

void NuPlayer2::HTTPLiveSource2::onSessionNotify(const sp<AMessage> &msg) {
    int32_t what;
    CHECK(msg->findInt32("what", &what));

    switch (what) {
        case LiveSession::kWhatPrepared:
        {
            // notify the current size here if we have it, otherwise report an initial size of (0,0)
            sp<AMessage> format = getFormat(false /* audio */);
            int32_t width;
            int32_t height;
            if (format != NULL &&
                    format->findInt32("width", &width) && format->findInt32("height", &height)) {
                notifyVideoSizeChanged(format);
            } else {
                notifyVideoSizeChanged();
            }

            uint32_t flags = 0;
            if (mLiveSession->isSeekable()) {
                flags |= FLAG_CAN_PAUSE;
                flags |= FLAG_CAN_SEEK;
                flags |= FLAG_CAN_SEEK_BACKWARD;
                flags |= FLAG_CAN_SEEK_FORWARD;
            }

            if (mLiveSession->hasDynamicDuration()) {
                flags |= FLAG_DYNAMIC_DURATION;
            }

            notifyFlagsChanged(flags);

            notifyPrepared();
            break;
        }

        case LiveSession::kWhatPreparationFailed:
        {
            status_t err;
            CHECK(msg->findInt32("err", &err));

            notifyPrepared(err);
            break;
        }

        case LiveSession::kWhatStreamsChanged:
        {
            uint32_t changedMask;
            CHECK(msg->findInt32(
                        "changedMask", (int32_t *)&changedMask));

            bool audio = changedMask & LiveSession::STREAMTYPE_AUDIO;
            bool video = changedMask & LiveSession::STREAMTYPE_VIDEO;

            sp<AMessage> reply;
            CHECK(msg->findMessage("reply", &reply));

            sp<AMessage> notify = dupNotify();
            notify->setInt32("what", kWhatQueueDecoderShutdown);
            notify->setInt32("audio", audio);
            notify->setInt32("video", video);
            notify->setMessage("reply", reply);
            notify->post();
            break;
        }

        case LiveSession::kWhatBufferingStart:
        {
            sp<AMessage> notify = dupNotify();
            notify->setInt32("what", kWhatPauseOnBufferingStart);
            notify->post();
            break;
        }

        case LiveSession::kWhatBufferingEnd:
        {
            sp<AMessage> notify = dupNotify();
            notify->setInt32("what", kWhatResumeOnBufferingEnd);
            notify->post();
            break;
        }


        case LiveSession::kWhatBufferingUpdate:
        {
            sp<AMessage> notify = dupNotify();
            int32_t percentage;
            CHECK(msg->findInt32("percentage", &percentage));
            notify->setInt32("what", kWhatBufferingUpdate);
            notify->setInt32("percentage", percentage);
            notify->post();
            break;
        }

        case LiveSession::kWhatMetadataDetected:
        {
            if (!mHasMetadata) {
                mHasMetadata = true;

                sp<AMessage> notify = dupNotify();
                // notification without buffer triggers MEDIA2_INFO_METADATA_UPDATE
                notify->setInt32("what", kWhatTimedMetaData);
                notify->post();
            }
            break;
        }

        case LiveSession::kWhatError:
        {
            break;
        }

        default:
            TRESPASS();
    }
}

}  // namespace android

