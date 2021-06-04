/*
 * Copyright (C) 2007 The Android Open Source Project
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
#define LOG_TAG "Layer"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "Layer.h"

#include <android-base/stringprintf.h>
#include <compositionengine/Display.h>
#include <compositionengine/Layer.h>
#include <compositionengine/LayerFECompositionState.h>
#include <compositionengine/OutputLayer.h>
#include <compositionengine/impl/LayerCompositionState.h>
#include <compositionengine/impl/OutputLayerCompositionState.h>
#include <cutils/compiler.h>
#include <cutils/native_handle.h>
#include <cutils/properties.h>
#include <gui/BufferItem.h>
#include <gui/LayerDebugInfo.h>
#include <gui/Surface.h>
#include <math.h>
#include <renderengine/RenderEngine.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ui/DebugUtils.h>
#include <ui/GraphicBuffer.h>
#include <ui/PixelFormat.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/NativeHandle.h>
#include <utils/StopWatch.h>
#include <utils/Trace.h>

#include <algorithm>
#include <mutex>
#include <sstream>

#include "BufferLayer.h"
#include "ColorLayer.h"
#include "Colorizer.h"
#include "DisplayDevice.h"
#include "DisplayHardware/HWComposer.h"
#include "LayerProtoHelper.h"
#include "LayerRejecter.h"
#include "MonitoredProducer.h"
#include "SurfaceFlinger.h"
#include "TimeStats/TimeStats.h"

#define DEBUG_RESIZE 0

namespace android {

using base::StringAppendF;

std::atomic<int32_t> Layer::sSequence{1};

Layer::Layer(const LayerCreationArgs& args)
      : mFlinger(args.flinger),
        mName(args.name),
        mClientRef(args.client),
        mWindowType(args.metadata.getInt32(METADATA_WINDOW_TYPE, 0)) {
    mCurrentCrop.makeInvalid();

    uint32_t layerFlags = 0;
    if (args.flags & ISurfaceComposerClient::eHidden) layerFlags |= layer_state_t::eLayerHidden;
    if (args.flags & ISurfaceComposerClient::eOpaque) layerFlags |= layer_state_t::eLayerOpaque;
    if (args.flags & ISurfaceComposerClient::eSecure) layerFlags |= layer_state_t::eLayerSecure;

    mTransactionName = String8("TX - ") + mName;

    mCurrentState.active_legacy.w = args.w;
    mCurrentState.active_legacy.h = args.h;
    mCurrentState.flags = layerFlags;
    mCurrentState.active_legacy.transform.set(0, 0);
    mCurrentState.crop_legacy.makeInvalid();
    mCurrentState.requestedCrop_legacy = mCurrentState.crop_legacy;
    mCurrentState.z = 0;
    mCurrentState.color.a = 1.0f;
    mCurrentState.layerStack = 0;
    mCurrentState.sequence = 0;
    mCurrentState.requested_legacy = mCurrentState.active_legacy;
    mCurrentState.active.w = UINT32_MAX;
    mCurrentState.active.h = UINT32_MAX;
    mCurrentState.active.transform.set(0, 0);
    mCurrentState.transform = 0;
    mCurrentState.transformToDisplayInverse = false;
    mCurrentState.crop.makeInvalid();
    mCurrentState.acquireFence = new Fence(-1);
    mCurrentState.dataspace = ui::Dataspace::UNKNOWN;
    mCurrentState.hdrMetadata.validTypes = 0;
    mCurrentState.surfaceDamageRegion.clear();
    mCurrentState.cornerRadius = 0.0f;
    mCurrentState.api = -1;
    mCurrentState.hasColorTransform = false;
    mCurrentState.colorSpaceAgnostic = false;
    mCurrentState.metadata = args.metadata;

    // drawing state & current state are identical
    mDrawingState = mCurrentState;

    CompositorTiming compositorTiming;
    args.flinger->getCompositorTiming(&compositorTiming);
    mFrameEventHistory.initializeCompositorTiming(compositorTiming);
    mFrameTracker.setDisplayRefreshPeriod(compositorTiming.interval);

    mSchedulerLayerHandle = mFlinger->mScheduler->registerLayer(mName.c_str(), mWindowType);

    mFlinger->onLayerCreated();
}

Layer::~Layer() {
    sp<Client> c(mClientRef.promote());
    if (c != 0) {
        c->detachLayer(this);
    }

    mFrameTracker.logAndResetStats(mName);
    mFlinger->onLayerDestroyed(this);
}

// ---------------------------------------------------------------------------
// callbacks
// ---------------------------------------------------------------------------

/*
 * onLayerDisplayed is only meaningful for BufferLayer, but, is called through
 * Layer.  So, the implementation is done in BufferLayer.  When called on a
 * ColorLayer object, it's essentially a NOP.
 */
void Layer::onLayerDisplayed(const sp<Fence>& /*releaseFence*/) {}

void Layer::removeRemoteSyncPoints() {
    for (auto& point : mRemoteSyncPoints) {
        point->setTransactionApplied();
    }
    mRemoteSyncPoints.clear();

    {
        Mutex::Autolock pendingStateLock(mPendingStateMutex);
        for (State pendingState : mPendingStates) {
            pendingState.barrierLayer_legacy = nullptr;
        }
    }
}

void Layer::removeRelativeZ(const std::vector<Layer*>& layersInTree) {
    if (mCurrentState.zOrderRelativeOf == nullptr) {
        return;
    }

    sp<Layer> strongRelative = mCurrentState.zOrderRelativeOf.promote();
    if (strongRelative == nullptr) {
        setZOrderRelativeOf(nullptr);
        return;
    }

    if (!std::binary_search(layersInTree.begin(), layersInTree.end(), strongRelative.get())) {
        strongRelative->removeZOrderRelative(this);
        mFlinger->setTransactionFlags(eTraversalNeeded);
        setZOrderRelativeOf(nullptr);
    }
}

void Layer::removeFromCurrentState() {
    mRemovedFromCurrentState = true;

    // Since we are no longer reachable from CurrentState SurfaceFlinger
    // will no longer invoke doTransaction for us, and so we will
    // never finish applying transactions. We signal the sync point
    // now so that another layer will not become indefinitely
    // blocked.
    removeRemoteSyncPoints();

    {
    Mutex::Autolock syncLock(mLocalSyncPointMutex);
    for (auto& point : mLocalSyncPoints) {
        point->setFrameAvailable();
    }
    mLocalSyncPoints.clear();
    }

    mFlinger->markLayerPendingRemovalLocked(this);
}

void Layer::onRemovedFromCurrentState() {
    auto layersInTree = getLayersInTree(LayerVector::StateSet::Current);
    std::sort(layersInTree.begin(), layersInTree.end());
    for (const auto& layer : layersInTree) {
        layer->removeFromCurrentState();
        layer->removeRelativeZ(layersInTree);
    }
}

void Layer::addToCurrentState() {
    mRemovedFromCurrentState = false;

    for (const auto& child : mCurrentChildren) {
        child->addToCurrentState();
    }
}

// ---------------------------------------------------------------------------
// set-up
// ---------------------------------------------------------------------------

const String8& Layer::getName() const {
    return mName;
}

bool Layer::getPremultipledAlpha() const {
    return mPremultipliedAlpha;
}

sp<IBinder> Layer::getHandle() {
    Mutex::Autolock _l(mLock);
    if (mGetHandleCalled) {
        ALOGE("Get handle called twice" );
        return nullptr;
    }
    mGetHandleCalled = true;
    return new Handle(mFlinger, this);
}

// ---------------------------------------------------------------------------
// h/w composer set-up
// ---------------------------------------------------------------------------

bool Layer::hasHwcLayer(const sp<const DisplayDevice>& displayDevice) {
    auto outputLayer = findOutputLayerForDisplay(displayDevice);
    LOG_FATAL_IF(!outputLayer);
    return outputLayer->getState().hwc && (*outputLayer->getState().hwc).hwcLayer != nullptr;
}

HWC2::Layer* Layer::getHwcLayer(const sp<const DisplayDevice>& displayDevice) {
    auto outputLayer = findOutputLayerForDisplay(displayDevice);
    if (!outputLayer || !outputLayer->getState().hwc) {
        return nullptr;
    }
    return (*outputLayer->getState().hwc).hwcLayer.get();
}

Rect Layer::getContentCrop() const {
    // this is the crop rectangle that applies to the buffer
    // itself (as opposed to the window)
    Rect crop;
    if (!mCurrentCrop.isEmpty()) {
        // if the buffer crop is defined, we use that
        crop = mCurrentCrop;
    } else if (mActiveBuffer != nullptr) {
        // otherwise we use the whole buffer
        crop = mActiveBuffer->getBounds();
    } else {
        // if we don't have a buffer yet, we use an empty/invalid crop
        crop.makeInvalid();
    }
    return crop;
}

static Rect reduce(const Rect& win, const Region& exclude) {
    if (CC_LIKELY(exclude.isEmpty())) {
        return win;
    }
    if (exclude.isRect()) {
        return win.reduce(exclude.getBounds());
    }
    return Region(win).subtract(exclude).getBounds();
}

static FloatRect reduce(const FloatRect& win, const Region& exclude) {
    if (CC_LIKELY(exclude.isEmpty())) {
        return win;
    }
    // Convert through Rect (by rounding) for lack of FloatRegion
    return Region(Rect{win}).subtract(exclude).getBounds().toFloatRect();
}

Rect Layer::getScreenBounds(bool reduceTransparentRegion) const {
    if (!reduceTransparentRegion) {
        return Rect{mScreenBounds};
    }

    FloatRect bounds = getBounds();
    ui::Transform t = getTransform();
    // Transform to screen space.
    bounds = t.transform(bounds);
    return Rect{bounds};
}

FloatRect Layer::getBounds() const {
    const State& s(getDrawingState());
    return getBounds(getActiveTransparentRegion(s));
}

