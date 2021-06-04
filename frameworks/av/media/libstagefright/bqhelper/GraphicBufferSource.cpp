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

#include <inttypes.h>

#define LOG_TAG "GraphicBufferSource"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#define STRINGIFY_ENUMS // for asString in HardwareAPI.h/VideoAPI.h

#include <media/stagefright/bqhelper/GraphicBufferSource.h>
#include <media/stagefright/bqhelper/FrameDropper.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ColorUtils.h>
#include <media/stagefright/foundation/FileDescriptor.h>

#include <media/hardware/MetadataBufferType.h>
#include <ui/GraphicBuffer.h>
#include <gui/BufferItem.h>
#include <gui/BufferQueue.h>
#include <gui/bufferqueue/1.0/WGraphicBufferProducer.h>
#include <gui/bufferqueue/2.0/B2HGraphicBufferProducer.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/IGraphicBufferConsumer.h>
#include <media/hardware/HardwareAPI.h>

#include <inttypes.h>

#include <functional>
#include <memory>
#include <cmath>

namespace android {

namespace {
// kTimestampFluctuation is an upper bound of timestamp fluctuation from the
// source that GraphicBufferSource allows. The unit of kTimestampFluctuation is
// frames. More specifically, GraphicBufferSource will drop a frame if
//
// expectedNewFrametimestamp - actualNewFrameTimestamp <
//     (0.5 - kTimestampFluctuation) * expectedtimePeriodBetweenFrames
//
// where
// - expectedNewFrameTimestamp is the calculated ideal timestamp of the new
//   incoming frame
// - actualNewFrameTimestamp is the timestamp received from the source
// - expectedTimePeriodBetweenFrames is the ideal difference of the timestamps
//   of two adjacent frames
//
// See GraphicBufferSource::calculateCodecTimestamp_l() for more detail about
// how kTimestampFluctuation is used.
//
// kTimestampFluctuation should be non-negative. A higher value causes a smaller
// chance of dropping frames, but at the same time a higher bound on the
// difference between the source timestamp and the interpreted (snapped)
// timestamp.
//
// The value of 0.05 means that GraphicBufferSource expects the input timestamps
// to fluctuate no more than 5% from the regular time period.
//
// TODO: Justify the choice of this value, or make it configurable.
constexpr double kTimestampFluctuation = 0.05;
}

/**
 * A copiable object managing a buffer in the buffer cache managed by the producer. This object
 * holds a reference to the buffer, and maintains which buffer slot it belongs to (if any), and
 * whether it is still in a buffer slot. It also maintains whether there are any outstanging acquire
 * references to it (by buffers acquired from the slot) mainly so that we can keep a debug
 * count of how many buffers we need to still release back to the producer.
 */
struct GraphicBufferSource::CachedBuffer {
    /**
     * Token that is used to track acquire counts (as opposed to all references to this object).
     */
    struct Acquirable { };

    /**
     * Create using a buffer cached in a slot.
     */
    CachedBuffer(slot_id slot, const sp<GraphicBuffer> &graphicBuffer)
        : mIsCached(true),
          mSlot(slot),
          mGraphicBuffer(graphicBuffer),
          mAcquirable(std::make_shared<Acquirable>()) {
    }

    /**
     * Returns the cache slot that this buffer is cached in, or -1 if it is no longer cached.
     *
     * This assumes that -1 slot id is invalid; though, it is just a benign collision used for
     * debugging. This object explicitly manages whether it is still cached.
     */
    slot_id getSlot() const {
        return mIsCached ? mSlot : -1;
    }

    /**
     * Returns the cached buffer.
     */
    sp<GraphicBuffer> getGraphicBuffer() const {
        return mGraphicBuffer;
    }

    /**
     * Checks whether this buffer is still in the buffer cache.
     */
    bool isCached() const {
        return mIsCached;
    }

    /**
     * Checks whether this buffer has an acquired reference.
     */
    bool isAcquired() const {
        return mAcquirable.use_count() > 1;
    }

    /**
     * Gets and returns a shared acquired reference.
     */
    std::shared_ptr<Acquirable> getAcquirable() {
        return mAcquirable;
    }

private:
    friend void GraphicBufferSource::discardBufferAtSlotIndex_l(ssize_t);

    /**
     * This method to be called when the buffer is no longer in the buffer cache.
     * Called from discardBufferAtSlotIndex_l.
     */
    void onDroppedFromCache() {
        CHECK_DBG(mIsCached);
        mIsCached = false;
    }

    bool mIsCached;
    slot_id mSlot;
    sp<GraphicBuffer> mGraphicBuffer;
    std::shared_ptr<Acquirable> mAcquirable;
};

/**
 * A copiable object managing a buffer acquired from the producer. This must always be a cached
 * buffer. This objects also manages its acquire fence and any release fences that may be returned
 * by the encoder for this buffer (this buffer may be queued to the encoder multiple times).
 * If no release fences are added by the encoder, the acquire fence is returned as the release
 * fence for this - as it is assumed that noone waited for the acquire fence. Otherwise, it is
 * assumed that the encoder has waited for the acquire fence (or returned it as the release
 * fence).
 */
struct GraphicBufferSource::AcquiredBuffer {
    AcquiredBuffer(
            const std::shared_ptr<CachedBuffer> &buffer,
            std::function<void(AcquiredBuffer *)> onReleased,
            const sp<Fence> &acquireFence)
        : mBuffer(buffer),
          mAcquirable(buffer->getAcquirable()),
          mAcquireFence(acquireFence),
          mGotReleaseFences(false),
          mOnReleased(onReleased) {
    }

    /**
     * Adds a release fence returned by the encoder to this object. If this is called with an
     * valid file descriptor, it is added to the list of release fences. These are returned to the
     * producer on release() as a merged fence. Regardless of the validity of the file descriptor,
     * we take note that a release fence was attempted to be added and the acquire fence can now be
     * assumed as acquired.
     */
    void addReleaseFenceFd(int fenceFd) {
        // save all release fences - these will be propagated to the producer if this buffer is
        // ever released to it
        if (fenceFd >= 0) {
            mReleaseFenceFds.push_back(fenceFd);
        }
        mGotReleaseFences = true;
    }

    /**
     * Returns the acquire fence file descriptor associated with this object.
     */
    int getAcquireFenceFd() {
        if (mAcquireFence == nullptr || !mAcquireFence->isValid()) {
            return -1;
        }
        return mAcquireFence->dup();
    }

    /**
     * Returns whether the buffer is still in the buffer cache.
     */
    bool isCached() const {
        return mBuffer->isCached();
    }

    /**
     * Returns the acquired buffer.
     */
    sp<GraphicBuffer> getGraphicBuffer() const {
        return mBuffer->getGraphicBuffer();
    }

