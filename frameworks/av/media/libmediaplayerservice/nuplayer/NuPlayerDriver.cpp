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
#define LOG_TAG "NuPlayerDriver"
#include <inttypes.h>
#include <android-base/macros.h>
#include <utils/Log.h>
#include <cutils/properties.h>

#include "NuPlayerDriver.h"

#include "NuPlayer.h"
#include "NuPlayerSource.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/MediaClock.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>

#include <media/IMediaAnalyticsService.h>

static const int kDumpLockRetries = 50;
static const int kDumpLockSleepUs = 20000;

namespace android {

// key for media statistics
static const char *kKeyPlayer = "nuplayer";
// attrs for media statistics
    // NB: these are matched with public Java API constants defined
    // in frameworks/base/media/java/android/media/MediaPlayer.java
    // These must be kept synchronized with the constants there.
static const char *kPlayerVMime = "android.media.mediaplayer.video.mime";
static const char *kPlayerVCodec = "android.media.mediaplayer.video.codec";
static const char *kPlayerWidth = "android.media.mediaplayer.width";
static const char *kPlayerHeight = "android.media.mediaplayer.height";
static const char *kPlayerFrames = "android.media.mediaplayer.frames";
static const char *kPlayerFramesDropped = "android.media.mediaplayer.dropped";
static const char *kPlayerFrameRate = "android.media.mediaplayer.fps";
static const char *kPlayerAMime = "android.media.mediaplayer.audio.mime";
static const char *kPlayerACodec = "android.media.mediaplayer.audio.codec";
static const char *kPlayerDuration = "android.media.mediaplayer.durationMs";
static const char *kPlayerPlaying = "android.media.mediaplayer.playingMs";
static const char *kPlayerError = "android.media.mediaplayer.err";
static const char *kPlayerErrorCode = "android.media.mediaplayer.errcode";

    // NB: These are not yet exposed as public Java API constants.
static const char *kPlayerErrorState = "android.media.mediaplayer.errstate";
static const char *kPlayerDataSourceType = "android.media.mediaplayer.dataSource";
//
static const char *kPlayerRebuffering = "android.media.mediaplayer.rebufferingMs";
static const char *kPlayerRebufferingCount = "android.media.mediaplayer.rebuffers";
static const char *kPlayerRebufferingAtExit = "android.media.mediaplayer.rebufferExit";


NuPlayerDriver::NuPlayerDriver(pid_t pid)
    : mState(STATE_IDLE),
      mIsAsyncPrepare(false),
      mAsyncResult(UNKNOWN_ERROR),
      mSetSurfaceInProgress(false),
      mDurationUs(-1),
      mPositionUs(-1),
      mSeekInProgress(false),
      mPlayingTimeUs(0),
      mRebufferingTimeUs(0),
      mRebufferingEvents(0),
      mRebufferingAtExit(false),
      mLooper(new ALooper),
      mMediaClock(new MediaClock),
      mPlayer(new NuPlayer(pid, mMediaClock)),
      mPlayerFlags(0),
      mAnalyticsItem(NULL),
      mClientUid(-1),
      mAtEOS(false),
      mLooping(false),
      mAutoLoop(false) {
    ALOGD("NuPlayerDriver(%p) created, clientPid(%d)", this, pid);
    mLooper->setName("NuPlayerDriver Looper");

    mMediaClock->init();

    // set up an analytics record
    mAnalyticsItem = MediaAnalyticsItem::create(kKeyPlayer);

    mLooper->start(
            false, /* runOnCallingThread */
            true,  /* canCallJava */
            PRIORITY_AUDIO);

    mLooper->registerHandler(mPlayer);

    mPlayer->init(this);
}

NuPlayerDriver::~NuPlayerDriver() {
    ALOGV("~NuPlayerDriver(%p)", this);
    mLooper->stop();

    // finalize any pending metrics, usually a no-op.
    updateMetrics("destructor");
    logMetrics("destructor");

    Mutex::Autolock autoLock(mMetricsLock);
    if (mAnalyticsItem != NULL) {
        delete mAnalyticsItem;
        mAnalyticsItem = NULL;
    }
}

status_t NuPlayerDriver::initCheck() {
    return OK;
}

status_t NuPlayerDriver::setUID(uid_t uid) {
    mPlayer->setUID(uid);
    mClientUid = uid;

    Mutex::Autolock autoLock(mMetricsLock);
    if (mAnalyticsItem) {
        mAnalyticsItem->setUid(mClientUid);
    }

    return OK;
}

status_t NuPlayerDriver::setDataSource(
        const sp<IMediaHTTPService> &httpService,
        const char *url,
        const KeyedVector<String8, String8> *headers) {
    ALOGV("setDataSource(%p) url(%s)", this, uriDebugString(url, false).c_str());
    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_IDLE) {
        return INVALID_OPERATION;
    }

