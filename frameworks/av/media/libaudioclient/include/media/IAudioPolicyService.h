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

#ifndef ANDROID_IAUDIOPOLICYSERVICE_H
#define ANDROID_IAUDIOPOLICYSERVICE_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <media/AudioSystem.h>
#include <media/AudioPolicy.h>
#include <media/IAudioPolicyServiceClient.h>
#include <system/audio_policy.h>
#include <vector>

namespace android {

// ----------------------------------------------------------------------------

class IAudioPolicyService : public IInterface
{
public:
    DECLARE_META_INTERFACE(AudioPolicyService);

    //
    // IAudioPolicyService interface (see AudioPolicyInterface for method descriptions)
    //
    virtual status_t setDeviceConnectionState(audio_devices_t device,
                                              audio_policy_dev_state_t state,
                                              const char *device_address,
                                              const char *device_name,
                                              audio_format_t encodedFormat) = 0;
    virtual audio_policy_dev_state_t getDeviceConnectionState(audio_devices_t device,
                                                                  const char *device_address) = 0;
    virtual status_t handleDeviceConfigChange(audio_devices_t device,
                                              const char *device_address,
                                              const char *device_name,
                                              audio_format_t encodedFormat) = 0;
    virtual status_t setPhoneState(audio_mode_t state) = 0;
    virtual status_t setForceUse(audio_policy_force_use_t usage,
                                    audio_policy_forced_cfg_t config) = 0;
    virtual audio_policy_forced_cfg_t getForceUse(audio_policy_force_use_t usage) = 0;
    virtual audio_io_handle_t getOutput(audio_stream_type_t stream) = 0;
    virtual status_t getOutputForAttr(audio_attributes_t *attr,
                                      audio_io_handle_t *output,
                                      audio_session_t session,
                                      audio_stream_type_t *stream,
                                      pid_t pid,
                                      uid_t uid,
                                      const audio_config_t *config,
                                      audio_output_flags_t flags,
                                      audio_port_handle_t *selectedDeviceId,
                                      audio_port_handle_t *portId,
                                      std::vector<audio_io_handle_t> *secondaryOutputs) = 0;
    virtual status_t startOutput(audio_port_handle_t portId) = 0;
    virtual status_t stopOutput(audio_port_handle_t portId) = 0;
    virtual void releaseOutput(audio_port_handle_t portId) = 0;
    virtual status_t  getInputForAttr(const audio_attributes_t *attr,
                              audio_io_handle_t *input,
                              audio_unique_id_t riid,
                              audio_session_t session,
                              pid_t pid,
                              uid_t uid,
                              const String16& opPackageName,
                              const audio_config_base_t *config,
                              audio_input_flags_t flags,
                              audio_port_handle_t *selectedDeviceId,
                              audio_port_handle_t *portId) = 0;
    virtual status_t startInput(audio_port_handle_t portId) = 0;
    virtual status_t stopInput(audio_port_handle_t portId) = 0;
    virtual void releaseInput(audio_port_handle_t portId) = 0;
    virtual status_t initStreamVolume(audio_stream_type_t stream,
                                      int indexMin,
                                      int indexMax) = 0;
    virtual status_t setStreamVolumeIndex(audio_stream_type_t stream,
                                          int index,
                                          audio_devices_t device) = 0;
    virtual status_t getStreamVolumeIndex(audio_stream_type_t stream,
                                          int *index,
                                          audio_devices_t device) = 0;

    virtual status_t setVolumeIndexForAttributes(const audio_attributes_t &attr,
                                                 int index,
                                                 audio_devices_t device) = 0;
    virtual status_t getVolumeIndexForAttributes(const audio_attributes_t &attr,
                                                 int &index,
                                                 audio_devices_t device) = 0;
    virtual status_t getMaxVolumeIndexForAttributes(const audio_attributes_t &attr, int &index) = 0;

    virtual status_t getMinVolumeIndexForAttributes(const audio_attributes_t &attr, int &index) = 0;

    virtual uint32_t getStrategyForStream(audio_stream_type_t stream) = 0;
    virtual audio_devices_t getDevicesForStream(audio_stream_type_t stream) = 0;
    virtual audio_io_handle_t getOutputForEffect(const effect_descriptor_t *desc) = 0;
    virtual status_t registerEffect(const effect_descriptor_t *desc,
                                    audio_io_handle_t io,
                                    uint32_t strategy,
                                    audio_session_t session,
                                    int id) = 0;
    virtual status_t unregisterEffect(int id) = 0;
    virtual status_t setEffectEnabled(int id, bool enabled) = 0;
    virtual status_t moveEffectsToIo(const std::vector<int>& ids, audio_io_handle_t io) = 0;
    virtual bool     isStreamActive(audio_stream_type_t stream, uint32_t inPastMs = 0) const = 0;
    virtual bool     isStreamActiveRemotely(audio_stream_type_t stream, uint32_t inPastMs = 0)
                             const = 0;
    virtual bool     isSourceActive(audio_source_t source) const = 0;
    virtual status_t queryDefaultPreProcessing(audio_session_t audioSession,
                                              effect_descriptor_t *descriptors,
                                              uint32_t *count) = 0;
    virtual status_t addSourceDefaultEffect(const effect_uuid_t *type,
                                            const String16& opPackageName,
                                            const effect_uuid_t *uuid,
                                            int32_t priority,
                                            audio_source_t source,
                                            audio_unique_id_t* id) = 0;
    virtual status_t addStreamDefaultEffect(const effect_uuid_t *type,
                                            const String16& opPackageName,
                                            const effect_uuid_t *uuid,
                                            int32_t priority,
                                            audio_usage_t usage,
                                            audio_unique_id_t* id) = 0;
    virtual status_t removeSourceDefaultEffect(audio_unique_id_t id) = 0;
    virtual status_t removeStreamDefaultEffect(audio_unique_id_t id) = 0;
    virtual status_t setAllowedCapturePolicy(uid_t uid, audio_flags_mask_t flags) = 0;
   // Check if offload is possible for given format, stream type, sample rate,
    // bit rate, duration, video and streaming or offload property is enabled
    virtual bool isOffloadSupported(const audio_offload_info_t& info) = 0;

