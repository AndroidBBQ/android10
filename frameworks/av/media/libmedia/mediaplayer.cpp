/*
**
** Copyright 2006, The Android Open Source Project
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
#define LOG_TAG "MediaPlayerNative"

#include <fcntl.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/Log.h>

#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include <gui/Surface.h>

#include <media/mediaplayer.h>
#include <media/AudioResamplerPublic.h>
#include <media/AudioSystem.h>
#include <media/AVSyncSettings.h>
#include <media/IDataSource.h>
#include <media/MediaAnalyticsItem.h>

#include <binder/MemoryBase.h>

#include <utils/KeyedVector.h>
#include <utils/String8.h>

#include <system/audio.h>
#include <system/window.h>

namespace android {

using media::VolumeShaper;

MediaPlayer::MediaPlayer()
{
    ALOGV("constructor");
    mListener = NULL;
    mCookie = NULL;
    mStreamType = AUDIO_STREAM_MUSIC;
    mAudioAttributesParcel = NULL;
    mCurrentPosition = -1;
    mCurrentSeekMode = MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC;
    mSeekPosition = -1;
    mSeekMode = MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC;
    mCurrentState = MEDIA_PLAYER_IDLE;
    mPrepareSync = false;
    mPrepareStatus = NO_ERROR;
    mLoop = false;
    mLeftVolume = mRightVolume = 1.0;
    mVideoWidth = mVideoHeight = 0;
    mLockThreadId = 0;
    mAudioSessionId = (audio_session_t) AudioSystem::newAudioUniqueId(AUDIO_UNIQUE_ID_USE_SESSION);
    AudioSystem::acquireAudioSessionId(mAudioSessionId, -1);
    mSendLevel = 0;
    mRetransmitEndpointValid = false;
}

MediaPlayer::~MediaPlayer()
{
    ALOGV("destructor");
    if (mAudioAttributesParcel != NULL) {
        delete mAudioAttributesParcel;
        mAudioAttributesParcel = NULL;
    }
    AudioSystem::releaseAudioSessionId(mAudioSessionId, -1);
    disconnect();
    IPCThreadState::self()->flushCommands();
}

void MediaPlayer::disconnect()
{
    ALOGV("disconnect");
    sp<IMediaPlayer> p;
    {
        Mutex::Autolock _l(mLock);
        p = mPlayer;
        mPlayer.clear();
    }

    if (p != 0) {
        p->disconnect();
    }
}

// always call with lock held
void MediaPlayer::clear_l()
{
    mCurrentPosition = -1;
    mCurrentSeekMode = MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC;
    mSeekPosition = -1;
    mSeekMode = MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC;
    mVideoWidth = mVideoHeight = 0;
    mRetransmitEndpointValid = false;
}

status_t MediaPlayer::setListener(const sp<MediaPlayerListener>& listener)
{
    ALOGV("setListener");
    Mutex::Autolock _l(mLock);
    mListener = listener;
    return NO_ERROR;
}


status_t MediaPlayer::attachNewPlayer(const sp<IMediaPlayer>& player)
{
    status_t err = UNKNOWN_ERROR;
    sp<IMediaPlayer> p;
    { // scope for the lock
        Mutex::Autolock _l(mLock);

        if ( !( (mCurrentState & MEDIA_PLAYER_IDLE) ||
                (mCurrentState == MEDIA_PLAYER_STATE_ERROR ) ) ) {
            ALOGE("attachNewPlayer called in state %d", mCurrentState);
            return INVALID_OPERATION;
        }

        clear_l();
        p = mPlayer;
        mPlayer = player;
        if (player != 0) {
            mCurrentState = MEDIA_PLAYER_INITIALIZED;
            err = NO_ERROR;
        } else {
            ALOGE("Unable to create media player");
        }
    }

    if (p != 0) {
        p->disconnect();
    }

    return err;
}

status_t MediaPlayer::setDataSource(
        const sp<IMediaHTTPService> &httpService,
        const char *url, const KeyedVector<String8, String8> *headers)
{
    ALOGV("setDataSource(%s)", url);
    status_t err = BAD_VALUE;
    if (url != NULL) {
        const sp<IMediaPlayerService> service(getMediaPlayerService());
        if (service != 0) {
            sp<IMediaPlayer> player(service->create(this, mAudioSessionId));
            if ((NO_ERROR != doSetRetransmitEndpoint(player)) ||
                (NO_ERROR != player->setDataSource(httpService, url, headers))) {
                player.clear();
            }
            err = attachNewPlayer(player);
        }
    }
    return err;
}

status_t MediaPlayer::setDataSource(int fd, int64_t offset, int64_t length)
{
    ALOGV("setDataSource(%d, %" PRId64 ", %" PRId64 ")", fd, offset, length);
    status_t err = UNKNOWN_ERROR;
    const sp<IMediaPlayerService> service(getMediaPlayerService());
    if (service != 0) {
        sp<IMediaPlayer> player(service->create(this, mAudioSessionId));
        if ((NO_ERROR != doSetRetransmitEndpoint(player)) ||
            (NO_ERROR != player->setDataSource(fd, offset, length))) {
            player.clear();
        }
        err = attachNewPlayer(player);
    }
    return err;
}

status_t MediaPlayer::setDataSource(const sp<IDataSource> &source)
{
    ALOGV("setDataSource(IDataSource)");
    status_t err = UNKNOWN_ERROR;
    const sp<IMediaPlayerService> service(getMediaPlayerService());
    if (service != 0) {
        sp<IMediaPlayer> player(service->create(this, mAudioSessionId));
        if ((NO_ERROR != doSetRetransmitEndpoint(player)) ||
            (NO_ERROR != player->setDataSource(source))) {
            player.clear();
        }
        err = attachNewPlayer(player);
    }
    return err;
}

status_t MediaPlayer::invoke(const Parcel& request, Parcel *reply)
{
    Mutex::Autolock _l(mLock);
    const bool hasBeenInitialized =
            (mCurrentState != MEDIA_PLAYER_STATE_ERROR) &&
            ((mCurrentState & MEDIA_PLAYER_IDLE) != MEDIA_PLAYER_IDLE);
    if ((mPlayer != NULL) && hasBeenInitialized) {
        ALOGV("invoke %zu", request.dataSize());
        return  mPlayer->invoke(request, reply);
    }
    ALOGE("invoke failed: wrong state %X, mPlayer(%p)", mCurrentState, mPlayer.get());
    return INVALID_OPERATION;
}

status_t MediaPlayer::setMetadataFilter(const Parcel& filter)
{
    ALOGD("setMetadataFilter");
    Mutex::Autolock lock(mLock);
    if (mPlayer == NULL) {
        return NO_INIT;
    }
    return mPlayer->setMetadataFilter(filter);
}

status_t MediaPlayer::getMetadata(bool update_only, bool apply_filter, Parcel *metadata)
{
    ALOGD("getMetadata");
    Mutex::Autolock lock(mLock);
    if (mPlayer == NULL) {
        return NO_INIT;
    }
    return mPlayer->getMetadata(update_only, apply_filter, metadata);
}

status_t MediaPlayer::setVideoSurfaceTexture(
        const sp<IGraphicBufferProducer>& bufferProducer)
{
    ALOGV("setVideoSurfaceTexture");
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return NO_INIT;
    return mPlayer->setVideoSurfaceTexture(bufferProducer);
}

status_t MediaPlayer::getBufferingSettings(BufferingSettings* buffering /* nonnull */)
{
    ALOGV("getBufferingSettings");

    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return NO_INIT;
    }
    return mPlayer->getBufferingSettings(buffering);
}

