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

#ifndef ANDROID_SERVERS_CAMERA_CAMERADEVICEBASE_H
#define ANDROID_SERVERS_CAMERA_CAMERADEVICEBASE_H

#include <list>

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/String16.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/Timers.h>
#include <utils/List.h>

#include "hardware/camera2.h"
#include "hardware/camera3.h"
#include "camera/CameraMetadata.h"
#include "camera/CaptureResult.h"
#include "gui/IGraphicBufferProducer.h"
#include "device3/Camera3StreamInterface.h"
#include "binder/Status.h"

namespace android {

class CameraProviderManager;

// Mapping of output stream index to surface ids
typedef std::unordered_map<int, std::vector<size_t> > SurfaceMap;

/**
 * Base interface for version >= 2 camera device classes, which interface to
 * camera HAL device versions >= 2.
 */
class CameraDeviceBase : public virtual RefBase {
  public:
    virtual ~CameraDeviceBase();

    /**
     * The device's camera ID
     */
    virtual const String8& getId() const = 0;

    /**
     * The device vendor tag ID
     */
    virtual metadata_vendor_id_t getVendorTagId() const = 0;

    virtual status_t initialize(sp<CameraProviderManager> manager, const String8& monitorTags) = 0;
    virtual status_t disconnect() = 0;

    virtual status_t dump(int fd, const Vector<String16> &args) = 0;

    /**
     * The device's static characteristics metadata buffer
     */
    virtual const CameraMetadata& info() const = 0;
    /**
     * The physical camera device's static characteristics metadata buffer
     */
    virtual const CameraMetadata& info(const String8& physicalId) const = 0;

    struct PhysicalCameraSettings {
        std::string cameraId;
        CameraMetadata metadata;
    };
    typedef List<PhysicalCameraSettings> PhysicalCameraSettingsList;

    /**
     * Submit request for capture. The CameraDevice takes ownership of the
     * passed-in buffer.
     * Output lastFrameNumber is the expected frame number of this request.
     */
    virtual status_t capture(CameraMetadata &request, int64_t *lastFrameNumber = NULL) = 0;

    /**
     * Submit a list of requests.
     * Output lastFrameNumber is the expected last frame number of the list of requests.
     */
    virtual status_t captureList(const List<const PhysicalCameraSettingsList> &requests,
                                 const std::list<const SurfaceMap> &surfaceMaps,
                                 int64_t *lastFrameNumber = NULL) = 0;

    /**
     * Submit request for streaming. The CameraDevice makes a copy of the
     * passed-in buffer and the caller retains ownership.
     * Output lastFrameNumber is the last frame number of the previous streaming request.
     */
    virtual status_t setStreamingRequest(const CameraMetadata &request,
                                         int64_t *lastFrameNumber = NULL) = 0;

    /**
     * Submit a list of requests for streaming.
     * Output lastFrameNumber is the last frame number of the previous streaming request.
     */
    virtual status_t setStreamingRequestList(const List<const PhysicalCameraSettingsList> &requests,
                                             const std::list<const SurfaceMap> &surfaceMaps,
                                             int64_t *lastFrameNumber = NULL) = 0;

    /**
     * Clear the streaming request slot.
     * Output lastFrameNumber is the last frame number of the previous streaming request.
     */
    virtual status_t clearStreamingRequest(int64_t *lastFrameNumber = NULL) = 0;

    /**
     * Wait until a request with the given ID has been dequeued by the
     * HAL. Returns TIMED_OUT if the timeout duration is reached. Returns
     * immediately if the latest request received by the HAL has this id.
     */
    virtual status_t waitUntilRequestReceived(int32_t requestId,
            nsecs_t timeout) = 0;

    /**
     * Create an output stream of the requested size, format, rotation and dataspace
     *
     * For HAL_PIXEL_FORMAT_BLOB formats, the width and height should be the
     * logical dimensions of the buffer, not the number of bytes.
     */
    virtual status_t createStream(sp<Surface> consumer,
            uint32_t width, uint32_t height, int format,
            android_dataspace dataSpace, camera3_stream_rotation_t rotation, int *id,
            const String8& physicalCameraId,
            std::vector<int> *surfaceIds = nullptr,
            int streamSetId = camera3::CAMERA3_STREAM_SET_ID_INVALID,
            bool isShared = false, uint64_t consumerUsage = 0) = 0;

