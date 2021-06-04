/*
 * Copyright (C) 2010 The Android Open Source Project
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
#define LOG_TAG "NuPlayer"

#include <inttypes.h>

#include <utils/Log.h>

#include "NuPlayer.h"

#include "HTTPLiveSource.h"
#include "NuPlayerCCDecoder.h"
#include "NuPlayerDecoder.h"
#include "NuPlayerDecoderBase.h"
#include "NuPlayerDecoderPassThrough.h"
#include "NuPlayerDriver.h"
#include "NuPlayerRenderer.h"
#include "NuPlayerSource.h"
#include "RTSPSource.h"
#include "StreamingSource.h"
#include "GenericSource.h"
#include "TextDescriptions.h"

#include "ATSParser.h"

#include <cutils/properties.h>

#include <media/AudioResamplerPublic.h>
#include <media/AVSyncSettings.h>
#include <media/MediaCodecBuffer.h>

#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaClock.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>

#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>


#include "ESDS.h"
#include <media/stagefright/Utils.h>

namespace android {

struct NuPlayer::Action : public RefBase {
    Action() {}

    virtual void execute(NuPlayer *player) = 0;

private:
    DISALLOW_EVIL_CONSTRUCTORS(Action);
};

struct NuPlayer::SeekAction : public Action {
    explicit SeekAction(int64_t seekTimeUs, MediaPlayerSeekMode mode)
        : mSeekTimeUs(seekTimeUs),
          mMode(mode) {
    }

    virtual void execute(NuPlayer *player) {
        player->performSeek(mSeekTimeUs, mMode);
    }

private:
    int64_t mSeekTimeUs;
    MediaPlayerSeekMode mMode;

    DISALLOW_EVIL_CONSTRUCTORS(SeekAction);
};

struct NuPlayer::ResumeDecoderAction : public Action {
    explicit ResumeDecoderAction(bool needNotify)
        : mNeedNotify(needNotify) {
    }

    virtual void execute(NuPlayer *player) {
        player->performResumeDecoders(mNeedNotify);
    }

private:
    bool mNeedNotify;

    DISALLOW_EVIL_CONSTRUCTORS(ResumeDecoderAction);
};

struct NuPlayer::SetSurfaceAction : public Action {
    explicit SetSurfaceAction(const sp<Surface> &surface)
        : mSurface(surface) {
    }

    virtual void execute(NuPlayer *player) {
        player->performSetSurface(mSurface);
    }

private:
    sp<Surface> mSurface;

    DISALLOW_EVIL_CONSTRUCTORS(SetSurfaceAction);
};

struct NuPlayer::FlushDecoderAction : public Action {
    FlushDecoderAction(FlushCommand audio, FlushCommand video)
        : mAudio(audio),
          mVideo(video) {
    }

    virtual void execute(NuPlayer *player) {
        player->performDecoderFlush(mAudio, mVideo);
    }

private:
    FlushCommand mAudio;
    FlushCommand mVideo;

    DISALLOW_EVIL_CONSTRUCTORS(FlushDecoderAction);
};

struct NuPlayer::PostMessageAction : public Action {
    explicit PostMessageAction(const sp<AMessage> &msg)
        : mMessage(msg) {
    }

    virtual void execute(NuPlayer *) {
        mMessage->post();
    }

private:
    sp<AMessage> mMessage;

    DISALLOW_EVIL_CONSTRUCTORS(PostMessageAction);
};

// Use this if there's no state necessary to save in order to execute
// the action.
struct NuPlayer::SimpleAction : public Action {
    typedef void (NuPlayer::*ActionFunc)();

    explicit SimpleAction(ActionFunc func)
        : mFunc(func) {
    }

    virtual void execute(NuPlayer *player) {
        (player->*mFunc)();
    }

private:
    ActionFunc mFunc;

    DISALLOW_EVIL_CONSTRUCTORS(SimpleAction);
};

////////////////////////////////////////////////////////////////////////////////

NuPlayer::NuPlayer(pid_t pid, const sp<MediaClock> &mediaClock)
    : mUIDValid(false),
      mPID(pid),
      mMediaClock(mediaClock),
      mSourceFlags(0),
      mOffloadAudio(false),
      mAudioDecoderGeneration(0),
      mVideoDecoderGeneration(0),
      mRendererGeneration(0),
      mLastStartedPlayingTimeNs(0),
      mLastStartedRebufferingTimeNs(0),
      mPreviousSeekTimeUs(0),
      mAudioEOS(false),
      mVideoEOS(false),
      mScanSourcesPending(false),
      mScanSourcesGeneration(0),
      mPollDurationGeneration(0),
      mTimedTextGeneration(0),
      mFlushingAudio(NONE),
      mFlushingVideo(NONE),
      mResumePending(false),
      mVideoScalingMode(NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW),
      mPlaybackSettings(AUDIO_PLAYBACK_RATE_DEFAULT),
      mVideoFpsHint(-1.f),
      mStarted(false),
      mPrepared(false),
      mResetting(false),
      mSourceStarted(false),
      mAudioDecoderError(false),
      mVideoDecoderError(false),
      mPaused(false),
      mPausedByClient(true),
      mPausedForBuffering(false),
      mIsDrmProtected(false),
      mDataSourceType(DATA_SOURCE_TYPE_NONE) {
    CHECK(mediaClock != NULL);
    clearFlushComplete();
}

NuPlayer::~NuPlayer() {
}

void NuPlayer::setUID(uid_t uid) {
    mUIDValid = true;
    mUID = uid;
}

void NuPlayer::init(const wp<NuPlayerDriver> &driver) {
    mDriver = driver;

    sp<AMessage> notify = new AMessage(kWhatMediaClockNotify, this);
    mMediaClock->setNotificationMessage(notify);
}

void NuPlayer::setDataSourceAsync(const sp<IStreamSource> &source) {
    sp<AMessage> msg = new AMessage(kWhatSetDataSource, this);

    sp<AMessage> notify = new AMessage(kWhatSourceNotify, this);

    msg->setObject("source", new StreamingSource(notify, source));
    msg->post();
    mDataSourceType = DATA_SOURCE_TYPE_STREAM;
}

static bool IsHTTPLiveURL(const char *url) {
    if (!strncasecmp("http://", url, 7)
            || !strncasecmp("https://", url, 8)
            || !strncasecmp("file://", url, 7)) {
        size_t len = strlen(url);
        if (len >= 5 && !strcasecmp(".m3u8", &url[len - 5])) {
            return true;
        }

        if (strstr(url,"m3u8")) {
            return true;
        }
    }

    return false;
}

void NuPlayer::setDataSourceAsync(
        const sp<IMediaHTTPService> &httpService,
        const char *url,
        const KeyedVector<String8, String8> *headers) {

    sp<AMessage> msg = new AMessage(kWhatSetDataSource, this);
    size_t len = strlen(url);

    sp<AMessage> notify = new AMessage(kWhatSourceNotify, this);

    sp<Source> source;
    if (IsHTTPLiveURL(url)) {
        source = new HTTPLiveSource(notify, httpService, url, headers);
        ALOGV("setDataSourceAsync HTTPLiveSource %s", url);
        mDataSourceType = DATA_SOURCE_TYPE_HTTP_LIVE;
    } else if (!strncasecmp(url, "rtsp://", 7)) {
        source = new RTSPSource(
                notify, httpService, url, headers, mUIDValid, mUID);
        ALOGV("setDataSourceAsync RTSPSource %s", url);
        mDataSourceType = DATA_SOURCE_TYPE_RTSP;
    } else if ((!strncasecmp(url, "http://", 7)
                || !strncasecmp(url, "https://", 8))
                    && ((len >= 4 && !strcasecmp(".sdp", &url[len - 4]))
                    || strstr(url, ".sdp?"))) {
        source = new RTSPSource(
                notify, httpService, url, headers, mUIDValid, mUID, true);
        ALOGV("setDataSourceAsync RTSPSource http/https/.sdp %s", url);
        mDataSourceType = DATA_SOURCE_TYPE_RTSP;
    } else {
        ALOGV("setDataSourceAsync GenericSource %s", url);

        sp<GenericSource> genericSource =
                new GenericSource(notify, mUIDValid, mUID, mMediaClock);

        status_t err = genericSource->setDataSource(httpService, url, headers);

        if (err == OK) {
            source = genericSource;
        } else {
            ALOGE("Failed to set data source!");
        }

        // regardless of success/failure
        mDataSourceType = DATA_SOURCE_TYPE_GENERIC_URL;
    }
    msg->setObject("source", source);
    msg->post();
}

void NuPlayer::setDataSourceAsync(int fd, int64_t offset, int64_t length) {
    sp<AMessage> msg = new AMessage(kWhatSetDataSource, this);

    sp<AMessage> notify = new AMessage(kWhatSourceNotify, this);

    sp<GenericSource> source =
            new GenericSource(notify, mUIDValid, mUID, mMediaClock);

    ALOGV("setDataSourceAsync fd %d/%lld/%lld source: %p",
            fd, (long long)offset, (long long)length, source.get());

    status_t err = source->setDataSource(fd, offset, length);

    if (err != OK) {
        ALOGE("Failed to set data source!");
        source = NULL;
    }

    msg->setObject("source", source);
    msg->post();
    mDataSourceType = DATA_SOURCE_TYPE_GENERIC_FD;
}

void NuPlayer::setDataSourceAsync(const sp<DataSource> &dataSource) {
    sp<AMessage> msg = new AMessage(kWhatSetDataSource, this);
    sp<AMessage> notify = new AMessage(kWhatSourceNotify, this);

    sp<GenericSource> source = new GenericSource(notify, mUIDValid, mUID, mMediaClock);
    status_t err = source->setDataSource(dataSource);

    if (err != OK) {
        ALOGE("Failed to set data source!");
        source = NULL;
    }

    msg->setObject("source", source);
    msg->post();
    mDataSourceType = DATA_SOURCE_TYPE_MEDIA;
}

status_t NuPlayer::getBufferingSettings(
        BufferingSettings *buffering /* nonnull */) {
    sp<AMessage> msg = new AMessage(kWhatGetBufferingSettings, this);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    if (err == OK && response != NULL) {
        CHECK(response->findInt32("err", &err));
        if (err == OK) {
            readFromAMessage(response, buffering);
        }
    }
    return err;
}

status_t NuPlayer::setBufferingSettings(const BufferingSettings& buffering) {
    sp<AMessage> msg = new AMessage(kWhatSetBufferingSettings, this);
    writeToAMessage(msg, buffering);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    if (err == OK && response != NULL) {
        CHECK(response->findInt32("err", &err));
    }
    return err;
}

void NuPlayer::prepareAsync() {
    ALOGV("prepareAsync");

    (new AMessage(kWhatPrepare, this))->post();
}

void NuPlayer::setVideoSurfaceTextureAsync(
        const sp<IGraphicBufferProducer> &bufferProducer) {
    sp<AMessage> msg = new AMessage(kWhatSetVideoSurface, this);

    if (bufferProducer == NULL) {
        msg->setObject("surface", NULL);
    } else {
        msg->setObject("surface", new Surface(bufferProducer, true /* controlledByApp */));
    }

    msg->post();
}

void NuPlayer::setAudioSink(const sp<MediaPlayerBase::AudioSink> &sink) {
    sp<AMessage> msg = new AMessage(kWhatSetAudioSink, this);
    msg->setObject("sink", sink);
    msg->post();
}

void NuPlayer::start() {
    (new AMessage(kWhatStart, this))->post();
}

