/*
 * Copyright (C) 2011 The Android Open Source Project
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

//#define USE_LOG SLAndroidLogLevel_Verbose

#include "sles_allinclusive.h"

#include <media/stagefright/foundation/ADebug.h>
#include <sys/stat.h>
#include <inttypes.h>

namespace android {

//--------------------------------------------------------------------------------------------------
GenericPlayer::GenericPlayer(const AudioPlayback_Parameters* params) :
        mDataLocatorType(kDataLocatorNone),
        mNotifyClient(NULL),
        mNotifyUser(NULL),
        mStateFlags(0),
        mPlaybackParams(*params),
        mDurationMsec(ANDROID_UNKNOWN_TIME),
        mPlaybackRatePermille(1000),
        mCacheStatus(kStatusEmpty),
        mCacheFill(0),
        mLastNotifiedCacheFill(0),
        mCacheFillNotifThreshold(100),
        mEventFlags(0),
        mMarkerPositionMs(ANDROID_UNKNOWN_TIME),
        mPositionUpdatePeriodMs(1000), // per spec
        mOneShotGeneration(0),
        mDeliveredNewPosMs(ANDROID_UNKNOWN_TIME),
        mObservedPositionMs(ANDROID_UNKNOWN_TIME)
{
    SL_LOGD("GenericPlayer::GenericPlayer()");

    mLooper = new android::ALooper();

    // Post-construction accesses need to be protected by mSettingsLock
    mAndroidAudioLevels.mFinalVolume[0] = 1.0f;
    mAndroidAudioLevels.mFinalVolume[1] = 1.0f;
}


GenericPlayer::~GenericPlayer() {
    SL_LOGV("GenericPlayer::~GenericPlayer()");

    resetDataLocator();
}


void GenericPlayer::init(const notif_cbf_t cbf, void* notifUser) {
    SL_LOGD("GenericPlayer::init()");

    {
        android::Mutex::Autolock autoLock(mNotifyClientLock);
        mNotifyClient = cbf;
        mNotifyUser = notifUser;
    }

    mLooper->registerHandler(this);
    mLooper->start(false /*runOnCallingThread*/, true /*canCallJava*/, PRIORITY_DEFAULT);
}


void GenericPlayer::preDestroy() {
    SL_LOGD("GenericPlayer::preDestroy()");
    {
        android::Mutex::Autolock autoLock(mNotifyClientLock);
        mNotifyClient = NULL;
        mNotifyUser = NULL;
    }

    mLooper->stop();
    mLooper->unregisterHandler(id());
}


void GenericPlayer::setDataSource(const char *uri) {
    SL_LOGV("GenericPlayer::setDataSource(uri=%s)", uri);
    resetDataLocator();

    mDataLocator.uriRef = uri;

    mDataLocatorType = kDataLocatorUri;
}


void GenericPlayer::setDataSource(int fd, int64_t offset, int64_t length, bool closeAfterUse) {
    SL_LOGV("GenericPlayer::setDataSource(fd=%d, offset=%lld, length=%lld, closeAfterUse=%s)", fd,
            offset, length, closeAfterUse ? "true" : "false");
    resetDataLocator();

    mDataLocator.fdi.fd = fd;

    struct stat sb;
    int ret = fstat(fd, &sb);
    if (ret != 0) {
        SL_LOGE("GenericPlayer::setDataSource: fstat(%d) failed: %d, %s", fd, ret, strerror(errno));
        return;
    }

    if (offset >= sb.st_size) {
        SL_LOGE("SfPlayer::setDataSource: invalid offset");
        return;
    }
    mDataLocator.fdi.offset = offset;

    if (PLAYER_FD_FIND_FILE_SIZE == length) {
        mDataLocator.fdi.length = sb.st_size;
    } else if (offset + length > sb.st_size) {
        mDataLocator.fdi.length = sb.st_size - offset;
    } else {
        mDataLocator.fdi.length = length;
    }

    mDataLocator.fdi.mCloseAfterUse = closeAfterUse;

    mDataLocatorType = kDataLocatorFd;
}


