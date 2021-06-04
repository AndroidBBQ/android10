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

#ifndef ANDROID_HARDWARE_CAMERA_HARDWARE_INTERFACE_H
#define ANDROID_HARDWARE_CAMERA_HARDWARE_INTERFACE_H

#include <unordered_map>
#include <binder/IMemory.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <utils/RefBase.h>
#include <ui/GraphicBuffer.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#include <system/window.h>
#include <hardware/camera.h>

#include <common/CameraProviderManager.h>

namespace android {

typedef void (*notify_callback)(int32_t msgType,
                            int32_t ext1,
                            int32_t ext2,
                            void* user);

typedef void (*data_callback)(int32_t msgType,
                            const sp<IMemory> &dataPtr,
                            camera_frame_metadata_t *metadata,
                            void* user);

typedef void (*data_callback_timestamp)(nsecs_t timestamp,
                            int32_t msgType,
                            const sp<IMemory> &dataPtr,
                            void *user);

struct HandleTimestampMessage {
    nsecs_t timestamp;
    const sp<IMemory> dataPtr;
};

typedef void (*data_callback_timestamp_batch)(
        int32_t msgType,
        const std::vector<HandleTimestampMessage>&, void* user);

/**
 * CameraHardwareInterface.h defines the interface to the
 * camera hardware abstraction layer, used for setting and getting
 * parameters, live previewing, and taking pictures. It is used for
 * HAL devices with version CAMERA_DEVICE_API_VERSION_1_0 only.
 *
 * It is a referenced counted interface with RefBase as its base class.
 * CameraService calls openCameraHardware() to retrieve a strong pointer to the
 * instance of this interface and may be called multiple times. The
 * following steps describe a typical sequence:
 *
 *   -# After CameraService calls openCameraHardware(), getParameters() and
 *      setParameters() are used to initialize the camera instance.
 *   -# startPreview() is called.
 *
 * Prior to taking a picture, CameraService often calls autofocus(). When auto
 * focusing has completed, the camera instance sends a CAMERA_MSG_FOCUS notification,
 * which informs the application whether focusing was successful. The camera instance
 * only sends this message once and it is up  to the application to call autoFocus()
 * again if refocusing is desired.
 *
 * CameraService calls takePicture() to request the camera instance take a
 * picture. At this point, if a shutter, postview, raw, and/or compressed
 * callback is desired, the corresponding message must be enabled. Any memory
 * provided in a data callback must be copied if it's needed after returning.
 */

class CameraHardwareInterface :
        public virtual RefBase,
        public virtual hardware::camera::device::V1_0::ICameraDeviceCallback,
        public virtual hardware::camera::device::V1_0::ICameraDevicePreviewCallback {

public:
    explicit CameraHardwareInterface(const char *name):
            mHidlDevice(nullptr),
            mName(name),
            mPreviewScalingMode(NOT_SET),
            mPreviewTransform(NOT_SET),
            mPreviewWidth(NOT_SET),
            mPreviewHeight(NOT_SET),
            mPreviewFormat(NOT_SET),
            mPreviewUsage(0),
            mPreviewSwapInterval(NOT_SET),
            mPreviewCrop{NOT_SET,NOT_SET,NOT_SET,NOT_SET}
    {
    }

    ~CameraHardwareInterface();

    status_t initialize(sp<CameraProviderManager> manager);

    /** Set the ANativeWindow to which preview frames are sent */
    status_t setPreviewWindow(const sp<ANativeWindow>& buf);

    status_t setPreviewScalingMode(int scalingMode);

    status_t setPreviewTransform(int transform);

    /** Set the notification and data callbacks */
    void setCallbacks(notify_callback notify_cb,
                      data_callback data_cb,
                      data_callback_timestamp data_cb_timestamp,
                      data_callback_timestamp_batch data_cb_timestamp_batch,
                      void* user);

    /**
     * The following three functions all take a msgtype,
     * which is a bitmask of the messages defined in
     * include/ui/Camera.h
     */

    /**
     * Enable a message, or set of messages.
     */
    void enableMsgType(int32_t msgType);

    /**
     * Disable a message, or a set of messages.
     *
     * Once received a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), camera hal
     * should not rely on its client to call releaseRecordingFrame() to release
     * video recording frames sent out by the cameral hal before and after the
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME) call. Camera hal clients must not
     * modify/access any video recording frame after calling
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME).
     */
    void disableMsgType(int32_t msgType);

    /**
     * Query whether a message, or a set of messages, is enabled.
     * Note that this is operates as an AND, if any of the messages
     * queried are off, this will return false.
     */
    int msgTypeEnabled(int32_t msgType);

    /**
     * Start preview mode.
     */
    status_t startPreview();

    /**
     * Stop a previously started preview.
     */
    void stopPreview();

    /**
     * Returns true if preview is enabled.
     */
    int previewEnabled();

    /**
     * Request the camera hal to store meta data or real YUV data in
     * the video buffers send out via CAMERA_MSG_VIDEO_FRRAME for a
     * recording session. If it is not called, the default camera
     * hal behavior is to store real YUV data in the video buffers.
     *
     * This method should be called before startRecording() in order
     * to be effective.
     *
     * If meta data is stored in the video buffers, it is up to the
     * receiver of the video buffers to interpret the contents and
     * to find the actual frame data with the help of the meta data
     * in the buffer. How this is done is outside of the scope of
     * this method.
     *
     * Some camera hal may not support storing meta data in the video
     * buffers, but all camera hal should support storing real YUV data
     * in the video buffers. If the camera hal does not support storing
     * the meta data in the video buffers when it is requested to do
     * do, INVALID_OPERATION must be returned. It is very useful for
     * the camera hal to pass meta data rather than the actual frame
     * data directly to the video encoder, since the amount of the
     * uncompressed frame data can be very large if video size is large.
     *
     * @param enable if true to instruct the camera hal to store
     *      meta data in the video buffers; false to instruct
     *      the camera hal to store real YUV data in the video
     *      buffers.
     *
     * @return OK on success.
     */

    status_t storeMetaDataInBuffers(int enable);

    /**
     * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
     * message is sent with the corresponding frame. Every record frame must be released
     * by a cameral hal client via releaseRecordingFrame() before the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
     * to manage the life-cycle of the video recording frames, and the client must
     * not modify/access any video recording frames.
     */
    status_t startRecording();

    /**
     * Stop a previously started recording.
     */
    void stopRecording();

    /**
     * Returns true if recording is enabled.
     */
    int recordingEnabled();

    /**
     * Release a record frame previously returned by CAMERA_MSG_VIDEO_FRAME.
     *
     * It is camera hal client's responsibility to release video recording
     * frames sent out by the camera hal before the camera hal receives
     * a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME). After it receives
     * the call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's
     * responsibility of managing the life-cycle of the video recording
     * frames.
     */
    void releaseRecordingFrame(const sp<IMemory>& mem);

    /**
     * Release a batch of recording frames previously returned by
     * CAMERA_MSG_VIDEO_FRAME. This method only supports frames that are
     * stored as VideoNativeHandleMetadata.
     *
     * It is camera hal client's responsibility to release video recording
     * frames sent out by the camera hal before the camera hal receives
     * a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME). After it receives
     * the call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's
     * responsibility of managing the life-cycle of the video recording
     * frames.
     */
    void releaseRecordingFrameBatch(const std::vector<sp<IMemory>>& frames);

    /**
     * Start auto focus, the notification callback routine is called
     * with CAMERA_MSG_FOCUS once when focusing is complete. autoFocus()
     * will be called again if another auto focus is needed.
     */
    status_t autoFocus();

    /**
     * Cancels auto-focus function. If the auto-focus is still in progress,
     * this function will cancel it. Whether the auto-focus is in progress
     * or not, this function will return the focus position to the default.
     * If the camera does not support auto-focus, this is a no-op.
     */
    status_t cancelAutoFocus();

    /**
     * Take a picture.
     */
    status_t takePicture();

    /**
     * Cancel a picture that was started with takePicture.  Calling this
     * method when no picture is being taken is a no-op.
     */
    status_t cancelPicture();

    /**
     * Set the camera parameters. This returns BAD_VALUE if any parameter is
     * invalid or not supported. */
    status_t setParameters(const CameraParameters &params);

    /** Return the camera parameters. */
    CameraParameters getParameters() const;

    /**
     * Send command to camera driver.
     */
    status_t sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);