    mState = STATE_SET_DATASOURCE_PENDING;

    mPlayer->setDataSourceAsync(httpService, url, headers);

    while (mState == STATE_SET_DATASOURCE_PENDING) {
        mCondition.wait(mLock);
    }

    return mAsyncResult;
}

status_t NuPlayerDriver::setDataSource(int fd, int64_t offset, int64_t length) {
    ALOGV("setDataSource(%p) file(%d)", this, fd);
    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_IDLE) {
        return INVALID_OPERATION;
    }

    mState = STATE_SET_DATASOURCE_PENDING;

    mPlayer->setDataSourceAsync(fd, offset, length);

    while (mState == STATE_SET_DATASOURCE_PENDING) {
        mCondition.wait(mLock);
    }

    return mAsyncResult;
}

status_t NuPlayerDriver::setDataSource(const sp<IStreamSource> &source) {
    ALOGV("setDataSource(%p) stream source", this);
    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_IDLE) {
        return INVALID_OPERATION;
    }

    mState = STATE_SET_DATASOURCE_PENDING;

    mPlayer->setDataSourceAsync(source);

    while (mState == STATE_SET_DATASOURCE_PENDING) {
        mCondition.wait(mLock);
    }

    return mAsyncResult;
}

status_t NuPlayerDriver::setDataSource(const sp<DataSource> &source) {
    ALOGV("setDataSource(%p) callback source", this);
    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_IDLE) {
        return INVALID_OPERATION;
    }

    mState = STATE_SET_DATASOURCE_PENDING;

    mPlayer->setDataSourceAsync(source);

    while (mState == STATE_SET_DATASOURCE_PENDING) {
        mCondition.wait(mLock);
    }

    return mAsyncResult;
}

status_t NuPlayerDriver::setVideoSurfaceTexture(
        const sp<IGraphicBufferProducer> &bufferProducer) {
    ALOGV("setVideoSurfaceTexture(%p)", this);
    Mutex::Autolock autoLock(mLock);

    if (mSetSurfaceInProgress) {
        return INVALID_OPERATION;
    }

    switch (mState) {
        case STATE_SET_DATASOURCE_PENDING:
        case STATE_RESET_IN_PROGRESS:
            return INVALID_OPERATION;

        default:
            break;
    }

    mSetSurfaceInProgress = true;

    mPlayer->setVideoSurfaceTextureAsync(bufferProducer);

    while (mSetSurfaceInProgress) {
        mCondition.wait(mLock);
    }

    return OK;
}

status_t NuPlayerDriver::getBufferingSettings(BufferingSettings* buffering) {
    ALOGV("getBufferingSettings(%p)", this);
    {
        Mutex::Autolock autoLock(mLock);
        if (mState == STATE_IDLE) {
            return INVALID_OPERATION;
        }
    }

    return mPlayer->getBufferingSettings(buffering);
}

status_t NuPlayerDriver::setBufferingSettings(const BufferingSettings& buffering) {
    ALOGV("setBufferingSettings(%p)", this);
    {
        Mutex::Autolock autoLock(mLock);
        if (mState == STATE_IDLE) {
            return INVALID_OPERATION;
        }
    }

    return mPlayer->setBufferingSettings(buffering);
}

status_t NuPlayerDriver::prepare() {
    ALOGV("prepare(%p)", this);
    Mutex::Autolock autoLock(mLock);
    return prepare_l();
}

status_t NuPlayerDriver::prepare_l() {
    switch (mState) {
        case STATE_UNPREPARED:
            mState = STATE_PREPARING;

            // Make sure we're not posting any notifications, success or
            // failure information is only communicated through our result
            // code.
            mIsAsyncPrepare = false;
            mPlayer->prepareAsync();
            while (mState == STATE_PREPARING) {
                mCondition.wait(mLock);
            }
            return (mState == STATE_PREPARED) ? OK : UNKNOWN_ERROR;
        case STATE_STOPPED:
            // this is really just paused. handle as seek to start
            mAtEOS = false;
            mState = STATE_STOPPED_AND_PREPARING;
            mIsAsyncPrepare = false;
            mPlayer->seekToAsync(0, MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC /* mode */,
                    true /* needNotify */);
            while (mState == STATE_STOPPED_AND_PREPARING) {
                mCondition.wait(mLock);
            }
            return (mState == STATE_STOPPED_AND_PREPARED) ? OK : UNKNOWN_ERROR;
        default:
            return INVALID_OPERATION;
    };
}