void GenericPlayer::prepare() {
    SL_LOGD("GenericPlayer::prepare()");
    // do not attempt prepare more than once
    if (!(mStateFlags & (kFlagPrepared | kFlagPreparedUnsuccessfully))) {
        sp<AMessage> msg = new AMessage(kWhatPrepare, this);
        msg->post();
    }
}


void GenericPlayer::play() {
    SL_LOGD("GenericPlayer::play()");
    sp<AMessage> msg = new AMessage(kWhatPlay, this);
    msg->post();
}


void GenericPlayer::pause() {
    SL_LOGD("GenericPlayer::pause()");
    sp<AMessage> msg = new AMessage(kWhatPause, this);
    msg->post();
}


void GenericPlayer::stop() {
    SL_LOGD("GenericPlayer::stop()");
    (new AMessage(kWhatPause, this))->post();

    // after a stop, playback should resume from the start.
    seek(0);
}


void GenericPlayer::seek(int64_t timeMsec) {
    SL_LOGV("GenericPlayer::seek %lld", timeMsec);
    if (timeMsec < 0 && timeMsec != ANDROID_UNKNOWN_TIME) {
        SL_LOGE("GenericPlayer::seek error, can't seek to negative time %" PRId64 "ms", timeMsec);
        return;
    }
    sp<AMessage> msg = new AMessage(kWhatSeek, this);
    msg->setInt64(WHATPARAM_SEEK_SEEKTIME_MS, timeMsec);
    msg->post();
}


void GenericPlayer::loop(bool loop) {
    SL_LOGV("GenericPlayer::loop %s", loop ? "true" : "false");
    sp<AMessage> msg = new AMessage(kWhatLoop, this);
    msg->setInt32(WHATPARAM_LOOP_LOOPING, (int32_t)loop);
    msg->post();
}


void GenericPlayer::setBufferingUpdateThreshold(int16_t thresholdPercent) {
    SL_LOGV("GenericPlayer::setBufferingUpdateThreshold %d", thresholdPercent);
    sp<AMessage> msg = new AMessage(kWhatBuffUpdateThres, this);
    msg->setInt32(WHATPARAM_BUFFERING_UPDATETHRESHOLD_PERCENT, (int32_t)thresholdPercent);
    msg->post();
}


//--------------------------------------------------
void GenericPlayer::getDurationMsec(int* msec) {
    Mutex::Autolock _l(mSettingsLock);
    *msec = mDurationMsec;
}

//--------------------------------------------------
void GenericPlayer::setVolume(float leftVol, float rightVol)
{
    {
        Mutex::Autolock _l(mSettingsLock);
        mAndroidAudioLevels.mFinalVolume[0] = leftVol;
        mAndroidAudioLevels.mFinalVolume[1] = rightVol;
    }
    // send a message for the volume to be updated by the object which implements the volume
    (new AMessage(kWhatVolumeUpdate, this))->post();
}


//--------------------------------------------------
void GenericPlayer::attachAuxEffect(int32_t effectId)
{
    SL_LOGV("GenericPlayer::attachAuxEffect(id=%d)", effectId);
    sp<AMessage> msg = new AMessage(kWhatAttachAuxEffect, this);
    msg->setInt32(WHATPARAM_ATTACHAUXEFFECT, effectId);
    msg->post();
}


//--------------------------------------------------
void GenericPlayer::setAuxEffectSendLevel(float level)
{
    SL_LOGV("GenericPlayer::setAuxEffectSendLevel(level=%g)", level);
    sp<AMessage> msg = new AMessage(kWhatSetAuxEffectSendLevel, this);
    msg->setFloat(WHATPARAM_SETAUXEFFECTSENDLEVEL, level);
    msg->post();
}


//--------------------------------------------------
void GenericPlayer::setPlaybackRate(int32_t ratePermille) {
    SL_LOGV("GenericPlayer::setPlaybackRate(ratePermille=%d)", ratePermille);
    {
        Mutex::Autolock _l(mSettingsLock);
        mPlaybackRatePermille = (int16_t)ratePermille;
    }
}