    /**
     * Release the hardware resources owned by this object.  Note that this is
     * *not* done in the destructor.
     */
    void release();

    /**
     * Dump state of the camera hardware
     */
    status_t dump(int fd, const Vector<String16>& /*args*/) const;

private:
    sp<hardware::camera::device::V1_0::ICameraDevice> mHidlDevice;
    String8 mName;

    static void sNotifyCb(int32_t msg_type, int32_t ext1,
                            int32_t ext2, void *user);

    static void sDataCb(int32_t msg_type,
                          const camera_memory_t *data, unsigned int index,
                          camera_frame_metadata_t *metadata,
                          void *user);

    static void sDataCbTimestamp(nsecs_t timestamp, int32_t msg_type,
                             const camera_memory_t *data, unsigned index,
                             void *user);

    // This is a utility class that combines a MemoryHeapBase and a MemoryBase
    // in one.  Since we tend to use them in a one-to-one relationship, this is
    // handy.
    class CameraHeapMemory : public RefBase {
    public:
        CameraHeapMemory(int fd, size_t buf_size, uint_t num_buffers = 1) :
                         mBufSize(buf_size),
                         mNumBufs(num_buffers)
        {
            mHeap = new MemoryHeapBase(fd, buf_size * num_buffers);
            commonInitialization();
        }

