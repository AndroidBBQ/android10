/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef GRAPHIC_BUFFER_SOURCE_H_

#define GRAPHIC_BUFFER_SOURCE_H_

#include <binder/Status.h>
#include <utils/RefBase.h>

#include <media/hardware/VideoAPI.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/bqhelper/ComponentWrapper.h>
#include <android/hardware/graphics/bufferqueue/1.0/IGraphicBufferProducer.h>
#include <android/hardware/graphics/bufferqueue/2.0/IGraphicBufferProducer.h>

namespace android {

using ::android::binder::Status;

struct FrameDropper;
class BufferItem;
class IGraphicBufferProducer;
class IGraphicBufferConsumer;
/*
 * This class is used to feed codecs from a Surface via BufferQueue or
 * HW producer.
 *
 * Instances of the class don't run on a dedicated thread.  Instead,
 * various events trigger data movement:
 *
 *  - Availability of a new frame of data from the BufferQueue (notified
 *    via the onFrameAvailable callback).
 *  - The return of a codec buffer.
 *  - Application signaling end-of-stream.
 *  - Transition to or from "executing" state.
 *
 * Frames of data (and, perhaps, the end-of-stream indication) can arrive
 * before the codec is in the "executing" state, so we need to queue
 * things up until we're ready to go.
 *
 * The GraphicBufferSource can be configure dynamically to discard frames
 * from the source:
 *
 * - if their timestamp is less than a start time
 * - if the source is suspended or stopped and the suspend/stop-time is reached
 * - if EOS was signaled
 * - if there is no encoder connected to it
 *
 * The source, furthermore, may choose to not encode (drop) frames if:
 *
 * - to throttle the frame rate (keep it under a certain limit)
 *
 * Finally the source may optionally hold onto the last non-discarded frame
 * (even if it was dropped) to reencode it after an interval if no further
 * frames are sent by the producer.
 */
class GraphicBufferSource : public RefBase {
public:
    GraphicBufferSource();

    virtual ~GraphicBufferSource();

    // We can't throw an exception if the constructor fails, so we just set
    // this and require that the caller test the value.
    status_t initCheck() const {
        return mInitCheck;
    }

    // Returns the handle to the producer side of the BufferQueue.  Buffers
    // queued on this will be received by GraphicBufferSource.
    sp<IGraphicBufferProducer> getIGraphicBufferProducer() const;

    // Returns the handle to the bufferqueue HAL (V1_0) producer side of the BufferQueue.
    // Buffers queued on this will be received by GraphicBufferSource.
    sp<::android::hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer>
        getHGraphicBufferProducer_V1_0() const;

    // Returns the handle to the bufferqueue HAL producer side of the BufferQueue.
    // Buffers queued on this will be received by GraphicBufferSource.
    sp<::android::hardware::graphics::bufferqueue::V2_0::IGraphicBufferProducer>
        getHGraphicBufferProducer() const;

    // This is called when component transitions to running state, which means
    // we can start handing it buffers.  If we already have buffers of data
    // sitting in the BufferQueue, this will send them to the codec.
    Status start();

    // This is called when component transitions to stopped, indicating that
    // the codec is meant to return all buffers back to the client for them
    // to be freed. Do NOT submit any more buffers to the component.
    Status stop();

    // This is called when component transitions to released, indicating that
    // we are shutting down.
    Status release();

    // A "codec buffer", i.e. a buffer that can be used to pass data into
    // the encoder, has been allocated.  (This call does not call back into
    // component.)
    Status onInputBufferAdded(int32_t bufferId);

    // Called when encoder is no longer using the buffer.  If we have a BQ
    // buffer available, fill it with a new frame of data; otherwise, just mark
    // it as available.
    Status onInputBufferEmptied(int32_t bufferId, int fenceFd);

    // IGraphicBufferSource interface
    // ------------------------------

    // Configure the buffer source to be used with a component with the default
    // data space.
    status_t configure(
        const sp<ComponentWrapper> &component,
        int32_t dataSpace,
        int32_t bufferCount,
        uint32_t frameWidth,
        uint32_t frameHeight,
        uint32_t consumerUsage);

    // This is called after the last input frame has been submitted or buffer
    // timestamp is greater or equal than stopTimeUs. We need to submit an empty
    // buffer with the EOS flag set.  If we don't have a codec buffer ready,
    // we just set the mEndOfStream flag.
    status_t signalEndOfInputStream();

    // If suspend is true, all incoming buffers (including those currently
    // in the BufferQueue) with timestamp larger than timeUs will be discarded
    // until the suspension is lifted. If suspend is false, all incoming buffers
    // including those currently in the BufferQueue) with timestamp larger than
    // timeUs will be processed. timeUs uses SYSTEM_TIME_MONOTONIC time base.
    status_t setSuspend(bool suspend, int64_t timeUs);

