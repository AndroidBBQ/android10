/*
 * Copyright 2016 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "Camera3-BufferManager"
#define ATRACE_TAG ATRACE_TAG_CAMERA

#include <gui/ISurfaceComposer.h>
#include <private/gui/ComposerService.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include "utils/CameraTraces.h"
#include "Camera3BufferManager.h"

namespace android {

namespace camera3 {

Camera3BufferManager::Camera3BufferManager() {
}

Camera3BufferManager::~Camera3BufferManager() {
}

status_t Camera3BufferManager::registerStream(wp<Camera3OutputStream>& stream,
        const StreamInfo& streamInfo) {
    ATRACE_CALL();

    int streamId = streamInfo.streamId;
    int streamSetId = streamInfo.streamSetId;

    if (streamId == CAMERA3_STREAM_ID_INVALID || streamSetId == CAMERA3_STREAM_SET_ID_INVALID) {
        ALOGE("%s: Stream id (%d) or stream set id (%d) is invalid",
                __FUNCTION__, streamId, streamSetId);
        return BAD_VALUE;
    }
    if (streamInfo.totalBufferCount > kMaxBufferCount || streamInfo.totalBufferCount == 0) {
        ALOGE("%s: Stream id (%d) with stream set id (%d) total buffer count %zu is invalid",
                __FUNCTION__, streamId, streamSetId, streamInfo.totalBufferCount);
        return BAD_VALUE;
    }
    if (!streamInfo.isConfigured) {
        ALOGE("%s: Stream (%d) is not configured", __FUNCTION__, streamId);
        return BAD_VALUE;
    }

    // For Gralloc v1, try to allocate a buffer and see if it is successful, otherwise, stream
    // buffer sharing for this newly added stream is not supported. For Gralloc v0, we don't
    // need check this, as the buffers are not really shared between streams, the buffers are
    // allocated for each stream individually, the allocation failure will be checked in
    // getBufferForStream() call.
    if (mGrallocVersion > HARDWARE_DEVICE_API_VERSION(0,1)) {
        // TODO: To be implemented.

        // In case allocation fails, return invalid operation
        return INVALID_OPERATION;
    }

    Mutex::Autolock l(mLock);

    // Check if this stream was registered with different stream set ID, if so, error out.
    for (size_t i = 0; i < mStreamSetMap.size(); i++) {
        ssize_t streamIdx = mStreamSetMap[i].streamInfoMap.indexOfKey(streamId);
        if (streamIdx != NAME_NOT_FOUND &&
            mStreamSetMap[i].streamInfoMap[streamIdx].streamSetId != streamInfo.streamSetId) {
            ALOGE("%s: It is illegal to register the same stream id with different stream set",
                    __FUNCTION__);
            return BAD_VALUE;
        }
    }
    // Check if there is an existing stream set registered; if not, create one; otherwise, add this
    // stream info to the existing stream set entry.
    ssize_t setIdx = mStreamSetMap.indexOfKey(streamSetId);
    if (setIdx == NAME_NOT_FOUND) {
        ALOGV("%s: stream set %d is not registered to stream set map yet, create it.",
                __FUNCTION__, streamSetId);
        // Create stream info map, then add to mStreamsetMap.
        StreamSet newStreamSet;
        setIdx = mStreamSetMap.add(streamSetId, newStreamSet);
    }
    // Update stream set map and water mark.
    StreamSet& currentStreamSet = mStreamSetMap.editValueAt(setIdx);
    ssize_t streamIdx = currentStreamSet.streamInfoMap.indexOfKey(streamId);
    if (streamIdx != NAME_NOT_FOUND) {
        ALOGW("%s: stream %d was already registered with stream set %d",
                __FUNCTION__, streamId, streamSetId);
        return OK;
    }
    currentStreamSet.streamInfoMap.add(streamId, streamInfo);
    currentStreamSet.handoutBufferCountMap.add(streamId, 0);
    currentStreamSet.attachedBufferCountMap.add(streamId, 0);
    mStreamMap.add(streamId, stream);

    // The max allowed buffer count should be the max of buffer count of each stream inside a stream
    // set.
    if (streamInfo.totalBufferCount > currentStreamSet.maxAllowedBufferCount) {
       currentStreamSet.maxAllowedBufferCount = streamInfo.totalBufferCount;
    }

    return OK;
}

status_t Camera3BufferManager::unregisterStream(int streamId, int streamSetId) {
    ATRACE_CALL();

    Mutex::Autolock l(mLock);
    ALOGV("%s: unregister stream %d with stream set %d", __FUNCTION__,
            streamId, streamSetId);

    if (!checkIfStreamRegisteredLocked(streamId, streamSetId)){
        ALOGE("%s: stream %d with set id %d wasn't properly registered to this buffer manager!",
                __FUNCTION__, streamId, streamSetId);
        return BAD_VALUE;
    }

    // De-list all the buffers associated with this stream first.
    StreamSet& currentSet = mStreamSetMap.editValueFor(streamSetId);
    BufferCountMap& handOutBufferCounts = currentSet.handoutBufferCountMap;
    BufferCountMap& attachedBufferCounts = currentSet.attachedBufferCountMap;
    InfoMap& infoMap = currentSet.streamInfoMap;
    handOutBufferCounts.removeItem(streamId);
    attachedBufferCounts.removeItem(streamId);

    // Remove the stream info from info map and recalculate the buffer count water mark.
    infoMap.removeItem(streamId);
    currentSet.maxAllowedBufferCount = 0;
    for (size_t i = 0; i < infoMap.size(); i++) {
        if (infoMap[i].totalBufferCount > currentSet.maxAllowedBufferCount) {
            currentSet.maxAllowedBufferCount = infoMap[i].totalBufferCount;
        }
    }
    mStreamMap.removeItem(streamId);

    // Lazy solution: when a stream is unregistered, the streams will be reconfigured, reset
    // the water mark and let it grow again.
    currentSet.allocatedBufferWaterMark = 0;

    // Remove this stream set if all its streams have been removed.
    if (handOutBufferCounts.size() == 0 && infoMap.size() == 0) {
        mStreamSetMap.removeItem(streamSetId);
    }

    return OK;
}

void Camera3BufferManager::notifyBufferRemoved(int streamId, int streamSetId) {
    Mutex::Autolock l(mLock);
    StreamSet &streamSet = mStreamSetMap.editValueFor(streamSetId);
    size_t& attachedBufferCount =
            streamSet.attachedBufferCountMap.editValueFor(streamId);
    attachedBufferCount--;
}

status_t Camera3BufferManager::checkAndFreeBufferOnOtherStreamsLocked(
        int streamId, int streamSetId) {
    StreamId firstOtherStreamId = CAMERA3_STREAM_ID_INVALID;
    StreamSet &streamSet = mStreamSetMap.editValueFor(streamSetId);
    if (streamSet.streamInfoMap.size() == 1) {
        ALOGV("StreamSet %d has no other stream available to free", streamSetId);
        return OK;
    }

    bool freeBufferIsAttached = false;
    for (size_t i = 0; i < streamSet.streamInfoMap.size(); i++) {
        firstOtherStreamId = streamSet.streamInfoMap[i].streamId;
        if (firstOtherStreamId != streamId) {

            size_t otherBufferCount  =
                    streamSet.handoutBufferCountMap.valueFor(firstOtherStreamId);
            size_t otherAttachedBufferCount =
                    streamSet.attachedBufferCountMap.valueFor(firstOtherStreamId);
            if (otherAttachedBufferCount > otherBufferCount) {
                freeBufferIsAttached = true;
                break;
            }
        }
        firstOtherStreamId = CAMERA3_STREAM_ID_INVALID;
    }
    if (firstOtherStreamId == CAMERA3_STREAM_ID_INVALID || !freeBufferIsAttached) {
        ALOGV("StreamSet %d has no buffer available to free", streamSetId);
        return OK;
    }


    // This will drop the reference to one free buffer, which will effectively free one
    // buffer (from the free buffer list) for the inactive streams.
    size_t totalAllocatedBufferCount = 0;
    for (size_t i = 0; i < streamSet.attachedBufferCountMap.size(); i++) {
        totalAllocatedBufferCount += streamSet.attachedBufferCountMap[i];
    }
    if (totalAllocatedBufferCount > streamSet.allocatedBufferWaterMark) {
        ALOGV("Stream %d: Freeing buffer: detach", firstOtherStreamId);
        sp<Camera3OutputStream> stream =
                mStreamMap.valueFor(firstOtherStreamId).promote();
        if (stream == nullptr) {
            ALOGE("%s: unable to promote stream %d to detach buffer", __FUNCTION__,
                    firstOtherStreamId);
            return INVALID_OPERATION;
        }

        // Detach and then drop the buffer.
        //
        // Need to unlock because the stream may also be calling
        // into the buffer manager in parallel to signal buffer
        // release, or acquire a new buffer.
        bool bufferFreed = false;
        {
            mLock.unlock();
            sp<GraphicBuffer> buffer;
            stream->detachBuffer(&buffer, /*fenceFd*/ nullptr);
            mLock.lock();
            if (buffer.get() != nullptr) {
                bufferFreed = true;
            }
        }
        if (bufferFreed) {
            size_t& otherAttachedBufferCount =
                    streamSet.attachedBufferCountMap.editValueFor(firstOtherStreamId);
            otherAttachedBufferCount--;
        }
    }

    return OK;
}