        explicit CameraHeapMemory(size_t buf_size, uint_t num_buffers = 1) :
                                  mBufSize(buf_size),
                                  mNumBufs(num_buffers)
        {
            mHeap = new MemoryHeapBase(buf_size * num_buffers);
            commonInitialization();
        }

        void commonInitialization()
        {
            handle.data = mHeap->base();
            handle.size = mBufSize * mNumBufs;
            handle.handle = this;

            mBuffers = new sp<MemoryBase>[mNumBufs];
            for (uint_t i = 0; i < mNumBufs; i++)
                mBuffers[i] = new MemoryBase(mHeap,
                                             i * mBufSize,
                                             mBufSize);

            handle.release = sPutMemory;
        }

        virtual ~CameraHeapMemory()
        {
            delete [] mBuffers;
        }

        size_t mBufSize;
        uint_t mNumBufs;
        sp<MemoryHeapBase> mHeap;
        sp<MemoryBase> *mBuffers;

        camera_memory_t handle;
    };

    static camera_memory_t* sGetMemory(int fd, size_t buf_size, uint_t num_bufs,
                                         void *user __attribute__((unused)));

    static void sPutMemory(camera_memory_t *data);

    std::pair<bool, uint64_t> getBufferId(ANativeWindowBuffer* anb);
    void cleanupCirculatingBuffers();

    /**
     * Implementation of android::hardware::camera::device::V1_0::ICameraDeviceCallback
     */
    hardware::Return<void> notifyCallback(
            hardware::camera::device::V1_0::NotifyCallbackMsg msgType,
            int32_t ext1, int32_t ext2) override;
    hardware::Return<uint32_t> registerMemory(
            const hardware::hidl_handle& descriptor,
            uint32_t bufferSize, uint32_t bufferCount) override;
    hardware::Return<void> unregisterMemory(uint32_t memId) override;
    hardware::Return<void> dataCallback(
            hardware::camera::device::V1_0::DataCallbackMsg msgType,
            uint32_t data, uint32_t bufferIndex,
            const hardware::camera::device::V1_0::CameraFrameMetadata& metadata) override;
    hardware::Return<void> dataCallbackTimestamp(
            hardware::camera::device::V1_0::DataCallbackMsg msgType,
            uint32_t data, uint32_t bufferIndex, int64_t timestamp) override;
    hardware::Return<void> handleCallbackTimestamp(
            hardware::camera::device::V1_0::DataCallbackMsg msgType,
            const hardware::hidl_handle& frameData, uint32_t data,
            uint32_t bufferIndex, int64_t timestamp) override;
    hardware::Return<void> handleCallbackTimestampBatch(
            hardware::camera::device::V1_0::DataCallbackMsg msgType,
            const hardware::hidl_vec<
                    hardware::camera::device::V1_0::HandleTimestampMessage>&) override;