//--------------------------------------------------
// Call after changing any of the IPlay settings related to SL_PLAYEVENT_*
void GenericPlayer::setPlayEvents(int32_t eventFlags, int32_t markerPositionMs,
        int32_t positionUpdatePeriodMs)
{
    // Normalize ms that are within the valid unsigned range, but not in the int32_t range
    if (markerPositionMs < 0) {
        markerPositionMs = ANDROID_UNKNOWN_TIME;
    }
    if (positionUpdatePeriodMs < 0) {
        positionUpdatePeriodMs = ANDROID_UNKNOWN_TIME;
    }
    // markers are delivered accurately, but new position updates are limited to every 100 ms
    if (positionUpdatePeriodMs < 100) {
        positionUpdatePeriodMs = 100;
    }
    sp<AMessage> msg = new AMessage(kWhatSetPlayEvents, this);
    msg->setInt32(WHATPARAM_SETPLAYEVENTS_FLAGS, eventFlags);
    msg->setInt32(WHATPARAM_SETPLAYEVENTS_MARKER, markerPositionMs);
    msg->setInt32(WHATPARAM_SETPLAYEVENTS_UPDATE, positionUpdatePeriodMs);
    msg->post();
}


//--------------------------------------------------
/*
 * post-condition: mDataLocatorType == kDataLocatorNone
 *
 */
void GenericPlayer::resetDataLocator() {
    SL_LOGV("GenericPlayer::resetDataLocator()");
    if (mDataLocatorType == kDataLocatorFd && mDataLocator.fdi.mCloseAfterUse) {
        (void) ::close(mDataLocator.fdi.fd);
        // would be redundant, as we're about to invalidate the union mDataLocator
        //mDataLocator.fdi.fd = -1;
        //mDataLocator.fdi.mCloseAfterUse = false;
    }
    mDataLocatorType = kDataLocatorNone;
}


void GenericPlayer::notify(const char* event, int data, bool async) {
    SL_LOGV("GenericPlayer::notify(event=%s, data=%d, async=%s)", event, data,
            async ? "true" : "false");
    sp<AMessage> msg = new AMessage(kWhatNotif, this);
    msg->setInt32(event, (int32_t)data);
    if (async) {
        msg->post();
    } else {
        onNotify(msg);
    }
}


void GenericPlayer::notify(const char* event, int data1, int data2, bool async) {
    SL_LOGV("GenericPlayer::notify(event=%s, data1=%d, data2=%d, async=%s)", event, data1, data2,
            async ? "true" : "false");
    sp<AMessage> msg = new AMessage(kWhatNotif, this);
    msg->setRect(event, 0, 0, (int32_t)data1, (int32_t)data2);
    if (async) {
        msg->post();
    } else {
        onNotify(msg);
    }
}


//--------------------------------------------------
// AHandler implementation
void GenericPlayer::onMessageReceived(const sp<AMessage> &msg) {
    SL_LOGV("GenericPlayer::onMessageReceived()");
    switch (msg->what()) {
        case kWhatPrepare:
            SL_LOGV("kWhatPrepare");
            onPrepare();
            break;

        case kWhatNotif:
            SL_LOGV("kWhatNotif");
            onNotify(msg);
            break;

        case kWhatPlay:
            SL_LOGV("kWhatPlay");
            onPlay();
            break;

        case kWhatPause:
            SL_LOGV("kWhatPause");
            onPause();
            break;

        case kWhatSeek:
            SL_LOGV("kWhatSeek");
            onSeek(msg);
            break;

        case kWhatLoop:
            SL_LOGV("kWhatLoop");
            onLoop(msg);
            break;

        case kWhatVolumeUpdate:
            SL_LOGV("kWhatVolumeUpdate");
            onVolumeUpdate();
            break;

        case kWhatSeekComplete:
            SL_LOGV("kWhatSeekComplete");
            onSeekComplete();
            break;

        case kWhatBufferingUpdate:
            SL_LOGV("kWhatBufferingUpdate");
            onBufferingUpdate(msg);
            break;

        case kWhatBuffUpdateThres:
            SL_LOGV("kWhatBuffUpdateThres");
            onSetBufferingUpdateThreshold(msg);
            break;

        case kWhatAttachAuxEffect:
            SL_LOGV("kWhatAttachAuxEffect");
            onAttachAuxEffect(msg);
            break;

        case kWhatSetAuxEffectSendLevel:
            SL_LOGV("kWhatSetAuxEffectSendLevel");
            onSetAuxEffectSendLevel(msg);
            break;

        case kWhatSetPlayEvents:
            SL_LOGV("kWhatSetPlayEvents");
            onSetPlayEvents(msg);
            break;

        case kWhatOneShot:
            SL_LOGV("kWhatOneShot");
            onOneShot(msg);
            break;

        default:
            SL_LOGE("GenericPlayer::onMessageReceived unknown message %d", msg->what());
            TRESPASS();
    }
}