    // Specifies the interval after which we requeue the buffer previously
    // queued to the encoder. This is useful in the case of surface flinger
    // providing the input surface if the resulting encoded stream is to
    // be displayed "live". If we were not to push through the extra frame
    // the decoder on the remote end would be unable to decode the latest frame.
    // This API must be called before transitioning the encoder to "executing"
    // state and once this behaviour is specified it cannot be reset.
    status_t setRepeatPreviousFrameDelayUs(int64_t repeatAfterUs);

    // Sets the input buffer timestamp offset.
    // When set, the sample's timestamp will be adjusted with the timeOffsetUs.
    status_t setTimeOffsetUs(int64_t timeOffsetUs);

    /*
     * Set the maximum frame rate on the source.
     *
     * When maxFps is a positive number, it indicates the maximum rate at which
     * the buffers from this source will be sent to the encoder. Excessive
     * frames will be dropped to meet the frame rate requirement.
     *
     * When maxFps is a negative number, any frame drop logic will be disabled
     * and all frames from this source will be sent to the encoder, even when
     * the timestamp goes backwards. Note that some components may still drop
     * out-of-order frames silently, so this usually has to be used in
     * conjunction with OMXNodeInstance::setMaxPtsGapUs() workaround.
     *
     * When maxFps is 0, this call will fail with BAD_VALUE.
     */
    status_t setMaxFps(float maxFps);

    // Sets the time lapse (or slow motion) parameters.
    // When set, the sample's timestamp will be modified to playback framerate,
    // and capture timestamp will be modified to capture rate.
    status_t setTimeLapseConfig(double fps, double captureFps);

    // Sets the start time us (in system time), samples before which should
    // be dropped and not submitted to encoder
    status_t setStartTimeUs(int64_t startTimeUs);

    // Sets the stop time us (in system time), samples after which should be dropped
    // and not submitted to encoder. timeUs uses SYSTEM_TIME_MONOTONIC time base.
    status_t setStopTimeUs(int64_t stopTimeUs);

    // Gets the stop time offset in us. This is the time offset between latest buffer
    // time and the stopTimeUs. If stop time is not set, INVALID_OPERATION will be returned.
    // If return is OK, *stopTimeOffsetUs will contain the valid offset. Otherwise,
    // *stopTimeOffsetUs will not be modified. Positive stopTimeOffsetUs means buffer time
    // larger than stopTimeUs.
    status_t getStopTimeOffsetUs(int64_t *stopTimeOffsetUs);

    // Sets the desired color aspects, e.g. to be used when producer does not specify a dataspace.
    status_t setColorAspects(int32_t aspectsPacked);

protected:

    // BufferQueue::ConsumerListener interface, called when a new frame of
    // data is available.  If we're executing and a codec buffer is
    // available, we acquire the buffer, copy the GraphicBuffer reference
    // into the codec buffer, and call Empty[This]Buffer.  If we're not yet
    // executing or there's no codec buffer available, we just increment
    // mNumFramesAvailable and return.
    void onFrameAvailable(const BufferItem& item) ;

    // BufferQueue::ConsumerListener interface, called when the client has
    // released one or more GraphicBuffers.  We clear out the appropriate
    // set of mBufferSlot entries.
    void onBuffersReleased() ;

    // BufferQueue::ConsumerListener interface, called when the client has
    // changed the sideband stream. GraphicBufferSource doesn't handle sideband
    // streams so this is a no-op (and should never be called).
    void onSidebandStreamChanged() ;

private:
    // BQ::ConsumerListener interface
    // ------------------------------
    struct ConsumerProxy;
    sp<ConsumerProxy> mConsumerProxy;

    // Lock, covers all member variables.
    mutable Mutex mMutex;

    // Used to report constructor failure.
    status_t mInitCheck;

    // Graphic buffer reference objects
    // --------------------------------

    // These are used to keep a shared reference to GraphicBuffers and gralloc handles owned by the
    // GraphicBufferSource as well as to manage the cache slots. Separate references are owned by
    // the buffer cache (controlled by the buffer queue/buffer producer) and the codec.

    // When we get a buffer from the producer (BQ) it designates them to be cached into specific
    // slots. Each slot owns a shared reference to the graphic buffer (we track these using
    // CachedBuffer) that is in that slot, but the producer controls the slots.
    struct CachedBuffer;

    // When we acquire a buffer, we must release it back to the producer once we (or the codec)
    // no longer uses it (as long as the buffer is still in the cache slot). We use shared
    // AcquiredBuffer instances for this purpose - and we call release buffer when the last
    // reference is relinquished.
    struct AcquiredBuffer;