status_t NuPlayerDriver::prepareAsync() {
    ALOGV("prepareAsync(%p)", this);
    Mutex::Autolock autoLock(mLock);

    switch (mState) {
        case STATE_UNPREPARED:
            mState = STATE_PREPARING;
            mIsAsyncPrepare = true;
            mPlayer->prepareAsync();
            return OK;
        case STATE_STOPPED:
            // this is really just paused. handle as seek to start
            mAtEOS = false;
            mState = STATE_STOPPED_AND_PREPARING;
            mIsAsyncPrepare = true;
            mPlayer->seekToAsync(0, MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC /* mode */,
                    true /* needNotify */);
            return OK;
        default:
            return INVALID_OPERATION;
    };
}

status_t NuPlayerDriver::start() {
    ALOGV("start(%p), state is %d, eos is %d", this, mState, mAtEOS);
    Mutex::Autolock autoLock(mLock);
    return start_l();
}

status_t NuPlayerDriver::start_l() {
    switch (mState) {
        case STATE_UNPREPARED:
        {
            status_t err = prepare_l();

            if (err != OK) {
                return err;
            }

            CHECK_EQ(mState, STATE_PREPARED);

            FALLTHROUGH_INTENDED;
        }

        case STATE_PAUSED:
        case STATE_STOPPED_AND_PREPARED:
        case STATE_PREPARED:
        {
            mPlayer->start();

            FALLTHROUGH_INTENDED;
        }

        case STATE_RUNNING:
        {
            if (mAtEOS) {
                mPlayer->seekToAsync(0);
                mAtEOS = false;
                mPositionUs = -1;
            }
            break;
        }

        default:
            return INVALID_OPERATION;
    }

    mState = STATE_RUNNING;

    return OK;
}

status_t NuPlayerDriver::stop() {
    ALOGD("stop(%p)", this);
    Mutex::Autolock autoLock(mLock);

    switch (mState) {
        case STATE_RUNNING:
            mPlayer->pause();
            FALLTHROUGH_INTENDED;

        case STATE_PAUSED:
            mState = STATE_STOPPED;
            notifyListener_l(MEDIA_STOPPED);
            break;

        case STATE_PREPARED:
        case STATE_STOPPED:
        case STATE_STOPPED_AND_PREPARING:
        case STATE_STOPPED_AND_PREPARED:
            mState = STATE_STOPPED;
            break;

        default:
            return INVALID_OPERATION;
    }

    return OK;
}

status_t NuPlayerDriver::pause() {
    ALOGD("pause(%p)", this);
    // The NuPlayerRenderer may get flushed if pause for long enough, e.g. the pause timeout tear
    // down for audio offload mode. If that happens, the NuPlayerRenderer will no longer know the
    // current position. So similar to seekTo, update |mPositionUs| to the pause position by calling
    // getCurrentPosition here.
    int unused;
    getCurrentPosition(&unused);

    Mutex::Autolock autoLock(mLock);

    switch (mState) {
        case STATE_PAUSED:
        case STATE_PREPARED:
            return OK;

        case STATE_RUNNING:
            mState = STATE_PAUSED;
            notifyListener_l(MEDIA_PAUSED);
            mPlayer->pause();
            break;

        default:
            return INVALID_OPERATION;
    }

    return OK;
}

bool NuPlayerDriver::isPlaying() {
    return mState == STATE_RUNNING && !mAtEOS;
}

status_t NuPlayerDriver::setPlaybackSettings(const AudioPlaybackRate &rate) {
    status_t err = mPlayer->setPlaybackSettings(rate);
    if (err == OK) {
        // try to update position
        int unused;
        getCurrentPosition(&unused);
        Mutex::Autolock autoLock(mLock);
        if (rate.mSpeed == 0.f && mState == STATE_RUNNING) {
            mState = STATE_PAUSED;
            notifyListener_l(MEDIA_PAUSED);
        } else if (rate.mSpeed != 0.f
                && (mState == STATE_PAUSED
                    || mState == STATE_STOPPED_AND_PREPARED
                    || mState == STATE_PREPARED)) {
            err = start_l();
        }
    }
    return err;
}

status_t NuPlayerDriver::getPlaybackSettings(AudioPlaybackRate *rate) {
    return mPlayer->getPlaybackSettings(rate);
}

status_t NuPlayerDriver::setSyncSettings(const AVSyncSettings &sync, float videoFpsHint) {
    return mPlayer->setSyncSettings(sync, videoFpsHint);
}

