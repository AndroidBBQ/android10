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

#define LOG_TAG "Camera3-Stream"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <utils/Trace.h>
#include "device3/Camera3Stream.h"
#include "device3/StatusTracker.h"

#include <cutils/properties.h>

namespace android {

namespace camera3 {

Camera3Stream::~Camera3Stream() {
    sp<StatusTracker> statusTracker = mStatusTracker.promote();
    if (statusTracker != 0 && mStatusId != StatusTracker::NO_STATUS_ID) {
        statusTracker->removeComponent(mStatusId);
    }
}

Camera3Stream* Camera3Stream::cast(camera3_stream *stream) {
    return static_cast<Camera3Stream*>(stream);
}

const Camera3Stream* Camera3Stream::cast(const camera3_stream *stream) {
    return static_cast<const Camera3Stream*>(stream);
}

Camera3Stream::Camera3Stream(int id,
        camera3_stream_type type,
        uint32_t width, uint32_t height, size_t maxSize, int format,
        android_dataspace dataSpace, camera3_stream_rotation_t rotation,
        const String8& physicalCameraId, int setId) :
    camera3_stream(),
    mId(id),
    mSetId(setId),
    mName(String8::format("Camera3Stream[%d]", id)),
    mMaxSize(maxSize),
    mState(STATE_CONSTRUCTED),
    mStatusId(StatusTracker::NO_STATUS_ID),
    mStreamUnpreparable(true),
    mUsage(0),
    mOldUsage(0),
    mOldMaxBuffers(0),
    mOldFormat(-1),
    mOldDataSpace(HAL_DATASPACE_UNKNOWN),
    mPrepared(false),
    mPrepareBlockRequest(true),
    mPreparedBufferIdx(0),
    mLastMaxCount(Camera3StreamInterface::ALLOCATE_PIPELINE_MAX),
    mBufferLimitLatency(kBufferLimitLatencyBinSize),
    mFormatOverridden(false),
    mOriginalFormat(format),
    mDataSpaceOverridden(false),
    mOriginalDataSpace(HAL_DATASPACE_UNKNOWN),
    mPhysicalCameraId(physicalCameraId),
    mLastTimestamp(0) {

    camera3_stream::stream_type = type;
    camera3_stream::width = width;
    camera3_stream::height = height;
    camera3_stream::format = format;
    camera3_stream::data_space = dataSpace;
    camera3_stream::rotation = rotation;
    camera3_stream::max_buffers = 0;
    camera3_stream::priv = NULL;
    camera3_stream::physical_camera_id = mPhysicalCameraId.string();

    if ((format == HAL_PIXEL_FORMAT_BLOB || format == HAL_PIXEL_FORMAT_RAW_OPAQUE) &&
            maxSize == 0) {
        ALOGE("%s: BLOB or RAW_OPAQUE format with size == 0", __FUNCTION__);
        mState = STATE_ERROR;
    }
}

int Camera3Stream::getId() const {
    return mId;
}

int Camera3Stream::getStreamSetId() const {
    return mSetId;
}

uint32_t Camera3Stream::getWidth() const {
    return camera3_stream::width;
}

uint32_t Camera3Stream::getHeight() const {
    return camera3_stream::height;
}

int Camera3Stream::getFormat() const {
    return camera3_stream::format;
}

android_dataspace Camera3Stream::getDataSpace() const {
    return camera3_stream::data_space;
}

uint64_t Camera3Stream::getUsage() const {
    return mUsage;
}

void Camera3Stream::setUsage(uint64_t usage) {
    mUsage = usage;
}

void Camera3Stream::setFormatOverride(bool formatOverridden) {
    mFormatOverridden = formatOverridden;
}

bool Camera3Stream::isFormatOverridden() const {
    return mFormatOverridden;
}

int Camera3Stream::getOriginalFormat() const {
    return mOriginalFormat;
}

void Camera3Stream::setDataSpaceOverride(bool dataSpaceOverridden) {
    mDataSpaceOverridden = dataSpaceOverridden;
    if (dataSpaceOverridden && mOriginalDataSpace == HAL_DATASPACE_UNKNOWN) {
        mOriginalDataSpace = camera3_stream::data_space;
    }
}

bool Camera3Stream::isDataSpaceOverridden() const {
    return mDataSpaceOverridden;
}

android_dataspace Camera3Stream::getOriginalDataSpace() const {
    return mOriginalDataSpace;
}

const String8& Camera3Stream::physicalCameraId() const {
    return mPhysicalCameraId;
}

status_t Camera3Stream::forceToIdle() {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);
    status_t res;