    // We also need to keep some extra metadata (other than the buffer reference) for acquired
    // buffers. These are tracked in VideoBuffer struct.
    struct VideoBuffer {
        std::shared_ptr<AcquiredBuffer> mBuffer;
        nsecs_t mTimestampNs;
        android_dataspace_t mDataspace;
    };

    // Cached and aquired buffers
    // --------------------------------

    typedef int slot_id;

    // Maps a slot to the cached buffer in that slot
    KeyedVector<slot_id, std::shared_ptr<CachedBuffer>> mBufferSlots;

    // Queue of buffers acquired in chronological order that are not yet submitted to the codec
    List<VideoBuffer> mAvailableBuffers;

    // Number of buffers that have been signaled by the producer that they are available, but
    // we've been unable to acquire them due to our max acquire count
    int32_t mNumAvailableUnacquiredBuffers;

    // Number of frames acquired from consumer (debug only)
    // (as in aquireBuffer called, and release needs to be called)
    int32_t mNumOutstandingAcquires;

    // Acquire a buffer from the BQ and store it in |item| if successful
    // \return OK on success, or error on failure.
    status_t acquireBuffer_l(VideoBuffer *item);

    // Called when a buffer was acquired from the producer
    void onBufferAcquired_l(const VideoBuffer &buffer);

    // marks the buffer at the slot no longer cached, and accounts for the outstanding
    // acquire count. Returns true if the slot was populated; otherwise, false.
    bool discardBufferInSlot_l(slot_id i);

    // marks the buffer at the slot index no longer cached, and accounts for the outstanding
    // acquire count
    void discardBufferAtSlotIndex_l(ssize_t bsi);

    // release all acquired and unacquired available buffers
    // This method will return if it fails to acquire an unacquired available buffer, which will
    // leave mNumAvailableUnacquiredBuffers positive on return.
    void releaseAllAvailableBuffers_l();

    // returns whether we have any available buffers (acquired or not-yet-acquired)
    bool haveAvailableBuffers_l() const {
        return !mAvailableBuffers.empty() || mNumAvailableUnacquiredBuffers > 0;
    }

    // Codec buffers
    // -------------

    // When we queue buffers to the encoder, we must hold the references to the graphic buffers
    // in those buffers - as the producer may free the slots.

    typedef int32_t codec_buffer_id;

    // set of codec buffer ID-s of buffers available to fill
    List<codec_buffer_id> mFreeCodecBuffers;

    // maps codec buffer ID-s to buffer info submitted to the codec. Used to keep a reference for
    // the graphics buffer.
    KeyedVector<codec_buffer_id, std::shared_ptr<AcquiredBuffer>> mSubmittedCodecBuffers;

    // Processes the next acquired frame. If there is no available codec buffer, it returns false
    // without any further action.
    //
    // Otherwise, it consumes the next acquired frame and determines if it needs to be discarded or
    // dropped. If neither are needed, it submits it to the codec. It also saves the latest
    // non-dropped frame and submits it for repeat encoding (if this is enabled).
    //
    // \require there must be an acquired frame (i.e. we're in the onFrameAvailable callback,
    // or if we're in codecBufferEmptied and mNumFramesAvailable is nonzero).
    // \require codec must be executing
    // \returns true if acquired (and handled) the next frame. Otherwise, false.
    bool fillCodecBuffer_l();

    // Calculates the media timestamp for |item| and on success it submits the buffer to the codec,
    // while also keeping a reference for it in mSubmittedCodecBuffers.
    // Returns UNKNOWN_ERROR if the buffer was not submitted due to buffer timestamp. Otherwise,
    // it returns any submit success or error value returned by the codec.
    status_t submitBuffer_l(const VideoBuffer &item);

    // Submits an empty buffer, with the EOS flag set if there is an available codec buffer and
    // sets mEndOfStreamSent flag. Does nothing if there is no codec buffer available.
    void submitEndOfInputStream_l();

    // Set to true if we want to send end-of-stream after we run out of available frames from the
    // producer
    bool mEndOfStream;

    // Flag that the EOS was submitted to the encoder
    bool mEndOfStreamSent;

    // Dataspace for the last frame submitted to the codec
    android_dataspace mLastDataspace;

    // Default color aspects for this source
    int32_t mDefaultColorAspectsPacked;

    // called when the data space of the input buffer changes
    void onDataspaceChanged_l(android_dataspace dataspace, android_pixel_format pixelFormat);

    // Pointer back to the component that created us.  We send buffers here.
    sp<ComponentWrapper> mComponent;

    // Set by start() / stop().
    bool mExecuting;

    bool mSuspended;

    // returns true if this source is unconditionally discarding acquired buffers at the moment
    // regardless of the metadata of those buffers
    bool areWeDiscardingAvailableBuffers_l();

    int64_t mLastFrameTimestampUs;