//--------------------------------------------------
// Event handlers
//  it is strictly verboten to call those methods outside of the event loop

void GenericPlayer::onPrepare() {
    SL_LOGV("GenericPlayer::onPrepare()");
    // Subclass is responsible for indicating whether prepare was successful or unsuccessful
    // by updating mStateFlags accordingly.  It must set exactly one of these two flags.
    assert(!(mStateFlags & kFlagPrepared) != !(mStateFlags & kFlagPreparedUnsuccessfully));
    notify(PLAYEREVENT_PREPARED, mStateFlags & kFlagPrepared ? PLAYER_SUCCESS : PLAYER_FAILURE,
            true /*async*/);
    SL_LOGD("GenericPlayer::onPrepare() done, mStateFlags=0x%x", mStateFlags);
}


void GenericPlayer::onNotify(const sp<AMessage> &msg) {
    SL_LOGV("GenericPlayer::onNotify()");
    notif_cbf_t notifClient;
    void*       notifUser;
    {
        android::Mutex::Autolock autoLock(mNotifyClientLock);
        if (NULL == mNotifyClient) {
            return;
        } else {
            notifClient = mNotifyClient;
            notifUser   = mNotifyUser;
        }
    }

    int32_t val1, val2;
    if (msg->findInt32(PLAYEREVENT_PREFETCHSTATUSCHANGE, &val1)) {
        SL_LOGV("GenericPlayer notifying %s = %d", PLAYEREVENT_PREFETCHSTATUSCHANGE, val1);
        notifClient(kEventPrefetchStatusChange, val1, 0, notifUser);
    // There is exactly one notification per message, hence "else if" instead of "if"
    } else if (msg->findInt32(PLAYEREVENT_PREFETCHFILLLEVELUPDATE, &val1)) {
        SL_LOGV("GenericPlayer notifying %s = %d", PLAYEREVENT_PREFETCHFILLLEVELUPDATE, val1);
        notifClient(kEventPrefetchFillLevelUpdate, val1, 0, notifUser);
    } else if (msg->findInt32(PLAYEREVENT_ENDOFSTREAM, &val1)) {
        SL_LOGV("GenericPlayer notifying %s = %d", PLAYEREVENT_ENDOFSTREAM, val1);
        notifClient(kEventEndOfStream, val1, 0, notifUser);
    } else if (msg->findInt32(PLAYEREVENT_PREPARED, &val1)) {
        SL_LOGV("GenericPlayer notifying %s = %d", PLAYEREVENT_PREPARED, val1);
        notifClient(kEventPrepared, val1, 0, notifUser);
    } else if (msg->findInt32(PLAYEREVENT_CHANNEL_COUNT, &val1)) {
        SL_LOGV("GenericPlayer notifying %s = %d", PLAYEREVENT_CHANNEL_COUNT, val1);
        notifClient(kEventChannelCount, val1, 0, notifUser);
    } else if (msg->findRect(PLAYEREVENT_VIDEO_SIZE_UPDATE, &val1, &val2, &val1, &val2)) {
        SL_LOGV("GenericPlayer notifying %s = %d, %d", PLAYEREVENT_VIDEO_SIZE_UPDATE, val1, val2);
        notifClient(kEventHasVideoSize, val1, val2, notifUser);
    } else if (msg->findInt32(PLAYEREVENT_PLAY, &val1)) {
        SL_LOGV("GenericPlayer notifying %s = %d", PLAYEREVENT_PLAY, val1);
        notifClient(kEventPlay, val1, 0, notifUser);
    } else if (msg->findInt32(PLAYEREVENT_ERRORAFTERPREPARE, &val1)) {
        SL_LOGV("GenericPlayer notifying %s = %d", PLAYEREVENT_ERRORAFTERPREPARE, val1);
        notifClient(kEventErrorAfterPrepare, val1, 0, notifUser);
    } else {
        SL_LOGV("GenericPlayer notifying unknown");
    }
}