    /**
     * Returns the slot that this buffer is cached at, or -1 otherwise.
     *
     * This assumes that -1 slot id is invalid; though, it is just a benign collision used for
     * debugging. This object explicitly manages whether it is still cached.
     */
    slot_id getSlot() const {
        return mBuffer->getSlot();
    }

    /**
     * Creates and returns a release fence object from the acquire fence and/or any release fences
     * added. If no release fences were added (even if invalid), returns the acquire fence.
     * Otherwise, it returns a merged fence from all the valid release fences added.
     */
    sp<Fence> getReleaseFence() {
        // If did not receive release fences, we assume this buffer was not consumed (it was
        // discarded or dropped). In this case release the acquire fence as the release fence.
        // We do this here to avoid a dup, close and recreation of the Fence object.
        if (!mGotReleaseFences) {
            return mAcquireFence;
        }
        sp<Fence> ret = getReleaseFence(0, mReleaseFenceFds.size());
        // clear fds as fence took ownership of them
        mReleaseFenceFds.clear();
        return ret;
    }

    // this video buffer is no longer referenced by the codec (or kept for later encoding)
    // it is now safe to release to the producer
    ~AcquiredBuffer() {
        //mAcquirable.clear();
        mOnReleased(this);
        // mOnRelease method should call getReleaseFence() that releases all fds but just in case
        ALOGW_IF(!mReleaseFenceFds.empty(), "release fences were not obtained, closing fds");
        for (int fildes : mReleaseFenceFds) {
            ::close(fildes);
            TRESPASS_DBG();
        }
    }

private:
    std::shared_ptr<GraphicBufferSource::CachedBuffer> mBuffer;
    std::shared_ptr<GraphicBufferSource::CachedBuffer::Acquirable> mAcquirable;
    sp<Fence> mAcquireFence;
    Vector<int> mReleaseFenceFds;
    bool mGotReleaseFences;
    std::function<void(AcquiredBuffer *)> mOnReleased;

    /**
     * Creates and returns a release fence from 0 or more release fence file descriptors in from
     * the specified range in the array.
     *
     * @param start start index
     * @param num   number of release fds to merge
     */
    sp<Fence> getReleaseFence(size_t start, size_t num) const {
        if (num == 0) {
            return Fence::NO_FENCE;
        } else if (num == 1) {
            return new Fence(mReleaseFenceFds[start]);
        } else {
            return Fence::merge("GBS::AB",
                                getReleaseFence(start, num >> 1),
                                getReleaseFence(start + (num >> 1), num - (num >> 1)));
        }
    }
};

struct GraphicBufferSource::ConsumerProxy : public BufferQueue::ConsumerListener {
    ConsumerProxy(const wp<GraphicBufferSource> &gbs) : mGbs(gbs) {}

    ~ConsumerProxy() = default;

    void onFrameAvailable(const BufferItem& item) override {
        sp<GraphicBufferSource> gbs = mGbs.promote();
        if (gbs != nullptr) {
            gbs->onFrameAvailable(item);
        }
    }

    void onBuffersReleased() override {
        sp<GraphicBufferSource> gbs = mGbs.promote();
        if (gbs != nullptr) {
            gbs->onBuffersReleased();
        }
    }

