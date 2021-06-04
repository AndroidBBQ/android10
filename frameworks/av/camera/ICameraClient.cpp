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
#define LOG_TAG "ICameraClient"
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <camera/CameraUtils.h>
#include <android/hardware/ICameraClient.h>
#include <media/hardware/HardwareAPI.h>

namespace android {
namespace hardware {

enum {
    NOTIFY_CALLBACK = IBinder::FIRST_CALL_TRANSACTION,
    DATA_CALLBACK,
    DATA_CALLBACK_TIMESTAMP,
    RECORDING_FRAME_HANDLE_CALLBACK_TIMESTAMP,
    RECORDING_FRAME_HANDLE_CALLBACK_TIMESTAMP_BATCH,
};

class BpCameraClient: public BpInterface<ICameraClient>
{
public:
    explicit BpCameraClient(const sp<IBinder>& impl)
        : BpInterface<ICameraClient>(impl)
    {
    }

    // generic callback from camera service to app
    void notifyCallback(int32_t msgType, int32_t ext1, int32_t ext2)
    {
        ALOGV("notifyCallback");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraClient::getInterfaceDescriptor());
        data.writeInt32(msgType);
        data.writeInt32(ext1);
        data.writeInt32(ext2);
        remote()->transact(NOTIFY_CALLBACK, data, &reply, IBinder::FLAG_ONEWAY);
    }

    // generic data callback from camera service to app with image data
    void dataCallback(int32_t msgType, const sp<IMemory>& imageData,
                      camera_frame_metadata_t *metadata)
    {
        ALOGV("dataCallback");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraClient::getInterfaceDescriptor());
        data.writeInt32(msgType);
        data.writeStrongBinder(IInterface::asBinder(imageData));
        if (metadata) {
            data.writeInt32(metadata->number_of_faces);
            data.write(metadata->faces, sizeof(camera_face_t) * metadata->number_of_faces);
        }
        remote()->transact(DATA_CALLBACK, data, &reply, IBinder::FLAG_ONEWAY);
    }