void GenericPlayer::onPlay() {
    SL_LOGD("GenericPlayer::onPlay()");
    if ((mStateFlags & (kFlagPrepared | kFlagPlaying)) == kFlagPrepared) {
        SL_LOGD("starting player");
        mStateFlags |= kFlagPlaying;
        updateOneShot();
    }
}


void GenericPlayer::onPause() {
    SL_LOGD("GenericPlayer::onPause()");
    if (!(~mStateFlags & (kFlagPrepared | kFlagPlaying))) {
        SL_LOGV("pausing player");
        mStateFlags &= ~kFlagPlaying;
        updateOneShot();
    }
}


void GenericPlayer::onSeek(const sp<AMessage> &msg) {
    SL_LOGV("GenericPlayer::onSeek");
}


void GenericPlayer::onLoop(const sp<AMessage> &msg) {
    SL_LOGV("GenericPlayer::onLoop");
}


void GenericPlayer::onVolumeUpdate() {
    SL_LOGV("GenericPlayer::onVolumeUpdate");
}


void GenericPlayer::onSeekComplete() {
    SL_LOGD("GenericPlayer::onSeekComplete()");
    mStateFlags &= ~kFlagSeeking;
    // avoid spurious or lost events caused by seeking past a marker
    mDeliveredNewPosMs = ANDROID_UNKNOWN_TIME;
    mObservedPositionMs = ANDROID_UNKNOWN_TIME;
    updateOneShot();
}


void GenericPlayer::onBufferingUpdate(const sp<AMessage> &msg) {
    SL_LOGV("GenericPlayer::onBufferingUpdate");
}


void GenericPlayer::onSetBufferingUpdateThreshold(const sp<AMessage> &msg) {
    SL_LOGV("GenericPlayer::onSetBufferingUpdateThreshold");
    int32_t thresholdPercent = 0;
    if (msg->findInt32(WHATPARAM_BUFFERING_UPDATETHRESHOLD_PERCENT, &thresholdPercent)) {
        Mutex::Autolock _l(mSettingsLock);
        mCacheFillNotifThreshold = (int16_t)thresholdPercent;
    }
}


void GenericPlayer::onAttachAuxEffect(const sp<AMessage> &msg) {
    SL_LOGV("GenericPlayer::onAttachAuxEffect()");
}


void GenericPlayer::onSetAuxEffectSendLevel(const sp<AMessage> &msg) {
    SL_LOGV("GenericPlayer::onSetAuxEffectSendLevel()");
}


void GenericPlayer::onSetPlayEvents(const sp<AMessage> &msg) {
    SL_LOGV("GenericPlayer::onSetPlayEvents()");
    int32_t eventFlags, markerPositionMs, positionUpdatePeriodMs;
    if (msg->findInt32(WHATPARAM_SETPLAYEVENTS_FLAGS, &eventFlags) &&
            msg->findInt32(WHATPARAM_SETPLAYEVENTS_MARKER, &markerPositionMs) &&
            msg->findInt32(WHATPARAM_SETPLAYEVENTS_UPDATE, &positionUpdatePeriodMs)) {
        mEventFlags = eventFlags;
        mMarkerPositionMs = markerPositionMs;
        mPositionUpdatePeriodMs = positionUpdatePeriodMs;
        updateOneShot();
    }
}