status_t NuPlayer::setPlaybackSettings(const AudioPlaybackRate &rate) {
    // do some cursory validation of the settings here. audio modes are
    // only validated when set on the audiosink.
     if ((rate.mSpeed != 0.f && rate.mSpeed < AUDIO_TIMESTRETCH_SPEED_MIN)
            || rate.mSpeed > AUDIO_TIMESTRETCH_SPEED_MAX
            || rate.mPitch < AUDIO_TIMESTRETCH_SPEED_MIN
            || rate.mPitch > AUDIO_TIMESTRETCH_SPEED_MAX) {
        return BAD_VALUE;
    }
    sp<AMessage> msg = new AMessage(kWhatConfigPlayback, this);
    writeToAMessage(msg, rate);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    if (err == OK && response != NULL) {
        CHECK(response->findInt32("err", &err));
    }
    return err;
}

status_t NuPlayer::getPlaybackSettings(AudioPlaybackRate *rate /* nonnull */) {
    sp<AMessage> msg = new AMessage(kWhatGetPlaybackSettings, this);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    if (err == OK && response != NULL) {
        CHECK(response->findInt32("err", &err));
        if (err == OK) {
            readFromAMessage(response, rate);
        }
    }
    return err;
}

status_t NuPlayer::setSyncSettings(const AVSyncSettings &sync, float videoFpsHint) {
    sp<AMessage> msg = new AMessage(kWhatConfigSync, this);
    writeToAMessage(msg, sync, videoFpsHint);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    if (err == OK && response != NULL) {
        CHECK(response->findInt32("err", &err));
    }
    return err;
}

status_t NuPlayer::getSyncSettings(
        AVSyncSettings *sync /* nonnull */, float *videoFps /* nonnull */) {
    sp<AMessage> msg = new AMessage(kWhatGetSyncSettings, this);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    if (err == OK && response != NULL) {
        CHECK(response->findInt32("err", &err));
        if (err == OK) {
            readFromAMessage(response, sync, videoFps);
        }
    }
    return err;
}

void NuPlayer::pause() {
    (new AMessage(kWhatPause, this))->post();
}

void NuPlayer::resetAsync() {
    sp<Source> source;
    {
        Mutex::Autolock autoLock(mSourceLock);
        source = mSource;
    }

    if (source != NULL) {
        // During a reset, the data source might be unresponsive already, we need to
        // disconnect explicitly so that reads exit promptly.
        // We can't queue the disconnect request to the looper, as it might be
        // queued behind a stuck read and never gets processed.
        // Doing a disconnect outside the looper to allows the pending reads to exit
        // (either successfully or with error).
        source->disconnect();
    }

    (new AMessage(kWhatReset, this))->post();
}

status_t NuPlayer::notifyAt(int64_t mediaTimeUs) {
    sp<AMessage> notify = new AMessage(kWhatNotifyTime, this);
    notify->setInt64("timerUs", mediaTimeUs);
    mMediaClock->addTimer(notify, mediaTimeUs);
    return OK;
}

void NuPlayer::seekToAsync(int64_t seekTimeUs, MediaPlayerSeekMode mode, bool needNotify) {
    sp<AMessage> msg = new AMessage(kWhatSeek, this);
    msg->setInt64("seekTimeUs", seekTimeUs);
    msg->setInt32("mode", mode);
    msg->setInt32("needNotify", needNotify);
    msg->post();
}


void NuPlayer::writeTrackInfo(
        Parcel* reply, const sp<AMessage>& format) const {
    if (format == NULL) {
        ALOGE("NULL format");
        return;
    }
    int32_t trackType;
    if (!format->findInt32("type", &trackType)) {
        ALOGE("no track type");
        return;
    }

    AString mime;
    if (!format->findString("mime", &mime)) {
        // Java MediaPlayer only uses mimetype for subtitle and timedtext tracks.
        // If we can't find the mimetype here it means that we wouldn't be needing
        // the mimetype on the Java end. We still write a placeholder mime to keep the
        // (de)serialization logic simple.
        if (trackType == MEDIA_TRACK_TYPE_AUDIO) {
            mime = "audio/";
        } else if (trackType == MEDIA_TRACK_TYPE_VIDEO) {
            mime = "video/";
        } else {
            ALOGE("unknown track type: %d", trackType);
            return;
        }
    }

    AString lang;
    if (!format->findString("language", &lang)) {
        ALOGE("no language");
        return;
    }

    reply->writeInt32(2); // write something non-zero
    reply->writeInt32(trackType);
    reply->writeString16(String16(mime.c_str()));
    reply->writeString16(String16(lang.c_str()));

    if (trackType == MEDIA_TRACK_TYPE_SUBTITLE) {
        int32_t isAuto, isDefault, isForced;
        CHECK(format->findInt32("auto", &isAuto));
        CHECK(format->findInt32("default", &isDefault));
        CHECK(format->findInt32("forced", &isForced));

        reply->writeInt32(isAuto);
        reply->writeInt32(isDefault);
        reply->writeInt32(isForced);
    }
}

