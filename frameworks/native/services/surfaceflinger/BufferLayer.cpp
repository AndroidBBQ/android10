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

//#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "BufferLayer"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "BufferLayer.h"

#include <compositionengine/CompositionEngine.h>
#include <compositionengine/Display.h>
#include <compositionengine/Layer.h>
#include <compositionengine/LayerCreationArgs.h>
#include <compositionengine/OutputLayer.h>
#include <compositionengine/impl/LayerCompositionState.h>
#include <compositionengine/impl/OutputLayerCompositionState.h>
#include <cutils/compiler.h>
#include <cutils/native_handle.h>
#include <cutils/properties.h>
#include <gui/BufferItem.h>
#include <gui/BufferQueue.h>
#include <gui/LayerDebugInfo.h>
#include <gui/Surface.h>
#include <renderengine/RenderEngine.h>
#include <ui/DebugUtils.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/NativeHandle.h>
#include <utils/StopWatch.h>
#include <utils/Trace.h>

#include <cmath>
#include <cstdlib>
#include <mutex>
#include <sstream>

#include "Colorizer.h"
#include "DisplayDevice.h"
#include "LayerRejecter.h"
#include "TimeStats/TimeStats.h"

namespace android {

BufferLayer::BufferLayer(const LayerCreationArgs& args)
      : Layer(args),
        mTextureName(args.flinger->getNewTexture()),
        mCompositionLayer{mFlinger->getCompositionEngine().createLayer(
                compositionengine::LayerCreationArgs{this})} {
    ALOGV("Creating Layer %s", args.name.string());

    mPremultipliedAlpha = !(args.flags & ISurfaceComposerClient::eNonPremultiplied);

    mPotentialCursor = args.flags & ISurfaceComposerClient::eCursorWindow;
    mProtectedByApp = args.flags & ISurfaceComposerClient::eProtectedByApp;
}

BufferLayer::~BufferLayer() {
    mFlinger->deleteTextureAsync(mTextureName);
    mFlinger->mTimeStats->onDestroy(getSequence());
}

void BufferLayer::useSurfaceDamage() {
    if (mFlinger->mForceFullDamage) {
        surfaceDamageRegion = Region::INVALID_REGION;
    } else {
        surfaceDamageRegion = getDrawingSurfaceDamage();
    }
}

void BufferLayer::useEmptyDamage() {
    surfaceDamageRegion.clear();
}

bool BufferLayer::isOpaque(const Layer::State& s) const {
    // if we don't have a buffer or sidebandStream yet, we're translucent regardless of the
    // layer's opaque flag.
    if ((mSidebandStream == nullptr) && (mActiveBuffer == nullptr)) {
        return false;
    }

    // if the layer has the opaque flag, then we're always opaque,
    // otherwise we use the current buffer's format.
    return ((s.flags & layer_state_t::eLayerOpaque) != 0) || getOpacityForFormat(getPixelFormat());
}

bool BufferLayer::isVisible() const {
    bool visible = !(isHiddenByPolicy()) && getAlpha() > 0.0f &&
            (mActiveBuffer != nullptr || mSidebandStream != nullptr);
    mFlinger->mScheduler->setLayerVisibility(mSchedulerLayerHandle, visible);

    return visible;
}

bool BufferLayer::isFixedSize() const {
    return getEffectiveScalingMode() != NATIVE_WINDOW_SCALING_MODE_FREEZE;
}

bool BufferLayer::usesSourceCrop() const {
    return true;
}

static constexpr mat4 inverseOrientation(uint32_t transform) {
    const mat4 flipH(-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1);
    const mat4 flipV(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1);
    const mat4 rot90(0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1);
    mat4 tr;

    if (transform & NATIVE_WINDOW_TRANSFORM_ROT_90) {
        tr = tr * rot90;
    }
    if (transform & NATIVE_WINDOW_TRANSFORM_FLIP_H) {
        tr = tr * flipH;
    }
    if (transform & NATIVE_WINDOW_TRANSFORM_FLIP_V) {
        tr = tr * flipV;
    }
    return inverse(tr);
}

bool BufferLayer::prepareClientLayer(const RenderArea& renderArea, const Region& clip,
                                     bool useIdentityTransform, Region& clearRegion,
                                     const bool supportProtectedContent,
                                     renderengine::LayerSettings& layer) {
    ATRACE_CALL();
    Layer::prepareClientLayer(renderArea, clip, useIdentityTransform, clearRegion,
                              supportProtectedContent, layer);
    if (CC_UNLIKELY(mActiveBuffer == 0)) {
        // the texture has not been created yet, this Layer has
        // in fact never been drawn into. This happens frequently with
        // SurfaceView because the WindowManager can't know when the client
        // has drawn the first time.

        // If there is nothing under us, we paint the screen in black, otherwise
        // we just skip this update.

        // figure out if there is something below us
        Region under;
        bool finished = false;
        mFlinger->mDrawingState.traverseInZOrder([&](Layer* layer) {
            if (finished || layer == static_cast<BufferLayer const*>(this)) {
                finished = true;
                return;
            }
            under.orSelf(layer->visibleRegion);
        });
        // if not everything below us is covered, we plug the holes!
        Region holes(clip.subtract(under));
        if (!holes.isEmpty()) {
            clearRegion.orSelf(holes);
        }
        return false;
    }
    bool blackOutLayer =
            (isProtected() && !supportProtectedContent) || (isSecure() && !renderArea.isSecure());
    const State& s(getDrawingState());
    if (!blackOutLayer) {
        layer.source.buffer.buffer = mActiveBuffer;
        layer.source.buffer.isOpaque = isOpaque(s);
        layer.source.buffer.fence = mActiveBufferFence;
        layer.source.buffer.textureName = mTextureName;
        layer.source.buffer.usePremultipliedAlpha = getPremultipledAlpha();
        layer.source.buffer.isY410BT2020 = isHdrY410();
        // TODO: we could be more subtle with isFixedSize()
        const bool useFiltering = needsFiltering(renderArea.getDisplayDevice()) ||
                renderArea.needsFiltering() || isFixedSize();

        // Query the texture matrix given our current filtering mode.
        float textureMatrix[16];
        setFilteringEnabled(useFiltering);
        getDrawingTransformMatrix(textureMatrix);

        if (getTransformToDisplayInverse()) {
            /*
             * the code below applies the primary display's inverse transform to
             * the texture transform
             */
            uint32_t transform = DisplayDevice::getPrimaryDisplayOrientationTransform();
            mat4 tr = inverseOrientation(transform);

            /**
             * TODO(b/36727915): This is basically a hack.
             *
             * Ensure that regardless of the parent transformation,
             * this buffer is always transformed from native display
             * orientation to display orientation. For example, in the case
             * of a camera where the buffer remains in native orientation,
             * we want the pixels to always be upright.
             */
            sp<Layer> p = mDrawingParent.promote();
            if (p != nullptr) {
                const auto parentTransform = p->getTransform();
                tr = tr * inverseOrientation(parentTransform.getOrientation());
            }

            // and finally apply it to the original texture matrix
            const mat4 texTransform(mat4(static_cast<const float*>(textureMatrix)) * tr);
            memcpy(textureMatrix, texTransform.asArray(), sizeof(textureMatrix));
        }

        const Rect win{getBounds()};
        float bufferWidth = getBufferSize(s).getWidth();
        float bufferHeight = getBufferSize(s).getHeight();

        // BufferStateLayers can have a "buffer size" of [0, 0, -1, -1] when no display frame has
        // been set and there is no parent layer bounds. In that case, the scale is meaningless so
        // ignore them.
        if (!getBufferSize(s).isValid()) {
            bufferWidth = float(win.right) - float(win.left);
            bufferHeight = float(win.bottom) - float(win.top);
        }

        const float scaleHeight = (float(win.bottom) - float(win.top)) / bufferHeight;
        const float scaleWidth = (float(win.right) - float(win.left)) / bufferWidth;
        const float translateY = float(win.top) / bufferHeight;
        const float translateX = float(win.left) / bufferWidth;

        // Flip y-coordinates because GLConsumer expects OpenGL convention.
        mat4 tr = mat4::translate(vec4(.5, .5, 0, 1)) * mat4::scale(vec4(1, -1, 1, 1)) *
                mat4::translate(vec4(-.5, -.5, 0, 1)) *
                mat4::translate(vec4(translateX, translateY, 0, 1)) *
                mat4::scale(vec4(scaleWidth, scaleHeight, 1.0, 1.0));

        layer.source.buffer.useTextureFiltering = useFiltering;
        layer.source.buffer.textureTransform = mat4(static_cast<const float*>(textureMatrix)) * tr;
    } else {
        // If layer is blacked out, force alpha to 1 so that we draw a black color
        // layer.
        layer.source.buffer.buffer = nullptr;
        layer.alpha = 1.0;
    }

    return true;
}

bool BufferLayer::isHdrY410() const {
    // pixel format is HDR Y410 masquerading as RGBA_1010102
    return (mCurrentDataSpace == ui::Dataspace::BT2020_ITU_PQ &&
            getDrawingApi() == NATIVE_WINDOW_API_MEDIA &&
            mActiveBuffer->getPixelFormat() == HAL_PIXEL_FORMAT_RGBA_1010102);
}

void BufferLayer::setPerFrameData(const sp<const DisplayDevice>& displayDevice,
                                  const ui::Transform& transform, const Rect& viewport,
                                  int32_t supportedPerFrameMetadata,
                                  const ui::Dataspace targetDataspace) {
    RETURN_IF_NO_HWC_LAYER(displayDevice);

    // Apply this display's projection's viewport to the visible region
    // before giving it to the HWC HAL.
    Region visible = transform.transform(visibleRegion.intersect(viewport));

    const auto outputLayer = findOutputLayerForDisplay(displayDevice);
    LOG_FATAL_IF(!outputLayer || !outputLayer->getState().hwc);

    auto& hwcLayer = (*outputLayer->getState().hwc).hwcLayer;
    auto error = hwcLayer->setVisibleRegion(visible);
    if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to set visible region: %s (%d)", mName.string(),
              to_string(error).c_str(), static_cast<int32_t>(error));
        visible.dump(LOG_TAG);
    }
    outputLayer->editState().visibleRegion = visible;

    auto& layerCompositionState = getCompositionLayer()->editState().frontEnd;

    error = hwcLayer->setSurfaceDamage(surfaceDamageRegion);
    if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to set surface damage: %s (%d)", mName.string(),
              to_string(error).c_str(), static_cast<int32_t>(error));
        surfaceDamageRegion.dump(LOG_TAG);
    }
    layerCompositionState.surfaceDamage = surfaceDamageRegion;

    // Sideband layers
    if (layerCompositionState.sidebandStream.get()) {
        setCompositionType(displayDevice, Hwc2::IComposerClient::Composition::SIDEBAND);
        ALOGV("[%s] Requesting Sideband composition", mName.string());
        error = hwcLayer->setSidebandStream(layerCompositionState.sidebandStream->handle());
        if (error != HWC2::Error::None) {
            ALOGE("[%s] Failed to set sideband stream %p: %s (%d)", mName.string(),
                  layerCompositionState.sidebandStream->handle(), to_string(error).c_str(),
                  static_cast<int32_t>(error));
        }
        layerCompositionState.compositionType = Hwc2::IComposerClient::Composition::SIDEBAND;
        return;
    }

    // Device or Cursor layers
    if (mPotentialCursor) {
        ALOGV("[%s] Requesting Cursor composition", mName.string());
        setCompositionType(displayDevice, Hwc2::IComposerClient::Composition::CURSOR);
    } else {
        ALOGV("[%s] Requesting Device composition", mName.string());
        setCompositionType(displayDevice, Hwc2::IComposerClient::Composition::DEVICE);
    }

    ui::Dataspace dataspace = isColorSpaceAgnostic() && targetDataspace != ui::Dataspace::UNKNOWN
            ? targetDataspace
            : mCurrentDataSpace;
    error = hwcLayer->setDataspace(dataspace);
    if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to set dataspace %d: %s (%d)", mName.string(), dataspace,
              to_string(error).c_str(), static_cast<int32_t>(error));
    }

    const HdrMetadata& metadata = getDrawingHdrMetadata();
    error = hwcLayer->setPerFrameMetadata(supportedPerFrameMetadata, metadata);
    if (error != HWC2::Error::None && error != HWC2::Error::Unsupported) {
        ALOGE("[%s] Failed to set hdrMetadata: %s (%d)", mName.string(),
              to_string(error).c_str(), static_cast<int32_t>(error));
    }

    error = hwcLayer->setColorTransform(getColorTransform());
    if (error == HWC2::Error::Unsupported) {
        // If per layer color transform is not supported, we use GPU composition.
        setCompositionType(displayDevice, Hwc2::IComposerClient::Composition::CLIENT);
    } else if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to setColorTransform: %s (%d)", mName.string(),
                to_string(error).c_str(), static_cast<int32_t>(error));
    }
    layerCompositionState.dataspace = mCurrentDataSpace;
    layerCompositionState.colorTransform = getColorTransform();
    layerCompositionState.hdrMetadata = metadata;

    setHwcLayerBuffer(displayDevice);
}