    switch (mState) {
        case STATE_ERROR:
        case STATE_CONSTRUCTED:
        case STATE_IN_CONFIG:
        case STATE_PREPARING:
        case STATE_IN_RECONFIG:
            ALOGE("%s: Invalid state: %d", __FUNCTION__, mState);
            res = NO_INIT;
            break;
        case STATE_CONFIGURED:
            if (hasOutstandingBuffersLocked()) {
                sp<StatusTracker> statusTracker = mStatusTracker.promote();
                if (statusTracker != 0) {
                    statusTracker->markComponentIdle(mStatusId, Fence::NO_FENCE);
                }
            }

            mState = STATE_IN_IDLE;
            res = OK;

            break;
        default:
            ALOGE("%s: Unknown state %d", __FUNCTION__, mState);
            res = NO_INIT;
    }

    return res;
}

status_t Camera3Stream::restoreConfiguredState() {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);
    status_t res;

    switch (mState) {
        case STATE_ERROR:
        case STATE_CONSTRUCTED:
        case STATE_IN_CONFIG:
        case STATE_PREPARING:
        case STATE_IN_RECONFIG:
        case STATE_CONFIGURED:
            ALOGE("%s: Invalid state: %d", __FUNCTION__, mState);
            res = NO_INIT;
            break;
        case STATE_IN_IDLE:
            if (hasOutstandingBuffersLocked()) {
                sp<StatusTracker> statusTracker = mStatusTracker.promote();
                if (statusTracker != 0) {
                    statusTracker->markComponentActive(mStatusId);
                }
            }

            mState = STATE_CONFIGURED;
            res = OK;

            break;
        default:
            ALOGE("%s: Unknown state %d", __FUNCTION__, mState);
            res = NO_INIT;
    }

    return res;
}

camera3_stream* Camera3Stream::startConfiguration() {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);
    status_t res;

    switch (mState) {
        case STATE_ERROR:
            ALOGE("%s: In error state", __FUNCTION__);
            return NULL;
        case STATE_CONSTRUCTED:
        case STATE_IN_IDLE:
            // OK
            break;
        case STATE_IN_CONFIG:
        case STATE_IN_RECONFIG:
            // Can start config again with no trouble; but don't redo
            // mOldUsage/mOldMaxBuffers
            return this;
        case STATE_CONFIGURED:
            if (hasOutstandingBuffersLocked()) {
                ALOGE("%s: Cannot configure stream; has outstanding buffers",
                        __FUNCTION__);
                return NULL;
            }
            break;
        default:
            ALOGE("%s: Unknown state %d", __FUNCTION__, mState);
            return NULL;
    }

    mOldUsage = mUsage;
    mOldMaxBuffers = camera3_stream::max_buffers;
    mOldFormat = camera3_stream::format;
    mOldDataSpace = camera3_stream::data_space;

    res = getEndpointUsage(&mUsage);
    if (res != OK) {
        ALOGE("%s: Cannot query consumer endpoint usage!",
                __FUNCTION__);
        return NULL;
    }

    if (mState == STATE_IN_IDLE) {
        // Skip configuration.
        return this;
    }

    // Stop tracking if currently doing so
    if (mStatusId != StatusTracker::NO_STATUS_ID) {
        sp<StatusTracker> statusTracker = mStatusTracker.promote();
        if (statusTracker != 0) {
            statusTracker->removeComponent(mStatusId);
        }
        mStatusId = StatusTracker::NO_STATUS_ID;
    }

    if (mState == STATE_CONSTRUCTED) {
        mState = STATE_IN_CONFIG;
    } else { // mState == STATE_CONFIGURED
        LOG_ALWAYS_FATAL_IF(mState != STATE_CONFIGURED, "Invalid state: 0x%x", mState);
        mState = STATE_IN_RECONFIG;
    }

    return this;
}

bool Camera3Stream::isConfiguring() const {
    Mutex::Autolock l(mLock);
    return (mState == STATE_IN_CONFIG) || (mState == STATE_IN_RECONFIG);
}