void NuPlayer::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatSetDataSource:
        {
            ALOGV("kWhatSetDataSource");

            CHECK(mSource == NULL);

            status_t err = OK;
            sp<RefBase> obj;
            CHECK(msg->findObject("source", &obj));
            if (obj != NULL) {
                Mutex::Autolock autoLock(mSourceLock);
                mSource = static_cast<Source *>(obj.get());
            } else {
                err = UNKNOWN_ERROR;
            }

            CHECK(mDriver != NULL);
            sp<NuPlayerDriver> driver = mDriver.promote();
            if (driver != NULL) {
                driver->notifySetDataSourceCompleted(err);
            }
            break;
        }

        case kWhatGetBufferingSettings:
        {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            ALOGV("kWhatGetBufferingSettings");
            BufferingSettings buffering;
            status_t err = OK;
            if (mSource != NULL) {
                err = mSource->getBufferingSettings(&buffering);
            } else {
                err = INVALID_OPERATION;
            }
            sp<AMessage> response = new AMessage;
            if (err == OK) {
                writeToAMessage(response, buffering);
            }
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        case kWhatSetBufferingSettings:
        {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            ALOGV("kWhatSetBufferingSettings");
            BufferingSettings buffering;
            readFromAMessage(msg, &buffering);
            status_t err = OK;
            if (mSource != NULL) {
                err = mSource->setBufferingSettings(buffering);
            } else {
                err = INVALID_OPERATION;
            }
            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        case kWhatPrepare:
        {
            ALOGV("onMessageReceived kWhatPrepare");

            mSource->prepareAsync();
            break;
        }

        case kWhatGetTrackInfo:
        {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            Parcel* reply;
            CHECK(msg->findPointer("reply", (void**)&reply));

            size_t inbandTracks = 0;
            if (mSource != NULL) {
                inbandTracks = mSource->getTrackCount();
            }

            size_t ccTracks = 0;
            if (mCCDecoder != NULL) {
                ccTracks = mCCDecoder->getTrackCount();
            }

            // total track count
            reply->writeInt32(inbandTracks + ccTracks);

            // write inband tracks
            for (size_t i = 0; i < inbandTracks; ++i) {
                writeTrackInfo(reply, mSource->getTrackInfo(i));
            }

            // write CC track
            for (size_t i = 0; i < ccTracks; ++i) {
                writeTrackInfo(reply, mCCDecoder->getTrackInfo(i));
            }

            sp<AMessage> response = new AMessage;
            response->postReply(replyID);
            break;
        }

        case kWhatGetSelectedTrack:
        {
            int32_t type32;
            CHECK(msg->findInt32("type", (int32_t*)&type32));
            media_track_type type = (media_track_type)type32;

            size_t inbandTracks = 0;
            status_t err = INVALID_OPERATION;
            ssize_t selectedTrack = -1;
            if (mSource != NULL) {
                err = OK;
                inbandTracks = mSource->getTrackCount();
                selectedTrack = mSource->getSelectedTrack(type);
            }

            if (selectedTrack == -1 && mCCDecoder != NULL) {
                err = OK;
                selectedTrack = mCCDecoder->getSelectedTrack(type);
                if (selectedTrack != -1) {
                    selectedTrack += inbandTracks;
                }
            }

            Parcel* reply;
            CHECK(msg->findPointer("reply", (void**)&reply));
            reply->writeInt32(selectedTrack);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);

            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));
            response->postReply(replyID);
            break;
        }

        case kWhatSelectTrack:
        {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            size_t trackIndex;
            int32_t select;
            int64_t timeUs;
            CHECK(msg->findSize("trackIndex", &trackIndex));
            CHECK(msg->findInt32("select", &select));
            CHECK(msg->findInt64("timeUs", &timeUs));

            status_t err = INVALID_OPERATION;

            size_t inbandTracks = 0;
            if (mSource != NULL) {
                inbandTracks = mSource->getTrackCount();
            }
            size_t ccTracks = 0;
            if (mCCDecoder != NULL) {
                ccTracks = mCCDecoder->getTrackCount();
            }

            if (trackIndex < inbandTracks) {
                err = mSource->selectTrack(trackIndex, select, timeUs);

                if (!select && err == OK) {
                    int32_t type;
                    sp<AMessage> info = mSource->getTrackInfo(trackIndex);
                    if (info != NULL
                            && info->findInt32("type", &type)
                            && type == MEDIA_TRACK_TYPE_TIMEDTEXT) {
                        ++mTimedTextGeneration;
                    }
                }
            } else {
                trackIndex -= inbandTracks;

                if (trackIndex < ccTracks) {
                    err = mCCDecoder->selectTrack(trackIndex, select);
                }
            }

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);

            response->postReply(replyID);
            break;
        }

        case kWhatPollDuration:
        {
            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));

            if (generation != mPollDurationGeneration) {
                // stale
                break;
            }

            int64_t durationUs;
            if (mDriver != NULL && mSource->getDuration(&durationUs) == OK) {
                sp<NuPlayerDriver> driver = mDriver.promote();
                if (driver != NULL) {
                    driver->notifyDuration(durationUs);
                }
            }

            msg->post(1000000LL);  // poll again in a second.
            break;
        }

        case kWhatSetVideoSurface:
        {

            sp<RefBase> obj;
            CHECK(msg->findObject("surface", &obj));
            sp<Surface> surface = static_cast<Surface *>(obj.get());

            ALOGD("onSetVideoSurface(%p, %s video decoder)",
                    surface.get(),
                    (mSource != NULL && mStarted && mSource->getFormat(false /* audio */) != NULL
                            && mVideoDecoder != NULL) ? "have" : "no");

            // Need to check mStarted before calling mSource->getFormat because NuPlayer might
            // be in preparing state and it could take long time.
            // When mStarted is true, mSource must have been set.
            if (mSource == NULL || !mStarted || mSource->getFormat(false /* audio */) == NULL
                    // NOTE: mVideoDecoder's mSurface is always non-null
                    || (mVideoDecoder != NULL && mVideoDecoder->setVideoSurface(surface) == OK)) {
                performSetSurface(surface);
                break;
            }

            mDeferredActions.push_back(
                    new FlushDecoderAction(
                            (obj != NULL ? FLUSH_CMD_FLUSH : FLUSH_CMD_NONE) /* audio */,
                                           FLUSH_CMD_SHUTDOWN /* video */));

            mDeferredActions.push_back(new SetSurfaceAction(surface));

            if (obj != NULL) {
                if (mStarted) {
                    // Issue a seek to refresh the video screen only if started otherwise
                    // the extractor may not yet be started and will assert.
                    // If the video decoder is not set (perhaps audio only in this case)
                    // do not perform a seek as it is not needed.
                    int64_t currentPositionUs = 0;
                    if (getCurrentPosition(&currentPositionUs) == OK) {
                        mDeferredActions.push_back(
                                new SeekAction(currentPositionUs,
                                        MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC /* mode */));
                    }
                }

                // If there is a new surface texture, instantiate decoders
                // again if possible.
                mDeferredActions.push_back(
                        new SimpleAction(&NuPlayer::performScanSources));

                // After a flush without shutdown, decoder is paused.
                // Don't resume it until source seek is done, otherwise it could
                // start pulling stale data too soon.
                mDeferredActions.push_back(
                        new ResumeDecoderAction(false /* needNotify */));
            }

            processDeferredActions();
            break;
        }

        case kWhatSetAudioSink:
        {
            ALOGV("kWhatSetAudioSink");

            sp<RefBase> obj;
            CHECK(msg->findObject("sink", &obj));

            mAudioSink = static_cast<MediaPlayerBase::AudioSink *>(obj.get());
            break;
        }

        case kWhatStart:
        {
            ALOGV("kWhatStart");
            if (mStarted) {
                // do not resume yet if the source is still buffering
                if (!mPausedForBuffering) {
                    onResume();
                }
            } else {
                onStart();
            }
            mPausedByClient = false;
            break;
        }

        case kWhatConfigPlayback:
        {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));
            AudioPlaybackRate rate /* sanitized */;
            readFromAMessage(msg, &rate);
            status_t err = OK;
            if (mRenderer != NULL) {
                // AudioSink allows only 1.f and 0.f for offload mode.
                // For other speed, switch to non-offload mode.
                if (mOffloadAudio && ((rate.mSpeed != 0.f && rate.mSpeed != 1.f)
                        || rate.mPitch != 1.f)) {
                    int64_t currentPositionUs;
                    if (getCurrentPosition(&currentPositionUs) != OK) {
                        currentPositionUs = mPreviousSeekTimeUs;
                    }

                    // Set mPlaybackSettings so that the new audio decoder can
                    // be created correctly.
                    mPlaybackSettings = rate;
                    if (!mPaused) {
                        mRenderer->pause();
                    }
                    restartAudio(
                            currentPositionUs, true /* forceNonOffload */,
                            true /* needsToCreateAudioDecoder */);
                    if (!mPaused) {
                        mRenderer->resume();
                    }
                }

                err = mRenderer->setPlaybackSettings(rate);
            }
            if (err == OK) {
                if (rate.mSpeed == 0.f) {
                    onPause();
                    mPausedByClient = true;
                    // save all other settings (using non-paused speed)
                    // so we can restore them on start
                    AudioPlaybackRate newRate = rate;
                    newRate.mSpeed = mPlaybackSettings.mSpeed;
                    mPlaybackSettings = newRate;
                } else { /* rate.mSpeed != 0.f */
                    mPlaybackSettings = rate;
                    if (mStarted) {
                        // do not resume yet if the source is still buffering
                        if (!mPausedForBuffering) {
                            onResume();
                        }
                    } else if (mPrepared) {
                        onStart();
                    }

                    mPausedByClient = false;
                }
            }

            if (mVideoDecoder != NULL) {
                sp<AMessage> params = new AMessage();
                params->setFloat("playback-speed", mPlaybackSettings.mSpeed);
                mVideoDecoder->setParameters(params);
            }

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        case kWhatGetPlaybackSettings:
        {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));
            AudioPlaybackRate rate = mPlaybackSettings;
            status_t err = OK;
            if (mRenderer != NULL) {
                err = mRenderer->getPlaybackSettings(&rate);
            }
            if (err == OK) {
                // get playback settings used by renderer, as it may be
                // slightly off due to audiosink not taking small changes.
                mPlaybackSettings = rate;
                if (mPaused) {
                    rate.mSpeed = 0.f;
                }
            }
            sp<AMessage> response = new AMessage;
            if (err == OK) {
                writeToAMessage(response, rate);
            }
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        case kWhatConfigSync:
        {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            ALOGV("kWhatConfigSync");
            AVSyncSettings sync;
            float videoFpsHint;
            readFromAMessage(msg, &sync, &videoFpsHint);
            status_t err = OK;
            if (mRenderer != NULL) {
                err = mRenderer->setSyncSettings(sync, videoFpsHint);
            }
            if (err == OK) {
                mSyncSettings = sync;
                mVideoFpsHint = videoFpsHint;
            }
            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        case kWhatGetSyncSettings:
        {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));
            AVSyncSettings sync = mSyncSettings;
            float videoFps = mVideoFpsHint;
            status_t err = OK;
            if (mRenderer != NULL) {
                err = mRenderer->getSyncSettings(&sync, &videoFps);
                if (err == OK) {
                    mSyncSettings = sync;
                    mVideoFpsHint = videoFps;
                }
            }
            sp<AMessage> response = new AMessage;
            if (err == OK) {
                writeToAMessage(response, sync, videoFps);
            }
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        case kWhatScanSources:
        {
            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));
            if (generation != mScanSourcesGeneration) {
                // Drop obsolete msg.
                break;
            }

            mScanSourcesPending = false;

            ALOGV("scanning sources haveAudio=%d, haveVideo=%d",
                 mAudioDecoder != NULL, mVideoDecoder != NULL);

            bool mHadAnySourcesBefore =
                (mAudioDecoder != NULL) || (mVideoDecoder != NULL);
            bool rescan = false;

            // initialize video before audio because successful initialization of
            // video may change deep buffer mode of audio.
            if (mSurface != NULL) {
                if (instantiateDecoder(false, &mVideoDecoder) == -EWOULDBLOCK) {
                    rescan = true;
                }
            }

            // Don't try to re-open audio sink if there's an existing decoder.
            if (mAudioSink != NULL && mAudioDecoder == NULL) {
                if (instantiateDecoder(true, &mAudioDecoder) == -EWOULDBLOCK) {
                    rescan = true;
                }
            }

            if (!mHadAnySourcesBefore
                    && (mAudioDecoder != NULL || mVideoDecoder != NULL)) {
                // This is the first time we've found anything playable.

                if (mSourceFlags & Source::FLAG_DYNAMIC_DURATION) {
                    schedulePollDuration();
                }
            }

            status_t err;
            if ((err = mSource->feedMoreTSData()) != OK) {
                if (mAudioDecoder == NULL && mVideoDecoder == NULL) {
                    // We're not currently decoding anything (no audio or
                    // video tracks found) and we just ran out of input data.

                    if (err == ERROR_END_OF_STREAM) {
                        notifyListener(MEDIA_PLAYBACK_COMPLETE, 0, 0);
                    } else {
                        notifyListener(MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, err);
                    }
                }
                break;
            }

            if (rescan) {
                msg->post(100000LL);
                mScanSourcesPending = true;
            }
            break;
        }

        case kWhatVideoNotify:
        case kWhatAudioNotify:
        {
            bool audio = msg->what() == kWhatAudioNotify;

            int32_t currentDecoderGeneration =
                (audio? mAudioDecoderGeneration : mVideoDecoderGeneration);
            int32_t requesterGeneration = currentDecoderGeneration - 1;
            CHECK(msg->findInt32("generation", &requesterGeneration));

            if (requesterGeneration != currentDecoderGeneration) {
                ALOGV("got message from old %s decoder, generation(%d:%d)",
                        audio ? "audio" : "video", requesterGeneration,
                        currentDecoderGeneration);
                sp<AMessage> reply;
                if (!(msg->findMessage("reply", &reply))) {
                    return;
                }

                reply->setInt32("err", INFO_DISCONTINUITY);
                reply->post();
                return;
            }

            int32_t what;
            CHECK(msg->findInt32("what", &what));

            if (what == DecoderBase::kWhatInputDiscontinuity) {
                int32_t formatChange;
                CHECK(msg->findInt32("formatChange", &formatChange));

                ALOGV("%s discontinuity: formatChange %d",
                        audio ? "audio" : "video", formatChange);

                if (formatChange) {
                    mDeferredActions.push_back(
                            new FlushDecoderAction(
                                audio ? FLUSH_CMD_SHUTDOWN : FLUSH_CMD_NONE,
                                audio ? FLUSH_CMD_NONE : FLUSH_CMD_SHUTDOWN));
                }

                mDeferredActions.push_back(
                        new SimpleAction(
                                &NuPlayer::performScanSources));

                processDeferredActions();
            } else if (what == DecoderBase::kWhatEOS) {
                int32_t err;
                CHECK(msg->findInt32("err", &err));

                if (err == ERROR_END_OF_STREAM) {
                    ALOGV("got %s decoder EOS", audio ? "audio" : "video");
                } else {
                    ALOGV("got %s decoder EOS w/ error %d",
                         audio ? "audio" : "video",
                         err);
                }

                mRenderer->queueEOS(audio, err);
            } else if (what == DecoderBase::kWhatFlushCompleted) {
                ALOGV("decoder %s flush completed", audio ? "audio" : "video");

                handleFlushComplete(audio, true /* isDecoder */);
                finishFlushIfPossible();
            } else if (what == DecoderBase::kWhatVideoSizeChanged) {
                sp<AMessage> format;
                CHECK(msg->findMessage("format", &format));

                sp<AMessage> inputFormat =
                        mSource->getFormat(false /* audio */);

                setVideoScalingMode(mVideoScalingMode);
                updateVideoSize(inputFormat, format);
            } else if (what == DecoderBase::kWhatShutdownCompleted) {
                ALOGV("%s shutdown completed", audio ? "audio" : "video");
                if (audio) {
                    Mutex::Autolock autoLock(mDecoderLock);
                    mAudioDecoder.clear();
                    mAudioDecoderError = false;
                    ++mAudioDecoderGeneration;

                    CHECK_EQ((int)mFlushingAudio, (int)SHUTTING_DOWN_DECODER);
                    mFlushingAudio = SHUT_DOWN;
                } else {
                    Mutex::Autolock autoLock(mDecoderLock);
                    mVideoDecoder.clear();
                    mVideoDecoderError = false;
                    ++mVideoDecoderGeneration;

                    CHECK_EQ((int)mFlushingVideo, (int)SHUTTING_DOWN_DECODER);
                    mFlushingVideo = SHUT_DOWN;
                }

                finishFlushIfPossible();
            } else if (what == DecoderBase::kWhatResumeCompleted) {
                finishResume();
            } else if (what == DecoderBase::kWhatError) {
                status_t err;
                if (!msg->findInt32("err", &err) || err == OK) {
                    err = UNKNOWN_ERROR;
                }

                // Decoder errors can be due to Source (e.g. from streaming),
                // or from decoding corrupted bitstreams, or from other decoder
                // MediaCodec operations (e.g. from an ongoing reset or seek).
                // They may also be due to openAudioSink failure at
                // decoder start or after a format change.
                //
                // We try to gracefully shut down the affected decoder if possible,
                // rather than trying to force the shutdown with something
                // similar to performReset(). This method can lead to a hang
                // if MediaCodec functions block after an error, but they should
                // typically return INVALID_OPERATION instead of blocking.

                FlushStatus *flushing = audio ? &mFlushingAudio : &mFlushingVideo;
                ALOGE("received error(%#x) from %s decoder, flushing(%d), now shutting down",
                        err, audio ? "audio" : "video", *flushing);

                switch (*flushing) {
                    case NONE:
                        mDeferredActions.push_back(
                                new FlushDecoderAction(
                                    audio ? FLUSH_CMD_SHUTDOWN : FLUSH_CMD_NONE,
                                    audio ? FLUSH_CMD_NONE : FLUSH_CMD_SHUTDOWN));
                        processDeferredActions();
                        break;
                    case FLUSHING_DECODER:
                        *flushing = FLUSHING_DECODER_SHUTDOWN; // initiate shutdown after flush.
                        break; // Wait for flush to complete.
                    case FLUSHING_DECODER_SHUTDOWN:
                        break; // Wait for flush to complete.
                    case SHUTTING_DOWN_DECODER:
                        break; // Wait for shutdown to complete.
                    case FLUSHED:
                        getDecoder(audio)->initiateShutdown(); // In the middle of a seek.
                        *flushing = SHUTTING_DOWN_DECODER;     // Shut down.
                        break;
                    case SHUT_DOWN:
                        finishFlushIfPossible();  // Should not occur.
                        break;                    // Finish anyways.
                }
                if (mSource != nullptr) {
                    if (audio) {
                        if (mVideoDecoderError || mSource->getFormat(false /* audio */) == NULL
                                || mSurface == NULL || mVideoDecoder == NULL) {
                            // When both audio and video have error, or this stream has only audio
                            // which has error, notify client of error.
                            notifyListener(MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, err);
                        } else {
                            // Only audio track has error. Video track could be still good to play.
                            notifyListener(MEDIA_INFO, MEDIA_INFO_PLAY_AUDIO_ERROR, err);
                        }
                        mAudioDecoderError = true;
                    } else {
                        if (mAudioDecoderError || mSource->getFormat(true /* audio */) == NULL
                                || mAudioSink == NULL || mAudioDecoder == NULL) {
                            // When both audio and video have error, or this stream has only video
                            // which has error, notify client of error.
                            notifyListener(MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, err);
                        } else {
                            // Only video track has error. Audio track could be still good to play.
                            notifyListener(MEDIA_INFO, MEDIA_INFO_PLAY_VIDEO_ERROR, err);
                        }
                        mVideoDecoderError = true;
                    }
                }
            } else {
                ALOGV("Unhandled decoder notification %d '%c%c%c%c'.",
                      what,
                      what >> 24,
                      (what >> 16) & 0xff,
                      (what >> 8) & 0xff,
                      what & 0xff);
            }

            break;
        }

        case kWhatRendererNotify:
        {
            int32_t requesterGeneration = mRendererGeneration - 1;
            CHECK(msg->findInt32("generation", &requesterGeneration));
            if (requesterGeneration != mRendererGeneration) {
                ALOGV("got message from old renderer, generation(%d:%d)",
                        requesterGeneration, mRendererGeneration);
                return;
            }

            int32_t what;
            CHECK(msg->findInt32("what", &what));

            if (what == Renderer::kWhatEOS) {
                int32_t audio;
                CHECK(msg->findInt32("audio", &audio));

                int32_t finalResult;
                CHECK(msg->findInt32("finalResult", &finalResult));

                if (audio) {
                    mAudioEOS = true;
                } else {
                    mVideoEOS = true;
                }

                if (finalResult == ERROR_END_OF_STREAM) {
                    ALOGV("reached %s EOS", audio ? "audio" : "video");
                } else {
                    ALOGE("%s track encountered an error (%d)",
                         audio ? "audio" : "video", finalResult);

                    notifyListener(
                            MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, finalResult);
                }

                if ((mAudioEOS || mAudioDecoder == NULL)
                        && (mVideoEOS || mVideoDecoder == NULL)) {
                    notifyListener(MEDIA_PLAYBACK_COMPLETE, 0, 0);
                }
            } else if (what == Renderer::kWhatFlushComplete) {
                int32_t audio;
                CHECK(msg->findInt32("audio", &audio));

                if (audio) {
                    mAudioEOS = false;
                } else {
                    mVideoEOS = false;
                }

                ALOGV("renderer %s flush completed.", audio ? "audio" : "video");
                if (audio && (mFlushingAudio == NONE || mFlushingAudio == FLUSHED
                        || mFlushingAudio == SHUT_DOWN)) {
                    // Flush has been handled by tear down.
                    break;
                }
                handleFlushComplete(audio, false /* isDecoder */);
                finishFlushIfPossible();
            } else if (what == Renderer::kWhatVideoRenderingStart) {
                notifyListener(MEDIA_INFO, MEDIA_INFO_RENDERING_START, 0);
            } else if (what == Renderer::kWhatMediaRenderingStart) {
                ALOGV("media rendering started");
                notifyListener(MEDIA_STARTED, 0, 0);
            } else if (what == Renderer::kWhatAudioTearDown) {
                int32_t reason;
                CHECK(msg->findInt32("reason", &reason));
                ALOGV("Tear down audio with reason %d.", reason);
                if (reason == Renderer::kDueToTimeout && !(mPaused && mOffloadAudio)) {
                    // TimeoutWhenPaused is only for offload mode.
                    ALOGW("Received a stale message for teardown, mPaused(%d), mOffloadAudio(%d)",
                          mPaused, mOffloadAudio);
                    break;
                }
                int64_t positionUs;
                if (!msg->findInt64("positionUs", &positionUs)) {
                    positionUs = mPreviousSeekTimeUs;
                }

                restartAudio(
                        positionUs, reason == Renderer::kForceNonOffload /* forceNonOffload */,
                        reason != Renderer::kDueToTimeout /* needsToCreateAudioDecoder */);
            }
            break;
        }

        case kWhatMoreDataQueued:
        {
            break;
        }

        case kWhatReset:
        {
            ALOGV("kWhatReset");

            mResetting = true;
            updatePlaybackTimer(true /* stopping */, "kWhatReset");
            updateRebufferingTimer(true /* stopping */, true /* exiting */);

            mDeferredActions.push_back(
                    new FlushDecoderAction(
                        FLUSH_CMD_SHUTDOWN /* audio */,
                        FLUSH_CMD_SHUTDOWN /* video */));

            mDeferredActions.push_back(
                    new SimpleAction(&NuPlayer::performReset));

            processDeferredActions();
            break;
        }

        case kWhatNotifyTime:
        {
            ALOGV("kWhatNotifyTime");
            int64_t timerUs;
            CHECK(msg->findInt64("timerUs", &timerUs));

            notifyListener(MEDIA_NOTIFY_TIME, timerUs, 0);
            break;
        }

        case kWhatSeek:
        {
            int64_t seekTimeUs;
            int32_t mode;
            int32_t needNotify;
            CHECK(msg->findInt64("seekTimeUs", &seekTimeUs));
            CHECK(msg->findInt32("mode", &mode));
            CHECK(msg->findInt32("needNotify", &needNotify));

            ALOGV("kWhatSeek seekTimeUs=%lld us, mode=%d, needNotify=%d",
                    (long long)seekTimeUs, mode, needNotify);

            if (!mStarted) {
                // Seek before the player is started. In order to preview video,
                // need to start the player and pause it. This branch is called
                // only once if needed. After the player is started, any seek
                // operation will go through normal path.
                // Audio-only cases are handled separately.
                onStart(seekTimeUs, (MediaPlayerSeekMode)mode);
                if (mStarted) {
                    onPause();
                    mPausedByClient = true;
                }
                if (needNotify) {
                    notifyDriverSeekComplete();
                }
                break;
            }

            mDeferredActions.push_back(
                    new FlushDecoderAction(FLUSH_CMD_FLUSH /* audio */,
                                           FLUSH_CMD_FLUSH /* video */));

            mDeferredActions.push_back(
                    new SeekAction(seekTimeUs, (MediaPlayerSeekMode)mode));

            // After a flush without shutdown, decoder is paused.
            // Don't resume it until source seek is done, otherwise it could
            // start pulling stale data too soon.
            mDeferredActions.push_back(
                    new ResumeDecoderAction(needNotify));

            processDeferredActions();
            break;
        }

        case kWhatPause:
        {
            onPause();
            mPausedByClient = true;
            break;
        }

        case kWhatSourceNotify:
        {
            onSourceNotify(msg);
            break;
        }

        case kWhatClosedCaptionNotify:
        {
            onClosedCaptionNotify(msg);
            break;
        }

        case kWhatPrepareDrm:
        {
            status_t status = onPrepareDrm(msg);

            sp<AMessage> response = new AMessage;
            response->setInt32("status", status);
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));
            response->postReply(replyID);
            break;
        }

        case kWhatReleaseDrm:
        {
            status_t status = onReleaseDrm();

            sp<AMessage> response = new AMessage;
            response->setInt32("status", status);
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));
            response->postReply(replyID);
            break;
        }

        case kWhatMediaClockNotify:
        {
            ALOGV("kWhatMediaClockNotify");
            int64_t anchorMediaUs, anchorRealUs;
            float playbackRate;
            CHECK(msg->findInt64("anchor-media-us", &anchorMediaUs));
            CHECK(msg->findInt64("anchor-real-us", &anchorRealUs));
            CHECK(msg->findFloat("playback-rate", &playbackRate));

            Parcel in;
            in.writeInt64(anchorMediaUs);
            in.writeInt64(anchorRealUs);
            in.writeFloat(playbackRate);

            notifyListener(MEDIA_TIME_DISCONTINUITY, 0, 0, &in);
            break;
        }

        default:
            TRESPASS();
            break;
    }
}