bool BufferLayer::onPreComposition(nsecs_t refreshStartTime) {
    if (mBufferLatched) {
        Mutex::Autolock lock(mFrameEventHistoryMutex);
        mFrameEventHistory.addPreComposition(mCurrentFrameNumber, refreshStartTime);
    }
    mRefreshPending = false;
    return hasReadyFrame();
}

bool BufferLayer::onPostComposition(const std::optional<DisplayId>& displayId,
                                    const std::shared_ptr<FenceTime>& glDoneFence,
                                    const std::shared_ptr<FenceTime>& presentFence,
                                    const CompositorTiming& compositorTiming) {
    // mFrameLatencyNeeded is true when a new frame was latched for the
    // composition.
    if (!mFrameLatencyNeeded) return false;

    // Update mFrameEventHistory.
    {
        Mutex::Autolock lock(mFrameEventHistoryMutex);
        mFrameEventHistory.addPostComposition(mCurrentFrameNumber, glDoneFence, presentFence,
                                              compositorTiming);
    }

    // Update mFrameTracker.
    nsecs_t desiredPresentTime = getDesiredPresentTime();
    mFrameTracker.setDesiredPresentTime(desiredPresentTime);

    const int32_t layerID = getSequence();
    mFlinger->mTimeStats->setDesiredTime(layerID, mCurrentFrameNumber, desiredPresentTime);

    std::shared_ptr<FenceTime> frameReadyFence = getCurrentFenceTime();
    if (frameReadyFence->isValid()) {
        mFrameTracker.setFrameReadyFence(std::move(frameReadyFence));
    } else {
        // There was no fence for this frame, so assume that it was ready
        // to be presented at the desired present time.
        mFrameTracker.setFrameReadyTime(desiredPresentTime);
    }

    if (presentFence->isValid()) {
        mFlinger->mTimeStats->setPresentFence(layerID, mCurrentFrameNumber, presentFence);
        mFrameTracker.setActualPresentFence(std::shared_ptr<FenceTime>(presentFence));
    } else if (displayId && mFlinger->getHwComposer().isConnected(*displayId)) {
        // The HWC doesn't support present fences, so use the refresh
        // timestamp instead.
        const nsecs_t actualPresentTime = mFlinger->getHwComposer().getRefreshTimestamp(*displayId);
        mFlinger->mTimeStats->setPresentTime(layerID, mCurrentFrameNumber, actualPresentTime);
        mFrameTracker.setActualPresentTime(actualPresentTime);
    }

    mFrameTracker.advanceFrame();
    mFrameLatencyNeeded = false;
    return true;
}