status_t Camera3BufferManager::getBufferForStream(int streamId, int streamSetId,
        sp<GraphicBuffer>* gb, int* fenceFd, bool noFreeBufferAtConsumer) {
    ATRACE_CALL();

    Mutex::Autolock l(mLock);
    ALOGV("%s: get buffer for stream %d with stream set %d", __FUNCTION__,
            streamId, streamSetId);

    if (!checkIfStreamRegisteredLocked(streamId, streamSetId)) {
        ALOGE("%s: stream %d is not registered with stream set %d yet!!!",
                __FUNCTION__, streamId, streamSetId);
        return BAD_VALUE;
    }

    StreamSet &streamSet = mStreamSetMap.editValueFor(streamSetId);
    BufferCountMap& handOutBufferCounts = streamSet.handoutBufferCountMap;
    size_t& bufferCount = handOutBufferCounts.editValueFor(streamId);
    BufferCountMap& attachedBufferCounts = streamSet.attachedBufferCountMap;
    size_t& attachedBufferCount = attachedBufferCounts.editValueFor(streamId);

    if (noFreeBufferAtConsumer) {
        attachedBufferCount = bufferCount;
    }

    if (bufferCount >= streamSet.maxAllowedBufferCount) {
        ALOGE("%s: bufferCount (%zu) exceeds the max allowed buffer count (%zu) of this stream set",
                __FUNCTION__, bufferCount, streamSet.maxAllowedBufferCount);
        return INVALID_OPERATION;
    }

    if (attachedBufferCount > bufferCount) {
        // We've already attached more buffers to this stream than we currently have
        // outstanding, so have the stream just use an already-attached buffer
        bufferCount++;
        return ALREADY_EXISTS;
    }
    ALOGV("Stream %d set %d: Get buffer for stream: Allocate new", streamId, streamSetId);

    if (mGrallocVersion < HARDWARE_DEVICE_API_VERSION(1,0)) {
        const StreamInfo& info = streamSet.streamInfoMap.valueFor(streamId);
        GraphicBufferEntry buffer;
        buffer.fenceFd = -1;
        buffer.graphicBuffer = new GraphicBuffer(
                info.width, info.height, PixelFormat(info.format), info.combinedUsage,
                std::string("Camera3BufferManager pid [") +
                        std::to_string(getpid()) + "]");
        status_t res = buffer.graphicBuffer->initCheck();

        ALOGV("%s: allocating a new graphic buffer (%dx%d, format 0x%x) %p with handle %p",
                __FUNCTION__, info.width, info.height, info.format,
                buffer.graphicBuffer.get(), buffer.graphicBuffer->handle);
        if (res < 0) {
            ALOGE("%s: graphic buffer allocation failed: (error %d %s) ",
                    __FUNCTION__, res, strerror(-res));
            return res;
        }
        ALOGV("%s: allocation done", __FUNCTION__);

        // Increase the hand-out and attached buffer counts for tracking purposes.
        bufferCount++;
        attachedBufferCount++;
        // Update the water mark to be the max hand-out buffer count + 1. An additional buffer is
        // added to reduce the chance of buffer allocation during stream steady state, especially
        // for cases where one stream is active, the other stream may request some buffers randomly.
        if (bufferCount + 1 > streamSet.allocatedBufferWaterMark) {
            streamSet.allocatedBufferWaterMark = bufferCount + 1;
        }
        *gb = buffer.graphicBuffer;
        *fenceFd = buffer.fenceFd;
        ALOGV("%s: get buffer (%p) with handle (%p).",
                __FUNCTION__, buffer.graphicBuffer.get(), buffer.graphicBuffer->handle);

        // Proactively free buffers for other streams if the current number of allocated buffers
        // exceeds the water mark. This only for Gralloc V1, for V2, this logic can also be handled
        // in returnBufferForStream() if we want to free buffer more quickly.
        // TODO: probably should find out all the inactive stream IDs, and free the firstly found
        // buffers for them.
        res = checkAndFreeBufferOnOtherStreamsLocked(streamId, streamSetId);
        if (res != OK) {
            return res;
        }
        // Since we just allocated one new buffer above, try free one more buffer from other streams
        // to prevent total buffer count from growing
        res = checkAndFreeBufferOnOtherStreamsLocked(streamId, streamSetId);
        if (res != OK) {
            return res;
        }
    } else {
        // TODO: implement this.
        return BAD_VALUE;
    }

    return OK;
}

