/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef ANDROID_AV_SYNC_SETTINGS_H
#define ANDROID_AV_SYNC_SETTINGS_H

namespace android {

enum AVSyncSource : unsigned {
    // let the system decide the best sync source
    AVSYNC_SOURCE_DEFAULT = 0,
    // sync to the system clock
    AVSYNC_SOURCE_SYSTEM_CLOCK = 1,
    // sync to the audio track
    AVSYNC_SOURCE_AUDIO = 2,
    // sync to the display vsync
    AVSYNC_SOURCE_VSYNC = 3,
    AVSYNC_SOURCE_MAX,
};

enum AVSyncAudioAdjustMode : unsigned {
    // let the system decide the best audio adjust mode
    AVSYNC_AUDIO_ADJUST_MODE_DEFAULT = 0,
    // adjust audio by time stretching
    AVSYNC_AUDIO_ADJUST_MODE_STRETCH = 1,
    // adjust audio by resampling
    AVSYNC_AUDIO_ADJUST_MODE_RESAMPLE = 2,
    AVSYNC_AUDIO_ADJUST_MODE_MAX,
};

// max tolerance when adjusting playback speed to desired playback speed
#define AVSYNC_TOLERANCE_MAX 1.0f

struct AVSyncSettings {
    AVSyncSource mSource;
    AVSyncAudioAdjustMode mAudioAdjustMode;
    float mTolerance;
    AVSyncSettings()
        : mSource(AVSYNC_SOURCE_DEFAULT),
          mAudioAdjustMode(AVSYNC_AUDIO_ADJUST_MODE_DEFAULT),
          mTolerance(.044f) { }
};

} // namespace android

// ---------------------------------------------------------------------------

#endif // ANDROID_AV_SYNC_SETTINGS_H