bool BufferLayer::latchBuffer(bool& recomputeVisibleRegions, nsecs_t latchTime) {
    ATRACE_CALL();

    bool refreshRequired = latchSidebandStream(recomputeVisibleRegions);

    if (refreshRequired) {
        return refreshRequired;
    }

    if (!hasReadyFrame()) {
        return false;
    }

    // if we've already called updateTexImage() without going through
    // a composition step, we have to skip this layer at this point
    // because we cannot call updateTeximage() without a corresponding
    // compositionComplete() call.
    // we'll trigger an update in onPreComposition().
    if (mRefreshPending) {
        return false;
    }

    // If the head buffer's acquire fence hasn't signaled yet, return and
    // try again later
    if (!fenceHasSignaled()) {
        ATRACE_NAME("!fenceHasSignaled()");
        mFlinger->signalLayerUpdate();
        return false;
    }

    // Capture the old state of the layer for comparisons later
    const State& s(getDrawingState());
    const bool oldOpacity = isOpaque(s);
    sp<GraphicBuffer> oldBuffer = mActiveBuffer;

    if (!allTransactionsSignaled()) {
        mFlinger->setTransactionFlags(eTraversalNeeded);
        return false;
    }

    status_t err = updateTexImage(recomputeVisibleRegions, latchTime);
    if (err != NO_ERROR) {
        return false;
    }

    err = updateActiveBuffer();
    if (err != NO_ERROR) {
        return false;
    }

    mBufferLatched = true;

    err = updateFrameNumber(latchTime);
    if (err != NO_ERROR) {
        return false;
    }

    mRefreshPending = true;
    mFrameLatencyNeeded = true;
    if (oldBuffer == nullptr) {
        // the first time we receive a buffer, we need to trigger a
        // geometry invalidation.
        recomputeVisibleRegions = true;
    }

    ui::Dataspace dataSpace = getDrawingDataSpace();
    // translate legacy dataspaces to modern dataspaces
    switch (dataSpace) {
        case ui::Dataspace::SRGB:
            dataSpace = ui::Dataspace::V0_SRGB;
            break;
        case ui::Dataspace::SRGB_LINEAR:
            dataSpace = ui::Dataspace::V0_SRGB_LINEAR;
            break;
        case ui::Dataspace::JFIF:
            dataSpace = ui::Dataspace::V0_JFIF;
            break;
        case ui::Dataspace::BT601_625:
            dataSpace = ui::Dataspace::V0_BT601_625;
            break;
        case ui::Dataspace::BT601_525:
            dataSpace = ui::Dataspace::V0_BT601_525;
            break;
        case ui::Dataspace::BT709:
            dataSpace = ui::Dataspace::V0_BT709;
            break;
        default:
            break;
    }
    mCurrentDataSpace = dataSpace;

    Rect crop(getDrawingCrop());
    const uint32_t transform(getDrawingTransform());
    const uint32_t scalingMode(getDrawingScalingMode());
    const bool transformToDisplayInverse(getTransformToDisplayInverse());
    if ((crop != mCurrentCrop) || (transform != mCurrentTransform) ||
        (scalingMode != mCurrentScalingMode) ||
        (transformToDisplayInverse != mTransformToDisplayInverse)) {
        mCurrentCrop = crop;
        mCurrentTransform = transform;
        mCurrentScalingMode = scalingMode;
        mTransformToDisplayInverse = transformToDisplayInverse;
        recomputeVisibleRegions = true;
    }

    if (oldBuffer != nullptr) {
        uint32_t bufWidth = mActiveBuffer->getWidth();
        uint32_t bufHeight = mActiveBuffer->getHeight();
        if (bufWidth != uint32_t(oldBuffer->width) || bufHeight != uint32_t(oldBuffer->height)) {
            recomputeVisibleRegions = true;
        }
    }

    if (oldOpacity != isOpaque(s)) {
        recomputeVisibleRegions = true;
    }

    // Remove any sync points corresponding to the buffer which was just
    // latched
    {
        Mutex::Autolock lock(mLocalSyncPointMutex);
        auto point = mLocalSyncPoints.begin();
        while (point != mLocalSyncPoints.end()) {
            if (!(*point)->frameIsAvailable() || !(*point)->transactionIsApplied()) {
                // This sync point must have been added since we started
                // latching. Don't drop it yet.
                ++point;
                continue;
            }

            if ((*point)->getFrameNumber() <= mCurrentFrameNumber) {
                std::stringstream ss;
                ss << "Dropping sync point " << (*point)->getFrameNumber();
                ATRACE_NAME(ss.str().c_str());
                point = mLocalSyncPoints.erase(point);
            } else {
                ++point;
            }
        }
    }

    return true;
}