status_t Camera3BufferManager::onBufferReleased(
        int streamId, int streamSetId, bool* shouldFreeBuffer) {
    ATRACE_CALL();

    if (shouldFreeBuffer == nullptr) {
        ALOGE("%s: shouldFreeBuffer is null", __FUNCTION__);
        return BAD_VALUE;
    }

    Mutex::Autolock l(mLock);
    ALOGV("Stream %d set %d: Buffer released", streamId, streamSetId);
    *shouldFreeBuffer = false;

    if (!checkIfStreamRegisteredLocked(streamId, streamSetId)){
        ALOGV("%s: signaling buffer release for an already unregistered stream "
                "(stream %d with set id %d)", __FUNCTION__, streamId, streamSetId);
        return OK;
    }

    if (mGrallocVersion < HARDWARE_DEVICE_API_VERSION(1,0)) {
        StreamSet& streamSet = mStreamSetMap.editValueFor(streamSetId);
        BufferCountMap& handOutBufferCounts = streamSet.handoutBufferCountMap;
        size_t& bufferCount = handOutBufferCounts.editValueFor(streamId);
        bufferCount--;
        ALOGV("%s: Stream %d set %d: Buffer count now %zu", __FUNCTION__, streamId, streamSetId,
                bufferCount);

        size_t totalAllocatedBufferCount = 0;
        size_t totalHandOutBufferCount = 0;
        for (size_t i = 0; i < streamSet.attachedBufferCountMap.size(); i++) {
            totalAllocatedBufferCount += streamSet.attachedBufferCountMap[i];
            totalHandOutBufferCount += streamSet.handoutBufferCountMap[i];
        }

        size_t newWaterMark = totalHandOutBufferCount + BUFFER_WATERMARK_DEC_THRESHOLD;
        if (totalAllocatedBufferCount > newWaterMark &&
                    streamSet.allocatedBufferWaterMark > newWaterMark) {
            // BufferManager got more than enough buffers, so decrease watermark
            // to trigger more buffers free operation.
            streamSet.allocatedBufferWaterMark = newWaterMark;
            ALOGV("%s: Stream %d set %d: watermark--; now %zu",
                    __FUNCTION__, streamId, streamSetId, streamSet.allocatedBufferWaterMark);
        }

        size_t attachedBufferCount = streamSet.attachedBufferCountMap.valueFor(streamId);
        if (attachedBufferCount <= bufferCount) {
            ALOGV("%s: stream %d has no buffer available to free.", __FUNCTION__, streamId);
        }

        bool freeBufferIsAttached = (attachedBufferCount > bufferCount);
        if (freeBufferIsAttached &&
                totalAllocatedBufferCount > streamSet.allocatedBufferWaterMark &&
                attachedBufferCount > bufferCount + BUFFER_FREE_THRESHOLD) {
            ALOGV("%s: free a buffer from stream %d", __FUNCTION__, streamId);
            *shouldFreeBuffer = true;
        }
    } else {
        // TODO: implement gralloc V1 support
        return BAD_VALUE;
    }

    return OK;
}

