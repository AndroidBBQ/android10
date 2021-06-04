/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_ICAMERA_H
#define ANDROID_HARDWARE_ICAMERA_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <binder/Status.h>
#include <utils/String8.h>

namespace android {

class IGraphicBufferProducer;
class Surface;

namespace hardware {

class ICameraClient;

class ICamera: public android::IInterface
{
    /**
     * Keep up-to-date with ICamera.aidl in frameworks/base
     */
public:
    enum {
        // Pass real YUV data in video buffers through ICameraClient.dataCallbackTimestamp().
        VIDEO_BUFFER_MODE_DATA_CALLBACK_YUV = 0,
        // Pass metadata in video buffers through ICameraClient.dataCallbackTimestamp().
        VIDEO_BUFFER_MODE_DATA_CALLBACK_METADATA = 1,
        // Pass video buffers through IGraphicBufferProducer set with setVideoTarget().
        VIDEO_BUFFER_MODE_BUFFER_QUEUE = 2,
    };

    DECLARE_META_INTERFACE(Camera);

    virtual binder::Status  disconnect() = 0;

    // connect new client with existing camera remote
    virtual status_t        connect(const sp<ICameraClient>& client) = 0;

    // prevent other processes from using this ICamera interface
    virtual status_t        lock() = 0;

    // allow other processes to use this ICamera interface
    virtual status_t        unlock() = 0;

    // pass the buffered IGraphicBufferProducer to the camera service
    virtual status_t        setPreviewTarget(
            const sp<IGraphicBufferProducer>& bufferProducer) = 0;

    // set the preview callback flag to affect how the received frames from
    // preview are handled. Enabling preview callback flags disables any active
    // preview callback surface set by setPreviewCallbackTarget().
    virtual void            setPreviewCallbackFlag(int flag) = 0;
    // set a buffer interface to use for client-received preview frames instead
    // of preview callback buffers. Passing a valid interface here disables any
    // active preview callbacks set by setPreviewCallbackFlag(). Passing NULL
    // disables the use of the callback target.
    virtual status_t        setPreviewCallbackTarget(
            const sp<IGraphicBufferProducer>& callbackProducer) = 0;

    // start preview mode, must call setPreviewTarget first
    virtual status_t        startPreview() = 0;

    // stop preview mode
    virtual void            stopPreview() = 0;

    // get preview state
    virtual bool            previewEnabled() = 0;

    // start recording mode
    virtual status_t        startRecording() = 0;

    // stop recording mode
    virtual void            stopRecording() = 0;

    // get recording state
    virtual bool            recordingEnabled() = 0;

    // Release a recording frame that was received via ICameraClient::dataCallbackTimestamp.
    virtual void            releaseRecordingFrame(const sp<IMemory>& mem) = 0;

    // Release a recording frame handle that was received via
    // ICameraClient::recordingFrameHandleCallbackTimestamp.
    virtual void            releaseRecordingFrameHandle(native_handle_t *handle) = 0;

    // Release a batch of recording frame handles that was received via
    // ICameraClient::recordingFrameHandleCallbackTimestampBatch
    virtual void            releaseRecordingFrameHandleBatch(
            const std::vector<native_handle_t*>& handles) = 0;

    // auto focus
    virtual status_t        autoFocus() = 0;

    // cancel auto focus
    virtual status_t        cancelAutoFocus() = 0;

    /*
     * take a picture.
     * @param msgType the message type an application selectively turn on/off
     * on a photo-by-photo basis. The supported message types are:
     * CAMERA_MSG_SHUTTER, CAMERA_MSG_RAW_IMAGE, CAMERA_MSG_COMPRESSED_IMAGE,
     * and CAMERA_MSG_POSTVIEW_FRAME. Any other message types will be ignored.
     */
    virtual status_t        takePicture(int msgType) = 0;

    // set preview/capture parameters - key/value pairs
    virtual status_t        setParameters(const String8& params) = 0;

    // get preview/capture parameters - key/value pairs
    virtual String8         getParameters() const = 0;

    // send command to camera driver
    virtual status_t        sendCommand(int32_t cmd, int32_t arg1, int32_t arg2) = 0;


    // Tell camera how to pass video buffers. videoBufferMode is one of VIDEO_BUFFER_MODE_*.
    // Returns OK if the specified video buffer mode is supported. If videoBufferMode is
    // VIDEO_BUFFER_MODE_BUFFER_QUEUE, setVideoTarget() must be called before starting video
    // recording.
    virtual status_t        setVideoBufferMode(int32_t videoBufferMode) = 0;

    // Set the video buffer producer for camera to use in VIDEO_BUFFER_MODE_BUFFER_QUEUE mode.
    virtual status_t        setVideoTarget(
            const sp<IGraphicBufferProducer>& bufferProducer) = 0;
};

// ----------------------------------------------------------------------------

class BnCamera: public android::BnInterface<ICamera>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

} // namespace hardware
} // namespace android

#endif