    void onSidebandStreamChanged() override {
        sp<GraphicBufferSource> gbs = mGbs.promote();
        if (gbs != nullptr) {
            gbs->onSidebandStreamChanged();
        }
    }

private:
    // Note that GraphicBufferSource is holding an sp to us, we can't hold
    // an sp back to GraphicBufferSource as the circular dependency will
    // make both immortal.
    wp<GraphicBufferSource> mGbs;
};

GraphicBufferSource::GraphicBufferSource() :
    mInitCheck(UNKNOWN_ERROR),
    mNumAvailableUnacquiredBuffers(0),
    mNumOutstandingAcquires(0),
    mEndOfStream(false),
    mEndOfStreamSent(false),
    mLastDataspace(HAL_DATASPACE_UNKNOWN),
    mExecuting(false),
    mSuspended(false),
    mLastFrameTimestampUs(-1),
    mStopTimeUs(-1),
    mLastActionTimeUs(-1LL),
    mSkipFramesBeforeNs(-1LL),
    mFrameRepeatIntervalUs(-1LL),
    mRepeatLastFrameGeneration(0),
    mOutstandingFrameRepeatCount(0),
    mFrameRepeatBlockedOnCodecBuffer(false),
    mFps(-1.0),
    mCaptureFps(-1.0),
    mBaseCaptureUs(-1LL),
    mBaseFrameUs(-1LL),
    mFrameCount(0),
    mPrevCaptureUs(-1LL),
    mPrevFrameUs(-1LL),
    mInputBufferTimeOffsetUs(0LL) {
    ALOGV("GraphicBufferSource");

    String8 name("GraphicBufferSource");

    BufferQueue::createBufferQueue(&mProducer, &mConsumer);
    mConsumer->setConsumerName(name);

    // create the consumer listener interface, and hold sp so that this
    // interface lives as long as the GraphicBufferSource.
    mConsumerProxy = new ConsumerProxy(this);

    sp<IConsumerListener> proxy =
            new BufferQueue::ProxyConsumerListener(mConsumerProxy);

    mInitCheck = mConsumer->consumerConnect(proxy, false);
    if (mInitCheck != NO_ERROR) {
        ALOGE("Error connecting to BufferQueue: %s (%d)",
                strerror(-mInitCheck), mInitCheck);
        return;
    }

    memset(&mDefaultColorAspectsPacked, 0, sizeof(mDefaultColorAspectsPacked));

    CHECK(mInitCheck == NO_ERROR);
}

GraphicBufferSource::~GraphicBufferSource() {
    ALOGV("~GraphicBufferSource");
    {
        // all acquired buffers must be freed with the mutex locked otherwise our debug assertion
        // may trigger
        Mutex::Autolock autoLock(mMutex);
        mAvailableBuffers.clear();
        mSubmittedCodecBuffers.clear();
        mLatestBuffer.mBuffer.reset();
    }

    if (mNumOutstandingAcquires != 0) {
        ALOGW("potential buffer leak: acquired=%d", mNumOutstandingAcquires);
        TRESPASS_DBG();
    }
    if (mConsumer != NULL) {
        status_t err = mConsumer->consumerDisconnect();
        if (err != NO_ERROR) {
            ALOGW("consumerDisconnect failed: %d", err);
        }
    }
}

sp<IGraphicBufferProducer> GraphicBufferSource::getIGraphicBufferProducer() const {
    return mProducer;
}

sp<::android::hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer>
GraphicBufferSource::getHGraphicBufferProducer_V1_0() const {
    using TWGraphicBufferProducer = ::android::TWGraphicBufferProducer<
        ::android::hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer>;

    return new TWGraphicBufferProducer(getIGraphicBufferProducer());
}

sp<::android::hardware::graphics::bufferqueue::V2_0::IGraphicBufferProducer>
GraphicBufferSource::getHGraphicBufferProducer() const {
    return new ::android::hardware::graphics::bufferqueue::V2_0::utils::
                    B2HGraphicBufferProducer(getIGraphicBufferProducer());
}

Status GraphicBufferSource::start() {
    Mutex::Autolock autoLock(mMutex);
    ALOGV("--> start; available=%zu, submittable=%zd",
            mAvailableBuffers.size(), mFreeCodecBuffers.size());
    CHECK(!mExecuting);
    mExecuting = true;
    mLastDataspace = HAL_DATASPACE_UNKNOWN;
    ALOGV("clearing last dataSpace");

    // Start by loading up as many buffers as possible.  We want to do this,
    // rather than just submit the first buffer, to avoid a degenerate case:
    // if all BQ buffers arrive before we start executing, and we only submit
    // one here, the other BQ buffers will just sit until we get notified
    // that the codec buffer has been released.  We'd then acquire and
    // submit a single additional buffer, repeatedly, never using more than
    // one codec buffer simultaneously.  (We could instead try to submit
    // all BQ buffers whenever any codec buffer is freed, but if we get the
    // initial conditions right that will never be useful.)
    while (haveAvailableBuffers_l()) {
        if (!fillCodecBuffer_l()) {
            ALOGV("stop load with available=%zu+%d",
                    mAvailableBuffers.size(), mNumAvailableUnacquiredBuffers);
            break;
        }
    }

    ALOGV("done loading initial frames, available=%zu+%d",
            mAvailableBuffers.size(), mNumAvailableUnacquiredBuffers);

    // If EOS has already been signaled, and there are no more frames to
    // submit, try to send EOS now as well.
    if (mStopTimeUs == -1 && mEndOfStream && !haveAvailableBuffers_l()) {
        submitEndOfInputStream_l();
    }

    if (mFrameRepeatIntervalUs > 0LL && mLooper == NULL) {
        mReflector = new AHandlerReflector<GraphicBufferSource>(this);

        mLooper = new ALooper;
        mLooper->registerHandler(mReflector);
        mLooper->start();

        if (mLatestBuffer.mBuffer != nullptr) {
            queueFrameRepeat_l();
        }
    }

    return Status::ok();
}

Status GraphicBufferSource::stop() {
    ALOGV("stop");

    Mutex::Autolock autoLock(mMutex);

    if (mExecuting) {
        // We are only interested in the transition from executing->idle,
        // not loaded->idle.
        mExecuting = false;
    }
    return Status::ok();
}

Status GraphicBufferSource::release(){
    sp<ALooper> looper;
    {
        Mutex::Autolock autoLock(mMutex);
        looper = mLooper;
        if (mLooper != NULL) {
            mLooper->unregisterHandler(mReflector->id());
            mReflector.clear();

            mLooper.clear();
        }

        ALOGV("--> release; available=%zu+%d eos=%d eosSent=%d acquired=%d",
                mAvailableBuffers.size(), mNumAvailableUnacquiredBuffers,
                mEndOfStream, mEndOfStreamSent, mNumOutstandingAcquires);

        // Codec is no longer executing.  Releasing all buffers to bq.
        mFreeCodecBuffers.clear();
        mSubmittedCodecBuffers.clear();
        mLatestBuffer.mBuffer.reset();
        mComponent.clear();
        mExecuting = false;
    }
    if (looper != NULL) {
        looper->stop();
    }
    return Status::ok();
}

Status GraphicBufferSource::onInputBufferAdded(codec_buffer_id bufferId) {
    Mutex::Autolock autoLock(mMutex);

    if (mExecuting) {
        // This should never happen -- buffers can only be allocated when
        // transitioning from "loaded" to "idle".
        ALOGE("addCodecBuffer: buffer added while executing");
        return Status::fromServiceSpecificError(INVALID_OPERATION);
    }

    ALOGV("addCodecBuffer: bufferId=%u", bufferId);

    mFreeCodecBuffers.push_back(bufferId);
    return Status::ok();
}

Status GraphicBufferSource::onInputBufferEmptied(codec_buffer_id bufferId, int fenceFd) {
    Mutex::Autolock autoLock(mMutex);
    FileDescriptor::Autoclose fence(fenceFd);

    ssize_t cbi = mSubmittedCodecBuffers.indexOfKey(bufferId);
    if (cbi < 0) {
        // This should never happen.
        ALOGE("onInputBufferEmptied: buffer not recognized (bufferId=%u)", bufferId);
        return Status::fromServiceSpecificError(BAD_VALUE);
    }

    std::shared_ptr<AcquiredBuffer> buffer = mSubmittedCodecBuffers.valueAt(cbi);

    // Move buffer to available buffers
    mSubmittedCodecBuffers.removeItemsAt(cbi);
    mFreeCodecBuffers.push_back(bufferId);

    // header->nFilledLen may not be the original value, so we can't compare
    // that to zero to see of this was the EOS buffer.  Instead we just
    // see if there is a null AcquiredBuffer, which should only ever happen for EOS.
    if (buffer == nullptr) {
        if (!(mEndOfStream && mEndOfStreamSent)) {
            // This can happen when broken code sends us the same buffer twice in a row.
            ALOGE("onInputBufferEmptied: non-EOS null buffer (bufferId=%u)", bufferId);
        } else {
            ALOGV("onInputBufferEmptied: EOS null buffer (bufferId=%u@%zd)", bufferId, cbi);
        }
        // No GraphicBuffer to deal with, no additional input or output is expected, so just return.
        return Status::fromServiceSpecificError(BAD_VALUE);
    }

    if (!mExecuting) {
        // this is fine since this could happen when going from Idle to Loaded
        ALOGV("onInputBufferEmptied: no longer executing (bufferId=%u@%zd)", bufferId, cbi);
        return Status::fromServiceSpecificError(OK);
    }

    ALOGV("onInputBufferEmptied: bufferId=%d@%zd [slot=%d, useCount=%ld, handle=%p] acquired=%d",
            bufferId, cbi, buffer->getSlot(), buffer.use_count(), buffer->getGraphicBuffer()->handle,
            mNumOutstandingAcquires);

    buffer->addReleaseFenceFd(fence.release());
    // release codec reference for video buffer just in case remove does not it
    buffer.reset();

    if (haveAvailableBuffers_l()) {
        // Fill this codec buffer.
        CHECK(!mEndOfStreamSent);
        ALOGV("onInputBufferEmptied: buffer freed, feeding codec (available=%zu+%d, eos=%d)",
                mAvailableBuffers.size(), mNumAvailableUnacquiredBuffers, mEndOfStream);
        fillCodecBuffer_l();
    } else if (mEndOfStream && mStopTimeUs == -1) {
        // No frames available, but EOS is pending and no stop time, so use this buffer to
        // send that.
        ALOGV("onInputBufferEmptied: buffer freed, submitting EOS");
        submitEndOfInputStream_l();
    } else if (mFrameRepeatBlockedOnCodecBuffer) {
        bool success = repeatLatestBuffer_l();
        ALOGV("onInputBufferEmptied: completing deferred repeatLatestBuffer_l %s",
                success ? "SUCCESS" : "FAILURE");
        mFrameRepeatBlockedOnCodecBuffer = false;
    }

    // releaseReleasableBuffers_l();
    return Status::ok();
}

void GraphicBufferSource::onDataspaceChanged_l(
        android_dataspace dataspace, android_pixel_format pixelFormat) {
    ALOGD("got buffer with new dataSpace #%x", dataspace);
    mLastDataspace = dataspace;

    if (ColorUtils::convertDataSpaceToV0(dataspace)) {
        mComponent->dispatchDataSpaceChanged(
                mLastDataspace, mDefaultColorAspectsPacked, pixelFormat);
    }
}

bool GraphicBufferSource::fillCodecBuffer_l() {
    CHECK(mExecuting && haveAvailableBuffers_l());

    if (mFreeCodecBuffers.empty()) {
        // No buffers available, bail.
        ALOGV("fillCodecBuffer_l: no codec buffers, available=%zu+%d",
                mAvailableBuffers.size(), mNumAvailableUnacquiredBuffers);
        return false;
    }

    VideoBuffer item;
    if (mAvailableBuffers.empty()) {
        ALOGV("fillCodecBuffer_l: acquiring available buffer, available=%zu+%d",
                mAvailableBuffers.size(), mNumAvailableUnacquiredBuffers);
        if (acquireBuffer_l(&item) != OK) {
            ALOGE("fillCodecBuffer_l: failed to acquire available buffer");
            return false;
        }
    } else {
        ALOGV("fillCodecBuffer_l: getting available buffer, available=%zu+%d",
                mAvailableBuffers.size(), mNumAvailableUnacquiredBuffers);
        item = *mAvailableBuffers.begin();
        mAvailableBuffers.erase(mAvailableBuffers.begin());
    }

    int64_t itemTimeUs = item.mTimestampNs / 1000;

    // Process ActionItem in the Queue if there is any. If a buffer's timestamp
    // is smaller than the first action's timestamp, no action need to be performed.
    // If buffer's timestamp is larger or equal than the last action's timestamp,
    // only the last action needs to be performed as all the acitions before the
    // the action are overridden by the last action. For the other cases, traverse
    // the Queue to find the newest action that with timestamp smaller or equal to
    // the buffer's timestamp. For example, an action queue like
    // [pause 1us], [resume 2us], [pause 3us], [resume 4us], [pause 5us].... Upon
    // receiving a buffer with timestamp 3.5us, only the action [pause, 3us] needs
    // to be handled and [pause, 1us], [resume 2us] will be discarded.
    bool done = false;
    bool seeStopAction = false;
    if (!mActionQueue.empty()) {
        // First scan to check if bufferTimestamp is smaller than first action's timestamp.
        ActionItem nextAction = *(mActionQueue.begin());
        if (itemTimeUs < nextAction.mActionTimeUs) {
            ALOGV("No action. buffer timestamp %lld us < action timestamp: %lld us",
                (long long)itemTimeUs, (long long)nextAction.mActionTimeUs);
            // All the actions are ahead. No action need to perform now.
            // Release the buffer if is in suspended state, or process the buffer
            // if not in suspended state.
            done = true;
        }

        if (!done) {
            // Find the newest action that with timestamp smaller than itemTimeUs. Then
            // remove all the actions before and include the newest action.
            List<ActionItem>::iterator it = mActionQueue.begin();
            while (it != mActionQueue.end() && it->mActionTimeUs <= itemTimeUs
                    && nextAction.mAction != ActionItem::STOP) {
                nextAction = *it;
                ++it;
            }
            mActionQueue.erase(mActionQueue.begin(), it);

            CHECK(itemTimeUs >= nextAction.mActionTimeUs);
            switch (nextAction.mAction) {
                case ActionItem::PAUSE:
                {
                    mSuspended = true;
                    ALOGV("RUNNING/PAUSE -> PAUSE at buffer %lld us  PAUSE Time: %lld us",
                            (long long)itemTimeUs, (long long)nextAction.mActionTimeUs);
                    break;
                }
                case ActionItem::RESUME:
                {
                    mSuspended = false;
                    ALOGV("PAUSE/RUNNING -> RUNNING at buffer %lld us  RESUME Time: %lld us",
                            (long long)itemTimeUs, (long long)nextAction.mActionTimeUs);
                    break;
                }
                case ActionItem::STOP:
                {
                    ALOGV("RUNNING/PAUSE -> STOP at buffer %lld us  STOP Time: %lld us",
                            (long long)itemTimeUs, (long long)nextAction.mActionTimeUs);
                    // Clear the whole ActionQueue as recording is done
                    mActionQueue.clear();
                    seeStopAction = true;
                    break;
                }
                default:
                    TRESPASS_DBG("Unknown action type");
                    // return true here because we did consume an available buffer, so the
                    // loop in start will eventually terminate even if we hit this.
                    return false;
            }
        }
    }

    if (seeStopAction) {
        // Clear all the buffers before setting mEndOfStream and signal EndOfInputStream.
        releaseAllAvailableBuffers_l();
        mEndOfStream = true;
        submitEndOfInputStream_l();
        return true;
    }

    if (mSuspended) {
        return true;
    }

    int err = UNKNOWN_ERROR;

    // only submit sample if start time is unspecified, or sample
    // is queued after the specified start time
    if (mSkipFramesBeforeNs < 0LL || item.mTimestampNs >= mSkipFramesBeforeNs) {
        // if start time is set, offset time stamp by start time
        if (mSkipFramesBeforeNs > 0) {
            item.mTimestampNs -= mSkipFramesBeforeNs;
        }

        int64_t timeUs = item.mTimestampNs / 1000;
        if (mFrameDropper != NULL && mFrameDropper->shouldDrop(timeUs)) {
            ALOGV("skipping frame (%lld) to meet max framerate", static_cast<long long>(timeUs));
            // set err to OK so that the skipped frame can still be saved as the lastest frame
            err = OK;
        } else {
            err = submitBuffer_l(item); // this takes shared ownership of the acquired buffer on succeess
        }
    }

    if (err != OK) {
        ALOGV("submitBuffer_l failed, will release bq slot %d", item.mBuffer->getSlot());
        return true;
    } else {
        // Don't set the last buffer id if we're not repeating,
        // we'll be holding on to the last buffer for nothing.
        if (mFrameRepeatIntervalUs > 0LL) {
            setLatestBuffer_l(item);
        }
        ALOGV("buffer submitted [slot=%d, useCount=%ld] acquired=%d",
                item.mBuffer->getSlot(), item.mBuffer.use_count(), mNumOutstandingAcquires);
        mLastFrameTimestampUs = itemTimeUs;
    }

    return true;
}

bool GraphicBufferSource::repeatLatestBuffer_l() {
    CHECK(mExecuting && !haveAvailableBuffers_l());

    if (mLatestBuffer.mBuffer == nullptr || mSuspended) {
        return false;
    }

    if (mFreeCodecBuffers.empty()) {
        // No buffers available, bail.
        ALOGV("repeatLatestBuffer_l: no codec buffers.");
        return false;
    }

    if (!mLatestBuffer.mBuffer->isCached()) {
        ALOGV("repeatLatestBuffer_l: slot was discarded, but repeating our own reference");
    }

    // it is ok to update the timestamp of latest buffer as it is only used for submission
    status_t err = submitBuffer_l(mLatestBuffer);
    if (err != OK) {
        return false;
    }

    /* repeat last frame up to kRepeatLastFrameCount times.
     * in case of static scene, a single repeat might not get rid of encoder
     * ghosting completely, refresh a couple more times to get better quality
     */
    if (--mOutstandingFrameRepeatCount > 0) {
        // set up timestamp for repeat frame
        mLatestBuffer.mTimestampNs += mFrameRepeatIntervalUs * 1000;
        queueFrameRepeat_l();
    }

    return true;
}

void GraphicBufferSource::setLatestBuffer_l(const VideoBuffer &item) {
    mLatestBuffer = item;

    ALOGV("setLatestBuffer_l: [slot=%d, useCount=%ld]",
            mLatestBuffer.mBuffer->getSlot(), mLatestBuffer.mBuffer.use_count());

    mOutstandingFrameRepeatCount = kRepeatLastFrameCount;
    // set up timestamp for repeat frame
    mLatestBuffer.mTimestampNs += mFrameRepeatIntervalUs * 1000;
    queueFrameRepeat_l();
}

void GraphicBufferSource::queueFrameRepeat_l() {
    mFrameRepeatBlockedOnCodecBuffer = false;

    if (mReflector != NULL) {
        sp<AMessage> msg = new AMessage(kWhatRepeatLastFrame, mReflector);
        msg->setInt32("generation", ++mRepeatLastFrameGeneration);
        msg->post(mFrameRepeatIntervalUs);
    }
}

bool GraphicBufferSource::calculateCodecTimestamp_l(
        nsecs_t bufferTimeNs, int64_t *codecTimeUs) {
    int64_t timeUs = bufferTimeNs / 1000;
    timeUs += mInputBufferTimeOffsetUs;

    if (mCaptureFps > 0.
            && (mFps > 2 * mCaptureFps
            || mCaptureFps > 2 * mFps)) {
        // Time lapse or slow motion mode
        if (mPrevCaptureUs < 0LL) {
            // first capture
            mPrevCaptureUs = mBaseCaptureUs = timeUs;
            // adjust the first sample timestamp.
            mPrevFrameUs = mBaseFrameUs =
                    std::llround((timeUs * mCaptureFps) / mFps);
            mFrameCount = 0;
        } else {
            // snap to nearest capture point
            double nFrames = (timeUs - mPrevCaptureUs) * mCaptureFps / 1000000;
            if (nFrames < 0.5 - kTimestampFluctuation) {
                // skip this frame as it's too close to previous capture
                ALOGD("skipping frame, timeUs %lld", static_cast<long long>(timeUs));
                return false;
            }
            if (nFrames <= 1.0) {
                nFrames = 1.0;
            }
            mFrameCount += std::llround(nFrames);
            mPrevCaptureUs = mBaseCaptureUs + std::llround(
                    mFrameCount * 1000000 / mCaptureFps);
            mPrevFrameUs = mBaseFrameUs + std::llround(
                    mFrameCount * 1000000 / mFps);
        }

        ALOGV("timeUs %lld, captureUs %lld, frameUs %lld",
                static_cast<long long>(timeUs),
                static_cast<long long>(mPrevCaptureUs),
                static_cast<long long>(mPrevFrameUs));
    } else {
        if (timeUs <= mPrevFrameUs) {
            if (mFrameDropper != NULL && mFrameDropper->disabled()) {
                // Warn only, client has disabled frame drop logic possibly for image
                // encoding cases where camera's ZSL mode could send out of order frames.
                ALOGW("Received frame that's going backward in time");
            } else {
                // Drop the frame if it's going backward in time. Bad timestamp
                // could disrupt encoder's rate control completely.
                ALOGW("Dropping frame that's going backward in time");
                return false;
            }
        }

        mPrevFrameUs = timeUs;
    }

    *codecTimeUs = mPrevFrameUs;
    return true;
}

status_t GraphicBufferSource::submitBuffer_l(const VideoBuffer &item) {
    CHECK(!mFreeCodecBuffers.empty());
    uint32_t codecBufferId = *mFreeCodecBuffers.begin();

    ALOGV("submitBuffer_l [slot=%d, bufferId=%d]", item.mBuffer->getSlot(), codecBufferId);

    int64_t codecTimeUs;
    if (!calculateCodecTimestamp_l(item.mTimestampNs, &codecTimeUs)) {
        return UNKNOWN_ERROR;
    }

    if ((android_dataspace)item.mDataspace != mLastDataspace) {
        onDataspaceChanged_l(
                item.mDataspace,
                (android_pixel_format)item.mBuffer->getGraphicBuffer()->format);
    }

    std::shared_ptr<AcquiredBuffer> buffer = item.mBuffer;
    // use a GraphicBuffer for now as component is using GraphicBuffers to hold references
    // and it requires this graphic buffer to be able to hold its reference
    // and thus we would need to create a new GraphicBuffer from an ANWBuffer separate from the
    // acquired GraphicBuffer.
    // TODO: this can be reworked globally to use ANWBuffer references
    sp<GraphicBuffer> graphicBuffer = buffer->getGraphicBuffer();
    status_t err = mComponent->submitBuffer(
            codecBufferId, graphicBuffer, codecTimeUs, buffer->getAcquireFenceFd());

    if (err != OK) {
        ALOGW("WARNING: emptyGraphicBuffer failed: 0x%x", err);
        return err;
    }

    mFreeCodecBuffers.erase(mFreeCodecBuffers.begin());

    ssize_t cbix = mSubmittedCodecBuffers.add(codecBufferId, buffer);
    ALOGV("emptyGraphicBuffer succeeded, bufferId=%u@%zd bufhandle=%p",
            codecBufferId, cbix, graphicBuffer->handle);
    return OK;
}

void GraphicBufferSource::submitEndOfInputStream_l() {
    CHECK(mEndOfStream);
    if (mEndOfStreamSent) {
        ALOGV("EOS already sent");
        return;
    }

    if (mFreeCodecBuffers.empty()) {
        ALOGV("submitEndOfInputStream_l: no codec buffers available");
        return;
    }
    uint32_t codecBufferId = *mFreeCodecBuffers.begin();

    // We reject any additional incoming graphic buffers. There is no acquired buffer used for EOS
    status_t err = mComponent->submitEos(codecBufferId);
    if (err != OK) {
        ALOGW("emptyDirectBuffer EOS failed: 0x%x", err);
    } else {
        mFreeCodecBuffers.erase(mFreeCodecBuffers.begin());
        ssize_t cbix = mSubmittedCodecBuffers.add(codecBufferId, nullptr);
        ALOGV("submitEndOfInputStream_l: buffer submitted, bufferId=%u@%zd", codecBufferId, cbix);
        mEndOfStreamSent = true;

        // no need to hold onto any buffers for frame repeating
        ++mRepeatLastFrameGeneration;
        mLatestBuffer.mBuffer.reset();
    }
}

status_t GraphicBufferSource::acquireBuffer_l(VideoBuffer *ab) {
    BufferItem bi;
    status_t err = mConsumer->acquireBuffer(&bi, 0);
    if (err == BufferQueue::NO_BUFFER_AVAILABLE) {
        // shouldn't happen
        ALOGW("acquireBuffer_l: frame was not available");
        return err;
    } else if (err != OK) {
        ALOGW("acquireBuffer_l: failed with err=%d", err);
        return err;
    }
    --mNumAvailableUnacquiredBuffers;

    // Manage our buffer cache.
    std::shared_ptr<CachedBuffer> buffer;
    ssize_t bsi = mBufferSlots.indexOfKey(bi.mSlot);
    if (bi.mGraphicBuffer != NULL) {
        // replace/initialize slot with new buffer
        ALOGV("acquireBuffer_l: %s buffer slot %d", bsi < 0 ? "setting" : "UPDATING", bi.mSlot);
        if (bsi >= 0) {
            discardBufferAtSlotIndex_l(bsi);
        } else {
            bsi = mBufferSlots.add(bi.mSlot, nullptr);
        }
        buffer = std::make_shared<CachedBuffer>(bi.mSlot, bi.mGraphicBuffer);
        mBufferSlots.replaceValueAt(bsi, buffer);
    } else {
        buffer = mBufferSlots.valueAt(bsi);
    }
    int64_t frameNum = bi.mFrameNumber;

    std::shared_ptr<AcquiredBuffer> acquiredBuffer =
        std::make_shared<AcquiredBuffer>(
                buffer,
                [frameNum, this](AcquiredBuffer *buffer){
                    // AcquiredBuffer's destructor should always be called when mMutex is locked.
                    // If we had a reentrant mutex, we could just lock it again to ensure this.
                    if (mMutex.tryLock() == 0) {
                        TRESPASS_DBG();
                        mMutex.unlock();
                    }

                    // we can release buffers immediately if not using adapters
                    // alternately, we could add them to mSlotsToRelease, but we would
                    // somehow need to propagate frame number to that queue
                    if (buffer->isCached()) {
                        --mNumOutstandingAcquires;
                        mConsumer->releaseBuffer(
                                buffer->getSlot(), frameNum, EGL_NO_DISPLAY, EGL_NO_SYNC_KHR,
                                buffer->getReleaseFence());
                    }
                },
                bi.mFence);
    VideoBuffer videoBuffer{acquiredBuffer, bi.mTimestamp, bi.mDataSpace};
    *ab = videoBuffer;
    ++mNumOutstandingAcquires;
    return OK;
}

// BufferQueue::ConsumerListener callback
void GraphicBufferSource::onFrameAvailable(const BufferItem& item __unused) {
    Mutex::Autolock autoLock(mMutex);

    ALOGV("onFrameAvailable: executing=%d available=%zu+%d",
            mExecuting, mAvailableBuffers.size(), mNumAvailableUnacquiredBuffers);
    ++mNumAvailableUnacquiredBuffers;

    // For BufferQueue we cannot acquire a buffer if we cannot immediately feed it to the codec
    // UNLESS we are discarding this buffer (acquiring and immediately releasing it), which makes
    // this an ugly logic.
    // NOTE: We could also rely on our debug counter but that is meant only as a debug counter.
    if (!areWeDiscardingAvailableBuffers_l() && mFreeCodecBuffers.empty()) {
        // we may not be allowed to acquire a possibly encodable buffer, so just note that
        // it is available
        ALOGV("onFrameAvailable: cannot acquire buffer right now, do it later");

        ++mRepeatLastFrameGeneration; // cancel any pending frame repeat
        return;
    }

    VideoBuffer buffer;
    status_t err = acquireBuffer_l(&buffer);
    if (err != OK) {
        ALOGE("onFrameAvailable: acquireBuffer returned err=%d", err);
    } else {
        onBufferAcquired_l(buffer);
    }
}

bool GraphicBufferSource::areWeDiscardingAvailableBuffers_l() {
    return mEndOfStreamSent // already sent EOS to codec
            || mComponent == nullptr // there is no codec connected
            || (mSuspended && mActionQueue.empty()) // we are suspended and not waiting for
                                                    // any further action
            || !mExecuting;
}

void GraphicBufferSource::onBufferAcquired_l(const VideoBuffer &buffer) {
    if (mEndOfStreamSent) {
        // This should only be possible if a new buffer was queued after
        // EOS was signaled, i.e. the app is misbehaving.
        ALOGW("onFrameAvailable: EOS is sent, ignoring frame");
    } else if (mComponent == NULL || (mSuspended && mActionQueue.empty())) {
        // FIXME: if we are suspended but have a resume queued we will stop repeating the last
        // frame. Is that the desired behavior?
        ALOGV("onFrameAvailable: suspended, ignoring frame");
    } else {
        ++mRepeatLastFrameGeneration; // cancel any pending frame repeat
        mAvailableBuffers.push_back(buffer);
        if (mExecuting) {
            fillCodecBuffer_l();
        }
    }
}

// BufferQueue::ConsumerListener callback
void GraphicBufferSource::onBuffersReleased() {
    Mutex::Autolock lock(mMutex);

    uint64_t slotMask;
    uint64_t releaseMask;
    if (mConsumer->getReleasedBuffers(&releaseMask) != NO_ERROR) {
        slotMask = 0xffffffffffffffffULL;
        ALOGW("onBuffersReleased: unable to get released buffer set");
    } else {
        slotMask = releaseMask;
        ALOGV("onBuffersReleased: 0x%016" PRIx64, slotMask);
    }

    AString unpopulated;
    for (int i = 0; i < BufferQueue::NUM_BUFFER_SLOTS; i++) {
        if ((slotMask & 0x01) != 0) {
            if (!discardBufferInSlot_l(i)) {
                if (!unpopulated.empty()) {
                    unpopulated.append(", ");
                }
                unpopulated.append(i);
            }
        }
        slotMask >>= 1;
    }
    if (!unpopulated.empty()) {
        ALOGW("released unpopulated slots: [%s]", unpopulated.c_str());
    }
}

bool GraphicBufferSource::discardBufferInSlot_l(GraphicBufferSource::slot_id i) {
    ssize_t bsi = mBufferSlots.indexOfKey(i);
    if (bsi < 0) {
        return false;
    } else {
        discardBufferAtSlotIndex_l(bsi);
        mBufferSlots.removeItemsAt(bsi);
        return true;
    }
}

void GraphicBufferSource::discardBufferAtSlotIndex_l(ssize_t bsi) {
    const std::shared_ptr<CachedBuffer>& buffer = mBufferSlots.valueAt(bsi);
    // use -2 if there is no latest buffer, and -1 if it is no longer cached
    slot_id latestBufferSlot =
        mLatestBuffer.mBuffer == nullptr ? -2 : mLatestBuffer.mBuffer->getSlot();
    ALOGV("releasing acquired buffer: [slot=%d, useCount=%ld], latest: [slot=%d]",
            mBufferSlots.keyAt(bsi), buffer.use_count(), latestBufferSlot);
    mBufferSlots.valueAt(bsi)->onDroppedFromCache();

    // If the slot of an acquired buffer is discarded, that buffer will not have to be
    // released to the producer, so account it here. However, it is possible that the
    // acquired buffer has already been discarded so check if it still is.
    if (buffer->isAcquired()) {
        --mNumOutstandingAcquires;
    }

    // clear the buffer reference (not technically needed as caller either replaces or deletes
    // it; done here for safety).
    mBufferSlots.editValueAt(bsi).reset();
    CHECK_DBG(buffer == nullptr);
}

void GraphicBufferSource::releaseAllAvailableBuffers_l() {
    mAvailableBuffers.clear();
    while (mNumAvailableUnacquiredBuffers > 0) {
        VideoBuffer item;
        if (acquireBuffer_l(&item) != OK) {
            ALOGW("releaseAllAvailableBuffers: failed to acquire available unacquired buffer");
            break;
        }
    }
}

// BufferQueue::ConsumerListener callback
void GraphicBufferSource::onSidebandStreamChanged() {
    ALOG_ASSERT(false, "GraphicBufferSource can't consume sideband streams");
}

status_t GraphicBufferSource::configure(
        const sp<ComponentWrapper>& component,
        int32_t dataSpace,
        int32_t bufferCount,
        uint32_t frameWidth,
        uint32_t frameHeight,
        uint32_t consumerUsage) {
    if (component == NULL) {
        return BAD_VALUE;
    }


    // Call setMaxAcquiredBufferCount without lock.
    // setMaxAcquiredBufferCount could call back to onBuffersReleased
    // if the buffer count change results in releasing of existing buffers,
    // which would lead to deadlock.
    status_t err = mConsumer->setMaxAcquiredBufferCount(bufferCount);
    if (err != NO_ERROR) {
        ALOGE("Unable to set BQ max acquired buffer count to %u: %d",
                bufferCount, err);
        return err;
    }

    {
        Mutex::Autolock autoLock(mMutex);
        mComponent = component;

        err = mConsumer->setDefaultBufferSize(frameWidth, frameHeight);
        if (err != NO_ERROR) {
            ALOGE("Unable to set BQ default buffer size to %ux%u: %d",
                    frameWidth, frameHeight, err);
            return err;
        }

        consumerUsage |= GRALLOC_USAGE_HW_VIDEO_ENCODER;
        mConsumer->setConsumerUsageBits(consumerUsage);

        // Set impl. defined format as default. Depending on the usage flags
        // the device-specific implementation will derive the exact format.
        err = mConsumer->setDefaultBufferFormat(HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED);
        if (err != NO_ERROR) {
            ALOGE("Failed to configure surface default format ret: %d", err);
            return err;
        }

        // Sets the default buffer data space
        ALOGD("setting dataspace: %#x, acquired=%d", dataSpace, mNumOutstandingAcquires);
        mConsumer->setDefaultBufferDataSpace((android_dataspace)dataSpace);
        mLastDataspace = (android_dataspace)dataSpace;

        mExecuting = false;
        mSuspended = false;
        mEndOfStream = false;
        mEndOfStreamSent = false;
        mSkipFramesBeforeNs = -1LL;
        mFrameDropper.clear();
        mFrameRepeatIntervalUs = -1LL;
        mRepeatLastFrameGeneration = 0;
        mOutstandingFrameRepeatCount = 0;
        mLatestBuffer.mBuffer.reset();
        mFrameRepeatBlockedOnCodecBuffer = false;
        mFps = -1.0;
        mCaptureFps = -1.0;
        mBaseCaptureUs = -1LL;
        mBaseFrameUs = -1LL;
        mPrevCaptureUs = -1LL;
        mPrevFrameUs = -1LL;
        mFrameCount = 0;
        mInputBufferTimeOffsetUs = 0;
        mStopTimeUs = -1;
        mActionQueue.clear();
    }

    return OK;
}

status_t GraphicBufferSource::setSuspend(bool suspend, int64_t suspendStartTimeUs) {
    ALOGV("setSuspend=%d at time %lld us", suspend, (long long)suspendStartTimeUs);

    Mutex::Autolock autoLock(mMutex);

    if (mStopTimeUs != -1) {
        ALOGE("setSuspend failed as STOP action is pending");
        return INVALID_OPERATION;
    }

    // Push the action to the queue.
    if (suspendStartTimeUs != -1) {
        // suspendStartTimeUs must be smaller or equal to current systemTime.
        int64_t currentSystemTimeUs = systemTime() / 1000;
        if (suspendStartTimeUs > currentSystemTimeUs) {
            ALOGE("setSuspend failed. %lld is larger than current system time %lld us",
                    (long long)suspendStartTimeUs, (long long)currentSystemTimeUs);
            return INVALID_OPERATION;
        }
        if (mLastActionTimeUs != -1 && suspendStartTimeUs < mLastActionTimeUs) {
            ALOGE("setSuspend failed. %lld is smaller than last action time %lld us",
                    (long long)suspendStartTimeUs, (long long)mLastActionTimeUs);
            return INVALID_OPERATION;
        }
        mLastActionTimeUs = suspendStartTimeUs;
        ActionItem action;
        action.mAction = suspend ? ActionItem::PAUSE : ActionItem::RESUME;
        action.mActionTimeUs = suspendStartTimeUs;
        ALOGV("Push %s action into actionQueue", suspend ? "PAUSE" : "RESUME");
        mActionQueue.push_back(action);
    } else {
        if (suspend) {
            mSuspended = true;
            releaseAllAvailableBuffers_l();
            return OK;
        } else {
            mSuspended = false;
            if (mExecuting && !haveAvailableBuffers_l()
                    && mFrameRepeatBlockedOnCodecBuffer) {
                if (repeatLatestBuffer_l()) {
                    ALOGV("suspend/deferred repeatLatestBuffer_l SUCCESS");
                    mFrameRepeatBlockedOnCodecBuffer = false;
                } else {
                    ALOGV("suspend/deferred repeatLatestBuffer_l FAILURE");
                }
            }
        }
    }
    return OK;
}

status_t GraphicBufferSource::setRepeatPreviousFrameDelayUs(int64_t repeatAfterUs) {
    ALOGV("setRepeatPreviousFrameDelayUs: delayUs=%lld", (long long)repeatAfterUs);

    Mutex::Autolock autoLock(mMutex);

    if (mExecuting || repeatAfterUs <= 0LL) {
        return INVALID_OPERATION;
    }

    mFrameRepeatIntervalUs = repeatAfterUs;
    return OK;
}

status_t GraphicBufferSource::setTimeOffsetUs(int64_t timeOffsetUs) {
    Mutex::Autolock autoLock(mMutex);

    // timeOffsetUs must be negative for adjustment.
    if (timeOffsetUs >= 0LL) {
        return INVALID_OPERATION;
    }

    mInputBufferTimeOffsetUs = timeOffsetUs;
    return OK;
}

status_t GraphicBufferSource::setMaxFps(float maxFps) {
    ALOGV("setMaxFps: maxFps=%lld", (long long)maxFps);

    Mutex::Autolock autoLock(mMutex);

    if (mExecuting) {
        return INVALID_OPERATION;
    }

    mFrameDropper = new FrameDropper();
    status_t err = mFrameDropper->setMaxFrameRate(maxFps);
    if (err != OK) {
        mFrameDropper.clear();
        return err;
    }

    return OK;
}

status_t GraphicBufferSource::setStartTimeUs(int64_t skipFramesBeforeUs) {
    ALOGV("setStartTimeUs: skipFramesBeforeUs=%lld", (long long)skipFramesBeforeUs);

    Mutex::Autolock autoLock(mMutex);

    mSkipFramesBeforeNs =
            (skipFramesBeforeUs > 0 && skipFramesBeforeUs <= INT64_MAX / 1000) ?
            (skipFramesBeforeUs * 1000) : -1LL;

    return OK;
}

status_t GraphicBufferSource::setStopTimeUs(int64_t stopTimeUs) {
    ALOGV("setStopTimeUs: %lld us", (long long)stopTimeUs);
    Mutex::Autolock autoLock(mMutex);

    if (mStopTimeUs != -1) {
        // Ignore if stop time has already been set
        return OK;
    }

    // stopTimeUs must be smaller or equal to current systemTime.
    int64_t currentSystemTimeUs = systemTime() / 1000;
    if (stopTimeUs > currentSystemTimeUs) {
        ALOGE("setStopTimeUs failed. %lld is larger than current system time %lld us",
            (long long)stopTimeUs, (long long)currentSystemTimeUs);
        return INVALID_OPERATION;
    }
    if (mLastActionTimeUs != -1 && stopTimeUs < mLastActionTimeUs) {
        ALOGE("setSuspend failed. %lld is smaller than last action time %lld us",
            (long long)stopTimeUs, (long long)mLastActionTimeUs);
        return INVALID_OPERATION;
    }
    mLastActionTimeUs = stopTimeUs;
    ActionItem action;
    action.mAction = ActionItem::STOP;
    action.mActionTimeUs = stopTimeUs;
    mActionQueue.push_back(action);
    mStopTimeUs = stopTimeUs;
    return OK;
}

status_t GraphicBufferSource::getStopTimeOffsetUs(int64_t *stopTimeOffsetUs) {
    ALOGV("getStopTimeOffsetUs");
    Mutex::Autolock autoLock(mMutex);
    if (mStopTimeUs == -1) {
        ALOGW("Fail to return stopTimeOffsetUs as stop time is not set");
        return INVALID_OPERATION;
    }
    *stopTimeOffsetUs =
        mLastFrameTimestampUs == -1 ? 0 : mStopTimeUs - mLastFrameTimestampUs;
    return OK;
}

status_t GraphicBufferSource::setTimeLapseConfig(double fps, double captureFps) {
    ALOGV("setTimeLapseConfig: fps=%lg, captureFps=%lg",
            fps, captureFps);
    Mutex::Autolock autoLock(mMutex);

    if (mExecuting || !(fps > 0) || !(captureFps > 0)) {
        return INVALID_OPERATION;
    }

    mFps = fps;
    mCaptureFps = captureFps;

    return OK;
}

status_t GraphicBufferSource::setColorAspects(int32_t aspectsPacked) {
    Mutex::Autolock autoLock(mMutex);
    mDefaultColorAspectsPacked = aspectsPacked;
    ColorAspects colorAspects = ColorUtils::unpackToColorAspects(aspectsPacked);
    ALOGD("requesting color aspects (R:%d(%s), P:%d(%s), M:%d(%s), T:%d(%s))",
            colorAspects.mRange, asString(colorAspects.mRange),
            colorAspects.mPrimaries, asString(colorAspects.mPrimaries),
            colorAspects.mMatrixCoeffs, asString(colorAspects.mMatrixCoeffs),
            colorAspects.mTransfer, asString(colorAspects.mTransfer));

    return OK;
}

status_t GraphicBufferSource::signalEndOfInputStream() {
    Mutex::Autolock autoLock(mMutex);
    ALOGV("signalEndOfInputStream: executing=%d available=%zu+%d eos=%d",
            mExecuting, mAvailableBuffers.size(), mNumAvailableUnacquiredBuffers, mEndOfStream);

    if (mEndOfStream) {
        ALOGE("EOS was already signaled");
        return INVALID_OPERATION;
    }

    // Set the end-of-stream flag.  If no frames are pending from the
    // BufferQueue, and a codec buffer is available, and we're executing,
    // and there is no stop timestamp, we initiate the EOS from here.
    // Otherwise, we'll let codecBufferEmptied() (or start) do it.
    //
    // Note: if there are no pending frames and all codec buffers are
    // available, we *must* submit the EOS from here or we'll just
    // stall since no future events are expected.
    mEndOfStream = true;

    if (mStopTimeUs == -1 && mExecuting && !haveAvailableBuffers_l()) {
        submitEndOfInputStream_l();
    }

    return OK;
}

void GraphicBufferSource::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatRepeatLastFrame:
        {
            Mutex::Autolock autoLock(mMutex);

            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));

            if (generation != mRepeatLastFrameGeneration) {
                // stale
                break;
            }

            if (!mExecuting || haveAvailableBuffers_l()) {
                break;
            }

            bool success = repeatLatestBuffer_l();
            if (success) {
                ALOGV("repeatLatestBuffer_l SUCCESS");
            } else {
                ALOGV("repeatLatestBuffer_l FAILURE");
                mFrameRepeatBlockedOnCodecBuffer = true;
            }
            break;
        }

        default:
            TRESPASS();
    }
}

}  // namespace android