FloatRect Layer::getBounds(const Region& activeTransparentRegion) const {
    // Subtract the transparent region and snap to the bounds.
    return reduce(mBounds, activeTransparentRegion);
}

ui::Transform Layer::getBufferScaleTransform() const {
    // If the layer is not using NATIVE_WINDOW_SCALING_MODE_FREEZE (e.g.
    // it isFixedSize) then there may be additional scaling not accounted
    // for in the layer transform.
    if (!isFixedSize() || !mActiveBuffer) {
        return {};
    }

    // If the layer is a buffer state layer, the active width and height
    // could be infinite. In that case, return the effective transform.
    const uint32_t activeWidth = getActiveWidth(getDrawingState());
    const uint32_t activeHeight = getActiveHeight(getDrawingState());
    if (activeWidth >= UINT32_MAX && activeHeight >= UINT32_MAX) {
        return {};
    }

    int bufferWidth = mActiveBuffer->getWidth();
    int bufferHeight = mActiveBuffer->getHeight();

    if (mCurrentTransform & NATIVE_WINDOW_TRANSFORM_ROT_90) {
        std::swap(bufferWidth, bufferHeight);
    }

    float sx = activeWidth / static_cast<float>(bufferWidth);
    float sy = activeHeight / static_cast<float>(bufferHeight);

    ui::Transform extraParentScaling;
    extraParentScaling.set(sx, 0, 0, sy);
    return extraParentScaling;
}

ui::Transform Layer::getTransformWithScale(const ui::Transform& bufferScaleTransform) const {
    // We need to mirror this scaling to child surfaces or we will break the contract where WM can
    // treat child surfaces as pixels in the parent surface.
    if (!isFixedSize() || !mActiveBuffer) {
        return mEffectiveTransform;
    }
    return mEffectiveTransform * bufferScaleTransform;
}

FloatRect Layer::getBoundsPreScaling(const ui::Transform& bufferScaleTransform) const {
    // We need the pre scaled layer bounds when computing child bounds to make sure the child is
    // cropped to its parent layer after any buffer transform scaling is applied.
    if (!isFixedSize() || !mActiveBuffer) {
        return mBounds;
    }
    return bufferScaleTransform.inverse().transform(mBounds);
}

void Layer::computeBounds(FloatRect parentBounds, ui::Transform parentTransform) {
    const State& s(getDrawingState());

    // Calculate effective layer transform
    mEffectiveTransform = parentTransform * getActiveTransform(s);

    // Transform parent bounds to layer space
    parentBounds = getActiveTransform(s).inverse().transform(parentBounds);

    // Calculate source bounds
    mSourceBounds = computeSourceBounds(parentBounds);

    // Calculate bounds by croping diplay frame with layer crop and parent bounds
    FloatRect bounds = mSourceBounds;
    const Rect layerCrop = getCrop(s);
    if (!layerCrop.isEmpty()) {
        bounds = mSourceBounds.intersect(layerCrop.toFloatRect());
    }
    bounds = bounds.intersect(parentBounds);

    mBounds = bounds;
    mScreenBounds = mEffectiveTransform.transform(mBounds);

    // Add any buffer scaling to the layer's children.
    ui::Transform bufferScaleTransform = getBufferScaleTransform();
    for (const sp<Layer>& child : mDrawingChildren) {
        child->computeBounds(getBoundsPreScaling(bufferScaleTransform),
                             getTransformWithScale(bufferScaleTransform));
    }
}

Rect Layer::getCroppedBufferSize(const State& s) const {
    Rect size = getBufferSize(s);
    Rect crop = getCrop(s);
    if (!crop.isEmpty() && size.isValid()) {
        size.intersect(crop, &size);
    } else if (!crop.isEmpty()) {
        size = crop;
    }
    return size;
}

void Layer::setupRoundedCornersCropCoordinates(Rect win,
                                               const FloatRect& roundedCornersCrop) const {
    // Translate win by the rounded corners rect coordinates, to have all values in
    // layer coordinate space.
    win.left -= roundedCornersCrop.left;
    win.right -= roundedCornersCrop.left;
    win.top -= roundedCornersCrop.top;
    win.bottom -= roundedCornersCrop.top;
}

void Layer::latchGeometry(compositionengine::LayerFECompositionState& compositionState) const {
    const auto& drawingState{getDrawingState()};
    auto alpha = static_cast<float>(getAlpha());
    auto blendMode = HWC2::BlendMode::None;
    if (!isOpaque(drawingState) || alpha != 1.0f) {
        blendMode =
                mPremultipliedAlpha ? HWC2::BlendMode::Premultiplied : HWC2::BlendMode::Coverage;
    }

    int type = drawingState.metadata.getInt32(METADATA_WINDOW_TYPE, 0);
    int appId = drawingState.metadata.getInt32(METADATA_OWNER_UID, 0);
    sp<Layer> parent = mDrawingParent.promote();
    if (parent.get()) {
        auto& parentState = parent->getDrawingState();
        const int parentType = parentState.metadata.getInt32(METADATA_WINDOW_TYPE, 0);
        const int parentAppId = parentState.metadata.getInt32(METADATA_OWNER_UID, 0);
        if (parentType >= 0 || parentAppId >= 0) {
            type = parentType;
            appId = parentAppId;
        }
    }

    compositionState.geomLayerTransform = getTransform();
    compositionState.geomInverseLayerTransform = compositionState.geomLayerTransform.inverse();
    compositionState.geomBufferSize = getBufferSize(drawingState);
    compositionState.geomContentCrop = getContentCrop();
    compositionState.geomCrop = getCrop(drawingState);
    compositionState.geomBufferTransform = mCurrentTransform;
    compositionState.geomBufferUsesDisplayInverseTransform = getTransformToDisplayInverse();
    compositionState.geomActiveTransparentRegion = getActiveTransparentRegion(drawingState);
    compositionState.geomLayerBounds = mBounds;
    compositionState.geomUsesSourceCrop = usesSourceCrop();
    compositionState.isSecure = isSecure();

    compositionState.blendMode = static_cast<Hwc2::IComposerClient::BlendMode>(blendMode);
    compositionState.alpha = alpha;
    compositionState.type = type;
    compositionState.appId = appId;
}

void Layer::latchCompositionState(compositionengine::LayerFECompositionState& compositionState,
                                  bool includeGeometry) const {
    if (includeGeometry) {
        latchGeometry(compositionState);
    }
}

const char* Layer::getDebugName() const {
    return mName.string();
}

void Layer::forceClientComposition(const sp<DisplayDevice>& display) {
    const auto outputLayer = findOutputLayerForDisplay(display);
    LOG_FATAL_IF(!outputLayer);
    outputLayer->editState().forceClientComposition = true;
}

bool Layer::getForceClientComposition(const sp<DisplayDevice>& display) {
    const auto outputLayer = findOutputLayerForDisplay(display);
    LOG_FATAL_IF(!outputLayer);
    return outputLayer->getState().forceClientComposition;
}

void Layer::updateCursorPosition(const sp<const DisplayDevice>& display) {
    const auto outputLayer = findOutputLayerForDisplay(display);
    LOG_FATAL_IF(!outputLayer);

    if (!outputLayer->getState().hwc ||
        (*outputLayer->getState().hwc).hwcCompositionType !=
                Hwc2::IComposerClient::Composition::CURSOR) {
        return;
    }

    // This gives us only the "orientation" component of the transform
    const State& s(getDrawingState());

    // Apply the layer's transform, followed by the display's global transform
    // Here we're guaranteed that the layer's transform preserves rects
    Rect win = getCroppedBufferSize(s);
    // Subtract the transparent region and snap to the bounds
    Rect bounds = reduce(win, getActiveTransparentRegion(s));
    Rect frame(getTransform().transform(bounds));
    frame.intersect(display->getViewport(), &frame);
    auto& displayTransform = display->getTransform();
    auto position = displayTransform.transform(frame);

    auto error =
            (*outputLayer->getState().hwc).hwcLayer->setCursorPosition(position.left, position.top);
    ALOGE_IF(error != HWC2::Error::None,
             "[%s] Failed to set cursor position "
             "to (%d, %d): %s (%d)",
             mName.string(), position.left, position.top, to_string(error).c_str(),
             static_cast<int32_t>(error));
}

// ---------------------------------------------------------------------------
// drawing...
// ---------------------------------------------------------------------------

bool Layer::prepareClientLayer(const RenderArea& renderArea, const Region& clip,
                               Region& clearRegion, const bool supportProtectedContent,
                               renderengine::LayerSettings& layer) {
    return prepareClientLayer(renderArea, clip, false, clearRegion, supportProtectedContent, layer);
}

bool Layer::prepareClientLayer(const RenderArea& renderArea, bool useIdentityTransform,
                               Region& clearRegion, const bool supportProtectedContent,
                               renderengine::LayerSettings& layer) {
    return prepareClientLayer(renderArea, Region(renderArea.getBounds()), useIdentityTransform,
                              clearRegion, supportProtectedContent, layer);
}

bool Layer::prepareClientLayer(const RenderArea& /*renderArea*/, const Region& /*clip*/,
                               bool useIdentityTransform, Region& /*clearRegion*/,
                               const bool /*supportProtectedContent*/,
                               renderengine::LayerSettings& layer) {
    FloatRect bounds = getBounds();
    half alpha = getAlpha();
    layer.geometry.boundaries = bounds;
    if (useIdentityTransform) {
        layer.geometry.positionTransform = mat4();
    } else {
        const ui::Transform transform = getTransform();
        mat4 m;
        m[0][0] = transform[0][0];
        m[0][1] = transform[0][1];
        m[0][3] = transform[0][2];
        m[1][0] = transform[1][0];
        m[1][1] = transform[1][1];
        m[1][3] = transform[1][2];
        m[3][0] = transform[2][0];
        m[3][1] = transform[2][1];
        m[3][3] = transform[2][2];
        layer.geometry.positionTransform = m;
    }

    if (hasColorTransform()) {
        layer.colorTransform = getColorTransform();
    }

    const auto roundedCornerState = getRoundedCornerState();
    layer.geometry.roundedCornersRadius = roundedCornerState.radius;
    layer.geometry.roundedCornersCrop = roundedCornerState.cropRect;

    layer.alpha = alpha;
    layer.sourceDataspace = mCurrentDataSpace;
    return true;
}