    /**
     * Create an output stream of the requested size, format, rotation and
     * dataspace with a number of consumers.
     *
     * For HAL_PIXEL_FORMAT_BLOB formats, the width and height should be the
     * logical dimensions of the buffer, not the number of bytes.
     */
    virtual status_t createStream(const std::vector<sp<Surface>>& consumers,
            bool hasDeferredConsumer, uint32_t width, uint32_t height, int format,
            android_dataspace dataSpace, camera3_stream_rotation_t rotation, int *id,
            const String8& physicalCameraId,
            std::vector<int> *surfaceIds = nullptr,
            int streamSetId = camera3::CAMERA3_STREAM_SET_ID_INVALID,
            bool isShared = false, uint64_t consumerUsage = 0) = 0;

    /**
     * Create an input stream of width, height, and format.
     *
     * Return value is the stream ID if non-negative and an error if negative.
     */
    virtual status_t createInputStream(uint32_t width, uint32_t height,
            int32_t format, /*out*/ int32_t *id) = 0;

    struct StreamInfo {
        uint32_t width;
        uint32_t height;

        uint32_t format;
        bool formatOverridden;
        uint32_t originalFormat;

        android_dataspace dataSpace;
        bool dataSpaceOverridden;
        android_dataspace originalDataSpace;

        StreamInfo() : width(0), height(0), format(0), formatOverridden(false), originalFormat(0),
                dataSpace(HAL_DATASPACE_UNKNOWN), dataSpaceOverridden(false),
                originalDataSpace(HAL_DATASPACE_UNKNOWN) {}
        /**
         * Check whether the format matches the current or the original one in case
         * it got overridden.
         */
        bool matchFormat(uint32_t clientFormat) const {
            if ((formatOverridden && (originalFormat == clientFormat)) ||
                    (format == clientFormat)) {
                return true;
            }
            return false;
        }

        /**
         * Check whether the dataspace matches the current or the original one in case
         * it got overridden.
         */
        bool matchDataSpace(android_dataspace clientDataSpace) const {
            if ((dataSpaceOverridden && (originalDataSpace == clientDataSpace)) ||
                    (dataSpace == clientDataSpace)) {
                return true;
            }
            return false;
        }

    };

    /**
     * Get information about a given stream.
     */
    virtual status_t getStreamInfo(int id, StreamInfo *streamInfo) = 0;

    /**
     * Set stream gralloc buffer transform
     */
    virtual status_t setStreamTransform(int id, int transform) = 0;

    /**
     * Delete stream. Must not be called if there are requests in flight which
     * reference that stream.
     */
    virtual status_t deleteStream(int id) = 0;

    /**
     * Take the currently-defined set of streams and configure the HAL to use
     * them. This is a long-running operation (may be several hundered ms).
     *
     * The device must be idle (see waitUntilDrained) before calling this.
     *
     * Returns OK on success; otherwise on error:
     * - BAD_VALUE if the set of streams was invalid (e.g. fmts or sizes)
     * - INVALID_OPERATION if the device was in the wrong state
     */
    virtual status_t configureStreams(const CameraMetadata& sessionParams,
            int operatingMode = 0) = 0;

    // get the buffer producer of the input stream
    virtual status_t getInputBufferProducer(
            sp<IGraphicBufferProducer> *producer) = 0;

    /**
     * Create a metadata buffer with fields that the HAL device believes are
     * best for the given use case
     */
    virtual status_t createDefaultRequest(int templateId,
            CameraMetadata *request) = 0;

    /**
     * Wait until all requests have been processed. Returns INVALID_OPERATION if
     * the streaming slot is not empty, or TIMED_OUT if the requests haven't
     * finished processing in 10 seconds.
     */
    virtual status_t waitUntilDrained() = 0;

    /**
     * Get Jpeg buffer size for a given jpeg resolution.
     * Negative values are error codes.
     */
    virtual ssize_t getJpegBufferSize(uint32_t width, uint32_t height) const = 0;

    /**
     * Abstract class for HAL notification listeners
     */
    class NotificationListener : public virtual RefBase {
      public:
        // The set of notifications is a merge of the notifications required for
        // API1 and API2.

        // Required for API 1 and 2
        virtual void notifyError(int32_t errorCode,
                                 const CaptureResultExtras &resultExtras) = 0;

