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

#ifndef ANDROID_SERVERS_CAMERA3_STREAM_H
#define ANDROID_SERVERS_CAMERA3_STREAM_H

#include <gui/Surface.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/String16.h>
#include <utils/List.h>

#include "hardware/camera3.h"

#include "utils/LatencyHistogram.h"
#include "Camera3StreamBufferListener.h"
#include "Camera3StreamInterface.h"

namespace android {

namespace camera3 {

/**
 * A class for managing a single stream of input or output data from the camera
 * device.
 *
 * The stream has an internal state machine to track whether it's
 * connected/configured/etc.
 *
 * States:
 *
 *  STATE_ERROR: A serious error has occurred, stream is unusable. Outstanding
 *    buffers may still be returned.
 *
 *  STATE_CONSTRUCTED: The stream is ready for configuration, but buffers cannot
 *    be gotten yet. Not connected to any endpoint, no buffers are registered
 *    with the HAL.
 *
 *  STATE_IN_CONFIG: Configuration has started, but not yet concluded. During this
 *    time, the usage, max_buffers, and priv fields of camera3_stream returned by
 *    startConfiguration() may be modified.
 *
 *  STATE_IN_RE_CONFIG: Configuration has started, and the stream has been
 *    configured before. Need to track separately from IN_CONFIG to avoid
 *    re-registering buffers with HAL.
 *
 *  STATE_CONFIGURED: Stream is configured, and has registered buffers with the
 *    HAL (if necessary). The stream's getBuffer/returnBuffer work. The priv
 *    pointer may still be modified.
 *
 *  STATE_PREPARING: The stream's buffers are being pre-allocated for use.  On
 *    older HALs, this is done as part of configuration, but in newer HALs
 *    buffers may be allocated at time of first use. But some use cases require
 *    buffer allocation upfront, to minmize disruption due to lengthy allocation
 *    duration.  In this state, only prepareNextBuffer() and cancelPrepare()
 *    may be called.
 *
 *  STATE_IN_IDLE: This is a temporary state only intended to be used for input
 *    streams and only for the case where we need to re-configure the camera device
 *    while the input stream has an outstanding buffer. All other streams should not
 *    be able to switch to this state. For them this is invalid and should be handled
 *    as an unknown state.
 *
 * Transition table:
 *
 *    <none>               => STATE_CONSTRUCTED:
 *        When constructed with valid arguments
 *    <none>               => STATE_ERROR:
 *        When constructed with invalid arguments
 *    STATE_CONSTRUCTED    => STATE_IN_CONFIG:
 *        When startConfiguration() is called
 *    STATE_IN_CONFIG      => STATE_CONFIGURED:
 *        When finishConfiguration() is called
 *    STATE_IN_CONFIG      => STATE_ERROR:
 *        When finishConfiguration() fails to allocate or register buffers.
 *    STATE_CONFIGURED     => STATE_IN_RE_CONFIG:  *
 *        When startConfiguration() is called again, after making sure stream is
 *        idle with waitUntilIdle().
 *    STATE_IN_RE_CONFIG   => STATE_CONFIGURED:
 *        When finishConfiguration() is called.
 *    STATE_IN_RE_CONFIG   => STATE_ERROR:
 *        When finishConfiguration() fails to allocate or register buffers.
 *    STATE_CONFIGURED     => STATE_CONSTRUCTED:
 *        When disconnect() is called after making sure stream is idle with
 *        waitUntilIdle().
 *    STATE_CONFIGURED     => STATE_PREPARING:
 *        When startPrepare is called before the stream has a buffer
 *        queued back into it for the first time.
 *    STATE_PREPARING      => STATE_CONFIGURED:
 *        When sufficient prepareNextBuffer calls have been made to allocate
 *        all stream buffers, or cancelPrepare is called.
 *    STATE_CONFIGURED     => STATE_ABANDONED:
 *        When the buffer queue of the stream is abandoned.
 *    STATE_CONFIGURED     => STATE_IN_IDLE:
 *        Only for an input stream which has an outstanding buffer.
 *    STATE_IN_IDLE     => STATE_CONFIGURED:
 *        After the internal re-configuration, the input should revert back to
 *        the configured state.
 *
 * Status Tracking:
 *    Each stream is tracked by StatusTracker as a separate component,
 *    depending on the handed out buffer count. The state must be STATE_CONFIGURED
 *    in order for the component to be marked.
 *
 *    It's marked in one of two ways:
 *
 *    - ACTIVE: One or more buffers have been handed out (with #getBuffer).
 *    - IDLE: All buffers have been returned (with #returnBuffer), and their
 *          respective release_fence(s) have been signaled. The only exception to this
 *          rule is an input stream that moves to "STATE_IN_IDLE" during internal
 *          re-configuration.
 *
 *    A typical use case is output streams. When the HAL has any buffers
 *    dequeued, the stream is marked ACTIVE. When the HAL returns all buffers
 *    (e.g. if no capture requests are active), the stream is marked IDLE.
 *    In this use case, the app consumer does not affect the component status.
 *
 */
class Camera3Stream :
        protected camera3_stream,
        public virtual Camera3StreamInterface,
        public virtual RefBase {
  public:

    virtual ~Camera3Stream();

    static Camera3Stream*       cast(camera3_stream *stream);
    static const Camera3Stream* cast(const camera3_stream *stream);

    /**
     * Get the stream's ID
     */
    int              getId() const;

    /**
     * Get the output stream set id.
     */
    int              getStreamSetId() const;

    /**
     * Get the stream's dimensions and format
     */
    uint32_t          getWidth() const;
    uint32_t          getHeight() const;
    int               getFormat() const;
    android_dataspace getDataSpace() const;
    uint64_t          getUsage() const;
    void              setUsage(uint64_t usage);
    void              setFormatOverride(bool formatOverriden);
    bool              isFormatOverridden() const;
    int               getOriginalFormat() const;
    void              setDataSpaceOverride(bool dataSpaceOverriden);
    bool              isDataSpaceOverridden() const;
    android_dataspace getOriginalDataSpace() const;
    const String8&    physicalCameraId() const;

    camera3_stream*   asHalStream() override {
        return this;
    }

    /**
     * Start the stream configuration process. Returns a handle to the stream's
     * information to be passed into the HAL device's configure_streams call.
     *
     * Until finishConfiguration() is called, no other methods on the stream may be
     * called. The usage and max_buffers fields of camera3_stream may be modified
     * between start/finishConfiguration, but may not be changed after that.
     * The priv field of camera3_stream may be modified at any time after
     * startConfiguration.
     *
     * Returns NULL in case of error starting configuration.
     */
    camera3_stream*  startConfiguration();

    /**
     * Check if the stream is mid-configuration (start has been called, but not
     * finish).  Used for lazy completion of configuration.
     */
    bool             isConfiguring() const;

    /**
     * Completes the stream configuration process. The stream information
     * structure returned by startConfiguration() may no longer be modified
     * after this call, but can still be read until the destruction of the
     * stream.
     *
     * streamReconfigured: set to true when a stream is being reconfigured.
     *
     * Returns:
     *   OK on a successful configuration
     *   NO_INIT in case of a serious error from the HAL device
     *   NO_MEMORY in case of an error registering buffers
     *   INVALID_OPERATION in case connecting to the consumer failed or consumer
     *       doesn't exist yet.
     */
    status_t         finishConfiguration(/*out*/bool* streamReconfigured = nullptr);

    /**
     * Cancels the stream configuration process. This returns the stream to the
     * initial state, allowing it to be configured again later.
     * This is done if the HAL rejects the proposed combined stream configuration
     */
    status_t         cancelConfiguration();

    /**
     * Determine whether the stream has already become in-use (has received
     * a valid filled buffer), which determines if a stream can still have
     * prepareNextBuffer called on it.
     */
    bool             isUnpreparable();

    /**
     * Start stream preparation. May only be called in the CONFIGURED state,
     * when no valid buffers have yet been returned to this stream. Prepares
     * up to maxCount buffers, or the maximum number of buffers needed by the
     * pipeline if maxCount is ALLOCATE_PIPELINE_MAX.
     *
     * If no prepartion is necessary, returns OK and does not transition to
     * PREPARING state. Otherwise, returns NOT_ENOUGH_DATA and transitions
     * to PREPARING.
     *
     * This call performs no allocation, so is quick to call.
     *
     * blockRequest specifies whether prepare will block upcoming capture
     * request. This flag should only be set to false if the caller guarantees
     * the whole buffer preparation process is done before capture request
     * comes in.
     *
     * Returns:
     *    OK if no more buffers need to be preallocated
     *    NOT_ENOUGH_DATA if calls to prepareNextBuffer are needed to finish
     *        buffer pre-allocation, and transitions to the PREPARING state.
     *    NO_INIT in case of a serious error from the HAL device
     *    INVALID_OPERATION if called when not in CONFIGURED state, or a
     *        valid buffer has already been returned to this stream.
     */
    status_t         startPrepare(int maxCount, bool blockRequest);

    /**
     * Check if the request on a stream is blocked by prepare.
     */
    bool             isBlockedByPrepare() const;

    /**
     * Continue stream buffer preparation by allocating the next
     * buffer for this stream.  May only be called in the PREPARED state.
     *
     * Returns OK and transitions to the CONFIGURED state if all buffers
     * are allocated after the call concludes. Otherwise returns NOT_ENOUGH_DATA.
     *
     * This call allocates one buffer, which may take several milliseconds for
     * large buffers.
     *
     * Returns:
     *    OK if no more buffers need to be preallocated, and transitions
     *        to the CONFIGURED state.
     *    NOT_ENOUGH_DATA if more calls to prepareNextBuffer are needed to finish
     *        buffer pre-allocation.
     *    NO_INIT in case of a serious error from the HAL device
     *    INVALID_OPERATION if called when not in CONFIGURED state, or a
     *        valid buffer has already been returned to this stream.
     */
    status_t         prepareNextBuffer();

    /**
     * Cancel stream preparation early. In case allocation needs to be
     * stopped, this method transitions the stream back to the CONFIGURED state.
     * Buffers that have been allocated with prepareNextBuffer remain that way,
     * but a later use of prepareNextBuffer will require just as many
     * calls as if the earlier prepare attempt had not existed.
     *
     * Returns:
     *    OK if cancellation succeeded, and transitions to the CONFIGURED state
     *    INVALID_OPERATION if not in the PREPARING state
     *    NO_INIT in case of a serious error from the HAL device
     */
    status_t        cancelPrepare();

    /**
     * Tear down memory for this stream. This frees all unused gralloc buffers
     * allocated for this stream, but leaves it ready for operation afterward.
     *
     * May only be called in the CONFIGURED state, and keeps the stream in
     * the CONFIGURED state.
     *
     * Returns:
     *    OK if teardown succeeded.
     *    INVALID_OPERATION if not in the CONFIGURED state
     *    NO_INIT in case of a serious error from the HAL device
     */
    status_t       tearDown();

    /**
     * Fill in the camera3_stream_buffer with the next valid buffer for this
     * stream, to hand over to the HAL.
     *
     * Multiple surfaces could share the same HAL stream, but a request may
     * be only for a subset of surfaces. In this case, the
     * Camera3StreamInterface object needs the surface ID information to acquire
     * buffers for those surfaces.
     *
     * This method may only be called once finishConfiguration has been called.
     * For bidirectional streams, this method applies to the output-side
     * buffers.
     *
     */
    status_t         getBuffer(camera3_stream_buffer *buffer,
            nsecs_t waitBufferTimeout,
            const std::vector<size_t>& surface_ids = std::vector<size_t>());

    /**
     * Return a buffer to the stream after use by the HAL.
     *
     * Multiple surfaces could share the same HAL stream, but a request may
     * be only for a subset of surfaces. In this case, the
     * Camera3StreamInterface object needs the surface ID information to attach
     * buffers for those surfaces.
     *
     * This method may only be called for buffers provided by getBuffer().
     * For bidirectional streams, this method applies to the output-side buffers
     */
    status_t         returnBuffer(const camera3_stream_buffer &buffer,
            nsecs_t timestamp, bool timestampIncreasing,
            const std::vector<size_t>& surface_ids = std::vector<size_t>(),
            uint64_t frameNumber = 0);

    /**
     * Fill in the camera3_stream_buffer with the next valid buffer for this
     * stream, to hand over to the HAL.
     *
     * This method may only be called once finishConfiguration has been called.
     * For bidirectional streams, this method applies to the input-side
     * buffers.
     *
     * Normally this call will block until the handed out buffer count is less than the stream
     * max buffer count; if respectHalLimit is set to false, this is ignored.
     */
    status_t         getInputBuffer(camera3_stream_buffer *buffer, bool respectHalLimit = true);

    /**
     * Return a buffer to the stream after use by the HAL.
     *
     * This method may only be called for buffers provided by getBuffer().
     * For bidirectional streams, this method applies to the input-side buffers
     */
    status_t         returnInputBuffer(const camera3_stream_buffer &buffer);

    // get the buffer producer of the input buffer queue.
    // only apply to input streams.
    status_t         getInputBufferProducer(sp<IGraphicBufferProducer> *producer);

    /**
     * Whether any of the stream's buffers are currently in use by the HAL,
     * including buffers that have been returned but not yet had their
     * release fence signaled.
     */
    bool             hasOutstandingBuffers() const;

    /**
     * Get number of buffers currently handed out to HAL
     */
    size_t           getOutstandingBuffersCount() const;

    enum {
        TIMEOUT_NEVER = -1
    };

    /**
     * Set the status tracker to notify about idle transitions
     */
    virtual status_t setStatusTracker(sp<StatusTracker> statusTracker);

    /**
     * Disconnect stream from its non-HAL endpoint. After this,
     * start/finishConfiguration must be called before the stream can be used
     * again. This cannot be called if the stream has outstanding dequeued
     * buffers.
     */
    status_t         disconnect();

    /**
     * Debug dump of the stream's state.
     */
    virtual void     dump(int fd, const Vector<String16> &args) const;

    /**
     * Add a camera3 buffer listener. Adding the same listener twice has
     * no effect.
     */
    void             addBufferListener(
            wp<Camera3StreamBufferListener> listener);

    /**
     * Remove a camera3 buffer listener. Removing the same listener twice
     * or the listener that was never added has no effect.
     */
    void             removeBufferListener(
            const sp<Camera3StreamBufferListener>& listener);


    // Setting listener will remove previous listener (if exists)
    virtual void     setBufferFreedListener(
            wp<Camera3StreamBufferFreedListener> listener) override;

    /**
     * Return if the buffer queue of the stream is abandoned.
     */
    bool             isAbandoned() const;

    /**
     * Switch a configured stream with possibly outstanding buffers in idle
     * state. Configuration for such streams will be skipped assuming there
     * are no changes to the stream parameters.
     */
    status_t         forceToIdle();

    /**
     * Restore a forced idle stream to configured state, marking it active
     * in case it contains outstanding buffers.
     */
    status_t         restoreConfiguredState();

    /**
     * Notify buffer stream listeners about incoming request with particular frame number.
     */
    void fireBufferRequestForFrameNumber(uint64_t frameNumber,
            const CameraMetadata& settings) override;

  protected:
    const int mId;
    /**
     * Stream set id, used to indicate which group of this stream belongs to for buffer sharing
     * across multiple streams.
     *
     * The default value is set to CAMERA3_STREAM_SET_ID_INVALID, which indicates that this stream
     * doesn't intend to share buffers with any other streams, and this stream will fall back to
     * the existing BufferQueue mechanism to manage the buffer allocations and buffer circulation.
     * When a valid stream set id is set, this stream intends to use the Camera3BufferManager to
     * manage the buffer allocations; the BufferQueue will only handle the buffer transaction
     * between the producer and consumer. For this case, upon successfully registration, the streams
     * with the same stream set id will potentially share the buffers allocated by
     * Camera3BufferManager.
     */
    const int mSetId;

    const String8 mName;
    // Zero for formats with fixed buffer size for given dimensions.
    const size_t mMaxSize;

    enum StreamState {
        STATE_ERROR,
        STATE_CONSTRUCTED,
        STATE_IN_CONFIG,
        STATE_IN_RECONFIG,
        STATE_CONFIGURED,
        STATE_PREPARING,
        STATE_ABANDONED,
        STATE_IN_IDLE
    } mState;

    mutable Mutex mLock;

    Camera3Stream(int id, camera3_stream_type type,
            uint32_t width, uint32_t height, size_t maxSize, int format,
            android_dataspace dataSpace, camera3_stream_rotation_t rotation,
            const String8& physicalCameraId, int setId);

    wp<Camera3StreamBufferFreedListener> mBufferFreedListener;

    /**
     * Interface to be implemented by derived classes
     */

    // getBuffer / returnBuffer implementations

    // Since camera3_stream_buffer includes a raw pointer to the stream,
    // cast to camera3_stream*, implementations must increment the
    // refcount of the stream manually in getBufferLocked, and decrement it in
    // returnBufferLocked.
    virtual status_t getBufferLocked(camera3_stream_buffer *buffer,
            const std::vector<size_t>& surface_ids = std::vector<size_t>());
    virtual status_t returnBufferLocked(const camera3_stream_buffer &buffer,
            nsecs_t timestamp,
            const std::vector<size_t>& surface_ids = std::vector<size_t>());
    virtual status_t getInputBufferLocked(camera3_stream_buffer *buffer);
    virtual status_t returnInputBufferLocked(
            const camera3_stream_buffer &buffer);
    virtual bool     hasOutstandingBuffersLocked() const = 0;
    // Get the buffer producer of the input buffer queue. Only apply to input streams.
    virtual status_t getInputBufferProducerLocked(sp<IGraphicBufferProducer> *producer);

    // Can return -ENOTCONN when we are already disconnected (not an error)
    virtual status_t disconnectLocked() = 0;

    // Configure the buffer queue interface to the other end of the stream,
    // after the HAL has provided usage and max_buffers values. After this call,
    // the stream must be ready to produce all buffers for registration with
    // HAL.
    // Returns NO_INIT or DEAD_OBJECT if the queue has been abandoned.
    virtual status_t configureQueueLocked() = 0;

    // Get the total number of buffers in the queue
    virtual size_t   getBufferCountLocked() = 0;

    // Get handout output buffer count.
    virtual size_t   getHandoutOutputBufferCountLocked() const = 0;

    // Get handout input buffer count.
    virtual size_t   getHandoutInputBufferCountLocked() = 0;

    // Get the usage flags for the other endpoint, or return
    // INVALID_OPERATION if they cannot be obtained.
    virtual status_t getEndpointUsage(uint64_t *usage) const = 0;

    // Return whether the buffer is in the list of outstanding buffers.
    bool isOutstandingBuffer(const camera3_stream_buffer& buffer) const;

    // Tracking for idle state
    wp<StatusTracker> mStatusTracker;
    // Status tracker component ID
    int mStatusId;

    // Tracking for stream prepare - whether this stream can still have
    // prepareNextBuffer called on it.
    bool mStreamUnpreparable;

    uint64_t mUsage;

  private:
    // Previously configured stream properties (post HAL override)
    uint64_t mOldUsage;
    uint32_t mOldMaxBuffers;
    int mOldFormat;
    android_dataspace mOldDataSpace;

    Condition mOutputBufferReturnedSignal;
    Condition mInputBufferReturnedSignal;
    static const nsecs_t kWaitForBufferDuration = 3000000000LL; // 3000 ms

    void fireBufferListenersLocked(const camera3_stream_buffer& buffer,
            bool acquired, bool output, nsecs_t timestamp = 0, uint64_t frameNumber = 0);
    List<wp<Camera3StreamBufferListener> > mBufferListenerList;

    status_t        cancelPrepareLocked();

    // Remove the buffer from the list of outstanding buffers.
    void removeOutstandingBuffer(const camera3_stream_buffer& buffer);

    // Tracking for PREPARING state

    // State of buffer preallocation. Only true if either prepareNextBuffer
    // has been called sufficient number of times, or stream configuration
    // had to register buffers with the HAL
    bool mPrepared;
    bool mPrepareBlockRequest;

    Vector<camera3_stream_buffer_t> mPreparedBuffers;
    size_t mPreparedBufferIdx;

    // Number of buffers allocated on last prepare call.
    size_t mLastMaxCount;

    mutable Mutex mOutstandingBuffersLock;
    // Outstanding buffers dequeued from the stream's buffer queue.
    List<buffer_handle_t> mOutstandingBuffers;

    // Latency histogram of the wait time for handout buffer count to drop below
    // max_buffers.
    static const int32_t kBufferLimitLatencyBinSize = 33; //in ms
    CameraLatencyHistogram mBufferLimitLatency;

    //Keep track of original format when the stream is created in case it gets overridden
    bool mFormatOverridden;
    const int mOriginalFormat;

    //Keep track of original dataSpace in case it gets overridden
    bool mDataSpaceOverridden;
    android_dataspace mOriginalDataSpace;

    String8 mPhysicalCameraId;
    nsecs_t mLastTimestamp;
}; // class Camera3Stream

}; // namespace camera3

}; // namespace android

#endif