status_t NuPlayerDriver::getSyncSettings(AVSyncSettings *sync, float *videoFps) {
    return mPlayer->getSyncSettings(sync, videoFps);
}

status_t NuPlayerDriver::seekTo(int msec, MediaPlayerSeekMode mode) {
    ALOGV("seekTo(%p) (%d ms, %d) at state %d", this, msec, mode, mState);
    Mutex::Autolock autoLock(mLock);

    int64_t seekTimeUs = msec * 1000LL;

    switch (mState) {
        case STATE_PREPARED:
        case STATE_STOPPED_AND_PREPARED:
        case STATE_PAUSED:
        case STATE_RUNNING:
        {
            mAtEOS = false;
            mSeekInProgress = true;
            // seeks can take a while, so we essentially paused
            notifyListener_l(MEDIA_PAUSED);
            mPlayer->seekToAsync(seekTimeUs, mode, true /* needNotify */);
            break;
        }

        default:
            return INVALID_OPERATION;
    }

    mPositionUs = seekTimeUs;
    return OK;
}

status_t NuPlayerDriver::getCurrentPosition(int *msec) {
    int64_t tempUs = 0;
    {
        Mutex::Autolock autoLock(mLock);
        if (mSeekInProgress || (mState == STATE_PAUSED && !mAtEOS)) {
            tempUs = (mPositionUs <= 0) ? 0 : mPositionUs;
            *msec = (int)divRound(tempUs, (int64_t)(1000));
            return OK;
        }
    }

    status_t ret = mPlayer->getCurrentPosition(&tempUs);

    Mutex::Autolock autoLock(mLock);
    // We need to check mSeekInProgress here because mPlayer->seekToAsync is an async call, which
    // means getCurrentPosition can be called before seek is completed. Iow, renderer may return a
    // position value that's different the seek to position.
    if (ret != OK) {
        tempUs = (mPositionUs <= 0) ? 0 : mPositionUs;
    } else {
        mPositionUs = tempUs;
    }
    *msec = (int)divRound(tempUs, (int64_t)(1000));
    return OK;
}

status_t NuPlayerDriver::getDuration(int *msec) {
    Mutex::Autolock autoLock(mLock);

    if (mDurationUs < 0) {
        return UNKNOWN_ERROR;
    }

    *msec = (mDurationUs + 500LL) / 1000;

    return OK;
}

void NuPlayerDriver::updateMetrics(const char *where) {

    if (where == NULL) {
        where = "unknown";
    }
    ALOGV("updateMetrics(%p) from %s at state %d", this, where, mState);

    // gather the final track statistics for this record
    Vector<sp<AMessage>> trackStats;
    mPlayer->getStats(&trackStats);

    // getDuration() uses mLock
    int duration_ms = -1;
    getDuration(&duration_ms);
    mAnalyticsItem->setInt64(kPlayerDuration, duration_ms);

    mPlayer->updateInternalTimers();

    int64_t playingTimeUs;
    int64_t rebufferingTimeUs;
    int32_t rebufferingEvents;
    bool rebufferingAtExit;
    {
        Mutex::Autolock autoLock(mLock);

        playingTimeUs = mPlayingTimeUs;
        rebufferingTimeUs = mRebufferingTimeUs;
        rebufferingEvents = mRebufferingEvents;
        rebufferingAtExit = mRebufferingAtExit;
    }

    // finish the rest of the gathering holding mLock;
    // some of the fields we read are updated under mLock.
    // we also avoid any races within mAnalyticsItem machinery
    Mutex::Autolock autoLock(mMetricsLock);

    mAnalyticsItem->setInt64(kPlayerPlaying, (playingTimeUs+500)/1000 );

    if (mRebufferingEvents != 0) {
        mAnalyticsItem->setInt64(kPlayerRebuffering, (rebufferingTimeUs+500)/1000 );
        mAnalyticsItem->setInt32(kPlayerRebufferingCount, rebufferingEvents);
        mAnalyticsItem->setInt32(kPlayerRebufferingAtExit, rebufferingAtExit);

     }

    mAnalyticsItem->setCString(kPlayerDataSourceType, mPlayer->getDataSourceType());

    if (trackStats.size() > 0) {
        for (size_t i = 0; i < trackStats.size(); ++i) {
            const sp<AMessage> &stats = trackStats.itemAt(i);

            AString mime;
            stats->findString("mime", &mime);

            AString name;
            stats->findString("component-name", &name);

            if (mime.startsWith("video/")) {
                int32_t width, height;
                mAnalyticsItem->setCString(kPlayerVMime, mime.c_str());
                if (!name.empty()) {
                    mAnalyticsItem->setCString(kPlayerVCodec, name.c_str());
                }

                if (stats->findInt32("width", &width)
                        && stats->findInt32("height", &height)) {
                    mAnalyticsItem->setInt32(kPlayerWidth, width);
                    mAnalyticsItem->setInt32(kPlayerHeight, height);
                }

                int64_t numFramesTotal = 0;
                int64_t numFramesDropped = 0;
                stats->findInt64("frames-total", &numFramesTotal);
                stats->findInt64("frames-dropped-output", &numFramesDropped);

                mAnalyticsItem->setInt64(kPlayerFrames, numFramesTotal);
                mAnalyticsItem->setInt64(kPlayerFramesDropped, numFramesDropped);

                float frameRate = 0;
                if (stats->findFloat("frame-rate-total", &frameRate)) {
                    mAnalyticsItem->setDouble(kPlayerFrameRate, (double) frameRate);
                }

            } else if (mime.startsWith("audio/")) {
                mAnalyticsItem->setCString(kPlayerAMime, mime.c_str());
                if (!name.empty()) {
                    mAnalyticsItem->setCString(kPlayerACodec, name.c_str());
                }
            }
        }
    }
}