void NuPlayer::onResume() {
    if (!mPaused || mResetting) {
        ALOGD_IF(mResetting, "resetting, onResume discarded");
        return;
    }
    mPaused = false;
    if (mSource != NULL) {
        mSource->resume();
    } else {
        ALOGW("resume called when source is gone or not set");
    }
    // |mAudioDecoder| may have been released due to the pause timeout, so re-create it if
    // needed.
    if (audioDecoderStillNeeded() && mAudioDecoder == NULL) {
        instantiateDecoder(true /* audio */, &mAudioDecoder);
    }
    if (mRenderer != NULL) {
        mRenderer->resume();
    } else {
        ALOGW("resume called when renderer is gone or not set");
    }

    startPlaybackTimer("onresume");
}

status_t NuPlayer::onInstantiateSecureDecoders() {
    status_t err;
    if (!(mSourceFlags & Source::FLAG_SECURE)) {
        return BAD_TYPE;
    }

    if (mRenderer != NULL) {
        ALOGE("renderer should not be set when instantiating secure decoders");
        return UNKNOWN_ERROR;
    }

    // TRICKY: We rely on mRenderer being null, so that decoder does not start requesting
    // data on instantiation.
    if (mSurface != NULL) {
        err = instantiateDecoder(false, &mVideoDecoder);
        if (err != OK) {
            return err;
        }
    }

    if (mAudioSink != NULL) {
        err = instantiateDecoder(true, &mAudioDecoder);
        if (err != OK) {
            return err;
        }
    }
    return OK;
}

void NuPlayer::onStart(int64_t startPositionUs, MediaPlayerSeekMode mode) {
    ALOGV("onStart: mCrypto: %p (%d)", mCrypto.get(),
            (mCrypto != NULL ? mCrypto->getStrongCount() : 0));

    if (!mSourceStarted) {
        mSourceStarted = true;
        mSource->start();
    }
    if (startPositionUs > 0) {
        performSeek(startPositionUs, mode);
        if (mSource->getFormat(false /* audio */) == NULL) {
            return;
        }
    }

    mOffloadAudio = false;
    mAudioEOS = false;
    mVideoEOS = false;
    mStarted = true;
    mPaused = false;

    uint32_t flags = 0;

    if (mSource->isRealTime()) {
        flags |= Renderer::FLAG_REAL_TIME;
    }

    bool hasAudio = (mSource->getFormat(true /* audio */) != NULL);
    bool hasVideo = (mSource->getFormat(false /* audio */) != NULL);
    if (!hasAudio && !hasVideo) {
        ALOGE("no metadata for either audio or video source");
        mSource->stop();
        mSourceStarted = false;
        notifyListener(MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, ERROR_MALFORMED);
        return;
    }
    ALOGV_IF(!hasAudio, "no metadata for audio source");  // video only stream

    sp<MetaData> audioMeta = mSource->getFormatMeta(true /* audio */);

    audio_stream_type_t streamType = AUDIO_STREAM_MUSIC;
    if (mAudioSink != NULL) {
        streamType = mAudioSink->getAudioStreamType();
    }

    mOffloadAudio =
        canOffloadStream(audioMeta, hasVideo, mSource->isStreaming(), streamType)
                && (mPlaybackSettings.mSpeed == 1.f && mPlaybackSettings.mPitch == 1.f);

    // Modular DRM: Disabling audio offload if the source is protected
    if (mOffloadAudio && mIsDrmProtected) {
        mOffloadAudio = false;
        ALOGV("onStart: Disabling mOffloadAudio now that the source is protected.");
    }

    if (mOffloadAudio) {
        flags |= Renderer::FLAG_OFFLOAD_AUDIO;
    }

    sp<AMessage> notify = new AMessage(kWhatRendererNotify, this);
    ++mRendererGeneration;
    notify->setInt32("generation", mRendererGeneration);
    mRenderer = new Renderer(mAudioSink, mMediaClock, notify, flags);
    mRendererLooper = new ALooper;
    mRendererLooper->setName("NuPlayerRenderer");
    mRendererLooper->start(false, false, ANDROID_PRIORITY_AUDIO);
    mRendererLooper->registerHandler(mRenderer);

    status_t err = mRenderer->setPlaybackSettings(mPlaybackSettings);
    if (err != OK) {
        mSource->stop();
        mSourceStarted = false;
        notifyListener(MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, err);
        return;
    }

    float rate = getFrameRate();
    if (rate > 0) {
        mRenderer->setVideoFrameRate(rate);
    }

    if (mVideoDecoder != NULL) {
        mVideoDecoder->setRenderer(mRenderer);
    }
    if (mAudioDecoder != NULL) {
        mAudioDecoder->setRenderer(mRenderer);
    }

    startPlaybackTimer("onstart");

    postScanSources();
}

