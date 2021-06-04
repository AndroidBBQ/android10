/*
 * Copyright (C) 2013-2018 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA3_OUTPUT_STREAM_H
#define ANDROID_SERVERS_CAMERA3_OUTPUT_STREAM_H

#include <utils/RefBase.h>
#include <gui/IProducerListener.h>
#include <gui/Surface.h>

#include "utils/LatencyHistogram.h"
#include "Camera3Stream.h"
#include "Camera3IOStreamBase.h"
#include "Camera3OutputStreamInterface.h"
#include "Camera3BufferManager.h"

namespace android {

namespace camera3 {

class Camera3BufferManager;

/**
 * Stream info structure that holds the necessary stream info for buffer manager to use for
 * buffer allocation and management.
 */
struct StreamInfo {
    int streamId;
    int streamSetId;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    android_dataspace dataSpace;
    uint64_t combinedUsage;
    size_t totalBufferCount;
    bool isConfigured;
    explicit StreamInfo(int id = CAMERA3_STREAM_ID_INVALID,
            int setId = CAMERA3_STREAM_SET_ID_INVALID,
            uint32_t w = 0,
            uint32_t h = 0,
            uint32_t fmt = 0,
            android_dataspace ds = HAL_DATASPACE_UNKNOWN,
            uint64_t usage = 0,
            size_t bufferCount = 0,
            bool configured = false) :
                streamId(id),
                streamSetId(setId),
                width(w),
                height(h),
                format(fmt),
                dataSpace(ds),
                combinedUsage(usage),
                totalBufferCount(bufferCount),
                isConfigured(configured){}
};

/**
 * A class for managing a single stream of output data from the camera device.
 */
