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

#ifndef __ANDROID_RECORDING_ACTIVITY_TRACKER_H__
#define __ANDROID_RECORDING_ACTIVITY_TRACKER_H__

#include <utils/StrongPointer.h>

namespace android {

class IAudioManager;
class IBinder;

class RecordingActivityTracker
{
public:
    RecordingActivityTracker();
    ~RecordingActivityTracker();
    audio_unique_id_t getRiid();
    void recordingStarted();
    void recordingStopped();

private:
    sp<IAudioManager> mAudioManager;
    audio_unique_id_t mRIId;
    sp<IBinder>       mToken;
};

} // namespace android

#endif // __ANDROID_RECORDING_ACTIVITY_TRACKER_H__