// transaction
void BufferLayer::notifyAvailableFrames() {
    const auto headFrameNumber = getHeadFrameNumber();
    const bool headFenceSignaled = fenceHasSignaled();
    const bool presentTimeIsCurrent = framePresentTimeIsCurrent();
    Mutex::Autolock lock(mLocalSyncPointMutex);
    for (auto& point : mLocalSyncPoints) {
        if (headFrameNumber >= point->getFrameNumber() && headFenceSignaled &&
            presentTimeIsCurrent) {
            point->setFrameAvailable();
            sp<Layer> requestedSyncLayer = point->getRequestedSyncLayer();
            if (requestedSyncLayer) {
                // Need to update the transaction flag to ensure the layer's pending transaction
                // gets applied.
                requestedSyncLayer->setTransactionFlags(eTransactionNeeded);
            }
        }
    }
}

bool BufferLayer::hasReadyFrame() const {
    return hasFrameUpdate() || getSidebandStreamChanged() || getAutoRefresh();
}

uint32_t BufferLayer::getEffectiveScalingMode() const {
    if (mOverrideScalingMode >= 0) {
        return mOverrideScalingMode;
    }

    return mCurrentScalingMode;
}

bool BufferLayer::isProtected() const {
    const sp<GraphicBuffer>& buffer(mActiveBuffer);
    return (buffer != 0) && (buffer->getUsage() & GRALLOC_USAGE_PROTECTED);
}

