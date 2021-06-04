/*
**
** Copyright 2008, The Android Open Source Project
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

#include <arpa/inet.h>
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include <media/AudioResamplerPublic.h>
#include <media/AVSyncSettings.h>
#include <media/BufferingSettings.h>

#include <media/IDataSource.h>
#include <media/IMediaHTTPService.h>
#include <media/IMediaPlayer.h>
#include <media/IStreamSource.h>

#include <gui/IGraphicBufferProducer.h>
#include <utils/String8.h>

namespace android {

using media::VolumeShaper;

enum {
    DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    SET_DATA_SOURCE_URL,
    SET_DATA_SOURCE_FD,
    SET_DATA_SOURCE_STREAM,
    SET_DATA_SOURCE_CALLBACK,
    SET_BUFFERING_SETTINGS,
    GET_BUFFERING_SETTINGS,
    PREPARE_ASYNC,
    START,
    STOP,
    IS_PLAYING,
    SET_PLAYBACK_SETTINGS,
    GET_PLAYBACK_SETTINGS,
    SET_SYNC_SETTINGS,
    GET_SYNC_SETTINGS,
    PAUSE,
    SEEK_TO,
    GET_CURRENT_POSITION,
    GET_DURATION,
    RESET,
    NOTIFY_AT,
    SET_AUDIO_STREAM_TYPE,
    SET_LOOPING,
    SET_VOLUME,
    INVOKE,
    SET_METADATA_FILTER,
    GET_METADATA,
    SET_AUX_EFFECT_SEND_LEVEL,
    ATTACH_AUX_EFFECT,
    SET_VIDEO_SURFACETEXTURE,
    SET_PARAMETER,
    GET_PARAMETER,
    SET_RETRANSMIT_ENDPOINT,
    GET_RETRANSMIT_ENDPOINT,
    SET_NEXT_PLAYER,
    APPLY_VOLUME_SHAPER,
    GET_VOLUME_SHAPER_STATE,
    // Modular DRM
    PREPARE_DRM,
    RELEASE_DRM,
    // AudioRouting
    SET_OUTPUT_DEVICE,
    GET_ROUTED_DEVICE_ID,
    ENABLE_AUDIO_DEVICE_CALLBACK,
};

// ModDrm helpers
static void readVector(const Parcel& reply, Vector<uint8_t>& vector) {
    uint32_t size = reply.readUint32();
    vector.insertAt((size_t)0, size);
    reply.read(vector.editArray(), size);
}

static void writeVector(Parcel& data, Vector<uint8_t> const& vector) {
    data.writeUint32(vector.size());
    data.write(vector.array(), vector.size());
}

class BpMediaPlayer: public BpInterface<IMediaPlayer>
{
public:
    explicit BpMediaPlayer(const sp<IBinder>& impl)
        : BpInterface<IMediaPlayer>(impl)
    {
    }

    // disconnect from media player service
    void disconnect()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(DISCONNECT, data, &reply);
    }

    status_t setDataSource(
            const sp<IMediaHTTPService> &httpService,
            const char* url,
            const KeyedVector<String8, String8>* headers)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeInt32(httpService != NULL);
        if (httpService != NULL) {
            data.writeStrongBinder(IInterface::asBinder(httpService));
        }
        data.writeCString(url);
        if (headers == NULL) {
            data.writeInt32(0);
        } else {
            // serialize the headers
            data.writeInt32(headers->size());
            for (size_t i = 0; i < headers->size(); ++i) {
                data.writeString8(headers->keyAt(i));
                data.writeString8(headers->valueAt(i));
            }
        }
        remote()->transact(SET_DATA_SOURCE_URL, data, &reply);
        return reply.readInt32();
    }

    status_t setDataSource(int fd, int64_t offset, int64_t length) {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeFileDescriptor(fd);
        data.writeInt64(offset);
        data.writeInt64(length);
        remote()->transact(SET_DATA_SOURCE_FD, data, &reply);
        return reply.readInt32();
    }

    status_t setDataSource(const sp<IStreamSource> &source) {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(source));
        remote()->transact(SET_DATA_SOURCE_STREAM, data, &reply);
        return reply.readInt32();
    }

    status_t setDataSource(const sp<IDataSource> &source) {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(source));
        remote()->transact(SET_DATA_SOURCE_CALLBACK, data, &reply);
        return reply.readInt32();
    }

    // pass the buffered IGraphicBufferProducer to the media player service
    status_t setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        sp<IBinder> b(IInterface::asBinder(bufferProducer));
        data.writeStrongBinder(b);
        remote()->transact(SET_VIDEO_SURFACETEXTURE, data, &reply);
        return reply.readInt32();
    }

    status_t setBufferingSettings(const BufferingSettings& buffering)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeInt32(buffering.mInitialMarkMs);
        data.writeInt32(buffering.mResumePlaybackMarkMs);
        remote()->transact(SET_BUFFERING_SETTINGS, data, &reply);
        return reply.readInt32();
    }

    status_t getBufferingSettings(BufferingSettings* buffering /* nonnull */)
    {
        if (buffering == nullptr) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(GET_BUFFERING_SETTINGS, data, &reply);
        status_t err = reply.readInt32();
        if (err == OK) {
            buffering->mInitialMarkMs = reply.readInt32();
            buffering->mResumePlaybackMarkMs = reply.readInt32();
        }
        return err;
    }

    status_t prepareAsync()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(PREPARE_ASYNC, data, &reply);
        return reply.readInt32();
    }

    status_t start()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(START, data, &reply);
        return reply.readInt32();
    }

    status_t stop()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(STOP, data, &reply);
        return reply.readInt32();
    }

    status_t isPlaying(bool* state)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(IS_PLAYING, data, &reply);
        *state = reply.readInt32();
        return reply.readInt32();
    }

    status_t setPlaybackSettings(const AudioPlaybackRate& rate)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeFloat(rate.mSpeed);
        data.writeFloat(rate.mPitch);
        data.writeInt32((int32_t)rate.mFallbackMode);
        data.writeInt32((int32_t)rate.mStretchMode);
        remote()->transact(SET_PLAYBACK_SETTINGS, data, &reply);
        return reply.readInt32();
    }

    status_t getPlaybackSettings(AudioPlaybackRate* rate /* nonnull */)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(GET_PLAYBACK_SETTINGS, data, &reply);
        status_t err = reply.readInt32();
        if (err == OK) {
            *rate = AUDIO_PLAYBACK_RATE_DEFAULT;
            rate->mSpeed = reply.readFloat();
            rate->mPitch = reply.readFloat();
            rate->mFallbackMode = (AudioTimestretchFallbackMode)reply.readInt32();
            rate->mStretchMode = (AudioTimestretchStretchMode)reply.readInt32();
        }
        return err;
    }

    status_t setSyncSettings(const AVSyncSettings& sync, float videoFpsHint)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeInt32((int32_t)sync.mSource);
        data.writeInt32((int32_t)sync.mAudioAdjustMode);
        data.writeFloat(sync.mTolerance);
        data.writeFloat(videoFpsHint);
        remote()->transact(SET_SYNC_SETTINGS, data, &reply);
        return reply.readInt32();
    }

    status_t getSyncSettings(AVSyncSettings* sync /* nonnull */, float* videoFps /* nonnull */)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(GET_SYNC_SETTINGS, data, &reply);
        status_t err = reply.readInt32();
        if (err == OK) {
            AVSyncSettings settings;
            settings.mSource = (AVSyncSource)reply.readInt32();
            settings.mAudioAdjustMode = (AVSyncAudioAdjustMode)reply.readInt32();
            settings.mTolerance = reply.readFloat();
            *sync = settings;
            *videoFps = reply.readFloat();
        }
        return err;
    }

    status_t pause()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(PAUSE, data, &reply);
        return reply.readInt32();
    }

    status_t seekTo(int msec, MediaPlayerSeekMode mode)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeInt32(msec);
        data.writeInt32(mode);
        remote()->transact(SEEK_TO, data, &reply);
        return reply.readInt32();
    }

    status_t getCurrentPosition(int* msec)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(GET_CURRENT_POSITION, data, &reply);
        *msec = reply.readInt32();
        return reply.readInt32();
    }

    status_t getDuration(int* msec)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(GET_DURATION, data, &reply);
        *msec = reply.readInt32();
        return reply.readInt32();
    }

    status_t reset()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        remote()->transact(RESET, data, &reply);
        return reply.readInt32();
    }

    status_t notifyAt(int64_t mediaTimeUs)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeInt64(mediaTimeUs);
        remote()->transact(NOTIFY_AT, data, &reply);
        return reply.readInt32();
    }

    status_t setAudioStreamType(audio_stream_type_t stream)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeInt32((int32_t) stream);
        remote()->transact(SET_AUDIO_STREAM_TYPE, data, &reply);
        return reply.readInt32();
    }

    status_t setLooping(int loop)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeInt32(loop);
        remote()->transact(SET_LOOPING, data, &reply);
        return reply.readInt32();
    }

    status_t setVolume(float leftVolume, float rightVolume)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeFloat(leftVolume);
        data.writeFloat(rightVolume);
        remote()->transact(SET_VOLUME, data, &reply);
        return reply.readInt32();
    }

    status_t invoke(const Parcel& request, Parcel *reply)
    {
        // Avoid doing any extra copy. The interface descriptor should
        // have been set by MediaPlayer.java.
        return remote()->transact(INVOKE, request, reply);
    }

    status_t setMetadataFilter(const Parcel& request)
    {
        Parcel reply;
        // Avoid doing any extra copy of the request. The interface
        // descriptor should have been set by MediaPlayer.java.
        remote()->transact(SET_METADATA_FILTER, request, &reply);
        return reply.readInt32();
    }

    status_t getMetadata(bool update_only, bool apply_filter, Parcel *reply)
    {
        Parcel request;
        request.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        // TODO: Burning 2 ints for 2 boolean. Should probably use flags in an int here.
        request.writeInt32(update_only);
        request.writeInt32(apply_filter);
        remote()->transact(GET_METADATA, request, reply);
        return reply->readInt32();
    }

    status_t setAuxEffectSendLevel(float level)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeFloat(level);
        remote()->transact(SET_AUX_EFFECT_SEND_LEVEL, data, &reply);
        return reply.readInt32();
    }

    status_t attachAuxEffect(int effectId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeInt32(effectId);
        remote()->transact(ATTACH_AUX_EFFECT, data, &reply);
        return reply.readInt32();
    }

    status_t setParameter(int key, const Parcel& request)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeInt32(key);
        if (request.dataSize() > 0) {
            data.appendFrom(const_cast<Parcel *>(&request), 0, request.dataSize());
        }
        remote()->transact(SET_PARAMETER, data, &reply);
        return reply.readInt32();
    }

    status_t getParameter(int key, Parcel *reply)
    {
        Parcel data;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        data.writeInt32(key);
        return remote()->transact(GET_PARAMETER, data, reply);
    }

    status_t setRetransmitEndpoint(const struct sockaddr_in* endpoint)
    {
        Parcel data, reply;
        status_t err;

        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        if (NULL != endpoint) {
            data.writeInt32(sizeof(*endpoint));
            data.write(endpoint, sizeof(*endpoint));
        } else {
            data.writeInt32(0);
        }

        err = remote()->transact(SET_RETRANSMIT_ENDPOINT, data, &reply);
        if (OK != err) {
            return err;
        }
        return reply.readInt32();
    }

    status_t setNextPlayer(const sp<IMediaPlayer>& player) {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        sp<IBinder> b(IInterface::asBinder(player));
        data.writeStrongBinder(b);
        remote()->transact(SET_NEXT_PLAYER, data, &reply);
        return reply.readInt32();
    }

    status_t getRetransmitEndpoint(struct sockaddr_in* endpoint)
    {
        Parcel data, reply;
        status_t err;

        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
        err = remote()->transact(GET_RETRANSMIT_ENDPOINT, data, &reply);

        if ((OK != err) || (OK != (err = reply.readInt32()))) {
            return err;
        }

        data.read(endpoint, sizeof(*endpoint));

        return err;
    }

    virtual VolumeShaper::Status applyVolumeShaper(
            const sp<VolumeShaper::Configuration>& configuration,
            const sp<VolumeShaper::Operation>& operation) {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());

        status_t tmp;
        status_t status = configuration.get() == nullptr
                ? data.writeInt32(0)
                : (tmp = data.writeInt32(1)) != NO_ERROR
                    ? tmp : configuration->writeToParcel(&data);
        if (status != NO_ERROR) {
            return VolumeShaper::Status(status);
        }

        status = operation.get() == nullptr
                ? status = data.writeInt32(0)
                : (tmp = data.writeInt32(1)) != NO_ERROR
                    ? tmp : operation->writeToParcel(&data);
        if (status != NO_ERROR) {
            return VolumeShaper::Status(status);
        }

        int32_t remoteVolumeShaperStatus;
        status = remote()->transact(APPLY_VOLUME_SHAPER, data, &reply);
        if (status == NO_ERROR) {
            status = reply.readInt32(&remoteVolumeShaperStatus);
        }
        if (status != NO_ERROR) {
            return VolumeShaper::Status(status);
        }
        return VolumeShaper::Status(remoteVolumeShaperStatus);
    }

    virtual sp<VolumeShaper::State> getVolumeShaperState(int id) {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());

        data.writeInt32(id);
        status_t status = remote()->transact(GET_VOLUME_SHAPER_STATE, data, &reply);
        if (status != NO_ERROR) {
            return nullptr;
        }
        sp<VolumeShaper::State> state = new VolumeShaper::State();
        status = state->readFromParcel(&reply);
        if (status != NO_ERROR) {
            return nullptr;
        }
        return state;
    }

    // Modular DRM
    status_t prepareDrm(const uint8_t uuid[16], const Vector<uint8_t>& drmSessionId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());

        data.write(uuid, 16);
        writeVector(data, drmSessionId);

        status_t status = remote()->transact(PREPARE_DRM, data, &reply);
        if (status != OK) {
            ALOGE("prepareDrm: binder call failed: %d", status);
            return status;
        }

        return reply.readInt32();
    }

    status_t releaseDrm()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());

        status_t status = remote()->transact(RELEASE_DRM, data, &reply);
        if (status != OK) {
            ALOGE("releaseDrm: binder call failed: %d", status);
            return status;
        }

        return reply.readInt32();
    }

    status_t setOutputDevice(audio_port_handle_t deviceId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());

        data.writeInt32(deviceId);

        status_t status = remote()->transact(SET_OUTPUT_DEVICE, data, &reply);
        if (status != OK) {
            ALOGE("setOutputDevice: binder call failed: %d", status);
            return status;
        }

        return reply.readInt32();
    }

    status_t getRoutedDeviceId(audio_port_handle_t* deviceId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());

        status_t status = remote()->transact(GET_ROUTED_DEVICE_ID, data, &reply);
        if (status != OK) {
            ALOGE("getRoutedDeviceid: binder call failed: %d", status);
            *deviceId = AUDIO_PORT_HANDLE_NONE;
            return status;
        }

        status = reply.readInt32();
        if (status != NO_ERROR) {
            *deviceId = AUDIO_PORT_HANDLE_NONE;
        } else {
            *deviceId = reply.readInt32();
        }
        return status;
    }

    status_t enableAudioDeviceCallback(bool enabled)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());

        data.writeBool(enabled);

        status_t status = remote()->transact(ENABLE_AUDIO_DEVICE_CALLBACK, data, &reply);
        if (status != OK) {
            ALOGE("enableAudioDeviceCallback: binder call failed: %d, %d", enabled, status);
            return status;
        }

        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(MediaPlayer, "android.media.IMediaPlayer");

