/*
**
** Copyright 2017, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaPlayer2Native"

#include <android/binder_ibinder.h>
#include <media/AudioSystem.h>
#include <media/DataSourceDesc.h>
#include <media/MemoryLeakTrackUtil.h>
#include <media/NdkWrapper.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooperRoster.h>
#include <mediaplayer2/MediaPlayer2AudioOutput.h>
#include <mediaplayer2/mediaplayer2.h>

#include <utils/Log.h>
#include <utils/SortedVector.h>
#include <utils/String8.h>

#include <system/audio.h>
#include <system/window.h>

#include <nuplayer2/NuPlayer2Driver.h>

#include <dirent.h>
#include <sys/stat.h>

namespace android {

extern ALooperRoster gLooperRoster;

namespace {

const int kDumpLockRetries = 50;
const int kDumpLockSleepUs = 20000;

class proxyListener : public MediaPlayer2InterfaceListener {
public:
    proxyListener(const wp<MediaPlayer2> &player)
        : mPlayer(player) { }

    ~proxyListener() { };

    virtual void notify(int64_t srcId, int msg, int ext1, int ext2,
            const PlayerMessage *obj) override {
        sp<MediaPlayer2> player = mPlayer.promote();
        if (player != NULL) {
            player->notify(srcId, msg, ext1, ext2, obj);
        }
    }

private:
    wp<MediaPlayer2> mPlayer;
};

Mutex sRecordLock;
SortedVector<wp<MediaPlayer2> > *sPlayers;

void ensureInit_l() {
    if (sPlayers == NULL) {
        sPlayers = new SortedVector<wp<MediaPlayer2> >();
    }
}

void addPlayer(const wp<MediaPlayer2>& player) {
    Mutex::Autolock lock(sRecordLock);
    ensureInit_l();
    sPlayers->add(player);
}

void removePlayer(const wp<MediaPlayer2>& player) {
    Mutex::Autolock lock(sRecordLock);
    ensureInit_l();
    sPlayers->remove(player);
}

/**
 * The only arguments this understands right now are -c, -von and -voff,
 * which are parsed by ALooperRoster::dump()
 */
status_t dumpPlayers(int fd, const Vector<String16>& args) {
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    SortedVector< sp<MediaPlayer2> > players; //to serialise the mutex unlock & client destruction.

    {
        Mutex::Autolock lock(sRecordLock);
        ensureInit_l();
        for (int i = 0, n = sPlayers->size(); i < n; ++i) {
            sp<MediaPlayer2> p = (*sPlayers)[i].promote();
            if (p != 0) {
                p->dump(fd, args);
            }
            players.add(p);
        }
    }

    result.append(" Files opened and/or mapped:\n");
    snprintf(buffer, SIZE, "/proc/%d/maps", getpid());
    FILE *f = fopen(buffer, "r");
    if (f) {
        while (!feof(f)) {
            fgets(buffer, SIZE, f);
            if (strstr(buffer, " /storage/") ||
                strstr(buffer, " /system/sounds/") ||
                strstr(buffer, " /data/") ||
                strstr(buffer, " /system/media/")) {
                result.append("  ");
                result.append(buffer);
            }
        }
        fclose(f);
    } else {
        result.append("couldn't open ");
        result.append(buffer);
        result.append("\n");
    }

    snprintf(buffer, SIZE, "/proc/%d/fd", getpid());
    DIR *d = opendir(buffer);
    if (d) {
        struct dirent *ent;
        while((ent = readdir(d)) != NULL) {
            if (strcmp(ent->d_name,".") && strcmp(ent->d_name,"..")) {
                snprintf(buffer, SIZE, "/proc/%d/fd/%s", getpid(), ent->d_name);
                struct stat s;
                if (lstat(buffer, &s) == 0) {
                    if ((s.st_mode & S_IFMT) == S_IFLNK) {
                        char linkto[256];
                        int len = readlink(buffer, linkto, sizeof(linkto));
                        if(len > 0) {
                            if(len > 255) {
                                linkto[252] = '.';
                                linkto[253] = '.';
                                linkto[254] = '.';
                                linkto[255] = 0;
                            } else {
                                linkto[len] = 0;
                            }
                            if (strstr(linkto, "/storage/") == linkto ||
                                strstr(linkto, "/system/sounds/") == linkto ||
                                strstr(linkto, "/data/") == linkto ||
                                strstr(linkto, "/system/media/") == linkto) {
                                result.append("  ");
                                result.append(buffer);
                                result.append(" -> ");
                                result.append(linkto);
                                result.append("\n");
                            }
                        }
                    } else {
                        result.append("  unexpected type for ");
                        result.append(buffer);
                        result.append("\n");
                    }
                }
            }
        }
        closedir(d);
    } else {
        result.append("couldn't open ");
        result.append(buffer);
        result.append("\n");
    }

    gLooperRoster.dump(fd, args);

    bool dumpMem = false;
    bool unreachableMemory = false;
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == String16("-m")) {
            dumpMem = true;
        } else if (args[i] == String16("--unreachable")) {
            unreachableMemory = true;
        }
    }
    if (dumpMem) {
        result.append("\nDumping memory:\n");
        std::string s = dumpMemoryAddresses(100 /* limit */);
        result.append(s.c_str(), s.size());
    }
    if (unreachableMemory) {
        result.append("\nDumping unreachable memory:\n");
        // TODO - should limit be an argument parameter?
        // TODO: enable GetUnreachableMemoryString if it's part of stable API
        //std::string s = GetUnreachableMemoryString(true /* contents */, 10000 /* limit */);
        //result.append(s.c_str(), s.size());
    }

    write(fd, result.string(), result.size());
    return NO_ERROR;
}

}  // anonymous namespace