        // Required only for API2
        virtual void notifyIdle() = 0;
        virtual void notifyShutter(const CaptureResultExtras &resultExtras,
                nsecs_t timestamp) = 0;
        virtual void notifyPrepared(int streamId) = 0;
        virtual void notifyRequestQueueEmpty() = 0;

        // Required only for API1
        virtual void notifyAutoFocus(uint8_t newState, int triggerId) = 0;
        virtual void notifyAutoExposure(uint8_t newState, int triggerId) = 0;
        virtual void notifyAutoWhitebalance(uint8_t newState,
                int triggerId) = 0;
        virtual void notifyRepeatingRequestError(long lastFrameNumber) = 0;
      protected:
        virtual ~NotificationListener();
    };

    /**
     * Connect HAL notifications to a listener. Overwrites previous
     * listener. Set to NULL to stop receiving notifications.
     */
    virtual status_t setNotifyCallback(wp<NotificationListener> listener) = 0;

    /**
     * Whether the device supports calling notifyAutofocus, notifyAutoExposure,
     * and notifyAutoWhitebalance; if this returns false, the client must
     * synthesize these notifications from received frame metadata.
     */
    virtual bool     willNotify3A() = 0;

    /**
     * Wait for a new frame to be produced, with timeout in nanoseconds.
     * Returns TIMED_OUT when no frame produced within the specified duration
     * May be called concurrently to most methods, except for getNextFrame
     */
    virtual status_t waitForNextFrame(nsecs_t timeout) = 0;

    /**
     * Get next capture result frame from the result queue. Returns NOT_ENOUGH_DATA
     * if the queue is empty; caller takes ownership of the metadata buffer inside
     * the capture result object's metadata field.
     * May be called concurrently to most methods, except for waitForNextFrame.
     */
    virtual status_t getNextResult(CaptureResult *frame) = 0;

    /**
     * Trigger auto-focus. The latest ID used in a trigger autofocus or cancel
     * autofocus call will be returned by the HAL in all subsequent AF
     * notifications.
     */
    virtual status_t triggerAutofocus(uint32_t id) = 0;

    /**
     * Cancel auto-focus. The latest ID used in a trigger autofocus/cancel
     * autofocus call will be returned by the HAL in all subsequent AF
     * notifications.
     */
    virtual status_t triggerCancelAutofocus(uint32_t id) = 0;

    /**
     * Trigger pre-capture metering. The latest ID used in a trigger pre-capture
     * call will be returned by the HAL in all subsequent AE and AWB
     * notifications.
     */
    virtual status_t triggerPrecaptureMetering(uint32_t id) = 0;

    /**
     * Flush all pending and in-flight requests. Blocks until flush is
     * complete.
     * Output lastFrameNumber is the last frame number of the previous streaming request.
     */
    virtual status_t flush(int64_t *lastFrameNumber = NULL) = 0;

    /**
     * Prepare stream by preallocating buffers for it asynchronously.
     * Calls notifyPrepared() once allocation is complete.
     */
    virtual status_t prepare(int streamId) = 0;

    /**
     * Free stream resources by dumping its unused gralloc buffers.
     */
    virtual status_t tearDown(int streamId) = 0;

    /**
     * Add buffer listener for a particular stream in the device.
     */
    virtual status_t addBufferListenerForStream(int streamId,
            wp<camera3::Camera3StreamBufferListener> listener) = 0;

    /**
     * Prepare stream by preallocating up to maxCount buffers for it asynchronously.
     * Calls notifyPrepared() once allocation is complete.
     */
    virtual status_t prepare(int maxCount, int streamId) = 0;

    /**
     * Set the deferred consumer surface and finish the rest of the stream configuration.
     */
    virtual status_t setConsumerSurfaces(int streamId,
            const std::vector<sp<Surface>>& consumers, std::vector<int> *surfaceIds /*out*/) = 0;

    /**
     * Update a given stream.
     */
    virtual status_t updateStream(int streamId, const std::vector<sp<Surface>> &newSurfaces,
            const std::vector<android::camera3::OutputStreamInfo> &outputInfo,
            const std::vector<size_t> &removedSurfaceIds,
            KeyedVector<sp<Surface>, size_t> *outputMap/*out*/) = 0;

    /**
     * Drop buffers for stream of streamId if dropping is true. If dropping is false, do not
     * drop buffers for stream of streamId.
     */
    virtual status_t dropStreamBuffers(bool /*dropping*/, int /*streamId*/) = 0;
};

}; // namespace android

#endif
