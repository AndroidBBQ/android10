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

#ifndef __ANDROID_TRACK_PLAYER_BASE_H__
#define __ANDROID_TRACK_PLAYER_BASE_H__

#include <media/AudioTrack.h>
#include <media/PlayerBase.h>

namespace android {

class TrackPlayerBase : public PlayerBase
{
public:
    explicit TrackPlayerBase();
    virtual ~TrackPlayerBase();

            void init(AudioTrack* pat, player_type_t playerType, audio_usage_t usage);
    virtual void destroy();

    //IPlayer implementation
    virtual binder::Status applyVolumeShaper(
            const media::VolumeShaper::Configuration& configuration,
            const media::VolumeShaper::Operation& operation);

    //FIXME move to protected field, so far made public to minimize changes to AudioTrack logic
    sp<AudioTrack> mAudioTrack;

            void setPlayerVolume(float vl, float vr);

protected:

    //PlayerBase virtuals
    virtual status_t playerStart();
    virtual status_t playerPause();
    virtual status_t playerStop();
    virtual status_t playerSetVolume();

private:
            void doDestroy();
            status_t doSetVolume();

    // volume coming from the player volume API
    float mPlayerVolumeL, mPlayerVolumeR;
};

} // namespace android

#endif /* __ANDROID_TRACK_PLAYER_BASE_H__ */