//static
sp<MediaPlayer2> MediaPlayer2::Create(int32_t sessionId, jobject context) {
    sp<MediaPlayer2> player = new MediaPlayer2(sessionId, context);

    if (!player->init()) {
        return NULL;
    }

    ALOGV("Create new player(%p)", player.get());

    addPlayer(player);
    return player;
}

// static
status_t MediaPlayer2::DumpAll(int fd, const Vector<String16>& args) {
    return dumpPlayers(fd, args);
}

MediaPlayer2::MediaPlayer2(int32_t sessionId, jobject context) {
    ALOGV("constructor");
    mSrcId = 0;
    mLockThreadId = 0;
    mListener = NULL;
    mStreamType = AUDIO_STREAM_MUSIC;
    mAudioAttributes = NULL;
    mContext = new JObjectHolder(context);
    mCurrentPosition = -1;
    mCurrentSeekMode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC;
    mSeekPosition = -1;
    mSeekMode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC;
    mCurrentState = MEDIA_PLAYER2_IDLE;
    mTransitionToNext = false;
    mLoop = false;
    mVolume = 1.0;
    mVideoWidth = mVideoHeight = 0;
    mSendLevel = 0;

    mPid = AIBinder_getCallingPid();
    mUid = AIBinder_getCallingUid();

    mAudioOutput = new MediaPlayer2AudioOutput(sessionId, mUid, mPid, NULL /*attributes*/);
}

MediaPlayer2::~MediaPlayer2() {
    ALOGV("destructor");
    disconnect();
    removePlayer(this);
}

bool MediaPlayer2::init() {
    // TODO: after merge with NuPlayer2Driver, MediaPlayer2 will have its own
    // looper for notification.
    return true;
}

void MediaPlayer2::disconnect() {
    ALOGV("disconnect");
    sp<MediaPlayer2Interface> p;
    {
        Mutex::Autolock _l(mLock);
        p = mPlayer;
        mPlayer.clear();
    }

    if (p != 0) {
        p->setListener(NULL);
        p->reset();
    }

    {
        Mutex::Autolock _l(mLock);
        disconnectNativeWindow_l();
    }
}

void MediaPlayer2::clear_l() {
    mCurrentPosition = -1;
    mCurrentSeekMode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC;
    mSeekPosition = -1;
    mSeekMode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC;
    mVideoWidth = mVideoHeight = 0;
}

status_t MediaPlayer2::setListener(const sp<MediaPlayer2Listener>& listener) {
    ALOGV("setListener");
    Mutex::Autolock _l(mLock);
    mListener = listener;
    return NO_ERROR;
}

status_t MediaPlayer2::getSrcId(int64_t *srcId) {
    if (srcId == NULL) {
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mLock);
    *srcId = mSrcId;
    return OK;
}

status_t MediaPlayer2::setDataSource(const sp<DataSourceDesc> &dsd) {
    if (dsd == NULL) {
        return BAD_VALUE;
    }
    // Microsecond is used in NuPlayer2.
    if (dsd->mStartPositionMs > DataSourceDesc::kMaxTimeMs) {
        dsd->mStartPositionMs = DataSourceDesc::kMaxTimeMs;
        ALOGW("setDataSource, start poistion clamped to %lld ms", (long long)dsd->mStartPositionMs);
    }
    if (dsd->mEndPositionMs > DataSourceDesc::kMaxTimeMs) {
        dsd->mEndPositionMs = DataSourceDesc::kMaxTimeMs;
        ALOGW("setDataSource, end poistion clamped to %lld ms", (long long)dsd->mStartPositionMs);
    }
    ALOGV("setDataSource type(%d), srcId(%lld)", dsd->mType, (long long)dsd->mId);

    sp<MediaPlayer2Interface> oldPlayer;

    {
        Mutex::Autolock _l(mLock);
        if (!((mCurrentState & MEDIA_PLAYER2_IDLE)
              || mCurrentState == MEDIA_PLAYER2_STATE_ERROR)) {
            ALOGE("setDataSource called in wrong state %d", mCurrentState);
            return INVALID_OPERATION;
        }

        sp<MediaPlayer2Interface> player = new NuPlayer2Driver(mPid, mUid, mContext);
        status_t err = player->initCheck();
        if (err != NO_ERROR) {
            ALOGE("Failed to create player object, initCheck failed(%d)", err);
            return err;
        }

        clear_l();

        player->setListener(new proxyListener(this));
        player->setAudioSink(mAudioOutput);

        err = player->setDataSource(dsd);
        if (err != OK) {
            ALOGE("setDataSource error: %d", err);
            return err;
        }

        sp<MediaPlayer2Interface> oldPlayer = mPlayer;
        mPlayer = player;
        mSrcId = dsd->mId;
        mCurrentState = MEDIA_PLAYER2_INITIALIZED;
    }

    if (oldPlayer != NULL) {
        oldPlayer->setListener(NULL);
        oldPlayer->reset();
    }

    return OK;
}

