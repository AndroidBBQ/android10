/*
 * Copyright 2017, The Android Open Source Project
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

#ifndef CCODEC_BUFFER_CHANNEL_H_

#define CCODEC_BUFFER_CHANNEL_H_

#include <map>
#include <memory>
#include <vector>

#include <C2Buffer.h>
#include <C2Component.h>
#include <Codec2Mapper.h>

#include <codec2/hidl/client.h>
#include <media/stagefright/foundation/Mutexed.h>
#include <media/stagefright/CodecBase.h>
#include <media/ICrypto.h>

#include "CCodecBuffers.h"
#include "InputSurfaceWrapper.h"
#include "PipelineWatcher.h"

namespace android {

class MemoryDealer;

class CCodecCallback {
public:
    virtual ~CCodecCallback() = default;
    virtual void onError(status_t err, enum ActionCode actionCode) = 0;
    virtual void onOutputFramesRendered(int64_t mediaTimeUs, nsecs_t renderTimeNs) = 0;
    virtual void onOutputBuffersChanged() = 0;
};

/**
 * BufferChannelBase implementation for CCodec.
 */
class CCodecBufferChannel
    : public BufferChannelBase, public std::enable_shared_from_this<CCodecBufferChannel> {
public:
    explicit CCodecBufferChannel(const std::shared_ptr<CCodecCallback> &callback);
    virtual ~CCodecBufferChannel();

    // BufferChannelBase interface
    virtual status_t queueInputBuffer(const sp<MediaCodecBuffer> &buffer) override;
    virtual status_t queueSecureInputBuffer(
            const sp<MediaCodecBuffer> &buffer,
            bool secure,
            const uint8_t *key,
            const uint8_t *iv,
            CryptoPlugin::Mode mode,
            CryptoPlugin::Pattern pattern,
            const CryptoPlugin::SubSample *subSamples,
            size_t numSubSamples,
            AString *errorDetailMsg) override;
    virtual status_t renderOutputBuffer(
            const sp<MediaCodecBuffer> &buffer, int64_t timestampNs) override;
    virtual status_t discardBuffer(const sp<MediaCodecBuffer> &buffer) override;
    virtual void getInputBufferArray(Vector<sp<MediaCodecBuffer>> *array) override;
    virtual void getOutputBufferArray(Vector<sp<MediaCodecBuffer>> *array) override;

    // Methods below are interface for CCodec to use.

    /**
     * Set the component object for buffer processing.
     */
    void setComponent(const std::shared_ptr<Codec2Client::Component> &component);

    /**
     * Set output graphic surface for rendering.
     */
    status_t setSurface(const sp<Surface> &surface);

    /**
     * Set GraphicBufferSource object from which the component extracts input
     * buffers.
     */
    status_t setInputSurface(const std::shared_ptr<InputSurfaceWrapper> &surface);

    /**
     * Signal EOS to input surface.
     */
    status_t signalEndOfInputStream();

    /**
     * Set parameters.
     */
    status_t setParameters(std::vector<std::unique_ptr<C2Param>> &params);

    /**
     * Start queueing buffers to the component. This object should never queue
     * buffers before this call has completed.
     */
    status_t start(const sp<AMessage> &inputFormat, const sp<AMessage> &outputFormat);

    /**
     * Request initial input buffers to be filled by client.
     */
    status_t requestInitialInputBuffers();

    /**
     * Stop queueing buffers to the component. This object should never queue
     * buffers after this call, until start() is called.
     */
    void stop();

    void flush(const std::list<std::unique_ptr<C2Work>> &flushedWork);

    /**
     * Notify input client about work done.
     *
     * @param workItems   finished work item.
     * @param outputFormat new output format if it has changed, otherwise nullptr
     * @param initData    new init data (CSD) if it has changed, otherwise nullptr
     */
    void onWorkDone(
            std::unique_ptr<C2Work> work, const sp<AMessage> &outputFormat,
            const C2StreamInitDataInfo::output *initData);

    /**
     * Make an input buffer available for the client as it is no longer needed
     * by the codec.
     *
     * @param frameIndex The index of input work
     * @param arrayIndex The index of buffer in the input work buffers.
     */
    void onInputBufferDone(uint64_t frameIndex, size_t arrayIndex);

    PipelineWatcher::Clock::duration elapsed();

    enum MetaMode {
        MODE_NONE,
        MODE_ANW,
    };

    void setMetaMode(MetaMode mode);

private:
    class QueueGuard;

    /**
     * Special mutex-like object with the following properties:
     *
     * - At STOPPED state (initial, or after stop())
     *   - QueueGuard object gets created at STOPPED state, and the client is
     *     supposed to return immediately.
     * - At RUNNING state (after start())
     *   - Each QueueGuard object
     */
    class QueueSync {
    public:
        /**
         * At construction the sync object is in STOPPED state.
         */
        inline QueueSync() {}
        ~QueueSync() = default;

        /**
         * Transition to RUNNING state when stopped. No-op if already in RUNNING
         * state.
         */
        void start();

        /**
         * At RUNNING state, wait until all QueueGuard object created during
         * RUNNING state are destroyed, and then transition to STOPPED state.
         * No-op if already in STOPPED state.
         */
        void stop();

    private:
        Mutex mGuardLock;

        struct Counter {
            inline Counter() : value(-1) {}
            int32_t value;
            Condition cond;
        };
        Mutexed<Counter> mCount;

        friend class CCodecBufferChannel::QueueGuard;
    };

    class QueueGuard {
    public:
        QueueGuard(QueueSync &sync);
        ~QueueGuard();
        inline bool isRunning() { return mRunning; }

    private:
        QueueSync &mSync;
        bool mRunning;
    };

    void feedInputBufferIfAvailable();
    void feedInputBufferIfAvailableInternal();
    status_t queueInputBufferInternal(sp<MediaCodecBuffer> buffer);
    bool handleWork(
            std::unique_ptr<C2Work> work, const sp<AMessage> &outputFormat,
            const C2StreamInitDataInfo::output *initData);
    void sendOutputBuffers();

    QueueSync mSync;
    sp<MemoryDealer> mDealer;
    sp<IMemory> mDecryptDestination;
    int32_t mHeapSeqNum;

    std::shared_ptr<Codec2Client::Component> mComponent;
    std::string mComponentName; ///< component name for debugging
    const char *mName; ///< C-string version of component name
    std::shared_ptr<CCodecCallback> mCCodecCallback;
    std::shared_ptr<C2BlockPool> mInputAllocator;
    QueueSync mQueueSync;
    std::vector<std::unique_ptr<C2Param>> mParamsToBeSet;

    struct Input {
        Input();

        std::unique_ptr<InputBuffers> buffers;
        size_t numSlots;
        FlexBuffersImpl extraBuffers;
        size_t numExtraSlots;
        uint32_t inputDelay;
        uint32_t pipelineDelay;
    };
    Mutexed<Input> mInput;
    struct Output {
        std::unique_ptr<OutputBuffers> buffers;
        size_t numSlots;
        uint32_t outputDelay;
    };
    Mutexed<Output> mOutput;
    Mutexed<std::list<sp<ABuffer>>> mFlushedConfigs;

    std::atomic_uint64_t mFrameIndex;
    std::atomic_uint64_t mFirstValidFrameIndex;

    sp<MemoryDealer> makeMemoryDealer(size_t heapSize);

    struct OutputSurface {
        sp<Surface> surface;
        uint32_t generation;
        int maxDequeueBuffers;
    };
    Mutexed<OutputSurface> mOutputSurface;

    struct BlockPools {
        C2Allocator::id_t inputAllocatorId;
        std::shared_ptr<C2BlockPool> inputPool;
        C2Allocator::id_t outputAllocatorId;
        C2BlockPool::local_id_t outputPoolId;
        std::shared_ptr<Codec2Client::Configurable> outputPoolIntf;
    };
    Mutexed<BlockPools> mBlockPools;

    std::shared_ptr<InputSurfaceWrapper> mInputSurface;

    MetaMode mMetaMode;

    Mutexed<PipelineWatcher> mPipelineWatcher;

    class ReorderStash {
    public:
        struct Entry {
            inline Entry() : buffer(nullptr), timestamp(0), flags(0), ordinal({0, 0, 0}) {}
            inline Entry(
                    const std::shared_ptr<C2Buffer> &b,
                    int64_t t,
                    int32_t f,
                    const C2WorkOrdinalStruct &o)
                : buffer(b), timestamp(t), flags(f), ordinal(o) {}
            std::shared_ptr<C2Buffer> buffer;
            int64_t timestamp;
            int32_t flags;
            C2WorkOrdinalStruct ordinal;
        };

        ReorderStash();

        void clear();
        void flush();
        void setDepth(uint32_t depth);
        void setKey(C2Config::ordinal_key_t key);
        bool pop(Entry *entry);
        void emplace(
                const std::shared_ptr<C2Buffer> &buffer,
                int64_t timestamp,
                int32_t flags,
                const C2WorkOrdinalStruct &ordinal);
        void defer(const Entry &entry);
        bool hasPending() const;
        uint32_t depth() const { return mDepth; }

    private:
        std::list<Entry> mPending;
        std::list<Entry> mStash;
        uint32_t mDepth;
        C2Config::ordinal_key_t mKey;

        bool less(const C2WorkOrdinalStruct &o1, const C2WorkOrdinalStruct &o2);
    };
    Mutexed<ReorderStash> mReorderStash;

    std::atomic_bool mInputMetEos;
    std::once_flag mRenderWarningFlag;

    inline bool hasCryptoOrDescrambler() {
        return mCrypto != nullptr || mDescrambler != nullptr;
    }
};

