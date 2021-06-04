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

#ifndef ANDROID_GUI_RINGBUFFERCONSUMER_H
#define ANDROID_GUI_RINGBUFFERCONSUMER_H

#include <gui/BufferItem.h>
#include <gui/ConsumerBase.h>
#include <gui/BufferQueue.h>

#include <utils/List.h>

#define ANDROID_GRAPHICS_RINGBUFFERCONSUMER_JNI_ID "mRingBufferConsumer"

namespace android {

class String8;

/**
 * The RingBufferConsumer maintains a ring buffer of BufferItem objects,
 * (which are 'acquired' as long as they are part of the ring buffer, and
 *  'released' when they leave the ring buffer).
 *
 * When new buffers are produced, the oldest non-pinned buffer item is immediately
 * dropped from the ring buffer, and overridden with the newest buffer.
 *
 * Users can only access a buffer item after pinning it (which also guarantees
 * that during its duration it will not be released back into the BufferQueue).
 *
 * Note that the 'oldest' buffer is the one with the smallest timestamp.
 *
 * Edge cases:
 *  - If ringbuffer is not full, no drops occur when a buffer is produced.
 *  - If all the buffers get filled or pinned then there will be no empty
 *    buffers left, so the producer will block on dequeue.
 */
class RingBufferConsumer : public ConsumerBase,
                           public ConsumerBase::FrameAvailableListener
{
  public:
    typedef ConsumerBase::FrameAvailableListener FrameAvailableListener;

    enum { INVALID_BUFFER_SLOT = BufferQueue::INVALID_BUFFER_SLOT };
    enum { NO_BUFFER_AVAILABLE = BufferQueue::NO_BUFFER_AVAILABLE };

    // Create a new ring buffer consumer. The consumerUsage parameter determines
    // the consumer usage flags passed to the graphics allocator. The
    // bufferCount parameter specifies how many buffers can be pinned for user
    // access at the same time.
    RingBufferConsumer(const sp<IGraphicBufferConsumer>& consumer, uint64_t consumerUsage,
            int bufferCount);

    virtual ~RingBufferConsumer();

    // set the name of the RingBufferConsumer that will be used to identify it in
    // log messages.
    void setName(const String8& name);

    // setDefaultBufferSize is used to set the size of buffers returned by
    // requestBuffers when a with and height of zero is requested.
    status_t setDefaultBufferSize(uint32_t w, uint32_t h);

    // setDefaultBufferFormat allows the BufferQueue to create
    // GraphicBuffers of a defaultFormat if no format is specified
    // by the producer endpoint.
    status_t setDefaultBufferFormat(uint32_t defaultFormat);

    // setConsumerUsage allows the BufferQueue consumer usage to be
    // set at a later time after construction.
    status_t setConsumerUsage(uint64_t usage);

    // Buffer info, minus the graphics buffer/slot itself.
    struct BufferInfo {
        // mCrop is the current crop rectangle for this buffer slot.
        Rect mCrop;

        // mTransform is the current transform flags for this buffer slot.
        uint32_t mTransform;

        // mScalingMode is the current scaling mode for this buffer slot.
        uint32_t mScalingMode;

        // mTimestamp is the current timestamp for this buffer slot. This gets
        // to set by queueBuffer each time this slot is queued.
        int64_t mTimestamp;

        // mFrameNumber is the number of the queued frame for this slot.
        uint64_t mFrameNumber;

        // mPinned is whether or not the buffer has been pinned already.
        bool mPinned;
    };

    struct RingBufferComparator {
        // Return < 0 to select i1, > 0 to select i2, 0 for neither
        // i1 or i2 can be NULL.
        //
        // The comparator has to implement a total ordering. Otherwise
        // a linear scan won't find the most preferred buffer.
        virtual int compare(const BufferInfo* i1,
                            const BufferInfo* i2) const = 0;

        virtual ~RingBufferComparator() {}
    };

    struct PinnedBufferItem : public LightRefBase<PinnedBufferItem> {
        PinnedBufferItem(wp<RingBufferConsumer> consumer,
                         const BufferItem& item) :
                mConsumer(consumer),
                mBufferItem(item) {
        }

        ~PinnedBufferItem() {
            sp<RingBufferConsumer> consumer = mConsumer.promote();
            if (consumer != NULL) {
                consumer->unpinBuffer(mBufferItem);
            }
        }

        bool isEmpty() {
            return mBufferItem.mSlot == BufferQueue::INVALID_BUFFER_SLOT;
        }

        BufferItem& getBufferItem() { return mBufferItem; }
        const BufferItem& getBufferItem() const { return mBufferItem; }

      private:
        wp<RingBufferConsumer> mConsumer;
        BufferItem             mBufferItem;
    };

    // Find a buffer using the filter, then pin it before returning it.
    //
    // The filter will be invoked on each buffer item in the ring buffer,
    // passing the item that was selected from each previous iteration,
    // as well as the current iteration's item.
    //
    // Pinning will ensure that the buffer will not be dropped when a new
    // frame is available.
    sp<PinnedBufferItem> pinSelectedBuffer(const RingBufferComparator& filter,
                                           bool waitForFence = true);

    // Release all the non-pinned buffers in the ring buffer
    status_t clear();

    // Return 0 if RingBuffer is empty, otherwise return timestamp of latest buffer.
    nsecs_t getLatestTimestamp();

  private:

    // Override ConsumerBase::onFrameAvailable
    virtual void onFrameAvailable(const BufferItem& item);

    void pinBufferLocked(const BufferItem& item);
    void unpinBuffer(const BufferItem& item);

    // Releases oldest buffer. Returns NO_BUFFER_AVAILABLE
    // if all the buffers were pinned.
    // Returns NOT_ENOUGH_DATA if list was empty.
    status_t releaseOldestBufferLocked(size_t* pinnedFrames);

    struct RingBufferItem : public BufferItem {
        RingBufferItem() : BufferItem(), mPinCount(0) {}
        int mPinCount;
    };

    // List of acquired buffers in our ring buffer
    List<RingBufferItem>       mBufferItemList;
    const int                  mBufferCount;

    // Timestamp of latest buffer
    nsecs_t mLatestTimestamp;
};

} // namespace android

#endif // ANDROID_GUI_RINGBUFFERCONSUMER_H