status_t MediaPlayer2::prepareNextDataSource(const sp<DataSourceDesc> &dsd) {
    if (dsd == NULL) {
        return BAD_VALUE;
    }
    ALOGV("prepareNextDataSource type(%d), srcId(%lld)", dsd->mType, (long long)dsd->mId);

    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        ALOGE("prepareNextDataSource failed: state %X, mPlayer(%p)", mCurrentState, mPlayer.get());
        return INVALID_OPERATION;
    }
    return mPlayer->prepareNextDataSource(dsd);
}

status_t MediaPlayer2::playNextDataSource(int64_t srcId) {
    ALOGV("playNextDataSource srcId(%lld)", (long long)srcId);

    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        ALOGE("playNextDataSource failed: state %X, mPlayer(%p)", mCurrentState, mPlayer.get());
        return INVALID_OPERATION;
    }
    mSrcId = srcId;
    mTransitionToNext = true;
    return mPlayer->playNextDataSource(srcId);
}

status_t MediaPlayer2::invoke(const PlayerMessage &request, PlayerMessage *reply) {
    Mutex::Autolock _l(mLock);
    const bool hasBeenInitialized =
            (mCurrentState != MEDIA_PLAYER2_STATE_ERROR) &&
            ((mCurrentState & MEDIA_PLAYER2_IDLE) != MEDIA_PLAYER2_IDLE);
    if ((mPlayer == NULL) || !hasBeenInitialized) {
        ALOGE("invoke() failed: wrong state %X, mPlayer(%p)", mCurrentState, mPlayer.get());
        return INVALID_OPERATION;
    }
    return mPlayer->invoke(request, reply);
}

void MediaPlayer2::disconnectNativeWindow_l() {
    if (mConnectedWindow != NULL && mConnectedWindow->getANativeWindow() != NULL) {
        status_t err = native_window_api_disconnect(
                mConnectedWindow->getANativeWindow(), NATIVE_WINDOW_API_MEDIA);

        if (err != OK) {
            ALOGW("nativeWindowDisconnect returned an error: %s (%d)",
                  strerror(-err), err);
        }
    }
    mConnectedWindow.clear();
}

status_t MediaPlayer2::setVideoSurfaceTexture(const sp<ANativeWindowWrapper>& nww) {
    ANativeWindow *anw = (nww == NULL ? NULL : nww->getANativeWindow());
    ALOGV("setVideoSurfaceTexture(%p)", anw);
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return NO_INIT;
    }

    if (anw != NULL) {
        if (mConnectedWindow != NULL
            && mConnectedWindow->getANativeWindow() == anw) {
            return OK;
        }
        status_t err = native_window_api_connect(anw, NATIVE_WINDOW_API_MEDIA);

        if (err != OK) {
            ALOGE("setVideoSurfaceTexture failed: %d", err);
            // Note that we must do the reset before disconnecting from the ANW.
            // Otherwise queue/dequeue calls could be made on the disconnected
            // ANW, which may result in errors.
            mPlayer->reset();
            disconnectNativeWindow_l();
            return err;
        }
    }

    // Note that we must set the player's new GraphicBufferProducer before
    // disconnecting the old one.  Otherwise queue/dequeue calls could be made
    // on the disconnected ANW, which may result in errors.
    status_t err = mPlayer->setVideoSurfaceTexture(nww);

    disconnectNativeWindow_l();

    if (err == OK) {
        mConnectedWindow = nww;
        mLock.unlock();
    } else if (anw != NULL) {
        mLock.unlock();
        status_t err = native_window_api_disconnect(anw, NATIVE_WINDOW_API_MEDIA);

        if (err != OK) {
            ALOGW("nativeWindowDisconnect returned an error: %s (%d)",
                  strerror(-err), err);
        }
    }

    return err;
}

status_t MediaPlayer2::getBufferingSettings(BufferingSettings* buffering /* nonnull */) {
    ALOGV("getBufferingSettings");

    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return NO_INIT;
    }

    status_t ret = mPlayer->getBufferingSettings(buffering);
    if (ret == NO_ERROR) {
        ALOGV("getBufferingSettings{%s}", buffering->toString().string());
    } else {
        ALOGE("getBufferingSettings returned %d", ret);
    }
    return ret;
}

status_t MediaPlayer2::setBufferingSettings(const BufferingSettings& buffering) {
    ALOGV("setBufferingSettings{%s}", buffering.toString().string());

    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return NO_INIT;
    }
    return mPlayer->setBufferingSettings(buffering);
}

status_t MediaPlayer2::setAudioAttributes_l(const jobject attributes) {
    if (mAudioOutput != NULL) {
        mAudioOutput->setAudioAttributes(attributes);
    }
    return NO_ERROR;
}

status_t MediaPlayer2::prepareAsync() {
    ALOGV("prepareAsync");
    Mutex::Autolock _l(mLock);
    if ((mPlayer != 0) && (mCurrentState & MEDIA_PLAYER2_INITIALIZED)) {
        if (mAudioAttributes != NULL) {
            status_t err = setAudioAttributes_l(mAudioAttributes->getJObject());
            if (err != OK) {
                return err;
            }
        }
        mCurrentState = MEDIA_PLAYER2_PREPARING;
        return mPlayer->prepareAsync();
    }
    ALOGE("prepareAsync called in state %d, mPlayer(%p)", mCurrentState, mPlayer.get());
    return INVALID_OPERATION;
}