class Camera3OutputStream :
        public Camera3IOStreamBase,
        public Camera3OutputStreamInterface {
  public:
    /**
     * Set up a stream for formats that have 2 dimensions, such as RAW and YUV.
     * A valid stream set id needs to be set to support buffer sharing between multiple
     * streams.
     */
    Camera3OutputStream(int id, sp<Surface> consumer,
            uint32_t width, uint32_t height, int format,
            android_dataspace dataSpace, camera3_stream_rotation_t rotation,
            nsecs_t timestampOffset, const String8& physicalCameraId,
            int setId = CAMERA3_STREAM_SET_ID_INVALID);

    /**
     * Set up a stream for formats that have a variable buffer size for the same
     * dimensions, such as compressed JPEG.
     * A valid stream set id needs to be set to support buffer sharing between multiple
     * streams.
     */
    Camera3OutputStream(int id, sp<Surface> consumer,
            uint32_t width, uint32_t height, size_t maxSize, int format,
            android_dataspace dataSpace, camera3_stream_rotation_t rotation,
            nsecs_t timestampOffset, const String8& physicalCameraId,
            int setId = CAMERA3_STREAM_SET_ID_INVALID);

    /**
     * Set up a stream with deferred consumer for formats that have 2 dimensions, such as
     * RAW and YUV. The consumer must be set before using this stream for output. A valid
     * stream set id needs to be set to support buffer sharing between multiple streams.
     */
    Camera3OutputStream(int id, uint32_t width, uint32_t height, int format,
            uint64_t consumerUsage, android_dataspace dataSpace,
            camera3_stream_rotation_t rotation, nsecs_t timestampOffset,
            const String8& physicalCameraId,
            int setId = CAMERA3_STREAM_SET_ID_INVALID);

    virtual ~Camera3OutputStream();

    /**
     * Camera3Stream interface
     */

    virtual void     dump(int fd, const Vector<String16> &args) const;

    /**
     * Set the transform on the output stream; one of the
     * HAL_TRANSFORM_* / NATIVE_WINDOW_TRANSFORM_* constants.
     */
    status_t         setTransform(int transform);

    /**
     * Return if this output stream is for video encoding.
     */
    bool isVideoStream() const;
    /**
     * Return if this output stream is consumed by hardware composer.
     */
    bool isConsumedByHWComposer() const;

    /**
     * Return if this output stream is consumed by hardware texture.
     */
    bool isConsumedByHWTexture() const;

    /**
     * Return if the consumer configuration of this stream is deferred.
     */
    virtual bool isConsumerConfigurationDeferred(size_t surface_id) const;

    /**
     * Set the consumer surfaces to the output stream.
     */
    virtual status_t setConsumers(const std::vector<sp<Surface>>& consumers);

    class BufferProducerListener : public SurfaceListener {
        public:
            BufferProducerListener(wp<Camera3OutputStream> parent, bool needsReleaseNotify)
                    : mParent(parent), mNeedsReleaseNotify(needsReleaseNotify) {}

            /**
            * Implementation of IProducerListener, used to notify this stream that the consumer
            * has returned a buffer and it is ready to return to Camera3BufferManager for reuse.
            */
            virtual void onBufferReleased();
            virtual bool needsReleaseNotify() { return mNeedsReleaseNotify; }
            virtual void onBuffersDiscarded(const std::vector<sp<GraphicBuffer>>& buffers);

        private:
            wp<Camera3OutputStream> mParent;
            bool mNeedsReleaseNotify;
    };

    virtual status_t detachBuffer(sp<GraphicBuffer>* buffer, int* fenceFd);

    /**
     * Notify that the buffer is being released to the buffer queue instead of
     * being queued to the consumer.
     */
    virtual status_t notifyBufferReleased(ANativeWindowBuffer *anwBuffer);

    /**
     * Drop buffers if dropping is true. If dropping is false, do not drop buffers.
     */
    virtual status_t dropBuffers(bool dropping) override;

    /**
     * Query the physical camera id for the output stream.
     */
    virtual const String8& getPhysicalCameraId() const override;

    /**
     * Set the graphic buffer manager to get/return the stream buffers.
     *
     * It is only legal to call this method when stream is in STATE_CONSTRUCTED state.
     */
    status_t setBufferManager(sp<Camera3BufferManager> bufferManager);

    /**
     * Query the ouput surface id.
     */
    virtual ssize_t getSurfaceId(const sp<Surface> &/*surface*/) { return 0; }

    virtual status_t getUniqueSurfaceIds(const std::vector<size_t>&,
            /*out*/std::vector<size_t>*) { return INVALID_OPERATION; };

    /**
     * Update the stream output surfaces.
     */
    virtual status_t updateStream(const std::vector<sp<Surface>> &outputSurfaces,
            const std::vector<OutputStreamInfo> &outputInfo,
            const std::vector<size_t> &removedSurfaceIds,
            KeyedVector<sp<Surface>, size_t> *outputMap/*out*/);

    /**
     * Apply ZSL related consumer usage quirk.
     */
    static void applyZSLUsageQuirk(int format, uint64_t *consumerUsage /*inout*/);

  protected:
    Camera3OutputStream(int id, camera3_stream_type_t type,
            uint32_t width, uint32_t height, int format,
            android_dataspace dataSpace, camera3_stream_rotation_t rotation,
            const String8& physicalCameraId,
            uint64_t consumerUsage = 0, nsecs_t timestampOffset = 0,
            int setId = CAMERA3_STREAM_SET_ID_INVALID);

    /**
     * Note that we release the lock briefly in this function
     */
    virtual status_t returnBufferCheckedLocked(
            const camera3_stream_buffer &buffer,
            nsecs_t timestamp,
            bool output,
            const std::vector<size_t>& surface_ids,
            /*out*/
            sp<Fence> *releaseFenceOut);

    virtual status_t disconnectLocked();

    status_t getEndpointUsageForSurface(uint64_t *usage,
            const sp<Surface>& surface) const;
    status_t configureConsumerQueueLocked();

    // Consumer as the output of camera HAL
    sp<Surface> mConsumer;

    uint64_t getPresetConsumerUsage() const { return mConsumerUsage; }

    static const nsecs_t       kDequeueBufferTimeout   = 1000000000; // 1 sec

    status_t getBufferLockedCommon(ANativeWindowBuffer** anb, int* fenceFd);


  private:

    int               mTransform;

    virtual status_t  setTransformLocked(int transform);

    bool mTraceFirstBuffer;

    // Name of Surface consumer
    String8           mConsumerName;

    // Whether consumer assumes MONOTONIC timestamp
    bool mUseMonoTimestamp;

    /**
     * GraphicBuffer manager this stream is registered to. Used to replace the buffer
     * allocation/deallocation role of BufferQueue.
     */
    sp<Camera3BufferManager> mBufferManager;

    /**
     * Buffer producer listener, used to handle notification when a buffer is released
     * from consumer side, or a set of buffers are discarded by the consumer.
     */
    sp<BufferProducerListener> mBufferProducerListener;

    /**
     * Flag indicating if the buffer manager is used to allocate the stream buffers
     */
    bool mUseBufferManager;

    /**
     * Timestamp offset for video and hardware composer consumed streams
     */
    nsecs_t mTimestampOffset;

    /**
     * Consumer end point usage flag set by the constructor for the deferred
     * consumer case.
     */
    uint64_t    mConsumerUsage;

    // Whether to drop valid buffers.
    bool mDropBuffers;

    /**
     * Internal Camera3Stream interface
     */
    virtual status_t getBufferLocked(camera3_stream_buffer *buffer,
            const std::vector<size_t>& surface_ids);

    virtual status_t returnBufferLocked(
            const camera3_stream_buffer &buffer,
            nsecs_t timestamp, const std::vector<size_t>& surface_ids);

    virtual status_t queueBufferToConsumer(sp<ANativeWindow>& consumer,
            ANativeWindowBuffer* buffer, int anwReleaseFence,
            const std::vector<size_t>& surface_ids);

    virtual status_t configureQueueLocked();

    virtual status_t getEndpointUsage(uint64_t *usage) const;

    /**
     * Private methods
     */
    void onBuffersRemovedLocked(const std::vector<sp<GraphicBuffer>>&);
    status_t detachBufferLocked(sp<GraphicBuffer>* buffer, int* fenceFd);
    // Call this after each dequeueBuffer/attachBuffer/detachNextBuffer call to get update on
    // removed buffers. Set notifyBufferManager to false when the call is initiated by buffer
    // manager so buffer manager doesn't need to be notified.
    void checkRemovedBuffersLocked(bool notifyBufferManager = true);

    // Check return status of IGBP calls and set abandoned state accordingly
    void checkRetAndSetAbandonedLocked(status_t res);

    // If the status indicates abandonded stream, only log when state hasn't been updated to
    // STATE_ABANDONED
    static bool shouldLogError(status_t res, StreamState state);

    static const int32_t kDequeueLatencyBinSize = 5; // in ms
    CameraLatencyHistogram mDequeueBufferLatency;

}; // class Camera3OutputStream

} // namespace camera3

} // namespace android

#endif