void GenericPlayer::onOneShot(const sp<AMessage> &msg) {
    SL_LOGV("GenericPlayer::onOneShot()");
    int32_t generation;
    if (msg->findInt32(WHATPARAM_ONESHOT_GENERATION, &generation)) {
        if (generation != mOneShotGeneration) {
            SL_LOGV("GenericPlayer::onOneShot() generation %d cancelled; latest is %d",
                    generation, mOneShotGeneration);
            return;
        }
        updateOneShot();
    }
}


//-------------------------------------------------
void GenericPlayer::notifyStatus() {
    SL_LOGV("GenericPlayer::notifyStatus");
    notify(PLAYEREVENT_PREFETCHSTATUSCHANGE, (int32_t)mCacheStatus, true /*async*/);
}


void GenericPlayer::notifyCacheFill() {
    SL_LOGV("GenericPlayer::notifyCacheFill");
    mLastNotifiedCacheFill = mCacheFill;
    notify(PLAYEREVENT_PREFETCHFILLLEVELUPDATE, (int32_t)mLastNotifiedCacheFill, true/*async*/);
}


void GenericPlayer::seekComplete() {
    SL_LOGV("GenericPlayer::seekComplete");
    sp<AMessage> msg = new AMessage(kWhatSeekComplete, this);
    msg->post();
}


void GenericPlayer::bufferingUpdate(int16_t fillLevelPerMille) {
    SL_LOGV("GenericPlayer::bufferingUpdate");
    sp<AMessage> msg = new AMessage(kWhatBufferingUpdate, this);
    msg->setInt32(WHATPARAM_BUFFERING_UPDATE, fillLevelPerMille);
    msg->post();
}