status_t MediaPlayer2::start() {
    ALOGV("start");

    status_t ret = NO_ERROR;
    Mutex::Autolock _l(mLock);

    mLockThreadId = getThreadId();

    if (mCurrentState & MEDIA_PLAYER2_STARTED) {
        ret = NO_ERROR;
    } else if ( (mPlayer != 0) && ( mCurrentState & ( MEDIA_PLAYER2_PREPARED |
                    MEDIA_PLAYER2_PLAYBACK_COMPLETE | MEDIA_PLAYER2_PAUSED ) ) ) {
        mPlayer->setLooping(mLoop);

        if (mAudioOutput != 0) {
            mAudioOutput->setVolume(mVolume);
        }

        if (mAudioOutput != 0) {
            mAudioOutput->setAuxEffectSendLevel(mSendLevel);
        }
        mCurrentState = MEDIA_PLAYER2_STARTED;
        ret = mPlayer->start();
        if (ret != NO_ERROR) {
            mCurrentState = MEDIA_PLAYER2_STATE_ERROR;
        } else {
            if (mCurrentState == MEDIA_PLAYER2_PLAYBACK_COMPLETE) {
                ALOGV("playback completed immediately following start()");
            }
        }
    } else {
        ALOGE("start called in state %d, mPlayer(%p)", mCurrentState, mPlayer.get());
        ret = INVALID_OPERATION;
    }

    mLockThreadId = 0;

    return ret;
}

status_t MediaPlayer2::pause() {
    ALOGV("pause");
    Mutex::Autolock _l(mLock);
    if (mCurrentState & (MEDIA_PLAYER2_PAUSED|MEDIA_PLAYER2_PLAYBACK_COMPLETE))
        return NO_ERROR;
    if ((mPlayer != 0) && (mCurrentState & (MEDIA_PLAYER2_STARTED | MEDIA_PLAYER2_PREPARED))) {
        status_t ret = mPlayer->pause();
        if (ret != NO_ERROR) {
            mCurrentState = MEDIA_PLAYER2_STATE_ERROR;
        } else {
            mCurrentState = MEDIA_PLAYER2_PAUSED;
            mTransitionToNext = false;
        }
        return ret;
    }
    ALOGE("pause called in state %d, mPlayer(%p)", mCurrentState, mPlayer.get());
    return INVALID_OPERATION;
}

bool MediaPlayer2::isPlaying() {
    Mutex::Autolock _l(mLock);
    if (mPlayer != 0) {
        bool temp = mPlayer->isPlaying();
        ALOGV("isPlaying: %d", temp);
        if ((mCurrentState & MEDIA_PLAYER2_STARTED) && ! temp) {
            ALOGE("internal/external state mismatch corrected");
            mCurrentState = MEDIA_PLAYER2_PAUSED;
        } else if ((mCurrentState & MEDIA_PLAYER2_PAUSED) && temp) {
            ALOGE("internal/external state mismatch corrected");
            mCurrentState = MEDIA_PLAYER2_STARTED;
        }
        return temp;
    }
    ALOGV("isPlaying: no active player");
    return false;
}

mediaplayer2_states MediaPlayer2::getState() {
    Mutex::Autolock _l(mLock);
    if (mCurrentState & MEDIA_PLAYER2_STATE_ERROR) {
        return MEDIAPLAYER2_STATE_ERROR;
    }
    if (mPlayer == 0
        || (mCurrentState &
            (MEDIA_PLAYER2_IDLE | MEDIA_PLAYER2_INITIALIZED | MEDIA_PLAYER2_PREPARING))) {
        return MEDIAPLAYER2_STATE_IDLE;
    }
    if (mCurrentState & MEDIA_PLAYER2_STARTED) {
        return MEDIAPLAYER2_STATE_PLAYING;
    }
    if (mCurrentState & (MEDIA_PLAYER2_PAUSED | MEDIA_PLAYER2_PLAYBACK_COMPLETE)) {
        return MEDIAPLAYER2_STATE_PAUSED;
    }
    // now only mCurrentState & MEDIA_PLAYER2_PREPARED is true
    return MEDIAPLAYER2_STATE_PREPARED;
}

status_t MediaPlayer2::setPlaybackSettings(const AudioPlaybackRate& rate) {
    ALOGV("setPlaybackSettings: %f %f %d %d",
            rate.mSpeed, rate.mPitch, rate.mFallbackMode, rate.mStretchMode);
    // Negative speed and pitch does not make sense. Further validation will
    // be done by the respective mediaplayers.
    if (rate.mSpeed <= 0.f || rate.mPitch < 0.f) {
        return BAD_VALUE;
    }
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return INVALID_OPERATION;
    }

    status_t err = mPlayer->setPlaybackSettings(rate);
    return err;
}

status_t MediaPlayer2::getPlaybackSettings(AudioPlaybackRate* rate /* nonnull */) {
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return INVALID_OPERATION;
    }
    status_t ret = mPlayer->getPlaybackSettings(rate);
    if (ret == NO_ERROR) {
        ALOGV("getPlaybackSettings(%f, %f, %d, %d)",
                rate->mSpeed, rate->mPitch, rate->mFallbackMode, rate->mStretchMode);
    } else {
        ALOGV("getPlaybackSettings returned %d", ret);
    }
    return ret;
}

status_t MediaPlayer2::setSyncSettings(const AVSyncSettings& sync, float videoFpsHint) {
    ALOGV("setSyncSettings: %u %u %f %f",
            sync.mSource, sync.mAudioAdjustMode, sync.mTolerance, videoFpsHint);
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return INVALID_OPERATION;
    return mPlayer->setSyncSettings(sync, videoFpsHint);
}