void Layer::setCompositionType(const sp<const DisplayDevice>& display,
                               Hwc2::IComposerClient::Composition type) {
    const auto outputLayer = findOutputLayerForDisplay(display);
    LOG_FATAL_IF(!outputLayer);
    LOG_FATAL_IF(!outputLayer->getState().hwc);
    auto& compositionState = outputLayer->editState();

    ALOGV("setCompositionType(%" PRIx64 ", %s, %d)", ((*compositionState.hwc).hwcLayer)->getId(),
          toString(type).c_str(), 1);
    if ((*compositionState.hwc).hwcCompositionType != type) {
        ALOGV("    actually setting");
        (*compositionState.hwc).hwcCompositionType = type;

        auto error = (*compositionState.hwc)
                             .hwcLayer->setCompositionType(static_cast<HWC2::Composition>(type));
        ALOGE_IF(error != HWC2::Error::None,
                 "[%s] Failed to set "
                 "composition type %s: %s (%d)",
                 mName.string(), toString(type).c_str(), to_string(error).c_str(),
                 static_cast<int32_t>(error));
    }
}

Hwc2::IComposerClient::Composition Layer::getCompositionType(
        const sp<const DisplayDevice>& display) const {
    const auto outputLayer = findOutputLayerForDisplay(display);
    LOG_FATAL_IF(!outputLayer);
    return outputLayer->getState().hwc ? (*outputLayer->getState().hwc).hwcCompositionType
                                       : Hwc2::IComposerClient::Composition::CLIENT;
}

bool Layer::getClearClientTarget(const sp<const DisplayDevice>& display) const {
    const auto outputLayer = findOutputLayerForDisplay(display);
    LOG_FATAL_IF(!outputLayer);
    return outputLayer->getState().clearClientTarget;
}

bool Layer::addSyncPoint(const std::shared_ptr<SyncPoint>& point) {
    if (point->getFrameNumber() <= mCurrentFrameNumber) {
        // Don't bother with a SyncPoint, since we've already latched the
        // relevant frame
        return false;
    }
    if (isRemovedFromCurrentState()) {
        return false;
    }

    Mutex::Autolock lock(mLocalSyncPointMutex);
    mLocalSyncPoints.push_back(point);
    return true;
}

// ----------------------------------------------------------------------------
// local state
// ----------------------------------------------------------------------------

void Layer::computeGeometry(const RenderArea& renderArea,
                            renderengine::Mesh& mesh,
                            bool useIdentityTransform) const {
    const ui::Transform renderAreaTransform(renderArea.getTransform());
    FloatRect win = getBounds();

    vec2 lt = vec2(win.left, win.top);
    vec2 lb = vec2(win.left, win.bottom);
    vec2 rb = vec2(win.right, win.bottom);
    vec2 rt = vec2(win.right, win.top);

    ui::Transform layerTransform = getTransform();
    if (!useIdentityTransform) {
        lt = layerTransform.transform(lt);
        lb = layerTransform.transform(lb);
        rb = layerTransform.transform(rb);
        rt = layerTransform.transform(rt);
    }

    renderengine::Mesh::VertexArray<vec2> position(mesh.getPositionArray<vec2>());
    position[0] = renderAreaTransform.transform(lt);
    position[1] = renderAreaTransform.transform(lb);
    position[2] = renderAreaTransform.transform(rb);
    position[3] = renderAreaTransform.transform(rt);
}

bool Layer::isSecure() const {
    const State& s(mDrawingState);
    return (s.flags & layer_state_t::eLayerSecure);
}

void Layer::setVisibleRegion(const Region& visibleRegion) {
    // always called from main thread
    this->visibleRegion = visibleRegion;
}

void Layer::setCoveredRegion(const Region& coveredRegion) {
    // always called from main thread
    this->coveredRegion = coveredRegion;
}

void Layer::setVisibleNonTransparentRegion(const Region& setVisibleNonTransparentRegion) {
    // always called from main thread
    this->visibleNonTransparentRegion = setVisibleNonTransparentRegion;
}

void Layer::clearVisibilityRegions() {
    visibleRegion.clear();
    visibleNonTransparentRegion.clear();
    coveredRegion.clear();
}

// ----------------------------------------------------------------------------
// transaction
// ----------------------------------------------------------------------------

void Layer::pushPendingState() {
    if (!mCurrentState.modified) {
        return;
    }
    ATRACE_CALL();

    // If this transaction is waiting on the receipt of a frame, generate a sync
    // point and send it to the remote layer.
    // We don't allow installing sync points after we are removed from the current state
    // as we won't be able to signal our end.
    if (mCurrentState.barrierLayer_legacy != nullptr && !isRemovedFromCurrentState()) {
        sp<Layer> barrierLayer = mCurrentState.barrierLayer_legacy.promote();
        if (barrierLayer == nullptr) {
            ALOGE("[%s] Unable to promote barrier Layer.", mName.string());
            // If we can't promote the layer we are intended to wait on,
            // then it is expired or otherwise invalid. Allow this transaction
            // to be applied as per normal (no synchronization).
            mCurrentState.barrierLayer_legacy = nullptr;
        } else {
            auto syncPoint = std::make_shared<SyncPoint>(mCurrentState.frameNumber_legacy, this);
            if (barrierLayer->addSyncPoint(syncPoint)) {
                std::stringstream ss;
                ss << "Adding sync point " << mCurrentState.frameNumber_legacy;
                ATRACE_NAME(ss.str().c_str());
                mRemoteSyncPoints.push_back(std::move(syncPoint));
            } else {
                // We already missed the frame we're supposed to synchronize
                // on, so go ahead and apply the state update
                mCurrentState.barrierLayer_legacy = nullptr;
            }
        }

        // Wake us up to check if the frame has been received
        setTransactionFlags(eTransactionNeeded);
        mFlinger->setTransactionFlags(eTraversalNeeded);
    }
    mPendingStates.push_back(mCurrentState);
    ATRACE_INT(mTransactionName.string(), mPendingStates.size());
}

void Layer::popPendingState(State* stateToCommit) {
    ATRACE_CALL();
    *stateToCommit = mPendingStates[0];

    mPendingStates.removeAt(0);
    ATRACE_INT(mTransactionName.string(), mPendingStates.size());
}

bool Layer::applyPendingStates(State* stateToCommit) {
    bool stateUpdateAvailable = false;
    while (!mPendingStates.empty()) {
        if (mPendingStates[0].barrierLayer_legacy != nullptr) {
            if (mRemoteSyncPoints.empty()) {
                // If we don't have a sync point for this, apply it anyway. It
                // will be visually wrong, but it should keep us from getting
                // into too much trouble.
                ALOGE("[%s] No local sync point found", mName.string());
                popPendingState(stateToCommit);
                stateUpdateAvailable = true;
                continue;
            }

            if (mRemoteSyncPoints.front()->getFrameNumber() !=
                mPendingStates[0].frameNumber_legacy) {
                ALOGE("[%s] Unexpected sync point frame number found", mName.string());

                // Signal our end of the sync point and then dispose of it
                mRemoteSyncPoints.front()->setTransactionApplied();
                mRemoteSyncPoints.pop_front();
                continue;
            }

            if (mRemoteSyncPoints.front()->frameIsAvailable()) {
                ATRACE_NAME("frameIsAvailable");
                // Apply the state update
                popPendingState(stateToCommit);
                stateUpdateAvailable = true;

                // Signal our end of the sync point and then dispose of it
                mRemoteSyncPoints.front()->setTransactionApplied();
                mRemoteSyncPoints.pop_front();
            } else {
                ATRACE_NAME("!frameIsAvailable");
                break;
            }
        } else {
            popPendingState(stateToCommit);
            stateUpdateAvailable = true;
        }
    }

    // If we still have pending updates, wake SurfaceFlinger back up and point
    // it at this layer so we can process them
    if (!mPendingStates.empty()) {
        setTransactionFlags(eTransactionNeeded);
        mFlinger->setTransactionFlags(eTraversalNeeded);
    }

    mCurrentState.modified = false;
    return stateUpdateAvailable;
}

