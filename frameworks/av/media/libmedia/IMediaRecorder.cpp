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

//#define LOG_NDEBUG 0
#define LOG_TAG "IMediaRecorder"

#include <inttypes.h>
#include <unistd.h>

#include <utils/Log.h>
#include <binder/Parcel.h>
#include <camera/android/hardware/ICamera.h>
#include <camera/ICameraRecordingProxy.h>
#include <media/IMediaRecorderClient.h>
#include <media/IMediaRecorder.h>
#include <gui/Surface.h>
#include <gui/IGraphicBufferProducer.h>
#include <media/stagefright/PersistentSurface.h>

namespace android {

enum {
    RELEASE = IBinder::FIRST_CALL_TRANSACTION,
    INIT,
    CLOSE,
    SET_INPUT_SURFACE,
    QUERY_SURFACE_MEDIASOURCE,
    RESET,
    STOP,
    START,
    PREPARE,
    GET_MAX_AMPLITUDE,
    SET_VIDEO_SOURCE,
    SET_AUDIO_SOURCE,
    SET_OUTPUT_FORMAT,
    SET_VIDEO_ENCODER,
    SET_AUDIO_ENCODER,
    SET_OUTPUT_FILE_FD,
    SET_NEXT_OUTPUT_FILE_FD,
    SET_VIDEO_SIZE,
    SET_VIDEO_FRAMERATE,
    SET_PARAMETERS,
    SET_PREVIEW_SURFACE,
    SET_CAMERA,
    SET_LISTENER,
    SET_CLIENT_NAME,
    PAUSE,
    RESUME,
    GET_METRICS,
    SET_INPUT_DEVICE,
    GET_ROUTED_DEVICE_ID,
    ENABLE_AUDIO_DEVICE_CALLBACK,
    GET_ACTIVE_MICROPHONES,
    GET_PORT_ID,
    SET_PREFERRED_MICROPHONE_DIRECTION,
    SET_PREFERRED_MICROPHONE_FIELD_DIMENSION
};

class BpMediaRecorder: public BpInterface<IMediaRecorder>
{
public:
    explicit BpMediaRecorder(const sp<IBinder>& impl)
    : BpInterface<IMediaRecorder>(impl)
    {
    }

    status_t setCamera(const sp<hardware::ICamera>& camera, const sp<ICameraRecordingProxy>& proxy)
    {
        ALOGV("setCamera(%p,%p)", camera.get(), proxy.get());
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(camera));
        data.writeStrongBinder(IInterface::asBinder(proxy));
        remote()->transact(SET_CAMERA, data, &reply);
        return reply.readInt32();
    }

    status_t setInputSurface(const sp<PersistentSurface>& surface)
    {
        ALOGV("setInputSurface(%p)", surface.get());
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        surface->writeToParcel(&data);
        remote()->transact(SET_INPUT_SURFACE, data, &reply);
        return reply.readInt32();
    }

    sp<IGraphicBufferProducer> querySurfaceMediaSource()
    {
        ALOGV("Query SurfaceMediaSource");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(QUERY_SURFACE_MEDIASOURCE, data, &reply);
        int returnedNull = reply.readInt32();
        if (returnedNull) {
            return NULL;
        }
        return interface_cast<IGraphicBufferProducer>(reply.readStrongBinder());
    }

