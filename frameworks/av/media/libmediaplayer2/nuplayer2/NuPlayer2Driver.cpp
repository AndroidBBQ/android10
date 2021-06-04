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
#define LOG_TAG "NuPlayer2Driver"
#include <inttypes.h>
#include <android-base/macros.h>
#include <utils/Log.h>
#include <cutils/properties.h>

#include "NuPlayer2Driver.h"

#include "NuPlayer2.h"
#include "NuPlayer2Source.h"

#include <media/DataSourceDesc.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/MediaClock.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>

#include <media/IMediaAnalyticsService.h>

using google::protobuf::RepeatedPtrField;
using android::media::MediaPlayer2Proto::Value;

static const int kDumpLockRetries = 50;
static const int kDumpLockSleepUs = 20000;

namespace android {

struct PlayerMessageWrapper : public RefBase {
    static sp<PlayerMessageWrapper> Create(const PlayerMessage *p) {
        if (p != NULL) {
            sp<PlayerMessageWrapper> pw = new PlayerMessageWrapper();
            pw->copyFrom(p);
            return pw;
        }
        return NULL;
    }

    const PlayerMessage *getPlayerMessage() {
        return mPlayerMessage;
    }

protected:
    virtual ~PlayerMessageWrapper() {
        if (mPlayerMessage != NULL) {
            delete mPlayerMessage;
        }
    }

private:
    PlayerMessageWrapper()
        : mPlayerMessage(NULL) { }

    void copyFrom(const PlayerMessage *p) {
        if (mPlayerMessage == NULL) {
            mPlayerMessage = new PlayerMessage;
        }
        mPlayerMessage->CopyFrom(*p);
    }

    PlayerMessage *mPlayerMessage;
};

// key for media statistics
static const char *kKeyPlayer = "nuplayer2";
// attrs for media statistics
    // NB: these are matched with public Java API constants defined
    // in frameworks/base/media/java/android/media/MediaPlayer2.java
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

static const char *kPlayerVersion = "android.media.mediaplayer.version";


NuPlayer2Driver::NuPlayer2Driver(pid_t pid, uid_t uid, const sp<JObjectHolder> &context)
    : mState(STATE_IDLE),
      mAsyncResult(UNKNOWN_ERROR),
      mSrcId(0),
      mSetSurfaceInProgress(false),
      mDurationUs(-1),
      mPositionUs(-1),
      mSeekInProgress(false),
      mPlayingTimeUs(0),
      mRebufferingTimeUs(0),
      mRebufferingEvents(0),
      mRebufferingAtExit(false),
      mLooper(new ALooper),
      mNuPlayer2Looper(new ALooper),
      mMediaClock(new MediaClock),
      mPlayer(new NuPlayer2(pid, uid, mMediaClock, context)),
      mPlayerFlags(0),
      mMetricsHandle(0),
      mPlayerVersion(0),
      mClientUid(uid),
      mAtEOS(false),
      mLooping(false),
      mAutoLoop(false) {
    ALOGD("NuPlayer2Driver(%p) created, clientPid(%d)", this, pid);
    mLooper->setName("NuPlayer2Driver Looper");
    mNuPlayer2Looper->setName("NuPlayer2 Looper");

    mMediaClock->init();

    // XXX: what version are we?
    // Ideally, this ticks with the apk version info for the APEX packaging

    // set up media metrics record
    mMetricsHandle = mediametrics_create(kKeyPlayer);
    mediametrics_setUid(mMetricsHandle, mClientUid);
    mediametrics_setInt64(mMetricsHandle, kPlayerVersion, mPlayerVersion);

    mNuPlayer2Looper->start(
            false, /* runOnCallingThread */
            true,  /* canCallJava */
            PRIORITY_AUDIO);

    mNuPlayer2Looper->registerHandler(mPlayer);

    mPlayer->setDriver(this);
}

NuPlayer2Driver::~NuPlayer2Driver() {
    ALOGV("~NuPlayer2Driver(%p)", this);
    mNuPlayer2Looper->stop();
    mLooper->stop();

    // finalize any pending metrics, usually a no-op.
    updateMetrics("destructor");
    logMetrics("destructor");

    mediametrics_delete(mMetricsHandle);
}

status_t NuPlayer2Driver::initCheck() {
    mLooper->start(
            false, /* runOnCallingThread */
            true,  /* canCallJava */
            PRIORITY_AUDIO);

    mLooper->registerHandler(this);
    return OK;
}

status_t NuPlayer2Driver::setDataSource(const sp<DataSourceDesc> &dsd) {
    ALOGV("setDataSource(%p)", this);
    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_IDLE) {
        return INVALID_OPERATION;
    }