uint32_t Layer::doTransactionResize(uint32_t flags, State* stateToCommit) {
    const State& s(getDrawingState());

    const bool sizeChanged = (stateToCommit->requested_legacy.w != s.requested_legacy.w) ||
            (stateToCommit->requested_legacy.h != s.requested_legacy.h);

    if (sizeChanged) {
        // the size changed, we need to ask our client to request a new buffer
        ALOGD_IF(DEBUG_RESIZE,
                 "doTransaction: geometry (layer=%p '%s'), tr=%02x, scalingMode=%d\n"
                 "  current={ active   ={ wh={%4u,%4u} crop={%4d,%4d,%4d,%4d} (%4d,%4d) }\n"
                 "            requested={ wh={%4u,%4u} }}\n"
                 "  drawing={ active   ={ wh={%4u,%4u} crop={%4d,%4d,%4d,%4d} (%4d,%4d) }\n"
                 "            requested={ wh={%4u,%4u} }}\n",
                 this, getName().string(), mCurrentTransform, getEffectiveScalingMode(),
                 stateToCommit->active_legacy.w, stateToCommit->active_legacy.h,
                 stateToCommit->crop_legacy.left, stateToCommit->crop_legacy.top,
                 stateToCommit->crop_legacy.right, stateToCommit->crop_legacy.bottom,
                 stateToCommit->crop_legacy.getWidth(), stateToCommit->crop_legacy.getHeight(),
                 stateToCommit->requested_legacy.w, stateToCommit->requested_legacy.h,
                 s.active_legacy.w, s.active_legacy.h, s.crop_legacy.left, s.crop_legacy.top,
                 s.crop_legacy.right, s.crop_legacy.bottom, s.crop_legacy.getWidth(),
                 s.crop_legacy.getHeight(), s.requested_legacy.w, s.requested_legacy.h);
    }

    // Don't let Layer::doTransaction update the drawing state
    // if we have a pending resize, unless we are in fixed-size mode.
    // the drawing state will be updated only once we receive a buffer
    // with the correct size.
    //
    // In particular, we want to make sure the clip (which is part
    // of the geometry state) is latched together with the size but is
    // latched immediately when no resizing is involved.
    //
    // If a sideband stream is attached, however, we want to skip this
    // optimization so that transactions aren't missed when a buffer
    // never arrives
    //
    // In the case that we don't have a buffer we ignore other factors
    // and avoid entering the resizePending state. At a high level the
    // resizePending state is to avoid applying the state of the new buffer
    // to the old buffer. However in the state where we don't have an old buffer
    // there is no such concern but we may still be being used as a parent layer.
    const bool resizePending =
            ((stateToCommit->requested_legacy.w != stateToCommit->active_legacy.w) ||
             (stateToCommit->requested_legacy.h != stateToCommit->active_legacy.h)) &&
            (mActiveBuffer != nullptr);
    if (!isFixedSize()) {
        if (resizePending && mSidebandStream == nullptr) {
            flags |= eDontUpdateGeometryState;
        }
    }

    // Here we apply various requested geometry states, depending on our
    // latching configuration. See Layer.h for a detailed discussion of
    // how geometry latching is controlled.
    if (!(flags & eDontUpdateGeometryState)) {
        State& editCurrentState(getCurrentState());

        // If mFreezeGeometryUpdates is true we are in the setGeometryAppliesWithResize
        // mode, which causes attributes which normally latch regardless of scaling mode,
        // to be delayed. We copy the requested state to the active state making sure
        // to respect these rules (again see Layer.h for a detailed discussion).
        //
        // There is an awkward asymmetry in the handling of the crop states in the position
        // states, as can be seen below. Largely this arises from position and transform
        // being stored in the same data structure while having different latching rules.
        // b/38182305
        //
        // Careful that "stateToCommit" and editCurrentState may not begin as equivalent due to
        // applyPendingStates in the presence of deferred transactions.
        if (mFreezeGeometryUpdates) {
            float tx = stateToCommit->active_legacy.transform.tx();
            float ty = stateToCommit->active_legacy.transform.ty();
            stateToCommit->active_legacy = stateToCommit->requested_legacy;
            stateToCommit->active_legacy.transform.set(tx, ty);
            editCurrentState.active_legacy = stateToCommit->active_legacy;
        } else {
            editCurrentState.active_legacy = editCurrentState.requested_legacy;
            stateToCommit->active_legacy = stateToCommit->requested_legacy;
        }
    }

    return flags;
}

uint32_t Layer::doTransaction(uint32_t flags) {
    ATRACE_CALL();

    if (mLayerDetached) {
        return flags;
    }

    if (mChildrenChanged) {
        flags |= eVisibleRegion;
        mChildrenChanged = false;
    }

    pushPendingState();
    State c = getCurrentState();
    if (!applyPendingStates(&c)) {
        return flags;
    }

    flags = doTransactionResize(flags, &c);

    const State& s(getDrawingState());

    if (getActiveGeometry(c) != getActiveGeometry(s)) {
        // invalidate and recompute the visible regions if needed
        flags |= Layer::eVisibleRegion;
    }

    if (c.sequence != s.sequence) {
        // invalidate and recompute the visible regions if needed
        flags |= eVisibleRegion;
        this->contentDirty = true;

        // we may use linear filtering, if the matrix scales us
        const uint8_t type = getActiveTransform(c).getType();
        mNeedsFiltering = (!getActiveTransform(c).preserveRects() || type >= ui::Transform::SCALE);
    }

    if (mCurrentState.inputInfoChanged) {
        flags |= eInputInfoChanged;
        mCurrentState.inputInfoChanged = false;
    }

    // Commit the transaction
    commitTransaction(c);
    mCurrentState.callbackHandles = {};
    return flags;
}

void Layer::commitTransaction(const State& stateToCommit) {
    mDrawingState = stateToCommit;
}

uint32_t Layer::getTransactionFlags(uint32_t flags) {
    return mTransactionFlags.fetch_and(~flags) & flags;
}

uint32_t Layer::setTransactionFlags(uint32_t flags) {
    return mTransactionFlags.fetch_or(flags);
}

bool Layer::setPosition(float x, float y, bool immediate) {
    if (mCurrentState.requested_legacy.transform.tx() == x &&
        mCurrentState.requested_legacy.transform.ty() == y)
        return false;
    mCurrentState.sequence++;

    // We update the requested and active position simultaneously because
    // we want to apply the position portion of the transform matrix immediately,
    // but still delay scaling when resizing a SCALING_MODE_FREEZE layer.
    mCurrentState.requested_legacy.transform.set(x, y);
    if (immediate && !mFreezeGeometryUpdates) {
        // Here we directly update the active state
        // unlike other setters, because we store it within
        // the transform, but use different latching rules.
        // b/38182305
        mCurrentState.active_legacy.transform.set(x, y);
    }
    mFreezeGeometryUpdates = mFreezeGeometryUpdates || !immediate;

    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

bool Layer::setChildLayer(const sp<Layer>& childLayer, int32_t z) {
    ssize_t idx = mCurrentChildren.indexOf(childLayer);
    if (idx < 0) {
        return false;
    }
    if (childLayer->setLayer(z)) {
        mCurrentChildren.removeAt(idx);
        mCurrentChildren.add(childLayer);
        return true;
    }
    return false;
}

bool Layer::setChildRelativeLayer(const sp<Layer>& childLayer,
        const sp<IBinder>& relativeToHandle, int32_t relativeZ) {
    ssize_t idx = mCurrentChildren.indexOf(childLayer);
    if (idx < 0) {
        return false;
    }
    if (childLayer->setRelativeLayer(relativeToHandle, relativeZ)) {
        mCurrentChildren.removeAt(idx);
        mCurrentChildren.add(childLayer);
        return true;
    }
    return false;
}

bool Layer::setLayer(int32_t z) {
    if (mCurrentState.z == z && !usingRelativeZ(LayerVector::StateSet::Current)) return false;
    mCurrentState.sequence++;
    mCurrentState.z = z;
    mCurrentState.modified = true;

    // Discard all relative layering.
    if (mCurrentState.zOrderRelativeOf != nullptr) {
        sp<Layer> strongRelative = mCurrentState.zOrderRelativeOf.promote();
        if (strongRelative != nullptr) {
            strongRelative->removeZOrderRelative(this);
        }
        setZOrderRelativeOf(nullptr);
    }
    setTransactionFlags(eTransactionNeeded);
    return true;
}

void Layer::removeZOrderRelative(const wp<Layer>& relative) {
    mCurrentState.zOrderRelatives.remove(relative);
    mCurrentState.sequence++;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
}

void Layer::addZOrderRelative(const wp<Layer>& relative) {
    mCurrentState.zOrderRelatives.add(relative);
    mCurrentState.modified = true;
    mCurrentState.sequence++;
    setTransactionFlags(eTransactionNeeded);
}

void Layer::setZOrderRelativeOf(const wp<Layer>& relativeOf) {
    mCurrentState.zOrderRelativeOf = relativeOf;
    mCurrentState.sequence++;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
}

bool Layer::setRelativeLayer(const sp<IBinder>& relativeToHandle, int32_t relativeZ) {
    sp<Handle> handle = static_cast<Handle*>(relativeToHandle.get());
    if (handle == nullptr) {
        return false;
    }
    sp<Layer> relative = handle->owner.promote();
    if (relative == nullptr) {
        return false;
    }

    if (mCurrentState.z == relativeZ && usingRelativeZ(LayerVector::StateSet::Current) &&
        mCurrentState.zOrderRelativeOf == relative) {
        return false;
    }

    mCurrentState.sequence++;
    mCurrentState.modified = true;
    mCurrentState.z = relativeZ;

    auto oldZOrderRelativeOf = mCurrentState.zOrderRelativeOf.promote();
    if (oldZOrderRelativeOf != nullptr) {
        oldZOrderRelativeOf->removeZOrderRelative(this);
    }
    setZOrderRelativeOf(relative);
    relative->addZOrderRelative(this);

    setTransactionFlags(eTransactionNeeded);

    return true;
}

bool Layer::setSize(uint32_t w, uint32_t h) {
    if (mCurrentState.requested_legacy.w == w && mCurrentState.requested_legacy.h == h)
        return false;
    mCurrentState.requested_legacy.w = w;
    mCurrentState.requested_legacy.h = h;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);

    // record the new size, from this point on, when the client request
    // a buffer, it'll get the new size.
    setDefaultBufferSize(mCurrentState.requested_legacy.w, mCurrentState.requested_legacy.h);
    return true;
}
bool Layer::setAlpha(float alpha) {
    if (mCurrentState.color.a == alpha) return false;
    mCurrentState.sequence++;
    mCurrentState.color.a = alpha;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

bool Layer::setBackgroundColor(const half3& color, float alpha, ui::Dataspace dataspace) {
    if (!mCurrentState.bgColorLayer && alpha == 0) {
        return false;
    }
    mCurrentState.sequence++;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);

    if (!mCurrentState.bgColorLayer && alpha != 0) {
        // create background color layer if one does not yet exist
        uint32_t flags = ISurfaceComposerClient::eFXSurfaceColor;
        const String8& name = mName + "BackgroundColorLayer";
        mCurrentState.bgColorLayer = new ColorLayer(
                LayerCreationArgs(mFlinger.get(), nullptr, name, 0, 0, flags, LayerMetadata()));

        // add to child list
        addChild(mCurrentState.bgColorLayer);
        mFlinger->mLayersAdded = true;
        // set up SF to handle added color layer
        if (isRemovedFromCurrentState()) {
            mCurrentState.bgColorLayer->onRemovedFromCurrentState();
        }
        mFlinger->setTransactionFlags(eTransactionNeeded);
    } else if (mCurrentState.bgColorLayer && alpha == 0) {
        mCurrentState.bgColorLayer->reparent(nullptr);
        mCurrentState.bgColorLayer = nullptr;
        return true;
    }

    mCurrentState.bgColorLayer->setColor(color);
    mCurrentState.bgColorLayer->setLayer(std::numeric_limits<int32_t>::min());
    mCurrentState.bgColorLayer->setAlpha(alpha);
    mCurrentState.bgColorLayer->setDataspace(dataspace);

    return true;
}