void NuPlayerDriver::logMetrics(const char *where) {
    if (where == NULL) {
        where = "unknown";
    }
    ALOGV("logMetrics(%p) from %s at state %d", this, where, mState);

    // make sure that the stats are stable while we're writing them.
    Mutex::Autolock autoLock(mMetricsLock);

    if (mAnalyticsItem == NULL || mAnalyticsItem->isEnabled() == false) {
        return;
    }

    // log only non-empty records
    // we always updateMetrics() before we get here
    // and that always injects 3 fields (duration, playing time, and
    // datasource) into the record.
    // So the canonical "empty" record has 3 elements in it.
    if (mAnalyticsItem->count() > 3) {

        mAnalyticsItem->selfrecord();

        // re-init in case we prepare() and start() again.
        delete mAnalyticsItem ;
        mAnalyticsItem = MediaAnalyticsItem::create(kKeyPlayer);
        if (mAnalyticsItem) {
            mAnalyticsItem->setUid(mClientUid);
        }
    } else {
        ALOGV("nothing to record (only %d fields)", mAnalyticsItem->count());
    }
}

status_t NuPlayerDriver::reset() {
    ALOGD("reset(%p) at state %d", this, mState);

    updateMetrics("reset");
    logMetrics("reset");

    Mutex::Autolock autoLock(mLock);

    switch (mState) {
        case STATE_IDLE:
            return OK;

        case STATE_SET_DATASOURCE_PENDING:
        case STATE_RESET_IN_PROGRESS:
            return INVALID_OPERATION;

        case STATE_PREPARING:
        {
            CHECK(mIsAsyncPrepare);

            notifyListener_l(MEDIA_PREPARED);
            break;
        }

        default:
            break;
    }

    if (mState != STATE_STOPPED) {
        notifyListener_l(MEDIA_STOPPED);
    }

    if (property_get_bool("persist.debug.sf.stats", false)) {
        Vector<String16> args;
        dump(-1, args);
    }

    mState = STATE_RESET_IN_PROGRESS;
    mPlayer->resetAsync();

    while (mState == STATE_RESET_IN_PROGRESS) {
        mCondition.wait(mLock);
    }

    mDurationUs = -1;
    mPositionUs = -1;
    mLooping = false;
    mPlayingTimeUs = 0;
    mRebufferingTimeUs = 0;
    mRebufferingEvents = 0;
    mRebufferingAtExit = false;

    return OK;
}

status_t NuPlayerDriver::notifyAt(int64_t mediaTimeUs) {
    ALOGV("notifyAt(%p), time:%lld", this, (long long)mediaTimeUs);
    return mPlayer->notifyAt(mediaTimeUs);
}

status_t NuPlayerDriver::setLooping(int loop) {
    mLooping = loop != 0;
    return OK;
}

player_type NuPlayerDriver::playerType() {
    return NU_PLAYER;
}