    /**
     * Implementation of android::hardware::camera::device::V1_0::ICameraDevicePreviewCallback
     */
    hardware::Return<void> dequeueBuffer(dequeueBuffer_cb _hidl_cb) override;
    hardware::Return<hardware::camera::common::V1_0::Status>
            enqueueBuffer(uint64_t bufferId) override;
    hardware::Return<hardware::camera::common::V1_0::Status>
            cancelBuffer(uint64_t bufferId) override;
    hardware::Return<hardware::camera::common::V1_0::Status>
            setBufferCount(uint32_t count) override;
    hardware::Return<hardware::camera::common::V1_0::Status>
            setBuffersGeometry(uint32_t w, uint32_t h,
                    hardware::graphics::common::V1_0::PixelFormat format) override;
    hardware::Return<hardware::camera::common::V1_0::Status>
            setCrop(int32_t left, int32_t top, int32_t right, int32_t bottom) override;
    hardware::Return<hardware::camera::common::V1_0::Status>
            setUsage(hardware::graphics::common::V1_0::BufferUsage usage) override;
    hardware::Return<hardware::camera::common::V1_0::Status>
            setSwapInterval(int32_t interval) override;
    hardware::Return<void> getMinUndequeuedBufferCount(
        getMinUndequeuedBufferCount_cb _hidl_cb) override;
    hardware::Return<hardware::camera::common::V1_0::Status>
            setTimestamp(int64_t timestamp) override;

    sp<ANativeWindow>        mPreviewWindow;

    notify_callback               mNotifyCb;
    data_callback                 mDataCb;
    data_callback_timestamp       mDataCbTimestamp;
    data_callback_timestamp_batch mDataCbTimestampBatch;
    void *mCbUser;

    // Cached values for preview stream parameters
    static const int NOT_SET = -1;
    int mPreviewScalingMode;
    int mPreviewTransform;
    int mPreviewWidth;
    int mPreviewHeight;
    int mPreviewFormat;
    uint64_t mPreviewUsage;
    int mPreviewSwapInterval;
    android_native_rect_t mPreviewCrop;

    struct BufferHasher {
        size_t operator()(const buffer_handle_t& buf) const {
            if (buf == nullptr)
                return 0;

            size_t result = 1;
            result = 31 * result + buf->numFds;
            result = 31 * result + buf->numInts;
            int length = buf->numFds + buf->numInts;
            for (int i = 0; i < length; i++) {
                result = 31 * result + buf->data[i];
            }
            return result;
        }
    };

    struct BufferComparator {
        bool operator()(const buffer_handle_t& buf1, const buffer_handle_t& buf2) const {
            if (buf1->numFds == buf2->numFds && buf1->numInts == buf2->numInts) {
                int length = buf1->numFds + buf1->numInts;
                for (int i = 0; i < length; i++) {
                    if (buf1->data[i] != buf2->data[i]) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }
    };

    std::mutex mBufferIdMapLock; // protecting mBufferIdMap and mNextBufferId
    typedef std::unordered_map<const buffer_handle_t, uint64_t,
            BufferHasher, BufferComparator> BufferIdMap;
    // stream ID -> per stream buffer ID map
    BufferIdMap mBufferIdMap;
    std::unordered_map<uint64_t, ANativeWindowBuffer*> mReversedBufMap;
    uint64_t mNextBufferId = 1;
    static const uint64_t BUFFER_ID_NO_BUFFER = 0;

    std::mutex mHidlMemPoolMapLock; // protecting mHidlMemPoolMap
    std::unordered_map<int, camera_memory_t*> mHidlMemPoolMap;
};

};  // namespace android

#endif
