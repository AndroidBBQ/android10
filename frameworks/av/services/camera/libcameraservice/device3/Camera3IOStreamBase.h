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

#ifndef ANDROID_SERVERS_CAMERA3_IO_STREAM_BASE_H
#define ANDROID_SERVERS_CAMERA3_IO_STREAM_BASE_H

#include <utils/RefBase.h>
#include <gui/Surface.h>

#include "Camera3Stream.h"

namespace android {

namespace camera3 {

/**
 * A base class for managing a single stream of I/O data from the camera device.
 */
class Camera3IOStreamBase :
        public Camera3Stream {
  protected:
    Camera3IOStreamBase(int id, camera3_stream_type_t type,
            uint32_t width, uint32_t height, size_t maxSize, int format,
            android_dataspace dataSpace, camera3_stream_rotation_t rotation,
            const String8& physicalCameraId,
            int setId = CAMERA3_STREAM_SET_ID_INVALID);

  public:

    virtual ~Camera3IOStreamBase();

    /**
     * Camera3Stream interface
     */

    virtual void     dump(int fd, const Vector<String16> &args) const;

  protected:
    size_t            mTotalBufferCount;
    // sum of input and output buffers that are currently acquired by HAL
    size_t            mHandoutTotalBufferCount;
    // number of output buffers that are currently acquired by HAL. This will be
    // Redundant when camera3 streams are no longer bidirectional streams.
    size_t            mHandoutOutputBufferCount;
    Condition         mBufferReturnedSignal;
    uint32_t          mFrameCount;
    // Last received output buffer's timestamp
    nsecs_t           mLastTimestamp;

    // The merged release fence for all returned buffers
    sp<Fence>         mCombinedFence;

    status_t         returnAnyBufferLocked(
            const camera3_stream_buffer &buffer,
            nsecs_t timestamp,
            bool output,
            const std::vector<size_t>& surface_ids = std::vector<size_t>());

    virtual status_t returnBufferCheckedLocked(
            const camera3_stream_buffer &buffer,
            nsecs_t timestamp,
            bool output,
            const std::vector<size_t>& surface_ids,
            /*out*/
            sp<Fence> *releaseFenceOut) = 0;

    /**
     * Internal Camera3Stream interface
     */
    virtual bool     hasOutstandingBuffersLocked() const;

    virtual size_t   getBufferCountLocked();

    virtual size_t   getHandoutOutputBufferCountLocked() const;

    virtual size_t   getHandoutInputBufferCountLocked();

    virtual status_t getEndpointUsage(uint64_t *usage) const = 0;

    status_t getBufferPreconditionCheckLocked() const;
    status_t returnBufferPreconditionCheckLocked() const;

    // State check only
    virtual status_t configureQueueLocked();
    // State checks only
    virtual status_t disconnectLocked();

    // Hand out the buffer to a native location,
    //   incrementing the internal refcount and dequeued buffer count
    void handoutBufferLocked(camera3_stream_buffer &buffer,
                             buffer_handle_t *handle,
                             int acquire_fence,
                             int release_fence,
                             camera3_buffer_status_t status,
                             bool output);

}; // class Camera3IOStreamBase

} // namespace camera3

} // namespace android

#endif