bool BufferLayer::latchUnsignaledBuffers() {
    static bool propertyLoaded = false;
    static bool latch = false;
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (!propertyLoaded) {
        char value[PROPERTY_VALUE_MAX] = {};
        property_get("debug.sf.latch_unsignaled", value, "0");
        latch = atoi(value);
        propertyLoaded = true;
    }
    return latch;
}

// h/w composer set-up
bool BufferLayer::allTransactionsSignaled() {
    auto headFrameNumber = getHeadFrameNumber();
    bool matchingFramesFound = false;
    bool allTransactionsApplied = true;
    Mutex::Autolock lock(mLocalSyncPointMutex);

    for (auto& point : mLocalSyncPoints) {
        if (point->getFrameNumber() > headFrameNumber) {
            break;
        }
        matchingFramesFound = true;

        if (!point->frameIsAvailable()) {
            // We haven't notified the remote layer that the frame for
            // this point is available yet. Notify it now, and then
            // abort this attempt to latch.
            point->setFrameAvailable();
            allTransactionsApplied = false;
            break;
        }

        allTransactionsApplied = allTransactionsApplied && point->transactionIsApplied();
    }
    return !matchingFramesFound || allTransactionsApplied;
}

// As documented in libhardware header, formats in the range
// 0x100 - 0x1FF are specific to the HAL implementation, and
// are known to have no alpha channel
// TODO: move definition for device-specific range into
// hardware.h, instead of using hard-coded values here.
#define HARDWARE_IS_DEVICE_FORMAT(f) ((f) >= 0x100 && (f) <= 0x1FF)

