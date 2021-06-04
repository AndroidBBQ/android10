/*
 * Copyright 2014,2016 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_STREAMSPLITTER_H
#define ANDROID_SERVERS_STREAMSPLITTER_H

#include <unordered_set>

#include <gui/IConsumerListener.h>
#include <gui/IProducerListener.h>
#include <gui/BufferItemConsumer.h>

#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
#include <utils/Timers.h>

#define SP_LOGV(x, ...) ALOGV("[%s] " x, mConsumerName.string(), ##__VA_ARGS__)
#define SP_LOGI(x, ...) ALOGI("[%s] " x, mConsumerName.string(), ##__VA_ARGS__)
#define SP_LOGW(x, ...) ALOGW("[%s] " x, mConsumerName.string(), ##__VA_ARGS__)
#define SP_LOGE(x, ...) ALOGE("[%s] " x, mConsumerName.string(), ##__VA_ARGS__)

namespace android {

class GraphicBuffer;
class IGraphicBufferConsumer;
class IGraphicBufferProducer;

// Camera3StreamSplitter is an autonomous class that manages one input BufferQueue
// and multiple output BufferQueues. By using the buffer attach and detach logic
// in BufferQueue, it is able to present the illusion of a single split
// BufferQueue, where each buffer queued to the input is available to be
// acquired by each of the outputs, and is able to be dequeued by the input
// again only once all of the outputs have released it.
class Camera3StreamSplitter : public BnConsumerListener {
public:

    // Constructor
    Camera3StreamSplitter(bool useHalBufManager = false);

    // Connect to the stream splitter by creating buffer queue and connecting it
    // with output surfaces.
    status_t connect(const std::unordered_map<size_t, sp<Surface>> &surfaces,
            uint64_t consumerUsage, uint64_t producerUsage, size_t halMaxBuffers, uint32_t width,
            uint32_t height, android::PixelFormat format, sp<Surface>* consumer);

    // addOutput adds an output BufferQueue to the splitter. The splitter
    // connects to outputQueue as a CPU producer, and any buffers queued
    // to the input will be queued to each output. If any  output is abandoned
    // by its consumer, the splitter will abandon its input queue (see onAbandoned).
    //
    // A return value other than NO_ERROR means that an error has occurred and
    // outputQueue has not been added to the splitter. BAD_VALUE is returned if
    // outputQueue is NULL. See IGraphicBufferProducer::connect for explanations
    // of other error codes.
    status_t addOutput(size_t surfaceId, const sp<Surface>& outputQueue);

    //removeOutput will remove a BufferQueue that was previously added to
    //the splitter outputs. Any pending buffers in the BufferQueue will get
    //reclaimed.
    status_t removeOutput(size_t surfaceId);

    // Notification that the graphic buffer has been released to the input
    // BufferQueue. The buffer should be reused by the camera device instead of
    // queuing to the outputs.
    status_t notifyBufferReleased(const sp<GraphicBuffer>& buffer);

    // Attach a buffer to the specified outputs. This call reserves a buffer
    // slot in the output queue.
    status_t attachBufferToOutputs(ANativeWindowBuffer* anb,
            const std::vector<size_t>& surface_ids);

    // Get return value of onFrameAvailable to work around problem that
    // onFrameAvailable is void. This function should be called by the producer
    // right after calling queueBuffer().
    status_t getOnFrameAvailableResult();

    // Disconnect the buffer queue from output surfaces.
    void disconnect();

private:
    // From IConsumerListener
    //
    // During this callback, we store some tracking information, detach the
    // buffer from the input, and attach it to each of the outputs. This call
    // can block if there are too many outstanding buffers. If it blocks, it
    // will resume when onBufferReleasedByOutput releases a buffer back to the
    // input.
    void onFrameAvailable(const BufferItem& item) override;

    // From IConsumerListener
    // We don't care about released buffers because we detach each buffer as
    // soon as we acquire it. See the comment for onBufferReleased below for
    // some clarifying notes about the name.
    void onBuffersReleased() override {}

    // From IConsumerListener
    // We don't care about sideband streams, since we won't be splitting them
    void onSidebandStreamChanged() override {}

    // This is the implementation of the onBufferReleased callback from
    // IProducerListener. It gets called from an OutputListener (see below), and
    // 'from' is which producer interface from which the callback was received.
    //
    // During this callback, we detach the buffer from the output queue that
    // generated the callback, update our state tracking to see if this is the
    // last output releasing the buffer, and if so, release it to the input.
    // If we release the buffer to the input, we allow a blocked
    // onFrameAvailable call to proceed.
    void onBufferReleasedByOutput(const sp<IGraphicBufferProducer>& from);

    // Called by outputBufferLocked when a buffer in the async buffer queue got replaced.
    void onBufferReplacedLocked(const sp<IGraphicBufferProducer>& from, size_t surfaceId);

    // When this is called, the splitter disconnects from (i.e., abandons) its
    // input queue and signals any waiting onFrameAvailable calls to wake up.
    // It still processes callbacks from other outputs, but only detaches their
    // buffers so they can continue operating until they run out of buffers to
    // acquire. This must be called with mMutex locked.
    void onAbandonedLocked();

    // Decrement the buffer's reference count. Once the reference count becomes
    // 0, return the buffer back to the input BufferQueue.
    void decrementBufRefCountLocked(uint64_t id, size_t surfaceId);

    // Check for and handle any output surface dequeue errors.
    void handleOutputDequeueStatusLocked(status_t res, int slot);

    // Handles released output surface buffers.
    void returnOutputBufferLocked(const sp<Fence>& fence, const sp<IGraphicBufferProducer>& from,
            size_t surfaceId, int slot);

    // This is a thin wrapper class that lets us determine which BufferQueue
    // the IProducerListener::onBufferReleased callback is associated with. We
    // create one of these per output BufferQueue, and then pass the producer
    // into onBufferReleasedByOutput above.
    class OutputListener : public BnProducerListener,
                           public IBinder::DeathRecipient {
    public:
        OutputListener(wp<Camera3StreamSplitter> splitter,
                wp<IGraphicBufferProducer> output);
        virtual ~OutputListener() = default;

        // From IProducerListener
        void onBufferReleased() override;

        // From IBinder::DeathRecipient
        void binderDied(const wp<IBinder>& who) override;

    private:
        wp<Camera3StreamSplitter> mSplitter;
        wp<IGraphicBufferProducer> mOutput;
    };

    class BufferTracker {
    public:
        BufferTracker(const sp<GraphicBuffer>& buffer,
                const std::vector<size_t>& requestedSurfaces);
        ~BufferTracker() = default;

        const sp<GraphicBuffer>& getBuffer() const { return mBuffer; }
        const sp<Fence>& getMergedFence() const { return mMergedFence; }

        void mergeFence(const sp<Fence>& with);

        // Returns the new value
        // Only called while mMutex is held
        size_t decrementReferenceCountLocked(size_t surfaceId);

        const std::vector<size_t> requestedSurfaces() const { return mRequestedSurfaces; }

    private:

        // Disallow copying
        BufferTracker(const BufferTracker& other);
        BufferTracker& operator=(const BufferTracker& other);

        sp<GraphicBuffer> mBuffer; // One instance that holds this native handle
        sp<Fence> mMergedFence;

        // Request surfaces for a particular buffer. And when the buffer becomes
        // available from the input queue, the registered surfaces are used to decide
        // which output is the buffer sent to.
        std::vector<size_t> mRequestedSurfaces;
        size_t mReferenceCount;
    };

    // Must be accessed through RefBase
    virtual ~Camera3StreamSplitter();

    status_t addOutputLocked(size_t surfaceId, const sp<Surface>& outputQueue);

    status_t removeOutputLocked(size_t surfaceId);

    // Send a buffer to particular output, and increment the reference count
    // of the buffer. If this output is abandoned, the buffer's reference count
    // won't be incremented.
    status_t outputBufferLocked(const sp<IGraphicBufferProducer>& output,
            const BufferItem& bufferItem, size_t surfaceId);

    // Get unique name for the buffer queue consumer
    String8 getUniqueConsumerName();

    // Helper function to get the BufferQueue slot where a particular buffer is attached to.
    int getSlotForOutputLocked(const sp<IGraphicBufferProducer>& gbp,
            const sp<GraphicBuffer>& gb);

    // Sum of max consumer buffers for all outputs
    size_t mMaxConsumerBuffers = 0;
    size_t mMaxHalBuffers = 0;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    android::PixelFormat mFormat = android::PIXEL_FORMAT_NONE;
    uint64_t mProducerUsage = 0;

    // The attachBuffer call will happen on different thread according to mUseHalBufManager and have
    // different timing constraint.
    static const nsecs_t kNormalDequeueBufferTimeout    = s2ns(1);  // 1 sec
    static const nsecs_t kHalBufMgrDequeueBufferTimeout = ms2ns(1); // 1 msec

    Mutex mMutex;

    sp<IGraphicBufferProducer> mProducer;
    sp<IGraphicBufferConsumer> mConsumer;
    sp<BufferItemConsumer> mBufferItemConsumer;
    sp<Surface> mSurface;

    //Map graphic buffer ids -> buffer items
    std::unordered_map<uint64_t, BufferItem> mInputSlots;

    //Map surface ids -> gbp outputs
    std::unordered_map<int, sp<IGraphicBufferProducer> > mOutputs;

    //Map surface ids -> consumer buffer count
    std::unordered_map<int, size_t > mConsumerBufferCount;

    // Map of GraphicBuffer IDs (GraphicBuffer::getId()) to buffer tracking
    // objects (which are mostly for counting how many outputs have released the
    // buffer, but also contain merged release fences).
    std::unordered_map<uint64_t, std::unique_ptr<BufferTracker> > mBuffers;

    struct GBPHash {
        std::size_t operator()(const sp<IGraphicBufferProducer>& producer) const {
            return std::hash<IGraphicBufferProducer *>{}(producer.get());
        }
    };

    std::unordered_map<sp<IGraphicBufferProducer>, sp<OutputListener>,
            GBPHash> mNotifiers;

    typedef std::vector<sp<GraphicBuffer>> OutputSlots;
    std::unordered_map<sp<IGraphicBufferProducer>, std::unique_ptr<OutputSlots>,
            GBPHash> mOutputSlots;

    //A set of buffers that could potentially stay in some of the outputs after removal
    //and therefore should be detached from the input queue.
    std::unordered_set<uint64_t> mDetachedBuffers;

    // Latest onFrameAvailable return value
    std::atomic<status_t> mOnFrameAvailableRes{0};

    // Currently acquired input buffers
    size_t mAcquiredInputBuffers;

    String8 mConsumerName;

    const bool mUseHalBufManager;
};

} // namespace android

#endif
