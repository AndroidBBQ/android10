/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "IAudioManager"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <audiomanager/AudioManager.h>
#include <audiomanager/IAudioManager.h>

namespace android {

class BpAudioManager : public BpInterface<IAudioManager>
{
public:
    explicit BpAudioManager(const sp<IBinder>& impl)
        : BpInterface<IAudioManager>(impl)
    {
    }

    virtual audio_unique_id_t trackPlayer(player_type_t playerType, audio_usage_t usage,
            audio_content_type_t content, const sp<IBinder>& player) {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioManager::getInterfaceDescriptor());
        data.writeInt32(1); // non-null PlayerIdCard parcelable
        // marshall PlayerIdCard data
        data.writeInt32((int32_t) playerType);
        //   write attributes of PlayerIdCard
        data.writeInt32((int32_t) usage);
        data.writeInt32((int32_t) content);
        data.writeInt32(0 /*source: none here, this is a player*/);
        data.writeInt32(0 /*flags*/);
        //   write attributes' tags
        data.writeInt32(1 /*FLATTEN_TAGS*/);
        data.writeString16(String16("")); // no tags
        //   write attributes' bundle
        data.writeInt32(-1977 /*ATTR_PARCEL_IS_NULL_BUNDLE*/); // no bundle
        //   write IPlayer
        data.writeStrongBinder(player);
        // get new PIId in reply
        const status_t res = remote()->transact(TRACK_PLAYER, data, &reply, 0);
        if (res != OK || reply.readExceptionCode() != 0) {
            ALOGE("trackPlayer() failed, piid is %d", PLAYER_PIID_INVALID);
            return PLAYER_PIID_INVALID;
        } else {
            const audio_unique_id_t piid = (audio_unique_id_t) reply.readInt32();
            ALOGV("trackPlayer() returned piid %d", piid);
            return piid;
        }
    }

    virtual status_t playerAttributes(audio_unique_id_t piid, audio_usage_t usage,
            audio_content_type_t content) {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioManager::getInterfaceDescriptor());
        data.writeInt32((int32_t) piid);
        data.writeInt32(1); // non-null AudioAttributes parcelable
        data.writeInt32((int32_t) usage);
        data.writeInt32((int32_t) content);
        data.writeInt32(0 /*source: none here, this is a player*/);
        data.writeInt32(0 /*flags*/);
        //   write attributes' tags
        data.writeInt32(1 /*FLATTEN_TAGS*/);
        data.writeString16(String16("")); // no tags
        //   write attributes' bundle
        data.writeInt32(-1977 /*ATTR_PARCEL_IS_NULL_BUNDLE*/); // no bundle
        return remote()->transact(PLAYER_ATTRIBUTES, data, &reply, IBinder::FLAG_ONEWAY);
    }

    virtual status_t playerEvent(audio_unique_id_t piid, player_state_t event) {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioManager::getInterfaceDescriptor());
        data.writeInt32((int32_t) piid);
        data.writeInt32((int32_t) event);
        return remote()->transact(PLAYER_EVENT, data, &reply, IBinder::FLAG_ONEWAY);
    }

    virtual status_t releasePlayer(audio_unique_id_t piid) {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioManager::getInterfaceDescriptor());
        data.writeInt32((int32_t) piid);
        return remote()->transact(RELEASE_PLAYER, data, &reply, IBinder::FLAG_ONEWAY);
    }

    virtual audio_unique_id_t trackRecorder(const sp<IBinder>& recorder) {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioManager::getInterfaceDescriptor());
        data.writeStrongBinder(recorder);
        // get new RIId in reply
        const status_t res = remote()->transact(TRACK_RECORDER, data, &reply, 0);
        if (res != OK || reply.readExceptionCode() != 0) {
            ALOGE("trackRecorder() failed, riid is %d", RECORD_RIID_INVALID);
            return RECORD_RIID_INVALID;
        } else {
            const audio_unique_id_t riid = (audio_unique_id_t) reply.readInt32();
            ALOGV("trackRecorder() returned riid %d", riid);
            return riid;
        }
    }

    virtual status_t recorderEvent(audio_unique_id_t riid, recorder_state_t event) {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioManager::getInterfaceDescriptor());
        data.writeInt32((int32_t) riid);
        data.writeInt32((int32_t) event);
        return remote()->transact(RECORDER_EVENT, data, &reply, IBinder::FLAG_ONEWAY);
    }

    virtual status_t releaseRecorder(audio_unique_id_t riid) {
        Parcel data, reply;
        data.writeInterfaceToken(IAudioManager::getInterfaceDescriptor());
        data.writeInt32((int32_t) riid);
        return remote()->transact(RELEASE_RECORDER, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(AudioManager, "android.media.IAudioService");

// ----------------------------------------------------------------------------

}; // namespace android