status_t Camera3Stream::finishConfiguration(/*out*/bool* streamReconfigured) {
    ATRACE_CALL();
    if (streamReconfigured != nullptr) {
        *streamReconfigured = false;
    }
    Mutex::Autolock l(mLock);
    switch (mState) {
        case STATE_ERROR:
            ALOGE("%s: In error state", __FUNCTION__);
            return INVALID_OPERATION;
        case STATE_IN_CONFIG:
        case STATE_IN_RECONFIG:
            // OK
            break;
        case STATE_CONSTRUCTED:
        case STATE_CONFIGURED:
            ALOGE("%s: Cannot finish configuration that hasn't been started",
                    __FUNCTION__);
            return INVALID_OPERATION;
        case STATE_IN_IDLE:
            //Skip configuration in this state
            return OK;
        default:
            ALOGE("%s: Unknown state", __FUNCTION__);
            return INVALID_OPERATION;
    }

    // Register for idle tracking
    sp<StatusTracker> statusTracker = mStatusTracker.promote();
    if (statusTracker != 0 && mStatusId == StatusTracker::NO_STATUS_ID) {
        mStatusId = statusTracker->addComponent();
    }

    // Check if the stream configuration is unchanged, and skip reallocation if
    // so. As documented in hardware/camera3.h:configure_streams().
    if (mState == STATE_IN_RECONFIG &&
            mOldUsage == mUsage &&
            mOldMaxBuffers == camera3_stream::max_buffers &&
            mOldDataSpace == camera3_stream::data_space &&
            mOldFormat == camera3_stream::format) {
        mState = STATE_CONFIGURED;
        return OK;
    }

    // Reset prepared state, since buffer config has changed, and existing
    // allocations are no longer valid
    mPrepared = false;
    mPrepareBlockRequest = true;
    mStreamUnpreparable = false;

    bool reconfiguring = (mState == STATE_IN_RECONFIG);
    status_t res;
    res = configureQueueLocked();
    // configureQueueLocked could return error in case of abandoned surface.
    // Treat as non-fatal error.
    if (res == NO_INIT || res == DEAD_OBJECT) {
        ALOGE("%s: Unable to configure stream %d queue (non-fatal): %s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        mState = STATE_ABANDONED;
        return res;
    } else if (res != OK) {
        ALOGE("%s: Unable to configure stream %d queue: %s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        mState = STATE_ERROR;
        return res;
    }

    if (reconfiguring && streamReconfigured != nullptr) {
        *streamReconfigured = true;
    }
    mState = STATE_CONFIGURED;

    return res;
}

status_t Camera3Stream::cancelConfiguration() {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);
    switch (mState) {
        case STATE_ERROR:
            ALOGE("%s: In error state", __FUNCTION__);
            return INVALID_OPERATION;
        case STATE_IN_CONFIG:
        case STATE_IN_RECONFIG:
        case STATE_IN_IDLE:
            // OK
            break;
        case STATE_CONSTRUCTED:
        case STATE_CONFIGURED:
            ALOGE("%s: Cannot cancel configuration that hasn't been started",
                    __FUNCTION__);
            return INVALID_OPERATION;
        default:
            ALOGE("%s: Unknown state", __FUNCTION__);
            return INVALID_OPERATION;
    }

    mUsage = mOldUsage;
    camera3_stream::max_buffers = mOldMaxBuffers;

    mState = ((mState == STATE_IN_RECONFIG) || (mState == STATE_IN_IDLE)) ? STATE_CONFIGURED :
            STATE_CONSTRUCTED;

    return OK;
}

bool Camera3Stream::isUnpreparable() {
    ATRACE_CALL();

    Mutex::Autolock l(mLock);
    return mStreamUnpreparable;
}

status_t Camera3Stream::startPrepare(int maxCount, bool blockRequest) {
    ATRACE_CALL();

    Mutex::Autolock l(mLock);

    if (maxCount < 0) {
        ALOGE("%s: Stream %d: Can't prepare stream if max buffer count (%d) is < 0",
                __FUNCTION__, mId, maxCount);
        return BAD_VALUE;
    }

    // This function should be only called when the stream is configured already.
    if (mState != STATE_CONFIGURED) {
        ALOGE("%s: Stream %d: Can't prepare stream if stream is not in CONFIGURED "
                "state %d", __FUNCTION__, mId, mState);
        return INVALID_OPERATION;
    }

    // This function can't be called if the stream has already received filled
    // buffers
    if (mStreamUnpreparable) {
        ALOGE("%s: Stream %d: Can't prepare stream that's already in use",
                __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    if (getHandoutOutputBufferCountLocked() > 0) {
        ALOGE("%s: Stream %d: Can't prepare stream that has outstanding buffers",
                __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    size_t pipelineMax = getBufferCountLocked();
    size_t clampedCount = (pipelineMax < static_cast<size_t>(maxCount)) ?
            pipelineMax : static_cast<size_t>(maxCount);
    size_t bufferCount = (maxCount == Camera3StreamInterface::ALLOCATE_PIPELINE_MAX) ?
            pipelineMax : clampedCount;

    mPrepared = bufferCount <= mLastMaxCount;
    mPrepareBlockRequest = blockRequest;

    if (mPrepared) return OK;

    mLastMaxCount = bufferCount;

    mPreparedBuffers.insertAt(camera3_stream_buffer_t(), /*index*/0, bufferCount);
    mPreparedBufferIdx = 0;

    mState = STATE_PREPARING;

    return NOT_ENOUGH_DATA;
}

bool Camera3Stream::isBlockedByPrepare() const {
    Mutex::Autolock l(mLock);
    return mState == STATE_PREPARING && mPrepareBlockRequest;
}

bool Camera3Stream::isAbandoned() const {
    Mutex::Autolock l(mLock);
    return mState == STATE_ABANDONED;
}

status_t Camera3Stream::prepareNextBuffer() {
    ATRACE_CALL();

    Mutex::Autolock l(mLock);
    status_t res = OK;

    // This function should be only called when the stream is preparing
    if (mState != STATE_PREPARING) {
        ALOGE("%s: Stream %d: Can't prepare buffer if stream is not in PREPARING "
                "state %d", __FUNCTION__, mId, mState);
        return INVALID_OPERATION;
    }

    // Get next buffer - this may allocate, and take a while for large buffers
    res = getBufferLocked( &mPreparedBuffers.editItemAt(mPreparedBufferIdx) );
    if (res != OK) {
        ALOGE("%s: Stream %d: Unable to allocate buffer %zu during preparation",
                __FUNCTION__, mId, mPreparedBufferIdx);
        return NO_INIT;
    }

    mPreparedBufferIdx++;

    // Check if we still have buffers left to allocate
    if (mPreparedBufferIdx < mPreparedBuffers.size()) {
        return NOT_ENOUGH_DATA;
    }

    // Done with prepare - mark stream as such, and return all buffers
    // via cancelPrepare
    mPrepared = true;

    return cancelPrepareLocked();
}

status_t Camera3Stream::cancelPrepare() {
    ATRACE_CALL();

    Mutex::Autolock l(mLock);

    return cancelPrepareLocked();
}

status_t Camera3Stream::cancelPrepareLocked() {
    status_t res = OK;

    // This function should be only called when the stream is mid-preparing.
    if (mState != STATE_PREPARING) {
        ALOGE("%s: Stream %d: Can't cancel prepare stream if stream is not in "
                "PREPARING state %d", __FUNCTION__, mId, mState);
        return INVALID_OPERATION;
    }

    // Return all valid buffers to stream, in ERROR state to indicate
    // they weren't filled.
    for (size_t i = 0; i < mPreparedBufferIdx; i++) {
        mPreparedBuffers.editItemAt(i).release_fence = -1;
        mPreparedBuffers.editItemAt(i).status = CAMERA3_BUFFER_STATUS_ERROR;
        returnBufferLocked(mPreparedBuffers[i], 0);
    }
    mPreparedBuffers.clear();
    mPreparedBufferIdx = 0;

    mState = STATE_CONFIGURED;

    return res;
}

status_t Camera3Stream::tearDown() {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);

    status_t res = OK;

    // This function should be only called when the stream is configured.
    if (mState != STATE_CONFIGURED) {
        ALOGE("%s: Stream %d: Can't tear down stream if stream is not in "
                "CONFIGURED state %d", __FUNCTION__, mId, mState);
        return INVALID_OPERATION;
    }

    // If any buffers have been handed to the HAL, the stream cannot be torn down.
    if (getHandoutOutputBufferCountLocked() > 0) {
        ALOGE("%s: Stream %d: Can't tear down a stream that has outstanding buffers",
                __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    // Free buffers by disconnecting and then reconnecting to the buffer queue
    // Only unused buffers will be dropped immediately; buffers that have been filled
    // and are waiting to be acquired by the consumer and buffers that are currently
    // acquired will be freed once they are released by the consumer.

    res = disconnectLocked();
    if (res != OK) {
        if (res == -ENOTCONN) {
            // queue has been disconnected, nothing left to do, so exit with success
            return OK;
        }
        ALOGE("%s: Stream %d: Unable to disconnect to tear down buffers: %s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        return res;
    }

    mState = STATE_IN_CONFIG;

    res = configureQueueLocked();
    if (res != OK) {
        ALOGE("%s: Unable to configure stream %d queue: %s (%d)",
                __FUNCTION__, mId, strerror(-res), res);
        mState = STATE_ERROR;
        return res;
    }

    // Reset prepared state, since we've reconnected to the queue and can prepare again.
    mPrepared = false;
    mStreamUnpreparable = false;

    mState = STATE_CONFIGURED;

    return OK;
}

status_t Camera3Stream::getBuffer(camera3_stream_buffer *buffer,
        nsecs_t waitBufferTimeout,
        const std::vector<size_t>& surface_ids) {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);
    status_t res = OK;

    // This function should be only called when the stream is configured already.
    if (mState != STATE_CONFIGURED) {
        ALOGE("%s: Stream %d: Can't get buffers if stream is not in CONFIGURED state %d",
                __FUNCTION__, mId, mState);
        if (mState == STATE_ABANDONED) {
            return DEAD_OBJECT;
        } else {
            return INVALID_OPERATION;
        }
    }

    // Wait for new buffer returned back if we are running into the limit.
    if (getHandoutOutputBufferCountLocked() == camera3_stream::max_buffers) {
        ALOGV("%s: Already dequeued max output buffers (%d), wait for next returned one.",
                        __FUNCTION__, camera3_stream::max_buffers);
        nsecs_t waitStart = systemTime(SYSTEM_TIME_MONOTONIC);
        if (waitBufferTimeout < kWaitForBufferDuration) {
            waitBufferTimeout = kWaitForBufferDuration;
        }
        res = mOutputBufferReturnedSignal.waitRelative(mLock, waitBufferTimeout);
        nsecs_t waitEnd = systemTime(SYSTEM_TIME_MONOTONIC);
        mBufferLimitLatency.add(waitStart, waitEnd);
        if (res != OK) {
            if (res == TIMED_OUT) {
                ALOGE("%s: wait for output buffer return timed out after %lldms (max_buffers %d)",
                        __FUNCTION__, waitBufferTimeout / 1000000LL,
                        camera3_stream::max_buffers);
            }
            return res;
        }
    }

    res = getBufferLocked(buffer, surface_ids);
    if (res == OK) {
        fireBufferListenersLocked(*buffer, /*acquired*/true, /*output*/true);
        if (buffer->buffer) {
            Mutex::Autolock l(mOutstandingBuffersLock);
            mOutstandingBuffers.push_back(*buffer->buffer);
        }
    }

    return res;
}

bool Camera3Stream::isOutstandingBuffer(const camera3_stream_buffer &buffer) const{
    if (buffer.buffer == nullptr) {
        return false;
    }

    Mutex::Autolock l(mOutstandingBuffersLock);

    for (auto b : mOutstandingBuffers) {
        if (b == *buffer.buffer) {
            return true;
        }
    }
    return false;
}

void Camera3Stream::removeOutstandingBuffer(const camera3_stream_buffer &buffer) {
    if (buffer.buffer == nullptr) {
        return;
    }

    Mutex::Autolock l(mOutstandingBuffersLock);

    for (auto b = mOutstandingBuffers.begin(); b != mOutstandingBuffers.end(); b++) {
        if (*b == *buffer.buffer) {
            mOutstandingBuffers.erase(b);
            return;
        }
    }
}

status_t Camera3Stream::returnBuffer(const camera3_stream_buffer &buffer,
        nsecs_t timestamp, bool timestampIncreasing,
         const std::vector<size_t>& surface_ids, uint64_t frameNumber) {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);

    // Check if this buffer is outstanding.
    if (!isOutstandingBuffer(buffer)) {
        ALOGE("%s: Stream %d: Returning an unknown buffer.", __FUNCTION__, mId);
        return BAD_VALUE;
    }

    removeOutstandingBuffer(buffer);

    // Buffer status may be changed, so make a copy of the stream_buffer struct.
    camera3_stream_buffer b = buffer;
    if (timestampIncreasing && timestamp != 0 && timestamp <= mLastTimestamp) {
        ALOGE("%s: Stream %d: timestamp %" PRId64 " is not increasing. Prev timestamp %" PRId64,
                __FUNCTION__, mId, timestamp, mLastTimestamp);
        b.status = CAMERA3_BUFFER_STATUS_ERROR;
    }
    mLastTimestamp = timestamp;

    /**
     * TODO: Check that the state is valid first.
     *
     * <HAL3.2 IN_CONFIG and IN_RECONFIG in addition to CONFIGURED.
     * >= HAL3.2 CONFIGURED only
     *
     * Do this for getBuffer as well.
     */
    status_t res = returnBufferLocked(b, timestamp, surface_ids);
    if (res == OK) {
        fireBufferListenersLocked(b, /*acquired*/false, /*output*/true, timestamp, frameNumber);
    }

    // Even if returning the buffer failed, we still want to signal whoever is waiting for the
    // buffer to be returned.
    mOutputBufferReturnedSignal.signal();

    return res;
}

status_t Camera3Stream::getInputBuffer(camera3_stream_buffer *buffer, bool respectHalLimit) {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);
    status_t res = OK;

    // This function should be only called when the stream is configured already.
    if (mState != STATE_CONFIGURED) {
        ALOGE("%s: Stream %d: Can't get input buffers if stream is not in CONFIGURED state %d",
                __FUNCTION__, mId, mState);
        return INVALID_OPERATION;
    }

    // Wait for new buffer returned back if we are running into the limit.
    if (getHandoutInputBufferCountLocked() == camera3_stream::max_buffers && respectHalLimit) {
        ALOGV("%s: Already dequeued max input buffers (%d), wait for next returned one.",
                __FUNCTION__, camera3_stream::max_buffers);
        res = mInputBufferReturnedSignal.waitRelative(mLock, kWaitForBufferDuration);
        if (res != OK) {
            if (res == TIMED_OUT) {
                ALOGE("%s: wait for input buffer return timed out after %lldms", __FUNCTION__,
                        kWaitForBufferDuration / 1000000LL);
            }
            return res;
        }
    }

    res = getInputBufferLocked(buffer);
    if (res == OK) {
        fireBufferListenersLocked(*buffer, /*acquired*/true, /*output*/false);
        if (buffer->buffer) {
            Mutex::Autolock l(mOutstandingBuffersLock);
            mOutstandingBuffers.push_back(*buffer->buffer);
        }
    }

    return res;
}

status_t Camera3Stream::returnInputBuffer(const camera3_stream_buffer &buffer) {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);

    // Check if this buffer is outstanding.
    if (!isOutstandingBuffer(buffer)) {
        ALOGE("%s: Stream %d: Returning an unknown buffer.", __FUNCTION__, mId);
        return BAD_VALUE;
    }

    removeOutstandingBuffer(buffer);

    status_t res = returnInputBufferLocked(buffer);
    if (res == OK) {
        fireBufferListenersLocked(buffer, /*acquired*/false, /*output*/false);
        mInputBufferReturnedSignal.signal();
    }

    return res;
}

status_t Camera3Stream::getInputBufferProducer(sp<IGraphicBufferProducer> *producer) {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);

    return getInputBufferProducerLocked(producer);
}

void Camera3Stream::fireBufferRequestForFrameNumber(uint64_t frameNumber,
        const CameraMetadata& settings) {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);

    for (auto &it : mBufferListenerList) {
        sp<Camera3StreamBufferListener> listener = it.promote();
        if (listener.get() != nullptr) {
            listener->onBufferRequestForFrameNumber(frameNumber, getId(), settings);
        }
    }
}

void Camera3Stream::fireBufferListenersLocked(
        const camera3_stream_buffer& buffer, bool acquired, bool output, nsecs_t timestamp,
        uint64_t frameNumber) {
    List<wp<Camera3StreamBufferListener> >::iterator it, end;

    // TODO: finish implementing

    Camera3StreamBufferListener::BufferInfo info =
        Camera3StreamBufferListener::BufferInfo();
    info.mOutput = output;
    info.mError = (buffer.status == CAMERA3_BUFFER_STATUS_ERROR);
    info.mFrameNumber = frameNumber;
    info.mTimestamp = timestamp;
    // TODO: rest of fields

    for (it = mBufferListenerList.begin(), end = mBufferListenerList.end();
         it != end;
         ++it) {

        sp<Camera3StreamBufferListener> listener = it->promote();
        if (listener != 0) {
            if (acquired) {
                listener->onBufferAcquired(info);
            } else {
                listener->onBufferReleased(info);
            }
        }
    }
}

bool Camera3Stream::hasOutstandingBuffers() const {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);
    return hasOutstandingBuffersLocked();
}

