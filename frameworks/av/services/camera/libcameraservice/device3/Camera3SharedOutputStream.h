/*
 * Copyright (C) 2016-2018 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA3_SHARED_OUTPUT_STREAM_H
#define ANDROID_SERVERS_CAMERA3_SHARED_OUTPUT_STREAM_H

#include <array>
#include "Camera3StreamSplitter.h"
#include "Camera3OutputStream.h"

namespace android {

namespace camera3 {

class Camera3SharedOutputStream :
        public Camera3OutputStream {
public:
    /**
     * Set up a stream for formats that have 2 dimensions, with multiple
     * surfaces. A valid stream set id needs to be set to support buffer
     * sharing between multiple streams.
     */
    Camera3SharedOutputStream(int id, const std::vector<sp<Surface>>& surfaces,
            uint32_t width, uint32_t height, int format,
            uint64_t consumerUsage, android_dataspace dataSpace,
            camera3_stream_rotation_t rotation, nsecs_t timestampOffset,
            const String8& physicalCameraId,
            int setId = CAMERA3_STREAM_SET_ID_INVALID,
            bool useHalBufManager = false);

    virtual ~Camera3SharedOutputStream();

    virtual status_t notifyBufferReleased(ANativeWindowBuffer *buffer);

    virtual bool isConsumerConfigurationDeferred(size_t surface_id) const;

    virtual status_t setConsumers(const std::vector<sp<Surface>>& consumers);

    virtual ssize_t getSurfaceId(const sp<Surface> &surface);

    /**
     * Query the unique surface IDs of current surfaceIds.
     * When passing unique surface IDs in returnBuffer(), if the
     * surfaceId has been removed from the stream, the output corresponding to
     * the unique surface ID will be ignored and not delivered to client.
     */
    virtual status_t getUniqueSurfaceIds(const std::vector<size_t>& surfaceIds,
            /*out*/std::vector<size_t>* outUniqueIds) override;

    virtual status_t updateStream(const std::vector<sp<Surface>> &outputSurfaces,
            const std::vector<OutputStreamInfo> &outputInfo,
            const std::vector<size_t> &removedSurfaceIds,
            KeyedVector<sp<Surface>, size_t> *outputMap/*out*/);

private:

    static const size_t kMaxOutputs = 4;

    // Whether HAL is in control for buffer management. Surface sharing behavior
    // depends on this flag.
    const bool mUseHalBufManager;

    // Pair of an output Surface and its unique ID
    typedef std::pair<sp<Surface>, size_t> SurfaceUniqueId;

    // Map surfaceId -> (output surface, unique surface ID)
    std::array<SurfaceUniqueId, kMaxOutputs> mSurfaceUniqueIds;

    size_t mNextUniqueSurfaceId = 0;

    ssize_t getNextSurfaceIdLocked();

    status_t revertPartialUpdateLocked(const KeyedVector<sp<Surface>, size_t> &removedSurfaces,
            const KeyedVector<sp<Surface>, size_t> &attachedSurfaces);

    /**
     * The Camera3StreamSplitter object this stream uses for stream
     * sharing.
     */
    sp<Camera3StreamSplitter> mStreamSplitter;

    /**
     * Initialize stream splitter.
     */
    status_t connectStreamSplitterLocked();

    /**
     * Attach the output buffer to stream splitter.
     * When camera service is doing buffer management, this method will be called
     * before the buffer is handed out to HAL in request thread.
     * When HAL is doing buffer management, this method will be called when
     * the buffer is returned from HAL in hwbinder callback thread.
     */
    status_t attachBufferToSplitterLocked(ANativeWindowBuffer* anb,
            const std::vector<size_t>& surface_ids);

    /**
     * Internal Camera3Stream interface
     */
    virtual status_t getBufferLocked(camera3_stream_buffer *buffer,
            const std::vector<size_t>& surface_ids);

    virtual status_t queueBufferToConsumer(sp<ANativeWindow>& consumer,
            ANativeWindowBuffer* buffer, int anwReleaseFence,
            const std::vector<size_t>& uniqueSurfaceIds);

    virtual status_t configureQueueLocked();

    virtual status_t disconnectLocked();

    virtual status_t getEndpointUsage(uint64_t *usage) const;

}; // class Camera3SharedOutputStream

} // namespace camera3

} // namespace android

#endif // ANDROID_SERVERS_CAMERA3_SHARED_OUTPUT_STREAM_H