status_t MediaPlayer2::getSyncSettings(
        AVSyncSettings* sync /* nonnull */, float* videoFps /* nonnull */) {
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return INVALID_OPERATION;
    }
    status_t ret = mPlayer->getSyncSettings(sync, videoFps);
    if (ret == NO_ERROR) {
        ALOGV("getSyncSettings(%u, %u, %f, %f)",
                sync->mSource, sync->mAudioAdjustMode, sync->mTolerance, *videoFps);
    } else {
        ALOGV("getSyncSettings returned %d", ret);
    }
    return ret;

}

status_t MediaPlayer2::getVideoWidth(int *w) {
    ALOGV("getVideoWidth");
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return INVALID_OPERATION;
    }
    *w = mVideoWidth;
    return NO_ERROR;
}

status_t MediaPlayer2::getVideoHeight(int *h) {
    ALOGV("getVideoHeight");
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return INVALID_OPERATION;
    }
    *h = mVideoHeight;
    return NO_ERROR;
}

status_t MediaPlayer2::getCurrentPosition(int64_t *msec) {
    ALOGV("getCurrentPosition");
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return INVALID_OPERATION;
    }
    if (mCurrentPosition >= 0) {
        ALOGV("Using cached seek position: %lld", (long long)mCurrentPosition);
        *msec = mCurrentPosition;
        return NO_ERROR;
    }
    status_t ret = mPlayer->getCurrentPosition(msec);
    if (ret == NO_ERROR) {
        ALOGV("getCurrentPosition = %lld", (long long)*msec);
    } else {
        ALOGE("getCurrentPosition returned %d", ret);
    }
    return ret;
}

status_t MediaPlayer2::getDuration(int64_t srcId, int64_t *msec) {
    Mutex::Autolock _l(mLock);
    // TODO: cache duration for currentSrcId and nextSrcId, and return correct
    // value for nextSrcId.
    if (srcId != mSrcId) {
        *msec = -1;
        return OK;
    }

    ALOGV("getDuration_l");
    bool isValidState = (mCurrentState & (MEDIA_PLAYER2_PREPARED | MEDIA_PLAYER2_STARTED |
            MEDIA_PLAYER2_PAUSED | MEDIA_PLAYER2_PLAYBACK_COMPLETE));
    if (mPlayer == 0 || !isValidState) {
        ALOGE("Attempt to call getDuration in wrong state: mPlayer=%p, mCurrentState=%u",
                mPlayer.get(), mCurrentState);
        return INVALID_OPERATION;
    }
    int64_t durationMs;
    status_t ret = mPlayer->getDuration(&durationMs);

    if (ret == NO_ERROR) {
        ALOGV("getDuration = %lld", (long long)durationMs);
    } else {
        ALOGE("getDuration returned %d", ret);
        // Do not enter error state just because no duration was available.
        durationMs = -1;
    }

    if (msec) {
        *msec = durationMs;
    }
    return OK;
}

status_t MediaPlayer2::seekTo_l(int64_t msec, MediaPlayer2SeekMode mode) {
    ALOGV("seekTo (%lld, %d)", (long long)msec, mode);
    if ((mPlayer == 0) || !(mCurrentState & (MEDIA_PLAYER2_STARTED | MEDIA_PLAYER2_PREPARED |
            MEDIA_PLAYER2_PAUSED | MEDIA_PLAYER2_PLAYBACK_COMPLETE))) {
        ALOGE("Attempt to perform seekTo in wrong state: mPlayer=%p, mCurrentState=%u",
              mPlayer.get(), mCurrentState);
        return INVALID_OPERATION;
    }
    if (msec < 0) {
        ALOGW("Attempt to seek to invalid position: %lld", (long long)msec);
        msec = 0;
    }

    int64_t durationMs;
    status_t err = mPlayer->getDuration(&durationMs);

    if (err != OK) {
        ALOGW("Stream has no duration and is therefore not seekable.");
        return err;
    }

    if (msec > durationMs) {
        ALOGW("Attempt to seek to past end of file: request = %lld, durationMs = %lld",
              (long long)msec, (long long)durationMs);

        msec = durationMs;
    }

    // cache duration
    mCurrentPosition = msec;
    mCurrentSeekMode = mode;
    if (mSeekPosition < 0) {
        mSeekPosition = msec;
        mSeekMode = mode;
        return mPlayer->seekTo(msec, mode);
    }
    ALOGV("Seek in progress - queue up seekTo[%lld, %d]", (long long)msec, mode);
    return NO_ERROR;
}

status_t MediaPlayer2::seekTo(int64_t msec, MediaPlayer2SeekMode mode) {
    mLockThreadId = getThreadId();
    Mutex::Autolock _l(mLock);
    status_t result = seekTo_l(msec, mode);
    mLockThreadId = 0;

    return result;
}

status_t MediaPlayer2::notifyAt(int64_t mediaTimeUs) {
    Mutex::Autolock _l(mLock);
    if (mPlayer != 0) {
        return INVALID_OPERATION;
    }

    return mPlayer->notifyAt(mediaTimeUs);
}