    mSrcId = dsd->mId;
    mState = STATE_SET_DATASOURCE_PENDING;

    mPlayer->setDataSourceAsync(dsd);

    while (mState == STATE_SET_DATASOURCE_PENDING) {
        mCondition.wait(mLock);
    }

    return mAsyncResult;
}

status_t NuPlayer2Driver::prepareNextDataSource(const sp<DataSourceDesc> &dsd) {
    ALOGV("prepareNextDataSource(%p)", this);
    Mutex::Autolock autoLock(mLock);

    mPlayer->prepareNextDataSourceAsync(dsd);

    return OK;
}

status_t NuPlayer2Driver::playNextDataSource(int64_t srcId) {
    ALOGV("playNextDataSource(%p)", this);
    Mutex::Autolock autoLock(mLock);

    mSrcId = srcId;
    mPlayer->playNextDataSource(srcId);

    return OK;
}

status_t NuPlayer2Driver::setVideoSurfaceTexture(const sp<ANativeWindowWrapper> &nww) {
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

    mPlayer->setVideoSurfaceTextureAsync(nww);

    while (mSetSurfaceInProgress) {
        mCondition.wait(mLock);
    }

    return OK;
}

status_t NuPlayer2Driver::getBufferingSettings(BufferingSettings* buffering) {
    ALOGV("getBufferingSettings(%p)", this);
    {
        Mutex::Autolock autoLock(mLock);
        if (mState == STATE_IDLE) {
            return INVALID_OPERATION;
        }
    }

    return mPlayer->getBufferingSettings(buffering);
}

status_t NuPlayer2Driver::setBufferingSettings(const BufferingSettings& buffering) {
    ALOGV("setBufferingSettings(%p)", this);
    {
        Mutex::Autolock autoLock(mLock);
        if (mState == STATE_IDLE) {
            return INVALID_OPERATION;
        }
    }

    return mPlayer->setBufferingSettings(buffering);
}

status_t NuPlayer2Driver::prepareAsync() {
    ALOGV("prepareAsync(%p)", this);
    Mutex::Autolock autoLock(mLock);

    switch (mState) {
        case STATE_UNPREPARED:
            mState = STATE_PREPARING;
            mPlayer->prepareAsync();
            return OK;
        default:
            return INVALID_OPERATION;
    };
}

status_t NuPlayer2Driver::start() {
    ALOGD("start(%p), state is %d, eos is %d", this, mState, mAtEOS);
    Mutex::Autolock autoLock(mLock);
    return start_l();
}