status_t Camera3BufferManager::onBuffersRemoved(int streamId, int streamSetId, size_t count) {
    ATRACE_CALL();
    Mutex::Autolock l(mLock);

    ALOGV("Stream %d set %d: Buffer removed", streamId, streamSetId);

    if (!checkIfStreamRegisteredLocked(streamId, streamSetId)){
        ALOGV("%s: signaling buffer removal for an already unregistered stream "
                "(stream %d with set id %d)", __FUNCTION__, streamId, streamSetId);
        return OK;
    }

    if (mGrallocVersion < HARDWARE_DEVICE_API_VERSION(1,0)) {
        StreamSet& streamSet = mStreamSetMap.editValueFor(streamSetId);
        BufferCountMap& handOutBufferCounts = streamSet.handoutBufferCountMap;
        size_t& totalHandoutCount = handOutBufferCounts.editValueFor(streamId);
        BufferCountMap& attachedBufferCounts = streamSet.attachedBufferCountMap;
        size_t& totalAttachedCount = attachedBufferCounts.editValueFor(streamId);

        if (count > totalHandoutCount) {
            ALOGE("%s: Removed buffer count %zu greater than current handout count %zu",
                    __FUNCTION__, count, totalHandoutCount);
            return BAD_VALUE;
        }
        if (count > totalAttachedCount) {
            ALOGE("%s: Removed buffer count %zu greater than current attached count %zu",
                  __FUNCTION__, count, totalAttachedCount);
            return BAD_VALUE;
        }

        totalHandoutCount -= count;
        totalAttachedCount -= count;
        ALOGV("%s: Stream %d set %d: Buffer count now %zu, attached buffer count now %zu",
                __FUNCTION__, streamId, streamSetId, totalHandoutCount, totalAttachedCount);
    } else {
        // TODO: implement gralloc V1 support
        return BAD_VALUE;
    }

    return OK;
}