    status_t setPreviewSurface(const sp<IGraphicBufferProducer>& surface)
    {
        ALOGV("setPreviewSurface(%p)", surface.get());
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(surface));
        remote()->transact(SET_PREVIEW_SURFACE, data, &reply);
        return reply.readInt32();
    }

    status_t init()
    {
        ALOGV("init");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(INIT, data, &reply);
        return reply.readInt32();
    }

    status_t setVideoSource(int vs)
    {
        ALOGV("setVideoSource(%d)", vs);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeInt32(vs);
        remote()->transact(SET_VIDEO_SOURCE, data, &reply);
        return reply.readInt32();
    }

    status_t setAudioSource(int as)
    {
        ALOGV("setAudioSource(%d)", as);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeInt32(as);
        remote()->transact(SET_AUDIO_SOURCE, data, &reply);
        return reply.readInt32();
    }

    status_t setOutputFormat(int of)
    {
        ALOGV("setOutputFormat(%d)", of);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeInt32(of);
        remote()->transact(SET_OUTPUT_FORMAT, data, &reply);
        return reply.readInt32();
    }

    status_t setVideoEncoder(int ve)
    {
        ALOGV("setVideoEncoder(%d)", ve);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeInt32(ve);
        remote()->transact(SET_VIDEO_ENCODER, data, &reply);
        return reply.readInt32();
    }

    status_t setAudioEncoder(int ae)
    {
        ALOGV("setAudioEncoder(%d)", ae);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeInt32(ae);
        remote()->transact(SET_AUDIO_ENCODER, data, &reply);
        return reply.readInt32();
    }

    status_t setOutputFile(int fd) {
        ALOGV("setOutputFile(%d)", fd);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeFileDescriptor(fd);
        remote()->transact(SET_OUTPUT_FILE_FD, data, &reply);
        return reply.readInt32();
    }

    status_t setNextOutputFile(int fd) {
        ALOGV("setNextOutputFile(%d)", fd);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeFileDescriptor(fd);
        remote()->transact(SET_NEXT_OUTPUT_FILE_FD, data, &reply);
        return reply.readInt32();
    }

    status_t setVideoSize(int width, int height)
    {
        ALOGV("setVideoSize(%dx%d)", width, height);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeInt32(width);
        data.writeInt32(height);
        remote()->transact(SET_VIDEO_SIZE, data, &reply);
        return reply.readInt32();
    }

    status_t setVideoFrameRate(int frames_per_second)
    {
        ALOGV("setVideoFrameRate(%d)", frames_per_second);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeInt32(frames_per_second);
        remote()->transact(SET_VIDEO_FRAMERATE, data, &reply);
        return reply.readInt32();
    }

    status_t setParameters(const String8& params)
    {
        ALOGV("setParameter(%s)", params.string());
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeString8(params);
        remote()->transact(SET_PARAMETERS, data, &reply);
        return reply.readInt32();
    }

    status_t setListener(const sp<IMediaRecorderClient>& listener)
    {
        ALOGV("setListener(%p)", listener.get());
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(listener));
        remote()->transact(SET_LISTENER, data, &reply);
        return reply.readInt32();
    }

    status_t setClientName(const String16& clientName)
    {
        ALOGV("setClientName(%s)", String8(clientName).string());
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeString16(clientName);
        remote()->transact(SET_CLIENT_NAME, data, &reply);
        return reply.readInt32();
    }

    status_t prepare()
    {
        ALOGV("prepare");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(PREPARE, data, &reply);
        return reply.readInt32();
    }

    status_t getMaxAmplitude(int* max)
    {
        ALOGV("getMaxAmplitude");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(GET_MAX_AMPLITUDE, data, &reply);
        *max = reply.readInt32();
        return reply.readInt32();
    }

    status_t getMetrics(Parcel* reply)
    {
        ALOGV("getMetrics");
        Parcel data;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        status_t ret = remote()->transact(GET_METRICS, data, reply);
        if (ret == NO_ERROR) {
            return OK;
        }
        return UNKNOWN_ERROR;
    }

    status_t start()
    {
        ALOGV("start");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(START, data, &reply);
        return reply.readInt32();
    }

    status_t stop()
    {
        ALOGV("stop");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(STOP, data, &reply);
        return reply.readInt32();
    }

    status_t reset()
    {
        ALOGV("reset");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(RESET, data, &reply);
        return reply.readInt32();
    }

    status_t pause()
    {
        ALOGV("pause");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(PAUSE, data, &reply);
        return reply.readInt32();
    }

    status_t resume()
    {
        ALOGV("resume");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(RESUME, data, &reply);
        return reply.readInt32();
    }

    status_t close()
    {
        ALOGV("close");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(CLOSE, data, &reply);
        return reply.readInt32();
    }

    status_t release()
    {
        ALOGV("release");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        remote()->transact(RELEASE, data, &reply);
        return reply.readInt32();
    }

    status_t setInputDevice(audio_port_handle_t deviceId)
    {
        ALOGV("setInputDevice");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeInt32(deviceId);

        status_t status = remote()->transact(SET_INPUT_DEVICE, data, &reply);
        if (status != OK) {
            ALOGE("setInputDevice binder call failed: %d", status);
            return status;
        }
        return reply.readInt32();;
    }

    audio_port_handle_t getRoutedDeviceId(audio_port_handle_t *deviceId)
    {
        ALOGV("getRoutedDeviceId");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());

        status_t status = remote()->transact(GET_ROUTED_DEVICE_ID, data, &reply);
        if (status != OK) {
            ALOGE("getRoutedDeviceid binder call failed: %d", status);
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
        ALOGV("enableAudioDeviceCallback");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeBool(enabled);
        status_t status = remote()->transact(ENABLE_AUDIO_DEVICE_CALLBACK, data, &reply);
        if (status != OK) {
            ALOGE("enableAudioDeviceCallback binder call failed: %d, %d", enabled, status);
            return status;
        }
        return reply.readInt32();
    }

    status_t getActiveMicrophones(std::vector<media::MicrophoneInfo>* activeMicrophones)
    {
        ALOGV("getActiveMicrophones");
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        status_t status = remote()->transact(GET_ACTIVE_MICROPHONES, data, &reply);
        if (status != OK
                || (status = (status_t)reply.readInt32()) != NO_ERROR) {
            return status;
        }
        status = reply.readParcelableVector(activeMicrophones);
        return status;
    }

    status_t setPreferredMicrophoneDirection(audio_microphone_direction_t direction) {
        ALOGV("setPreferredMicrophoneDirection(%d)", direction);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeInt32(direction);
        status_t status = remote()->transact(SET_PREFERRED_MICROPHONE_DIRECTION, data, &reply);
        return status == NO_ERROR ? (status_t)reply.readInt32() : status;
    }

    status_t setPreferredMicrophoneFieldDimension(float zoom) {
        ALOGV("setPreferredMicrophoneFieldDimension(%f)", zoom);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        data.writeFloat(zoom);
        status_t status = remote()->transact(SET_PREFERRED_MICROPHONE_FIELD_DIMENSION, data, &reply);
        return status == NO_ERROR ? (status_t)reply.readInt32() : status;
    }

    status_t getPortId(audio_port_handle_t *portId)
    {
        ALOGV("getPortId");
        if (portId == nullptr) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IMediaRecorder::getInterfaceDescriptor());
        status_t status = remote()->transact(GET_PORT_ID, data, &reply);
        if (status != OK
                || (status = (status_t)reply.readInt32()) != NO_ERROR) {
            *portId = AUDIO_PORT_HANDLE_NONE;
            return status;
        }
        *portId = (audio_port_handle_t)reply.readInt32();
        return NO_ERROR;
    }
};

