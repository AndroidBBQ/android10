/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <binder/IServiceManager.h>
#include <media/PlayerBase.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

namespace android {

using media::VolumeShaper;

//--------------------------------------------------------------------------------------------------
PlayerBase::PlayerBase() : BnPlayer(),
        mPanMultiplierL(1.0f), mPanMultiplierR(1.0f),
        mVolumeMultiplierL(1.0f), mVolumeMultiplierR(1.0f),
        mPIId(PLAYER_PIID_INVALID), mLastReportedEvent(PLAYER_STATE_UNKNOWN)
{
    ALOGD("PlayerBase::PlayerBase()");
    // use checkService() to avoid blocking if audio service is not up yet
    sp<IBinder> binder = defaultServiceManager()->checkService(String16("audio"));
    if (binder == 0) {
        ALOGE("PlayerBase(): binding to audio service failed, service up?");
    } else {
        mAudioManager = interface_cast<IAudioManager>(binder);
    }
}


PlayerBase::~PlayerBase() {
    ALOGD("PlayerBase::~PlayerBase()");
    baseDestroy();
}

void PlayerBase::init(player_type_t playerType, audio_usage_t usage) {
    if (mAudioManager == 0) {
                ALOGE("AudioPlayer realize: no audio service, player will not be registered");
    } else {
        mPIId = mAudioManager->trackPlayer(playerType, usage, AUDIO_CONTENT_TYPE_UNKNOWN, this);
    }
}

void PlayerBase::baseDestroy() {
    serviceReleasePlayer();
    if (mAudioManager != 0) {
        mAudioManager.clear();
    }
}

//------------------------------------------------------------------------------
void PlayerBase::servicePlayerEvent(player_state_t event) {
    if (mAudioManager != 0) {
        // only report state change
        Mutex::Autolock _l(mPlayerStateLock);
        if (event != mLastReportedEvent
                && mPIId != PLAYER_PIID_INVALID) {
            mLastReportedEvent = event;
            mAudioManager->playerEvent(mPIId, event);
        }
    }
}

void PlayerBase::serviceReleasePlayer() {
    if (mAudioManager != 0
            && mPIId != PLAYER_PIID_INVALID) {
        mAudioManager->releasePlayer(mPIId);
    }
}

//FIXME temporary method while some player state is outside of this class
void PlayerBase::reportEvent(player_state_t event) {
    servicePlayerEvent(event);
}

status_t PlayerBase::startWithStatus() {
    status_t status = playerStart();
    if (status == NO_ERROR) {
        servicePlayerEvent(PLAYER_STATE_STARTED);
    } else {
        ALOGW("PlayerBase::start() error %d", status);
    }
    return status;
}

status_t PlayerBase::pauseWithStatus() {
    status_t status = playerPause();
    if (status == NO_ERROR) {
        servicePlayerEvent(PLAYER_STATE_PAUSED);
    } else {
        ALOGW("PlayerBase::pause() error %d", status);
    }
    return status;
}


status_t PlayerBase::stopWithStatus() {
    status_t status = playerStop();
    if (status == NO_ERROR) {
        servicePlayerEvent(PLAYER_STATE_STOPPED);
    } else {
        ALOGW("PlayerBase::stop() error %d", status);
    }
    return status;
}

//------------------------------------------------------------------------------
// Implementation of IPlayer
binder::Status PlayerBase::start() {
    ALOGD("PlayerBase::start() from IPlayer");
    (void)startWithStatus();
    return binder::Status::ok();
}

binder::Status PlayerBase::pause() {
    ALOGD("PlayerBase::pause() from IPlayer");
    (void)pauseWithStatus();
    return binder::Status::ok();
}


binder::Status PlayerBase::stop() {
    ALOGD("PlayerBase::stop() from IPlayer");
    (void)stopWithStatus();
    return binder::Status::ok();
}

binder::Status PlayerBase::setVolume(float vol) {
    ALOGD("PlayerBase::setVolume() from IPlayer");
    {
        Mutex::Autolock _l(mSettingsLock);
        mVolumeMultiplierL = vol;
        mVolumeMultiplierR = vol;
    }
    status_t status = playerSetVolume();
    if (status != NO_ERROR) {
        ALOGW("PlayerBase::setVolume() error %d", status);
    }
    return binder::Status::fromStatusT(status);
}

binder::Status PlayerBase::setPan(float pan) {
    ALOGD("PlayerBase::setPan() from IPlayer");
    {
        Mutex::Autolock _l(mSettingsLock);
        pan = min(max(-1.0f, pan), 1.0f);
        if (pan >= 0.0f) {
            mPanMultiplierL = 1.0f - pan;
            mPanMultiplierR = 1.0f;
        } else {
            mPanMultiplierL = 1.0f;
            mPanMultiplierR = 1.0f + pan;
        }
    }
    status_t status = playerSetVolume();
    if (status != NO_ERROR) {
        ALOGW("PlayerBase::setPan() error %d", status);
    }
    return binder::Status::fromStatusT(status);
}

binder::Status PlayerBase::setStartDelayMs(int32_t delayMs __unused) {
    ALOGW("setStartDelay() is not supported");
    return binder::Status::ok();
}

binder::Status PlayerBase::applyVolumeShaper(
            const VolumeShaper::Configuration& configuration __unused,
            const VolumeShaper::Operation& operation __unused) {
    ALOGW("applyVolumeShaper() is not supported");
    return binder::Status::ok();
}

} // namespace android
