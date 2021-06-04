/*
 * Copyright (C) 2011 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "ICameraRecordingProxy"
#include <camera/CameraUtils.h>
#include <camera/ICameraRecordingProxy.h>
#include <camera/ICameraRecordingProxyListener.h>
#include <binder/IMemory.h>
#include <binder/Parcel.h>
#include <media/hardware/HardwareAPI.h>
#include <stdint.h>
#include <utils/Log.h>

namespace android {

enum {
    START_RECORDING = IBinder::FIRST_CALL_TRANSACTION,
    STOP_RECORDING,
    RELEASE_RECORDING_FRAME,
    RELEASE_RECORDING_FRAME_HANDLE,
    RELEASE_RECORDING_FRAME_HANDLE_BATCH,
};


class BpCameraRecordingProxy: public BpInterface<ICameraRecordingProxy>
{
public:
    explicit BpCameraRecordingProxy(const sp<IBinder>& impl)
        : BpInterface<ICameraRecordingProxy>(impl)
    {
    }

    status_t startRecording(const sp<ICameraRecordingProxyListener>& listener)
    {
        ALOGV("startRecording");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraRecordingProxy::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(listener));
        remote()->transact(START_RECORDING, data, &reply);
        return reply.readInt32();
    }

    void stopRecording()
    {
        ALOGV("stopRecording");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraRecordingProxy::getInterfaceDescriptor());
        remote()->transact(STOP_RECORDING, data, &reply);
    }

    void releaseRecordingFrame(const sp<IMemory>& mem)
    {
        ALOGV("releaseRecordingFrame");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraRecordingProxy::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(mem));
        remote()->transact(RELEASE_RECORDING_FRAME, data, &reply);
    }

    void releaseRecordingFrameHandle(native_handle_t *handle) {
        ALOGV("releaseRecordingFrameHandle");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraRecordingProxy::getInterfaceDescriptor());
        data.writeNativeHandle(handle);

        remote()->transact(RELEASE_RECORDING_FRAME_HANDLE, data, &reply);

        // Close the native handle because camera received a dup copy.
        native_handle_close(handle);
        native_handle_delete(handle);
    }

    void releaseRecordingFrameHandleBatch(const std::vector<native_handle_t*>& handles) {
        ALOGV("releaseRecordingFrameHandleBatch");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraRecordingProxy::getInterfaceDescriptor());
        uint32_t n = handles.size();
        data.writeUint32(n);
        for (auto& handle : handles) {
            data.writeNativeHandle(handle);
        }
        remote()->transact(RELEASE_RECORDING_FRAME_HANDLE_BATCH, data, &reply);

        // Close the native handle because camera received a dup copy.
        for (auto& handle : handles) {
            native_handle_close(handle);
            native_handle_delete(handle);
        }
    }
};

IMPLEMENT_META_INTERFACE(CameraRecordingProxy, "android.hardware.ICameraRecordingProxy");

// ----------------------------------------------------------------------

status_t BnCameraRecordingProxy::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case START_RECORDING: {
            ALOGV("START_RECORDING");
            CHECK_INTERFACE(ICameraRecordingProxy, data, reply);
            sp<ICameraRecordingProxyListener> listener =
                interface_cast<ICameraRecordingProxyListener>(data.readStrongBinder());
            reply->writeInt32(startRecording(listener));
            return NO_ERROR;
        } break;
        case STOP_RECORDING: {
            ALOGV("STOP_RECORDING");
            CHECK_INTERFACE(ICameraRecordingProxy, data, reply);
            stopRecording();
            return NO_ERROR;
        } break;
        case RELEASE_RECORDING_FRAME: {
            ALOGV("RELEASE_RECORDING_FRAME");
            CHECK_INTERFACE(ICameraRecordingProxy, data, reply);
            sp<IMemory> mem = interface_cast<IMemory>(data.readStrongBinder());
            releaseRecordingFrame(mem);
            return NO_ERROR;
        } break;
        case RELEASE_RECORDING_FRAME_HANDLE: {
            ALOGV("RELEASE_RECORDING_FRAME_HANDLE");
            CHECK_INTERFACE(ICameraRecordingProxy, data, reply);

            // releaseRecordingFrameHandle will be responsble to close the native handle.
            releaseRecordingFrameHandle(data.readNativeHandle());
            return NO_ERROR;
        } break;
        case RELEASE_RECORDING_FRAME_HANDLE_BATCH: {
            ALOGV("RELEASE_RECORDING_FRAME_HANDLE_BATCH");
            CHECK_INTERFACE(ICameraRecordingProxy, data, reply);
            uint32_t n = 0;
            status_t res = data.readUint32(&n);
            if (res != OK) {
                ALOGE("%s: Failed to read batch size: %s (%d)", __FUNCTION__, strerror(-res), res);
                return BAD_VALUE;
            }
            std::vector<native_handle_t*> handles;
            handles.reserve(n);
            for (uint32_t i = 0; i < n; i++) {
                native_handle_t* handle = data.readNativeHandle();
                if (handle == nullptr) {
                    ALOGE("%s: Received a null native handle at handles[%d]",
                            __FUNCTION__, i);
                    return BAD_VALUE;
                }
                handles.push_back(handle);
            }

            // releaseRecordingFrameHandleBatch will be responsble to close the native handle.
            releaseRecordingFrameHandleBatch(handles);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android

