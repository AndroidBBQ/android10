/*
**
** Copyright 2009, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "IAudioPolicyService"
#include <utils/Log.h>

#include <stdint.h>
#include <math.h>
#include <sys/types.h>

#include <binder/IPCThreadState.h>
#include <binder/Parcel.h>
#include <media/AudioEffect.h>
#include <media/IAudioPolicyService.h>
#include <media/TimeCheck.h>
#include <mediautils/ServiceUtilities.h>
#include <system/audio.h>

namespace android {

enum {
    SET_DEVICE_CONNECTION_STATE = IBinder::FIRST_CALL_TRANSACTION,
    GET_DEVICE_CONNECTION_STATE,
    HANDLE_DEVICE_CONFIG_CHANGE,
    SET_PHONE_STATE,
    SET_RINGER_MODE,    // reserved, no longer used
    SET_FORCE_USE,
    GET_FORCE_USE,
    GET_OUTPUT,
    START_OUTPUT,
    STOP_OUTPUT,
    RELEASE_OUTPUT,
    GET_INPUT_FOR_ATTR,
    START_INPUT,
    STOP_INPUT,
    RELEASE_INPUT,
    INIT_STREAM_VOLUME,
    SET_STREAM_VOLUME,
    GET_STREAM_VOLUME,
    SET_VOLUME_ATTRIBUTES,
    GET_VOLUME_ATTRIBUTES,
    GET_MIN_VOLUME_FOR_ATTRIBUTES,
    GET_MAX_VOLUME_FOR_ATTRIBUTES,
    GET_STRATEGY_FOR_STREAM,
    GET_OUTPUT_FOR_EFFECT,
    REGISTER_EFFECT,
    UNREGISTER_EFFECT,
    IS_STREAM_ACTIVE,
    IS_SOURCE_ACTIVE,
    GET_DEVICES_FOR_STREAM,
    QUERY_DEFAULT_PRE_PROCESSING,
    SET_EFFECT_ENABLED,
    IS_STREAM_ACTIVE_REMOTELY,
    IS_OFFLOAD_SUPPORTED,
    IS_DIRECT_OUTPUT_SUPPORTED,
    LIST_AUDIO_PORTS,
    GET_AUDIO_PORT,
    CREATE_AUDIO_PATCH,
    RELEASE_AUDIO_PATCH,
    LIST_AUDIO_PATCHES,
    SET_AUDIO_PORT_CONFIG,
    REGISTER_CLIENT,
    GET_OUTPUT_FOR_ATTR,
    ACQUIRE_SOUNDTRIGGER_SESSION,
    RELEASE_SOUNDTRIGGER_SESSION,
    GET_PHONE_STATE,
    REGISTER_POLICY_MIXES,
    START_AUDIO_SOURCE,
    STOP_AUDIO_SOURCE,
    SET_AUDIO_PORT_CALLBACK_ENABLED,
    SET_AUDIO_VOLUME_GROUP_CALLBACK_ENABLED,
    SET_MASTER_MONO,
    GET_MASTER_MONO,
    GET_STREAM_VOLUME_DB,
    GET_SURROUND_FORMATS,
    SET_SURROUND_FORMAT_ENABLED,
    ADD_STREAM_DEFAULT_EFFECT,
    REMOVE_STREAM_DEFAULT_EFFECT,
    ADD_SOURCE_DEFAULT_EFFECT,
    REMOVE_SOURCE_DEFAULT_EFFECT,
    SET_ASSISTANT_UID,
    SET_A11Y_SERVICES_UIDS,
    IS_HAPTIC_PLAYBACK_SUPPORTED,
    SET_UID_DEVICE_AFFINITY,
    REMOVE_UID_DEVICE_AFFINITY,
    GET_OFFLOAD_FORMATS_A2DP,
    LIST_AUDIO_PRODUCT_STRATEGIES,
    GET_STRATEGY_FOR_ATTRIBUTES,
    LIST_AUDIO_VOLUME_GROUPS,
    GET_VOLUME_GROUP_FOR_ATTRIBUTES,
    SET_ALLOWED_CAPTURE_POLICY,
    MOVE_EFFECTS_TO_IO,
    SET_RTT_ENABLED
};

#define MAX_ITEMS_PER_LIST 1024

class BpAudioPolicyService : public BpInterface<IAudioPolicyService>
{
public:
    explicit BpAudioPolicyService(const sp<IBinder>& impl)
        : BpInterface<IAudioPolicyService>(impl)
    {
    }

    virtual status_t setDeviceConnectionState(
                                    audio_devices_t device,
                                    audio_policy_dev_state_t state,
                                    const char *device_address,
                                    const char *device_name,
                                    audio_format_t encodedFormat)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(device));
        data.writeInt32(static_cast <uint32_t>(state));
        data.writeCString(device_address);
        data.writeCString(device_name);
        data.writeInt32(static_cast <uint32_t>(encodedFormat));
        remote()->transact(SET_DEVICE_CONNECTION_STATE, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual audio_policy_dev_state_t getDeviceConnectionState(
                                    audio_devices_t device,
                                    const char *device_address)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(device));
        data.writeCString(device_address);
        remote()->transact(GET_DEVICE_CONNECTION_STATE, data, &reply);
        return static_cast <audio_policy_dev_state_t>(reply.readInt32());
    }

    virtual status_t handleDeviceConfigChange(audio_devices_t device,
                                              const char *device_address,
                                              const char *device_name,
                                              audio_format_t encodedFormat)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(device));
        data.writeCString(device_address);
        data.writeCString(device_name);
        data.writeInt32(static_cast <uint32_t>(encodedFormat));
        remote()->transact(HANDLE_DEVICE_CONFIG_CHANGE, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t setPhoneState(audio_mode_t state)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(state);
        remote()->transact(SET_PHONE_STATE, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t setForceUse(audio_policy_force_use_t usage, audio_policy_forced_cfg_t config)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(usage));
        data.writeInt32(static_cast <uint32_t>(config));
        remote()->transact(SET_FORCE_USE, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual audio_policy_forced_cfg_t getForceUse(audio_policy_force_use_t usage)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(usage));
        remote()->transact(GET_FORCE_USE, data, &reply);
        return static_cast <audio_policy_forced_cfg_t> (reply.readInt32());
    }

    virtual audio_io_handle_t getOutput(audio_stream_type_t stream)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(stream));
        remote()->transact(GET_OUTPUT, data, &reply);
        return static_cast <audio_io_handle_t> (reply.readInt32());
    }

    status_t getOutputForAttr(audio_attributes_t *attr,
                              audio_io_handle_t *output,
                              audio_session_t session,
                              audio_stream_type_t *stream,
                              pid_t pid,
                              uid_t uid,
                              const audio_config_t *config,
                              audio_output_flags_t flags,
                              audio_port_handle_t *selectedDeviceId,
                              audio_port_handle_t *portId,
                              std::vector<audio_io_handle_t> *secondaryOutputs) override
        {
            Parcel data, reply;
            data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
            if (attr == nullptr) {
                ALOGE("%s NULL audio attributes", __func__);
                return BAD_VALUE;
            }
            if (output == nullptr) {
                ALOGE("%s NULL output - shouldn't happen", __func__);
                return BAD_VALUE;
            }
            if (selectedDeviceId == nullptr) {
                ALOGE("%s NULL selectedDeviceId - shouldn't happen", __func__);
                return BAD_VALUE;
            }
            if (portId == nullptr) {
                ALOGE("%s NULL portId - shouldn't happen", __func__);
                return BAD_VALUE;
            }
            if (secondaryOutputs == nullptr) {
                ALOGE("%s NULL secondaryOutputs - shouldn't happen", __func__);
                return BAD_VALUE;
            }
            data.write(attr, sizeof(audio_attributes_t));
            data.writeInt32(session);
            if (stream == NULL) {
                data.writeInt32(0);
            } else {
                data.writeInt32(1);
                data.writeInt32(*stream);
            }
            data.writeInt32(pid);
            data.writeInt32(uid);
            data.write(config, sizeof(audio_config_t));
            data.writeInt32(static_cast <uint32_t>(flags));
            data.writeInt32(*selectedDeviceId);
            data.writeInt32(*portId);
            status_t status = remote()->transact(GET_OUTPUT_FOR_ATTR, data, &reply);
            if (status != NO_ERROR) {
                return status;
            }
            status = (status_t)reply.readInt32();
            if (status != NO_ERROR) {
                return status;
            }
            status = (status_t)reply.read(&attr, sizeof(audio_attributes_t));
            if (status != NO_ERROR) {
                return status;
            }
            *output = (audio_io_handle_t)reply.readInt32();
            audio_stream_type_t lStream = (audio_stream_type_t)reply.readInt32();
            if (stream != NULL) {
                *stream = lStream;
            }
            *selectedDeviceId = (audio_port_handle_t)reply.readInt32();
            *portId = (audio_port_handle_t)reply.readInt32();
            secondaryOutputs->resize(reply.readInt32());
            return reply.read(secondaryOutputs->data(),
                              secondaryOutputs->size() * sizeof(audio_io_handle_t));
        }

    virtual status_t startOutput(audio_port_handle_t portId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32((int32_t)portId);
        remote()->transact(START_OUTPUT, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t stopOutput(audio_port_handle_t portId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32((int32_t)portId);
        remote()->transact(STOP_OUTPUT, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual void releaseOutput(audio_port_handle_t portId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32((int32_t)portId);
        remote()->transact(RELEASE_OUTPUT, data, &reply);
    }

    virtual status_t getInputForAttr(const audio_attributes_t *attr,
                                     audio_io_handle_t *input,
                                     audio_unique_id_t riid,
                                     audio_session_t session,
                                     pid_t pid,
                                     uid_t uid,
                                     const String16& opPackageName,
                                     const audio_config_base_t *config,
                                     audio_input_flags_t flags,
                                     audio_port_handle_t *selectedDeviceId,
                                     audio_port_handle_t *portId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        if (attr == NULL) {
            ALOGE("getInputForAttr NULL attr - shouldn't happen");
            return BAD_VALUE;
        }
        if (input == NULL) {
            ALOGE("getInputForAttr NULL input - shouldn't happen");
            return BAD_VALUE;
        }
        if (selectedDeviceId == NULL) {
            ALOGE("getInputForAttr NULL selectedDeviceId - shouldn't happen");
            return BAD_VALUE;
        }
        if (portId == NULL) {
            ALOGE("getInputForAttr NULL portId - shouldn't happen");
            return BAD_VALUE;
        }
        data.write(attr, sizeof(audio_attributes_t));
        data.writeInt32(*input);
        data.writeInt32(riid);
        data.writeInt32(session);
        data.writeInt32(pid);
        data.writeInt32(uid);
        data.writeString16(opPackageName);
        data.write(config, sizeof(audio_config_base_t));
        data.writeInt32(flags);
        data.writeInt32(*selectedDeviceId);
        data.writeInt32(*portId);
        status_t status = remote()->transact(GET_INPUT_FOR_ATTR, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = reply.readInt32();
        if (status != NO_ERROR) {
            return status;
        }
        *input = (audio_io_handle_t)reply.readInt32();
        *selectedDeviceId = (audio_port_handle_t)reply.readInt32();
        *portId = (audio_port_handle_t)reply.readInt32();
        return NO_ERROR;
    }

    virtual status_t startInput(audio_port_handle_t portId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(portId);
        remote()->transact(START_INPUT, data, &reply);
        status_t status = static_cast <status_t> (reply.readInt32());
        return status;
    }

    virtual status_t stopInput(audio_port_handle_t portId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(portId);
        remote()->transact(STOP_INPUT, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual void releaseInput(audio_port_handle_t portId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(portId);
        remote()->transact(RELEASE_INPUT, data, &reply);
    }

    virtual status_t initStreamVolume(audio_stream_type_t stream,
                                    int indexMin,
                                    int indexMax)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(stream));
        data.writeInt32(indexMin);
        data.writeInt32(indexMax);
        remote()->transact(INIT_STREAM_VOLUME, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t setStreamVolumeIndex(audio_stream_type_t stream,
                                          int index,
                                          audio_devices_t device)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(stream));
        data.writeInt32(index);
        data.writeInt32(static_cast <uint32_t>(device));
        remote()->transact(SET_STREAM_VOLUME, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t getStreamVolumeIndex(audio_stream_type_t stream,
                                          int *index,
                                          audio_devices_t device)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(stream));
        data.writeInt32(static_cast <uint32_t>(device));

        remote()->transact(GET_STREAM_VOLUME, data, &reply);
        int lIndex = reply.readInt32();
        if (index) *index = lIndex;
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t setVolumeIndexForAttributes(const audio_attributes_t &attr, int index,
                                                 audio_devices_t device)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(&attr, sizeof(audio_attributes_t));
        data.writeInt32(index);
        data.writeInt32(static_cast <uint32_t>(device));
        status_t status = remote()->transact(SET_VOLUME_ATTRIBUTES, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        return static_cast <status_t> (reply.readInt32());
    }
    virtual status_t getVolumeIndexForAttributes(const audio_attributes_t &attr, int &index,
                                                 audio_devices_t device)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(&attr, sizeof(audio_attributes_t));
        data.writeInt32(static_cast <uint32_t>(device));
        status_t status = remote()->transact(GET_VOLUME_ATTRIBUTES, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = static_cast <status_t> (reply.readInt32());
        if (status != NO_ERROR) {
            return status;
        }
        index = reply.readInt32();
        return NO_ERROR;
    }
    virtual status_t getMinVolumeIndexForAttributes(const audio_attributes_t &attr, int &index)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(&attr, sizeof(audio_attributes_t));
        status_t status = remote()->transact(GET_MIN_VOLUME_FOR_ATTRIBUTES, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = static_cast <status_t> (reply.readInt32());
        if (status != NO_ERROR) {
            return status;
        }
        index = reply.readInt32();
        return NO_ERROR;
    }
    virtual status_t getMaxVolumeIndexForAttributes(const audio_attributes_t &attr, int &index)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(&attr, sizeof(audio_attributes_t));
        status_t status = remote()->transact(GET_MAX_VOLUME_FOR_ATTRIBUTES, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = static_cast <status_t> (reply.readInt32());
        if (status != NO_ERROR) {
            return status;
        }
        index = reply.readInt32();
        return NO_ERROR;
    }
    virtual uint32_t getStrategyForStream(audio_stream_type_t stream)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(stream));
        remote()->transact(GET_STRATEGY_FOR_STREAM, data, &reply);
        return reply.readUint32();
    }

    virtual audio_devices_t getDevicesForStream(audio_stream_type_t stream)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <uint32_t>(stream));
        remote()->transact(GET_DEVICES_FOR_STREAM, data, &reply);
        return (audio_devices_t) reply.readInt32();
    }

    virtual audio_io_handle_t getOutputForEffect(const effect_descriptor_t *desc)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(desc, sizeof(effect_descriptor_t));
        remote()->transact(GET_OUTPUT_FOR_EFFECT, data, &reply);
        return static_cast <audio_io_handle_t> (reply.readInt32());
    }

    virtual status_t registerEffect(const effect_descriptor_t *desc,
                                        audio_io_handle_t io,
                                        uint32_t strategy,
                                        audio_session_t session,
                                        int id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(desc, sizeof(effect_descriptor_t));
        data.writeInt32(io);
        data.writeInt32(strategy);
        data.writeInt32(session);
        data.writeInt32(id);
        remote()->transact(REGISTER_EFFECT, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t unregisterEffect(int id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(id);
        remote()->transact(UNREGISTER_EFFECT, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t setEffectEnabled(int id, bool enabled)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(id);
        data.writeInt32(enabled);
        remote()->transact(SET_EFFECT_ENABLED, data, &reply);
        return static_cast <status_t> (reply.readInt32());
    }

    status_t moveEffectsToIo(const std::vector<int>& ids, audio_io_handle_t io) override
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(ids.size());
        for (auto id : ids) {
            data.writeInt32(id);
        }
        data.writeInt32(io);
        status_t status = remote()->transact(MOVE_EFFECTS_TO_IO, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        return static_cast <status_t> (reply.readInt32());
    }

    virtual bool isStreamActive(audio_stream_type_t stream, uint32_t inPastMs) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32((int32_t) stream);
        data.writeInt32(inPastMs);
        remote()->transact(IS_STREAM_ACTIVE, data, &reply);
        return reply.readInt32();
    }

    virtual bool isStreamActiveRemotely(audio_stream_type_t stream, uint32_t inPastMs) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32((int32_t) stream);
        data.writeInt32(inPastMs);
        remote()->transact(IS_STREAM_ACTIVE_REMOTELY, data, &reply);
        return reply.readInt32();
    }

    virtual bool isSourceActive(audio_source_t source) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32((int32_t) source);
        remote()->transact(IS_SOURCE_ACTIVE, data, &reply);
        return reply.readInt32();
    }

    virtual status_t queryDefaultPreProcessing(audio_session_t audioSession,
                                               effect_descriptor_t *descriptors,
                                               uint32_t *count)
    {
        if (descriptors == NULL || count == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(audioSession);
        data.writeInt32(*count);
        status_t status = remote()->transact(QUERY_DEFAULT_PRE_PROCESSING, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = static_cast <status_t> (reply.readInt32());
        uint32_t retCount = reply.readInt32();
        if (retCount != 0) {
            uint32_t numDesc = (retCount < *count) ? retCount : *count;
            reply.read(descriptors, sizeof(effect_descriptor_t) * numDesc);
        }
        *count = retCount;
        return status;
    }

    status_t setAllowedCapturePolicy(uid_t uid, audio_flags_mask_t flags) override {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(uid);
        data.writeInt32(flags);
        remote()->transact(SET_ALLOWED_CAPTURE_POLICY, data, &reply);
        return reply.readInt32();
    }

    virtual bool isOffloadSupported(const audio_offload_info_t& info)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(&info, sizeof(audio_offload_info_t));
        remote()->transact(IS_OFFLOAD_SUPPORTED, data, &reply);
        return reply.readInt32();
    }

    virtual bool isDirectOutputSupported(const audio_config_base_t& config,
                                         const audio_attributes_t& attributes) {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(&config, sizeof(audio_config_base_t));
        data.write(&attributes, sizeof(audio_attributes_t));
        status_t status = remote()->transact(IS_DIRECT_OUTPUT_SUPPORTED, data, &reply);
        return status == NO_ERROR ? static_cast<bool>(reply.readInt32()) : false;
    }

    virtual status_t listAudioPorts(audio_port_role_t role,
                                    audio_port_type_t type,
                                    unsigned int *num_ports,
                                    struct audio_port *ports,
                                    unsigned int *generation)
    {
        if (num_ports == NULL || (*num_ports != 0 && ports == NULL) ||
                generation == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        unsigned int numPortsReq = (ports == NULL) ? 0 : *num_ports;
        data.writeInt32(role);
        data.writeInt32(type);
        data.writeInt32(numPortsReq);
        status_t status = remote()->transact(LIST_AUDIO_PORTS, data, &reply);
        if (status == NO_ERROR) {
            status = (status_t)reply.readInt32();
            *num_ports = (unsigned int)reply.readInt32();
        }
        if (status == NO_ERROR) {
            if (numPortsReq > *num_ports) {
                numPortsReq = *num_ports;
            }
            if (numPortsReq > 0) {
                reply.read(ports, numPortsReq * sizeof(struct audio_port));
            }
            *generation = reply.readInt32();
        }
        return status;
    }

    virtual status_t getAudioPort(struct audio_port *port)
    {
        if (port == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(port, sizeof(struct audio_port));
        status_t status = remote()->transact(GET_AUDIO_PORT, data, &reply);
        if (status != NO_ERROR ||
                (status = (status_t)reply.readInt32()) != NO_ERROR) {
            return status;
        }
        reply.read(port, sizeof(struct audio_port));
        return status;
    }

    virtual status_t createAudioPatch(const struct audio_patch *patch,
                                       audio_patch_handle_t *handle)
    {
        if (patch == NULL || handle == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(patch, sizeof(struct audio_patch));
        data.write(handle, sizeof(audio_patch_handle_t));
        status_t status = remote()->transact(CREATE_AUDIO_PATCH, data, &reply);
        if (status != NO_ERROR ||
                (status = (status_t)reply.readInt32()) != NO_ERROR) {
            return status;
        }
        reply.read(handle, sizeof(audio_patch_handle_t));
        return status;
    }

    virtual status_t releaseAudioPatch(audio_patch_handle_t handle)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(&handle, sizeof(audio_patch_handle_t));
        status_t status = remote()->transact(RELEASE_AUDIO_PATCH, data, &reply);
        if (status != NO_ERROR) {
            status = (status_t)reply.readInt32();
        }
        return status;
    }

    virtual status_t listAudioPatches(unsigned int *num_patches,
                                      struct audio_patch *patches,
                                      unsigned int *generation)
    {
        if (num_patches == NULL || (*num_patches != 0 && patches == NULL) ||
                generation == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        unsigned int numPatchesReq = (patches == NULL) ? 0 : *num_patches;
        data.writeInt32(numPatchesReq);
        status_t status = remote()->transact(LIST_AUDIO_PATCHES, data, &reply);
        if (status == NO_ERROR) {
            status = (status_t)reply.readInt32();
            *num_patches = (unsigned int)reply.readInt32();
        }
        if (status == NO_ERROR) {
            if (numPatchesReq > *num_patches) {
                numPatchesReq = *num_patches;
            }
            if (numPatchesReq > 0) {
                reply.read(patches, numPatchesReq * sizeof(struct audio_patch));
            }
            *generation = reply.readInt32();
        }
        return status;
    }

    virtual status_t setAudioPortConfig(const struct audio_port_config *config)
    {
        if (config == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(config, sizeof(struct audio_port_config));
        status_t status = remote()->transact(SET_AUDIO_PORT_CONFIG, data, &reply);
        if (status != NO_ERROR) {
            status = (status_t)reply.readInt32();
        }
        return status;
    }

    virtual void registerClient(const sp<IAudioPolicyServiceClient>& client)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(client));
        remote()->transact(REGISTER_CLIENT, data, &reply);
    }

    virtual void setAudioPortCallbacksEnabled(bool enabled)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(enabled ? 1 : 0);
        remote()->transact(SET_AUDIO_PORT_CALLBACK_ENABLED, data, &reply);
    }

    virtual void setAudioVolumeGroupCallbacksEnabled(bool enabled)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(enabled ? 1 : 0);
        remote()->transact(SET_AUDIO_VOLUME_GROUP_CALLBACK_ENABLED, data, &reply);
    }

    virtual status_t acquireSoundTriggerSession(audio_session_t *session,
                                            audio_io_handle_t *ioHandle,
                                            audio_devices_t *device)
    {
        if (session == NULL || ioHandle == NULL || device == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        status_t status = remote()->transact(ACQUIRE_SOUNDTRIGGER_SESSION, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = (status_t)reply.readInt32();
        if (status == NO_ERROR) {
            *session = (audio_session_t)reply.readInt32();
            *ioHandle = (audio_io_handle_t)reply.readInt32();
            *device = (audio_devices_t)reply.readInt32();
        }
        return status;
    }

    virtual status_t releaseSoundTriggerSession(audio_session_t session)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(session);
        status_t status = remote()->transact(RELEASE_SOUNDTRIGGER_SESSION, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        return (status_t)reply.readInt32();
    }

    virtual audio_mode_t getPhoneState()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        status_t status = remote()->transact(GET_PHONE_STATE, data, &reply);
        if (status != NO_ERROR) {
            return AUDIO_MODE_INVALID;
        }
        return (audio_mode_t)reply.readInt32();
    }

    virtual status_t registerPolicyMixes(const Vector<AudioMix>& mixes, bool registration)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(registration ? 1 : 0);
        size_t size = mixes.size();
        if (size > MAX_MIXES_PER_POLICY) {
            size = MAX_MIXES_PER_POLICY;
        }
        size_t sizePosition = data.dataPosition();
        data.writeInt32(size);
        size_t finalSize = size;
        for (size_t i = 0; i < size; i++) {
            size_t position = data.dataPosition();
            if (mixes[i].writeToParcel(&data) != NO_ERROR) {
                data.setDataPosition(position);
                finalSize--;
            }
        }
        if (size != finalSize) {
            size_t position = data.dataPosition();
            data.setDataPosition(sizePosition);
            data.writeInt32(finalSize);
            data.setDataPosition(position);
        }
        status_t status = remote()->transact(REGISTER_POLICY_MIXES, data, &reply);
        if (status == NO_ERROR) {
            status = (status_t)reply.readInt32();
        }
        return status;
    }

    virtual status_t startAudioSource(const struct audio_port_config *source,
                                      const audio_attributes_t *attributes,
                                      audio_port_handle_t *portId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        if (source == NULL || attributes == NULL || portId == NULL) {
            return BAD_VALUE;
        }
        data.write(source, sizeof(struct audio_port_config));
        data.write(attributes, sizeof(audio_attributes_t));
        status_t status = remote()->transact(START_AUDIO_SOURCE, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = (status_t)reply.readInt32();
        if (status != NO_ERROR) {
            return status;
        }
        *portId = (audio_port_handle_t)reply.readInt32();
        return status;
    }

    virtual status_t stopAudioSource(audio_port_handle_t portId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(portId);
        status_t status = remote()->transact(STOP_AUDIO_SOURCE, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = (status_t)reply.readInt32();
        return status;
    }

    virtual status_t setMasterMono(bool mono)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast<int32_t>(mono));
        status_t status = remote()->transact(SET_MASTER_MONO, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        return static_cast<status_t>(reply.readInt32());
    }

    virtual status_t getMasterMono(bool *mono)
    {
        if (mono == nullptr) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());

        status_t status = remote()->transact(GET_MASTER_MONO, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = static_cast<status_t>(reply.readInt32());
        if (status == NO_ERROR) {
            *mono = static_cast<bool>(reply.readInt32());
        }
        return status;
    }

    virtual float getStreamVolumeDB(audio_stream_type_t stream, int index, audio_devices_t device)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast <int32_t>(stream));
        data.writeInt32(static_cast <int32_t>(index));
        data.writeUint32(static_cast <uint32_t>(device));
        status_t status = remote()->transact(GET_STREAM_VOLUME_DB, data, &reply);
        if (status != NO_ERROR) {
            return NAN;
        }
        return reply.readFloat();
    }

    virtual status_t getSurroundFormats(unsigned int *numSurroundFormats,
                                        audio_format_t *surroundFormats,
                                        bool *surroundFormatsEnabled,
                                        bool reported)
    {
        if (numSurroundFormats == NULL || (*numSurroundFormats != 0 &&
                (surroundFormats == NULL || surroundFormatsEnabled == NULL))) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        unsigned int numSurroundFormatsReq = *numSurroundFormats;
        data.writeUint32(numSurroundFormatsReq);
        data.writeBool(reported);
        status_t status = remote()->transact(GET_SURROUND_FORMATS, data, &reply);
        if (status == NO_ERROR && (status = (status_t)reply.readInt32()) == NO_ERROR) {
            *numSurroundFormats = reply.readUint32();
        }
        if (status == NO_ERROR) {
            if (numSurroundFormatsReq > *numSurroundFormats) {
                numSurroundFormatsReq = *numSurroundFormats;
            }
            if (numSurroundFormatsReq > 0) {
                status = reply.read(surroundFormats,
                                    numSurroundFormatsReq * sizeof(audio_format_t));
                if (status != NO_ERROR) {
                    return status;
                }
                status = reply.read(surroundFormatsEnabled,
                                    numSurroundFormatsReq * sizeof(bool));
            }
        }
        return status;
    }

    virtual status_t setSurroundFormatEnabled(audio_format_t audioFormat, bool enabled)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(audioFormat);
        data.writeBool(enabled);
        status_t status = remote()->transact(SET_SURROUND_FORMAT_ENABLED, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        return reply.readInt32();
    }

    virtual status_t getHwOffloadEncodingFormatsSupportedForA2DP(
                std::vector<audio_format_t> *formats)
    {
        if (formats == NULL) {
            return BAD_VALUE;
        }

        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        status_t status = remote()->transact(GET_OFFLOAD_FORMATS_A2DP, data, &reply);
        if (status != NO_ERROR || (status = (status_t)reply.readInt32()) != NO_ERROR) {
            return status;
        }

        size_t list_size = reply.readUint32();

        for (size_t i = 0; i < list_size; i++) {
            formats->push_back(static_cast<audio_format_t>(reply.readInt32()));
        }
        return NO_ERROR;
    }


     virtual status_t addStreamDefaultEffect(const effect_uuid_t *type,
                                            const String16& opPackageName,
                                            const effect_uuid_t *uuid,
                                            int32_t priority,
                                            audio_usage_t usage,
                                            audio_unique_id_t* id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(type, sizeof(effect_uuid_t));
        data.writeString16(opPackageName);
        data.write(uuid, sizeof(effect_uuid_t));
        data.writeInt32(priority);
        data.writeInt32((int32_t) usage);
        status_t status = remote()->transact(ADD_STREAM_DEFAULT_EFFECT, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = static_cast <status_t> (reply.readInt32());
        *id = reply.readInt32();
        return status;
    }

    virtual status_t removeStreamDefaultEffect(audio_unique_id_t id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(id);
        status_t status = remote()->transact(REMOVE_STREAM_DEFAULT_EFFECT, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t addSourceDefaultEffect(const effect_uuid_t *type,
                                            const String16& opPackageName,
                                            const effect_uuid_t *uuid,
                                            int32_t priority,
                                            audio_source_t source,
                                            audio_unique_id_t* id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.write(type, sizeof(effect_uuid_t));
        data.writeString16(opPackageName);
        data.write(uuid, sizeof(effect_uuid_t));
        data.writeInt32(priority);
        data.writeInt32((int32_t) source);
        status_t status = remote()->transact(ADD_SOURCE_DEFAULT_EFFECT, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = static_cast <status_t> (reply.readInt32());
        *id = reply.readInt32();
        return status;
    }

    virtual status_t removeSourceDefaultEffect(audio_unique_id_t id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(id);
        status_t status = remote()->transact(REMOVE_SOURCE_DEFAULT_EFFECT, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t setAssistantUid(uid_t uid)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(uid);
        status_t status = remote()->transact(SET_ASSISTANT_UID, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        return static_cast <status_t> (reply.readInt32());
    }

    virtual status_t setA11yServicesUids(const std::vector<uid_t>& uids)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(uids.size());
        for (auto uid : uids) {
            data.writeInt32(uid);
        }
        status_t status = remote()->transact(SET_A11Y_SERVICES_UIDS, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        return static_cast <status_t> (reply.readInt32());
    }

    virtual bool isHapticPlaybackSupported()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        status_t status = remote()->transact(IS_HAPTIC_PLAYBACK_SUPPORTED, data, &reply);
        if (status != NO_ERROR) {
            return false;
        }
        return reply.readBool();
    }

    virtual status_t setUidDeviceAffinities(uid_t uid, const Vector<AudioDeviceTypeAddr>& devices)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());

        data.writeInt32((int32_t) uid);
        size_t size = devices.size();
        size_t sizePosition = data.dataPosition();
        data.writeInt32((int32_t) size);
        size_t finalSize = size;
        for (size_t i = 0; i < size; i++) {
            size_t position = data.dataPosition();
            if (devices[i].writeToParcel(&data) != NO_ERROR) {
                data.setDataPosition(position);
                finalSize--;
            }
        }
        if (size != finalSize) {
            size_t position = data.dataPosition();
            data.setDataPosition(sizePosition);
            data.writeInt32(finalSize);
            data.setDataPosition(position);
        }

        status_t status = remote()->transact(SET_UID_DEVICE_AFFINITY, data, &reply);
        if (status == NO_ERROR) {
            status = (status_t)reply.readInt32();
        }
        return status;
    }

    virtual status_t removeUidDeviceAffinities(uid_t uid) {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());

        data.writeInt32((int32_t) uid);

        status_t status =
            remote()->transact(REMOVE_UID_DEVICE_AFFINITY, data, &reply);
        if (status == NO_ERROR) {
            status = (status_t) reply.readInt32();
        }
        return status;
    }

    virtual status_t listAudioProductStrategies(AudioProductStrategyVector &strategies)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());

        status_t status = remote()->transact(LIST_AUDIO_PRODUCT_STRATEGIES, data, &reply);
        if (status != NO_ERROR) {
            ALOGE("%s: permission denied", __func__);
            return status;
        }
        status = static_cast<status_t>(reply.readInt32());
        if (status != NO_ERROR) {
            return status;
        }
        uint32_t numStrategies = static_cast<uint32_t>(reply.readInt32());
        for (size_t i = 0; i < numStrategies; i++) {
            AudioProductStrategy strategy;
            status = strategy.readFromParcel(&reply);
            if (status != NO_ERROR) {
                ALOGE("%s: failed to read strategies", __FUNCTION__);
                strategies.clear();
                return status;
            }
            strategies.push_back(strategy);
        }
        return NO_ERROR;
    }

    virtual status_t getProductStrategyFromAudioAttributes(const AudioAttributes &aa,
                                                           product_strategy_t &productStrategy)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        status_t status = aa.writeToParcel(&data);
        if (status != NO_ERROR) {
            return status;
        }
        status = remote()->transact(GET_STRATEGY_FOR_ATTRIBUTES, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = static_cast<status_t>(reply.readInt32());
        if (status != NO_ERROR) {
            return status;
        }
        productStrategy = static_cast<product_strategy_t>(reply.readInt32());
        return NO_ERROR;
    }

    virtual status_t listAudioVolumeGroups(AudioVolumeGroupVector &groups)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());

        status_t status = remote()->transact(LIST_AUDIO_VOLUME_GROUPS, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = static_cast<status_t>(reply.readInt32());
        if (status != NO_ERROR) {
            return status;
        }
        uint32_t numGroups = static_cast<uint32_t>(reply.readInt32());
        for (size_t i = 0; i < numGroups; i++) {
            AudioVolumeGroup group;
            status = group.readFromParcel(&reply);
            if (status != NO_ERROR) {
                ALOGE("%s: failed to read volume groups", __FUNCTION__);
                groups.clear();
                return status;
            }
            groups.push_back(group);
        }
        return NO_ERROR;
    }

    virtual status_t getVolumeGroupFromAudioAttributes(const AudioAttributes &aa,
                                                       volume_group_t &volumeGroup)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        status_t status = aa.writeToParcel(&data);
        if (status != NO_ERROR) {
            return status;
        }
        status = remote()->transact(GET_VOLUME_GROUP_FOR_ATTRIBUTES, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = static_cast<status_t>(reply.readInt32());
        if (status != NO_ERROR) {
            return status;
        }
        volumeGroup = static_cast<volume_group_t>(reply.readInt32());
        return NO_ERROR;
    }

    virtual status_t setRttEnabled(bool enabled)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioPolicyService::getInterfaceDescriptor());
        data.writeInt32(static_cast<int32_t>(enabled));
        status_t status = remote()->transact(SET_RTT_ENABLED, data, &reply);
        if (status != NO_ERROR) {
           return status;
        }
        return static_cast<status_t>(reply.readInt32());
    }
};

IMPLEMENT_META_INTERFACE(AudioPolicyService, "android.media.IAudioPolicyService");

// ----------------------------------------------------------------------

status_t BnAudioPolicyService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    // make sure transactions reserved to AudioFlinger do not come from other processes
    switch (code) {
        case START_OUTPUT:
        case STOP_OUTPUT:
        case RELEASE_OUTPUT:
        case GET_INPUT_FOR_ATTR:
        case START_INPUT:
        case STOP_INPUT:
        case RELEASE_INPUT:
        case GET_OUTPUT_FOR_EFFECT:
        case REGISTER_EFFECT:
        case UNREGISTER_EFFECT:
        case SET_EFFECT_ENABLED:
        case GET_OUTPUT_FOR_ATTR:
        case ACQUIRE_SOUNDTRIGGER_SESSION:
        case RELEASE_SOUNDTRIGGER_SESSION:
        case MOVE_EFFECTS_TO_IO:
            ALOGW("%s: transaction %d received from PID %d",
                  __func__, code, IPCThreadState::self()->getCallingPid());
            // return status only for non void methods
            switch (code) {
                case RELEASE_OUTPUT:
                case RELEASE_INPUT:
                    break;
                default:
                    reply->writeInt32(static_cast<int32_t> (INVALID_OPERATION));
                    break;
            }
            return OK;
        default:
            break;
    }

    // make sure the following transactions come from system components
    switch (code) {
        case SET_DEVICE_CONNECTION_STATE:
        case HANDLE_DEVICE_CONFIG_CHANGE:
        case SET_PHONE_STATE:
//FIXME: Allow SET_FORCE_USE calls from system apps until a better use case routing API is available
//      case SET_FORCE_USE:
        case INIT_STREAM_VOLUME:
        case SET_STREAM_VOLUME:
        case REGISTER_POLICY_MIXES:
        case SET_MASTER_MONO:
        case GET_SURROUND_FORMATS:
        case SET_SURROUND_FORMAT_ENABLED:
        case SET_ASSISTANT_UID:
        case SET_A11Y_SERVICES_UIDS:
        case SET_UID_DEVICE_AFFINITY:
        case REMOVE_UID_DEVICE_AFFINITY:
        case GET_OFFLOAD_FORMATS_A2DP:
        case LIST_AUDIO_VOLUME_GROUPS:
        case GET_VOLUME_GROUP_FOR_ATTRIBUTES:
        case SET_RTT_ENABLED:
        case SET_ALLOWED_CAPTURE_POLICY: {
            if (!isServiceUid(IPCThreadState::self()->getCallingUid())) {
                ALOGW("%s: transaction %d received from PID %d unauthorized UID %d",
                      __func__, code, IPCThreadState::self()->getCallingPid(),
                      IPCThreadState::self()->getCallingUid());
                reply->writeInt32(static_cast<int32_t> (INVALID_OPERATION));
                return OK;
            }
        } break;
        default:
            break;
    }

    std::string tag("IAudioPolicyService command " + std::to_string(code));
    TimeCheck check(tag.c_str());

    switch (code) {
        case SET_DEVICE_CONNECTION_STATE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_devices_t device =
                    static_cast <audio_devices_t>(data.readInt32());
            audio_policy_dev_state_t state =
                    static_cast <audio_policy_dev_state_t>(data.readInt32());
            const char *device_address = data.readCString();
            const char *device_name = data.readCString();
            audio_format_t codecFormat = static_cast <audio_format_t>(data.readInt32());
            if (device_address == nullptr || device_name == nullptr) {
                ALOGE("Bad Binder transaction: SET_DEVICE_CONNECTION_STATE for device %u", device);
                reply->writeInt32(static_cast<int32_t> (BAD_VALUE));
            } else {
                reply->writeInt32(static_cast<uint32_t> (setDeviceConnectionState(device,
                                                                                  state,
                                                                                  device_address,
                                                                                  device_name,
                                                                                  codecFormat)));
            }
            return NO_ERROR;
        } break;

        case GET_DEVICE_CONNECTION_STATE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_devices_t device =
                    static_cast<audio_devices_t> (data.readInt32());
            const char *device_address = data.readCString();
            if (device_address == nullptr) {
                ALOGE("Bad Binder transaction: GET_DEVICE_CONNECTION_STATE for device %u", device);
                reply->writeInt32(static_cast<int32_t> (AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE));
            } else {
                reply->writeInt32(static_cast<uint32_t> (getDeviceConnectionState(device,
                                                                                  device_address)));
            }
            return NO_ERROR;
        } break;

        case HANDLE_DEVICE_CONFIG_CHANGE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_devices_t device =
                    static_cast <audio_devices_t>(data.readInt32());
            const char *device_address = data.readCString();
            const char *device_name = data.readCString();
            audio_format_t codecFormat =
                    static_cast <audio_format_t>(data.readInt32());
            if (device_address == nullptr || device_name == nullptr) {
                ALOGE("Bad Binder transaction: HANDLE_DEVICE_CONFIG_CHANGE for device %u", device);
                reply->writeInt32(static_cast<int32_t> (BAD_VALUE));
            } else {
                reply->writeInt32(static_cast<uint32_t> (handleDeviceConfigChange(device,
                                                                                  device_address,
                                                                                  device_name,
                                                                                  codecFormat)));
            }
            return NO_ERROR;
        } break;

        case SET_PHONE_STATE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            reply->writeInt32(static_cast <uint32_t>(setPhoneState(
                    (audio_mode_t) data.readInt32())));
            return NO_ERROR;
        } break;

        case SET_FORCE_USE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_policy_force_use_t usage = static_cast <audio_policy_force_use_t>(
                    data.readInt32());
            audio_policy_forced_cfg_t config =
                    static_cast <audio_policy_forced_cfg_t>(data.readInt32());
            reply->writeInt32(static_cast <uint32_t>(setForceUse(usage, config)));
            return NO_ERROR;
        } break;

        case GET_FORCE_USE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_policy_force_use_t usage = static_cast <audio_policy_force_use_t>(
                    data.readInt32());
            reply->writeInt32(static_cast <uint32_t>(getForceUse(usage)));
            return NO_ERROR;
        } break;

        case GET_OUTPUT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_stream_type_t stream =
                    static_cast <audio_stream_type_t>(data.readInt32());
            audio_io_handle_t output = getOutput(stream);
            reply->writeInt32(static_cast <int>(output));
            return NO_ERROR;
        } break;

        case GET_OUTPUT_FOR_ATTR: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_attributes_t attr = AUDIO_ATTRIBUTES_INITIALIZER;
            status_t status = data.read(&attr, sizeof(audio_attributes_t));
            if (status != NO_ERROR) {
                return status;
            }
            sanetizeAudioAttributes(&attr);
            audio_session_t session = (audio_session_t)data.readInt32();
            audio_stream_type_t stream = AUDIO_STREAM_DEFAULT;
            bool hasStream = data.readInt32() != 0;
            if (hasStream) {
                stream = (audio_stream_type_t)data.readInt32();
            }
            pid_t pid = (pid_t)data.readInt32();
            uid_t uid = (uid_t)data.readInt32();
            audio_config_t config;
            memset(&config, 0, sizeof(audio_config_t));
            data.read(&config, sizeof(audio_config_t));
            audio_output_flags_t flags =
                    static_cast <audio_output_flags_t>(data.readInt32());
            audio_port_handle_t selectedDeviceId = data.readInt32();
            audio_port_handle_t portId = (audio_port_handle_t)data.readInt32();
            audio_io_handle_t output = 0;
            std::vector<audio_io_handle_t> secondaryOutputs;
            status = getOutputForAttr(&attr,
                    &output, session, &stream, pid, uid,
                    &config,
                    flags, &selectedDeviceId, &portId, &secondaryOutputs);
            reply->writeInt32(status);
            status = reply->write(&attr, sizeof(audio_attributes_t));
            if (status != NO_ERROR) {
                return status;
            }
            reply->writeInt32(output);
            reply->writeInt32(stream);
            reply->writeInt32(selectedDeviceId);
            reply->writeInt32(portId);
            reply->writeInt32(secondaryOutputs.size());
            return reply->write(secondaryOutputs.data(),
                                secondaryOutputs.size() * sizeof(audio_io_handle_t));
        } break;

        case START_OUTPUT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            const audio_port_handle_t portId = static_cast <audio_port_handle_t>(data.readInt32());
            reply->writeInt32(static_cast <uint32_t>(startOutput(portId)));
            return NO_ERROR;
        } break;

        case STOP_OUTPUT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            const audio_port_handle_t portId = static_cast <audio_port_handle_t>(data.readInt32());
            reply->writeInt32(static_cast <uint32_t>(stopOutput(portId)));
            return NO_ERROR;
        } break;

        case RELEASE_OUTPUT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            const audio_port_handle_t portId = static_cast <audio_port_handle_t>(data.readInt32());
            releaseOutput(portId);
            return NO_ERROR;
        } break;

        case GET_INPUT_FOR_ATTR: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_attributes_t attr = {};
            data.read(&attr, sizeof(audio_attributes_t));
            sanetizeAudioAttributes(&attr);
            audio_io_handle_t input = (audio_io_handle_t)data.readInt32();
            audio_unique_id_t riid = (audio_unique_id_t)data.readInt32();
            audio_session_t session = (audio_session_t)data.readInt32();
            pid_t pid = (pid_t)data.readInt32();
            uid_t uid = (uid_t)data.readInt32();
            const String16 opPackageName = data.readString16();
            audio_config_base_t config;
            memset(&config, 0, sizeof(audio_config_base_t));
            data.read(&config, sizeof(audio_config_base_t));
            audio_input_flags_t flags = (audio_input_flags_t) data.readInt32();
            audio_port_handle_t selectedDeviceId = (audio_port_handle_t) data.readInt32();
            audio_port_handle_t portId = (audio_port_handle_t)data.readInt32();
            status_t status = getInputForAttr(&attr, &input, riid, session, pid, uid,
                                              opPackageName, &config,
                                              flags, &selectedDeviceId, &portId);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->writeInt32(input);
                reply->writeInt32(selectedDeviceId);
                reply->writeInt32(portId);
            }
            return NO_ERROR;
        } break;

        case START_INPUT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_port_handle_t portId = static_cast <audio_port_handle_t>(data.readInt32());
            status_t status = startInput(portId);
            reply->writeInt32(static_cast <uint32_t>(status));
            return NO_ERROR;
        } break;

        case STOP_INPUT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_port_handle_t portId = static_cast <audio_port_handle_t>(data.readInt32());
            reply->writeInt32(static_cast <uint32_t>(stopInput(portId)));
            return NO_ERROR;
        } break;

        case RELEASE_INPUT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_port_handle_t portId = static_cast <audio_port_handle_t>(data.readInt32());
            releaseInput(portId);
            return NO_ERROR;
        } break;

        case INIT_STREAM_VOLUME: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_stream_type_t stream =
                    static_cast <audio_stream_type_t>(data.readInt32());
            int indexMin = data.readInt32();
            int indexMax = data.readInt32();
            reply->writeInt32(static_cast <uint32_t>(initStreamVolume(stream, indexMin,indexMax)));
            return NO_ERROR;
        } break;

        case SET_STREAM_VOLUME: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_stream_type_t stream =
                    static_cast <audio_stream_type_t>(data.readInt32());
            int index = data.readInt32();
            audio_devices_t device = static_cast <audio_devices_t>(data.readInt32());
            reply->writeInt32(static_cast <uint32_t>(setStreamVolumeIndex(stream,
                                                                          index,
                                                                          device)));
            return NO_ERROR;
        } break;

        case GET_STREAM_VOLUME: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_stream_type_t stream =
                    static_cast <audio_stream_type_t>(data.readInt32());
            audio_devices_t device = static_cast <audio_devices_t>(data.readInt32());
            int index = 0;
            status_t status = getStreamVolumeIndex(stream, &index, device);
            reply->writeInt32(index);
            reply->writeInt32(static_cast <uint32_t>(status));
            return NO_ERROR;
        } break;

        case GET_STRATEGY_FOR_STREAM: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_stream_type_t stream =
                    static_cast <audio_stream_type_t>(data.readInt32());
            reply->writeUint32(getStrategyForStream(stream));
            return NO_ERROR;
        } break;

        case SET_VOLUME_ATTRIBUTES: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_attributes_t attributes = {};
            status_t status = data.read(&attributes, sizeof(audio_attributes_t));
            if (status != NO_ERROR) {
                return status;
            }
            int index = data.readInt32();
            audio_devices_t device = static_cast <audio_devices_t>(data.readInt32());

            reply->writeInt32(static_cast <uint32_t>(setVolumeIndexForAttributes(attributes,
                                                                                 index, device)));
            return NO_ERROR;
        } break;

        case GET_VOLUME_ATTRIBUTES: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_attributes_t attributes = {};
            status_t status = data.read(&attributes, sizeof(audio_attributes_t));
            if (status != NO_ERROR) {
                return status;
            }
            audio_devices_t device = static_cast <audio_devices_t>(data.readInt32());

            int index = 0;
            status = getVolumeIndexForAttributes(attributes, index, device);
            reply->writeInt32(static_cast <uint32_t>(status));
            if (status == NO_ERROR) {
                reply->writeInt32(index);
            }
            return NO_ERROR;
        } break;

        case GET_MIN_VOLUME_FOR_ATTRIBUTES: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_attributes_t attributes = {};
            status_t status = data.read(&attributes, sizeof(audio_attributes_t));
            if (status != NO_ERROR) {
                return status;
            }

            int index = 0;
            status = getMinVolumeIndexForAttributes(attributes, index);
            reply->writeInt32(static_cast <uint32_t>(status));
            if (status == NO_ERROR) {
                reply->writeInt32(index);
            }
            return NO_ERROR;
        } break;

        case GET_MAX_VOLUME_FOR_ATTRIBUTES: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_attributes_t attributes = {};
            status_t status = data.read(&attributes, sizeof(audio_attributes_t));
            if (status != NO_ERROR) {
                return status;
            }

            int index = 0;
            status = getMaxVolumeIndexForAttributes(attributes, index);
            reply->writeInt32(static_cast <uint32_t>(status));
            if (status == NO_ERROR) {
                reply->writeInt32(index);
            }
            return NO_ERROR;
        } break;

        case GET_DEVICES_FOR_STREAM: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_stream_type_t stream =
                    static_cast <audio_stream_type_t>(data.readInt32());
            reply->writeInt32(static_cast <int>(getDevicesForStream(stream)));
            return NO_ERROR;
        } break;

        case GET_OUTPUT_FOR_EFFECT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            effect_descriptor_t desc = {};
            if (data.read(&desc, sizeof(desc)) != NO_ERROR) {
                android_errorWriteLog(0x534e4554, "73126106");
            }
            (void)sanitizeEffectDescriptor(&desc);
            audio_io_handle_t output = getOutputForEffect(&desc);
            reply->writeInt32(static_cast <int>(output));
            return NO_ERROR;
        } break;

        case REGISTER_EFFECT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            effect_descriptor_t desc = {};
            if (data.read(&desc, sizeof(desc)) != NO_ERROR) {
                android_errorWriteLog(0x534e4554, "73126106");
            }
            (void)sanitizeEffectDescriptor(&desc);
            audio_io_handle_t io = data.readInt32();
            uint32_t strategy = data.readInt32();
            audio_session_t session = (audio_session_t) data.readInt32();
            int id = data.readInt32();
            reply->writeInt32(static_cast <int32_t>(registerEffect(&desc,
                                                                   io,
                                                                   strategy,
                                                                   session,
                                                                   id)));
            return NO_ERROR;
        } break;

        case UNREGISTER_EFFECT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            int id = data.readInt32();
            reply->writeInt32(static_cast <int32_t>(unregisterEffect(id)));
            return NO_ERROR;
        } break;

        case SET_EFFECT_ENABLED: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            int id = data.readInt32();
            bool enabled = static_cast <bool>(data.readInt32());
            reply->writeInt32(static_cast <int32_t>(setEffectEnabled(id, enabled)));
            return NO_ERROR;
        } break;

        case MOVE_EFFECTS_TO_IO: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            std::vector<int> ids;
            int32_t size;
            status_t status = data.readInt32(&size);
            if (status != NO_ERROR) {
                return status;
            }
            if (size > MAX_ITEMS_PER_LIST) {
                return BAD_VALUE;
            }
            for (int32_t i = 0; i < size; i++) {
                int id;
                status =  data.readInt32(&id);
                if (status != NO_ERROR) {
                    return status;
                }
                ids.push_back(id);
            }

            audio_io_handle_t io = data.readInt32();
            reply->writeInt32(static_cast <int32_t>(moveEffectsToIo(ids, io)));
            return NO_ERROR;
        } break;

        case IS_STREAM_ACTIVE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_stream_type_t stream = (audio_stream_type_t) data.readInt32();
            uint32_t inPastMs = (uint32_t)data.readInt32();
            reply->writeInt32( isStreamActive(stream, inPastMs) );
            return NO_ERROR;
        } break;

        case IS_STREAM_ACTIVE_REMOTELY: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_stream_type_t stream = (audio_stream_type_t) data.readInt32();
            uint32_t inPastMs = (uint32_t)data.readInt32();
            reply->writeInt32( isStreamActiveRemotely(stream, inPastMs) );
            return NO_ERROR;
        } break;

        case IS_SOURCE_ACTIVE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_source_t source = (audio_source_t) data.readInt32();
            reply->writeInt32( isSourceActive(source));
            return NO_ERROR;
        }

        case QUERY_DEFAULT_PRE_PROCESSING: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_session_t audioSession = (audio_session_t) data.readInt32();
            uint32_t count = data.readInt32();
            if (count > AudioEffect::kMaxPreProcessing) {
                count = AudioEffect::kMaxPreProcessing;
            }
            uint32_t retCount = count;
            effect_descriptor_t *descriptors = new effect_descriptor_t[count]{};
            status_t status = queryDefaultPreProcessing(audioSession, descriptors, &retCount);
            reply->writeInt32(status);
            if (status != NO_ERROR && status != NO_MEMORY) {
                retCount = 0;
            }
            reply->writeInt32(retCount);
            if (retCount != 0) {
                if (retCount < count) {
                    count = retCount;
                }
                reply->write(descriptors, sizeof(effect_descriptor_t) * count);
            }
            delete[] descriptors;
            return status;
        }

        case IS_OFFLOAD_SUPPORTED: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_offload_info_t info = {};
            data.read(&info, sizeof(audio_offload_info_t));
            bool isSupported = isOffloadSupported(info);
            reply->writeInt32(isSupported);
            return NO_ERROR;
        }

        case IS_DIRECT_OUTPUT_SUPPORTED: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_config_base_t config = {};
            audio_attributes_t attributes = {};
            status_t status = data.read(&config, sizeof(audio_config_base_t));
            if (status != NO_ERROR) return status;
            status = data.read(&attributes, sizeof(audio_attributes_t));
            if (status != NO_ERROR) return status;
            reply->writeInt32(isDirectOutputSupported(config, attributes));
            return NO_ERROR;
        }

        case LIST_AUDIO_PORTS: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_port_role_t role = (audio_port_role_t)data.readInt32();
            audio_port_type_t type = (audio_port_type_t)data.readInt32();
            unsigned int numPortsReq = data.readInt32();
            if (numPortsReq > MAX_ITEMS_PER_LIST) {
                numPortsReq = MAX_ITEMS_PER_LIST;
            }
            unsigned int numPorts = numPortsReq;
            struct audio_port *ports =
                    (struct audio_port *)calloc(numPortsReq, sizeof(struct audio_port));
            if (ports == NULL) {
                reply->writeInt32(NO_MEMORY);
                reply->writeInt32(0);
                return NO_ERROR;
            }
            unsigned int generation;
            status_t status = listAudioPorts(role, type, &numPorts, ports, &generation);
            reply->writeInt32(status);
            reply->writeInt32(numPorts);

            if (status == NO_ERROR) {
                if (numPortsReq > numPorts) {
                    numPortsReq = numPorts;
                }
                reply->write(ports, numPortsReq * sizeof(struct audio_port));
                reply->writeInt32(generation);
            }
            free(ports);
            return NO_ERROR;
        }

        case GET_AUDIO_PORT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            struct audio_port port = {};
            if (data.read(&port, sizeof(struct audio_port)) != NO_ERROR) {
                ALOGE("b/23912202");
            }
            status_t status = getAudioPort(&port);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->write(&port, sizeof(struct audio_port));
            }
            return NO_ERROR;
        }

        case CREATE_AUDIO_PATCH: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            struct audio_patch patch = {};
            data.read(&patch, sizeof(struct audio_patch));
            audio_patch_handle_t handle = AUDIO_PATCH_HANDLE_NONE;
            if (data.read(&handle, sizeof(audio_patch_handle_t)) != NO_ERROR) {
                ALOGE("b/23912202");
            }
            status_t status = createAudioPatch(&patch, &handle);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->write(&handle, sizeof(audio_patch_handle_t));
            }
            return NO_ERROR;
        }

        case RELEASE_AUDIO_PATCH: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_patch_handle_t handle = AUDIO_PATCH_HANDLE_NONE;
            data.read(&handle, sizeof(audio_patch_handle_t));
            status_t status = releaseAudioPatch(handle);
            reply->writeInt32(status);
            return NO_ERROR;
        }

        case LIST_AUDIO_PATCHES: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            unsigned int numPatchesReq = data.readInt32();
            if (numPatchesReq > MAX_ITEMS_PER_LIST) {
                numPatchesReq = MAX_ITEMS_PER_LIST;
            }
            unsigned int numPatches = numPatchesReq;
            struct audio_patch *patches =
                    (struct audio_patch *)calloc(numPatchesReq,
                                                 sizeof(struct audio_patch));
            if (patches == NULL) {
                reply->writeInt32(NO_MEMORY);
                reply->writeInt32(0);
                return NO_ERROR;
            }
            unsigned int generation;
            status_t status = listAudioPatches(&numPatches, patches, &generation);
            reply->writeInt32(status);
            reply->writeInt32(numPatches);
            if (status == NO_ERROR) {
                if (numPatchesReq > numPatches) {
                    numPatchesReq = numPatches;
                }
                reply->write(patches, numPatchesReq * sizeof(struct audio_patch));
                reply->writeInt32(generation);
            }
            free(patches);
            return NO_ERROR;
        }

        case SET_AUDIO_PORT_CONFIG: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            struct audio_port_config config = {};
            data.read(&config, sizeof(struct audio_port_config));
            (void)sanitizeAudioPortConfig(&config);
            status_t status = setAudioPortConfig(&config);
            reply->writeInt32(status);
            return NO_ERROR;
        }

        case REGISTER_CLIENT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            sp<IAudioPolicyServiceClient> client = interface_cast<IAudioPolicyServiceClient>(
                    data.readStrongBinder());
            registerClient(client);
            return NO_ERROR;
        } break;

        case SET_AUDIO_PORT_CALLBACK_ENABLED: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            setAudioPortCallbacksEnabled(data.readInt32() == 1);
            return NO_ERROR;
        } break;

        case SET_AUDIO_VOLUME_GROUP_CALLBACK_ENABLED: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            setAudioVolumeGroupCallbacksEnabled(data.readInt32() == 1);
            return NO_ERROR;
        } break;

        case ACQUIRE_SOUNDTRIGGER_SESSION: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            sp<IAudioPolicyServiceClient> client = interface_cast<IAudioPolicyServiceClient>(
                    data.readStrongBinder());
            audio_session_t session = AUDIO_SESSION_NONE;
            audio_io_handle_t ioHandle = AUDIO_IO_HANDLE_NONE;
            audio_devices_t device = AUDIO_DEVICE_NONE;
            status_t status = acquireSoundTriggerSession(&session, &ioHandle, &device);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->writeInt32(session);
                reply->writeInt32(ioHandle);
                reply->writeInt32(device);
            }
            return NO_ERROR;
        } break;

        case RELEASE_SOUNDTRIGGER_SESSION: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            sp<IAudioPolicyServiceClient> client = interface_cast<IAudioPolicyServiceClient>(
                    data.readStrongBinder());
            audio_session_t session = (audio_session_t)data.readInt32();
            status_t status = releaseSoundTriggerSession(session);
            reply->writeInt32(status);
            return NO_ERROR;
        } break;

        case GET_PHONE_STATE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            reply->writeInt32((int32_t)getPhoneState());
            return NO_ERROR;
        } break;

        case REGISTER_POLICY_MIXES: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            bool registration = data.readInt32() == 1;
            Vector<AudioMix> mixes;
            size_t size = (size_t)data.readInt32();
            if (size > MAX_MIXES_PER_POLICY) {
                size = MAX_MIXES_PER_POLICY;
            }
            for (size_t i = 0; i < size; i++) {
                AudioMix mix;
                if (mix.readFromParcel((Parcel*)&data) == NO_ERROR) {
                    mixes.add(mix);
                }
            }
            status_t status = registerPolicyMixes(mixes, registration);
            reply->writeInt32(status);
            return NO_ERROR;
        } break;

        case START_AUDIO_SOURCE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            struct audio_port_config source = {};
            data.read(&source, sizeof(struct audio_port_config));
            (void)sanitizeAudioPortConfig(&source);
            audio_attributes_t attributes = {};
            data.read(&attributes, sizeof(audio_attributes_t));
            sanetizeAudioAttributes(&attributes);
            audio_port_handle_t portId = AUDIO_PORT_HANDLE_NONE;
            status_t status = startAudioSource(&source, &attributes, &portId);
            reply->writeInt32(status);
            reply->writeInt32(portId);
            return NO_ERROR;
        } break;

        case STOP_AUDIO_SOURCE: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_port_handle_t portId = (audio_port_handle_t) data.readInt32();
            status_t status = stopAudioSource(portId);
            reply->writeInt32(status);
            return NO_ERROR;
        } break;

        case SET_MASTER_MONO: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            bool mono = static_cast<bool>(data.readInt32());
            status_t status = setMasterMono(mono);
            reply->writeInt32(status);
            return NO_ERROR;
        } break;

        case GET_MASTER_MONO: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            bool mono;
            status_t status = getMasterMono(&mono);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->writeInt32(static_cast<int32_t>(mono));
            }
            return NO_ERROR;
        } break;

        case GET_STREAM_VOLUME_DB: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_stream_type_t stream =
                    static_cast <audio_stream_type_t>(data.readInt32());
            int index = static_cast <int>(data.readInt32());
            audio_devices_t device =
                    static_cast <audio_devices_t>(data.readUint32());
            reply->writeFloat(getStreamVolumeDB(stream, index, device));
            return NO_ERROR;
        }

        case GET_SURROUND_FORMATS: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            unsigned int numSurroundFormatsReq = data.readUint32();
            if (numSurroundFormatsReq > MAX_ITEMS_PER_LIST) {
                numSurroundFormatsReq = MAX_ITEMS_PER_LIST;
            }
            bool reported = data.readBool();
            unsigned int numSurroundFormats = numSurroundFormatsReq;
            audio_format_t *surroundFormats = (audio_format_t *)calloc(
                    numSurroundFormats, sizeof(audio_format_t));
            bool *surroundFormatsEnabled = (bool *)calloc(numSurroundFormats, sizeof(bool));
            if (numSurroundFormatsReq > 0 &&
                    (surroundFormats == NULL || surroundFormatsEnabled == NULL)) {
                free(surroundFormats);
                free(surroundFormatsEnabled);
                reply->writeInt32(NO_MEMORY);
                return NO_ERROR;
            }
            status_t status = getSurroundFormats(
                    &numSurroundFormats, surroundFormats, surroundFormatsEnabled, reported);
            reply->writeInt32(status);

            if (status == NO_ERROR) {
                reply->writeUint32(numSurroundFormats);
                if (numSurroundFormatsReq > numSurroundFormats) {
                    numSurroundFormatsReq = numSurroundFormats;
                }
                reply->write(surroundFormats, numSurroundFormatsReq * sizeof(audio_format_t));
                reply->write(surroundFormatsEnabled, numSurroundFormatsReq * sizeof(bool));
            }
            free(surroundFormats);
            free(surroundFormatsEnabled);
            return NO_ERROR;
        }

        case SET_SURROUND_FORMAT_ENABLED: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_format_t audioFormat = (audio_format_t) data.readInt32();
            bool enabled = data.readBool();
            status_t status = setSurroundFormatEnabled(audioFormat, enabled);
            reply->writeInt32(status);
            return NO_ERROR;
        }

        case GET_OFFLOAD_FORMATS_A2DP: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            std::vector<audio_format_t> encodingFormats;
            status_t status = getHwOffloadEncodingFormatsSupportedForA2DP(&encodingFormats);
            reply->writeInt32(status);
            if (status != NO_ERROR) {
                return NO_ERROR;
            }
            reply->writeUint32(static_cast<uint32_t>(encodingFormats.size()));
            for (size_t i = 0; i < encodingFormats.size(); i++)
                reply->writeInt32(static_cast<int32_t>(encodingFormats[i]));
            return NO_ERROR;
        }


        case ADD_STREAM_DEFAULT_EFFECT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            effect_uuid_t type;
            status_t status = data.read(&type, sizeof(effect_uuid_t));
            if (status != NO_ERROR) {
                return status;
            }
            String16 opPackageName;
            status = data.readString16(&opPackageName);
            if (status != NO_ERROR) {
                return status;
            }
            effect_uuid_t uuid;
            status = data.read(&uuid, sizeof(effect_uuid_t));
            if (status != NO_ERROR) {
                return status;
            }
            int32_t priority = data.readInt32();
            audio_usage_t usage = (audio_usage_t) data.readInt32();
            audio_unique_id_t id = 0;
            reply->writeInt32(static_cast <int32_t>(addStreamDefaultEffect(&type,
                                                                           opPackageName,
                                                                           &uuid,
                                                                           priority,
                                                                           usage,
                                                                           &id)));
            reply->writeInt32(id);
            return NO_ERROR;
        }

        case REMOVE_STREAM_DEFAULT_EFFECT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_unique_id_t id = static_cast<audio_unique_id_t>(data.readInt32());
            reply->writeInt32(static_cast <int32_t>(removeStreamDefaultEffect(id)));
            return NO_ERROR;
        }

        case ADD_SOURCE_DEFAULT_EFFECT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            effect_uuid_t type;
            status_t status = data.read(&type, sizeof(effect_uuid_t));
            if (status != NO_ERROR) {
                return status;
            }
            String16 opPackageName;
            status = data.readString16(&opPackageName);
            if (status != NO_ERROR) {
                return status;
            }
            effect_uuid_t uuid;
            status = data.read(&uuid, sizeof(effect_uuid_t));
            if (status != NO_ERROR) {
                return status;
            }
            int32_t priority = data.readInt32();
            audio_source_t source = (audio_source_t) data.readInt32();
            audio_unique_id_t id = 0;
            reply->writeInt32(static_cast <int32_t>(addSourceDefaultEffect(&type,
                                                                           opPackageName,
                                                                           &uuid,
                                                                           priority,
                                                                           source,
                                                                           &id)));
            reply->writeInt32(id);
            return NO_ERROR;
        }

        case REMOVE_SOURCE_DEFAULT_EFFECT: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            audio_unique_id_t id = static_cast<audio_unique_id_t>(data.readInt32());
            reply->writeInt32(static_cast <int32_t>(removeSourceDefaultEffect(id)));
            return NO_ERROR;
        }

        case SET_ASSISTANT_UID: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            int32_t uid;
            status_t status = data.readInt32(&uid);
            if (status != NO_ERROR) {
                return status;
            }
            status = setAssistantUid(uid);
            reply->writeInt32(static_cast <int32_t>(status));
            return NO_ERROR;
        }

        case SET_A11Y_SERVICES_UIDS: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            std::vector<uid_t> uids;
            int32_t size;
            status_t status = data.readInt32(&size);
            if (status != NO_ERROR) {
                return status;
            }
            if (size > MAX_ITEMS_PER_LIST) {
                size = MAX_ITEMS_PER_LIST;
            }
            for (int32_t i = 0; i < size; i++) {
                int32_t uid;
                status =  data.readInt32(&uid);
                if (status != NO_ERROR) {
                    return status;
                }
                uids.push_back(uid);
            }
            status = setA11yServicesUids(uids);
            reply->writeInt32(static_cast <int32_t>(status));
            return NO_ERROR;
        }

        case IS_HAPTIC_PLAYBACK_SUPPORTED: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            bool isSupported = isHapticPlaybackSupported();
            reply->writeBool(isSupported);
            return NO_ERROR;
        }

        case SET_UID_DEVICE_AFFINITY: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            const uid_t uid = (uid_t) data.readInt32();
            Vector<AudioDeviceTypeAddr> devices;
            size_t size = (size_t)data.readInt32();
            for (size_t i = 0; i < size; i++) {
                AudioDeviceTypeAddr device;
                if (device.readFromParcel((Parcel*)&data) == NO_ERROR) {
                    devices.add(device);
                }
            }
            status_t status = setUidDeviceAffinities(uid, devices);
            reply->writeInt32(status);
            return NO_ERROR;
        }

        case REMOVE_UID_DEVICE_AFFINITY: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            const uid_t uid = (uid_t) data.readInt32();
            status_t status = removeUidDeviceAffinities(uid);
            reply->writeInt32(status);
            return NO_ERROR;
        }

        case LIST_AUDIO_PRODUCT_STRATEGIES: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            AudioProductStrategyVector strategies;
            status_t status = listAudioProductStrategies(strategies);
            reply->writeInt32(status);
            if (status != NO_ERROR) {
                return NO_ERROR;
            }
            size_t size = strategies.size();
            size_t sizePosition = reply->dataPosition();
            reply->writeInt32(size);
            size_t finalSize = size;
            for (size_t i = 0; i < size; i++) {
                size_t position = reply->dataPosition();
                if (strategies[i].writeToParcel(reply) != NO_ERROR) {
                    reply->setDataPosition(position);
                    finalSize--;
                }
            }
            if (size != finalSize) {
                size_t position = reply->dataPosition();
                reply->setDataPosition(sizePosition);
                reply->writeInt32(finalSize);
                reply->setDataPosition(position);
            }
            return NO_ERROR;
        }

        case GET_STRATEGY_FOR_ATTRIBUTES: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            AudioAttributes attributes;
            status_t status = attributes.readFromParcel(&data);
            if (status != NO_ERROR) {
                return status;
            }
            product_strategy_t strategy;
            status = getProductStrategyFromAudioAttributes(attributes, strategy);
            reply->writeInt32(status);
            if (status != NO_ERROR) {
                return NO_ERROR;
            }
            reply->writeUint32(static_cast<int>(strategy));
            return NO_ERROR;
        }

        case LIST_AUDIO_VOLUME_GROUPS: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            AudioVolumeGroupVector groups;
            status_t status = listAudioVolumeGroups(groups);
            reply->writeInt32(status);
            if (status != NO_ERROR) {
                return NO_ERROR;
            }
            size_t size = groups.size();
            size_t sizePosition = reply->dataPosition();
            reply->writeInt32(size);
            size_t finalSize = size;
            for (size_t i = 0; i < size; i++) {
                size_t position = reply->dataPosition();
                if (groups[i].writeToParcel(reply) != NO_ERROR) {
                    reply->setDataPosition(position);
                    finalSize--;
                }
            }
            if (size != finalSize) {
                size_t position = reply->dataPosition();
                reply->setDataPosition(sizePosition);
                reply->writeInt32(finalSize);
                reply->setDataPosition(position);
            }
            return NO_ERROR;
        }

        case GET_VOLUME_GROUP_FOR_ATTRIBUTES: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            AudioAttributes attributes;
            status_t status = attributes.readFromParcel(&data);
            if (status != NO_ERROR) {
                return status;
            }
            volume_group_t group;
            status = getVolumeGroupFromAudioAttributes(attributes, group);
            reply->writeInt32(status);
            if (status != NO_ERROR) {
                return NO_ERROR;
            }
            reply->writeUint32(static_cast<int>(group));
            return NO_ERROR;
        }

        case SET_ALLOWED_CAPTURE_POLICY: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            uid_t uid = data.readInt32();
            audio_flags_mask_t flags = data.readInt32();
            status_t status = setAllowedCapturePolicy(uid, flags);
            reply->writeInt32(status);
            return NO_ERROR;
        }

        case SET_RTT_ENABLED: {
            CHECK_INTERFACE(IAudioPolicyService, data, reply);
            bool enabled = static_cast<bool>(data.readInt32());
            status_t status = setRttEnabled(enabled);
            reply->writeInt32(status);
            return NO_ERROR;
        }

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

/** returns true if string overflow was prevented by zero termination */
template <size_t size>
static bool preventStringOverflow(char (&s)[size]) {
    if (strnlen(s, size) < size) return false;
    s[size - 1] = '\0';
    return true;
}