status_t MediaPlayer2::reset_l() {
    mLoop = false;
    if (mCurrentState == MEDIA_PLAYER2_IDLE) {
        return NO_ERROR;
    }
    if (mPlayer != 0) {
        status_t ret = mPlayer->reset();
        if (ret != NO_ERROR) {
            ALOGE("reset() failed with return code (%d)", ret);
            mCurrentState = MEDIA_PLAYER2_STATE_ERROR;
        } else {
            mPlayer->setListener(NULL);
            mCurrentState = MEDIA_PLAYER2_IDLE;
            mTransitionToNext = false;
        }
        // setDataSource has to be called again to create a
        // new mediaplayer.
        mPlayer = 0;
        return ret;
    }
    clear_l();
    return NO_ERROR;
}

status_t MediaPlayer2::reset() {
    ALOGV("reset");
    mLockThreadId = getThreadId();
    Mutex::Autolock _l(mLock);
    status_t result = reset_l();
    mLockThreadId = 0;

    return result;
}

status_t MediaPlayer2::setAudioStreamType(audio_stream_type_t type) {
    ALOGV("MediaPlayer2::setAudioStreamType");
    Mutex::Autolock _l(mLock);
    if (mStreamType == type) return NO_ERROR;
    if (mCurrentState & ( MEDIA_PLAYER2_PREPARED | MEDIA_PLAYER2_STARTED |
                MEDIA_PLAYER2_PAUSED | MEDIA_PLAYER2_PLAYBACK_COMPLETE ) ) {
        // Can't change the stream type after prepare
        ALOGE("setAudioStream called in state %d", mCurrentState);
        return INVALID_OPERATION;
    }
    // cache
    mStreamType = type;
    return OK;
}

status_t MediaPlayer2::getAudioStreamType(audio_stream_type_t *type) {
    ALOGV("getAudioStreamType");
    Mutex::Autolock _l(mLock);
    *type = mStreamType;
    return OK;
}

status_t MediaPlayer2::setLooping(int loop) {
    ALOGV("MediaPlayer2::setLooping");
    Mutex::Autolock _l(mLock);
    mLoop = (loop != 0);
    if (mPlayer != 0) {
        return mPlayer->setLooping(loop);
    }
    return OK;
}

bool MediaPlayer2::isLooping() {
    ALOGV("isLooping");
    Mutex::Autolock _l(mLock);
    if (mPlayer != 0) {
        return mLoop;
    }
    ALOGV("isLooping: no active player");
    return false;
}

status_t MediaPlayer2::setVolume(float volume) {
    ALOGV("MediaPlayer2::setVolume(%f)", volume);
    Mutex::Autolock _l(mLock);
    mVolume = volume;
    if (mAudioOutput != 0) {
        mAudioOutput->setVolume(volume);
    }
    return OK;
}

status_t MediaPlayer2::setAudioSessionId(int32_t sessionId) {
    ALOGV("MediaPlayer2::setAudioSessionId(%d)", sessionId);
    Mutex::Autolock _l(mLock);
    if (!(mCurrentState & MEDIA_PLAYER2_IDLE)) {
        ALOGE("setAudioSessionId called in state %d", mCurrentState);
        return INVALID_OPERATION;
    }
    if (sessionId < 0) {
        return BAD_VALUE;
    }
    if (mAudioOutput != NULL && sessionId != mAudioOutput->getSessionId()) {
        mAudioOutput->setSessionId(sessionId);
    }
    return NO_ERROR;
}

int32_t MediaPlayer2::getAudioSessionId() {
    Mutex::Autolock _l(mLock);
    if (mAudioOutput != NULL) {
        return mAudioOutput->getSessionId();
    }
    return 0;
}

status_t MediaPlayer2::setAuxEffectSendLevel(float level) {
    ALOGV("MediaPlayer2::setAuxEffectSendLevel(%f)", level);
    Mutex::Autolock _l(mLock);
    mSendLevel = level;
    if (mAudioOutput != 0) {
        return mAudioOutput->setAuxEffectSendLevel(level);
    }
    return OK;
}

status_t MediaPlayer2::attachAuxEffect(int effectId) {
    ALOGV("MediaPlayer2::attachAuxEffect(%d)", effectId);
    Mutex::Autolock _l(mLock);
    if (mAudioOutput == 0 ||
        (mCurrentState & MEDIA_PLAYER2_IDLE) ||
        (mCurrentState == MEDIA_PLAYER2_STATE_ERROR )) {
        ALOGE("attachAuxEffect called in state %d, mPlayer(%p)", mCurrentState, mPlayer.get());
        return INVALID_OPERATION;
    }

    return mAudioOutput->attachAuxEffect(effectId);
}

// always call with lock held
status_t MediaPlayer2::checkState_l() {
    if (mCurrentState & ( MEDIA_PLAYER2_PREPARED | MEDIA_PLAYER2_STARTED |
            MEDIA_PLAYER2_PAUSED | MEDIA_PLAYER2_PLAYBACK_COMPLETE) ) {
        // Can't change the audio attributes after prepare
        ALOGE("trying to set audio attributes called in state %d", mCurrentState);
        return INVALID_OPERATION;
    }
    return OK;
}

status_t MediaPlayer2::setAudioAttributes(const jobject attributes) {
    ALOGV("MediaPlayer2::setAudioAttributes");
    status_t status = INVALID_OPERATION;
    Mutex::Autolock _l(mLock);
    if (checkState_l() != OK) {
        return status;
    }
    mAudioAttributes = new JObjectHolder(attributes);
    status = setAudioAttributes_l(attributes);
    return status;
}

jobject MediaPlayer2::getAudioAttributes() {
    ALOGV("MediaPlayer2::getAudioAttributes)");
    Mutex::Autolock _l(mLock);
    return mAudioAttributes != NULL ? mAudioAttributes->getJObject() : NULL;
}

