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

#include <utils/String8.h>

namespace android {

/*
 * A new BufferQueue and a new BufferLayerConsumer are created when the
 * BufferLayer is first referenced.
 *
 * This also implements onFrameAvailable(), which notifies SurfaceFlinger
 * that new data has arrived.
 */
class BufferQueueLayer : public BufferLayer, public BufferLayerConsumer::ContentsChangedListener {
public:
    explicit BufferQueueLayer(const LayerCreationArgs&);
    ~BufferQueueLayer() override;

    // -----------------------------------------------------------------------
    // Interface implementation for Layer
    // -----------------------------------------------------------------------
public:
    void onLayerDisplayed(const sp<Fence>& releaseFence) override;

    void setTransformHint(uint32_t orientation) const override;

    std::vector<OccupancyTracker::Segment> getOccupancyHistory(bool forceFlush) override;

    bool getTransformToDisplayInverse() const override;

    // If a buffer was replaced this frame, release the former buffer
    void releasePendingBuffer(nsecs_t dequeueReadyTime) override;

    void setDefaultBufferSize(uint32_t w, uint32_t h) override;

    int32_t getQueuedFrameCount() const override;

    bool shouldPresentNow(nsecs_t expectedPresentTime) const override;
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    // Interface implementation for BufferLayer
    // -----------------------------------------------------------------------
public:
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

    void setHwcLayerBuffer(const sp<const DisplayDevice>& displayDevice) override;

    // -----------------------------------------------------------------------
    // Interface implementation for BufferLayerConsumer::ContentsChangedListener
    // -----------------------------------------------------------------------
protected:
    void onFrameAvailable(const BufferItem& item) override;
    void onFrameReplaced(const BufferItem& item) override;
    void onSidebandStreamChanged() override;
    // -----------------------------------------------------------------------

public:
    status_t setDefaultBufferProperties(uint32_t w, uint32_t h, PixelFormat format);

    sp<IGraphicBufferProducer> getProducer() const;

private:
    // Temporary - Used only for LEGACY camera mode.
    uint32_t getProducerStickyTransform() const;

    void onFirstRef() override;

    sp<BufferLayerConsumer> mConsumer;
    sp<IGraphicBufferProducer> mProducer;

    PixelFormat mFormat{PIXEL_FORMAT_NONE};

    // Only accessed on the main thread.
    uint64_t mPreviousFrameNumber{0};
    bool mUpdateTexImageFailed{false};

    // Local copy of the queued contents of the incoming BufferQueue
    mutable Mutex mQueueItemLock;
    Condition mQueueItemCondition;
    Vector<BufferItem> mQueueItems;
    std::atomic<uint64_t> mLastFrameNumberReceived{0};

    bool mAutoRefresh{false};
    int mActiveBufferSlot{BufferQueue::INVALID_BUFFER_SLOT};

    // thread-safe
    std::atomic<int32_t> mQueuedFrames{0};
    std::atomic<bool> mSidebandStreamChanged{false};

    void fakeVsync();
};

} // namespace android
