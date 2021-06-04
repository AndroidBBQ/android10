/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <sys/types.h>
#include <cstdint>
#include <list>

#include <gui/ISurfaceComposerClient.h>
#include <gui/LayerState.h>
#include <renderengine/Image.h>
#include <renderengine/Mesh.h>
#include <renderengine/Texture.h>
#include <system/window.h> // For NATIVE_WINDOW_SCALING_MODE_FREEZE
#include <ui/FrameStats.h>
#include <ui/GraphicBuffer.h>
#include <ui/PixelFormat.h>
#include <ui/Region.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Timers.h>

#include "BufferLayerConsumer.h"
#include "Client.h"
#include "DisplayHardware/HWComposer.h"
#include "FrameTracker.h"
#include "Layer.h"
#include "LayerVector.h"
#include "MonitoredProducer.h"
#include "SurfaceFlinger.h"

namespace android {

class BufferLayer : public Layer {
public:
    explicit BufferLayer(const LayerCreationArgs& args);
    virtual ~BufferLayer() override;

    // -----------------------------------------------------------------------
    // Overriden from Layer
    // -----------------------------------------------------------------------
public:
    std::shared_ptr<compositionengine::Layer> getCompositionLayer() const override;

    // If we have received a new buffer this frame, we will pass its surface
    // damage down to hardware composer. Otherwise, we must send a region with
    // one empty rect.
    void useSurfaceDamage() override;
    void useEmptyDamage() override;

    // getTypeId - Provide unique string for each class type in the Layer
    // hierarchy
    const char* getTypeId() const override { return "BufferLayer"; }

    bool isOpaque(const Layer::State& s) const override;

    // isVisible - true if this layer is visible, false otherwise
    bool isVisible() const override;

    // isProtected - true if the layer may contain protected content in the
    // GRALLOC_USAGE_PROTECTED sense.
    bool isProtected() const override;

    // isFixedSize - true if content has a fixed size
    bool isFixedSize() const override;

    bool usesSourceCrop() const override;

    bool isHdrY410() const override;

    void setPerFrameData(const sp<const DisplayDevice>& display, const ui::Transform& transform,
                         const Rect& viewport, int32_t supportedPerFrameMetadata,
                         const ui::Dataspace targetDataspace) override;

    bool onPreComposition(nsecs_t refreshStartTime) override;
    bool onPostComposition(const std::optional<DisplayId>& displayId,
                           const std::shared_ptr<FenceTime>& glDoneFence,
                           const std::shared_ptr<FenceTime>& presentFence,
                           const CompositorTiming& compositorTiming) override;

    // latchBuffer - called each time the screen is redrawn and returns whether
    // the visible regions need to be recomputed (this is a fairly heavy
    // operation, so this should be set only if needed). Typically this is used
    // to figure out if the content or size of a surface has changed.
    bool latchBuffer(bool& recomputeVisibleRegions, nsecs_t latchTime) override;

    bool isBufferLatched() const override { return mRefreshPending; }

    void notifyAvailableFrames() override;

    bool hasReadyFrame() const override;

    // Returns the current scaling mode, unless mOverrideScalingMode
    // is set, in which case, it returns mOverrideScalingMode
    uint32_t getEffectiveScalingMode() const override;
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    // Functions that must be implemented by derived classes
    // -----------------------------------------------------------------------
private:
    virtual bool fenceHasSignaled() const = 0;
    virtual bool framePresentTimeIsCurrent() const = 0;

    virtual nsecs_t getDesiredPresentTime() = 0;
    virtual std::shared_ptr<FenceTime> getCurrentFenceTime() const = 0;

    virtual void getDrawingTransformMatrix(float *matrix) = 0;
    virtual uint32_t getDrawingTransform() const = 0;
    virtual ui::Dataspace getDrawingDataSpace() const = 0;
    virtual Rect getDrawingCrop() const = 0;
    virtual uint32_t getDrawingScalingMode() const = 0;
    virtual Region getDrawingSurfaceDamage() const = 0;
    virtual const HdrMetadata& getDrawingHdrMetadata() const = 0;
    virtual int getDrawingApi() const = 0;
    virtual PixelFormat getPixelFormat() const = 0;

    virtual uint64_t getFrameNumber() const = 0;

    virtual bool getAutoRefresh() const = 0;
    virtual bool getSidebandStreamChanged() const = 0;

    // Latch sideband stream and returns true if the dirty region should be updated.
    virtual bool latchSidebandStream(bool& recomputeVisibleRegions) = 0;

    virtual bool hasFrameUpdate() const = 0;

    virtual void setFilteringEnabled(bool enabled) = 0;

    virtual status_t bindTextureImage() = 0;
    virtual status_t updateTexImage(bool& recomputeVisibleRegions, nsecs_t latchTime) = 0;

    virtual status_t updateActiveBuffer() = 0;
    virtual status_t updateFrameNumber(nsecs_t latchTime) = 0;

    virtual void setHwcLayerBuffer(const sp<const DisplayDevice>& displayDevice) = 0;

protected:
    // Loads the corresponding system property once per process
    static bool latchUnsignaledBuffers();

    // Check all of the local sync points to ensure that all transactions
    // which need to have been applied prior to the frame which is about to
    // be latched have signaled
    bool allTransactionsSignaled();

    static bool getOpacityForFormat(uint32_t format);

    // from GLES
    const uint32_t mTextureName;

    bool mRefreshPending{false};

    // prepareClientLayer - constructs a RenderEngine layer for GPU composition.
    bool prepareClientLayer(const RenderArea& renderArea, const Region& clip,
                            bool useIdentityTransform, Region& clearRegion,
                            const bool supportProtectedContent,
                            renderengine::LayerSettings& layer) override;

private:
    // Returns true if this layer requires filtering
    bool needsFiltering(const sp<const DisplayDevice>& displayDevice) const;

    uint64_t getHeadFrameNumber() const;

    uint32_t mCurrentScalingMode{NATIVE_WINDOW_SCALING_MODE_FREEZE};

    bool mTransformToDisplayInverse{false};

    // main thread.
    bool mBufferLatched{false}; // TODO: Use mActiveBuffer?

    // BufferStateLayers can return Rect::INVALID_RECT if the layer does not have a display frame
    // and its parent layer is not bounded
    Rect getBufferSize(const State& s) const override;

    std::shared_ptr<compositionengine::Layer> mCompositionLayer;

    FloatRect computeSourceBounds(const FloatRect& parentBounds) const override;
};

} // namespace android
