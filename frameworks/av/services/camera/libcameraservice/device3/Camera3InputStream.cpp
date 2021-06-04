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

#define LOG_TAG "Camera3-InputStream"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <gui/BufferItem.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include "Camera3InputStream.h"

namespace android {

namespace camera3 {

const String8 Camera3InputStream::DUMMY_ID;

Camera3InputStream::Camera3InputStream(int id,
        uint32_t width, uint32_t height, int format) :
        Camera3IOStreamBase(id, CAMERA3_STREAM_INPUT, width, height, /*maxSize*/0,
                            format, HAL_DATASPACE_UNKNOWN, CAMERA3_STREAM_ROTATION_0,
                            DUMMY_ID) {

    if (format == HAL_PIXEL_FORMAT_BLOB) {
        ALOGE("%s: Bad format, BLOB not supported", __FUNCTION__);
        mState = STATE_ERROR;
    }
}

Camera3InputStream::~Camera3InputStream() {
    disconnectLocked();
}

status_t Camera3InputStream::getInputBufferLocked(
        camera3_stream_buffer *buffer) {
    ATRACE_CALL();
    status_t res;

    // FIXME: will not work in (re-)registration
    if (mState == STATE_IN_CONFIG || mState == STATE_IN_RECONFIG) {
        ALOGE("%s: Stream %d: Buffer registration for input streams"
              " not implemented (state %d)",
              __FUNCTION__, mId, mState);
        return INVALID_OPERATION;
    }

    if ((res = getBufferPreconditionCheckLocked()) != OK) {
        return res;
    }

    ANativeWindowBuffer* anb;
    int fenceFd;

    assert(mConsumer != 0);

    BufferItem bufferItem;

    res = mConsumer->acquireBuffer(&bufferItem, /*waitForFence*/false);
    if (res != OK) {
        ALOGE("%s: Stream %d: Can't acquire next output buffer: %s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        return res;
    }

    anb = bufferItem.mGraphicBuffer->getNativeBuffer();
    assert(anb != NULL);
    fenceFd = bufferItem.mFence->dup();

    /**
     * FenceFD now owned by HAL except in case of error,
     * in which case we reassign it to acquire_fence
     */
    handoutBufferLocked(*buffer, &(anb->handle), /*acquireFence*/fenceFd,
                        /*releaseFence*/-1, CAMERA3_BUFFER_STATUS_OK, /*output*/false);
    mBuffersInFlight.push_back(bufferItem);

    mFrameCount++;
    mLastTimestamp = bufferItem.mTimestamp;

    return OK;
}

status_t Camera3InputStream::returnBufferCheckedLocked(
            const camera3_stream_buffer &buffer,
            nsecs_t timestamp,
            bool output,
            const std::vector<size_t>&,
            /*out*/
            sp<Fence> *releaseFenceOut) {

    (void)timestamp;
    (void)output;
    ALOG_ASSERT(!output, "Expected output to be false");

    status_t res;

    bool bufferFound = false;
    BufferItem bufferItem;
    {
        // Find the buffer we are returning
        Vector<BufferItem>::iterator it, end;
        for (it = mBuffersInFlight.begin(), end = mBuffersInFlight.end();
             it != end;
             ++it) {

            const BufferItem& tmp = *it;
            ANativeWindowBuffer *anb = tmp.mGraphicBuffer->getNativeBuffer();
            if (anb != NULL && &(anb->handle) == buffer.buffer) {
                bufferFound = true;
                bufferItem = tmp;
                mBuffersInFlight.erase(it);
                break;
            }
        }
    }
    if (!bufferFound) {
        ALOGE("%s: Stream %d: Can't return buffer that wasn't sent to HAL",
              __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    if (buffer.status == CAMERA3_BUFFER_STATUS_ERROR) {
        if (buffer.release_fence != -1) {
            ALOGE("%s: Stream %d: HAL should not set release_fence(%d) when "
                  "there is an error", __FUNCTION__, mId, buffer.release_fence);
            close(buffer.release_fence);
        }

        /**
         * Reassign release fence as the acquire fence incase of error
         */
        const_cast<camera3_stream_buffer*>(&buffer)->release_fence =
                buffer.acquire_fence;
    }

    /**
     * Unconditionally return buffer to the buffer queue.
     * - Fwk takes over the release_fence ownership
     */
    sp<Fence> releaseFence = new Fence(buffer.release_fence);
    res = mConsumer->releaseBuffer(bufferItem, releaseFence);
    if (res != OK) {
        ALOGE("%s: Stream %d: Error releasing buffer back to buffer queue:"
                " %s (%d)", __FUNCTION__, mId, strerror(-res), res);
    }

    *releaseFenceOut = releaseFence;

    return res;
}

status_t Camera3InputStream::returnInputBufferLocked(
        const camera3_stream_buffer &buffer) {
    ATRACE_CALL();

    return returnAnyBufferLocked(buffer, /*timestamp*/0, /*output*/false);
}

status_t Camera3InputStream::getInputBufferProducerLocked(
            sp<IGraphicBufferProducer> *producer) {
    ATRACE_CALL();

    if (producer == NULL) {
        return BAD_VALUE;
    } else if (mProducer == NULL) {
        ALOGE("%s: No input stream is configured", __FUNCTION__);
        return INVALID_OPERATION;
    }

    *producer = mProducer;
    return OK;
}

status_t Camera3InputStream::disconnectLocked() {

    status_t res;

    if ((res = Camera3IOStreamBase::disconnectLocked()) != OK) {
        return res;
    }

    assert(mBuffersInFlight.size() == 0);

    mConsumer->abandon();

    /**
     *  no-op since we can't disconnect the producer from the consumer-side
     */

    mState = (mState == STATE_IN_RECONFIG) ? STATE_IN_CONFIG
                                           : STATE_CONSTRUCTED;
    return OK;
}

void Camera3InputStream::dump(int fd, const Vector<String16> &args) const {
    (void) args;
    String8 lines;
    lines.appendFormat("    Stream[%d]: Input\n", mId);
    write(fd, lines.string(), lines.size());

    Camera3IOStreamBase::dump(fd, args);
}

status_t Camera3InputStream::configureQueueLocked() {
    status_t res;

    if ((res = Camera3IOStreamBase::configureQueueLocked()) != OK) {
        return res;
    }

    assert(mMaxSize == 0);
    assert(camera3_stream::format != HAL_PIXEL_FORMAT_BLOB);

    mHandoutTotalBufferCount = 0;
    mFrameCount = 0;
    mLastTimestamp = 0;

    if (mConsumer.get() == 0) {
        sp<IGraphicBufferProducer> producer;
        sp<IGraphicBufferConsumer> consumer;
        BufferQueue::createBufferQueue(&producer, &consumer);

        int minUndequeuedBuffers = 0;
        res = producer->query(NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeuedBuffers);
        if (res != OK || minUndequeuedBuffers < 0) {
            ALOGE("%s: Stream %d: Could not query min undequeued buffers (error %d, bufCount %d)",
                    __FUNCTION__, mId, res, minUndequeuedBuffers);
            return res;
        }
        size_t minBufs = static_cast<size_t>(minUndequeuedBuffers);

        if (camera3_stream::max_buffers == 0) {
            ALOGE("%s: %d: HAL sets max_buffer to 0. Must be at least 1.",
                    __FUNCTION__, __LINE__);
            return INVALID_OPERATION;
        }

        /*
         * We promise never to 'acquire' more than camera3_stream::max_buffers
         * at any one time.
         *
         * Boost the number up to meet the minimum required buffer count.
         *
         * (Note that this sets consumer-side buffer count only,
         * and not the sum of producer+consumer side as in other camera streams).
         */
        mTotalBufferCount = camera3_stream::max_buffers > minBufs ?
            camera3_stream::max_buffers : minBufs;
        // TODO: somehow set the total buffer count when producer connects?

        mConsumer = new BufferItemConsumer(consumer, mUsage,
                                           mTotalBufferCount);
        mConsumer->setName(String8::format("Camera3-InputStream-%d", mId));

        mProducer = producer;

        mConsumer->setBufferFreedListener(this);
    }

    res = mConsumer->setDefaultBufferSize(camera3_stream::width,
                                          camera3_stream::height);
    if (res != OK) {
        ALOGE("%s: Stream %d: Could not set buffer dimensions %dx%d",
              __FUNCTION__, mId, camera3_stream::width, camera3_stream::height);
        return res;
    }
    res = mConsumer->setDefaultBufferFormat(camera3_stream::format);
    if (res != OK) {
        ALOGE("%s: Stream %d: Could not set buffer format %d",
              __FUNCTION__, mId, camera3_stream::format);
        return res;
    }

    return OK;
}

status_t Camera3InputStream::getEndpointUsage(uint64_t *usage) const {
    // Per HAL3 spec, input streams have 0 for their initial usage field.
    *usage = 0;
    return OK;
}

void Camera3InputStream::onBufferFreed(const wp<GraphicBuffer>& gb) {
    const sp<GraphicBuffer> buffer = gb.promote();
    if (buffer != nullptr) {
        camera3_stream_buffer streamBuffer =
                {nullptr, &buffer->handle, 0, -1, -1};
        // Check if this buffer is outstanding.
        if (isOutstandingBuffer(streamBuffer)) {
            ALOGV("%s: Stream %d: Trying to free a buffer that is still being "
                    "processed.", __FUNCTION__, mId);
            return;
        }

        sp<Camera3StreamBufferFreedListener> callback = mBufferFreedListener.promote();
        if (callback != nullptr) {
            callback->onBufferFreed(mId, buffer->handle);
        }
    } else {
        ALOGE("%s: GraphicBuffer is freed before onBufferFreed callback finishes!", __FUNCTION__);
    }
}

}; // namespace camera3

}; // namespace android
