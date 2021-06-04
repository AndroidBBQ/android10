/*
**
** Copyright 2007, The Android Open Source Project
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

#define LOG_TAG "IAudioFlinger"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/IPCThreadState.h>
#include <binder/Parcel.h>
#include <media/TimeCheck.h>
#include <mediautils/ServiceUtilities.h>
#include "IAudioFlinger.h"

namespace android {

enum {
    CREATE_TRACK = IBinder::FIRST_CALL_TRANSACTION,
    CREATE_RECORD,
    SAMPLE_RATE,
    RESERVED,   // obsolete, was CHANNEL_COUNT
    FORMAT,
    FRAME_COUNT,
    LATENCY,
    SET_MASTER_VOLUME,
    SET_MASTER_MUTE,
    MASTER_VOLUME,
    MASTER_MUTE,
    SET_STREAM_VOLUME,
    SET_STREAM_MUTE,
    STREAM_VOLUME,
    STREAM_MUTE,
    SET_MODE,
    SET_MIC_MUTE,
    GET_MIC_MUTE,
    SET_RECORD_SILENCED,
    SET_PARAMETERS,
    GET_PARAMETERS,
    REGISTER_CLIENT,
    GET_INPUTBUFFERSIZE,
    OPEN_OUTPUT,
    OPEN_DUPLICATE_OUTPUT,
    CLOSE_OUTPUT,
    SUSPEND_OUTPUT,
    RESTORE_OUTPUT,
    OPEN_INPUT,
    CLOSE_INPUT,
    INVALIDATE_STREAM,
    SET_VOICE_VOLUME,
    GET_RENDER_POSITION,
    GET_INPUT_FRAMES_LOST,
    NEW_AUDIO_UNIQUE_ID,
    ACQUIRE_AUDIO_SESSION_ID,
    RELEASE_AUDIO_SESSION_ID,
    QUERY_NUM_EFFECTS,
    QUERY_EFFECT,
    GET_EFFECT_DESCRIPTOR,
    CREATE_EFFECT,
    MOVE_EFFECTS,
    LOAD_HW_MODULE,
    GET_PRIMARY_OUTPUT_SAMPLING_RATE,
    GET_PRIMARY_OUTPUT_FRAME_COUNT,
    SET_LOW_RAM_DEVICE,
    LIST_AUDIO_PORTS,
    GET_AUDIO_PORT,
    CREATE_AUDIO_PATCH,
    RELEASE_AUDIO_PATCH,
    LIST_AUDIO_PATCHES,
    SET_AUDIO_PORT_CONFIG,
    GET_AUDIO_HW_SYNC_FOR_SESSION,
    SYSTEM_READY,
    FRAME_COUNT_HAL,
    GET_MICROPHONES,
    SET_MASTER_BALANCE,
    GET_MASTER_BALANCE,
    SET_EFFECT_SUSPENDED,
};

#define MAX_ITEMS_PER_LIST 1024

class BpAudioFlinger : public BpInterface<IAudioFlinger>
{
public:
    explicit BpAudioFlinger(const sp<IBinder>& impl)
        : BpInterface<IAudioFlinger>(impl)
    {
    }

    virtual sp<IAudioTrack> createTrack(const CreateTrackInput& input,
                                        CreateTrackOutput& output,
                                        status_t *status)
    {
        Parcel data, reply;
        sp<IAudioTrack> track;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());

        if (status == nullptr) {
            return track;
        }

        input.writeToParcel(&data);

        status_t lStatus = remote()->transact(CREATE_TRACK, data, &reply);
        if (lStatus != NO_ERROR) {
            ALOGE("createTrack transaction error %d", lStatus);
            *status = DEAD_OBJECT;
            return track;
        }
        *status = reply.readInt32();
        if (*status != NO_ERROR) {
            ALOGE("createTrack returned error %d", *status);
            return track;
        }
        track = interface_cast<IAudioTrack>(reply.readStrongBinder());
        if (track == 0) {
            ALOGE("createTrack returned an NULL IAudioTrack with status OK");
            *status = DEAD_OBJECT;
            return track;
        }
        output.readFromParcel(&reply);
        return track;
    }

    virtual sp<media::IAudioRecord> createRecord(const CreateRecordInput& input,
                                                 CreateRecordOutput& output,
                                                 status_t *status)
    {
        Parcel data, reply;
        sp<media::IAudioRecord> record;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());

        if (status == nullptr) {
            return record;
        }

        input.writeToParcel(&data);

        status_t lStatus = remote()->transact(CREATE_RECORD, data, &reply);
        if (lStatus != NO_ERROR) {
            ALOGE("createRecord transaction error %d", lStatus);
            *status = DEAD_OBJECT;
            return record;
        }
        *status = reply.readInt32();
        if (*status != NO_ERROR) {
            ALOGE("createRecord returned error %d", *status);
            return record;
        }

        record = interface_cast<media::IAudioRecord>(reply.readStrongBinder());
        if (record == 0) {
            ALOGE("createRecord returned a NULL IAudioRecord with status OK");
            *status = DEAD_OBJECT;
            return record;
        }
        output.readFromParcel(&reply);
        return record;
    }

    virtual uint32_t sampleRate(audio_io_handle_t ioHandle) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) ioHandle);
        remote()->transact(SAMPLE_RATE, data, &reply);
        return reply.readInt32();
    }

    // RESERVED for channelCount()

    virtual audio_format_t format(audio_io_handle_t output) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) output);
        remote()->transact(FORMAT, data, &reply);
        return (audio_format_t) reply.readInt32();
    }

    virtual size_t frameCount(audio_io_handle_t ioHandle) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) ioHandle);
        remote()->transact(FRAME_COUNT, data, &reply);
        return reply.readInt64();
    }

    virtual uint32_t latency(audio_io_handle_t output) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) output);
        remote()->transact(LATENCY, data, &reply);
        return reply.readInt32();
    }

    virtual status_t setMasterVolume(float value)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeFloat(value);
        remote()->transact(SET_MASTER_VOLUME, data, &reply);
        return reply.readInt32();
    }

    virtual status_t setMasterMute(bool muted)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(muted);
        remote()->transact(SET_MASTER_MUTE, data, &reply);
        return reply.readInt32();
    }

    virtual float masterVolume() const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        remote()->transact(MASTER_VOLUME, data, &reply);
        return reply.readFloat();
    }

    virtual bool masterMute() const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        remote()->transact(MASTER_MUTE, data, &reply);
        return reply.readInt32();
    }

    status_t setMasterBalance(float balance) override
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeFloat(balance);
        status_t status = remote()->transact(SET_MASTER_BALANCE, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        return reply.readInt32();
    }

    status_t getMasterBalance(float *balance) const override
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        status_t status = remote()->transact(GET_MASTER_BALANCE, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = (status_t)reply.readInt32();
        if (status != NO_ERROR) {
            return status;
        }
        *balance = reply.readFloat();
        return NO_ERROR;
    }

    virtual status_t setStreamVolume(audio_stream_type_t stream, float value,
            audio_io_handle_t output)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) stream);
        data.writeFloat(value);
        data.writeInt32((int32_t) output);
        remote()->transact(SET_STREAM_VOLUME, data, &reply);
        return reply.readInt32();
    }

    virtual status_t setStreamMute(audio_stream_type_t stream, bool muted)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) stream);
        data.writeInt32(muted);
        remote()->transact(SET_STREAM_MUTE, data, &reply);
        return reply.readInt32();
    }

    virtual float streamVolume(audio_stream_type_t stream, audio_io_handle_t output) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) stream);
        data.writeInt32((int32_t) output);
        remote()->transact(STREAM_VOLUME, data, &reply);
        return reply.readFloat();
    }

    virtual bool streamMute(audio_stream_type_t stream) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) stream);
        remote()->transact(STREAM_MUTE, data, &reply);
        return reply.readInt32();
    }

    virtual status_t setMode(audio_mode_t mode)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(mode);
        remote()->transact(SET_MODE, data, &reply);
        return reply.readInt32();
    }

    virtual status_t setMicMute(bool state)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(state);
        remote()->transact(SET_MIC_MUTE, data, &reply);
        return reply.readInt32();
    }

    virtual bool getMicMute() const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        remote()->transact(GET_MIC_MUTE, data, &reply);
        return reply.readInt32();
    }

    virtual void setRecordSilenced(uid_t uid, bool silenced)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(uid);
        data.writeInt32(silenced ? 1 : 0);
        remote()->transact(SET_RECORD_SILENCED, data, &reply);
    }

    virtual status_t setParameters(audio_io_handle_t ioHandle, const String8& keyValuePairs)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) ioHandle);
        data.writeString8(keyValuePairs);
        remote()->transact(SET_PARAMETERS, data, &reply);
        return reply.readInt32();
    }

    virtual String8 getParameters(audio_io_handle_t ioHandle, const String8& keys) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) ioHandle);
        data.writeString8(keys);
        remote()->transact(GET_PARAMETERS, data, &reply);
        return reply.readString8();
    }

    virtual void registerClient(const sp<IAudioFlingerClient>& client)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(client));
        remote()->transact(REGISTER_CLIENT, data, &reply);
    }

    virtual size_t getInputBufferSize(uint32_t sampleRate, audio_format_t format,
            audio_channel_mask_t channelMask) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(sampleRate);
        data.writeInt32(format);
        data.writeInt32(channelMask);
        remote()->transact(GET_INPUTBUFFERSIZE, data, &reply);
        return reply.readInt64();
    }

    virtual status_t openOutput(audio_module_handle_t module,
                                audio_io_handle_t *output,
                                audio_config_t *config,
                                audio_devices_t *devices,
                                const String8& address,
                                uint32_t *latencyMs,
                                audio_output_flags_t flags)
    {
        if (output == NULL || config == NULL || devices == NULL || latencyMs == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(module);
        data.write(config, sizeof(audio_config_t));
        data.writeInt32(*devices);
        data.writeString8(address);
        data.writeInt32((int32_t) flags);
        status_t status = remote()->transact(OPEN_OUTPUT, data, &reply);
        if (status != NO_ERROR) {
            *output = AUDIO_IO_HANDLE_NONE;
            return status;
        }
        status = (status_t)reply.readInt32();
        if (status != NO_ERROR) {
            *output = AUDIO_IO_HANDLE_NONE;
            return status;
        }
        *output = (audio_io_handle_t)reply.readInt32();
        ALOGV("openOutput() returned output, %d", *output);
        reply.read(config, sizeof(audio_config_t));
        *devices = (audio_devices_t)reply.readInt32();
        *latencyMs = reply.readInt32();
        return NO_ERROR;
    }

    virtual audio_io_handle_t openDuplicateOutput(audio_io_handle_t output1,
            audio_io_handle_t output2)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) output1);
        data.writeInt32((int32_t) output2);
        remote()->transact(OPEN_DUPLICATE_OUTPUT, data, &reply);
        return (audio_io_handle_t) reply.readInt32();
    }

    virtual status_t closeOutput(audio_io_handle_t output)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) output);
        remote()->transact(CLOSE_OUTPUT, data, &reply);
        return reply.readInt32();
    }

    virtual status_t suspendOutput(audio_io_handle_t output)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) output);
        remote()->transact(SUSPEND_OUTPUT, data, &reply);
        return reply.readInt32();
    }

    virtual status_t restoreOutput(audio_io_handle_t output)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) output);
        remote()->transact(RESTORE_OUTPUT, data, &reply);
        return reply.readInt32();
    }

    virtual status_t openInput(audio_module_handle_t module,
                               audio_io_handle_t *input,
                               audio_config_t *config,
                               audio_devices_t *device,
                               const String8& address,
                               audio_source_t source,
                               audio_input_flags_t flags)
    {
        if (input == NULL || config == NULL || device == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(module);
        data.writeInt32(*input);
        data.write(config, sizeof(audio_config_t));
        data.writeInt32(*device);
        data.writeString8(address);
        data.writeInt32(source);
        data.writeInt32(flags);
        status_t status = remote()->transact(OPEN_INPUT, data, &reply);
        if (status != NO_ERROR) {
            *input = AUDIO_IO_HANDLE_NONE;
            return status;
        }
        status = (status_t)reply.readInt32();
        if (status != NO_ERROR) {
            *input = AUDIO_IO_HANDLE_NONE;
            return status;
        }
        *input = (audio_io_handle_t)reply.readInt32();
        reply.read(config, sizeof(audio_config_t));
        *device = (audio_devices_t)reply.readInt32();
        return NO_ERROR;
    }

    virtual status_t closeInput(int input)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(input);
        remote()->transact(CLOSE_INPUT, data, &reply);
        return reply.readInt32();
    }

    virtual status_t invalidateStream(audio_stream_type_t stream)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) stream);
        remote()->transact(INVALIDATE_STREAM, data, &reply);
        return reply.readInt32();
    }

    virtual status_t setVoiceVolume(float volume)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeFloat(volume);
        remote()->transact(SET_VOICE_VOLUME, data, &reply);
        return reply.readInt32();
    }

    virtual status_t getRenderPosition(uint32_t *halFrames, uint32_t *dspFrames,
            audio_io_handle_t output) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) output);
        remote()->transact(GET_RENDER_POSITION, data, &reply);
        status_t status = reply.readInt32();
        if (status == NO_ERROR) {
            uint32_t tmp = reply.readInt32();
            if (halFrames != NULL) {
                *halFrames = tmp;
            }
            tmp = reply.readInt32();
            if (dspFrames != NULL) {
                *dspFrames = tmp;
            }
        }
        return status;
    }

    virtual uint32_t getInputFramesLost(audio_io_handle_t ioHandle) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) ioHandle);
        status_t status = remote()->transact(GET_INPUT_FRAMES_LOST, data, &reply);
        if (status != NO_ERROR) {
            return 0;
        }
        return (uint32_t) reply.readInt32();
    }

    virtual audio_unique_id_t newAudioUniqueId(audio_unique_id_use_t use)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) use);
        status_t status = remote()->transact(NEW_AUDIO_UNIQUE_ID, data, &reply);
        audio_unique_id_t id = AUDIO_UNIQUE_ID_ALLOCATE;
        if (status == NO_ERROR) {
            id = reply.readInt32();
        }
        return id;
    }

    virtual void acquireAudioSessionId(audio_session_t audioSession, int pid)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(audioSession);
        data.writeInt32(pid);
        remote()->transact(ACQUIRE_AUDIO_SESSION_ID, data, &reply);
    }

    virtual void releaseAudioSessionId(audio_session_t audioSession, int pid)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(audioSession);
        data.writeInt32(pid);
        remote()->transact(RELEASE_AUDIO_SESSION_ID, data, &reply);
    }

    virtual status_t queryNumberEffects(uint32_t *numEffects) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        status_t status = remote()->transact(QUERY_NUM_EFFECTS, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = reply.readInt32();
        if (status != NO_ERROR) {
            return status;
        }
        if (numEffects != NULL) {
            *numEffects = (uint32_t)reply.readInt32();
        }
        return NO_ERROR;
    }

    virtual status_t queryEffect(uint32_t index, effect_descriptor_t *pDescriptor) const
    {
        if (pDescriptor == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(index);
        status_t status = remote()->transact(QUERY_EFFECT, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = reply.readInt32();
        if (status != NO_ERROR) {
            return status;
        }
        reply.read(pDescriptor, sizeof(effect_descriptor_t));
        return NO_ERROR;
    }

    virtual status_t getEffectDescriptor(const effect_uuid_t *pUuid,
                                         const effect_uuid_t *pType,
                                         uint32_t preferredTypeFlag,
                                         effect_descriptor_t *pDescriptor) const
    {
        if (pUuid == NULL || pType == NULL || pDescriptor == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.write(pUuid, sizeof(effect_uuid_t));
        data.write(pType, sizeof(effect_uuid_t));
        data.writeUint32(preferredTypeFlag);
        status_t status = remote()->transact(GET_EFFECT_DESCRIPTOR, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = reply.readInt32();
        if (status != NO_ERROR) {
            return status;
        }
        reply.read(pDescriptor, sizeof(effect_descriptor_t));
        return NO_ERROR;
    }

    virtual sp<IEffect> createEffect(
                                    effect_descriptor_t *pDesc,
                                    const sp<IEffectClient>& client,
                                    int32_t priority,
                                    audio_io_handle_t output,
                                    audio_session_t sessionId,
                                    const String16& opPackageName,
                                    pid_t pid,
                                    status_t *status,
                                    int *id,
                                    int *enabled)
    {
        Parcel data, reply;
        sp<IEffect> effect;

        if (pDesc == NULL) {
            if (status != NULL) {
                *status = BAD_VALUE;
            }
            return effect;
        }

        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.write(pDesc, sizeof(effect_descriptor_t));
        data.writeStrongBinder(IInterface::asBinder(client));
        data.writeInt32(priority);
        data.writeInt32((int32_t) output);
        data.writeInt32(sessionId);
        data.writeString16(opPackageName);
        data.writeInt32((int32_t) pid);

        status_t lStatus = remote()->transact(CREATE_EFFECT, data, &reply);
        if (lStatus != NO_ERROR) {
            ALOGE("createEffect error: %s", strerror(-lStatus));
        } else {
            lStatus = reply.readInt32();
            int tmp = reply.readInt32();
            if (id != NULL) {
                *id = tmp;
            }
            tmp = reply.readInt32();
            if (enabled != NULL) {
                *enabled = tmp;
            }
            effect = interface_cast<IEffect>(reply.readStrongBinder());
            reply.read(pDesc, sizeof(effect_descriptor_t));
        }
        if (status != NULL) {
            *status = lStatus;
        }

        return effect;
    }

    virtual status_t moveEffects(audio_session_t session, audio_io_handle_t srcOutput,
            audio_io_handle_t dstOutput)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(session);
        data.writeInt32((int32_t) srcOutput);
        data.writeInt32((int32_t) dstOutput);
        remote()->transact(MOVE_EFFECTS, data, &reply);
        return reply.readInt32();
    }

    virtual void setEffectSuspended(int effectId,
                                    audio_session_t sessionId,
                                    bool suspended)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(effectId);
        data.writeInt32(sessionId);
        data.writeInt32(suspended ? 1 : 0);
        remote()->transact(SET_EFFECT_SUSPENDED, data, &reply);
    }

    virtual audio_module_handle_t loadHwModule(const char *name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeCString(name);
        remote()->transact(LOAD_HW_MODULE, data, &reply);
        return (audio_module_handle_t) reply.readInt32();
    }

    virtual uint32_t getPrimaryOutputSamplingRate()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        remote()->transact(GET_PRIMARY_OUTPUT_SAMPLING_RATE, data, &reply);
        return reply.readInt32();
    }

    virtual size_t getPrimaryOutputFrameCount()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        remote()->transact(GET_PRIMARY_OUTPUT_FRAME_COUNT, data, &reply);
        return reply.readInt64();
    }

    virtual status_t setLowRamDevice(bool isLowRamDevice, int64_t totalMemory) override
    {
        Parcel data, reply;

        static_assert(NO_ERROR == 0, "NO_ERROR must be 0");
        return data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor())
                ?: data.writeInt32((int) isLowRamDevice)
                ?: data.writeInt64(totalMemory)
                ?: remote()->transact(SET_LOW_RAM_DEVICE, data, &reply)
                ?: reply.readInt32();
    }

    virtual status_t listAudioPorts(unsigned int *num_ports,
                                    struct audio_port *ports)
    {
        if (num_ports == NULL || *num_ports == 0 || ports == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(*num_ports);
        status_t status = remote()->transact(LIST_AUDIO_PORTS, data, &reply);
        if (status != NO_ERROR ||
                (status = (status_t)reply.readInt32()) != NO_ERROR) {
            return status;
        }
        *num_ports = (unsigned int)reply.readInt32();
        reply.read(ports, *num_ports * sizeof(struct audio_port));
        return status;
    }
    virtual status_t getAudioPort(struct audio_port *port)
    {
        if (port == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
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
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
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
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.write(&handle, sizeof(audio_patch_handle_t));
        status_t status = remote()->transact(RELEASE_AUDIO_PATCH, data, &reply);
        if (status != NO_ERROR) {
            status = (status_t)reply.readInt32();
        }
        return status;
    }
    virtual status_t listAudioPatches(unsigned int *num_patches,
                                      struct audio_patch *patches)
    {
        if (num_patches == NULL || *num_patches == 0 || patches == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(*num_patches);
        status_t status = remote()->transact(LIST_AUDIO_PATCHES, data, &reply);
        if (status != NO_ERROR ||
                (status = (status_t)reply.readInt32()) != NO_ERROR) {
            return status;
        }
        *num_patches = (unsigned int)reply.readInt32();
        reply.read(patches, *num_patches * sizeof(struct audio_patch));
        return status;
    }
    virtual status_t setAudioPortConfig(const struct audio_port_config *config)
    {
        if (config == NULL) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.write(config, sizeof(struct audio_port_config));
        status_t status = remote()->transact(SET_AUDIO_PORT_CONFIG, data, &reply);
        if (status != NO_ERROR) {
            status = (status_t)reply.readInt32();
        }
        return status;
    }
    virtual audio_hw_sync_t getAudioHwSyncForSession(audio_session_t sessionId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32(sessionId);
        status_t status = remote()->transact(GET_AUDIO_HW_SYNC_FOR_SESSION, data, &reply);
        if (status != NO_ERROR) {
            return AUDIO_HW_SYNC_INVALID;
        }
        return (audio_hw_sync_t)reply.readInt32();
    }
    virtual status_t systemReady()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        return remote()->transact(SYSTEM_READY, data, &reply, IBinder::FLAG_ONEWAY);
    }
    virtual size_t frameCountHAL(audio_io_handle_t ioHandle) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        data.writeInt32((int32_t) ioHandle);
        status_t status = remote()->transact(FRAME_COUNT_HAL, data, &reply);
        if (status != NO_ERROR) {
            return 0;
        }
        return reply.readInt64();
    }
    virtual status_t getMicrophones(std::vector<media::MicrophoneInfo> *microphones)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlinger::getInterfaceDescriptor());
        status_t status = remote()->transact(GET_MICROPHONES, data, &reply);
        if (status != NO_ERROR ||
                (status = (status_t)reply.readInt32()) != NO_ERROR) {
            return status;
        }
        status = reply.readParcelableVector(microphones);
        return status;
    }
};

IMPLEMENT_META_INTERFACE(AudioFlinger, "android.media.IAudioFlinger");

// ----------------------------------------------------------------------

status_t BnAudioFlinger::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    // make sure transactions reserved to AudioPolicyManager do not come from other processes
    switch (code) {
        case SET_STREAM_VOLUME:
        case SET_STREAM_MUTE:
        case OPEN_OUTPUT:
        case OPEN_DUPLICATE_OUTPUT:
        case CLOSE_OUTPUT:
        case SUSPEND_OUTPUT:
        case RESTORE_OUTPUT:
        case OPEN_INPUT:
        case CLOSE_INPUT:
        case INVALIDATE_STREAM:
        case SET_VOICE_VOLUME:
        case MOVE_EFFECTS:
        case SET_EFFECT_SUSPENDED:
        case LOAD_HW_MODULE:
        case LIST_AUDIO_PORTS:
        case GET_AUDIO_PORT:
        case CREATE_AUDIO_PATCH:
        case RELEASE_AUDIO_PATCH:
        case LIST_AUDIO_PATCHES:
        case SET_AUDIO_PORT_CONFIG:
        case SET_RECORD_SILENCED:
            ALOGW("%s: transaction %d received from PID %d",
                  __func__, code, IPCThreadState::self()->getCallingPid());
            // return status only for non void methods
            switch (code) {
                case SET_RECORD_SILENCED:
                case SET_EFFECT_SUSPENDED:
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
        case SET_MASTER_VOLUME:
        case SET_MASTER_MUTE:
        case SET_MODE:
        case SET_MIC_MUTE:
        case SET_LOW_RAM_DEVICE:
        case SYSTEM_READY: {
            if (!isServiceUid(IPCThreadState::self()->getCallingUid())) {
                ALOGW("%s: transaction %d received from PID %d unauthorized UID %d",
                      __func__, code, IPCThreadState::self()->getCallingPid(),
                      IPCThreadState::self()->getCallingUid());
                // return status only for non void methods
                switch (code) {
                    case SYSTEM_READY:
                        break;
                    default:
                        reply->writeInt32(static_cast<int32_t> (INVALID_OPERATION));
                        break;
                }
                return OK;
            }
        } break;
        default:
            break;
    }

    // Whitelist of relevant events to trigger log merging.
    // Log merging should activate during audio activity of any kind. This are considered the
    // most relevant events.
    // TODO should select more wisely the items from the list
    switch (code) {
        case CREATE_TRACK:
        case CREATE_RECORD:
        case SET_MASTER_VOLUME:
        case SET_MASTER_MUTE:
        case SET_MIC_MUTE:
        case SET_PARAMETERS:
        case CREATE_EFFECT:
        case SYSTEM_READY: {
            requestLogMerge();
            break;
        }
        default:
            break;
    }

    std::string tag("IAudioFlinger command " + std::to_string(code));
    TimeCheck check(tag.c_str());

    switch (code) {
        case CREATE_TRACK: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);

            CreateTrackInput input;
            if (input.readFromParcel((Parcel*)&data) != NO_ERROR) {
                reply->writeInt32(DEAD_OBJECT);
                return NO_ERROR;
            }

            status_t status;
            CreateTrackOutput output;

            sp<IAudioTrack> track= createTrack(input,
                                               output,
                                               &status);

            LOG_ALWAYS_FATAL_IF((track != 0) != (status == NO_ERROR));
            reply->writeInt32(status);
            if (status != NO_ERROR) {
                return NO_ERROR;
            }
            reply->writeStrongBinder(IInterface::asBinder(track));
            output.writeToParcel(reply);
            return NO_ERROR;
        } break;
        case CREATE_RECORD: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);

            CreateRecordInput input;
            if (input.readFromParcel((Parcel*)&data) != NO_ERROR) {
                reply->writeInt32(DEAD_OBJECT);
                return NO_ERROR;
            }

            status_t status;
            CreateRecordOutput output;

            sp<media::IAudioRecord> record = createRecord(input,
                                                          output,
                                                          &status);

            LOG_ALWAYS_FATAL_IF((record != 0) != (status == NO_ERROR));
            reply->writeInt32(status);
            if (status != NO_ERROR) {
                return NO_ERROR;
            }
            reply->writeStrongBinder(IInterface::asBinder(record));
            output.writeToParcel(reply);
            return NO_ERROR;
        } break;
        case SAMPLE_RATE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32( sampleRate((audio_io_handle_t) data.readInt32()) );
            return NO_ERROR;
        } break;

        // RESERVED for channelCount()

        case FORMAT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32( format((audio_io_handle_t) data.readInt32()) );
            return NO_ERROR;
        } break;
        case FRAME_COUNT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt64( frameCount((audio_io_handle_t) data.readInt32()) );
            return NO_ERROR;
        } break;
        case LATENCY: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32( latency((audio_io_handle_t) data.readInt32()) );
            return NO_ERROR;
        } break;
        case SET_MASTER_VOLUME: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32( setMasterVolume(data.readFloat()) );
            return NO_ERROR;
        } break;
        case SET_MASTER_MUTE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32( setMasterMute(data.readInt32()) );
            return NO_ERROR;
        } break;
        case MASTER_VOLUME: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeFloat( masterVolume() );
            return NO_ERROR;
        } break;
        case MASTER_MUTE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32( masterMute() );
            return NO_ERROR;
        } break;
        case SET_MASTER_BALANCE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32( setMasterBalance(data.readFloat()) );
            return NO_ERROR;
        } break;
        case GET_MASTER_BALANCE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            float f;
            const status_t status = getMasterBalance(&f);
            reply->writeInt32((int32_t)status);
            if (status == NO_ERROR) {
                (void)reply->writeFloat(f);
            }
            return NO_ERROR;
        } break;
        case SET_STREAM_VOLUME: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            int stream = data.readInt32();
            float volume = data.readFloat();
            audio_io_handle_t output = (audio_io_handle_t) data.readInt32();
            reply->writeInt32( setStreamVolume((audio_stream_type_t) stream, volume, output) );
            return NO_ERROR;
        } break;
        case SET_STREAM_MUTE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            int stream = data.readInt32();
            reply->writeInt32( setStreamMute((audio_stream_type_t) stream, data.readInt32()) );
            return NO_ERROR;
        } break;
        case STREAM_VOLUME: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            int stream = data.readInt32();
            int output = data.readInt32();
            reply->writeFloat( streamVolume((audio_stream_type_t) stream, output) );
            return NO_ERROR;
        } break;
        case STREAM_MUTE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            int stream = data.readInt32();
            reply->writeInt32( streamMute((audio_stream_type_t) stream) );
            return NO_ERROR;
        } break;
        case SET_MODE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_mode_t mode = (audio_mode_t) data.readInt32();
            reply->writeInt32( setMode(mode) );
            return NO_ERROR;
        } break;
        case SET_MIC_MUTE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            int state = data.readInt32();
            reply->writeInt32( setMicMute(state) );
            return NO_ERROR;
        } break;
        case GET_MIC_MUTE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32( getMicMute() );
            return NO_ERROR;
        } break;
        case SET_RECORD_SILENCED: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            uid_t uid = data.readInt32();
            audio_source_t source;
            data.read(&source, sizeof(audio_source_t));
            bool silenced = data.readInt32() == 1;
            setRecordSilenced(uid, silenced);
            return NO_ERROR;
        } break;
        case SET_PARAMETERS: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_io_handle_t ioHandle = (audio_io_handle_t) data.readInt32();
            String8 keyValuePairs(data.readString8());
            reply->writeInt32(setParameters(ioHandle, keyValuePairs));
            return NO_ERROR;
        } break;
        case GET_PARAMETERS: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_io_handle_t ioHandle = (audio_io_handle_t) data.readInt32();
            String8 keys(data.readString8());
            reply->writeString8(getParameters(ioHandle, keys));
            return NO_ERROR;
        } break;

        case REGISTER_CLIENT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            sp<IAudioFlingerClient> client = interface_cast<IAudioFlingerClient>(
                    data.readStrongBinder());
            registerClient(client);
            return NO_ERROR;
        } break;
        case GET_INPUTBUFFERSIZE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            uint32_t sampleRate = data.readInt32();
            audio_format_t format = (audio_format_t) data.readInt32();
            audio_channel_mask_t channelMask = data.readInt32();
            reply->writeInt64( getInputBufferSize(sampleRate, format, channelMask) );
            return NO_ERROR;
        } break;
        case OPEN_OUTPUT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_module_handle_t module = (audio_module_handle_t)data.readInt32();
            audio_config_t config = {};
            if (data.read(&config, sizeof(audio_config_t)) != NO_ERROR) {
                ALOGE("b/23905951");
            }
            audio_devices_t devices = (audio_devices_t)data.readInt32();
            String8 address(data.readString8());
            audio_output_flags_t flags = (audio_output_flags_t) data.readInt32();
            uint32_t latencyMs = 0;
            audio_io_handle_t output = AUDIO_IO_HANDLE_NONE;
            status_t status = openOutput(module, &output, &config,
                                         &devices, address, &latencyMs, flags);
            ALOGV("OPEN_OUTPUT output, %d", output);
            reply->writeInt32((int32_t)status);
            if (status == NO_ERROR) {
                reply->writeInt32((int32_t)output);
                reply->write(&config, sizeof(audio_config_t));
                reply->writeInt32(devices);
                reply->writeInt32(latencyMs);
            }
            return NO_ERROR;
        } break;
        case OPEN_DUPLICATE_OUTPUT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_io_handle_t output1 = (audio_io_handle_t) data.readInt32();
            audio_io_handle_t output2 = (audio_io_handle_t) data.readInt32();
            reply->writeInt32((int32_t) openDuplicateOutput(output1, output2));
            return NO_ERROR;
        } break;
        case CLOSE_OUTPUT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32(closeOutput((audio_io_handle_t) data.readInt32()));
            return NO_ERROR;
        } break;
        case SUSPEND_OUTPUT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32(suspendOutput((audio_io_handle_t) data.readInt32()));
            return NO_ERROR;
        } break;
        case RESTORE_OUTPUT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32(restoreOutput((audio_io_handle_t) data.readInt32()));
            return NO_ERROR;
        } break;
        case OPEN_INPUT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_module_handle_t module = (audio_module_handle_t)data.readInt32();
            audio_io_handle_t input = (audio_io_handle_t)data.readInt32();
            audio_config_t config = {};
            if (data.read(&config, sizeof(audio_config_t)) != NO_ERROR) {
                ALOGE("b/23905951");
            }
            audio_devices_t device = (audio_devices_t)data.readInt32();
            String8 address(data.readString8());
            audio_source_t source = (audio_source_t)data.readInt32();
            audio_input_flags_t flags = (audio_input_flags_t) data.readInt32();

            status_t status = openInput(module, &input, &config,
                                        &device, address, source, flags);
            reply->writeInt32((int32_t) status);
            if (status == NO_ERROR) {
                reply->writeInt32((int32_t) input);
                reply->write(&config, sizeof(audio_config_t));
                reply->writeInt32(device);
            }
            return NO_ERROR;
        } break;
        case CLOSE_INPUT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32(closeInput((audio_io_handle_t) data.readInt32()));
            return NO_ERROR;
        } break;
        case INVALIDATE_STREAM: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_stream_type_t stream = (audio_stream_type_t) data.readInt32();
            reply->writeInt32(invalidateStream(stream));
            return NO_ERROR;
        } break;
        case SET_VOICE_VOLUME: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            float volume = data.readFloat();
            reply->writeInt32( setVoiceVolume(volume) );
            return NO_ERROR;
        } break;
        case GET_RENDER_POSITION: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_io_handle_t output = (audio_io_handle_t) data.readInt32();
            uint32_t halFrames = 0;
            uint32_t dspFrames = 0;
            status_t status = getRenderPosition(&halFrames, &dspFrames, output);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->writeInt32(halFrames);
                reply->writeInt32(dspFrames);
            }
            return NO_ERROR;
        }
        case GET_INPUT_FRAMES_LOST: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_io_handle_t ioHandle = (audio_io_handle_t) data.readInt32();
            reply->writeInt32((int32_t) getInputFramesLost(ioHandle));
            return NO_ERROR;
        } break;
        case NEW_AUDIO_UNIQUE_ID: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32(newAudioUniqueId((audio_unique_id_use_t) data.readInt32()));
            return NO_ERROR;
        } break;
        case ACQUIRE_AUDIO_SESSION_ID: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_session_t audioSession = (audio_session_t) data.readInt32();
            int pid = data.readInt32();
            acquireAudioSessionId(audioSession, pid);
            return NO_ERROR;
        } break;
        case RELEASE_AUDIO_SESSION_ID: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_session_t audioSession = (audio_session_t) data.readInt32();
            int pid = data.readInt32();
            releaseAudioSessionId(audioSession, pid);
            return NO_ERROR;
        } break;
        case QUERY_NUM_EFFECTS: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            uint32_t numEffects = 0;
            status_t status = queryNumberEffects(&numEffects);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->writeInt32((int32_t)numEffects);
            }
            return NO_ERROR;
        }
        case QUERY_EFFECT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            effect_descriptor_t desc = {};
            status_t status = queryEffect(data.readInt32(), &desc);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->write(&desc, sizeof(effect_descriptor_t));
            }
            return NO_ERROR;
        }
        case GET_EFFECT_DESCRIPTOR: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            effect_uuid_t uuid = {};
            if (data.read(&uuid, sizeof(effect_uuid_t)) != NO_ERROR) {
                android_errorWriteLog(0x534e4554, "139417189");
            }
            effect_uuid_t type = {};
            if (data.read(&type, sizeof(effect_uuid_t)) != NO_ERROR) {
                android_errorWriteLog(0x534e4554, "139417189");
            }
            uint32_t preferredTypeFlag = data.readUint32();
            effect_descriptor_t desc = {};
            status_t status = getEffectDescriptor(&uuid, &type, preferredTypeFlag, &desc);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->write(&desc, sizeof(effect_descriptor_t));
            }
            return NO_ERROR;
        }
        case CREATE_EFFECT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            effect_descriptor_t desc = {};
            if (data.read(&desc, sizeof(effect_descriptor_t)) != NO_ERROR) {
                ALOGE("b/23905951");
            }
            sp<IEffectClient> client = interface_cast<IEffectClient>(data.readStrongBinder());
            int32_t priority = data.readInt32();
            audio_io_handle_t output = (audio_io_handle_t) data.readInt32();
            audio_session_t sessionId = (audio_session_t) data.readInt32();
            const String16 opPackageName = data.readString16();
            pid_t pid = (pid_t)data.readInt32();

            status_t status = NO_ERROR;
            int id = 0;
            int enabled = 0;

            sp<IEffect> effect = createEffect(&desc, client, priority, output, sessionId,
                    opPackageName, pid, &status, &id, &enabled);
            reply->writeInt32(status);
            reply->writeInt32(id);
            reply->writeInt32(enabled);
            reply->writeStrongBinder(IInterface::asBinder(effect));
            reply->write(&desc, sizeof(effect_descriptor_t));
            return NO_ERROR;
        } break;
        case MOVE_EFFECTS: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_session_t session = (audio_session_t) data.readInt32();
            audio_io_handle_t srcOutput = (audio_io_handle_t) data.readInt32();
            audio_io_handle_t dstOutput = (audio_io_handle_t) data.readInt32();
            reply->writeInt32(moveEffects(session, srcOutput, dstOutput));
            return NO_ERROR;
        } break;
        case SET_EFFECT_SUSPENDED: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            int effectId = data.readInt32();
            audio_session_t sessionId = (audio_session_t) data.readInt32();
            bool suspended = data.readInt32() == 1;
            setEffectSuspended(effectId, sessionId, suspended);
            return NO_ERROR;
        } break;
        case LOAD_HW_MODULE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32(loadHwModule(data.readCString()));
            return NO_ERROR;
        } break;
        case GET_PRIMARY_OUTPUT_SAMPLING_RATE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32(getPrimaryOutputSamplingRate());
            return NO_ERROR;
        } break;
        case GET_PRIMARY_OUTPUT_FRAME_COUNT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt64(getPrimaryOutputFrameCount());
            return NO_ERROR;
        } break;
        case SET_LOW_RAM_DEVICE: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            int32_t isLowRamDevice;
            int64_t totalMemory;
            const status_t status =
                    data.readInt32(&isLowRamDevice) ?:
                    data.readInt64(&totalMemory) ?:
                    setLowRamDevice(isLowRamDevice != 0, totalMemory);
            (void)reply->writeInt32(status);
            return NO_ERROR;
        } break;
        case LIST_AUDIO_PORTS: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            unsigned int numPortsReq = data.readInt32();
            if (numPortsReq > MAX_ITEMS_PER_LIST) {
                numPortsReq = MAX_ITEMS_PER_LIST;
            }
            unsigned int numPorts = numPortsReq;
            struct audio_port *ports =
                    (struct audio_port *)calloc(numPortsReq,
                                                           sizeof(struct audio_port));
            if (ports == NULL) {
                reply->writeInt32(NO_MEMORY);
                reply->writeInt32(0);
                return NO_ERROR;
            }
            status_t status = listAudioPorts(&numPorts, ports);
            reply->writeInt32(status);
            reply->writeInt32(numPorts);
            if (status == NO_ERROR) {
                if (numPortsReq > numPorts) {
                    numPortsReq = numPorts;
                }
                reply->write(ports, numPortsReq * sizeof(struct audio_port));
            }
            free(ports);
            return NO_ERROR;
        } break;
        case GET_AUDIO_PORT: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            struct audio_port port = {};
            if (data.read(&port, sizeof(struct audio_port)) != NO_ERROR) {
                ALOGE("b/23905951");
            }
            status_t status = getAudioPort(&port);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->write(&port, sizeof(struct audio_port));
            }
            return NO_ERROR;
        } break;
        case CREATE_AUDIO_PATCH: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            struct audio_patch patch;
            data.read(&patch, sizeof(struct audio_patch));
            audio_patch_handle_t handle = AUDIO_PATCH_HANDLE_NONE;
            if (data.read(&handle, sizeof(audio_patch_handle_t)) != NO_ERROR) {
                ALOGE("b/23905951");
            }
            status_t status = createAudioPatch(&patch, &handle);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->write(&handle, sizeof(audio_patch_handle_t));
            }
            return NO_ERROR;
        } break;
        case RELEASE_AUDIO_PATCH: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            audio_patch_handle_t handle;
            data.read(&handle, sizeof(audio_patch_handle_t));
            status_t status = releaseAudioPatch(handle);
            reply->writeInt32(status);
            return NO_ERROR;
        } break;
        case LIST_AUDIO_PATCHES: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
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
            status_t status = listAudioPatches(&numPatches, patches);
            reply->writeInt32(status);
            reply->writeInt32(numPatches);
            if (status == NO_ERROR) {
                if (numPatchesReq > numPatches) {
                    numPatchesReq = numPatches;
                }
                reply->write(patches, numPatchesReq * sizeof(struct audio_patch));
            }
            free(patches);
            return NO_ERROR;
        } break;
        case SET_AUDIO_PORT_CONFIG: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            struct audio_port_config config;
            data.read(&config, sizeof(struct audio_port_config));
            status_t status = setAudioPortConfig(&config);
            reply->writeInt32(status);
            return NO_ERROR;
        } break;
        case GET_AUDIO_HW_SYNC_FOR_SESSION: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt32(getAudioHwSyncForSession((audio_session_t) data.readInt32()));
            return NO_ERROR;
        } break;
        case SYSTEM_READY: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            systemReady();
            return NO_ERROR;
        } break;
        case FRAME_COUNT_HAL: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            reply->writeInt64( frameCountHAL((audio_io_handle_t) data.readInt32()) );
            return NO_ERROR;
        } break;
        case GET_MICROPHONES: {
            CHECK_INTERFACE(IAudioFlinger, data, reply);
            std::vector<media::MicrophoneInfo> microphones;
            status_t status = getMicrophones(&microphones);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->writeParcelableVector(microphones);
            }
            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

} // namespace android
