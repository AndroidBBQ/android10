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

#include <media/TrackPlayerBase.h>

namespace android {

using media::VolumeShaper;

//--------------------------------------------------------------------------------------------------
TrackPlayerBase::TrackPlayerBase() : PlayerBase(),
        mPlayerVolumeL(1.0f), mPlayerVolumeR(1.0f)
{
    ALOGD("TrackPlayerBase::TrackPlayerBase()");
}


TrackPlayerBase::~TrackPlayerBase() {
    ALOGD("TrackPlayerBase::~TrackPlayerBase()");
    doDestroy();
}

void TrackPlayerBase::init(AudioTrack* pat, player_type_t playerType, audio_usage_t usage) {
    PlayerBase::init(playerType, usage);
    mAudioTrack = pat;
}

void TrackPlayerBase::destroy() {
    doDestroy();
    baseDestroy();
}

void TrackPlayerBase::doDestroy() {
    if (mAudioTrack != 0) {
        mAudioTrack->stop();
        // Note that there may still be another reference in post-unlock phase of SetPlayState
        mAudioTrack.clear();
    }
}

void TrackPlayerBase::setPlayerVolume(float vl, float vr) {
    {
        Mutex::Autolock _l(mSettingsLock);
        mPlayerVolumeL = vl;
        mPlayerVolumeR = vr;
    }
    doSetVolume();
}

//------------------------------------------------------------------------------
// Implementation of IPlayer
status_t TrackPlayerBase::playerStart() {
    status_t status = NO_INIT;
    if (mAudioTrack != 0) {
        status = mAudioTrack->start();
    }
    return status;
}

status_t TrackPlayerBase::playerPause() {
    status_t status = NO_INIT;
    if (mAudioTrack != 0) {
        mAudioTrack->pause();
        status = NO_ERROR;
    }
    return status;
}


status_t TrackPlayerBase::playerStop() {
    status_t status = NO_INIT;
    if (mAudioTrack != 0) {
        mAudioTrack->stop();
        status = NO_ERROR;
    }
    return status;
}

status_t TrackPlayerBase::playerSetVolume() {
    return doSetVolume();
}

status_t TrackPlayerBase::doSetVolume() {
    status_t status = NO_INIT;
    if (mAudioTrack != 0) {
        float tl = mPlayerVolumeL * mPanMultiplierL * mVolumeMultiplierL;
        float tr = mPlayerVolumeR * mPanMultiplierR * mVolumeMultiplierR;
        mAudioTrack->setVolume(tl, tr);
        status = NO_ERROR;
    }
    return status;
}


binder::Status TrackPlayerBase::applyVolumeShaper(
        const VolumeShaper::Configuration& configuration,
        const VolumeShaper::Operation& operation) {

    sp<VolumeShaper::Configuration> spConfiguration = new VolumeShaper::Configuration(configuration);
    sp<VolumeShaper::Operation> spOperation = new VolumeShaper::Operation(operation);

    if (mAudioTrack != 0) {
        ALOGD("TrackPlayerBase::applyVolumeShaper() from IPlayer");
        VolumeShaper::Status status = mAudioTrack->applyVolumeShaper(spConfiguration, spOperation);
        if (status < 0) { // a non-negative value is the volume shaper id.
            ALOGE("TrackPlayerBase::applyVolumeShaper() failed with status %d", status);
        }
        return binder::Status::fromStatusT(status);
    } else {
        ALOGD("TrackPlayerBase::applyVolumeShaper()"
              " no AudioTrack for volume control from IPlayer");
        return binder::Status::ok();
    }
}

} // namespace android