void Camera3BufferManager::dump(int fd, const Vector<String16>& args) const {
    Mutex::Autolock l(mLock);

    (void) args;
    String8 lines;
    lines.appendFormat("      Total stream sets: %zu\n", mStreamSetMap.size());
    for (size_t i = 0; i < mStreamSetMap.size(); i++) {
        lines.appendFormat("        Stream set %d has below streams:\n", mStreamSetMap.keyAt(i));
        for (size_t j = 0; j < mStreamSetMap[i].streamInfoMap.size(); j++) {
            lines.appendFormat("          Stream %d\n", mStreamSetMap[i].streamInfoMap[j].streamId);
        }
        lines.appendFormat("          Stream set max allowed buffer count: %zu\n",
                mStreamSetMap[i].maxAllowedBufferCount);
        lines.appendFormat("          Stream set buffer count water mark: %zu\n",
                mStreamSetMap[i].allocatedBufferWaterMark);
        lines.appendFormat("          Handout buffer counts:\n");
        for (size_t m = 0; m < mStreamSetMap[i].handoutBufferCountMap.size(); m++) {
            int streamId = mStreamSetMap[i].handoutBufferCountMap.keyAt(m);
            size_t bufferCount = mStreamSetMap[i].handoutBufferCountMap.valueAt(m);
            lines.appendFormat("            stream id: %d, buffer count: %zu.\n",
                    streamId, bufferCount);
        }
        lines.appendFormat("          Attached buffer counts:\n");
        for (size_t m = 0; m < mStreamSetMap[i].attachedBufferCountMap.size(); m++) {
            int streamId = mStreamSetMap[i].attachedBufferCountMap.keyAt(m);
            size_t bufferCount = mStreamSetMap[i].attachedBufferCountMap.valueAt(m);
            lines.appendFormat("            stream id: %d, attached buffer count: %zu.\n",
                    streamId, bufferCount);
        }
    }
    write(fd, lines.string(), lines.size());
}

bool Camera3BufferManager::checkIfStreamRegisteredLocked(int streamId, int streamSetId) const {
    ssize_t setIdx = mStreamSetMap.indexOfKey(streamSetId);
    if (setIdx == NAME_NOT_FOUND) {
        ALOGV("%s: stream set %d is not registered to stream set map yet!",
                __FUNCTION__, streamSetId);
        return false;
    }

    ssize_t streamIdx = mStreamSetMap.valueAt(setIdx).streamInfoMap.indexOfKey(streamId);
    if (streamIdx == NAME_NOT_FOUND) {
        ALOGV("%s: stream %d is not registered to stream info map yet!", __FUNCTION__, streamId);
        return false;
    }

    size_t bufferWaterMark = mStreamSetMap[setIdx].maxAllowedBufferCount;
    if (bufferWaterMark == 0 || bufferWaterMark > kMaxBufferCount) {
        ALOGW("%s: stream %d with stream set %d is not registered correctly to stream set map,"
                " as the water mark (%zu) is wrong!",
                __FUNCTION__, streamId, streamSetId, bufferWaterMark);
        return false;
    }

    return true;
}

} // namespace camera3
} // namespace android