bool BufferLayer::getOpacityForFormat(uint32_t format) {
    if (HARDWARE_IS_DEVICE_FORMAT(format)) {
        return true;
    }
    switch (format) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case HAL_PIXEL_FORMAT_RGBA_FP16:
        case HAL_PIXEL_FORMAT_RGBA_1010102:
            return false;
    }
    // in all other case, we have no blending (also for unknown formats)
    return true;
}

bool BufferLayer::needsFiltering(const sp<const DisplayDevice>& displayDevice) const {
    // If we are not capturing based on the state of a known display device, we
    // only return mNeedsFiltering
    if (displayDevice == nullptr) {
        return mNeedsFiltering;
    }

    const auto outputLayer = findOutputLayerForDisplay(displayDevice);
    if (outputLayer == nullptr) {
        return mNeedsFiltering;
    }

    const auto& compositionState = outputLayer->getState();
    const auto displayFrame = compositionState.displayFrame;
    const auto sourceCrop = compositionState.sourceCrop;
    return mNeedsFiltering || sourceCrop.getHeight() != displayFrame.getHeight() ||
            sourceCrop.getWidth() != displayFrame.getWidth();
}

uint64_t BufferLayer::getHeadFrameNumber() const {
    if (hasFrameUpdate()) {
        return getFrameNumber();
    } else {
        return mCurrentFrameNumber;
    }
}

