/*
 * Copyright (C) 2014-2018 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA3_DUMMY_STREAM_H
#define ANDROID_SERVERS_CAMERA3_DUMMY_STREAM_H

#include <utils/RefBase.h>
#include <gui/Surface.h>

#include "Camera3Stream.h"
#include "Camera3IOStreamBase.h"
#include "Camera3OutputStreamInterface.h"

namespace android {
namespace camera3 {

/**
 * A dummy output stream class, to be used as a placeholder when no valid
 * streams are configured by the client.
 * This is necessary because camera HAL v3.2 or older disallow configuring
 * 0 output streams, while the public camera2 API allows for it.
 */
class Camera3DummyStream :
        public Camera3IOStreamBase,
        public Camera3OutputStreamInterface {

  public:
    /**
     * Set up a dummy stream; doesn't actually connect to anything, and uses
     * a default dummy format and size.
     */
    explicit Camera3DummyStream(int id);

    virtual ~Camera3DummyStream();

    /**
     * Camera3Stream interface
     */

    virtual void     dump(int fd, const Vector<String16> &args) const;

    status_t         setTransform(int transform);

    virtual status_t detachBuffer(sp<GraphicBuffer>* buffer, int* fenceFd);

    /**
     * Drop buffers for stream of streamId if dropping is true. If dropping is false, do not
     * drop buffers for stream of streamId.
     */
    virtual status_t dropBuffers(bool /*dropping*/) override;

    /**
     * Query the physical camera id for the output stream.
     */
    virtual const String8& getPhysicalCameraId() const override;

    /**
     * Return if this output stream is for video encoding.
     */
    bool isVideoStream() const;

    /**
     * Return if the consumer configuration of this stream is deferred.
     */
    virtual bool isConsumerConfigurationDeferred(size_t surface_id) const;

    /**
     * Set the consumer surfaces to the output stream.
     */
    virtual status_t setConsumers(const std::vector<sp<Surface>>& consumers);

    /**
     * Query the output surface id.
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

  protected:

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

  private:

    // Default dummy parameters; 320x240 is a required size for all devices,
    // otherwise act like a SurfaceView would.
    static const int DUMMY_WIDTH = 320;
    static const int DUMMY_HEIGHT = 240;
    static const int DUMMY_FORMAT = HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;
    static const android_dataspace DUMMY_DATASPACE = HAL_DATASPACE_UNKNOWN;
    static const camera3_stream_rotation_t DUMMY_ROTATION = CAMERA3_STREAM_ROTATION_0;
    static const uint64_t DUMMY_USAGE = GRALLOC_USAGE_HW_COMPOSER;
    static const String8 DUMMY_ID;

    /**
     * Internal Camera3Stream interface
     */
    virtual status_t getBufferLocked(camera3_stream_buffer *buffer,
            const std::vector<size_t>& surface_ids = std::vector<size_t>());
    virtual status_t returnBufferLocked(
            const camera3_stream_buffer &buffer,
            nsecs_t timestamp, const std::vector<size_t>& surface_ids);

    virtual status_t configureQueueLocked();

    virtual status_t getEndpointUsage(uint64_t *usage) const;

}; // class Camera3DummyStream

} // namespace camera3

} // namespace android

#endif