void BnAudioPolicyService::sanetizeAudioAttributes(audio_attributes_t* attr)
{
    const size_t tagsMaxSize = AUDIO_ATTRIBUTES_TAGS_MAX_SIZE;
    if (strnlen(attr->tags, tagsMaxSize) >= tagsMaxSize) {
        android_errorWriteLog(0x534e4554, "68953950"); // SafetyNet logging
    }
    attr->tags[tagsMaxSize - 1] = '\0';
}

/** returns BAD_VALUE if sanitization was required. */
status_t BnAudioPolicyService::sanitizeEffectDescriptor(effect_descriptor_t* desc)
{
    if (preventStringOverflow(desc->name)
        | /* always */ preventStringOverflow(desc->implementor)) {
        android_errorWriteLog(0x534e4554, "73126106"); // SafetyNet logging
        return BAD_VALUE;
    }
    return NO_ERROR;
}

/** returns BAD_VALUE if sanitization was required. */
status_t BnAudioPolicyService::sanitizeAudioPortConfig(struct audio_port_config* config)
{
    if (config->type == AUDIO_PORT_TYPE_DEVICE &&
        preventStringOverflow(config->ext.device.address)) {
        return BAD_VALUE;
    }
    return NO_ERROR;
}

// ----------------------------------------------------------------------------

} // namespace android
