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

#ifndef ANDROID_SERVERS_CAMERA3_BUFFER_MANAGER_H
#define ANDROID_SERVERS_CAMERA3_BUFFER_MANAGER_H

#include <list>
#include <algorithm>
#include <ui/GraphicBuffer.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include "Camera3OutputStream.h"

namespace android {

namespace camera3 {

struct StreamInfo;
class Camera3OutputStream;

/**
 * A class managing the graphic buffers that is used by camera output streams. It allocates and
 * hands out Gralloc buffers to the clients (e.g., Camera3OutputStream) based on the requests.
 * When clients request a buffer, buffer manager will pick a buffer if there are some already
 * allocated buffer available, will allocate a buffer otherwise. When there are too many allocated
 * buffer maintained by the buffer manager, it will dynamically deallocate some buffers that are
 * solely owned by this buffer manager.
 * In doing so, it reduces the memory footprint unless it is already minimal without impacting
 * performance.
 *
 */
class Camera3BufferManager: public virtual RefBase {
public:
    explicit Camera3BufferManager();

    virtual ~Camera3BufferManager();

    /**
     * This method registers an output stream to this buffer manager by using the provided stream
     * information.
     *
     * The stream info includes the necessary information such as stream size, format, buffer count,
     * usage flags, etc. for the buffer manager to allocate and hand out buffers for this stream.
     *
     * It's illegal to call this method if the stream is not CONFIGURED yet, as some critical
     * stream properties (e.g., combined usage flags) are only available in this state. It is also
     * illegal to call this method with an invalid stream set ID (CAMERA3_STREAM_SET_ID_INVALID),
     * as the invalid stream set ID indicates that this stream doesn't intend to use buffer manager.
     *
     *
     * Once a stream is successfully registered to this buffer manager, the buffer manager takes
     * over the buffer allocation role and provides buffers to this stream via getBufferForStream().
     * The returned buffer can be sent to the camera HAL for image output, and then queued to the
     * ANativeWindow (Surface) for downstream consumer to acquire. Once the image buffer is released
     * by the consumer end point, the BufferQueueProducer callback onBufferReleased will call
     * returnBufferForStream() to return the free buffer to this buffer manager. If the stream
     * uses buffer manager to manage the stream buffers, it should disable the BufferQueue
     * allocation via IGraphicBufferProducer::allowAllocation(false).
     *
     * Registering an already registered stream has no effect.
     *
     * Return values:
     *
     *  OK:                Registration of the new stream was successful.
     *  BAD_VALUE:         This stream is not at CONFIGURED state, or the stream ID or stream set
     *                     ID are invalid, or attempting to register the same stream to multiple
     *                     stream sets, or other stream properties are invalid.
     *  INVALID_OPERATION: This buffer manager doesn't support buffer sharing across this stream
     *                     and other streams that were already registered with the same stream set
     *                     ID.
     */
    status_t registerStream(wp<Camera3OutputStream>& stream, const StreamInfo &streamInfo);

    /**
     * This method unregisters a stream from this buffer manager.
     *
     * After a stream is unregistered, further getBufferForStream() calls will fail for this stream.
     * After all streams for a given stream set are unregistered, all the buffers solely owned (for
     * this stream set) by this buffer manager will be freed; all buffers subsequently returned to
     * this buffer manager for this stream set will be freed immediately.
     *
     * Return values:
     *
     *  OK:        Removal of the a stream from this buffer manager was successful.
     *  BAD_VALUE: stream ID or stream set ID are invalid, or stream ID and stream set ID
     *             combination doesn't match what was registered, or this stream wasn't registered
     *             to this buffer manager before.
     */
    status_t unregisterStream(int streamId, int streamSetId);