size_t Camera3Stream::getOutstandingBuffersCount() const {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);
    return getHandoutOutputBufferCountLocked();
}

status_t Camera3Stream::setStatusTracker(sp<StatusTracker> statusTracker) {
    Mutex::Autolock l(mLock);
    sp<StatusTracker> oldTracker = mStatusTracker.promote();
    if (oldTracker != 0 && mStatusId != StatusTracker::NO_STATUS_ID) {
        oldTracker->removeComponent(mStatusId);
    }
    mStatusId = StatusTracker::NO_STATUS_ID;
    mStatusTracker = statusTracker;

    return OK;
}

status_t Camera3Stream::disconnect() {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);
    ALOGV("%s: Stream %d: Disconnecting...", __FUNCTION__, mId);
    status_t res = disconnectLocked();

    mBufferLimitLatency.log("Stream %d latency histogram for wait on max_buffers", mId);
    mBufferLimitLatency.reset();

    if (res == -ENOTCONN) {
        // "Already disconnected" -- not an error
        return OK;
    } else {
        return res;
    }
}

void Camera3Stream::dump(int fd, const Vector<String16> &args) const
{
    (void)args;
    mBufferLimitLatency.dump(fd,
            "      Latency histogram for wait on max_buffers");
}

status_t Camera3Stream::getBufferLocked(camera3_stream_buffer *,
        const std::vector<size_t>&) {
    ALOGE("%s: This type of stream does not support output", __FUNCTION__);
    return INVALID_OPERATION;
}
status_t Camera3Stream::returnBufferLocked(const camera3_stream_buffer &,
                                           nsecs_t, const std::vector<size_t>&) {
    ALOGE("%s: This type of stream does not support output", __FUNCTION__);
    return INVALID_OPERATION;
}
status_t Camera3Stream::getInputBufferLocked(camera3_stream_buffer *) {
    ALOGE("%s: This type of stream does not support input", __FUNCTION__);
    return INVALID_OPERATION;
}
status_t Camera3Stream::returnInputBufferLocked(
        const camera3_stream_buffer &) {
    ALOGE("%s: This type of stream does not support input", __FUNCTION__);
    return INVALID_OPERATION;
}
status_t Camera3Stream::getInputBufferProducerLocked(sp<IGraphicBufferProducer>*) {
    ALOGE("%s: This type of stream does not support input", __FUNCTION__);
    return INVALID_OPERATION;
}