status_t MediaPlayer::setBufferingSettings(const BufferingSettings& buffering)
{
    ALOGV("setBufferingSettings");

    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) {
        return NO_INIT;
    }
    return mPlayer->setBufferingSettings(buffering);
}

// must call with lock held
status_t MediaPlayer::prepareAsync_l()
{
    if ( (mPlayer != 0) && ( mCurrentState & (MEDIA_PLAYER_INITIALIZED | MEDIA_PLAYER_STOPPED) ) ) {
        if (mAudioAttributesParcel != NULL) {
            mPlayer->setParameter(KEY_PARAMETER_AUDIO_ATTRIBUTES, *mAudioAttributesParcel);
        } else {
            mPlayer->setAudioStreamType(mStreamType);
        }
        mCurrentState = MEDIA_PLAYER_PREPARING;
        return mPlayer->prepareAsync();
    }
    ALOGE("prepareAsync called in state %d, mPlayer(%p)", mCurrentState, mPlayer.get());
    return INVALID_OPERATION;
}

// TODO: In case of error, prepareAsync provides the caller with 2 error codes,
// one defined in the Android framework and one provided by the implementation
// that generated the error. The sync version of prepare returns only 1 error
// code.
status_t MediaPlayer::prepare()
{
    ALOGV("prepare");
    Mutex::Autolock _l(mLock);
    mLockThreadId = getThreadId();
    if (mPrepareSync) {
        mLockThreadId = 0;
        return -EALREADY;
    }
    mPrepareSync = true;
    status_t ret = prepareAsync_l();
    if (ret != NO_ERROR) {
        mLockThreadId = 0;
        return ret;
    }

    if (mPrepareSync) {
        mSignal.wait(mLock);  // wait for prepare done
        mPrepareSync = false;
    }
    ALOGV("prepare complete - status=%d", mPrepareStatus);
    mLockThreadId = 0;
    return mPrepareStatus;
}