IMPLEMENT_META_INTERFACE(MediaRecorder, "android.media.IMediaRecorder");

// ----------------------------------------------------------------------

status_t BnMediaRecorder::onTransact(
                                     uint32_t code, const Parcel& data, Parcel* reply,
                                     uint32_t flags)
{
    switch (code) {
        case RELEASE: {
            ALOGV("RELEASE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(release());
            return NO_ERROR;
        } break;
        case INIT: {
            ALOGV("INIT");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(init());
            return NO_ERROR;
        } break;
        case CLOSE: {
            ALOGV("CLOSE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(close());
            return NO_ERROR;
        } break;
        case RESET: {
            ALOGV("RESET");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(reset());
            return NO_ERROR;
        } break;
        case STOP: {
            ALOGV("STOP");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(stop());
            return NO_ERROR;
        } break;
        case START: {
            ALOGV("START");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(start());
            return NO_ERROR;
        } break;
        case PAUSE: {
            ALOGV("PAUSE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(pause());
            return NO_ERROR;
        } break;
        case RESUME: {
            ALOGV("RESUME");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(resume());
            return NO_ERROR;
        } break;
        case PREPARE: {
            ALOGV("PREPARE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(prepare());
            return NO_ERROR;
        } break;
        case GET_MAX_AMPLITUDE: {
            ALOGV("GET_MAX_AMPLITUDE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int max = 0;
            status_t ret = getMaxAmplitude(&max);
            reply->writeInt32(max);
            reply->writeInt32(ret);
            return NO_ERROR;
        } break;
        case GET_METRICS: {
            ALOGV("GET_METRICS");
            status_t ret = getMetrics(reply);
            return ret;
        } break;
        case SET_VIDEO_SOURCE: {
            ALOGV("SET_VIDEO_SOURCE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int vs = data.readInt32();
            reply->writeInt32(setVideoSource(vs));
            return NO_ERROR;
        } break;
        case SET_AUDIO_SOURCE: {
            ALOGV("SET_AUDIO_SOURCE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int as = data.readInt32();
            reply->writeInt32(setAudioSource(as));
            return NO_ERROR;
        } break;
        case SET_OUTPUT_FORMAT: {
            ALOGV("SET_OUTPUT_FORMAT");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int of = data.readInt32();
            reply->writeInt32(setOutputFormat(of));
            return NO_ERROR;
        } break;
        case SET_VIDEO_ENCODER: {
            ALOGV("SET_VIDEO_ENCODER");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int ve = data.readInt32();
            reply->writeInt32(setVideoEncoder(ve));
            return NO_ERROR;
        } break;
        case SET_AUDIO_ENCODER: {
            ALOGV("SET_AUDIO_ENCODER");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int ae = data.readInt32();
            reply->writeInt32(setAudioEncoder(ae));
            return NO_ERROR;

        } break;
        case SET_OUTPUT_FILE_FD: {
            ALOGV("SET_OUTPUT_FILE_FD");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int fd = dup(data.readFileDescriptor());
            reply->writeInt32(setOutputFile(fd));
            ::close(fd);
            return NO_ERROR;
        } break;
        case SET_NEXT_OUTPUT_FILE_FD: {
            ALOGV("SET_NEXT_OUTPUT_FILE_FD");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int fd = dup(data.readFileDescriptor());
            reply->writeInt32(setNextOutputFile(fd));
            ::close(fd);
            return NO_ERROR;
        } break;
        case SET_VIDEO_SIZE: {
            ALOGV("SET_VIDEO_SIZE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int width = data.readInt32();
            int height = data.readInt32();
            reply->writeInt32(setVideoSize(width, height));
            return NO_ERROR;
        } break;
        case SET_VIDEO_FRAMERATE: {
            ALOGV("SET_VIDEO_FRAMERATE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int frames_per_second = data.readInt32();
            reply->writeInt32(setVideoFrameRate(frames_per_second));
            return NO_ERROR;
        } break;
        case SET_PARAMETERS: {
            ALOGV("SET_PARAMETER");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(setParameters(data.readString8()));
            return NO_ERROR;
        } break;
        case SET_LISTENER: {
            ALOGV("SET_LISTENER");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            sp<IMediaRecorderClient> listener =
                interface_cast<IMediaRecorderClient>(data.readStrongBinder());
            reply->writeInt32(setListener(listener));
            return NO_ERROR;
        } break;
        case SET_CLIENT_NAME: {
            ALOGV("SET_CLIENT_NAME");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            reply->writeInt32(setClientName(data.readString16()));
            return NO_ERROR;
        }
        case SET_PREVIEW_SURFACE: {
            ALOGV("SET_PREVIEW_SURFACE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            sp<IGraphicBufferProducer> surface = interface_cast<IGraphicBufferProducer>(
                    data.readStrongBinder());
            reply->writeInt32(setPreviewSurface(surface));
            return NO_ERROR;
        } break;
        case SET_CAMERA: {
            ALOGV("SET_CAMERA");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            sp<hardware::ICamera> camera =
                    interface_cast<hardware::ICamera>(data.readStrongBinder());
            sp<ICameraRecordingProxy> proxy =
                    interface_cast<ICameraRecordingProxy>(data.readStrongBinder());
            reply->writeInt32(setCamera(camera, proxy));
            return NO_ERROR;
        } break;
        case SET_INPUT_SURFACE: {
            ALOGV("SET_INPUT_SURFACE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            sp<PersistentSurface> surface = new PersistentSurface();
            surface->readFromParcel(&data);
            reply->writeInt32(setInputSurface(surface));
            return NO_ERROR;
        } break;
        case QUERY_SURFACE_MEDIASOURCE: {
            ALOGV("QUERY_SURFACE_MEDIASOURCE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            // call the mediaserver side to create
            // a surfacemediasource
            sp<IGraphicBufferProducer> surfaceMediaSource = querySurfaceMediaSource();
            // The mediaserver might have failed to create a source
            int returnedNull= (surfaceMediaSource == NULL) ? 1 : 0 ;
            reply->writeInt32(returnedNull);
            if (!returnedNull) {
                reply->writeStrongBinder(IInterface::asBinder(surfaceMediaSource));
            }
            return NO_ERROR;
        } break;
        case SET_INPUT_DEVICE: {
            ALOGV("SET_INPUT_DEVICE");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            audio_port_handle_t deviceId;
            status_t status = data.readInt32(&deviceId);
            if (status == NO_ERROR) {
                reply->writeInt32(setInputDevice(deviceId));
            } else {
                reply->writeInt32(BAD_VALUE);
            }
            return NO_ERROR;
        } break;
        case GET_ROUTED_DEVICE_ID: {
            ALOGV("GET_ROUTED_DEVICE_ID");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            audio_port_handle_t deviceId;
            status_t status = getRoutedDeviceId(&deviceId);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->writeInt32(deviceId);
            }
            return NO_ERROR;
        } break;
        case ENABLE_AUDIO_DEVICE_CALLBACK: {
            ALOGV("ENABLE_AUDIO_DEVICE_CALLBACK");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            bool enabled;
            status_t status = data.readBool(&enabled);
            if (status == NO_ERROR) {
                reply->writeInt32(enableAudioDeviceCallback(enabled));
            } else {
                reply->writeInt32(BAD_VALUE);
            }
            return NO_ERROR;
        } break;
        case GET_ACTIVE_MICROPHONES: {
            ALOGV("GET_ACTIVE_MICROPHONES");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            std::vector<media::MicrophoneInfo> activeMicrophones;
            status_t status = getActiveMicrophones(&activeMicrophones);
            reply->writeInt32(status);
            if (status != NO_ERROR) {
                return NO_ERROR;
            }
            reply->writeParcelableVector(activeMicrophones);
            return NO_ERROR;

        }
        case GET_PORT_ID: {
            ALOGV("GET_PORT_ID");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            audio_port_handle_t portId;
            status_t status = getPortId(&portId);
            reply->writeInt32(status);
            if (status == NO_ERROR) {
                reply->writeInt32(portId);
            }
            return NO_ERROR;
        }
        case SET_PREFERRED_MICROPHONE_DIRECTION: {
            ALOGV("SET_PREFERRED_MICROPHONE_DIRECTION");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            int direction = data.readInt32();
            status_t status = setPreferredMicrophoneDirection(
                    static_cast<audio_microphone_direction_t>(direction));
            reply->writeInt32(status);
            return NO_ERROR;
        }
        case SET_PREFERRED_MICROPHONE_FIELD_DIMENSION: {
            ALOGV("SET_MICROPHONE_FIELD_DIMENSION");
            CHECK_INTERFACE(IMediaRecorder, data, reply);
            float zoom = data.readFloat();
            status_t status = setPreferredMicrophoneFieldDimension(zoom);
            reply->writeInt32(status);
            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

} // namespace android