status_t MediaPlayer2::getParameter(int key, Parcel *reply) {
    ALOGV("MediaPlayer2::getParameter(%d)", key);
    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        ALOGV("getParameter: no active player");
        return INVALID_OPERATION;
    }

    status_t status =  mPlayer->getParameter(key, reply);
    if (status != OK) {
        ALOGD("getParameter returns %d", status);
    }
    return status;
}

// for mediametrics
status_t MediaPlayer2::getMetrics(char **buffer, size_t *length) {
    ALOGD("MediaPlayer2::getMetrics()");
    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        ALOGV("getMetrics: no active player");
        return INVALID_OPERATION;
    }

    status_t status =  mPlayer->getMetrics(buffer, length);
    if (status != OK) {
        ALOGD("getMetrics returns %d", status);
    }
    return status;
}

void MediaPlayer2::notify(int64_t srcId, int msg, int ext1, int ext2, const PlayerMessage *obj) {
    ALOGV("message received srcId=%lld, msg=%d, ext1=%d, ext2=%d",
          (long long)srcId, msg, ext1, ext2);

    bool send = true;
    bool locked = false;

    // TODO: In the future, we might be on the same thread if the app is
    // running in the same process as the media server. In that case,
    // this will deadlock.
    //
    // The threadId hack below works around this for the care of prepare,
    // seekTo, start, and reset within the same process.
    // FIXME: Remember, this is a hack, it's not even a hack that is applied
    // consistently for all use-cases, this needs to be revisited.
    if (mLockThreadId != getThreadId()) {
        mLock.lock();
        locked = true;
    }

    // Allows calls from JNI in idle state to notify errors
    if (!(msg == MEDIA2_ERROR && mCurrentState == MEDIA_PLAYER2_IDLE) && mPlayer == 0) {
        ALOGV("notify(%lld, %d, %d, %d) callback on disconnected mediaplayer",
              (long long)srcId, msg, ext1, ext2);
        if (locked) mLock.unlock();   // release the lock when done.
        return;
    }

    switch (msg) {
    case MEDIA2_NOP: // interface test message
        break;
    case MEDIA2_PREPARED:
        ALOGV("MediaPlayer2::notify() prepared, srcId=%lld", (long long)srcId);
        if (srcId == mSrcId) {
            mCurrentState = MEDIA_PLAYER2_PREPARED;
        }
        break;
    case MEDIA2_DRM_INFO:
        ALOGV("MediaPlayer2::notify() MEDIA2_DRM_INFO(%lld, %d, %d, %d, %p)",
              (long long)srcId, msg, ext1, ext2, obj);
        break;
    case MEDIA2_PLAYBACK_COMPLETE:
        ALOGV("playback complete");
        if (mCurrentState == MEDIA_PLAYER2_IDLE) {
            ALOGE("playback complete in idle state");
        }
        if (!mLoop && srcId == mSrcId) {
            mCurrentState = MEDIA_PLAYER2_PLAYBACK_COMPLETE;
        }
        break;
    case MEDIA2_ERROR:
        // Always log errors.
        // ext1: Media framework error code.
        // ext2: Implementation dependant error code.
        ALOGE("error (%d, %d)", ext1, ext2);
        mCurrentState = MEDIA_PLAYER2_STATE_ERROR;
        break;
    case MEDIA2_INFO:
        // ext1: Media framework error code.
        // ext2: Implementation dependant error code.
        if (ext1 != MEDIA2_INFO_VIDEO_TRACK_LAGGING) {
            ALOGW("info/warning (%d, %d)", ext1, ext2);

            if (ext1 == MEDIA2_INFO_DATA_SOURCE_START && srcId == mSrcId && mTransitionToNext) {
                mCurrentState = MEDIA_PLAYER2_STARTED;
                mTransitionToNext = false;
            }
        }
        break;
    case MEDIA2_SEEK_COMPLETE:
        ALOGV("Received seek complete");
        if (mSeekPosition != mCurrentPosition || (mSeekMode != mCurrentSeekMode)) {
            ALOGV("Executing queued seekTo(%lld, %d)",
                  (long long)mCurrentPosition, mCurrentSeekMode);
            mSeekPosition = -1;
            mSeekMode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC;
            seekTo_l(mCurrentPosition, mCurrentSeekMode);
        }
        else {
            ALOGV("All seeks complete - return to regularly scheduled program");
            mCurrentPosition = mSeekPosition = -1;
            mCurrentSeekMode = mSeekMode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC;
        }
        break;
    case MEDIA2_BUFFERING_UPDATE:
        ALOGV("buffering %d", ext1);
        break;
    case MEDIA2_SET_VIDEO_SIZE:
        ALOGV("New video size %d x %d", ext1, ext2);
        mVideoWidth = ext1;
        mVideoHeight = ext2;
        break;
    case MEDIA2_NOTIFY_TIME:
        ALOGV("Received notify time message");
        break;
    case MEDIA2_TIMED_TEXT:
        ALOGV("Received timed text message");
        break;
    case MEDIA2_SUBTITLE_DATA:
        ALOGV("Received subtitle data message");
        break;
    case MEDIA2_META_DATA:
        ALOGV("Received timed metadata message");
        break;
    default:
        ALOGV("unrecognized message: (%d, %d, %d)", msg, ext1, ext2);
        break;
    }

    sp<MediaPlayer2Listener> listener = mListener;
    if (locked) mLock.unlock();

    // this prevents re-entrant calls into client code
    if ((listener != 0) && send) {
        Mutex::Autolock _l(mNotifyLock);
        ALOGV("callback application");
        listener->notify(srcId, msg, ext1, ext2, obj);
        ALOGV("back from callback");
    }
}

