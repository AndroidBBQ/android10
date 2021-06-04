/*
 * Copyright (C) 2018 The Android Open Source Project
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

#pragma once

#include "BufferLayer.h"
#include "Layer.h"

#include <gui/GLConsumer.h>
#include <renderengine/Image.h>
#include <renderengine/RenderEngine.h>
#include <system/window.h>
#include <utils/String8.h>

#include <stack>

namespace android {

class SlotGenerationTest;

class BufferStateLayer : public BufferLayer {
public:
    explicit BufferStateLayer(const LayerCreationArgs&);

    ~BufferStateLayer() override;

    // -----------------------------------------------------------------------
    // Interface implementation for Layer
    // -----------------------------------------------------------------------
    void onLayerDisplayed(const sp<Fence>& releaseFence) override;
    void setTransformHint(uint32_t orientation) const override;
    void releasePendingBuffer(nsecs_t dequeueReadyTime) override;

    bool shouldPresentNow(nsecs_t expectedPresentTime) const override;

    bool getTransformToDisplayInverse() const override;

    uint32_t doTransactionResize(uint32_t flags, Layer::State* /*stateToCommit*/) override {
        return flags;
    }
    void pushPendingState() override;
    bool applyPendingStates(Layer::State* stateToCommit) override;

    uint32_t getActiveWidth(const Layer::State& s) const override { return s.active.w; }
    uint32_t getActiveHeight(const Layer::State& s) const override { return s.active.h; }
    ui::Transform getActiveTransform(const Layer::State& s) const override {
        return s.active.transform;
    }
    Region getActiveTransparentRegion(const Layer::State& s) const override {
        return s.transparentRegionHint;
    }
    Rect getCrop(const Layer::State& s) const;

    bool setTransform(uint32_t transform) override;
    bool setTransformToDisplayInverse(bool transformToDisplayInverse) override;
    bool setCrop(const Rect& crop) override;
    bool setFrame(const Rect& frame) override;
    bool setBuffer(const sp<GraphicBuffer>& buffer, nsecs_t postTime, nsecs_t desiredPresentTime,
                   const client_cache_t& clientCacheId) override;
    bool setAcquireFence(const sp<Fence>& fence) override;
    bool setDataspace(ui::Dataspace dataspace) override;
    bool setHdrMetadata(const HdrMetadata& hdrMetadata) override;
    bool setSurfaceDamageRegion(const Region& surfaceDamage) override;
    bool setApi(int32_t api) override;
    bool setSidebandStream(const sp<NativeHandle>& sidebandStream) override;
    bool setTransactionCompletedListeners(const std::vector<sp<CallbackHandle>>& handles) override;

    // Override to ignore legacy layer state properties that are not used by BufferStateLayer
    bool setSize(uint32_t /*w*/, uint32_t /*h*/) override { return false; }
    bool setPosition(float /*x*/, float /*y*/, bool /*immediate*/) override { return false; }
    bool setTransparentRegionHint(const Region& transparent) override;
    bool setMatrix(const layer_state_t::matrix22_t& /*matrix*/,
                   bool /*allowNonRectPreservingTransforms*/) override {
        return false;
    }
    bool setCrop_legacy(const Rect& /*crop*/, bool /*immediate*/) override { return false; }
    bool setOverrideScalingMode(int32_t /*overrideScalingMode*/) override { return false; }
    void deferTransactionUntil_legacy(const sp<IBinder>& /*barrierHandle*/,
                                      uint64_t /*frameNumber*/) override {}
    void deferTransactionUntil_legacy(const sp<Layer>& /*barrierLayer*/,
                                      uint64_t /*frameNumber*/) override {}

    Rect getBufferSize(const State& s) const override;
    FloatRect computeSourceBounds(const FloatRect& parentBounds) const override;

    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    // Interface implementation for BufferLayer
    // -----------------------------------------------------------------------
    bool fenceHasSignaled() const override;
    bool framePresentTimeIsCurrent() const override;

private:
    nsecs_t getDesiredPresentTime() override;
    std::shared_ptr<FenceTime> getCurrentFenceTime() const override;

    void getDrawingTransformMatrix(float *matrix) override;
    uint32_t getDrawingTransform() const override;
    ui::Dataspace getDrawingDataSpace() const override;
    Rect getDrawingCrop() const override;
    uint32_t getDrawingScalingMode() const override;
    Region getDrawingSurfaceDamage() const override;
    const HdrMetadata& getDrawingHdrMetadata() const override;
    int getDrawingApi() const override;
    PixelFormat getPixelFormat() const override;

    uint64_t getFrameNumber() const override;

    bool getAutoRefresh() const override;
    bool getSidebandStreamChanged() const override;

    bool latchSidebandStream(bool& recomputeVisibleRegions) override;

    bool hasFrameUpdate() const override;

    void setFilteringEnabled(bool enabled) override;

    status_t bindTextureImage() override;
    status_t updateTexImage(bool& recomputeVisibleRegions, nsecs_t latchTime) override;

    status_t updateActiveBuffer() override;
    status_t updateFrameNumber(nsecs_t latchTime) override;

    void setHwcLayerBuffer(const sp<const DisplayDevice>& display) override;

private:
    friend class SlotGenerationTest;
    void onFirstRef() override;
    bool willPresentCurrentTransaction() const;

    static const std::array<float, 16> IDENTITY_MATRIX;

    std::unique_ptr<renderengine::Image> mTextureImage;

    std::array<float, 16> mTransformMatrix{IDENTITY_MATRIX};

    std::atomic<bool> mSidebandStreamChanged{false};

    uint32_t mFrameNumber{0};

    sp<Fence> mPreviousReleaseFence;

    bool mCurrentStateModified = false;
    bool mReleasePreviousBuffer = false;
    nsecs_t mCallbackHandleAcquireTime = -1;

    nsecs_t mDesiredPresentTime = -1;

    // TODO(marissaw): support sticky transform for LEGACY camera mode

    class HwcSlotGenerator : public ClientCache::ErasedRecipient {
    public:
        HwcSlotGenerator() {
            for (uint32_t i = 0; i < BufferQueue::NUM_BUFFER_SLOTS; i++) {
                mFreeHwcCacheSlots.push(i);
            }
        }

        void bufferErased(const client_cache_t& clientCacheId);

        uint32_t getHwcCacheSlot(const client_cache_t& clientCacheId);

    private:
        friend class SlotGenerationTest;
        uint32_t addCachedBuffer(const client_cache_t& clientCacheId) REQUIRES(mMutex);
        uint32_t getFreeHwcCacheSlot() REQUIRES(mMutex);
        void evictLeastRecentlyUsed() REQUIRES(mMutex);
        void eraseBufferLocked(const client_cache_t& clientCacheId) REQUIRES(mMutex);

        struct CachedBufferHash {
            std::size_t operator()(const client_cache_t& clientCacheId) const {
                return std::hash<uint64_t>{}(clientCacheId.id);
            }
        };

        std::mutex mMutex;

        std::unordered_map<client_cache_t,
                           std::pair<uint32_t /*HwcCacheSlot*/, uint32_t /*counter*/>,
                           CachedBufferHash>
                mCachedBuffers GUARDED_BY(mMutex);
        std::stack<uint32_t /*HwcCacheSlot*/> mFreeHwcCacheSlots GUARDED_BY(mMutex);

        // The cache increments this counter value when a slot is updated or used.
        // Used to track the least recently-used buffer
        uint64_t mCounter = 0;
    };

    sp<HwcSlotGenerator> mHwcSlotGenerator;
};

} // namespace android