Rect BufferLayer::getBufferSize(const State& s) const {
    // If we have a sideband stream, or we are scaling the buffer then return the layer size since
    // we cannot determine the buffer size.
    if ((s.sidebandStream != nullptr) ||
        (getEffectiveScalingMode() != NATIVE_WINDOW_SCALING_MODE_FREEZE)) {
        return Rect(getActiveWidth(s), getActiveHeight(s));
    }

    if (mActiveBuffer == nullptr) {
        return Rect::INVALID_RECT;
    }

    uint32_t bufWidth = mActiveBuffer->getWidth();
    uint32_t bufHeight = mActiveBuffer->getHeight();

    // Undo any transformations on the buffer and return the result.
    if (mCurrentTransform & ui::Transform::ROT_90) {
        std::swap(bufWidth, bufHeight);
    }

    if (getTransformToDisplayInverse()) {
        uint32_t invTransform = DisplayDevice::getPrimaryDisplayOrientationTransform();
        if (invTransform & ui::Transform::ROT_90) {
            std::swap(bufWidth, bufHeight);
        }
    }

    return Rect(bufWidth, bufHeight);
}

std::shared_ptr<compositionengine::Layer> BufferLayer::getCompositionLayer() const {
    return mCompositionLayer;
}

FloatRect BufferLayer::computeSourceBounds(const FloatRect& parentBounds) const {
    const State& s(getDrawingState());

    // If we have a sideband stream, or we are scaling the buffer then return the layer size since
    // we cannot determine the buffer size.
    if ((s.sidebandStream != nullptr) ||
        (getEffectiveScalingMode() != NATIVE_WINDOW_SCALING_MODE_FREEZE)) {
        return FloatRect(0, 0, getActiveWidth(s), getActiveHeight(s));
    }

    if (mActiveBuffer == nullptr) {
        return parentBounds;
    }

    uint32_t bufWidth = mActiveBuffer->getWidth();
    uint32_t bufHeight = mActiveBuffer->getHeight();

    // Undo any transformations on the buffer and return the result.
    if (mCurrentTransform & ui::Transform::ROT_90) {
        std::swap(bufWidth, bufHeight);
    }

    if (getTransformToDisplayInverse()) {
        uint32_t invTransform = DisplayDevice::getPrimaryDisplayOrientationTransform();
        if (invTransform & ui::Transform::ROT_90) {
            std::swap(bufWidth, bufHeight);
        }
    }

    return FloatRect(0, 0, bufWidth, bufHeight);
}

} // namespace android

#if defined(__gl_h_)
#error "don't include gl/gl.h in this file"
#endif

#if defined(__gl2_h_)
#error "don't include gl2/gl2.h in this file"
#endif