status_t NuPlayerDriver::invoke(const Parcel &request, Parcel *reply) {
    if (reply == NULL) {
        ALOGE("reply is a NULL pointer");
        return BAD_VALUE;
    }

    int32_t methodId;
    status_t ret = request.readInt32(&methodId);
    if (ret != OK) {
        ALOGE("Failed to retrieve the requested method to invoke");
        return ret;
    }

    switch (methodId) {
        case INVOKE_ID_SET_VIDEO_SCALING_MODE:
        {
            int mode = request.readInt32();
            return mPlayer->setVideoScalingMode(mode);
        }

        case INVOKE_ID_GET_TRACK_INFO:
        {
            return mPlayer->getTrackInfo(reply);
        }

        case INVOKE_ID_SELECT_TRACK:
        {
            int trackIndex = request.readInt32();
            int msec = 0;
            // getCurrentPosition should always return OK
            getCurrentPosition(&msec);
            return mPlayer->selectTrack(trackIndex, true /* select */, msec * 1000LL);
        }

        case INVOKE_ID_UNSELECT_TRACK:
        {
            int trackIndex = request.readInt32();
            return mPlayer->selectTrack(trackIndex, false /* select */, 0xdeadbeef /* not used */);
        }

        case INVOKE_ID_GET_SELECTED_TRACK:
        {
            int32_t type = request.readInt32();
            return mPlayer->getSelectedTrack(type, reply);
        }

        default:
        {
            return INVALID_OPERATION;
        }
    }
}

void NuPlayerDriver::setAudioSink(const sp<AudioSink> &audioSink) {
    mPlayer->setAudioSink(audioSink);
    mAudioSink = audioSink;
}

status_t NuPlayerDriver::setParameter(
        int /* key */, const Parcel & /* request */) {
    return INVALID_OPERATION;
}

status_t NuPlayerDriver::getParameter(int key, Parcel *reply) {

    if (key == FOURCC('m','t','r','X')) {
        // mtrX -- a play on 'metrics' (not matrix)
        // gather current info all together, parcel it, and send it back
        updateMetrics("api");

        // ensure mAnalyticsItem stability while writing to parcel
        Mutex::Autolock autoLock(mMetricsLock);
        if (mAnalyticsItem != NULL) {
            mAnalyticsItem->writeToParcel(reply);
        }
        return OK;
    }

    return INVALID_OPERATION;
}

status_t NuPlayerDriver::getMetadata(
        const media::Metadata::Filter& /* ids */, Parcel *records) {
    Mutex::Autolock autoLock(mLock);

    using media::Metadata;

    Metadata meta(records);

    meta.appendBool(
            Metadata::kPauseAvailable,
            mPlayerFlags & NuPlayer::Source::FLAG_CAN_PAUSE);

    meta.appendBool(
            Metadata::kSeekBackwardAvailable,
            mPlayerFlags & NuPlayer::Source::FLAG_CAN_SEEK_BACKWARD);

    meta.appendBool(
            Metadata::kSeekForwardAvailable,
            mPlayerFlags & NuPlayer::Source::FLAG_CAN_SEEK_FORWARD);

    meta.appendBool(
            Metadata::kSeekAvailable,
            mPlayerFlags & NuPlayer::Source::FLAG_CAN_SEEK);

    return OK;
}

void NuPlayerDriver::notifyResetComplete() {
    ALOGD("notifyResetComplete(%p)", this);
    Mutex::Autolock autoLock(mLock);

    CHECK_EQ(mState, STATE_RESET_IN_PROGRESS);
    mState = STATE_IDLE;
    mCondition.broadcast();
}

void NuPlayerDriver::notifySetSurfaceComplete() {
    ALOGV("notifySetSurfaceComplete(%p)", this);
    Mutex::Autolock autoLock(mLock);

    CHECK(mSetSurfaceInProgress);
    mSetSurfaceInProgress = false;

    mCondition.broadcast();
}

void NuPlayerDriver::notifyDuration(int64_t durationUs) {
    Mutex::Autolock autoLock(mLock);
    mDurationUs = durationUs;
}

void NuPlayerDriver::notifyMorePlayingTimeUs(int64_t playingUs) {
    Mutex::Autolock autoLock(mLock);
    mPlayingTimeUs += playingUs;
}

void NuPlayerDriver::notifyMoreRebufferingTimeUs(int64_t rebufferingUs) {
    Mutex::Autolock autoLock(mLock);
    mRebufferingTimeUs += rebufferingUs;
    mRebufferingEvents++;
}

void NuPlayerDriver::notifyRebufferingWhenExit(bool status) {
    Mutex::Autolock autoLock(mLock);
    mRebufferingAtExit = status;
}

void NuPlayerDriver::notifySeekComplete() {
    ALOGV("notifySeekComplete(%p)", this);
    Mutex::Autolock autoLock(mLock);
    mSeekInProgress = false;
    notifySeekComplete_l();
}

void NuPlayerDriver::notifySeekComplete_l() {
    bool wasSeeking = true;
    if (mState == STATE_STOPPED_AND_PREPARING) {
        wasSeeking = false;
        mState = STATE_STOPPED_AND_PREPARED;
        mCondition.broadcast();
        if (!mIsAsyncPrepare) {
            // if we are preparing synchronously, no need to notify listener
            return;
        }
    } else if (mState == STATE_STOPPED) {
        // no need to notify listener
        return;
    }
    notifyListener_l(wasSeeking ? MEDIA_SEEK_COMPLETE : MEDIA_PREPARED);
}