void NuPlayer::startPlaybackTimer(const char *where) {
    Mutex::Autolock autoLock(mPlayingTimeLock);
    if (mLastStartedPlayingTimeNs == 0) {
        mLastStartedPlayingTimeNs = systemTime();
        ALOGV("startPlaybackTimer() time %20" PRId64 " (%s)",  mLastStartedPlayingTimeNs, where);
    }
}

void NuPlayer::updatePlaybackTimer(bool stopping, const char *where) {
    Mutex::Autolock autoLock(mPlayingTimeLock);

    ALOGV("updatePlaybackTimer(%s)  time %20" PRId64 " (%s)",
	  stopping ? "stop" : "snap", mLastStartedPlayingTimeNs, where);

    if (mLastStartedPlayingTimeNs != 0) {
        sp<NuPlayerDriver> driver = mDriver.promote();
        int64_t now = systemTime();
        if (driver != NULL) {
            int64_t played = now - mLastStartedPlayingTimeNs;
            ALOGV("updatePlaybackTimer()  log  %20" PRId64 "", played);

            if (played > 0) {
                driver->notifyMorePlayingTimeUs((played+500)/1000);
            }
        }
	if (stopping) {
            mLastStartedPlayingTimeNs = 0;
	} else {
            mLastStartedPlayingTimeNs = now;
	}
    }
}

void NuPlayer::startRebufferingTimer() {
    Mutex::Autolock autoLock(mPlayingTimeLock);
    if (mLastStartedRebufferingTimeNs == 0) {
        mLastStartedRebufferingTimeNs = systemTime();
        ALOGV("startRebufferingTimer() time %20" PRId64 "",  mLastStartedRebufferingTimeNs);
    }
}

void NuPlayer::updateRebufferingTimer(bool stopping, bool exitingPlayback) {
    Mutex::Autolock autoLock(mPlayingTimeLock);

    ALOGV("updateRebufferingTimer(%s)  time %20" PRId64 " (exiting %d)",
	  stopping ? "stop" : "snap", mLastStartedRebufferingTimeNs, exitingPlayback);

    if (mLastStartedRebufferingTimeNs != 0) {
        sp<NuPlayerDriver> driver = mDriver.promote();
        int64_t now = systemTime();
        if (driver != NULL) {
            int64_t rebuffered = now - mLastStartedRebufferingTimeNs;
            ALOGV("updateRebufferingTimer()  log  %20" PRId64 "", rebuffered);

            if (rebuffered > 0) {
                driver->notifyMoreRebufferingTimeUs((rebuffered+500)/1000);
                if (exitingPlayback) {
                    driver->notifyRebufferingWhenExit(true);
                }
            }
        }
	if (stopping) {
            mLastStartedRebufferingTimeNs = 0;
	} else {
            mLastStartedRebufferingTimeNs = now;
	}
    }
}

void NuPlayer::updateInternalTimers() {
    // update values, but ticking clocks keep ticking
    ALOGV("updateInternalTimers()");
    updatePlaybackTimer(false /* stopping */, "updateInternalTimers");
    updateRebufferingTimer(false /* stopping */, false /* exiting */);
}

void NuPlayer::onPause() {

    updatePlaybackTimer(true /* stopping */, "onPause");

    if (mPaused) {
        return;
    }
    mPaused = true;
    if (mSource != NULL) {
        mSource->pause();
    } else {
        ALOGW("pause called when source is gone or not set");
    }
    if (mRenderer != NULL) {
        mRenderer->pause();
    } else {
        ALOGW("pause called when renderer is gone or not set");
    }

}

bool NuPlayer::audioDecoderStillNeeded() {
    // Audio decoder is no longer needed if it's in shut/shutting down status.
    return ((mFlushingAudio != SHUT_DOWN) && (mFlushingAudio != SHUTTING_DOWN_DECODER));
}

void NuPlayer::handleFlushComplete(bool audio, bool isDecoder) {
    // We wait for both the decoder flush and the renderer flush to complete
    // before entering either the FLUSHED or the SHUTTING_DOWN_DECODER state.

    mFlushComplete[audio][isDecoder] = true;
    if (!mFlushComplete[audio][!isDecoder]) {
        return;
    }

    FlushStatus *state = audio ? &mFlushingAudio : &mFlushingVideo;
    switch (*state) {
        case FLUSHING_DECODER:
        {
            *state = FLUSHED;
            break;
        }

        case FLUSHING_DECODER_SHUTDOWN:
        {
            *state = SHUTTING_DOWN_DECODER;

            ALOGV("initiating %s decoder shutdown", audio ? "audio" : "video");
            getDecoder(audio)->initiateShutdown();
            break;
        }

        default:
            // decoder flush completes only occur in a flushing state.
            LOG_ALWAYS_FATAL_IF(isDecoder, "decoder flush in invalid state %d", *state);
            break;
    }
}

void NuPlayer::finishFlushIfPossible() {
    if (mFlushingAudio != NONE && mFlushingAudio != FLUSHED
            && mFlushingAudio != SHUT_DOWN) {
        return;
    }

    if (mFlushingVideo != NONE && mFlushingVideo != FLUSHED
            && mFlushingVideo != SHUT_DOWN) {
        return;
    }

    ALOGV("both audio and video are flushed now.");

    mFlushingAudio = NONE;
    mFlushingVideo = NONE;

    clearFlushComplete();

    processDeferredActions();
}

void NuPlayer::postScanSources() {
    if (mScanSourcesPending) {
        return;
    }

    sp<AMessage> msg = new AMessage(kWhatScanSources, this);
    msg->setInt32("generation", mScanSourcesGeneration);
    msg->post();

    mScanSourcesPending = true;
}

void NuPlayer::tryOpenAudioSinkForOffload(
        const sp<AMessage> &format, const sp<MetaData> &audioMeta, bool hasVideo) {
    // Note: This is called early in NuPlayer to determine whether offloading
    // is possible; otherwise the decoders call the renderer openAudioSink directly.

    status_t err = mRenderer->openAudioSink(
            format, true /* offloadOnly */, hasVideo,
            AUDIO_OUTPUT_FLAG_NONE, &mOffloadAudio, mSource->isStreaming());
    if (err != OK) {
        // Any failure we turn off mOffloadAudio.
        mOffloadAudio = false;
    } else if (mOffloadAudio) {
        sendMetaDataToHal(mAudioSink, audioMeta);
    }
}

void NuPlayer::closeAudioSink() {
    mRenderer->closeAudioSink();
}

void NuPlayer::restartAudio(
        int64_t currentPositionUs, bool forceNonOffload, bool needsToCreateAudioDecoder) {
    ALOGD("restartAudio timeUs(%lld), dontOffload(%d), createDecoder(%d)",
          (long long)currentPositionUs, forceNonOffload, needsToCreateAudioDecoder);
    if (mAudioDecoder != NULL) {
        mAudioDecoder->pause();
        Mutex::Autolock autoLock(mDecoderLock);
        mAudioDecoder.clear();
        mAudioDecoderError = false;
        ++mAudioDecoderGeneration;
    }
    if (mFlushingAudio == FLUSHING_DECODER) {
        mFlushComplete[1 /* audio */][1 /* isDecoder */] = true;
        mFlushingAudio = FLUSHED;
        finishFlushIfPossible();
    } else if (mFlushingAudio == FLUSHING_DECODER_SHUTDOWN
            || mFlushingAudio == SHUTTING_DOWN_DECODER) {
        mFlushComplete[1 /* audio */][1 /* isDecoder */] = true;
        mFlushingAudio = SHUT_DOWN;
        finishFlushIfPossible();
        needsToCreateAudioDecoder = false;
    }
    if (mRenderer == NULL) {
        return;
    }
    closeAudioSink();
    mRenderer->flush(true /* audio */, false /* notifyComplete */);
    if (mVideoDecoder != NULL) {
        mDeferredActions.push_back(
                new FlushDecoderAction(FLUSH_CMD_NONE /* audio */,
                                       FLUSH_CMD_FLUSH /* video */));
        mDeferredActions.push_back(
                new SeekAction(currentPositionUs,
                MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC /* mode */));
        // After a flush without shutdown, decoder is paused.
        // Don't resume it until source seek is done, otherwise it could
        // start pulling stale data too soon.
        mDeferredActions.push_back(new ResumeDecoderAction(false));
        processDeferredActions();
    } else {
        performSeek(currentPositionUs, MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC /* mode */);
    }

    if (forceNonOffload) {
        mRenderer->signalDisableOffloadAudio();
        mOffloadAudio = false;
    }
    if (needsToCreateAudioDecoder) {
        instantiateDecoder(true /* audio */, &mAudioDecoder, !forceNonOffload);
    }
}

void NuPlayer::determineAudioModeChange(const sp<AMessage> &audioFormat) {
    if (mSource == NULL || mAudioSink == NULL) {
        return;
    }

    if (mRenderer == NULL) {
        ALOGW("No renderer can be used to determine audio mode. Use non-offload for safety.");
        mOffloadAudio = false;
        return;
    }

    sp<MetaData> audioMeta = mSource->getFormatMeta(true /* audio */);
    sp<AMessage> videoFormat = mSource->getFormat(false /* audio */);
    audio_stream_type_t streamType = mAudioSink->getAudioStreamType();
    const bool hasVideo = (videoFormat != NULL);
    bool canOffload = canOffloadStream(
            audioMeta, hasVideo, mSource->isStreaming(), streamType)
                    && (mPlaybackSettings.mSpeed == 1.f && mPlaybackSettings.mPitch == 1.f);

    // Modular DRM: Disabling audio offload if the source is protected
    if (canOffload && mIsDrmProtected) {
        canOffload = false;
        ALOGV("determineAudioModeChange: Disabling mOffloadAudio b/c the source is protected.");
    }

    if (canOffload) {
        if (!mOffloadAudio) {
            mRenderer->signalEnableOffloadAudio();
        }
        // open audio sink early under offload mode.
        tryOpenAudioSinkForOffload(audioFormat, audioMeta, hasVideo);
    } else {
        if (mOffloadAudio) {
            mRenderer->signalDisableOffloadAudio();
            mOffloadAudio = false;
        }
    }
}

