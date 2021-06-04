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

#define LOG_TAG "AudioPolicyClientImpl"
//#define LOG_NDEBUG 0

#include <soundtrigger/SoundTrigger.h>
#include <utils/Log.h>
#include "AudioPolicyService.h"

namespace android {

/* implementation of the client interface from the policy manager */

audio_module_handle_t AudioPolicyService::AudioPolicyClient::loadHwModule(const char *name)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        ALOGW("%s: could not get AudioFlinger", __func__);
        return AUDIO_MODULE_HANDLE_NONE;
    }

    return af->loadHwModule(name);
}

status_t AudioPolicyService::AudioPolicyClient::openOutput(audio_module_handle_t module,
                                                           audio_io_handle_t *output,
                                                           audio_config_t *config,
                                                           audio_devices_t *devices,
                                                           const String8& address,
                                                           uint32_t *latencyMs,
                                                           audio_output_flags_t flags)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        ALOGW("%s: could not get AudioFlinger", __func__);
        return PERMISSION_DENIED;
    }
    return af->openOutput(module, output, config, devices, address, latencyMs, flags);
}

audio_io_handle_t AudioPolicyService::AudioPolicyClient::openDuplicateOutput(
                                                                audio_io_handle_t output1,
                                                                audio_io_handle_t output2)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        ALOGW("%s: could not get AudioFlinger", __func__);
        return 0;
    }
    return af->openDuplicateOutput(output1, output2);
}

status_t AudioPolicyService::AudioPolicyClient::closeOutput(audio_io_handle_t output)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        return PERMISSION_DENIED;
    }

    return af->closeOutput(output);
}

status_t AudioPolicyService::AudioPolicyClient::suspendOutput(audio_io_handle_t output)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        ALOGW("%s: could not get AudioFlinger", __func__);
        return PERMISSION_DENIED;
    }

    return af->suspendOutput(output);
}

status_t AudioPolicyService::AudioPolicyClient::restoreOutput(audio_io_handle_t output)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        ALOGW("%s: could not get AudioFlinger", __func__);
        return PERMISSION_DENIED;
    }

    return af->restoreOutput(output);
}

status_t AudioPolicyService::AudioPolicyClient::openInput(audio_module_handle_t module,
                                                          audio_io_handle_t *input,
                                                          audio_config_t *config,
                                                          audio_devices_t *device,
                                                          const String8& address,
                                                          audio_source_t source,
                                                          audio_input_flags_t flags)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        ALOGW("%s: could not get AudioFlinger", __func__);
        return PERMISSION_DENIED;
    }

    return af->openInput(module, input, config, device, address, source, flags);
}

status_t AudioPolicyService::AudioPolicyClient::closeInput(audio_io_handle_t input)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        return PERMISSION_DENIED;
    }

    return af->closeInput(input);
}

status_t AudioPolicyService::AudioPolicyClient::setStreamVolume(audio_stream_type_t stream,
                     float volume, audio_io_handle_t output,
                     int delay_ms)
{
    return mAudioPolicyService->setStreamVolume(stream, volume, output,
                                               delay_ms);
}

status_t AudioPolicyService::AudioPolicyClient::invalidateStream(audio_stream_type_t stream)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        return PERMISSION_DENIED;
    }

    return af->invalidateStream(stream);
}

void AudioPolicyService::AudioPolicyClient::setParameters(audio_io_handle_t io_handle,
                   const String8& keyValuePairs,
                   int delay_ms)
{
    mAudioPolicyService->setParameters(io_handle, keyValuePairs.string(), delay_ms);
}

String8 AudioPolicyService::AudioPolicyClient::getParameters(audio_io_handle_t io_handle,
                      const String8& keys)
{
    String8 result = AudioSystem::getParameters(io_handle, keys);
    return result;
}

status_t AudioPolicyService::AudioPolicyClient::setVoiceVolume(float volume, int delay_ms)
{
    return mAudioPolicyService->setVoiceVolume(volume, delay_ms);
}

status_t AudioPolicyService::AudioPolicyClient::moveEffects(audio_session_t session,
                        audio_io_handle_t src_output,
                        audio_io_handle_t dst_output)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        return PERMISSION_DENIED;
    }

    return af->moveEffects(session, src_output, dst_output);
}

void AudioPolicyService::AudioPolicyClient::setEffectSuspended(int effectId,
                                audio_session_t sessionId,
                                bool suspended)
{
    mAudioPolicyService->setEffectSuspended(effectId, sessionId, suspended);
}

status_t AudioPolicyService::AudioPolicyClient::createAudioPatch(const struct audio_patch *patch,
                                                                  audio_patch_handle_t *handle,
                                                                  int delayMs)
{
    return mAudioPolicyService->clientCreateAudioPatch(patch, handle, delayMs);
}

status_t AudioPolicyService::AudioPolicyClient::releaseAudioPatch(audio_patch_handle_t handle,
                                                                  int delayMs)
{
    return mAudioPolicyService->clientReleaseAudioPatch(handle, delayMs);
}

status_t AudioPolicyService::AudioPolicyClient::setAudioPortConfig(
                                                        const struct audio_port_config *config,
                                                        int delayMs)
{
    return mAudioPolicyService->clientSetAudioPortConfig(config, delayMs);
}

void AudioPolicyService::AudioPolicyClient::onAudioPortListUpdate()
{
    mAudioPolicyService->onAudioPortListUpdate();
}

void AudioPolicyService::AudioPolicyClient::onAudioPatchListUpdate()
{
    mAudioPolicyService->onAudioPatchListUpdate();
}

void AudioPolicyService::AudioPolicyClient::onDynamicPolicyMixStateUpdate(
        String8 regId, int32_t state)
{
    mAudioPolicyService->onDynamicPolicyMixStateUpdate(regId, state);
}

void AudioPolicyService::AudioPolicyClient::onRecordingConfigurationUpdate(
                                                    int event,
                                                    const record_client_info_t *clientInfo,
                                                    const audio_config_base_t *clientConfig,
                                                    std::vector<effect_descriptor_t> clientEffects,
                                                    const audio_config_base_t *deviceConfig,
                                                    std::vector<effect_descriptor_t> effects,
                                                    audio_patch_handle_t patchHandle,
                                                    audio_source_t source)
{
    mAudioPolicyService->onRecordingConfigurationUpdate(event, clientInfo,
            clientConfig, clientEffects, deviceConfig, effects, patchHandle, source);
}

void AudioPolicyService::AudioPolicyClient::onAudioVolumeGroupChanged(volume_group_t group,
                                                                      int flags)
{
    mAudioPolicyService->onAudioVolumeGroupChanged(group, flags);
}

audio_unique_id_t AudioPolicyService::AudioPolicyClient::newAudioUniqueId(audio_unique_id_use_t use)
{
    return AudioSystem::newAudioUniqueId(use);
}

} // namespace android