status_t NuPlayerDriver::dump(
        int fd, const Vector<String16> & /* args */) const {

    Vector<sp<AMessage> > trackStats;
    mPlayer->getStats(&trackStats);

    AString logString(" NuPlayer\n");
    char buf[256] = {0};

    bool locked = false;
    for (int i = 0; i < kDumpLockRetries; ++i) {
        if (mLock.tryLock() == NO_ERROR) {
            locked = true;
            break;
        }
        usleep(kDumpLockSleepUs);
    }

    if (locked) {
        snprintf(buf, sizeof(buf), "  state(%d), atEOS(%d), looping(%d), autoLoop(%d)\n",
                mState, mAtEOS, mLooping, mAutoLoop);
        mLock.unlock();
    } else {
        snprintf(buf, sizeof(buf), "  NPD(%p) lock is taken\n", this);
    }
    logString.append(buf);

    for (size_t i = 0; i < trackStats.size(); ++i) {
        const sp<AMessage> &stats = trackStats.itemAt(i);

        AString mime;
        if (stats->findString("mime", &mime)) {
            snprintf(buf, sizeof(buf), "  mime(%s)\n", mime.c_str());
            logString.append(buf);
        }

        AString name;
        if (stats->findString("component-name", &name)) {
            snprintf(buf, sizeof(buf), "    decoder(%s)\n", name.c_str());
            logString.append(buf);
        }

        if (mime.startsWith("video/")) {
            int32_t width, height;
            if (stats->findInt32("width", &width)
                    && stats->findInt32("height", &height)) {
                snprintf(buf, sizeof(buf), "    resolution(%d x %d)\n", width, height);
                logString.append(buf);
            }

            int64_t numFramesTotal = 0;
            int64_t numFramesDropped = 0;

            stats->findInt64("frames-total", &numFramesTotal);
            stats->findInt64("frames-dropped-output", &numFramesDropped);
            snprintf(buf, sizeof(buf), "    numFramesTotal(%lld), numFramesDropped(%lld), "
                     "percentageDropped(%.2f%%)\n",
                     (long long)numFramesTotal,
                     (long long)numFramesDropped,
                     numFramesTotal == 0
                            ? 0.0 : (double)(numFramesDropped * 100) / numFramesTotal);
            logString.append(buf);
        }
    }

    ALOGI("%s", logString.c_str());

    if (fd >= 0) {
        FILE *out = fdopen(dup(fd), "w");
        fprintf(out, "%s", logString.c_str());
        fclose(out);
        out = NULL;
    }

    return OK;
}

void NuPlayerDriver::notifyListener(
        int msg, int ext1, int ext2, const Parcel *in) {
    Mutex::Autolock autoLock(mLock);
    notifyListener_l(msg, ext1, ext2, in);
}

void NuPlayerDriver::notifyListener_l(
        int msg, int ext1, int ext2, const Parcel *in) {
    ALOGV("notifyListener_l(%p), (%d, %d, %d, %d), loop setting(%d, %d)",
            this, msg, ext1, ext2, (in == NULL ? -1 : (int)in->dataSize()), mAutoLoop, mLooping);
    switch (msg) {
        case MEDIA_PLAYBACK_COMPLETE:
        {
            if (mState != STATE_RESET_IN_PROGRESS) {
                if (mAutoLoop) {
                    audio_stream_type_t streamType = AUDIO_STREAM_MUSIC;
                    if (mAudioSink != NULL) {
                        streamType = mAudioSink->getAudioStreamType();
                    }
                    if (streamType == AUDIO_STREAM_NOTIFICATION) {
                        ALOGW("disabling auto-loop for notification");
                        mAutoLoop = false;
                    }
                }
                if (mLooping || mAutoLoop) {
                    mPlayer->seekToAsync(0);
                    if (mAudioSink != NULL) {
                        // The renderer has stopped the sink at the end in order to play out
                        // the last little bit of audio. If we're looping, we need to restart it.
                        mAudioSink->start();
                    }
                    // don't send completion event when looping
                    return;
                }
                if (property_get_bool("persist.debug.sf.stats", false)) {
                    Vector<String16> args;
                    dump(-1, args);
                }
                mPlayer->pause();
                mState = STATE_PAUSED;
            }
            FALLTHROUGH_INTENDED;
        }

        case MEDIA_ERROR:
        {
            // when we have an error, add it to the analytics for this playback.
            // ext1 is our primary 'error type' value. Only add ext2 when non-zero.
            // [test against msg is due to fall through from previous switch value]
            if (msg == MEDIA_ERROR) {
                Mutex::Autolock autoLock(mMetricsLock);
                if (mAnalyticsItem != NULL) {
                    mAnalyticsItem->setInt32(kPlayerError, ext1);
                    if (ext2 != 0) {
                        mAnalyticsItem->setInt32(kPlayerErrorCode, ext2);
                    }
                    mAnalyticsItem->setCString(kPlayerErrorState, stateString(mState).c_str());
                }
            }
            mAtEOS = true;
            break;
        }

        default:
            break;
    }

    mLock.unlock();
    sendEvent(msg, ext1, ext2, in);
    mLock.lock();
}