status_t NuPlayer::instantiateDecoder(
        bool audio, sp<DecoderBase> *decoder, bool checkAudioModeChange) {
    // The audio decoder could be cleared by tear down. If still in shut down
    // process, no need to create a new audio decoder.
    if (*decoder != NULL || (audio && mFlushingAudio == SHUT_DOWN)) {
        return OK;
    }

    sp<AMessage> format = mSource->getFormat(audio);

    if (format == NULL) {
        return UNKNOWN_ERROR;
    } else {
        status_t err;
        if (format->findInt32("err", &err) && err) {
            return err;
        }
    }

    format->setInt32("priority", 0 /* realtime */);

    if (!audio) {
        AString mime;
        CHECK(format->findString("mime", &mime));

        sp<AMessage> ccNotify = new AMessage(kWhatClosedCaptionNotify, this);
        if (mCCDecoder == NULL) {
            mCCDecoder = new CCDecoder(ccNotify);
        }

        if (mSourceFlags & Source::FLAG_SECURE) {
            format->setInt32("secure", true);
        }

        if (mSourceFlags & Source::FLAG_PROTECTED) {
            format->setInt32("protected", true);
        }

        float rate = getFrameRate();
        if (rate > 0) {
            format->setFloat("operating-rate", rate * mPlaybackSettings.mSpeed);
        }
    }

    Mutex::Autolock autoLock(mDecoderLock);

    if (audio) {
        sp<AMessage> notify = new AMessage(kWhatAudioNotify, this);
        ++mAudioDecoderGeneration;
        notify->setInt32("generation", mAudioDecoderGeneration);

        if (checkAudioModeChange) {
            determineAudioModeChange(format);
        }
        if (mOffloadAudio) {
            mSource->setOffloadAudio(true /* offload */);

            const bool hasVideo = (mSource->getFormat(false /*audio */) != NULL);
            format->setInt32("has-video", hasVideo);
            *decoder = new DecoderPassThrough(notify, mSource, mRenderer);
            ALOGV("instantiateDecoder audio DecoderPassThrough  hasVideo: %d", hasVideo);
        } else {
            mSource->setOffloadAudio(false /* offload */);

            *decoder = new Decoder(notify, mSource, mPID, mUID, mRenderer);
            ALOGV("instantiateDecoder audio Decoder");
        }
        mAudioDecoderError = false;
    } else {
        sp<AMessage> notify = new AMessage(kWhatVideoNotify, this);
        ++mVideoDecoderGeneration;
        notify->setInt32("generation", mVideoDecoderGeneration);

        *decoder = new Decoder(
                notify, mSource, mPID, mUID, mRenderer, mSurface, mCCDecoder);
        mVideoDecoderError = false;

        // enable FRC if high-quality AV sync is requested, even if not
        // directly queuing to display, as this will even improve textureview
        // playback.
        {
            if (property_get_bool("persist.sys.media.avsync", false)) {
                format->setInt32("auto-frc", 1);
            }
        }
    }
    (*decoder)->init();

    // Modular DRM
    if (mIsDrmProtected) {
        format->setPointer("crypto", mCrypto.get());
        ALOGV("instantiateDecoder: mCrypto: %p (%d) isSecure: %d", mCrypto.get(),
                (mCrypto != NULL ? mCrypto->getStrongCount() : 0),
                (mSourceFlags & Source::FLAG_SECURE) != 0);
    }

    (*decoder)->configure(format);

    if (!audio) {
        sp<AMessage> params = new AMessage();
        float rate = getFrameRate();
        if (rate > 0) {
            params->setFloat("frame-rate-total", rate);
        }

        sp<MetaData> fileMeta = getFileMeta();
        if (fileMeta != NULL) {
            int32_t videoTemporalLayerCount;
            if (fileMeta->findInt32(kKeyTemporalLayerCount, &videoTemporalLayerCount)
                    && videoTemporalLayerCount > 0) {
                params->setInt32("temporal-layer-count", videoTemporalLayerCount);
            }
        }

        if (params->countEntries() > 0) {
            (*decoder)->setParameters(params);
        }
    }
    return OK;
}

void NuPlayer::updateVideoSize(
        const sp<AMessage> &inputFormat,
        const sp<AMessage> &outputFormat) {
    if (inputFormat == NULL) {
        ALOGW("Unknown video size, reporting 0x0!");
        notifyListener(MEDIA_SET_VIDEO_SIZE, 0, 0);
        return;
    }
    int32_t err = OK;
    inputFormat->findInt32("err", &err);
    if (err == -EWOULDBLOCK) {
        ALOGW("Video meta is not available yet!");
        return;
    }
    if (err != OK) {
        ALOGW("Something is wrong with video meta!");
        return;
    }

    int32_t displayWidth, displayHeight;
    if (outputFormat != NULL) {
        int32_t width, height;
        CHECK(outputFormat->findInt32("width", &width));
        CHECK(outputFormat->findInt32("height", &height));

        int32_t cropLeft, cropTop, cropRight, cropBottom;
        CHECK(outputFormat->findRect(
                    "crop",
                    &cropLeft, &cropTop, &cropRight, &cropBottom));

        displayWidth = cropRight - cropLeft + 1;
        displayHeight = cropBottom - cropTop + 1;

        ALOGV("Video output format changed to %d x %d "
             "(crop: %d x %d @ (%d, %d))",
             width, height,
             displayWidth,
             displayHeight,
             cropLeft, cropTop);
    } else {
        CHECK(inputFormat->findInt32("width", &displayWidth));
        CHECK(inputFormat->findInt32("height", &displayHeight));

        ALOGV("Video input format %d x %d", displayWidth, displayHeight);
    }

    // Take into account sample aspect ratio if necessary:
    int32_t sarWidth, sarHeight;
    if (inputFormat->findInt32("sar-width", &sarWidth)
            && inputFormat->findInt32("sar-height", &sarHeight)
            && sarWidth > 0 && sarHeight > 0) {
        ALOGV("Sample aspect ratio %d : %d", sarWidth, sarHeight);

        displayWidth = (displayWidth * sarWidth) / sarHeight;

        ALOGV("display dimensions %d x %d", displayWidth, displayHeight);
    } else {
        int32_t width, height;
        if (inputFormat->findInt32("display-width", &width)
                && inputFormat->findInt32("display-height", &height)
                && width > 0 && height > 0
                && displayWidth > 0 && displayHeight > 0) {
            if (displayHeight * (int64_t)width / height > (int64_t)displayWidth) {
                displayHeight = (int32_t)(displayWidth * (int64_t)height / width);
            } else {
                displayWidth = (int32_t)(displayHeight * (int64_t)width / height);
            }
            ALOGV("Video display width and height are overridden to %d x %d",
                 displayWidth, displayHeight);
        }
    }

    int32_t rotationDegrees;
    if (!inputFormat->findInt32("rotation-degrees", &rotationDegrees)) {
        rotationDegrees = 0;
    }

    if (rotationDegrees == 90 || rotationDegrees == 270) {
        int32_t tmp = displayWidth;
        displayWidth = displayHeight;
        displayHeight = tmp;
    }

    notifyListener(
            MEDIA_SET_VIDEO_SIZE,
            displayWidth,
            displayHeight);
}

void NuPlayer::notifyListener(int msg, int ext1, int ext2, const Parcel *in) {
    if (mDriver == NULL) {
        return;
    }

    sp<NuPlayerDriver> driver = mDriver.promote();

    if (driver == NULL) {
        return;
    }

    driver->notifyListener(msg, ext1, ext2, in);
}

void NuPlayer::flushDecoder(bool audio, bool needShutdown) {
    ALOGV("[%s] flushDecoder needShutdown=%d",
          audio ? "audio" : "video", needShutdown);

    const sp<DecoderBase> &decoder = getDecoder(audio);
    if (decoder == NULL) {
        ALOGI("flushDecoder %s without decoder present",
             audio ? "audio" : "video");
        return;
    }

    // Make sure we don't continue to scan sources until we finish flushing.
    ++mScanSourcesGeneration;
    if (mScanSourcesPending) {
        if (!needShutdown) {
            mDeferredActions.push_back(
                    new SimpleAction(&NuPlayer::performScanSources));
        }
        mScanSourcesPending = false;
    }

    decoder->signalFlush();

    FlushStatus newStatus =
        needShutdown ? FLUSHING_DECODER_SHUTDOWN : FLUSHING_DECODER;

    mFlushComplete[audio][false /* isDecoder */] = (mRenderer == NULL);
    mFlushComplete[audio][true /* isDecoder */] = false;
    if (audio) {
        ALOGE_IF(mFlushingAudio != NONE,
                "audio flushDecoder() is called in state %d", mFlushingAudio);
        mFlushingAudio = newStatus;
    } else {
        ALOGE_IF(mFlushingVideo != NONE,
                "video flushDecoder() is called in state %d", mFlushingVideo);
        mFlushingVideo = newStatus;
    }
}

void NuPlayer::queueDecoderShutdown(
        bool audio, bool video, const sp<AMessage> &reply) {
    ALOGI("queueDecoderShutdown audio=%d, video=%d", audio, video);

    mDeferredActions.push_back(
            new FlushDecoderAction(
                audio ? FLUSH_CMD_SHUTDOWN : FLUSH_CMD_NONE,
                video ? FLUSH_CMD_SHUTDOWN : FLUSH_CMD_NONE));

    mDeferredActions.push_back(
            new SimpleAction(&NuPlayer::performScanSources));

    mDeferredActions.push_back(new PostMessageAction(reply));

    processDeferredActions();
}

status_t NuPlayer::setVideoScalingMode(int32_t mode) {
    mVideoScalingMode = mode;
    if (mSurface != NULL) {
        status_t ret = native_window_set_scaling_mode(mSurface.get(), mVideoScalingMode);
        if (ret != OK) {
            ALOGE("Failed to set scaling mode (%d): %s",
                -ret, strerror(-ret));
            return ret;
        }
    }
    return OK;
}

status_t NuPlayer::getTrackInfo(Parcel* reply) const {
    sp<AMessage> msg = new AMessage(kWhatGetTrackInfo, this);
    msg->setPointer("reply", reply);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    return err;
}

status_t NuPlayer::getSelectedTrack(int32_t type, Parcel* reply) const {
    sp<AMessage> msg = new AMessage(kWhatGetSelectedTrack, this);
    msg->setPointer("reply", reply);
    msg->setInt32("type", type);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    if (err == OK && response != NULL) {
        CHECK(response->findInt32("err", &err));
    }
    return err;
}

status_t NuPlayer::selectTrack(size_t trackIndex, bool select, int64_t timeUs) {
    sp<AMessage> msg = new AMessage(kWhatSelectTrack, this);
    msg->setSize("trackIndex", trackIndex);
    msg->setInt32("select", select);
    msg->setInt64("timeUs", timeUs);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if (err != OK) {
        return err;
    }

    if (!response->findInt32("err", &err)) {
        err = OK;
    }

    return err;
}

status_t NuPlayer::getCurrentPosition(int64_t *mediaUs) {
    sp<Renderer> renderer = mRenderer;
    if (renderer == NULL) {
        return NO_INIT;
    }

    return renderer->getCurrentPosition(mediaUs);
}

void NuPlayer::getStats(Vector<sp<AMessage> > *trackStats) {
    CHECK(trackStats != NULL);

    trackStats->clear();

    Mutex::Autolock autoLock(mDecoderLock);
    if (mVideoDecoder != NULL) {
        trackStats->push_back(mVideoDecoder->getStats());
    }
    if (mAudioDecoder != NULL) {
        trackStats->push_back(mAudioDecoder->getStats());
    }
}

sp<MetaData> NuPlayer::getFileMeta() {
    return mSource->getFileFormatMeta();
}

float NuPlayer::getFrameRate() {
    sp<MetaData> meta = mSource->getFormatMeta(false /* audio */);
    if (meta == NULL) {
        return 0;
    }
    int32_t rate;
    if (!meta->findInt32(kKeyFrameRate, &rate)) {
        // fall back to try file meta
        sp<MetaData> fileMeta = getFileMeta();
        if (fileMeta == NULL) {
            ALOGW("source has video meta but not file meta");
            return -1;
        }
        int32_t fileMetaRate;
        if (!fileMeta->findInt32(kKeyFrameRate, &fileMetaRate)) {
            return -1;
        }
        return fileMetaRate;
    }
    return rate;
}

void NuPlayer::schedulePollDuration() {
    sp<AMessage> msg = new AMessage(kWhatPollDuration, this);
    msg->setInt32("generation", mPollDurationGeneration);
    msg->post();
}

void NuPlayer::cancelPollDuration() {
    ++mPollDurationGeneration;
}