bool Layer::setCornerRadius(float cornerRadius) {
    if (mCurrentState.cornerRadius == cornerRadius) return false;

    mCurrentState.sequence++;
    mCurrentState.cornerRadius = cornerRadius;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

bool Layer::setMatrix(const layer_state_t::matrix22_t& matrix,
        bool allowNonRectPreservingTransforms) {
    ui::Transform t;
    t.set(matrix.dsdx, matrix.dtdy, matrix.dtdx, matrix.dsdy);

    if (!allowNonRectPreservingTransforms && !t.preserveRects()) {
        ALOGW("Attempt to set rotation matrix without permission ACCESS_SURFACE_FLINGER ignored");
        return false;
    }
    mCurrentState.sequence++;
    mCurrentState.requested_legacy.transform.set(matrix.dsdx, matrix.dtdy, matrix.dtdx,
                                                 matrix.dsdy);
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

bool Layer::setTransparentRegionHint(const Region& transparent) {
    mCurrentState.requestedTransparentRegion_legacy = transparent;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}
bool Layer::setFlags(uint8_t flags, uint8_t mask) {
    const uint32_t newFlags = (mCurrentState.flags & ~mask) | (flags & mask);
    if (mCurrentState.flags == newFlags) return false;
    mCurrentState.sequence++;
    mCurrentState.flags = newFlags;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

bool Layer::setCrop_legacy(const Rect& crop, bool immediate) {
    if (mCurrentState.requestedCrop_legacy == crop) return false;
    mCurrentState.sequence++;
    mCurrentState.requestedCrop_legacy = crop;
    if (immediate && !mFreezeGeometryUpdates) {
        mCurrentState.crop_legacy = crop;
    }
    mFreezeGeometryUpdates = mFreezeGeometryUpdates || !immediate;

    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

bool Layer::setOverrideScalingMode(int32_t scalingMode) {
    if (scalingMode == mOverrideScalingMode) return false;
    mOverrideScalingMode = scalingMode;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

bool Layer::setMetadata(const LayerMetadata& data) {
    if (!mCurrentState.metadata.merge(data, true /* eraseEmpty */)) return false;
    mCurrentState.sequence++;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

bool Layer::setLayerStack(uint32_t layerStack) {
    if (mCurrentState.layerStack == layerStack) return false;
    mCurrentState.sequence++;
    mCurrentState.layerStack = layerStack;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

bool Layer::setColorSpaceAgnostic(const bool agnostic) {
    if (mCurrentState.colorSpaceAgnostic == agnostic) {
        return false;
    }
    mCurrentState.sequence++;
    mCurrentState.colorSpaceAgnostic = agnostic;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

uint32_t Layer::getLayerStack() const {
    auto p = mDrawingParent.promote();
    if (p == nullptr) {
        return getDrawingState().layerStack;
    }
    return p->getLayerStack();
}

void Layer::deferTransactionUntil_legacy(const sp<Layer>& barrierLayer, uint64_t frameNumber) {
    ATRACE_CALL();
    mCurrentState.barrierLayer_legacy = barrierLayer;
    mCurrentState.frameNumber_legacy = frameNumber;
    // We don't set eTransactionNeeded, because just receiving a deferral
    // request without any other state updates shouldn't actually induce a delay
    mCurrentState.modified = true;
    pushPendingState();
    mCurrentState.barrierLayer_legacy = nullptr;
    mCurrentState.frameNumber_legacy = 0;
    mCurrentState.modified = false;
}

void Layer::deferTransactionUntil_legacy(const sp<IBinder>& barrierHandle, uint64_t frameNumber) {
    sp<Handle> handle = static_cast<Handle*>(barrierHandle.get());
    deferTransactionUntil_legacy(handle->owner.promote(), frameNumber);
}

// ----------------------------------------------------------------------------
// pageflip handling...
// ----------------------------------------------------------------------------

bool Layer::isHiddenByPolicy() const {
    const State& s(mDrawingState);
    const auto& parent = mDrawingParent.promote();
    if (parent != nullptr && parent->isHiddenByPolicy()) {
        return true;
    }
    if (usingRelativeZ(LayerVector::StateSet::Drawing)) {
        auto zOrderRelativeOf = mDrawingState.zOrderRelativeOf.promote();
        if (zOrderRelativeOf != nullptr) {
            if (zOrderRelativeOf->isHiddenByPolicy()) {
                return true;
            }
        }
    }
    return s.flags & layer_state_t::eLayerHidden;
}

uint32_t Layer::getEffectiveUsage(uint32_t usage) const {
    // TODO: should we do something special if mSecure is set?
    if (mProtectedByApp) {
        // need a hardware-protected path to external video sink
        usage |= GraphicBuffer::USAGE_PROTECTED;
    }
    if (mPotentialCursor) {
        usage |= GraphicBuffer::USAGE_CURSOR;
    }
    usage |= GraphicBuffer::USAGE_HW_COMPOSER;
    return usage;
}

void Layer::updateTransformHint(const sp<const DisplayDevice>& display) const {
    uint32_t orientation = 0;
    // Disable setting transform hint if the debug flag is set.
    if (!mFlinger->mDebugDisableTransformHint) {
        // The transform hint is used to improve performance, but we can
        // only have a single transform hint, it cannot
        // apply to all displays.
        const ui::Transform& planeTransform = display->getTransform();
        orientation = planeTransform.getOrientation();
        if (orientation & ui::Transform::ROT_INVALID) {
            orientation = 0;
        }
    }
    setTransformHint(orientation);
}

// ----------------------------------------------------------------------------
// debugging
// ----------------------------------------------------------------------------

// TODO(marissaw): add new layer state info to layer debugging
LayerDebugInfo Layer::getLayerDebugInfo() const {
    LayerDebugInfo info;
    const State& ds = getDrawingState();
    info.mName = getName();
    sp<Layer> parent = getParent();
    info.mParentName = (parent == nullptr ? std::string("none") : parent->getName().string());
    info.mType = std::string(getTypeId());
    info.mTransparentRegion = ds.activeTransparentRegion_legacy;
    info.mVisibleRegion = visibleRegion;
    info.mSurfaceDamageRegion = surfaceDamageRegion;
    info.mLayerStack = getLayerStack();
    info.mX = ds.active_legacy.transform.tx();
    info.mY = ds.active_legacy.transform.ty();
    info.mZ = ds.z;
    info.mWidth = ds.active_legacy.w;
    info.mHeight = ds.active_legacy.h;
    info.mCrop = ds.crop_legacy;
    info.mColor = ds.color;
    info.mFlags = ds.flags;
    info.mPixelFormat = getPixelFormat();
    info.mDataSpace = static_cast<android_dataspace>(mCurrentDataSpace);
    info.mMatrix[0][0] = ds.active_legacy.transform[0][0];
    info.mMatrix[0][1] = ds.active_legacy.transform[0][1];
    info.mMatrix[1][0] = ds.active_legacy.transform[1][0];
    info.mMatrix[1][1] = ds.active_legacy.transform[1][1];
    {
        sp<const GraphicBuffer> buffer = mActiveBuffer;
        if (buffer != 0) {
            info.mActiveBufferWidth = buffer->getWidth();
            info.mActiveBufferHeight = buffer->getHeight();
            info.mActiveBufferStride = buffer->getStride();
            info.mActiveBufferFormat = buffer->format;
        } else {
            info.mActiveBufferWidth = 0;
            info.mActiveBufferHeight = 0;
            info.mActiveBufferStride = 0;
            info.mActiveBufferFormat = 0;
        }
    }
    info.mNumQueuedFrames = getQueuedFrameCount();
    info.mRefreshPending = isBufferLatched();
    info.mIsOpaque = isOpaque(ds);
    info.mContentDirty = contentDirty;
    return info;
}

void Layer::miniDumpHeader(std::string& result) {
    result.append("-------------------------------");
    result.append("-------------------------------");
    result.append("-----------------------------\n");
    result.append(" Layer name\n");
    result.append("           Z | ");
    result.append(" Window Type | ");
    result.append(" Comp Type | ");
    result.append(" Transform | ");
    result.append("  Disp Frame (LTRB) | ");
    result.append("         Source Crop (LTRB)\n");
    result.append("-------------------------------");
    result.append("-------------------------------");
    result.append("-----------------------------\n");
}

void Layer::miniDump(std::string& result, const sp<DisplayDevice>& displayDevice) const {
    auto outputLayer = findOutputLayerForDisplay(displayDevice);
    if (!outputLayer) {
        return;
    }

    std::string name;
    if (mName.length() > 77) {
        std::string shortened;
        shortened.append(mName.string(), 36);
        shortened.append("[...]");
        shortened.append(mName.string() + (mName.length() - 36), 36);
        name = shortened;
    } else {
        name = std::string(mName.string(), mName.size());
    }

    StringAppendF(&result, " %s\n", name.c_str());

    const State& layerState(getDrawingState());
    const auto& compositionState = outputLayer->getState();

    if (layerState.zOrderRelativeOf != nullptr || mDrawingParent != nullptr) {
        StringAppendF(&result, "  rel %6d | ", layerState.z);
    } else {
        StringAppendF(&result, "  %10d | ", layerState.z);
    }
    StringAppendF(&result, "  %10d | ", mWindowType);
    StringAppendF(&result, "%10s | ", toString(getCompositionType(displayDevice)).c_str());
    StringAppendF(&result, "%10s | ",
                  toString(getCompositionLayer() ? compositionState.bufferTransform
                                                 : static_cast<Hwc2::Transform>(0))
                          .c_str());
    const Rect& frame = compositionState.displayFrame;
    StringAppendF(&result, "%4d %4d %4d %4d | ", frame.left, frame.top, frame.right, frame.bottom);
    const FloatRect& crop = compositionState.sourceCrop;
    StringAppendF(&result, "%6.1f %6.1f %6.1f %6.1f\n", crop.left, crop.top, crop.right,
                  crop.bottom);

    result.append("- - - - - - - - - - - - - - - -");
    result.append("- - - - - - - - - - - - - - - -");
    result.append("- - - - - - - - - - - - - - -\n");
}

void Layer::dumpFrameStats(std::string& result) const {
    mFrameTracker.dumpStats(result);
}

void Layer::clearFrameStats() {
    mFrameTracker.clearStats();
}

void Layer::logFrameStats() {
    mFrameTracker.logAndResetStats(mName);
}

void Layer::getFrameStats(FrameStats* outStats) const {
    mFrameTracker.getStats(outStats);
}

void Layer::dumpFrameEvents(std::string& result) {
    StringAppendF(&result, "- Layer %s (%s, %p)\n", getName().string(), getTypeId(), this);
    Mutex::Autolock lock(mFrameEventHistoryMutex);
    mFrameEventHistory.checkFencesForCompletion();
    mFrameEventHistory.dump(result);
}

void Layer::onDisconnect() {
    Mutex::Autolock lock(mFrameEventHistoryMutex);
    mFrameEventHistory.onDisconnect();
    mFlinger->mTimeStats->onDestroy(getSequence());
}

void Layer::addAndGetFrameTimestamps(const NewFrameEventsEntry* newTimestamps,
                                     FrameEventHistoryDelta* outDelta) {
    if (newTimestamps) {
        mFlinger->mTimeStats->setPostTime(getSequence(), newTimestamps->frameNumber,
                                          getName().c_str(), newTimestamps->postedTime);
    }

    Mutex::Autolock lock(mFrameEventHistoryMutex);
    if (newTimestamps) {
        // If there are any unsignaled fences in the aquire timeline at this
        // point, the previously queued frame hasn't been latched yet. Go ahead
        // and try to get the signal time here so the syscall is taken out of
        // the main thread's critical path.
        mAcquireTimeline.updateSignalTimes();
        // Push the new fence after updating since it's likely still pending.
        mAcquireTimeline.push(newTimestamps->acquireFence);
        mFrameEventHistory.addQueue(*newTimestamps);
    }

    if (outDelta) {
        mFrameEventHistory.getAndResetDelta(outDelta);
    }
}

size_t Layer::getChildrenCount() const {
    size_t count = 0;
    for (const sp<Layer>& child : mCurrentChildren) {
        count += 1 + child->getChildrenCount();
    }
    return count;
}

void Layer::addChild(const sp<Layer>& layer) {
    mChildrenChanged = true;
    setTransactionFlags(eTransactionNeeded);

    mCurrentChildren.add(layer);
    layer->setParent(this);
}

ssize_t Layer::removeChild(const sp<Layer>& layer) {
    mChildrenChanged = true;
    setTransactionFlags(eTransactionNeeded);

    layer->setParent(nullptr);
    return mCurrentChildren.remove(layer);
}

bool Layer::reparentChildren(const sp<IBinder>& newParentHandle) {
    sp<Handle> handle = nullptr;
    sp<Layer> newParent = nullptr;
    if (newParentHandle == nullptr) {
        return false;
    }
    handle = static_cast<Handle*>(newParentHandle.get());
    newParent = handle->owner.promote();
    if (newParent == nullptr) {
        ALOGE("Unable to promote Layer handle");
        return false;
    }

    if (attachChildren()) {
        setTransactionFlags(eTransactionNeeded);
    }
    for (const sp<Layer>& child : mCurrentChildren) {
        newParent->addChild(child);
    }
    mCurrentChildren.clear();

    return true;
}

void Layer::setChildrenDrawingParent(const sp<Layer>& newParent) {
    for (const sp<Layer>& child : mDrawingChildren) {
        child->mDrawingParent = newParent;
        child->computeBounds(newParent->mBounds,
                             newParent->getTransformWithScale(
                                     newParent->getBufferScaleTransform()));
    }
}

bool Layer::reparent(const sp<IBinder>& newParentHandle) {
    bool callSetTransactionFlags = false;

    // While layers are detached, we allow most operations
    // and simply halt performing the actual transaction. However
    // for reparent != null we would enter the mRemovedFromCurrentState
    // state, regardless of whether doTransaction was called, and
    // so we need to prevent the update here.
    if (mLayerDetached && newParentHandle == nullptr) {
        return false;
    }

    sp<Layer> newParent;
    if (newParentHandle != nullptr) {
        auto handle = static_cast<Handle*>(newParentHandle.get());
        newParent = handle->owner.promote();
        if (newParent == nullptr) {
            ALOGE("Unable to promote Layer handle");
            return false;
        }
        if (newParent == this) {
            ALOGE("Invalid attempt to reparent Layer (%s) to itself", getName().c_str());
            return false;
        }
    }

    sp<Layer> parent = getParent();
    if (parent != nullptr) {
        parent->removeChild(this);
    }

    if (newParentHandle != nullptr) {
        newParent->addChild(this);
        if (!newParent->isRemovedFromCurrentState()) {
            addToCurrentState();
        } else {
            onRemovedFromCurrentState();
        }

        if (mLayerDetached) {
            mLayerDetached = false;
            callSetTransactionFlags = true;
        }
    } else {
        onRemovedFromCurrentState();
    }

    if (callSetTransactionFlags || attachChildren()) {
        setTransactionFlags(eTransactionNeeded);
    }
    return true;
}

bool Layer::detachChildren() {
    for (const sp<Layer>& child : mCurrentChildren) {
        sp<Client> parentClient = mClientRef.promote();
        sp<Client> client(child->mClientRef.promote());
        if (client != nullptr && parentClient != client) {
            child->mLayerDetached = true;
            child->detachChildren();
            child->removeRemoteSyncPoints();
        }
    }

    return true;
}

bool Layer::attachChildren() {
    bool changed = false;
    for (const sp<Layer>& child : mCurrentChildren) {
        sp<Client> parentClient = mClientRef.promote();
        sp<Client> client(child->mClientRef.promote());
        if (client != nullptr && parentClient != client) {
            if (child->mLayerDetached) {
                child->mLayerDetached = false;
                changed = true;
            }
            changed |= child->attachChildren();
        }
    }

    return changed;
}

bool Layer::setColorTransform(const mat4& matrix) {
    static const mat4 identityMatrix = mat4();

    if (mCurrentState.colorTransform == matrix) {
        return false;
    }
    ++mCurrentState.sequence;
    mCurrentState.colorTransform = matrix;
    mCurrentState.hasColorTransform = matrix != identityMatrix;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

mat4 Layer::getColorTransform() const {
    mat4 colorTransform = mat4(getDrawingState().colorTransform);
    if (sp<Layer> parent = mDrawingParent.promote(); parent != nullptr) {
        colorTransform = parent->getColorTransform() * colorTransform;
    }
    return colorTransform;
}

bool Layer::hasColorTransform() const {
    bool hasColorTransform = getDrawingState().hasColorTransform;
    if (sp<Layer> parent = mDrawingParent.promote(); parent != nullptr) {
        hasColorTransform = hasColorTransform || parent->hasColorTransform();
    }
    return hasColorTransform;
}

bool Layer::isLegacyDataSpace() const {
    // return true when no higher bits are set
    return !(mCurrentDataSpace & (ui::Dataspace::STANDARD_MASK |
                ui::Dataspace::TRANSFER_MASK | ui::Dataspace::RANGE_MASK));
}

void Layer::setParent(const sp<Layer>& layer) {
    mCurrentParent = layer;
}

int32_t Layer::getZ() const {
    return mDrawingState.z;
}

bool Layer::usingRelativeZ(LayerVector::StateSet stateSet) const {
    const bool useDrawing = stateSet == LayerVector::StateSet::Drawing;
    const State& state = useDrawing ? mDrawingState : mCurrentState;
    return state.zOrderRelativeOf != nullptr;
}

__attribute__((no_sanitize("unsigned-integer-overflow"))) LayerVector Layer::makeTraversalList(
        LayerVector::StateSet stateSet, bool* outSkipRelativeZUsers) {
    LOG_ALWAYS_FATAL_IF(stateSet == LayerVector::StateSet::Invalid,
                        "makeTraversalList received invalid stateSet");
    const bool useDrawing = stateSet == LayerVector::StateSet::Drawing;
    const LayerVector& children = useDrawing ? mDrawingChildren : mCurrentChildren;
    const State& state = useDrawing ? mDrawingState : mCurrentState;

    if (state.zOrderRelatives.size() == 0) {
        *outSkipRelativeZUsers = true;
        return children;
    }

    LayerVector traverse(stateSet);
    for (const wp<Layer>& weakRelative : state.zOrderRelatives) {
        sp<Layer> strongRelative = weakRelative.promote();
        if (strongRelative != nullptr) {
            traverse.add(strongRelative);
        }
    }

    for (const sp<Layer>& child : children) {
        const State& childState = useDrawing ? child->mDrawingState : child->mCurrentState;
        if (childState.zOrderRelativeOf != nullptr) {
            continue;
        }
        traverse.add(child);
    }

    return traverse;
}

/**
 * Negatively signed relatives are before 'this' in Z-order.
 */
void Layer::traverseInZOrder(LayerVector::StateSet stateSet, const LayerVector::Visitor& visitor) {
    // In the case we have other layers who are using a relative Z to us, makeTraversalList will
    // produce a new list for traversing, including our relatives, and not including our children
    // who are relatives of another surface. In the case that there are no relative Z,
    // makeTraversalList returns our children directly to avoid significant overhead.
    // However in this case we need to take the responsibility for filtering children which
    // are relatives of another surface here.
    bool skipRelativeZUsers = false;
    const LayerVector list = makeTraversalList(stateSet, &skipRelativeZUsers);

    size_t i = 0;
    for (; i < list.size(); i++) {
        const auto& relative = list[i];
        if (skipRelativeZUsers && relative->usingRelativeZ(stateSet)) {
            continue;
        }

        if (relative->getZ() >= 0) {
            break;
        }
        relative->traverseInZOrder(stateSet, visitor);
    }

    visitor(this);
    for (; i < list.size(); i++) {
        const auto& relative = list[i];

        if (skipRelativeZUsers && relative->usingRelativeZ(stateSet)) {
            continue;
        }
        relative->traverseInZOrder(stateSet, visitor);
    }
}

/**
 * Positively signed relatives are before 'this' in reverse Z-order.
 */
void Layer::traverseInReverseZOrder(LayerVector::StateSet stateSet,
                                    const LayerVector::Visitor& visitor) {
    // See traverseInZOrder for documentation.
    bool skipRelativeZUsers = false;
    LayerVector list = makeTraversalList(stateSet, &skipRelativeZUsers);

    int32_t i = 0;
    for (i = int32_t(list.size()) - 1; i >= 0; i--) {
        const auto& relative = list[i];

        if (skipRelativeZUsers && relative->usingRelativeZ(stateSet)) {
            continue;
        }

        if (relative->getZ() < 0) {
            break;
        }
        relative->traverseInReverseZOrder(stateSet, visitor);
    }
    visitor(this);
    for (; i >= 0; i--) {
        const auto& relative = list[i];

        if (skipRelativeZUsers && relative->usingRelativeZ(stateSet)) {
            continue;
        }

        relative->traverseInReverseZOrder(stateSet, visitor);
    }
}

LayerVector Layer::makeChildrenTraversalList(LayerVector::StateSet stateSet,
                                             const std::vector<Layer*>& layersInTree) {
    LOG_ALWAYS_FATAL_IF(stateSet == LayerVector::StateSet::Invalid,
                        "makeTraversalList received invalid stateSet");
    const bool useDrawing = stateSet == LayerVector::StateSet::Drawing;
    const LayerVector& children = useDrawing ? mDrawingChildren : mCurrentChildren;
    const State& state = useDrawing ? mDrawingState : mCurrentState;

    LayerVector traverse(stateSet);
    for (const wp<Layer>& weakRelative : state.zOrderRelatives) {
        sp<Layer> strongRelative = weakRelative.promote();
        // Only add relative layers that are also descendents of the top most parent of the tree.
        // If a relative layer is not a descendent, then it should be ignored.
        if (std::binary_search(layersInTree.begin(), layersInTree.end(), strongRelative.get())) {
            traverse.add(strongRelative);
        }
    }

    for (const sp<Layer>& child : children) {
        const State& childState = useDrawing ? child->mDrawingState : child->mCurrentState;
        // If a layer has a relativeOf layer, only ignore if the layer it's relative to is a
        // descendent of the top most parent of the tree. If it's not a descendent, then just add
        // the child here since it won't be added later as a relative.
        if (std::binary_search(layersInTree.begin(), layersInTree.end(),
                               childState.zOrderRelativeOf.promote().get())) {
            continue;
        }
        traverse.add(child);
    }

    return traverse;
}

void Layer::traverseChildrenInZOrderInner(const std::vector<Layer*>& layersInTree,
                                          LayerVector::StateSet stateSet,
                                          const LayerVector::Visitor& visitor) {
    const LayerVector list = makeChildrenTraversalList(stateSet, layersInTree);

    size_t i = 0;
    for (; i < list.size(); i++) {
        const auto& relative = list[i];
        if (relative->getZ() >= 0) {
            break;
        }
        relative->traverseChildrenInZOrderInner(layersInTree, stateSet, visitor);
    }

    visitor(this);
    for (; i < list.size(); i++) {
        const auto& relative = list[i];
        relative->traverseChildrenInZOrderInner(layersInTree, stateSet, visitor);
    }
}

std::vector<Layer*> Layer::getLayersInTree(LayerVector::StateSet stateSet) {
    const bool useDrawing = stateSet == LayerVector::StateSet::Drawing;
    const LayerVector& children = useDrawing ? mDrawingChildren : mCurrentChildren;

    std::vector<Layer*> layersInTree = {this};
    for (size_t i = 0; i < children.size(); i++) {
        const auto& child = children[i];
        std::vector<Layer*> childLayers = child->getLayersInTree(stateSet);
        layersInTree.insert(layersInTree.end(), childLayers.cbegin(), childLayers.cend());
    }

    return layersInTree;
}

void Layer::traverseChildrenInZOrder(LayerVector::StateSet stateSet,
                                     const LayerVector::Visitor& visitor) {
    std::vector<Layer*> layersInTree = getLayersInTree(stateSet);
    std::sort(layersInTree.begin(), layersInTree.end());
    traverseChildrenInZOrderInner(layersInTree, stateSet, visitor);
}

ui::Transform Layer::getTransform() const {
    return mEffectiveTransform;
}

half Layer::getAlpha() const {
    const auto& p = mDrawingParent.promote();

    half parentAlpha = (p != nullptr) ? p->getAlpha() : 1.0_hf;
    return parentAlpha * getDrawingState().color.a;
}

half4 Layer::getColor() const {
    const half4 color(getDrawingState().color);
    return half4(color.r, color.g, color.b, getAlpha());
}

Layer::RoundedCornerState Layer::getRoundedCornerState() const {
    const auto& p = mDrawingParent.promote();
    if (p != nullptr) {
        RoundedCornerState parentState = p->getRoundedCornerState();
        if (parentState.radius > 0) {
            ui::Transform t = getActiveTransform(getDrawingState());
            t = t.inverse();
            parentState.cropRect = t.transform(parentState.cropRect);
            // The rounded corners shader only accepts 1 corner radius for performance reasons,
            // but a transform matrix can define horizontal and vertical scales.
            // Let's take the average between both of them and pass into the shader, practically we
            // never do this type of transformation on windows anyway.
            parentState.radius *= (t[0][0] + t[1][1]) / 2.0f;
            return parentState;
        }
    }
    const float radius = getDrawingState().cornerRadius;
    return radius > 0 && getCrop(getDrawingState()).isValid()
            ? RoundedCornerState(getCrop(getDrawingState()).toFloatRect(), radius)
            : RoundedCornerState();
}

void Layer::commitChildList() {
    for (size_t i = 0; i < mCurrentChildren.size(); i++) {
        const auto& child = mCurrentChildren[i];
        child->commitChildList();
    }
    mDrawingChildren = mCurrentChildren;
    mDrawingParent = mCurrentParent;
}

static wp<Layer> extractLayerFromBinder(const wp<IBinder>& weakBinderHandle) {
    if (weakBinderHandle == nullptr) {
        return nullptr;
    }
    sp<IBinder> binderHandle = weakBinderHandle.promote();
    if (binderHandle == nullptr) {
        return nullptr;
    }
    sp<Layer::Handle> handle = static_cast<Layer::Handle*>(binderHandle.get());
    if (handle == nullptr) {
        return nullptr;
    }
    return handle->owner;
}

void Layer::setInputInfo(const InputWindowInfo& info) {
    mCurrentState.inputInfo = info;
    mCurrentState.touchableRegionCrop = extractLayerFromBinder(info.touchableRegionCropHandle);
    mCurrentState.modified = true;
    mCurrentState.inputInfoChanged = true;
    setTransactionFlags(eTransactionNeeded);
}

void Layer::writeToProto(LayerProto* layerInfo, LayerVector::StateSet stateSet,
                         uint32_t traceFlags) {
    const bool useDrawing = stateSet == LayerVector::StateSet::Drawing;
    const LayerVector& children = useDrawing ? mDrawingChildren : mCurrentChildren;
    const State& state = useDrawing ? mDrawingState : mCurrentState;

    ui::Transform requestedTransform = state.active_legacy.transform;
    ui::Transform transform = getTransform();

    if (traceFlags & SurfaceTracing::TRACE_CRITICAL) {
        layerInfo->set_id(sequence);
        layerInfo->set_name(getName().c_str());
        layerInfo->set_type(String8(getTypeId()));

        for (const auto& child : children) {
            layerInfo->add_children(child->sequence);
        }

        for (const wp<Layer>& weakRelative : state.zOrderRelatives) {
            sp<Layer> strongRelative = weakRelative.promote();
            if (strongRelative != nullptr) {
                layerInfo->add_relatives(strongRelative->sequence);
            }
        }

        LayerProtoHelper::writeToProto(state.activeTransparentRegion_legacy,
                                       [&]() { return layerInfo->mutable_transparent_region(); });
        LayerProtoHelper::writeToProto(visibleRegion,
                                       [&]() { return layerInfo->mutable_visible_region(); });
        LayerProtoHelper::writeToProto(surfaceDamageRegion,
                                       [&]() { return layerInfo->mutable_damage_region(); });

        layerInfo->set_layer_stack(getLayerStack());
        layerInfo->set_z(state.z);

        LayerProtoHelper::writePositionToProto(transform.tx(), transform.ty(),
                                               [&]() { return layerInfo->mutable_position(); });

        LayerProtoHelper::writePositionToProto(requestedTransform.tx(), requestedTransform.ty(),
                                               [&]() {
                                                   return layerInfo->mutable_requested_position();
                                               });

        LayerProtoHelper::writeSizeToProto(state.active_legacy.w, state.active_legacy.h,
                                           [&]() { return layerInfo->mutable_size(); });

        LayerProtoHelper::writeToProto(state.crop_legacy,
                                       [&]() { return layerInfo->mutable_crop(); });
        layerInfo->set_corner_radius(getRoundedCornerState().radius);

        layerInfo->set_is_opaque(isOpaque(state));
        layerInfo->set_invalidate(contentDirty);
        layerInfo->set_is_protected(isProtected());

        // XXX (b/79210409) mCurrentDataSpace is not protected
        layerInfo->set_dataspace(
                dataspaceDetails(static_cast<android_dataspace>(mCurrentDataSpace)));

        layerInfo->set_pixel_format(decodePixelFormat(getPixelFormat()));
        LayerProtoHelper::writeToProto(getColor(), [&]() { return layerInfo->mutable_color(); });
        LayerProtoHelper::writeToProto(state.color,
                                       [&]() { return layerInfo->mutable_requested_color(); });
        layerInfo->set_flags(state.flags);

        LayerProtoHelper::writeToProto(transform, layerInfo->mutable_transform());
        LayerProtoHelper::writeToProto(requestedTransform,
                                       layerInfo->mutable_requested_transform());

        auto parent = useDrawing ? mDrawingParent.promote() : mCurrentParent.promote();
        if (parent != nullptr) {
            layerInfo->set_parent(parent->sequence);
        } else {
            layerInfo->set_parent(-1);
        }

        auto zOrderRelativeOf = state.zOrderRelativeOf.promote();
        if (zOrderRelativeOf != nullptr) {
            layerInfo->set_z_order_relative_of(zOrderRelativeOf->sequence);
        } else {
            layerInfo->set_z_order_relative_of(-1);
        }

        auto buffer = mActiveBuffer;
        if (buffer != nullptr) {
            LayerProtoHelper::writeToProto(buffer,
                                           [&]() { return layerInfo->mutable_active_buffer(); });
            LayerProtoHelper::writeToProto(ui::Transform(mCurrentTransform),
                                           layerInfo->mutable_buffer_transform());
        }

        layerInfo->set_queued_frames(getQueuedFrameCount());
        layerInfo->set_refresh_pending(isBufferLatched());
        layerInfo->set_curr_frame(mCurrentFrameNumber);
        layerInfo->set_effective_scaling_mode(getEffectiveScalingMode());

        for (const auto& pendingState : mPendingStates) {
            auto barrierLayer = pendingState.barrierLayer_legacy.promote();
            if (barrierLayer != nullptr) {
                BarrierLayerProto* barrierLayerProto = layerInfo->add_barrier_layer();
                barrierLayerProto->set_id(barrierLayer->sequence);
                barrierLayerProto->set_frame_number(pendingState.frameNumber_legacy);
            }
        }
        LayerProtoHelper::writeToProto(mBounds, [&]() { return layerInfo->mutable_bounds(); });
    }

    if (traceFlags & SurfaceTracing::TRACE_INPUT) {
        LayerProtoHelper::writeToProto(state.inputInfo, state.touchableRegionCrop,
                                       [&]() { return layerInfo->mutable_input_window_info(); });
    }

    if (traceFlags & SurfaceTracing::TRACE_EXTRA) {
        auto protoMap = layerInfo->mutable_metadata();
        for (const auto& entry : state.metadata.mMap) {
            (*protoMap)[entry.first] = std::string(entry.second.cbegin(), entry.second.cend());
        }
        LayerProtoHelper::writeToProto(mEffectiveTransform,
                                       layerInfo->mutable_effective_transform());
        LayerProtoHelper::writeToProto(mSourceBounds,
                                       [&]() { return layerInfo->mutable_source_bounds(); });
        LayerProtoHelper::writeToProto(mScreenBounds,
                                       [&]() { return layerInfo->mutable_screen_bounds(); });
    }
}

void Layer::writeToProto(LayerProto* layerInfo, const sp<DisplayDevice>& displayDevice,
                         uint32_t traceFlags) {
    auto outputLayer = findOutputLayerForDisplay(displayDevice);
    if (!outputLayer) {
        return;
    }

    writeToProto(layerInfo, LayerVector::StateSet::Drawing, traceFlags);

    const auto& compositionState = outputLayer->getState();

    const Rect& frame = compositionState.displayFrame;
    LayerProtoHelper::writeToProto(frame, [&]() { return layerInfo->mutable_hwc_frame(); });

    const FloatRect& crop = compositionState.sourceCrop;
    LayerProtoHelper::writeToProto(crop, [&]() { return layerInfo->mutable_hwc_crop(); });

    const int32_t transform =
            getCompositionLayer() ? static_cast<int32_t>(compositionState.bufferTransform) : 0;
    layerInfo->set_hwc_transform(transform);

    const int32_t compositionType =
            static_cast<int32_t>(compositionState.hwc ? (*compositionState.hwc).hwcCompositionType
                                                      : Hwc2::IComposerClient::Composition::CLIENT);
    layerInfo->set_hwc_composition_type(compositionType);

    if (std::strcmp(getTypeId(), "BufferLayer") == 0 &&
        static_cast<BufferLayer*>(this)->isProtected()) {
        layerInfo->set_is_protected(true);
    } else {
        layerInfo->set_is_protected(false);
    }
}

bool Layer::isRemovedFromCurrentState() const  {
    return mRemovedFromCurrentState;
}

// Debug helper for b/137560795
#define INT32_MIGHT_OVERFLOW(n) (((n) >= INT32_MAX / 2) || ((n) <= INT32_MIN / 2))

#define RECT_BOUNDS_INVALID(rect)                                               \
    (INT32_MIGHT_OVERFLOW((rect).left) || INT32_MIGHT_OVERFLOW((rect).right) || \
     INT32_MIGHT_OVERFLOW((rect).bottom) || INT32_MIGHT_OVERFLOW((rect).top))

InputWindowInfo Layer::fillInputInfo() {
    InputWindowInfo info = mDrawingState.inputInfo;

    if (info.displayId == ADISPLAY_ID_NONE) {
        info.displayId = getLayerStack();
    }

    ui::Transform t = getTransform();
    const float xScale = t.sx();
    const float yScale = t.sy();
    float xSurfaceInset = info.surfaceInset;
    float ySurfaceInset = info.surfaceInset;
    if (xScale != 1.0f || yScale != 1.0f) {
        info.windowXScale *= 1.0f / xScale;
        info.windowYScale *= 1.0f / yScale;
        info.touchableRegion.scaleSelf(xScale, yScale);
        xSurfaceInset *= xScale;
        ySurfaceInset *= yScale;
    }

    // Transform layer size to screen space and inset it by surface insets.
    // If this is a portal window, set the touchableRegion to the layerBounds.
    Rect layerBounds = info.portalToDisplayId == ADISPLAY_ID_NONE
            ? getBufferSize(getDrawingState())
            : info.touchableRegion.getBounds();
    if (!layerBounds.isValid()) {
        layerBounds = getCroppedBufferSize(getDrawingState());
    }
    layerBounds = t.transform(layerBounds);

    // debug check for b/137560795
    {
        if (RECT_BOUNDS_INVALID(layerBounds)) {
            ALOGE("layer %s bounds are invalid (%" PRIi32 ", %" PRIi32 ", %" PRIi32 ", %" PRIi32
                  ")",
                  mName.c_str(), layerBounds.left, layerBounds.top, layerBounds.right,
                  layerBounds.bottom);
            std::string out;
            getTransform().dump(out, "Transform");
            ALOGE("%s", out.c_str());
            layerBounds.left = layerBounds.top = layerBounds.right = layerBounds.bottom = 0;
        }

        if (INT32_MIGHT_OVERFLOW(xSurfaceInset) || INT32_MIGHT_OVERFLOW(ySurfaceInset)) {
            ALOGE("layer %s surface inset are invalid (%" PRIi32 ", %" PRIi32 ")", mName.c_str(),
                  int32_t(xSurfaceInset), int32_t(ySurfaceInset));
            xSurfaceInset = ySurfaceInset = 0;
        }
    }
    layerBounds.inset(xSurfaceInset, ySurfaceInset, xSurfaceInset, ySurfaceInset);

    // Input coordinate should match the layer bounds.
    info.frameLeft = layerBounds.left;
    info.frameTop = layerBounds.top;
    info.frameRight = layerBounds.right;
    info.frameBottom = layerBounds.bottom;

    // Position the touchable region relative to frame screen location and restrict it to frame
    // bounds.
    info.touchableRegion = info.touchableRegion.translate(info.frameLeft, info.frameTop);
    info.visible = canReceiveInput();

    auto cropLayer = mDrawingState.touchableRegionCrop.promote();
    if (info.replaceTouchableRegionWithCrop) {
        if (cropLayer == nullptr) {
            info.touchableRegion = Region(Rect{mScreenBounds});
        } else {
            info.touchableRegion = Region(Rect{cropLayer->mScreenBounds});
        }
    } else if (cropLayer != nullptr) {
        info.touchableRegion = info.touchableRegion.intersect(Rect{cropLayer->mScreenBounds});
    }

    return info;
}

bool Layer::hasInput() const {
    return mDrawingState.inputInfo.token != nullptr;
}

std::shared_ptr<compositionengine::Layer> Layer::getCompositionLayer() const {
    return nullptr;
}

bool Layer::canReceiveInput() const {
    return !isHiddenByPolicy();
}

compositionengine::OutputLayer* Layer::findOutputLayerForDisplay(
        const sp<const DisplayDevice>& display) const {
    return display->getCompositionDisplay()->getOutputLayerForLayer(getCompositionLayer().get());
}

// ---------------------------------------------------------------------------

}; // namespace android

#if defined(__gl_h_)
#error "don't include gl/gl.h in this file"
#endif

#if defined(__gl2_h_)
#error "don't include gl2/gl2.h in this file"
#endif