// Conversion of a c2_status_t value to a status_t value may depend on the
// operation that returns the c2_status_t value.
enum c2_operation_t {
    C2_OPERATION_NONE,
    C2_OPERATION_Component_connectToOmxInputSurface,
    C2_OPERATION_Component_createBlockPool,
    C2_OPERATION_Component_destroyBlockPool,
    C2_OPERATION_Component_disconnectFromInputSurface,
    C2_OPERATION_Component_drain,
    C2_OPERATION_Component_flush,
    C2_OPERATION_Component_queue,
    C2_OPERATION_Component_release,
    C2_OPERATION_Component_reset,
    C2_OPERATION_Component_setOutputSurface,
    C2_OPERATION_Component_start,
    C2_OPERATION_Component_stop,
    C2_OPERATION_ComponentStore_copyBuffer,
    C2_OPERATION_ComponentStore_createComponent,
    C2_OPERATION_ComponentStore_createInputSurface,
    C2_OPERATION_ComponentStore_createInterface,
    C2_OPERATION_Configurable_config,
    C2_OPERATION_Configurable_query,
    C2_OPERATION_Configurable_querySupportedParams,
    C2_OPERATION_Configurable_querySupportedValues,
    C2_OPERATION_InputSurface_connectToComponent,
    C2_OPERATION_InputSurfaceConnection_disconnect,
};

status_t toStatusT(c2_status_t c2s, c2_operation_t c2op = C2_OPERATION_NONE);

}  // namespace android

#endif  // CCODEC_BUFFER_CHANNEL_H_