    // Check if direct playback is possible for given format, sample rate, channel mask and flags.
    virtual bool isDirectOutputSupported(const audio_config_base_t& config,
                                         const audio_attributes_t& attributes) = 0;

    /* List available audio ports and their attributes */
    virtual status_t listAudioPorts(audio_port_role_t role,
                                    audio_port_type_t type,
                                    unsigned int *num_ports,
                                    struct audio_port *ports,
                                    unsigned int *generation) = 0;

    /* Get attributes for a given audio port */
    virtual status_t getAudioPort(struct audio_port *port) = 0;

    /* Create an audio patch between several source and sink ports */
    virtual status_t createAudioPatch(const struct audio_patch *patch,
                                       audio_patch_handle_t *handle) = 0;

    /* Release an audio patch */
    virtual status_t releaseAudioPatch(audio_patch_handle_t handle) = 0;

    /* List existing audio patches */
    virtual status_t listAudioPatches(unsigned int *num_patches,
                                      struct audio_patch *patches,
                                      unsigned int *generation) = 0;
    /* Set audio port configuration */
    virtual status_t setAudioPortConfig(const struct audio_port_config *config) = 0;

    virtual void registerClient(const sp<IAudioPolicyServiceClient>& client) = 0;

    virtual void setAudioPortCallbacksEnabled(bool enabled) = 0;

    virtual void setAudioVolumeGroupCallbacksEnabled(bool enabled) = 0;

    virtual status_t acquireSoundTriggerSession(audio_session_t *session,
                                           audio_io_handle_t *ioHandle,
                                           audio_devices_t *device) = 0;

    virtual status_t releaseSoundTriggerSession(audio_session_t session) = 0;

    virtual audio_mode_t getPhoneState() = 0;

    virtual status_t registerPolicyMixes(const Vector<AudioMix>& mixes, bool registration) = 0;

    virtual status_t setUidDeviceAffinities(uid_t uid, const Vector<AudioDeviceTypeAddr>& devices)
            = 0;

    virtual status_t removeUidDeviceAffinities(uid_t uid) = 0;

    virtual status_t startAudioSource(const struct audio_port_config *source,
                                      const audio_attributes_t *attributes,
                                      audio_port_handle_t *portId) = 0;
    virtual status_t stopAudioSource(audio_port_handle_t portId) = 0;

    virtual status_t setMasterMono(bool mono) = 0;
    virtual status_t getMasterMono(bool *mono) = 0;
    virtual float    getStreamVolumeDB(
            audio_stream_type_t stream, int index, audio_devices_t device) = 0;

    virtual status_t getSurroundFormats(unsigned int *numSurroundFormats,
                                        audio_format_t *surroundFormats,
                                        bool *surroundFormatsEnabled,
                                        bool reported) = 0;
    virtual status_t getHwOffloadEncodingFormatsSupportedForA2DP(
                                        std::vector<audio_format_t> *formats) = 0;
    virtual status_t setSurroundFormatEnabled(audio_format_t audioFormat, bool enabled) = 0;

    virtual status_t setAssistantUid(uid_t uid) = 0;
    virtual status_t setA11yServicesUids(const std::vector<uid_t>& uids) = 0;

    virtual bool     isHapticPlaybackSupported() = 0;
    virtual status_t listAudioProductStrategies(AudioProductStrategyVector &strategies) = 0;
    virtual status_t getProductStrategyFromAudioAttributes(const AudioAttributes &aa,
                                                           product_strategy_t &productStrategy) = 0;

    virtual status_t listAudioVolumeGroups(AudioVolumeGroupVector &groups) = 0;
    virtual status_t getVolumeGroupFromAudioAttributes(const AudioAttributes &aa,
                                                       volume_group_t &volumeGroup) = 0;

    virtual status_t setRttEnabled(bool enabled) = 0;
};


// ----------------------------------------------------------------------------

class BnAudioPolicyService : public BnInterface<IAudioPolicyService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
private:
    void sanetizeAudioAttributes(audio_attributes_t* attr);
    status_t sanitizeEffectDescriptor(effect_descriptor_t* desc);
    status_t sanitizeAudioPortConfig(struct audio_port_config* config);
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_IAUDIOPOLICYSERVICE_H
