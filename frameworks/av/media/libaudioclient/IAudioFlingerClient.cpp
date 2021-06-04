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

#define LOG_TAG "IAudioFlingerClient"
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include <media/IAudioFlingerClient.h>
#include <media/AudioSystem.h>

namespace android {

enum {
    IO_CONFIG_CHANGED = IBinder::FIRST_CALL_TRANSACTION
};

class BpAudioFlingerClient : public BpInterface<IAudioFlingerClient>
{
public:
    explicit BpAudioFlingerClient(const sp<IBinder>& impl)
        : BpInterface<IAudioFlingerClient>(impl)
    {
    }

    void ioConfigChanged(audio_io_config_event event, const sp<AudioIoDescriptor>& ioDesc)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioFlingerClient::getInterfaceDescriptor());
        data.writeInt32(event);
        data.writeInt32((int32_t)ioDesc->mIoHandle);
        data.write(&ioDesc->mPatch, sizeof(struct audio_patch));
        data.writeInt32(ioDesc->mSamplingRate);
        data.writeInt32(ioDesc->mFormat);
        data.writeInt32(ioDesc->mChannelMask);
        data.writeInt64(ioDesc->mFrameCount);
        data.writeInt64(ioDesc->mFrameCountHAL);
        data.writeInt32(ioDesc->mLatency);
        data.writeInt32(ioDesc->mPortId);
        remote()->transact(IO_CONFIG_CHANGED, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(AudioFlingerClient, "android.media.IAudioFlingerClient");

// ----------------------------------------------------------------------

status_t BnAudioFlingerClient::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
    case IO_CONFIG_CHANGED: {
            CHECK_INTERFACE(IAudioFlingerClient, data, reply);
            audio_io_config_event event = (audio_io_config_event)data.readInt32();
            sp<AudioIoDescriptor> ioDesc = new AudioIoDescriptor();
            ioDesc->mIoHandle = (audio_io_handle_t) data.readInt32();
            data.read(&ioDesc->mPatch, sizeof(struct audio_patch));
            ioDesc->mSamplingRate = data.readInt32();
            ioDesc->mFormat = (audio_format_t) data.readInt32();
            ioDesc->mChannelMask = (audio_channel_mask_t) data.readInt32();
            ioDesc->mFrameCount = data.readInt64();
            ioDesc->mFrameCountHAL = data.readInt64();
            ioDesc->mLatency = data.readInt32();
            ioDesc->mPortId = data.readInt32();
            ioConfigChanged(event, ioDesc);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

} // namespace android
