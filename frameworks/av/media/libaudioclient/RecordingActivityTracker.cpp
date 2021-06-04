/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <audiomanager/AudioManager.h>
#include <audiomanager/IAudioManager.h>
#include <binder/Binder.h>
#include <binder/IServiceManager.h>
#include <media/RecordingActivityTracker.h>

namespace android {

RecordingActivityTracker::RecordingActivityTracker()
        : mRIId(RECORD_RIID_INVALID), mToken(new BBinder())
{
    // use checkService() to avoid blocking if audio service is not up yet
    sp<IBinder> binder = defaultServiceManager()->checkService(String16("audio"));
    if (binder != 0) {
        mAudioManager = interface_cast<IAudioManager>(binder);
    } else {
        ALOGE("RecordingActivityTracker(): binding to audio service failed, service up?");
    }
}

RecordingActivityTracker::~RecordingActivityTracker()
{
    if (mRIId != RECORD_RIID_INVALID && mAudioManager) {
        mAudioManager->releaseRecorder(mRIId);
    }
}

audio_unique_id_t RecordingActivityTracker::getRiid()
{
    if (mRIId == RECORD_RIID_INVALID && mAudioManager) {
        mRIId = mAudioManager->trackRecorder(mToken);
    }
    return mRIId;
}

void RecordingActivityTracker::recordingStarted()
{
    if (getRiid() != RECORD_RIID_INVALID && mAudioManager) {
        mAudioManager->recorderEvent(mRIId, RECORDER_STATE_STARTED);
    }
}

void RecordingActivityTracker::recordingStopped()
{
    if (getRiid() != RECORD_RIID_INVALID && mAudioManager) {
        mAudioManager->recorderEvent(mRIId, RECORDER_STATE_STOPPED);
    }
}

} // namespace android
