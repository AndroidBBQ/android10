/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef ANDROID_IAUDIOPOLICYSERVICECLIENT_H
#define ANDROID_IAUDIOPOLICYSERVICECLIENT_H

#include <vector>

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <system/audio.h>
#include <system/audio_effect.h>
#include <media/AudioPolicy.h>
#include <media/AudioVolumeGroup.h>

namespace android {

// ----------------------------------------------------------------------------

struct record_client_info {
    audio_unique_id_t riid;
    uid_t uid;
    audio_session_t session;
    audio_source_t source;
    audio_port_handle_t port_id;
    bool silenced;
};

typedef struct record_client_info record_client_info_t;

// ----------------------------------------------------------------------------

class IAudioPolicyServiceClient : public IInterface
{
public:
    DECLARE_META_INTERFACE(AudioPolicyServiceClient);

    // Notifies a change of volume group
    virtual void onAudioVolumeGroupChanged(volume_group_t group, int flags) = 0;
    // Notifies a change of audio port configuration.
    virtual void onAudioPortListUpdate() = 0;
    // Notifies a change of audio patch configuration.
    virtual void onAudioPatchListUpdate() = 0;
    // Notifies a change in the mixing state of a specific mix in a dynamic audio policy
    virtual void onDynamicPolicyMixStateUpdate(String8 regId, int32_t state) = 0;
    // Notifies a change of audio recording configuration
    virtual void onRecordingConfigurationUpdate(int event,
            const record_client_info_t *clientInfo,
            const audio_config_base_t *clientConfig,
            std::vector<effect_descriptor_t> clientEffects,
            const audio_config_base_t *deviceConfig,
            std::vector<effect_descriptor_t> effects,
            audio_patch_handle_t patchHandle,
            audio_source_t source) = 0;
};


// ----------------------------------------------------------------------------

class BnAudioPolicyServiceClient : public BnInterface<IAudioPolicyServiceClient>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_IAUDIOPOLICYSERVICECLIENT_H