    // generic data callback from camera service to app with image data
    void dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& imageData)
    {
        ALOGV("dataCallback");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraClient::getInterfaceDescriptor());
        data.writeInt64(timestamp);
        data.writeInt32(msgType);
        data.writeStrongBinder(IInterface::asBinder(imageData));
        remote()->transact(DATA_CALLBACK_TIMESTAMP, data, &reply, IBinder::FLAG_ONEWAY);
    }

    void recordingFrameHandleCallbackTimestamp(nsecs_t timestamp, native_handle_t* handle) {
        ALOGV("recordingFrameHandleCallbackTimestamp");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraClient::getInterfaceDescriptor());
        data.writeInt64(timestamp);
        data.writeNativeHandle(handle);
        remote()->transact(RECORDING_FRAME_HANDLE_CALLBACK_TIMESTAMP, data, &reply,
                IBinder::FLAG_ONEWAY);
    }

    void recordingFrameHandleCallbackTimestampBatch(
            const std::vector<nsecs_t>& timestamps,
            const std::vector<native_handle_t*>& handles) {
        ALOGV("recordingFrameHandleCallbackTimestampBatch");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraClient::getInterfaceDescriptor());
        uint32_t n = timestamps.size();
        if (n != handles.size()) {
            ALOGE("%s: size of timestamps(%zu) and handles(%zu) mismatch!",
                    __FUNCTION__, timestamps.size(), handles.size());
            return;
        }
        data.writeUint32(n);
        for (auto ts : timestamps) {
            data.writeInt64(ts);
        }
        for (auto& handle : handles) {
            data.writeNativeHandle(handle);
        }
        remote()->transact(RECORDING_FRAME_HANDLE_CALLBACK_TIMESTAMP_BATCH, data, &reply,
                IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(CameraClient, "android.hardware.ICameraClient");

// ----------------------------------------------------------------------

status_t BnCameraClient::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case NOTIFY_CALLBACK: {
            ALOGV("NOTIFY_CALLBACK");
            CHECK_INTERFACE(ICameraClient, data, reply);
            int32_t msgType = data.readInt32();
            int32_t ext1 = data.readInt32();
            int32_t ext2 = data.readInt32();
            notifyCallback(msgType, ext1, ext2);
            return NO_ERROR;
        } break;
        case DATA_CALLBACK: {
            ALOGV("DATA_CALLBACK");
            CHECK_INTERFACE(ICameraClient, data, reply);
            int32_t msgType = data.readInt32();
            sp<IMemory> imageData = interface_cast<IMemory>(data.readStrongBinder());
            camera_frame_metadata_t *metadata = NULL;
            if (data.dataAvail() > 0) {
                metadata = new camera_frame_metadata_t;
                metadata->number_of_faces = data.readInt32();
                metadata->faces = (camera_face_t *) data.readInplace(
                        sizeof(camera_face_t) * metadata->number_of_faces);
            }
            dataCallback(msgType, imageData, metadata);
            if (metadata) delete metadata;
            return NO_ERROR;
        } break;
        case DATA_CALLBACK_TIMESTAMP: {
            ALOGV("DATA_CALLBACK_TIMESTAMP");
            CHECK_INTERFACE(ICameraClient, data, reply);
            nsecs_t timestamp = data.readInt64();
            int32_t msgType = data.readInt32();
            sp<IMemory> imageData = interface_cast<IMemory>(data.readStrongBinder());
            dataCallbackTimestamp(timestamp, msgType, imageData);
            return NO_ERROR;
        } break;
        case RECORDING_FRAME_HANDLE_CALLBACK_TIMESTAMP: {
            ALOGV("RECORDING_FRAME_HANDLE_CALLBACK_TIMESTAMP");
            CHECK_INTERFACE(ICameraClient, data, reply);
            nsecs_t timestamp;
            status_t res = data.readInt64(&timestamp);
            if (res != OK) {
                ALOGE("%s: Failed to read timestamp: %s (%d)", __FUNCTION__, strerror(-res), res);
                return BAD_VALUE;
            }
            native_handle_t* handle = data.readNativeHandle();
            if (handle == nullptr) {
                ALOGE("%s: Received a null native handle", __FUNCTION__);
                return BAD_VALUE;
            }

            // The native handle will be freed in BpCamera::releaseRecordingFrameHandle.
            recordingFrameHandleCallbackTimestamp(timestamp, handle);
            return NO_ERROR;
        } break;
        case RECORDING_FRAME_HANDLE_CALLBACK_TIMESTAMP_BATCH: {
            ALOGV("RECORDING_FRAME_HANDLE_CALLBACK_TIMESTAMP_BATCH");
            CHECK_INTERFACE(ICameraClient, data, reply);
            uint32_t n = 0;
            status_t res = data.readUint32(&n);
            if (res != OK) {
                ALOGE("%s: Failed to read batch size: %s (%d)", __FUNCTION__, strerror(-res), res);
                return BAD_VALUE;
            }
            std::vector<nsecs_t> timestamps;
            std::vector<native_handle_t*> handles;
            timestamps.reserve(n);
            handles.reserve(n);
            for (uint32_t i = 0; i < n; i++) {
                nsecs_t t;
                res = data.readInt64(&t);
                if (res != OK) {
                    ALOGE("%s: Failed to read timestamp[%d]: %s (%d)",
                            __FUNCTION__, i, strerror(-res), res);
                    return BAD_VALUE;
                }
                timestamps.push_back(t);
            }
            for (uint32_t i = 0; i < n; i++) {
                native_handle_t* handle = data.readNativeHandle();
                if (handle == nullptr) {
                    ALOGE("%s: Received a null native handle at handles[%d]",
                            __FUNCTION__, i);
                    return BAD_VALUE;
                }
                handles.push_back(handle);
            }

            // The native handle will be freed in BpCamera::releaseRecordingFrameHandleBatch.
            recordingFrameHandleCallbackTimestampBatch(timestamps, handles);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

} // namespace hardware
} // namespace android