void NuPlayerDriver::notifySetDataSourceCompleted(status_t err) {
    Mutex::Autolock autoLock(mLock);

    CHECK_EQ(mState, STATE_SET_DATASOURCE_PENDING);

    mAsyncResult = err;
    mState = (err == OK) ? STATE_UNPREPARED : STATE_IDLE;
    mCondition.broadcast();
}

void NuPlayerDriver::notifyPrepareCompleted(status_t err) {
    ALOGV("notifyPrepareCompleted %d", err);

    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_PREPARING) {
        // We were preparing asynchronously when the client called
        // reset(), we sent a premature "prepared" notification and
        // then initiated the reset. This notification is stale.
        CHECK(mState == STATE_RESET_IN_PROGRESS || mState == STATE_IDLE);
        return;
    }

    CHECK_EQ(mState, STATE_PREPARING);

    mAsyncResult = err;

    if (err == OK) {
        // update state before notifying client, so that if client calls back into NuPlayerDriver
        // in response, NuPlayerDriver has the right state
        mState = STATE_PREPARED;
        if (mIsAsyncPrepare) {
            notifyListener_l(MEDIA_PREPARED);
        }
    } else {
        mState = STATE_UNPREPARED;
        if (mIsAsyncPrepare) {
            notifyListener_l(MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, err);
        }
    }

    sp<MetaData> meta = mPlayer->getFileMeta();
    int32_t loop;
    if (meta != NULL
            && meta->findInt32(kKeyAutoLoop, &loop) && loop != 0) {
        mAutoLoop = true;
    }

    mCondition.broadcast();
}

void NuPlayerDriver::notifyFlagsChanged(uint32_t flags) {
    Mutex::Autolock autoLock(mLock);

    mPlayerFlags = flags;
}

// Modular DRM
status_t NuPlayerDriver::prepareDrm(const uint8_t uuid[16], const Vector<uint8_t> &drmSessionId)
{
    ALOGV("prepareDrm(%p) state: %d", this, mState);

    // leaving the state verification for mediaplayer.cpp
    status_t ret = mPlayer->prepareDrm(uuid, drmSessionId);

    ALOGV("prepareDrm ret: %d", ret);

    return ret;
}

status_t NuPlayerDriver::releaseDrm()
{
    ALOGV("releaseDrm(%p) state: %d", this, mState);

    // leaving the state verification for mediaplayer.cpp
    status_t ret = mPlayer->releaseDrm();

    ALOGV("releaseDrm ret: %d", ret);

    return ret;
}

std::string NuPlayerDriver::stateString(State state) {
    const char *rval = NULL;
    char rawbuffer[16];  // allows "%d"

    switch (state) {
        case STATE_IDLE: rval = "IDLE"; break;
        case STATE_SET_DATASOURCE_PENDING: rval = "SET_DATASOURCE_PENDING"; break;
        case STATE_UNPREPARED: rval = "UNPREPARED"; break;
        case STATE_PREPARING: rval = "PREPARING"; break;
        case STATE_PREPARED: rval = "PREPARED"; break;
        case STATE_RUNNING: rval = "RUNNING"; break;
        case STATE_PAUSED: rval = "PAUSED"; break;
        case STATE_RESET_IN_PROGRESS: rval = "RESET_IN_PROGRESS"; break;
        case STATE_STOPPED: rval = "STOPPED"; break;
        case STATE_STOPPED_AND_PREPARING: rval = "STOPPED_AND_PREPARING"; break;
        case STATE_STOPPED_AND_PREPARED: rval = "STOPPED_AND_PREPARED"; break;
        default:
            // yes, this buffer is shared and vulnerable to races
            snprintf(rawbuffer, sizeof(rawbuffer), "%d", state);
            rval = rawbuffer;
            break;
    }

    return rval;
}

}  // namespace android