    /**
     * This method obtains a buffer for a stream from this buffer manager.
     *
     * This method returns the first free buffer from the free buffer list (associated with this
     * stream set) if there is any. Otherwise, it will allocate a buffer for this stream, return
     * it and increment its count of handed-out buffers. When the total number of allocated buffers
     * is too high, it may deallocate the unused buffers to save memory footprint of this stream
     * set.
     *
     * After this call, the client takes over the ownership of this buffer if it is not freed.
     *
     * Sometimes free buffers are discarded from consumer side and the dequeueBuffer call returns
     * TIMED_OUT, in this case calling getBufferForStream again with noFreeBufferAtConsumer set to
     * true will notify buffer manager to update its states and also tries to allocate a new buffer.
     *
     * Return values:
     *
     *  OK:        Getting buffer for this stream was successful.
     *  ALREADY_EXISTS: Enough free buffers are already attached to this output buffer queue,
     *             user should just dequeue from the buffer queue.
     *  BAD_VALUE: stream ID or streamSetId are invalid, or stream ID and stream set ID
     *             combination doesn't match what was registered, or this stream wasn't registered
     *             to this buffer manager before.
     *  NO_MEMORY: Unable to allocate a buffer for this stream at this time.
     */
    status_t getBufferForStream(
            int streamId, int streamSetId, sp<GraphicBuffer>* gb, int* fenceFd,
            bool noFreeBufferAtConsumer = false);

    /**
     * This method notifies the manager that a buffer has been released by the consumer.
     *
     * The buffer is not returned to the buffer manager, but is available for the stream the buffer
     * is attached to for dequeuing.
     *
     * The notification lets the manager know how many buffers are directly available to the stream.
     *
     * If onBufferReleased is called for a given released buffer,
     * returnBufferForStream may not be called for the same buffer, until the
     * buffer has been reused. The manager will call detachBuffer on the stream
     * if it needs the released buffer otherwise.
     *
     * When shouldFreeBuffer is set to true, caller must detach and free one buffer from the
     * buffer queue, and then call notifyBufferRemoved to update the manager.
     *
     * Return values:
     *
     *  OK:        Buffer release was processed succesfully
     *  BAD_VALUE: stream ID or streamSetId are invalid, or stream ID and stream set ID
     *             combination doesn't match what was registered, or this stream wasn't registered
     *             to this buffer manager before, or shouldFreeBuffer is null/
     */
    status_t onBufferReleased(int streamId, int streamSetId, /*out*/bool* shouldFreeBuffer);

    /**
     * This method notifies the manager that certain buffers has been removed from the
     * buffer queue by detachBuffer from the consumer.
     *
     * The notification lets the manager update its internal handout buffer count and
     * attached buffer counts accordingly. When buffers are detached from
     * consumer, both handout and attached counts are decremented.
     *
     * Return values:
     *
     *  OK:        Buffer removal was processed succesfully
     *  BAD_VALUE: stream ID or streamSetId are invalid, or stream ID and stream set ID
     *             combination doesn't match what was registered, or this stream wasn't registered
     *             to this buffer manager before, or the removed buffer count is larger than
     *             current total handoutCount or attachedCount.
     */
    status_t onBuffersRemoved(int streamId, int streamSetId, size_t count);

    /**
     * This method notifiers the manager that a buffer is freed from the buffer queue, usually
     * because onBufferReleased signals the caller to free a buffer via the shouldFreeBuffer flag.
     */
    void notifyBufferRemoved(int streamId, int streamSetId);

    /**
     * Dump the buffer manager statistics.
     */
    void     dump(int fd, const Vector<String16> &args) const;

private:
    // allocatedBufferWaterMark will be decreased when:
    //   numAllocatedBuffersThisSet > numHandoutBuffersThisSet + BUFFER_WATERMARK_DEC_THRESHOLD
    // This allows the watermark go back down after a burst of buffer requests
    static const int BUFFER_WATERMARK_DEC_THRESHOLD = 3;

    // onBufferReleased will set shouldFreeBuffer to true when:
    //   numAllocatedBuffersThisSet > allocatedBufferWaterMark AND
    //   numAllocatedBuffersThisStream > numHandoutBuffersThisStream + BUFFER_FREE_THRESHOLD
    // So after a burst of buffer requests and back to steady state, the buffer queue should have
    // (BUFFER_FREE_THRESHOLD + steady state handout buffer count) buffers.
    static const int BUFFER_FREE_THRESHOLD = 3;

    /**
     * Lock to synchronize the access to the methods of this class.
     */
    mutable Mutex mLock;

    static const size_t kMaxBufferCount = BufferQueueDefs::NUM_BUFFER_SLOTS;