    // Our BufferQueue interfaces. mProducer is passed to the producer through
    // getIGraphicBufferProducer, and mConsumer is used internally to retrieve
    // the buffers queued by the producer.
    sp<IGraphicBufferProducer> mProducer;
    sp<IGraphicBufferConsumer> mConsumer;

    // The time to stop sending buffers.
    int64_t mStopTimeUs;

    struct ActionItem {
        typedef enum {
            PAUSE,
            RESUME,
            STOP
        } ActionType;
        ActionType mAction;
        int64_t mActionTimeUs;
    };

    // Maintain last action timestamp to ensure all the action timestamps are
    // monotonically increasing.
    int64_t mLastActionTimeUs;

    // An action queue that queue up all the actions sent to GraphicBufferSource.
    // STOP action should only show up at the end of the list as all the actions
    // after a STOP action will be discarded. mActionQueue is protected by mMutex.
    List<ActionItem> mActionQueue;

    ////
    friend struct AHandlerReflector<GraphicBufferSource>;

    enum {
        kWhatRepeatLastFrame,   ///< queue last frame for reencoding
    };
    enum {
        kRepeatLastFrameCount = 10,
    };

    int64_t mSkipFramesBeforeNs;

    sp<FrameDropper> mFrameDropper;

    sp<ALooper> mLooper;
    sp<AHandlerReflector<GraphicBufferSource> > mReflector;

    // Repeat last frame feature
    // -------------------------
    // configuration parameter: repeat interval for frame repeating (<0 if repeating is disabled)
    int64_t mFrameRepeatIntervalUs;

    // current frame repeat generation - used to cancel a pending frame repeat
    int32_t mRepeatLastFrameGeneration;

    // number of times to repeat latest frame (0 = none)
    int32_t mOutstandingFrameRepeatCount;

    // The previous buffer should've been repeated but
    // no codec buffer was available at the time.
    bool mFrameRepeatBlockedOnCodecBuffer;

    // hold a reference to the last acquired (and not discarded) frame for frame repeating
    VideoBuffer mLatestBuffer;

    // queue last frame for reencode after the repeat interval.
    void queueFrameRepeat_l();

    // save |item| as the latest buffer and queue it for reencode (repeat)
    void setLatestBuffer_l(const VideoBuffer &item);

    // submit last frame to encoder and queue it for reencode
    // \return true if buffer was submitted, false if it wasn't (e.g. source is suspended, there
    // is no available codec buffer)
    bool repeatLatestBuffer_l();

    // Time lapse / slow motion configuration
    // --------------------------------------

    // desired frame rate for encoding - value <= 0 if undefined
    double mFps;

    // desired frame rate for capture - value <= 0 if undefined
    double mCaptureFps;

    // Time lapse mode is enabled if the capture frame rate is defined and it is
    // smaller than half the encoding frame rate (if defined). In this mode,
    // frames that come in between the capture interval (the reciprocal of the
    // capture frame rate) are dropped and the encoding timestamp is adjusted to
    // match the desired encoding frame rate.
    //
    // Slow motion mode is enabled if both encoding and capture frame rates are
    // defined and the encoding frame rate is less than half the capture frame
    // rate. In this mode, the source is expected to produce frames with an even
    // timestamp interval (after rounding) with the configured capture fps. The
    // first source timestamp is used as the source base time. Afterwards, the
    // timestamp of each source frame is snapped to the nearest expected capture
    // timestamp and scaled to match the configured encoding frame rate.

    // These modes must be enabled before using this source.

    // adjusted capture timestamp of the base frame
    int64_t mBaseCaptureUs;

    // adjusted encoding timestamp of the base frame
    int64_t mBaseFrameUs;

    // number of frames from the base time
    int64_t mFrameCount;

    // adjusted capture timestamp for previous frame (negative if there were
    // none)
    int64_t mPrevCaptureUs;

    // adjusted media timestamp for previous frame (negative if there were none)
    int64_t mPrevFrameUs;

    // desired offset between media time and capture time
    int64_t mInputBufferTimeOffsetUs;

    // Calculates and outputs the timestamp to use for a buffer with a specific buffer timestamp
    // |bufferTimestampNs|. Returns false on failure (buffer too close or timestamp is moving
    // backwards). Otherwise, stores the media timestamp in |*codecTimeUs| and returns true.
    //
    // This method takes into account the start time offset and any time lapse or slow motion time
    // adjustment requests.
    bool calculateCodecTimestamp_l(nsecs_t bufferTimeNs, int64_t *codecTimeUs);

    void onMessageReceived(const sp<AMessage> &msg);

    DISALLOW_EVIL_CONSTRUCTORS(GraphicBufferSource);
};

}  // namespace android

#endif  // GRAPHIC_BUFFER_SOURCE_H_