status_t NuPlayer2Driver::start_l() {
    switch (mState) {
        case STATE_PAUSED:
        case STATE_PREPARED:
        {
            mPlayer->start();
            FALLTHROUGH_INTENDED;
        }

        case STATE_RUNNING:
        {
            if (mAtEOS) {
                mPlayer->rewind();
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

status_t NuPlayer2Driver::pause() {
    ALOGD("pause(%p)", this);
    // The NuPlayerRenderer may get flushed if pause for long enough, e.g. the pause timeout tear
    // down for audio offload mode. If that happens, the NuPlayerRenderer will no longer know the
    // current position. So similar to seekTo, update |mPositionUs| to the pause position by calling
    // getCurrentPosition here.
    int64_t unused;
    getCurrentPosition(&unused);

    Mutex::Autolock autoLock(mLock);

    switch (mState) {
        case STATE_PAUSED:
            return OK;

        case STATE_PREPARED:
        case STATE_RUNNING:
            mState = STATE_PAUSED;
            mPlayer->pause();
            break;

        default:
            return INVALID_OPERATION;
    }

    return OK;
}

bool NuPlayer2Driver::isPlaying() {
    return mState == STATE_RUNNING && !mAtEOS;
}

status_t NuPlayer2Driver::setPlaybackSettings(const AudioPlaybackRate &rate) {
    status_t err = mPlayer->setPlaybackSettings(rate);
    if (err == OK) {
        // try to update position
        int64_t unused;
        getCurrentPosition(&unused);
    }
    return err;
}

status_t NuPlayer2Driver::getPlaybackSettings(AudioPlaybackRate *rate) {
    return mPlayer->getPlaybackSettings(rate);
}

status_t NuPlayer2Driver::setSyncSettings(const AVSyncSettings &sync, float videoFpsHint) {
    return mPlayer->setSyncSettings(sync, videoFpsHint);
}

status_t NuPlayer2Driver::getSyncSettings(AVSyncSettings *sync, float *videoFps) {
    return mPlayer->getSyncSettings(sync, videoFps);
}

status_t NuPlayer2Driver::seekTo(int64_t msec, MediaPlayer2SeekMode mode) {
    ALOGD("seekTo(%p) (%lld ms, %d) at state %d", this, (long long)msec, mode, mState);
    Mutex::Autolock autoLock(mLock);

    int64_t seekTimeUs = msec * 1000LL;

    switch (mState) {
        case STATE_PREPARED:
        case STATE_PAUSED:
        case STATE_RUNNING:
        {
            mAtEOS = false;
            mSeekInProgress = true;
            mPlayer->seekToAsync(seekTimeUs, mode, true /* needNotify */);
            break;
        }

        default:
            return INVALID_OPERATION;
    }

    mPositionUs = seekTimeUs;
    return OK;
}

status_t NuPlayer2Driver::getCurrentPosition(int64_t *msec) {
    int64_t tempUs = 0;
    {
        Mutex::Autolock autoLock(mLock);
        if (mSeekInProgress || (mState == STATE_PAUSED && !mAtEOS)) {
            tempUs = (mPositionUs <= 0) ? 0 : mPositionUs;
            *msec = divRound(tempUs, (int64_t)(1000));
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
    *msec = divRound(tempUs, (int64_t)(1000));
    return OK;
}

status_t NuPlayer2Driver::getDuration(int64_t *msec) {
    Mutex::Autolock autoLock(mLock);

    if (mDurationUs < 0) {
        return UNKNOWN_ERROR;
    }

    *msec = (mDurationUs + 500LL) / 1000;

    return OK;
}

void NuPlayer2Driver::updateMetrics(const char *where) {
    if (where == NULL) {
        where = "unknown";
    }
    ALOGV("updateMetrics(%p) from %s at state %d", this, where, mState);

    // gather the final stats for this record
    Vector<sp<AMessage>> trackStats;
    mPlayer->getStats(&trackStats);

    if (trackStats.size() > 0) {
        for (size_t i = 0; i < trackStats.size(); ++i) {
            const sp<AMessage> &stats = trackStats.itemAt(i);

            AString mime;
            stats->findString("mime", &mime);

            AString name;
            stats->findString("component-name", &name);

            if (mime.startsWith("video/")) {
                int32_t width, height;
                mediametrics_setCString(mMetricsHandle, kPlayerVMime, mime.c_str());
                if (!name.empty()) {
                    mediametrics_setCString(mMetricsHandle, kPlayerVCodec, name.c_str());
                }

                if (stats->findInt32("width", &width)
                        && stats->findInt32("height", &height)) {
                    mediametrics_setInt32(mMetricsHandle, kPlayerWidth, width);
                    mediametrics_setInt32(mMetricsHandle, kPlayerHeight, height);
                }

                int64_t numFramesTotal = 0;
                int64_t numFramesDropped = 0;
                stats->findInt64("frames-total", &numFramesTotal);
                stats->findInt64("frames-dropped-output", &numFramesDropped);

                mediametrics_setInt64(mMetricsHandle, kPlayerFrames, numFramesTotal);
                mediametrics_setInt64(mMetricsHandle, kPlayerFramesDropped, numFramesDropped);

                float frameRate = 0;
                if (stats->findFloat("frame-rate-output", &frameRate)) {
                    mediametrics_setInt64(mMetricsHandle, kPlayerFrameRate, frameRate);
                }

            } else if (mime.startsWith("audio/")) {
                mediametrics_setCString(mMetricsHandle, kPlayerAMime, mime.c_str());
                if (!name.empty()) {
                    mediametrics_setCString(mMetricsHandle, kPlayerACodec, name.c_str());
                }
            }
        }
    }

    // always provide duration and playing time, even if they have 0/unknown values.

    // getDuration() uses mLock for mutex -- careful where we use it.
    int64_t duration_ms = -1;
    getDuration(&duration_ms);
    mediametrics_setInt64(mMetricsHandle, kPlayerDuration, duration_ms);

    mediametrics_setInt64(mMetricsHandle, kPlayerPlaying, (mPlayingTimeUs+500)/1000 );

    if (mRebufferingEvents != 0) {
        mediametrics_setInt64(mMetricsHandle, kPlayerRebuffering, (mRebufferingTimeUs+500)/1000 );
        mediametrics_setInt32(mMetricsHandle, kPlayerRebufferingCount, mRebufferingEvents);
        mediametrics_setInt32(mMetricsHandle, kPlayerRebufferingAtExit, mRebufferingAtExit);
    }

    mediametrics_setCString(mMetricsHandle, kPlayerDataSourceType, mPlayer->getDataSourceType());
}


void NuPlayer2Driver::logMetrics(const char *where) {
    if (where == NULL) {
        where = "unknown";
    }
    ALOGV("logMetrics(%p) from %s at state %d", this, where, mState);

    if (mMetricsHandle == 0 || mediametrics_isEnabled() == false) {
        return;
    }

    // log only non-empty records
    // we always updateMetrics() before we get here
    // and that always injects 3 fields (duration, playing time, and
    // datasource) into the record.
    // So the canonical "empty" record has 3 elements in it.
    if (mediametrics_count(mMetricsHandle) > 3) {
        mediametrics_selfRecord(mMetricsHandle);
        // re-init in case we prepare() and start() again.
        mediametrics_delete(mMetricsHandle);
        mMetricsHandle = mediametrics_create(kKeyPlayer);
        mediametrics_setUid(mMetricsHandle, mClientUid);
        mediametrics_setInt64(mMetricsHandle, kPlayerVersion, mPlayerVersion);
    } else {
        ALOGV("did not have anything to record");
    }
}

status_t NuPlayer2Driver::reset() {
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
            notifyListener_l(mSrcId, MEDIA2_PREPARED);
            break;
        }

        default:
            break;
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

status_t NuPlayer2Driver::notifyAt(int64_t mediaTimeUs) {
    ALOGV("notifyAt(%p), time:%lld", this, (long long)mediaTimeUs);
    return mPlayer->notifyAt(mediaTimeUs);
}

status_t NuPlayer2Driver::setLooping(int loop) {
    mLooping = loop != 0;
    return OK;
}

status_t NuPlayer2Driver::invoke(const PlayerMessage &request, PlayerMessage *response) {
    if (response == NULL) {
        ALOGE("reply is a NULL pointer");
        return BAD_VALUE;
    }

    RepeatedPtrField<const Value>::const_iterator it = request.values().cbegin();
    int32_t methodId = (it++)->int32_value();

    switch (methodId) {
        case MEDIA_PLAYER2_INVOKE_ID_SET_VIDEO_SCALING_MODE:
        {
            int mode = (it++)->int32_value();
            return mPlayer->setVideoScalingMode(mode);
        }

        case MEDIA_PLAYER2_INVOKE_ID_GET_TRACK_INFO:
        {
            int64_t srcId = (it++)->int64_value();
            return mPlayer->getTrackInfo(srcId, response);
        }

        case MEDIA_PLAYER2_INVOKE_ID_SELECT_TRACK:
        {
            int64_t srcId = (it++)->int64_value();
            int trackIndex = (it++)->int32_value();
            int64_t msec = 0;
            // getCurrentPosition should always return OK
            getCurrentPosition(&msec);
            return mPlayer->selectTrack(srcId, trackIndex, true /* select */, msec * 1000LL);
        }

        case MEDIA_PLAYER2_INVOKE_ID_UNSELECT_TRACK:
        {
            int64_t srcId = (it++)->int64_value();
            int trackIndex = (it++)->int32_value();
            return mPlayer->selectTrack(
                    srcId, trackIndex, false /* select */, 0xdeadbeef /* not used */);
        }

        case MEDIA_PLAYER2_INVOKE_ID_GET_SELECTED_TRACK:
        {
            int64_t srcId = (it++)->int64_value();
            int32_t type = (it++)->int32_value();
            return mPlayer->getSelectedTrack(srcId, type, response);
        }

        default:
        {
            return INVALID_OPERATION;
        }
    }
}

void NuPlayer2Driver::setAudioSink(const sp<AudioSink> &audioSink) {
    mPlayer->setAudioSink(audioSink);
    mAudioSink = audioSink;
}

status_t NuPlayer2Driver::setParameter(
        int /* key */, const Parcel & /* request */) {
    return INVALID_OPERATION;
}

status_t NuPlayer2Driver::getParameter(int key __unused, Parcel *reply __unused) {
    return INVALID_OPERATION;
}

status_t NuPlayer2Driver::getMetrics(char **buffer, size_t *length) {
    updateMetrics("api");
    if (mediametrics_getAttributes(mMetricsHandle, buffer, length))
        return OK;
    else
        return FAILED_TRANSACTION;
}

void NuPlayer2Driver::notifyResetComplete(int64_t /* srcId */) {
    ALOGD("notifyResetComplete(%p)", this);
    Mutex::Autolock autoLock(mLock);

    CHECK_EQ(mState, STATE_RESET_IN_PROGRESS);
    mState = STATE_IDLE;
    mCondition.broadcast();
}

void NuPlayer2Driver::notifySetSurfaceComplete(int64_t /* srcId */) {
    ALOGV("notifySetSurfaceComplete(%p)", this);
    Mutex::Autolock autoLock(mLock);

    CHECK(mSetSurfaceInProgress);
    mSetSurfaceInProgress = false;

    mCondition.broadcast();
}

void NuPlayer2Driver::notifyDuration(int64_t /* srcId */, int64_t durationUs) {
    Mutex::Autolock autoLock(mLock);
    mDurationUs = durationUs;
}

void NuPlayer2Driver::notifyMorePlayingTimeUs(int64_t /* srcId */, int64_t playingUs) {
    Mutex::Autolock autoLock(mLock);
    mPlayingTimeUs += playingUs;
}

void NuPlayer2Driver::notifyMoreRebufferingTimeUs(int64_t /* srcId */, int64_t rebufferingUs) {
    Mutex::Autolock autoLock(mLock);
    mRebufferingTimeUs += rebufferingUs;
    mRebufferingEvents++;
}

void NuPlayer2Driver::notifyRebufferingWhenExit(int64_t /* srcId */, bool status) {
    Mutex::Autolock autoLock(mLock);
    mRebufferingAtExit = status;
}

void NuPlayer2Driver::notifySeekComplete(int64_t srcId) {
    ALOGV("notifySeekComplete(%p)", this);
    Mutex::Autolock autoLock(mLock);
    mSeekInProgress = false;
    notifyListener_l(srcId, MEDIA2_SEEK_COMPLETE);
}

status_t NuPlayer2Driver::dump(
        int fd, const Vector<String16> & /* args */) const {

    Vector<sp<AMessage> > trackStats;
    mPlayer->getStats(&trackStats);

    AString logString(" NuPlayer2\n");
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

void NuPlayer2Driver::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatNotifyListener: {
            int64_t srcId;
            int32_t msgId;
            int32_t ext1 = 0;
            int32_t ext2 = 0;
            CHECK(msg->findInt64("srcId", &srcId));
            CHECK(msg->findInt32("messageId", &msgId));
            msg->findInt32("ext1", &ext1);
            msg->findInt32("ext2", &ext2);
            sp<PlayerMessageWrapper> in;
            sp<RefBase> obj;
            if (msg->findObject("obj", &obj) && obj != NULL) {
                in = static_cast<PlayerMessageWrapper *>(obj.get());
            }
            sendEvent(srcId, msgId, ext1, ext2, (in == NULL ? NULL : in->getPlayerMessage()));
            break;
        }
        default:
            break;
    }
}

void NuPlayer2Driver::notifyListener(
        int64_t srcId, int msg, int ext1, int ext2, const PlayerMessage *in) {
    Mutex::Autolock autoLock(mLock);
    notifyListener_l(srcId, msg, ext1, ext2, in);
}

void NuPlayer2Driver::notifyListener_l(
        int64_t srcId, int msg, int ext1, int ext2, const PlayerMessage *in) {
    ALOGD("notifyListener_l(%p), (%lld, %d, %d, %d, %d), loop setting(%d, %d)",
            this, (long long)srcId, msg, ext1, ext2,
            (in == NULL ? -1 : (int)in->ByteSize()), mAutoLoop, mLooping);
    if (srcId == mSrcId) {
        switch (msg) {
            case MEDIA2_PLAYBACK_COMPLETE:
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
                        mPlayer->rewind();
                        if (mAudioSink != NULL) {
                            // The renderer has stopped the sink at the end in order to play out
                            // the last little bit of audio. In looping mode, we need to restart it.
                            mAudioSink->start();
                        }

                        sp<AMessage> notify = new AMessage(kWhatNotifyListener, this);
                        notify->setInt64("srcId", srcId);
                        notify->setInt32("messageId", MEDIA2_INFO);
                        notify->setInt32("ext1", MEDIA2_INFO_DATA_SOURCE_REPEAT);
                        notify->post();
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

            case MEDIA2_ERROR:
            {
                // when we have an error, add it to the analytics for this playback.
                // ext1 is our primary 'error type' value. Only add ext2 when non-zero.
                // [test against msg is due to fall through from previous switch value]
                if (msg == MEDIA2_ERROR) {
                    mediametrics_setInt32(mMetricsHandle, kPlayerError, ext1);
                    if (ext2 != 0) {
                        mediametrics_setInt32(mMetricsHandle, kPlayerErrorCode, ext2);
                    }
                    mediametrics_setCString(mMetricsHandle, kPlayerErrorState, stateString(mState).c_str());
                }
                mAtEOS = true;
                break;
            }

            default:
                break;
        }
    }

    sp<AMessage> notify = new AMessage(kWhatNotifyListener, this);
    notify->setInt64("srcId", srcId);
    notify->setInt32("messageId", msg);
    notify->setInt32("ext1", ext1);
    notify->setInt32("ext2", ext2);
    notify->setObject("obj", PlayerMessageWrapper::Create((PlayerMessage*)in));
    notify->post();
}

void NuPlayer2Driver::notifySetDataSourceCompleted(int64_t /* srcId */, status_t err) {
    Mutex::Autolock autoLock(mLock);

    CHECK_EQ(mState, STATE_SET_DATASOURCE_PENDING);

    mAsyncResult = err;
    mState = (err == OK) ? STATE_UNPREPARED : STATE_IDLE;
    mCondition.broadcast();
}

void NuPlayer2Driver::notifyPrepareCompleted(int64_t srcId, status_t err) {
    ALOGV("notifyPrepareCompleted %d", err);

    Mutex::Autolock autoLock(mLock);

    if (srcId != mSrcId) {
        if (err == OK) {
            notifyListener_l(srcId, MEDIA2_PREPARED);
        } else {
            notifyListener_l(srcId, MEDIA2_ERROR, MEDIA2_ERROR_UNKNOWN, err);
        }
        return;
    }

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
        // update state before notifying client, so that if client calls back into NuPlayer2Driver
        // in response, NuPlayer2Driver has the right state
        mState = STATE_PREPARED;
        notifyListener_l(srcId, MEDIA2_PREPARED);
    } else {
        mState = STATE_UNPREPARED;
        notifyListener_l(srcId, MEDIA2_ERROR, MEDIA2_ERROR_UNKNOWN, err);
    }

    sp<MetaData> meta = mPlayer->getFileMeta();
    int32_t loop;
    if (meta != NULL
            && meta->findInt32(kKeyAutoLoop, &loop) && loop != 0) {
        mAutoLoop = true;
    }

    mCondition.broadcast();
}

void NuPlayer2Driver::notifyFlagsChanged(int64_t /* srcId */, uint32_t flags) {
    Mutex::Autolock autoLock(mLock);

    mPlayerFlags = flags;
}

// Modular DRM
status_t NuPlayer2Driver::prepareDrm(
        int64_t srcId, const uint8_t uuid[16], const Vector<uint8_t> &drmSessionId)
{
    ALOGV("prepareDrm(%p) state: %d", this, mState);

    // leaving the state verification for mediaplayer.cpp
    status_t ret = mPlayer->prepareDrm(srcId, uuid, drmSessionId);

    ALOGV("prepareDrm ret: %d", ret);

    return ret;
}

status_t NuPlayer2Driver::releaseDrm(int64_t srcId)
{
    ALOGV("releaseDrm(%p) state: %d", this, mState);

    // leaving the state verification for mediaplayer.cpp
    status_t ret = mPlayer->releaseDrm(srcId);

    ALOGV("releaseDrm ret: %d", ret);

    return ret;
}

std::string NuPlayer2Driver::stateString(State state) {
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
        default:
            // yes, this buffer is shared and vulnerable to races
            snprintf(rawbuffer, sizeof(rawbuffer), "%d", state);
            rval = rawbuffer;
            break;
    }

    return rval;
}

}  // namespace android
