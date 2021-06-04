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

#define LOG_TAG "Camera3-IOStreamBase"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <inttypes.h>

#include <utils/Log.h>
#include <utils/Trace.h>
#include "device3/Camera3IOStreamBase.h"
#include "device3/StatusTracker.h"

namespace android {

namespace camera3 {

Camera3IOStreamBase::Camera3IOStreamBase(int id, camera3_stream_type_t type,
        uint32_t width, uint32_t height, size_t maxSize, int format,
        android_dataspace dataSpace, camera3_stream_rotation_t rotation,
        const String8& physicalCameraId, int setId) :
        Camera3Stream(id, type,
                width, height, maxSize, format, dataSpace, rotation,
                physicalCameraId, setId),
        mTotalBufferCount(0),
        mHandoutTotalBufferCount(0),
        mHandoutOutputBufferCount(0),
        mFrameCount(0),
        mLastTimestamp(0) {

    mCombinedFence = new Fence();

    if (maxSize > 0 &&
            (format != HAL_PIXEL_FORMAT_BLOB && format != HAL_PIXEL_FORMAT_RAW_OPAQUE)) {
        ALOGE("%s: Bad format for size-only stream: %d", __FUNCTION__,
                format);
        mState = STATE_ERROR;
    }
}

Camera3IOStreamBase::~Camera3IOStreamBase() {
    disconnectLocked();
}

bool Camera3IOStreamBase::hasOutstandingBuffersLocked() const {
    nsecs_t signalTime = mCombinedFence->getSignalTime();
    ALOGV("%s: Stream %d: Has %zu outstanding buffers,"
            " buffer signal time is %" PRId64,
            __FUNCTION__, mId, mHandoutTotalBufferCount, signalTime);
    if (mHandoutTotalBufferCount > 0 || signalTime == INT64_MAX) {
        return true;
    }
    return false;
}

void Camera3IOStreamBase::dump(int fd, const Vector<String16> &args) const {
    (void) args;
    String8 lines;

    uint64_t consumerUsage = 0;
    status_t res = getEndpointUsage(&consumerUsage);
    if (res != OK) consumerUsage = 0;

    lines.appendFormat("      State: %d\n", mState);
    lines.appendFormat("      Dims: %d x %d, format 0x%x, dataspace 0x%x\n",
            camera3_stream::width, camera3_stream::height,
            camera3_stream::format, camera3_stream::data_space);
    lines.appendFormat("      Max size: %zu\n", mMaxSize);
    lines.appendFormat("      Combined usage: %" PRIu64 ", max HAL buffers: %d\n",
            mUsage | consumerUsage, camera3_stream::max_buffers);
    if (strlen(camera3_stream::physical_camera_id) > 0) {
        lines.appendFormat("      Physical camera id: %s\n", camera3_stream::physical_camera_id);
    }
    lines.appendFormat("      Frames produced: %d, last timestamp: %" PRId64 " ns\n",
            mFrameCount, mLastTimestamp);
    lines.appendFormat("      Total buffers: %zu, currently dequeued: %zu\n",
            mTotalBufferCount, mHandoutTotalBufferCount);
    write(fd, lines.string(), lines.size());

    Camera3Stream::dump(fd, args);
}

status_t Camera3IOStreamBase::configureQueueLocked() {
    status_t res;

    switch (mState) {
        case STATE_IN_RECONFIG:
            res = disconnectLocked();
            if (res != OK) {
                return res;
            }
            break;
        case STATE_IN_CONFIG:
            // OK
            break;
        default:
            ALOGE("%s: Bad state: %d", __FUNCTION__, mState);
            return INVALID_OPERATION;
    }

    return OK;
}

size_t Camera3IOStreamBase::getBufferCountLocked() {
    return mTotalBufferCount;
}

size_t Camera3IOStreamBase::getHandoutOutputBufferCountLocked() const {
    return mHandoutOutputBufferCount;
}

size_t Camera3IOStreamBase::getHandoutInputBufferCountLocked() {
    return (mHandoutTotalBufferCount - mHandoutOutputBufferCount);
}

status_t Camera3IOStreamBase::disconnectLocked() {
    switch (mState) {
        case STATE_IN_RECONFIG:
        case STATE_CONFIGURED:
        case STATE_ABANDONED:
            // OK
            break;
        default:
            // No connection, nothing to do
            ALOGV("%s: Stream %d: Already disconnected",
                  __FUNCTION__, mId);
            return -ENOTCONN;
    }

    if (mHandoutTotalBufferCount > 0) {
        ALOGE("%s: Can't disconnect with %zu buffers still dequeued!",
                __FUNCTION__, mHandoutTotalBufferCount);
        return INVALID_OPERATION;
    }

   return OK;
}

void Camera3IOStreamBase::handoutBufferLocked(camera3_stream_buffer &buffer,
                                              buffer_handle_t *handle,
                                              int acquireFence,
                                              int releaseFence,
                                              camera3_buffer_status_t status,
                                              bool output) {
    /**
     * Note that all fences are now owned by HAL.
     */

    // Handing out a raw pointer to this object. Increment internal refcount.
    incStrong(this);
    buffer.stream = this;
    buffer.buffer = handle;
    buffer.acquire_fence = acquireFence;
    buffer.release_fence = releaseFence;
    buffer.status = status;

    // Inform tracker about becoming busy
    if (mHandoutTotalBufferCount == 0 && mState != STATE_IN_CONFIG &&
            mState != STATE_IN_RECONFIG && mState != STATE_PREPARING) {
        /**
         * Avoid a spurious IDLE->ACTIVE->IDLE transition when using buffers
         * before/after register_stream_buffers during initial configuration
         * or re-configuration, or during prepare pre-allocation
         */
        sp<StatusTracker> statusTracker = mStatusTracker.promote();
        if (statusTracker != 0) {
            statusTracker->markComponentActive(mStatusId);
        }
    }
    mHandoutTotalBufferCount++;

    if (output) {
        mHandoutOutputBufferCount++;
    }
}

status_t Camera3IOStreamBase::getBufferPreconditionCheckLocked() const {
    // Allow dequeue during IN_[RE]CONFIG for registration, in
    // PREPARING for pre-allocation
    if (mState != STATE_CONFIGURED &&
            mState != STATE_IN_CONFIG && mState != STATE_IN_RECONFIG &&
            mState != STATE_PREPARING) {
        ALOGE("%s: Stream %d: Can't get buffers in unconfigured state %d",
                __FUNCTION__, mId, mState);
        return INVALID_OPERATION;
    }

    return OK;
}

status_t Camera3IOStreamBase::returnBufferPreconditionCheckLocked() const {
    // Allow buffers to be returned in the error state, to allow for disconnect
    // and in the in-config states for registration
    if (mState == STATE_CONSTRUCTED) {
        ALOGE("%s: Stream %d: Can't return buffers in unconfigured state %d",
                __FUNCTION__, mId, mState);
        return INVALID_OPERATION;
    }
    if (mHandoutTotalBufferCount == 0) {
        ALOGE("%s: Stream %d: No buffers outstanding to return", __FUNCTION__,
                mId);
        return INVALID_OPERATION;
    }

    return OK;
}

status_t Camera3IOStreamBase::returnAnyBufferLocked(
        const camera3_stream_buffer &buffer,
        nsecs_t timestamp,
        bool output,
        const std::vector<size_t>& surface_ids) {
    status_t res;

    // returnBuffer may be called from a raw pointer, not a sp<>, and we'll be
    // decrementing the internal refcount next. In case this is the last ref, we
    // might get destructed on the decStrong(), so keep an sp around until the
    // end of the call - otherwise have to sprinkle the decStrong on all exit
    // points.
    sp<Camera3IOStreamBase> keepAlive(this);
    decStrong(this);

    if ((res = returnBufferPreconditionCheckLocked()) != OK) {
        return res;
    }

    sp<Fence> releaseFence;
    res = returnBufferCheckedLocked(buffer, timestamp, output, surface_ids,
                                    &releaseFence);
    // Res may be an error, but we still want to decrement our owned count
    // to enable clean shutdown. So we'll just return the error but otherwise
    // carry on

    if (releaseFence != 0) {
        mCombinedFence = Fence::merge(mName, mCombinedFence, releaseFence);
    }

    if (output) {
        mHandoutOutputBufferCount--;
    }

    mHandoutTotalBufferCount--;
    if (mHandoutTotalBufferCount == 0 && mState != STATE_IN_CONFIG &&
            mState != STATE_IN_RECONFIG && mState != STATE_PREPARING) {
        /**
         * Avoid a spurious IDLE->ACTIVE->IDLE transition when using buffers
         * before/after register_stream_buffers during initial configuration
         * or re-configuration, or during prepare pre-allocation
         */
        ALOGV("%s: Stream %d: All buffers returned; now idle", __FUNCTION__,
                mId);
        sp<StatusTracker> statusTracker = mStatusTracker.promote();
        if (statusTracker != 0) {
            statusTracker->markComponentIdle(mStatusId, mCombinedFence);
        }
    }

    mBufferReturnedSignal.signal();

    if (output) {
        mLastTimestamp = timestamp;
    }

    return res;
}



}; // namespace camera3

}; // namespace android