status_t MediaPlayer::prepareAsync()
{
    ALOGV("prepareAsync");
    Mutex::Autolock _l(mLock);
    return prepareAsync_l();
}

status_t MediaPlayer::start()
{
    ALOGV("start");

    status_t ret = NO_ERROR;
    Mutex::Autolock _l(mLock);

    mLockThreadId = getThreadId();

    if (mCurrentState & MEDIA_PLAYER_STARTED) {
        ret = NO_ERROR;
    } else if ( (mPlayer != 0) && ( mCurrentState & ( MEDIA_PLAYER_PREPARED |
                    MEDIA_PLAYER_PLAYBACK_COMPLETE | MEDIA_PLAYER_PAUSED ) ) ) {
        mPlayer->setLooping(mLoop);
        mPlayer->setVolume(mLeftVolume, mRightVolume);
        mPlayer->setAuxEffectSendLevel(mSendLevel);
        mCurrentState = MEDIA_PLAYER_STARTED;
        ret = mPlayer->start();
        if (ret != NO_ERROR) {
            mCurrentState = MEDIA_PLAYER_STATE_ERROR;
        } else {
            if (mCurrentState == MEDIA_PLAYER_PLAYBACK_COMPLETE) {
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

status_t MediaPlayer::stop()
{
    ALOGV("stop");
    Mutex::Autolock _l(mLock);
    if (mCurrentState & MEDIA_PLAYER_STOPPED) return NO_ERROR;
    if ( (mPlayer != 0) && ( mCurrentState & ( MEDIA_PLAYER_STARTED | MEDIA_PLAYER_PREPARED |
                    MEDIA_PLAYER_PAUSED | MEDIA_PLAYER_PLAYBACK_COMPLETE ) ) ) {
        status_t ret = mPlayer->stop();
        if (ret != NO_ERROR) {
            mCurrentState = MEDIA_PLAYER_STATE_ERROR;
        } else {
            mCurrentState = MEDIA_PLAYER_STOPPED;
        }
        return ret;
    }
    ALOGE("stop called in state %d, mPlayer(%p)", mCurrentState, mPlayer.get());
    return INVALID_OPERATION;
}

status_t MediaPlayer::pause()
{
    ALOGV("pause");
    Mutex::Autolock _l(mLock);
    if (mCurrentState & (MEDIA_PLAYER_PAUSED|MEDIA_PLAYER_PLAYBACK_COMPLETE))
        return NO_ERROR;
    if ((mPlayer != 0) && (mCurrentState & MEDIA_PLAYER_STARTED)) {
        status_t ret = mPlayer->pause();
        if (ret != NO_ERROR) {
            mCurrentState = MEDIA_PLAYER_STATE_ERROR;
        } else {
            mCurrentState = MEDIA_PLAYER_PAUSED;
        }
        return ret;
    }
    ALOGE("pause called in state %d, mPlayer(%p)", mCurrentState, mPlayer.get());
    return INVALID_OPERATION;
}

bool MediaPlayer::isPlaying()
{
    Mutex::Autolock _l(mLock);
    if (mPlayer != 0) {
        bool temp = false;
        mPlayer->isPlaying(&temp);
        ALOGV("isPlaying: %d", temp);
        if ((mCurrentState & MEDIA_PLAYER_STARTED) && ! temp) {
            ALOGE("internal/external state mismatch corrected");
            mCurrentState = MEDIA_PLAYER_PAUSED;
        } else if ((mCurrentState & MEDIA_PLAYER_PAUSED) && temp) {
            ALOGE("internal/external state mismatch corrected");
            mCurrentState = MEDIA_PLAYER_STARTED;
        }
        return temp;
    }
    ALOGV("isPlaying: no active player");
    return false;
}

status_t MediaPlayer::setPlaybackSettings(const AudioPlaybackRate& rate)
{
    ALOGV("setPlaybackSettings: %f %f %d %d",
            rate.mSpeed, rate.mPitch, rate.mFallbackMode, rate.mStretchMode);
    // Negative speed and pitch does not make sense. Further validation will
    // be done by the respective mediaplayers.
    if (rate.mSpeed < 0.f || rate.mPitch < 0.f) {
        return BAD_VALUE;
    }
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0 || (mCurrentState & MEDIA_PLAYER_STOPPED)) {
        return INVALID_OPERATION;
    }

    if (rate.mSpeed != 0.f && !(mCurrentState & MEDIA_PLAYER_STARTED)
            && (mCurrentState & (MEDIA_PLAYER_PREPARED | MEDIA_PLAYER_PAUSED
                    | MEDIA_PLAYER_PLAYBACK_COMPLETE))) {
        mPlayer->setLooping(mLoop);
        mPlayer->setVolume(mLeftVolume, mRightVolume);
        mPlayer->setAuxEffectSendLevel(mSendLevel);
    }

    status_t err = mPlayer->setPlaybackSettings(rate);
    if (err == OK) {
        if (rate.mSpeed == 0.f && mCurrentState == MEDIA_PLAYER_STARTED) {
            mCurrentState = MEDIA_PLAYER_PAUSED;
        } else if (rate.mSpeed != 0.f
                && (mCurrentState & (MEDIA_PLAYER_PREPARED | MEDIA_PLAYER_PAUSED
                    | MEDIA_PLAYER_PLAYBACK_COMPLETE))) {
            mCurrentState = MEDIA_PLAYER_STARTED;
        }
    }
    return err;
}

status_t MediaPlayer::getPlaybackSettings(AudioPlaybackRate* rate /* nonnull */)
{
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return INVALID_OPERATION;
    return mPlayer->getPlaybackSettings(rate);
}

status_t MediaPlayer::setSyncSettings(const AVSyncSettings& sync, float videoFpsHint)
{
    ALOGV("setSyncSettings: %u %u %f %f",
            sync.mSource, sync.mAudioAdjustMode, sync.mTolerance, videoFpsHint);
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return INVALID_OPERATION;
    return mPlayer->setSyncSettings(sync, videoFpsHint);
}

status_t MediaPlayer::getSyncSettings(
        AVSyncSettings* sync /* nonnull */, float* videoFps /* nonnull */)
{
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return INVALID_OPERATION;
    return mPlayer->getSyncSettings(sync, videoFps);
}

status_t MediaPlayer::getVideoWidth(int *w)
{
    ALOGV("getVideoWidth");
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return INVALID_OPERATION;
    *w = mVideoWidth;
    return NO_ERROR;
}

status_t MediaPlayer::getVideoHeight(int *h)
{
    ALOGV("getVideoHeight");
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0) return INVALID_OPERATION;
    *h = mVideoHeight;
    return NO_ERROR;
}

status_t MediaPlayer::getCurrentPosition(int *msec)
{
    ALOGV("getCurrentPosition");
    Mutex::Autolock _l(mLock);
    if (mPlayer != 0) {
        if (mCurrentPosition >= 0) {
            ALOGV("Using cached seek position: %d", mCurrentPosition);
            *msec = mCurrentPosition;
            return NO_ERROR;
        }
        return mPlayer->getCurrentPosition(msec);
    }
    return INVALID_OPERATION;
}

status_t MediaPlayer::getDuration_l(int *msec)
{
    ALOGV("getDuration_l");
    bool isValidState = (mCurrentState & (MEDIA_PLAYER_PREPARED | MEDIA_PLAYER_STARTED |
            MEDIA_PLAYER_PAUSED | MEDIA_PLAYER_STOPPED | MEDIA_PLAYER_PLAYBACK_COMPLETE));
    if (mPlayer != 0 && isValidState) {
        int durationMs;
        status_t ret = mPlayer->getDuration(&durationMs);

        if (ret != OK) {
            // Do not enter error state just because no duration was available.
            durationMs = -1;
            ret = OK;
        }

        if (msec) {
            *msec = durationMs;
        }
        return ret;
    }
    ALOGE("Attempt to call getDuration in wrong state: mPlayer=%p, mCurrentState=%u",
            mPlayer.get(), mCurrentState);
    return INVALID_OPERATION;
}

status_t MediaPlayer::getDuration(int *msec)
{
    Mutex::Autolock _l(mLock);
    return getDuration_l(msec);
}

status_t MediaPlayer::seekTo_l(int msec, MediaPlayerSeekMode mode)
{
    ALOGV("seekTo (%d, %d)", msec, mode);
    if ((mPlayer != 0) && ( mCurrentState & ( MEDIA_PLAYER_STARTED | MEDIA_PLAYER_PREPARED |
            MEDIA_PLAYER_PAUSED |  MEDIA_PLAYER_PLAYBACK_COMPLETE) ) ) {
        if ( msec < 0 ) {
            ALOGW("Attempt to seek to invalid position: %d", msec);
            msec = 0;
        }

        int durationMs;
        status_t err = mPlayer->getDuration(&durationMs);

        if (err != OK) {
            ALOGW("Stream has no duration and is therefore not seekable.");
            return err;
        }

        if (msec > durationMs) {
            ALOGW("Attempt to seek to past end of file: request = %d, "
                  "durationMs = %d",
                  msec,
                  durationMs);

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
        else {
            ALOGV("Seek in progress - queue up seekTo[%d, %d]", msec, mode);
            return NO_ERROR;
        }
    }
    ALOGE("Attempt to perform seekTo in wrong state: mPlayer=%p, mCurrentState=%u", mPlayer.get(),
            mCurrentState);
    return INVALID_OPERATION;
}

status_t MediaPlayer::seekTo(int msec, MediaPlayerSeekMode mode)
{
    mLockThreadId = getThreadId();
    Mutex::Autolock _l(mLock);
    status_t result = seekTo_l(msec, mode);
    mLockThreadId = 0;

    return result;
}

status_t MediaPlayer::notifyAt(int64_t mediaTimeUs)
{
    Mutex::Autolock _l(mLock);
    if (mPlayer != 0) {
        return mPlayer->notifyAt(mediaTimeUs);
    }
    return INVALID_OPERATION;
}

status_t MediaPlayer::reset_l()
{
    mLoop = false;
    if (mCurrentState == MEDIA_PLAYER_IDLE) return NO_ERROR;
    mPrepareSync = false;
    if (mPlayer != 0) {
        status_t ret = mPlayer->reset();
        if (ret != NO_ERROR) {
            ALOGE("reset() failed with return code (%d)", ret);
            mCurrentState = MEDIA_PLAYER_STATE_ERROR;
        } else {
            mPlayer->disconnect();
            mCurrentState = MEDIA_PLAYER_IDLE;
        }
        // setDataSource has to be called again to create a
        // new mediaplayer.
        mPlayer = 0;
        return ret;
    }
    clear_l();
    return NO_ERROR;
}

status_t MediaPlayer::doSetRetransmitEndpoint(const sp<IMediaPlayer>& player) {
    Mutex::Autolock _l(mLock);

    if (player == NULL) {
        return UNKNOWN_ERROR;
    }

    if (mRetransmitEndpointValid) {
        return player->setRetransmitEndpoint(&mRetransmitEndpoint);
    }

    return OK;
}

status_t MediaPlayer::reset()
{
    ALOGV("reset");
    mLockThreadId = getThreadId();
    Mutex::Autolock _l(mLock);
    status_t result = reset_l();
    mLockThreadId = 0;

    return result;
}

status_t MediaPlayer::setAudioStreamType(audio_stream_type_t type)
{
    ALOGV("MediaPlayer::setAudioStreamType");
    Mutex::Autolock _l(mLock);
    if (mStreamType == type) return NO_ERROR;
    if (mCurrentState & ( MEDIA_PLAYER_PREPARED | MEDIA_PLAYER_STARTED |
                MEDIA_PLAYER_PAUSED | MEDIA_PLAYER_PLAYBACK_COMPLETE ) ) {
        // Can't change the stream type after prepare
        ALOGE("setAudioStream called in state %d", mCurrentState);
        return INVALID_OPERATION;
    }
    // cache
    mStreamType = type;
    return OK;
}

status_t MediaPlayer::getAudioStreamType(audio_stream_type_t *type)
{
    ALOGV("getAudioStreamType");
    Mutex::Autolock _l(mLock);
    *type = mStreamType;
    return OK;
}

status_t MediaPlayer::setLooping(int loop)
{
    ALOGV("MediaPlayer::setLooping");
    Mutex::Autolock _l(mLock);
    mLoop = (loop != 0);
    if (mPlayer != 0) {
        return mPlayer->setLooping(loop);
    }
    return OK;
}

bool MediaPlayer::isLooping() {
    ALOGV("isLooping");
    Mutex::Autolock _l(mLock);
    if (mPlayer != 0) {
        return mLoop;
    }
    ALOGV("isLooping: no active player");
    return false;
}

status_t MediaPlayer::setVolume(float leftVolume, float rightVolume)
{
    ALOGV("MediaPlayer::setVolume(%f, %f)", leftVolume, rightVolume);
    Mutex::Autolock _l(mLock);
    mLeftVolume = leftVolume;
    mRightVolume = rightVolume;
    if (mPlayer != 0) {
        return mPlayer->setVolume(leftVolume, rightVolume);
    }
    return OK;
}

status_t MediaPlayer::setAudioSessionId(audio_session_t sessionId)
{
    ALOGV("MediaPlayer::setAudioSessionId(%d)", sessionId);
    Mutex::Autolock _l(mLock);
    if (!(mCurrentState & MEDIA_PLAYER_IDLE)) {
        ALOGE("setAudioSessionId called in state %d", mCurrentState);
        return INVALID_OPERATION;
    }
    if (sessionId < 0) {
        return BAD_VALUE;
    }
    if (sessionId != mAudioSessionId) {
        AudioSystem::acquireAudioSessionId(sessionId, -1);
        AudioSystem::releaseAudioSessionId(mAudioSessionId, -1);
        mAudioSessionId = sessionId;
    }
    return NO_ERROR;
}

audio_session_t MediaPlayer::getAudioSessionId()
{
    Mutex::Autolock _l(mLock);
    return mAudioSessionId;
}

status_t MediaPlayer::setAuxEffectSendLevel(float level)
{
    ALOGV("MediaPlayer::setAuxEffectSendLevel(%f)", level);
    Mutex::Autolock _l(mLock);
    mSendLevel = level;
    if (mPlayer != 0) {
        return mPlayer->setAuxEffectSendLevel(level);
    }
    return OK;
}

status_t MediaPlayer::attachAuxEffect(int effectId)
{
    ALOGV("MediaPlayer::attachAuxEffect(%d)", effectId);
    Mutex::Autolock _l(mLock);
    if (mPlayer == 0 ||
        (mCurrentState & MEDIA_PLAYER_IDLE) ||
        (mCurrentState == MEDIA_PLAYER_STATE_ERROR )) {
        ALOGE("attachAuxEffect called in state %d, mPlayer(%p)", mCurrentState, mPlayer.get());
        return INVALID_OPERATION;
    }

    return mPlayer->attachAuxEffect(effectId);
}

// always call with lock held
status_t MediaPlayer::checkStateForKeySet_l(int key)
{
    switch(key) {
    case KEY_PARAMETER_AUDIO_ATTRIBUTES:
        if (mCurrentState & ( MEDIA_PLAYER_PREPARED | MEDIA_PLAYER_STARTED |
                MEDIA_PLAYER_PAUSED | MEDIA_PLAYER_PLAYBACK_COMPLETE) ) {
            // Can't change the audio attributes after prepare
            ALOGE("trying to set audio attributes called in state %d", mCurrentState);
            return INVALID_OPERATION;
        }
        break;
    default:
        // parameter doesn't require player state check
        break;
    }
    return OK;
}

status_t MediaPlayer::setParameter(int key, const Parcel& request)
{
    ALOGV("MediaPlayer::setParameter(%d)", key);
    status_t status = INVALID_OPERATION;
    Mutex::Autolock _l(mLock);
    if (checkStateForKeySet_l(key) != OK) {
        return status;
    }
    switch (key) {
    case KEY_PARAMETER_AUDIO_ATTRIBUTES:
        // save the marshalled audio attributes
        if (mAudioAttributesParcel != NULL) { delete mAudioAttributesParcel; };
        mAudioAttributesParcel = new Parcel();
        mAudioAttributesParcel->appendFrom(&request, 0, request.dataSize());
        status = OK;
        break;
    default:
        ALOGV_IF(mPlayer == NULL, "setParameter: no active player");
        break;
    }

    if (mPlayer != NULL) {
        status = mPlayer->setParameter(key, request);
    }
    return status;
}

status_t MediaPlayer::getParameter(int key, Parcel *reply)
{
    ALOGV("MediaPlayer::getParameter(%d)", key);
    Mutex::Autolock _l(mLock);
    if (mPlayer != NULL) {
        status_t status =  mPlayer->getParameter(key, reply);
        if (status != OK) {
            ALOGD("getParameter returns %d", status);
        }
        return status;
    }
    ALOGV("getParameter: no active player");
    return INVALID_OPERATION;
}

status_t MediaPlayer::setRetransmitEndpoint(const char* addrString,
                                            uint16_t port) {
    ALOGV("MediaPlayer::setRetransmitEndpoint(%s:%hu)",
            addrString ? addrString : "(null)", port);

    Mutex::Autolock _l(mLock);
    if ((mPlayer != NULL) || (mCurrentState != MEDIA_PLAYER_IDLE))
        return INVALID_OPERATION;

    if (NULL == addrString) {
        mRetransmitEndpointValid = false;
        return OK;
    }

    struct in_addr saddr;
    if(!inet_aton(addrString, &saddr)) {
        return BAD_VALUE;
    }

    memset(&mRetransmitEndpoint, 0, sizeof(mRetransmitEndpoint));
    mRetransmitEndpoint.sin_family = AF_INET;
    mRetransmitEndpoint.sin_addr   = saddr;
    mRetransmitEndpoint.sin_port   = htons(port);
    mRetransmitEndpointValid       = true;

    return OK;
}

void MediaPlayer::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
    ALOGV("message received msg=%d, ext1=%d, ext2=%d", msg, ext1, ext2);
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
    if (!(msg == MEDIA_ERROR && mCurrentState == MEDIA_PLAYER_IDLE) && mPlayer == 0) {
        ALOGV("notify(%d, %d, %d) callback on disconnected mediaplayer", msg, ext1, ext2);
        if (locked) mLock.unlock();   // release the lock when done.
        return;
    }

    switch (msg) {
    case MEDIA_NOP: // interface test message
        break;
    case MEDIA_PREPARED:
        ALOGV("MediaPlayer::notify() prepared");
        mCurrentState = MEDIA_PLAYER_PREPARED;
        if (mPrepareSync) {
            ALOGV("signal application thread");
            mPrepareSync = false;
            mPrepareStatus = NO_ERROR;
            mSignal.signal();
        }
        break;
    case MEDIA_DRM_INFO:
        ALOGV("MediaPlayer::notify() MEDIA_DRM_INFO(%d, %d, %d, %p)", msg, ext1, ext2, obj);
        break;
    case MEDIA_PLAYBACK_COMPLETE:
        ALOGV("playback complete");
        if (mCurrentState == MEDIA_PLAYER_IDLE) {
            ALOGE("playback complete in idle state");
        }
        if (!mLoop) {
            mCurrentState = MEDIA_PLAYER_PLAYBACK_COMPLETE;
        }
        break;
    case MEDIA_ERROR:
        // Always log errors.
        // ext1: Media framework error code.
        // ext2: Implementation dependant error code.
        ALOGE("error (%d, %d)", ext1, ext2);
        mCurrentState = MEDIA_PLAYER_STATE_ERROR;
        if (mPrepareSync)
        {
            ALOGV("signal application thread");
            mPrepareSync = false;
            mPrepareStatus = ext1;
            mSignal.signal();
            send = false;
        }
        break;
    case MEDIA_INFO:
        // ext1: Media framework error code.
        // ext2: Implementation dependant error code.
        if (ext1 != MEDIA_INFO_VIDEO_TRACK_LAGGING) {
            ALOGW("info/warning (%d, %d)", ext1, ext2);
        }
        break;
    case MEDIA_SEEK_COMPLETE:
        ALOGV("Received seek complete");
        if (mSeekPosition != mCurrentPosition || (mSeekMode != mCurrentSeekMode)) {
            ALOGV("Executing queued seekTo(%d, %d)", mCurrentPosition, mCurrentSeekMode);
            mSeekPosition = -1;
            mSeekMode = MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC;
            seekTo_l(mCurrentPosition, mCurrentSeekMode);
        }
        else {
            ALOGV("All seeks complete - return to regularly scheduled program");
            mCurrentPosition = mSeekPosition = -1;
            mCurrentSeekMode = mSeekMode = MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC;
        }
        break;
    case MEDIA_BUFFERING_UPDATE:
        ALOGV("buffering %d", ext1);
        break;
    case MEDIA_SET_VIDEO_SIZE:
        ALOGV("New video size %d x %d", ext1, ext2);
        mVideoWidth = ext1;
        mVideoHeight = ext2;
        break;
    case MEDIA_NOTIFY_TIME:
        ALOGV("Received notify time message");
        break;
    case MEDIA_TIMED_TEXT:
        ALOGV("Received timed text message");
        break;
    case MEDIA_SUBTITLE_DATA:
        ALOGV("Received subtitle data message");
        break;
    case MEDIA_META_DATA:
        ALOGV("Received timed metadata message");
        break;
    default:
        ALOGV("unrecognized message: (%d, %d, %d)", msg, ext1, ext2);
        break;
    }

    sp<MediaPlayerListener> listener = mListener;
    if (locked) mLock.unlock();

    // this prevents re-entrant calls into client code
    if ((listener != 0) && send) {
        Mutex::Autolock _l(mNotifyLock);
        ALOGV("callback application");
        listener->notify(msg, ext1, ext2, obj);
        ALOGV("back from callback");
    }
}

void MediaPlayer::died()
{
    ALOGV("died");
    notify(MEDIA_ERROR, MEDIA_ERROR_SERVER_DIED, 0);
}

status_t MediaPlayer::setNextMediaPlayer(const sp<MediaPlayer>& next) {
    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        return NO_INIT;
    }

    if (next != NULL && !(next->mCurrentState &
            (MEDIA_PLAYER_PREPARED | MEDIA_PLAYER_PAUSED | MEDIA_PLAYER_PLAYBACK_COMPLETE))) {
        ALOGE("next player is not prepared");
        return INVALID_OPERATION;
    }

    return mPlayer->setNextPlayer(next == NULL ? NULL : next->mPlayer);
}

VolumeShaper::Status MediaPlayer::applyVolumeShaper(
        const sp<VolumeShaper::Configuration>& configuration,
        const sp<VolumeShaper::Operation>& operation)
{
    Mutex::Autolock _l(mLock);
    if (mPlayer == nullptr) {
        return VolumeShaper::Status(NO_INIT);
    }
    VolumeShaper::Status status = mPlayer->applyVolumeShaper(configuration, operation);
    return status;
}

sp<VolumeShaper::State> MediaPlayer::getVolumeShaperState(int id)
{
    Mutex::Autolock _l(mLock);
    if (mPlayer == nullptr) {
        return nullptr;
    }
    return mPlayer->getVolumeShaperState(id);
}

// Modular DRM
status_t MediaPlayer::prepareDrm(const uint8_t uuid[16], const Vector<uint8_t>& drmSessionId)
{
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
    if (!(mCurrentState & (MEDIA_PLAYER_PREPARING | MEDIA_PLAYER_PREPARED))) {
        ALOGE("prepareDrm is called in the wrong state (%d).", mCurrentState);
        return INVALID_OPERATION;
    }

    if (drmSessionId.isEmpty()) {
        ALOGE("prepareDrm: Unexpected. Can't proceed with crypto. Empty drmSessionId.");
        return INVALID_OPERATION;
    }

    // Passing down to mediaserver mainly for creating the crypto
    status_t status = mPlayer->prepareDrm(uuid, drmSessionId);
    ALOGE_IF(status != OK, "prepareDrm: Failed at mediaserver with ret: %d", status);

    // TODO change to ALOGV
    ALOGD("prepareDrm: mediaserver::prepareDrm ret=%d", status);

    return status;
}

status_t MediaPlayer::releaseDrm()
{
    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        return NO_INIT;
    }

    // Not allowing releaseDrm in an active/resumable state
    if (mCurrentState & (MEDIA_PLAYER_STARTED |
                         MEDIA_PLAYER_PAUSED |
                         MEDIA_PLAYER_PLAYBACK_COMPLETE |
                         MEDIA_PLAYER_STATE_ERROR)) {
        ALOGE("releaseDrm Unexpected state %d. Can only be called in stopped/idle.", mCurrentState);
        return INVALID_OPERATION;
    }

    status_t status = mPlayer->releaseDrm();
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

status_t MediaPlayer::setOutputDevice(audio_port_handle_t deviceId)
{
    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        ALOGV("setOutputDevice: player not init");
        return NO_INIT;
    }
    return mPlayer->setOutputDevice(deviceId);
}

audio_port_handle_t MediaPlayer::getRoutedDeviceId()
{
    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        ALOGV("getRoutedDeviceId: player not init");
        return AUDIO_PORT_HANDLE_NONE;
    }
    audio_port_handle_t deviceId;
    status_t status = mPlayer->getRoutedDeviceId(&deviceId);
    if (status != NO_ERROR) {
        return AUDIO_PORT_HANDLE_NONE;
    }
    return deviceId;
}

status_t MediaPlayer::enableAudioDeviceCallback(bool enabled)
{
    Mutex::Autolock _l(mLock);
    if (mPlayer == NULL) {
        ALOGV("addAudioDeviceCallback: player not init");
        return NO_INIT;
    }
    return mPlayer->enableAudioDeviceCallback(enabled);
}

} // namespace android