void NuPlayer::processDeferredActions() {
    while (!mDeferredActions.empty()) {
        // We won't execute any deferred actions until we're no longer in
        // an intermediate state, i.e. one more more decoders are currently
        // flushing or shutting down.

        if (mFlushingAudio != NONE || mFlushingVideo != NONE) {
            // We're currently flushing, postpone the reset until that's
            // completed.

            ALOGV("postponing action mFlushingAudio=%d, mFlushingVideo=%d",
                  mFlushingAudio, mFlushingVideo);

            break;
        }

        sp<Action> action = *mDeferredActions.begin();
        mDeferredActions.erase(mDeferredActions.begin());

        action->execute(this);
    }
}

void NuPlayer::performSeek(int64_t seekTimeUs, MediaPlayerSeekMode mode) {
    ALOGV("performSeek seekTimeUs=%lld us (%.2f secs), mode=%d",
          (long long)seekTimeUs, seekTimeUs / 1E6, mode);

    if (mSource == NULL) {
        // This happens when reset occurs right before the loop mode
        // asynchronously seeks to the start of the stream.
        LOG_ALWAYS_FATAL_IF(mAudioDecoder != NULL || mVideoDecoder != NULL,
                "mSource is NULL and decoders not NULL audio(%p) video(%p)",
                mAudioDecoder.get(), mVideoDecoder.get());
        return;
    }
    mPreviousSeekTimeUs = seekTimeUs;
    mSource->seekTo(seekTimeUs, mode);
    ++mTimedTextGeneration;

    // everything's flushed, continue playback.
}

void NuPlayer::performDecoderFlush(FlushCommand audio, FlushCommand video) {
    ALOGV("performDecoderFlush audio=%d, video=%d", audio, video);

    if ((audio == FLUSH_CMD_NONE || mAudioDecoder == NULL)
            && (video == FLUSH_CMD_NONE || mVideoDecoder == NULL)) {
        return;
    }

    if (audio != FLUSH_CMD_NONE && mAudioDecoder != NULL) {
        flushDecoder(true /* audio */, (audio == FLUSH_CMD_SHUTDOWN));
    }

    if (video != FLUSH_CMD_NONE && mVideoDecoder != NULL) {
        flushDecoder(false /* audio */, (video == FLUSH_CMD_SHUTDOWN));
    }
}

void NuPlayer::performReset() {
    ALOGV("performReset");

    CHECK(mAudioDecoder == NULL);
    CHECK(mVideoDecoder == NULL);

    updatePlaybackTimer(true /* stopping */, "performReset");
    updateRebufferingTimer(true /* stopping */, true /* exiting */);

    cancelPollDuration();

    ++mScanSourcesGeneration;
    mScanSourcesPending = false;

    if (mRendererLooper != NULL) {
        if (mRenderer != NULL) {
            mRendererLooper->unregisterHandler(mRenderer->id());
        }
        mRendererLooper->stop();
        mRendererLooper.clear();
    }
    mRenderer.clear();
    ++mRendererGeneration;

    if (mSource != NULL) {
        mSource->stop();

        Mutex::Autolock autoLock(mSourceLock);
        mSource.clear();
    }

    if (mDriver != NULL) {
        sp<NuPlayerDriver> driver = mDriver.promote();
        if (driver != NULL) {
            driver->notifyResetComplete();
        }
    }

    mStarted = false;
    mPrepared = false;
    mResetting = false;
    mSourceStarted = false;

    // Modular DRM
    if (mCrypto != NULL) {
        // decoders will be flushed before this so their mCrypto would go away on their own
        // TODO change to ALOGV
        ALOGD("performReset mCrypto: %p (%d)", mCrypto.get(),
                (mCrypto != NULL ? mCrypto->getStrongCount() : 0));
        mCrypto.clear();
    }
    mIsDrmProtected = false;
}

void NuPlayer::performScanSources() {
    ALOGV("performScanSources");

    if (!mStarted) {
        return;
    }

    if (mAudioDecoder == NULL || mVideoDecoder == NULL) {
        postScanSources();
    }
}

void NuPlayer::performSetSurface(const sp<Surface> &surface) {
    ALOGV("performSetSurface");

    mSurface = surface;

    // XXX - ignore error from setVideoScalingMode for now
    setVideoScalingMode(mVideoScalingMode);

    if (mDriver != NULL) {
        sp<NuPlayerDriver> driver = mDriver.promote();
        if (driver != NULL) {
            driver->notifySetSurfaceComplete();
        }
    }
}

void NuPlayer::performResumeDecoders(bool needNotify) {
    if (needNotify) {
        mResumePending = true;
        if (mVideoDecoder == NULL) {
            // if audio-only, we can notify seek complete now,
            // as the resume operation will be relatively fast.
            finishResume();
        }
    }

    if (mVideoDecoder != NULL) {
        // When there is continuous seek, MediaPlayer will cache the seek
        // position, and send down new seek request when previous seek is
        // complete. Let's wait for at least one video output frame before
        // notifying seek complete, so that the video thumbnail gets updated
        // when seekbar is dragged.
        mVideoDecoder->signalResume(needNotify);
    }

    if (mAudioDecoder != NULL) {
        mAudioDecoder->signalResume(false /* needNotify */);
    }
}

void NuPlayer::finishResume() {
    if (mResumePending) {
        mResumePending = false;
        notifyDriverSeekComplete();
    }
}

void NuPlayer::notifyDriverSeekComplete() {
    if (mDriver != NULL) {
        sp<NuPlayerDriver> driver = mDriver.promote();
        if (driver != NULL) {
            driver->notifySeekComplete();
        }
    }
}

void NuPlayer::onSourceNotify(const sp<AMessage> &msg) {
    int32_t what;
    CHECK(msg->findInt32("what", &what));

    switch (what) {
        case Source::kWhatInstantiateSecureDecoders:
        {
            if (mSource == NULL) {
                // This is a stale notification from a source that was
                // asynchronously preparing when the client called reset().
                // We handled the reset, the source is gone.
                break;
            }

            sp<AMessage> reply;
            CHECK(msg->findMessage("reply", &reply));
            status_t err = onInstantiateSecureDecoders();
            reply->setInt32("err", err);
            reply->post();
            break;
        }

        case Source::kWhatPrepared:
        {
            ALOGV("NuPlayer::onSourceNotify Source::kWhatPrepared source: %p", mSource.get());
            if (mSource == NULL) {
                // This is a stale notification from a source that was
                // asynchronously preparing when the client called reset().
                // We handled the reset, the source is gone.
                break;
            }

            int32_t err;
            CHECK(msg->findInt32("err", &err));

            if (err != OK) {
                // shut down potential secure codecs in case client never calls reset
                mDeferredActions.push_back(
                        new FlushDecoderAction(FLUSH_CMD_SHUTDOWN /* audio */,
                                               FLUSH_CMD_SHUTDOWN /* video */));
                processDeferredActions();
            } else {
                mPrepared = true;
            }

            sp<NuPlayerDriver> driver = mDriver.promote();
            if (driver != NULL) {
                // notify duration first, so that it's definitely set when
                // the app received the "prepare complete" callback.
                int64_t durationUs;
                if (mSource->getDuration(&durationUs) == OK) {
                    driver->notifyDuration(durationUs);
                }
                driver->notifyPrepareCompleted(err);
            }

            break;
        }

        // Modular DRM
        case Source::kWhatDrmInfo:
        {
            Parcel parcel;
            sp<ABuffer> drmInfo;
            CHECK(msg->findBuffer("drmInfo", &drmInfo));
            parcel.setData(drmInfo->data(), drmInfo->size());

            ALOGV("onSourceNotify() kWhatDrmInfo MEDIA_DRM_INFO drmInfo: %p  parcel size: %zu",
                    drmInfo.get(), parcel.dataSize());

            notifyListener(MEDIA_DRM_INFO, 0 /* ext1 */, 0 /* ext2 */, &parcel);

            break;
        }

        case Source::kWhatFlagsChanged:
        {
            uint32_t flags;
            CHECK(msg->findInt32("flags", (int32_t *)&flags));

            sp<NuPlayerDriver> driver = mDriver.promote();
            if (driver != NULL) {

                ALOGV("onSourceNotify() kWhatFlagsChanged  FLAG_CAN_PAUSE: %d  "
                        "FLAG_CAN_SEEK_BACKWARD: %d \n\t\t\t\t FLAG_CAN_SEEK_FORWARD: %d  "
                        "FLAG_CAN_SEEK: %d  FLAG_DYNAMIC_DURATION: %d \n"
                        "\t\t\t\t FLAG_SECURE: %d  FLAG_PROTECTED: %d",
                        (flags & Source::FLAG_CAN_PAUSE) != 0,
                        (flags & Source::FLAG_CAN_SEEK_BACKWARD) != 0,
                        (flags & Source::FLAG_CAN_SEEK_FORWARD) != 0,
                        (flags & Source::FLAG_CAN_SEEK) != 0,
                        (flags & Source::FLAG_DYNAMIC_DURATION) != 0,
                        (flags & Source::FLAG_SECURE) != 0,
                        (flags & Source::FLAG_PROTECTED) != 0);

                if ((flags & NuPlayer::Source::FLAG_CAN_SEEK) == 0) {
                    driver->notifyListener(
                            MEDIA_INFO, MEDIA_INFO_NOT_SEEKABLE, 0);
                }
                driver->notifyFlagsChanged(flags);
            }

            if ((mSourceFlags & Source::FLAG_DYNAMIC_DURATION)
                    && (!(flags & Source::FLAG_DYNAMIC_DURATION))) {
                cancelPollDuration();
            } else if (!(mSourceFlags & Source::FLAG_DYNAMIC_DURATION)
                    && (flags & Source::FLAG_DYNAMIC_DURATION)
                    && (mAudioDecoder != NULL || mVideoDecoder != NULL)) {
                schedulePollDuration();
            }

            mSourceFlags = flags;
            break;
        }

        case Source::kWhatVideoSizeChanged:
        {
            sp<AMessage> format;
            CHECK(msg->findMessage("format", &format));

            updateVideoSize(format);
            break;
        }

        case Source::kWhatBufferingUpdate:
        {
            int32_t percentage;
            CHECK(msg->findInt32("percentage", &percentage));

            notifyListener(MEDIA_BUFFERING_UPDATE, percentage, 0);
            break;
        }

        case Source::kWhatPauseOnBufferingStart:
        {
            // ignore if not playing
            if (mStarted) {
                ALOGI("buffer low, pausing...");

                startRebufferingTimer();
                mPausedForBuffering = true;
                onPause();
            }
            notifyListener(MEDIA_INFO, MEDIA_INFO_BUFFERING_START, 0);
            break;
        }

        case Source::kWhatResumeOnBufferingEnd:
        {
            // ignore if not playing
            if (mStarted) {
                ALOGI("buffer ready, resuming...");

                updateRebufferingTimer(true /* stopping */, false /* exiting */);
                mPausedForBuffering = false;

                // do not resume yet if client didn't unpause
                if (!mPausedByClient) {
                    onResume();
                }
            }
            notifyListener(MEDIA_INFO, MEDIA_INFO_BUFFERING_END, 0);
            break;
        }

        case Source::kWhatCacheStats:
        {
            int32_t kbps;
            CHECK(msg->findInt32("bandwidth", &kbps));

            notifyListener(MEDIA_INFO, MEDIA_INFO_NETWORK_BANDWIDTH, kbps);
            break;
        }

        case Source::kWhatSubtitleData:
        {
            sp<ABuffer> buffer;
            CHECK(msg->findBuffer("buffer", &buffer));

            sendSubtitleData(buffer, 0 /* baseIndex */);
            break;
        }

        case Source::kWhatTimedMetaData:
        {
            sp<ABuffer> buffer;
            if (!msg->findBuffer("buffer", &buffer)) {
                notifyListener(MEDIA_INFO, MEDIA_INFO_METADATA_UPDATE, 0);
            } else {
                sendTimedMetaData(buffer);
            }
            break;
        }

        case Source::kWhatTimedTextData:
        {
            int32_t generation;
            if (msg->findInt32("generation", &generation)
                    && generation != mTimedTextGeneration) {
                break;
            }

            sp<ABuffer> buffer;
            CHECK(msg->findBuffer("buffer", &buffer));

            sp<NuPlayerDriver> driver = mDriver.promote();
            if (driver == NULL) {
                break;
            }

            int posMs;
            int64_t timeUs, posUs;
            driver->getCurrentPosition(&posMs);
            posUs = (int64_t) posMs * 1000LL;
            CHECK(buffer->meta()->findInt64("timeUs", &timeUs));

            if (posUs < timeUs) {
                if (!msg->findInt32("generation", &generation)) {
                    msg->setInt32("generation", mTimedTextGeneration);
                }
                msg->post(timeUs - posUs);
            } else {
                sendTimedTextData(buffer);
            }
            break;
        }

        case Source::kWhatQueueDecoderShutdown:
        {
            int32_t audio, video;
            CHECK(msg->findInt32("audio", &audio));
            CHECK(msg->findInt32("video", &video));

            sp<AMessage> reply;
            CHECK(msg->findMessage("reply", &reply));

            queueDecoderShutdown(audio, video, reply);
            break;
        }

        case Source::kWhatDrmNoLicense:
        {
            notifyListener(MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, ERROR_DRM_NO_LICENSE);
            break;
        }

        default:
            TRESPASS();
    }
}