// For the meaning of positionMs, see comment in declaration at android_GenericPlayer.h
void GenericPlayer::updateOneShot(int positionMs)
{
    SL_LOGV("GenericPlayer::updateOneShot");

    // nop until prepared
    if (!(mStateFlags & kFlagPrepared)) {
        return;
    }

    // cancel any pending one-shot(s)
    ++mOneShotGeneration;

    // don't restart one-shot if player is paused or stopped
    if (!(mStateFlags & kFlagPlaying)) {
        return;
    }

    // get current player position in milliseconds
    if (positionMs < 0) {
        positionMs = ANDROID_UNKNOWN_TIME;
    }
    if (positionMs == ANDROID_UNKNOWN_TIME) {
        getPositionMsec(&positionMs);
        // normalize it
        if (positionMs < 0) {
            positionMs = ANDROID_UNKNOWN_TIME;
        }
        if (ANDROID_UNKNOWN_TIME == positionMs) {
            // getPositionMsec is not working for some reason, give up
            //ALOGV("Does anyone really know what time it is?");
            return;
        }
    }

    // if we observe the player position going backwards, even without without a seek, then recover
    if (mObservedPositionMs != ANDROID_UNKNOWN_TIME && positionMs < mObservedPositionMs) {
        mDeliveredNewPosMs = ANDROID_UNKNOWN_TIME;
        mObservedPositionMs = positionMs;
    }

    // delayUs is the expected delay between current position and marker;
    // the default is infinity in case there are no upcoming marker(s)
    int64_t delayUs = -1;

    // is there a marker?
    if ((mEventFlags & SL_PLAYEVENT_HEADATMARKER) && (mMarkerPositionMs != ANDROID_UNKNOWN_TIME)) {
        // check to see if we have observed the position passing through the marker
        if (mObservedPositionMs <= mMarkerPositionMs && mMarkerPositionMs <= positionMs) {
            notify(PLAYEREVENT_PLAY, (int32_t) SL_PLAYEVENT_HEADATMARKER, true /*async*/);
        } else if (positionMs < mMarkerPositionMs) {
            delayUs = (mMarkerPositionMs - positionMs) * 1000LL;
        }
    }

    // are periodic position updates needed?
    if ((mEventFlags & SL_PLAYEVENT_HEADATNEWPOS) &&
            (mPositionUpdatePeriodMs != ANDROID_UNKNOWN_TIME)) {
        // check to see if we have observed the position passing through a virtual marker, where the
        // virtual marker is at the previously delivered new position plus position update period
        int32_t virtualMarkerMs;
        if (mDeliveredNewPosMs != ANDROID_UNKNOWN_TIME) {
            virtualMarkerMs = mDeliveredNewPosMs + mPositionUpdatePeriodMs;
        } else if (mObservedPositionMs != ANDROID_UNKNOWN_TIME) {
            virtualMarkerMs = mObservedPositionMs + mPositionUpdatePeriodMs;
            // pretend there has been an update in the past
            mDeliveredNewPosMs = mObservedPositionMs;
        } else {
            virtualMarkerMs = positionMs + mPositionUpdatePeriodMs;
            // pretend there has been an update in the past
            mDeliveredNewPosMs = positionMs;
        }
        // nextVirtualMarkerMs will be set to the position of the next upcoming virtual marker
        int32_t nextVirtualMarkerMs;
        if (mObservedPositionMs <= virtualMarkerMs && virtualMarkerMs <= positionMs) {
            // we did pass through the virtual marker, now compute the next virtual marker
            mDeliveredNewPosMs = virtualMarkerMs;
            nextVirtualMarkerMs = virtualMarkerMs + mPositionUpdatePeriodMs;
            // re-synchronize if we missed an update
            if (nextVirtualMarkerMs <= positionMs) {
                SL_LOGW("Missed SL_PLAYEVENT_HEADATNEWPOS for position %d; current position %d",
                        nextVirtualMarkerMs, positionMs);
                // try to catch up by setting next goal to current position plus update period
                mDeliveredNewPosMs = positionMs;
                nextVirtualMarkerMs = positionMs + mPositionUpdatePeriodMs;
            }
            notify(PLAYEREVENT_PLAY, (int32_t) SL_PLAYEVENT_HEADATNEWPOS, true /*async*/);
        } else {
            // we did not pass through the virtual marker yet, so use same marker again
            nextVirtualMarkerMs = virtualMarkerMs;
        }
        // note that if arithmetic overflow occurred, nextVirtualMarkerMs will be negative
        if (positionMs < nextVirtualMarkerMs) {
            int64_t trialDelayUs;
            trialDelayUs = (nextVirtualMarkerMs - positionMs) * 1000LL;
            if (trialDelayUs > 0 && (delayUs == -1 || trialDelayUs < delayUs)) {
                delayUs = trialDelayUs;
            }
        }
    }

    // we have a new observed position
    mObservedPositionMs = positionMs;

    if (mPlaybackRatePermille == 0) {
        // playback is frozen, no update expected (and no division by zero below)
        return;
    }

    // post the new one-shot message if needed
    if (advancesPositionInRealTime() && delayUs >= 0) {
        // scale delay according to playback rate (reported positions won't change, but reported
        // time will advance slower or faster depending on rate)
        {
            Mutex::Autolock _l(mSettingsLock);
            delayUs =  delayUs * 1000 / mPlaybackRatePermille;
        }

        // 20 ms min delay to avoid near busy waiting
        if (delayUs < 20000LL) {
            delayUs = 20000LL;
        }
        // 1 minute max delay avoids indefinite memory leaks caused by cancelled one-shots
        if (delayUs > 60000000LL) {
            delayUs = 60000000LL;
        }
        //SL_LOGI("delayUs = %lld", delayUs);
        sp<AMessage> msg = new AMessage(kWhatOneShot, this);
        msg->setInt32(WHATPARAM_ONESHOT_GENERATION, mOneShotGeneration);
        msg->post(delayUs);
    }

}

} // namespace android