// Modular DRM
status_t MediaPlayer2::prepareDrm(
        int64_t srcId, const uint8_t uuid[16], const Vector<uint8_t>& drmSessionId) {
    // TODO change to ALOGV
    ALOGD("prepareDrm: uuid: %p  drmSessionId: %p(%zu)", uuid,
            drmSessionId.array(), drmSessionId.size());
    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        return NO_INIT;
    }

    // Only allowed it in player's preparing/prepared state.
    // We get here only if MEDIA_DRM_INFO has already arrived (e.g., prepare is half-way through or
    // completed) so the state change to "prepared" might not have happened yet (e.g., buffering).
    // Still, we can allow prepareDrm for the use case of being called in OnDrmInfoListener.
    if (!(mCurrentState & (MEDIA_PLAYER2_PREPARING | MEDIA_PLAYER2_PREPARED))) {
        ALOGW("prepareDrm(%lld) called in non-prepare state(%d)", (long long)srcId, mCurrentState);
        if (srcId == mSrcId) {
            return INVALID_OPERATION;
        }
    }

    if (drmSessionId.isEmpty()) {
        ALOGE("prepareDrm: Unexpected. Can't proceed with crypto. Empty drmSessionId.");
        return INVALID_OPERATION;
    }

    // Passing down to mediaserver mainly for creating the crypto
    status_t status = mPlayer->prepareDrm(srcId, uuid, drmSessionId);
    ALOGE_IF(status != OK, "prepareDrm: Failed at mediaserver with ret: %d", status);

    // TODO change to ALOGV
    ALOGD("prepareDrm: mediaserver::prepareDrm ret=%d", status);

    return status;
}

status_t MediaPlayer2::releaseDrm(int64_t srcId) {
    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        return NO_INIT;
    }

    // Not allowing releaseDrm in an active/resumable state
    if (mCurrentState & (MEDIA_PLAYER2_STARTED |
                         MEDIA_PLAYER2_PAUSED |
                         MEDIA_PLAYER2_PLAYBACK_COMPLETE |
                         MEDIA_PLAYER2_STATE_ERROR)) {
        ALOGE("releaseDrm Unexpected state %d. Can only be called in stopped/idle.", mCurrentState);
        return INVALID_OPERATION;
    }

    status_t status = mPlayer->releaseDrm(srcId);
    // TODO change to ALOGV
    ALOGD("releaseDrm: mediaserver::releaseDrm ret: %d", status);
    if (status != OK) {
        ALOGE("releaseDrm: Failed at mediaserver with ret: %d", status);
        // Overriding to OK so the client proceed with its own cleanup
        // Client can't do more cleanup. mediaserver release its crypto at end of session anyway.
        status = OK;
    }

    return status;
}

status_t MediaPlayer2::setPreferredDevice(jobject device) {
    Mutex::Autolock _l(mLock);
    if (mAudioOutput == NULL) {
        ALOGV("setPreferredDevice: audio sink not init");
        return NO_INIT;
    }
    return mAudioOutput->setPreferredDevice(device);
}

jobject MediaPlayer2::getRoutedDevice() {
    Mutex::Autolock _l(mLock);
    if (mAudioOutput == NULL) {
        ALOGV("getRoutedDevice: audio sink not init");
        return nullptr;
    }
    return mAudioOutput->getRoutedDevice();
}

status_t MediaPlayer2::addAudioDeviceCallback(jobject routingDelegate) {
    Mutex::Autolock _l(mLock);
    if (mAudioOutput == NULL) {
        ALOGV("addAudioDeviceCallback: player not init");
        return NO_INIT;
    }
    return mAudioOutput->addAudioDeviceCallback(routingDelegate);
}

status_t MediaPlayer2::removeAudioDeviceCallback(jobject listener) {
    Mutex::Autolock _l(mLock);
    if (mAudioOutput == NULL) {
        ALOGV("addAudioDeviceCallback: player not init");
        return NO_INIT;
    }
    return mAudioOutput->removeAudioDeviceCallback(listener);
}

status_t MediaPlayer2::dump(int fd, const Vector<String16>& args) {
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append(" MediaPlayer2\n");
    snprintf(buffer, 255, "  pid(%d), looping(%s)\n", mPid, mLoop?"true": "false");
    result.append(buffer);

    sp<MediaPlayer2Interface> player;
    sp<MediaPlayer2AudioOutput> audioOutput;
    bool locked = false;
    for (int i = 0; i < kDumpLockRetries; ++i) {
        if (mLock.tryLock() == NO_ERROR) {
            locked = true;
            break;
        }
        usleep(kDumpLockSleepUs);
    }

    if (locked) {
        player = mPlayer;
        audioOutput = mAudioOutput;
        mLock.unlock();
    } else {
        result.append("  lock is taken, no dump from player and audio output\n");
    }
    write(fd, result.string(), result.size());

    if (player != NULL) {
        player->dump(fd, args);
    }
    if (audioOutput != 0) {
        audioOutput->dump(fd, args);
    }
    write(fd, "\n", 1);
    return NO_ERROR;
}

} // namespace android