void NuPlayer::onClosedCaptionNotify(const sp<AMessage> &msg) {
    int32_t what;
    CHECK(msg->findInt32("what", &what));

    switch (what) {
        case NuPlayer::CCDecoder::kWhatClosedCaptionData:
        {
            sp<ABuffer> buffer;
            CHECK(msg->findBuffer("buffer", &buffer));

            size_t inbandTracks = 0;
            if (mSource != NULL) {
                inbandTracks = mSource->getTrackCount();
            }

            sendSubtitleData(buffer, inbandTracks);
            break;
        }

        case NuPlayer::CCDecoder::kWhatTrackAdded:
        {
            notifyListener(MEDIA_INFO, MEDIA_INFO_METADATA_UPDATE, 0);

            break;
        }

        default:
            TRESPASS();
    }


}

void NuPlayer::sendSubtitleData(const sp<ABuffer> &buffer, int32_t baseIndex) {
    int32_t trackIndex;
    int64_t timeUs, durationUs;
    CHECK(buffer->meta()->findInt32("track-index", &trackIndex));
    CHECK(buffer->meta()->findInt64("timeUs", &timeUs));
    CHECK(buffer->meta()->findInt64("durationUs", &durationUs));

    Parcel in;
    in.writeInt32(trackIndex + baseIndex);
    in.writeInt64(timeUs);
    in.writeInt64(durationUs);
    in.writeInt32(buffer->size());
    in.writeInt32(buffer->size());
    in.write(buffer->data(), buffer->size());

    notifyListener(MEDIA_SUBTITLE_DATA, 0, 0, &in);
}

void NuPlayer::sendTimedMetaData(const sp<ABuffer> &buffer) {
    int64_t timeUs;
    CHECK(buffer->meta()->findInt64("timeUs", &timeUs));

    Parcel in;
    in.writeInt64(timeUs);
    in.writeInt32(buffer->size());
    in.writeInt32(buffer->size());
    in.write(buffer->data(), buffer->size());

    notifyListener(MEDIA_META_DATA, 0, 0, &in);
}

void NuPlayer::sendTimedTextData(const sp<ABuffer> &buffer) {
    const void *data;
    size_t size = 0;
    int64_t timeUs;
    int32_t flag = TextDescriptions::IN_BAND_TEXT_3GPP;

    AString mime;
    CHECK(buffer->meta()->findString("mime", &mime));
    CHECK(strcasecmp(mime.c_str(), MEDIA_MIMETYPE_TEXT_3GPP) == 0);

    data = buffer->data();
    size = buffer->size();

    Parcel parcel;
    if (size > 0) {
        CHECK(buffer->meta()->findInt64("timeUs", &timeUs));
        int32_t global = 0;
        if (buffer->meta()->findInt32("global", &global) && global) {
            flag |= TextDescriptions::GLOBAL_DESCRIPTIONS;
        } else {
            flag |= TextDescriptions::LOCAL_DESCRIPTIONS;
        }
        TextDescriptions::getParcelOfDescriptions(
                (const uint8_t *)data, size, flag, timeUs / 1000, &parcel);
    }

    if ((parcel.dataSize() > 0)) {
        notifyListener(MEDIA_TIMED_TEXT, 0, 0, &parcel);
    } else {  // send an empty timed text
        notifyListener(MEDIA_TIMED_TEXT, 0, 0);
    }
}

const char *NuPlayer::getDataSourceType() {
    switch (mDataSourceType) {
        case DATA_SOURCE_TYPE_HTTP_LIVE:
            return "HTTPLive";

        case DATA_SOURCE_TYPE_RTSP:
            return "RTSP";

        case DATA_SOURCE_TYPE_GENERIC_URL:
            return "GenURL";

        case DATA_SOURCE_TYPE_GENERIC_FD:
            return "GenFD";

        case DATA_SOURCE_TYPE_MEDIA:
            return "Media";

        case DATA_SOURCE_TYPE_STREAM:
            return "Stream";

        case DATA_SOURCE_TYPE_NONE:
        default:
            return "None";
    }
 }

// Modular DRM begin
status_t NuPlayer::prepareDrm(const uint8_t uuid[16], const Vector<uint8_t> &drmSessionId)
{
    ALOGV("prepareDrm ");

    // Passing to the looper anyway; called in a pre-config prepared state so no race on mCrypto
    sp<AMessage> msg = new AMessage(kWhatPrepareDrm, this);
    // synchronous call so just passing the address but with local copies of "const" args
    uint8_t UUID[16];
    memcpy(UUID, uuid, sizeof(UUID));
    Vector<uint8_t> sessionId = drmSessionId;
    msg->setPointer("uuid", (void*)UUID);
    msg->setPointer("drmSessionId", (void*)&sessionId);

    sp<AMessage> response;
    status_t status = msg->postAndAwaitResponse(&response);

    if (status == OK && response != NULL) {
        CHECK(response->findInt32("status", &status));
        ALOGV("prepareDrm ret: %d ", status);
    } else {
        ALOGE("prepareDrm err: %d", status);
    }

    return status;
}

status_t NuPlayer::releaseDrm()
{
    ALOGV("releaseDrm ");

    sp<AMessage> msg = new AMessage(kWhatReleaseDrm, this);

    sp<AMessage> response;
    status_t status = msg->postAndAwaitResponse(&response);

    if (status == OK && response != NULL) {
        CHECK(response->findInt32("status", &status));
        ALOGV("releaseDrm ret: %d ", status);
    } else {
        ALOGE("releaseDrm err: %d", status);
    }

    return status;
}

status_t NuPlayer::onPrepareDrm(const sp<AMessage> &msg)
{
    // TODO change to ALOGV
    ALOGD("onPrepareDrm ");

    status_t status = INVALID_OPERATION;
    if (mSource == NULL) {
        ALOGE("onPrepareDrm: No source. onPrepareDrm failed with %d.", status);
        return status;
    }

    uint8_t *uuid;
    Vector<uint8_t> *drmSessionId;
    CHECK(msg->findPointer("uuid", (void**)&uuid));
    CHECK(msg->findPointer("drmSessionId", (void**)&drmSessionId));

    status = OK;
    sp<ICrypto> crypto = NULL;

    status = mSource->prepareDrm(uuid, *drmSessionId, &crypto);
    if (crypto == NULL) {
        ALOGE("onPrepareDrm: mSource->prepareDrm failed. status: %d", status);
        return status;
    }
    ALOGV("onPrepareDrm: mSource->prepareDrm succeeded");

    if (mCrypto != NULL) {
        ALOGE("onPrepareDrm: Unexpected. Already having mCrypto: %p (%d)",
                mCrypto.get(), mCrypto->getStrongCount());
        mCrypto.clear();
    }

    mCrypto = crypto;
    mIsDrmProtected = true;
    // TODO change to ALOGV
    ALOGD("onPrepareDrm: mCrypto: %p (%d)", mCrypto.get(),
            (mCrypto != NULL ? mCrypto->getStrongCount() : 0));

    return status;
}

status_t NuPlayer::onReleaseDrm()
{
    // TODO change to ALOGV
    ALOGD("onReleaseDrm ");

    if (!mIsDrmProtected) {
        ALOGW("onReleaseDrm: Unexpected. mIsDrmProtected is already false.");
    }

    mIsDrmProtected = false;

    status_t status;
    if (mCrypto != NULL) {
        // notifying the source first before removing crypto from codec
        if (mSource != NULL) {
            mSource->releaseDrm();
        }

        status=OK;
        // first making sure the codecs have released their crypto reference
        const sp<DecoderBase> &videoDecoder = getDecoder(false/*audio*/);
        if (videoDecoder != NULL) {
            status = videoDecoder->releaseCrypto();
            ALOGV("onReleaseDrm: video decoder ret: %d", status);
        }

        const sp<DecoderBase> &audioDecoder = getDecoder(true/*audio*/);
        if (audioDecoder != NULL) {
            status_t status_audio = audioDecoder->releaseCrypto();
            if (status == OK) {   // otherwise, returning the first error
                status = status_audio;
            }
            ALOGV("onReleaseDrm: audio decoder ret: %d", status_audio);
        }

        // TODO change to ALOGV
        ALOGD("onReleaseDrm: mCrypto: %p (%d)", mCrypto.get(),
                (mCrypto != NULL ? mCrypto->getStrongCount() : 0));
        mCrypto.clear();
    } else {   // mCrypto == NULL
        ALOGE("onReleaseDrm: Unexpected. There is no crypto.");
        status = INVALID_OPERATION;
    }

    return status;
}
// Modular DRM end
////////////////////////////////////////////////////////////////////////////////

sp<AMessage> NuPlayer::Source::getFormat(bool audio) {
    sp<MetaData> meta = getFormatMeta(audio);

    if (meta == NULL) {
        return NULL;
    }

    sp<AMessage> msg = new AMessage;

    if(convertMetaDataToMessage(meta, &msg) == OK) {
        return msg;
    }
    return NULL;
}

void NuPlayer::Source::notifyFlagsChanged(uint32_t flags) {
    sp<AMessage> notify = dupNotify();
    notify->setInt32("what", kWhatFlagsChanged);
    notify->setInt32("flags", flags);
    notify->post();
}

void NuPlayer::Source::notifyVideoSizeChanged(const sp<AMessage> &format) {
    sp<AMessage> notify = dupNotify();
    notify->setInt32("what", kWhatVideoSizeChanged);
    notify->setMessage("format", format);
    notify->post();
}

void NuPlayer::Source::notifyPrepared(status_t err) {
    ALOGV("Source::notifyPrepared %d", err);
    sp<AMessage> notify = dupNotify();
    notify->setInt32("what", kWhatPrepared);
    notify->setInt32("err", err);
    notify->post();
}

void NuPlayer::Source::notifyDrmInfo(const sp<ABuffer> &drmInfoBuffer)
{
    ALOGV("Source::notifyDrmInfo");

    sp<AMessage> notify = dupNotify();
    notify->setInt32("what", kWhatDrmInfo);
    notify->setBuffer("drmInfo", drmInfoBuffer);

    notify->post();
}

void NuPlayer::Source::notifyInstantiateSecureDecoders(const sp<AMessage> &reply) {
    sp<AMessage> notify = dupNotify();
    notify->setInt32("what", kWhatInstantiateSecureDecoders);
    notify->setMessage("reply", reply);
    notify->post();
}

void NuPlayer::Source::onMessageReceived(const sp<AMessage> & /* msg */) {
    TRESPASS();
}

}  // namespace android