// ----------------------------------------------------------------------

status_t BnMediaPlayer::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case DISCONNECT: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            disconnect();
            return NO_ERROR;
        } break;
        case SET_DATA_SOURCE_URL: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);

            sp<IMediaHTTPService> httpService;
            if (data.readInt32()) {
                httpService =
                    interface_cast<IMediaHTTPService>(data.readStrongBinder());
            }

            const char* url = data.readCString();
            if (url == NULL) {
                reply->writeInt32(BAD_VALUE);
                return NO_ERROR;
            }
            KeyedVector<String8, String8> headers;
            int32_t numHeaders = data.readInt32();
            for (int i = 0; i < numHeaders; ++i) {
                String8 key = data.readString8();
                String8 value = data.readString8();
                headers.add(key, value);
            }
            reply->writeInt32(setDataSource(
                        httpService, url, numHeaders > 0 ? &headers : NULL));
            return NO_ERROR;
        } break;
        case SET_DATA_SOURCE_FD: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            int fd = data.readFileDescriptor();
            int64_t offset = data.readInt64();
            int64_t length = data.readInt64();
            reply->writeInt32(setDataSource(fd, offset, length));
            return NO_ERROR;
        }
        case SET_DATA_SOURCE_STREAM: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            sp<IStreamSource> source =
                interface_cast<IStreamSource>(data.readStrongBinder());
            if (source == NULL) {
                reply->writeInt32(BAD_VALUE);
            } else {
                reply->writeInt32(setDataSource(source));
            }
            return NO_ERROR;
        }
        case SET_DATA_SOURCE_CALLBACK: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            sp<IDataSource> source =
                interface_cast<IDataSource>(data.readStrongBinder());
            if (source == NULL) {
                reply->writeInt32(BAD_VALUE);
            } else {
                reply->writeInt32(setDataSource(source));
            }
            return NO_ERROR;
        }
        case SET_VIDEO_SURFACETEXTURE: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            sp<IGraphicBufferProducer> bufferProducer =
                    interface_cast<IGraphicBufferProducer>(data.readStrongBinder());
            reply->writeInt32(setVideoSurfaceTexture(bufferProducer));
            return NO_ERROR;
        } break;
        case SET_BUFFERING_SETTINGS: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            BufferingSettings buffering;
            buffering.mInitialMarkMs = data.readInt32();
            buffering.mResumePlaybackMarkMs = data.readInt32();
            reply->writeInt32(setBufferingSettings(buffering));
            return NO_ERROR;
        } break;
        case GET_BUFFERING_SETTINGS: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            BufferingSettings buffering;
            status_t err = getBufferingSettings(&buffering);
            reply->writeInt32(err);
            if (err == OK) {
                reply->writeInt32(buffering.mInitialMarkMs);
                reply->writeInt32(buffering.mResumePlaybackMarkMs);
            }
            return NO_ERROR;
        } break;
        case PREPARE_ASYNC: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(prepareAsync());
            return NO_ERROR;
        } break;
        case START: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(start());
            return NO_ERROR;
        } break;
        case STOP: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(stop());
            return NO_ERROR;
        } break;
        case IS_PLAYING: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            bool state;
            status_t ret = isPlaying(&state);
            reply->writeInt32(state);
            reply->writeInt32(ret);
            return NO_ERROR;
        } break;
        case SET_PLAYBACK_SETTINGS: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            AudioPlaybackRate rate = AUDIO_PLAYBACK_RATE_DEFAULT;
            rate.mSpeed = data.readFloat();
            rate.mPitch = data.readFloat();
            rate.mFallbackMode = (AudioTimestretchFallbackMode)data.readInt32();
            rate.mStretchMode = (AudioTimestretchStretchMode)data.readInt32();
            reply->writeInt32(setPlaybackSettings(rate));
            return NO_ERROR;
        } break;
        case GET_PLAYBACK_SETTINGS: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            AudioPlaybackRate rate = AUDIO_PLAYBACK_RATE_DEFAULT;
            status_t err = getPlaybackSettings(&rate);
            reply->writeInt32(err);
            if (err == OK) {
                reply->writeFloat(rate.mSpeed);
                reply->writeFloat(rate.mPitch);
                reply->writeInt32((int32_t)rate.mFallbackMode);
                reply->writeInt32((int32_t)rate.mStretchMode);
            }
            return NO_ERROR;
        } break;
        case SET_SYNC_SETTINGS: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            AVSyncSettings sync;
            sync.mSource = (AVSyncSource)data.readInt32();
            sync.mAudioAdjustMode = (AVSyncAudioAdjustMode)data.readInt32();
            sync.mTolerance = data.readFloat();
            float videoFpsHint = data.readFloat();
            reply->writeInt32(setSyncSettings(sync, videoFpsHint));
            return NO_ERROR;
        } break;
        case GET_SYNC_SETTINGS: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            AVSyncSettings sync;
            float videoFps;
            status_t err = getSyncSettings(&sync, &videoFps);
            reply->writeInt32(err);
            if (err == OK) {
                reply->writeInt32((int32_t)sync.mSource);
                reply->writeInt32((int32_t)sync.mAudioAdjustMode);
                reply->writeFloat(sync.mTolerance);
                reply->writeFloat(videoFps);
            }
            return NO_ERROR;
        } break;
        case PAUSE: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(pause());
            return NO_ERROR;
        } break;
        case SEEK_TO: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            int msec = data.readInt32();
            MediaPlayerSeekMode mode = (MediaPlayerSeekMode)data.readInt32();
            reply->writeInt32(seekTo(msec, mode));
            return NO_ERROR;
        } break;
        case GET_CURRENT_POSITION: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            int msec = 0;
            status_t ret = getCurrentPosition(&msec);
            reply->writeInt32(msec);
            reply->writeInt32(ret);
            return NO_ERROR;
        } break;
        case GET_DURATION: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            int msec = 0;
            status_t ret = getDuration(&msec);
            reply->writeInt32(msec);
            reply->writeInt32(ret);
            return NO_ERROR;
        } break;
        case RESET: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(reset());
            return NO_ERROR;
        } break;
        case NOTIFY_AT: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(notifyAt(data.readInt64()));
            return NO_ERROR;
        } break;
        case SET_AUDIO_STREAM_TYPE: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(setAudioStreamType((audio_stream_type_t) data.readInt32()));
            return NO_ERROR;
        } break;
        case SET_LOOPING: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(setLooping(data.readInt32()));
            return NO_ERROR;
        } break;
        case SET_VOLUME: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            float leftVolume = data.readFloat();
            float rightVolume = data.readFloat();
            reply->writeInt32(setVolume(leftVolume, rightVolume));
            return NO_ERROR;
        } break;
        case INVOKE: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            status_t result = invoke(data, reply);
            return result;
        } break;
        case SET_METADATA_FILTER: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(setMetadataFilter(data));
            return NO_ERROR;
        } break;
        case GET_METADATA: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            bool update_only = static_cast<bool>(data.readInt32());
            bool apply_filter = static_cast<bool>(data.readInt32());
            const status_t retcode = getMetadata(update_only, apply_filter, reply);
            reply->setDataPosition(0);
            reply->writeInt32(retcode);
            reply->setDataPosition(0);
            return NO_ERROR;
        } break;
        case SET_AUX_EFFECT_SEND_LEVEL: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(setAuxEffectSendLevel(data.readFloat()));
            return NO_ERROR;
        } break;
        case ATTACH_AUX_EFFECT: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(attachAuxEffect(data.readInt32()));
            return NO_ERROR;
        } break;
        case SET_PARAMETER: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            int key = data.readInt32();

            Parcel request;
            if (data.dataAvail() > 0) {
                request.appendFrom(
                        const_cast<Parcel *>(&data), data.dataPosition(), data.dataAvail());
            }
            request.setDataPosition(0);
            reply->writeInt32(setParameter(key, request));
            return NO_ERROR;
        } break;
        case GET_PARAMETER: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            return getParameter(data.readInt32(), reply);
        } break;
        case SET_RETRANSMIT_ENDPOINT: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);

            struct sockaddr_in endpoint;
            memset(&endpoint, 0, sizeof(endpoint));
            int amt = data.readInt32();
            if (amt == sizeof(endpoint)) {
                data.read(&endpoint, sizeof(struct sockaddr_in));
                reply->writeInt32(setRetransmitEndpoint(&endpoint));
            } else {
                reply->writeInt32(setRetransmitEndpoint(NULL));
            }

            return NO_ERROR;
        } break;
        case GET_RETRANSMIT_ENDPOINT: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);

            struct sockaddr_in endpoint;
            memset(&endpoint, 0, sizeof(endpoint));
            status_t res = getRetransmitEndpoint(&endpoint);

            reply->writeInt32(res);
            reply->write(&endpoint, sizeof(endpoint));

            return NO_ERROR;
        } break;
        case SET_NEXT_PLAYER: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            reply->writeInt32(setNextPlayer(interface_cast<IMediaPlayer>(data.readStrongBinder())));

            return NO_ERROR;
        } break;

        case APPLY_VOLUME_SHAPER: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            sp<VolumeShaper::Configuration> configuration;
            sp<VolumeShaper::Operation> operation;

            int32_t present;
            status_t status = data.readInt32(&present);
            if (status == NO_ERROR && present != 0) {
                configuration = new VolumeShaper::Configuration();
                status = configuration->readFromParcel(&data);
            }
            if (status == NO_ERROR) {
                status = data.readInt32(&present);
            }
            if (status == NO_ERROR && present != 0) {
                operation = new VolumeShaper::Operation();
                status = operation->readFromParcel(&data);
            }
            if (status == NO_ERROR) {
                status = (status_t)applyVolumeShaper(configuration, operation);
            }
            reply->writeInt32(status);
            return NO_ERROR;
        } break;
        case GET_VOLUME_SHAPER_STATE: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            int id;
            status_t status = data.readInt32(&id);
            if (status == NO_ERROR) {
                sp<VolumeShaper::State> state = getVolumeShaperState(id);
                if (state.get() != nullptr) {
                     status = state->writeToParcel(reply);
                }
            }
            return NO_ERROR;
        } break;

        // Modular DRM
        case PREPARE_DRM: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);

            uint8_t uuid[16];
            data.read(uuid, sizeof(uuid));
            Vector<uint8_t> drmSessionId;
            readVector(data, drmSessionId);

            uint32_t result = prepareDrm(uuid, drmSessionId);
            reply->writeInt32(result);
            return OK;
        }
        case RELEASE_DRM: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);

            uint32_t result = releaseDrm();
            reply->writeInt32(result);
            return OK;
        }

        // AudioRouting
        case SET_OUTPUT_DEVICE: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            int deviceId;
            status_t status = data.readInt32(&deviceId);
            if (status == NO_ERROR) {
                reply->writeInt32(setOutputDevice(deviceId));
            } else {
                reply->writeInt32(BAD_VALUE);
            }
            return NO_ERROR;
        }
        case GET_ROUTED_DEVICE_ID: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            audio_port_handle_t deviceId;
            status_t ret = getRoutedDeviceId(&deviceId);
            reply->writeInt32(ret);
            if (ret == NO_ERROR) {
                reply->writeInt32(deviceId);
            }
            return NO_ERROR;
        } break;
        case ENABLE_AUDIO_DEVICE_CALLBACK: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            bool enabled;
            status_t status = data.readBool(&enabled);
            if (status == NO_ERROR) {
                reply->writeInt32(enableAudioDeviceCallback(enabled));
            } else {
                reply->writeInt32(BAD_VALUE);
            }
            return NO_ERROR;
        } break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

} // namespace android