    struct GraphicBufferEntry {
        sp<GraphicBuffer> graphicBuffer;
        int fenceFd;
        explicit GraphicBufferEntry(const sp<GraphicBuffer>& gb = 0, int fd = -1) :
            graphicBuffer(gb),
            fenceFd(fd) {}
    };

    /**
     * A buffer entry (indexed by stream ID) represents a single physically allocated buffer. For
     * Gralloc V0, since each physical buffer is associated with one stream, this is
     * a single entry map. For Gralloc V1, one physical buffer can be shared between different
     * streams in one stream set, so this entry may include multiple entries, where the different
     * graphic buffers have the same common Gralloc backing store.
     */
    typedef int StreamId;
    typedef KeyedVector<StreamId, GraphicBufferEntry> BufferEntry;

    typedef std::list<BufferEntry> BufferList;

    /**
     * Stream info map (indexed by stream ID) tracks all the streams registered to a particular
     * stream set.
     */
    typedef KeyedVector<StreamId, StreamInfo> InfoMap;

    /**
     * Stream set buffer count map (indexed by stream ID) tracks all buffer counts of the streams
     * registered to a particular stream set.
     */
    typedef KeyedVector<StreamId, size_t> BufferCountMap;

    /**
     * StreamSet keeps track of the stream info, free buffer list and hand-out buffer counts for
     * each stream set.
     */
    struct StreamSet {
        /**
         * Stream set buffer count water mark representing the max number of allocated buffers
         * (hand-out buffers + free buffers) count for each stream set. For a given stream set, when
         * getBufferForStream() is called on this buffer manager, if the total allocated buffer
         * count exceeds this water mark, the buffer manager will attempt to reduce it as follows:
         *
         * In getBufferForStream(), find a buffer associated with other streams (inside the same
         * stream set) on the free buffer list and free it. For Gralloc V1, can just free the top
         * of the free buffer list if the physical buffer sharing in this stream is supported.
         *
         * For a particular stream set, a larger allocatedBufferWaterMark increases the memory
         * footprint of the stream set, but reduces the chance that getBufferForStream() will have
         * to allocate a new buffer. We assume that the streams in one stream set are not streaming
         * simultaneously, the max allocated buffer count water mark for a stream set will the max
         * of all streams' total buffer counts. This will avoid new buffer allocation in steady
         * streaming state.
         *
         * This water mark can be dynamically changed, and will grow when the hand-out buffer count
         * of each stream increases, until it reaches the maxAllowedBufferCount.
         */
        size_t allocatedBufferWaterMark;

        /**
         * The max allowed buffer count for this stream set. It is the max of total number of
         * buffers for each stream. This is the upper bound of the allocatedBufferWaterMark.
         */
        size_t maxAllowedBufferCount;

        /**
         * The stream info for all streams in this set
         */
        InfoMap streamInfoMap;
        /**
         * The count of the buffers that were handed out to the streams of this set.
         */
        BufferCountMap handoutBufferCountMap;
        /**
         * The count of the buffers that are attached to the streams of this set.
         * An attached buffer may be free or handed out
         */
        BufferCountMap attachedBufferCountMap;

        StreamSet() {
            allocatedBufferWaterMark = 0;
            maxAllowedBufferCount = 0;
        }
    };

    /**
     * Stream set map managed by this buffer manager.
     */
    typedef int StreamSetId;
    KeyedVector<StreamSetId, StreamSet> mStreamSetMap;
    KeyedVector<StreamId, wp<Camera3OutputStream>> mStreamMap;

    // TODO: There is no easy way to query the Gralloc version in this code yet, we have different
    // code paths for different Gralloc versions, hardcode something here for now.
    const uint32_t mGrallocVersion = GRALLOC_DEVICE_API_VERSION_0_1;

    /**
     * Check if this stream was successfully registered already. This method needs to be called with
     * mLock held.
     */
    bool checkIfStreamRegisteredLocked(int streamId, int streamSetId) const;

    /**
     * Check if other streams in the stream set has extra buffer available to be freed, and
     * free one if so.
     */
    status_t checkAndFreeBufferOnOtherStreamsLocked(int streamId, int streamSetId);
};

} // namespace camera3
} // namespace android

#endif // ANDROID_SERVERS_CAMERA3_BUFFER_MANAGER_H
