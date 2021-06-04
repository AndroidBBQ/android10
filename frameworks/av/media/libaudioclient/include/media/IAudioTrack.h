/*
 * Copyright (C) 2007 The Android Open Source Project
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

#ifndef ANDROID_IAUDIOTRACK_H
#define ANDROID_IAUDIOTRACK_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/IMemory.h>
#include <utils/String8.h>
#include <media/AudioTimestamp.h>
#include <media/VolumeShaper.h>

namespace android {

// ----------------------------------------------------------------------------

class IAudioTrack : public IInterface
{
public:
    DECLARE_META_INTERFACE(AudioTrack);

    /* Get this track's control block */
    virtual sp<IMemory> getCblk() const = 0;

    /* After it's created the track is not active. Call start() to
     * make it active.
     */
    virtual status_t    start() = 0;

    /* Stop a track. If set, the callback will cease being called and
     * obtainBuffer will return an error. Buffers that are already released
     * will continue to be processed, unless/until flush() is called.
     */
    virtual void        stop() = 0;

    /* Flush a stopped or paused track. All pending/released buffers are discarded.
     * This function has no effect if the track is not stopped or paused.
     */
    virtual void        flush() = 0;

    /* Pause a track. If set, the callback will cease being called and
     * obtainBuffer will return an error. Buffers that are already released
     * will continue to be processed, unless/until flush() is called.
     */
    virtual void        pause() = 0;

    /* Attach track auxiliary output to specified effect. Use effectId = 0
     * to detach track from effect.
     */
    virtual status_t    attachAuxEffect(int effectId) = 0;

    /* Send parameters to the audio hardware */
    virtual status_t    setParameters(const String8& keyValuePairs) = 0;

    /* Selects the presentation (if available) */
    virtual status_t    selectPresentation(int presentationId, int programId) = 0;

    /* Return NO_ERROR if timestamp is valid.  timestamp is undefined otherwise. */
    virtual status_t    getTimestamp(AudioTimestamp& timestamp) = 0;

    /* Signal the playback thread for a change in control block */
    virtual void        signal() = 0;

    /* Sets the volume shaper */
    virtual media::VolumeShaper::Status applyVolumeShaper(
            const sp<media::VolumeShaper::Configuration>& configuration,
            const sp<media::VolumeShaper::Operation>& operation) = 0;

    /* gets the volume shaper state */
    virtual sp<media::VolumeShaper::State> getVolumeShaperState(int id) = 0;
};

// ----------------------------------------------------------------------------

class BnAudioTrack : public BnInterface<IAudioTrack>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_IAUDIOTRACK_H