void Camera3Stream::addBufferListener(
        wp<Camera3StreamBufferListener> listener) {
    Mutex::Autolock l(mLock);

    List<wp<Camera3StreamBufferListener> >::iterator it, end;
    for (it = mBufferListenerList.begin(), end = mBufferListenerList.end();
         it != end;
         ) {
        if (*it == listener) {
            ALOGE("%s: Try to add the same listener twice, ignoring...", __FUNCTION__);
            return;
        }
        it++;
    }

    mBufferListenerList.push_back(listener);
}

void Camera3Stream::removeBufferListener(
        const sp<Camera3StreamBufferListener>& listener) {
    Mutex::Autolock l(mLock);

    bool erased = true;
    List<wp<Camera3StreamBufferListener> >::iterator it, end;
    for (it = mBufferListenerList.begin(), end = mBufferListenerList.end();
         it != end;
         ) {

        if (*it == listener) {
            it = mBufferListenerList.erase(it);
            erased = true;
        } else {
            ++it;
        }
    }

    if (!erased) {
        ALOGW("%s: Could not find listener to remove, already removed",
              __FUNCTION__);
    }
}

void Camera3Stream::setBufferFreedListener(
        wp<Camera3StreamBufferFreedListener> listener) {
    Mutex::Autolock l(mLock);
    // Only allow set listener during stream configuration because stream is guaranteed to be IDLE
    // at this state, so setBufferFreedListener won't collide with onBufferFreed callbacks
    if (mState != STATE_IN_CONFIG && mState != STATE_IN_RECONFIG) {
        ALOGE("%s: listener must be set during stream configuration!",__FUNCTION__);
        return;
    }
    mBufferFreedListener = listener;
}

}; // namespace camera3

}; // namespace android
