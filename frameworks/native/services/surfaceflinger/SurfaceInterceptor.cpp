/*
 * Copyright 2016 The Android Open Source Project
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
#undef LOG_TAG
#define LOG_TAG "SurfaceInterceptor"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "Layer.h"
#include "SurfaceFlinger.h"
#include "SurfaceInterceptor.h"

#include <fstream>

#include <android-base/file.h>
#include <log/log.h>
#include <utils/Trace.h>

namespace android {

// ----------------------------------------------------------------------------
// TODO(marissaw): add new layer state values to SurfaceInterceptor

SurfaceInterceptor::~SurfaceInterceptor() = default;

namespace impl {

SurfaceInterceptor::SurfaceInterceptor(SurfaceFlinger* flinger)
    :   mFlinger(flinger)
{
}

void SurfaceInterceptor::enable(const SortedVector<sp<Layer>>& layers,
        const DefaultKeyedVector< wp<IBinder>, DisplayDeviceState>& displays)
{
    if (mEnabled) {
        return;
    }
    ATRACE_CALL();
    mEnabled = true;
    std::lock_guard<std::mutex> protoGuard(mTraceMutex);
    saveExistingDisplaysLocked(displays);
    saveExistingSurfacesLocked(layers);
}

void SurfaceInterceptor::disable() {
    if (!mEnabled) {
        return;
    }
    ATRACE_CALL();
    std::lock_guard<std::mutex> protoGuard(mTraceMutex);
    mEnabled = false;
    status_t err(writeProtoFileLocked());
    ALOGE_IF(err == PERMISSION_DENIED, "Could not save the proto file! Permission denied");
    ALOGE_IF(err == NOT_ENOUGH_DATA, "Could not save the proto file! There are missing fields");
    mTrace.Clear();
}

bool SurfaceInterceptor::isEnabled() {
    return mEnabled;
}

void SurfaceInterceptor::saveExistingDisplaysLocked(
        const DefaultKeyedVector< wp<IBinder>, DisplayDeviceState>& displays)
{
    // Caveat: The initial snapshot does not capture the power mode of the existing displays
    ATRACE_CALL();
    for (size_t i = 0 ; i < displays.size() ; i++) {
        addDisplayCreationLocked(createTraceIncrementLocked(), displays[i]);
        addInitialDisplayStateLocked(createTraceIncrementLocked(), displays[i]);
    }
}

void SurfaceInterceptor::saveExistingSurfacesLocked(const SortedVector<sp<Layer>>& layers) {
    ATRACE_CALL();
    for (const auto& l : layers) {
        l->traverseInZOrder(LayerVector::StateSet::Drawing, [this](Layer* layer) {
            addSurfaceCreationLocked(createTraceIncrementLocked(), layer);
            addInitialSurfaceStateLocked(createTraceIncrementLocked(), layer);
        });
    }
}

void SurfaceInterceptor::addInitialSurfaceStateLocked(Increment* increment,
        const sp<const Layer>& layer)
{
    Transaction* transaction(increment->mutable_transaction());
    const uint32_t layerFlags = layer->getTransactionFlags();
    transaction->set_synchronous(layerFlags & BnSurfaceComposer::eSynchronous);
    transaction->set_animation(layerFlags & BnSurfaceComposer::eAnimation);

    const int32_t layerId(getLayerId(layer));
    addPositionLocked(transaction, layerId, layer->mCurrentState.active_legacy.transform.tx(),
                      layer->mCurrentState.active_legacy.transform.ty());
    addDepthLocked(transaction, layerId, layer->mCurrentState.z);
    addAlphaLocked(transaction, layerId, layer->mCurrentState.color.a);
    addTransparentRegionLocked(transaction, layerId,
                               layer->mCurrentState.activeTransparentRegion_legacy);
    addLayerStackLocked(transaction, layerId, layer->mCurrentState.layerStack);
    addCropLocked(transaction, layerId, layer->mCurrentState.crop_legacy);
    addCornerRadiusLocked(transaction, layerId, layer->mCurrentState.cornerRadius);
    if (layer->mCurrentState.barrierLayer_legacy != nullptr) {
        addDeferTransactionLocked(transaction, layerId,
                                  layer->mCurrentState.barrierLayer_legacy.promote(),
                                  layer->mCurrentState.frameNumber_legacy);
    }
    addOverrideScalingModeLocked(transaction, layerId, layer->getEffectiveScalingMode());
    addFlagsLocked(transaction, layerId, layer->mCurrentState.flags);
}

void SurfaceInterceptor::addInitialDisplayStateLocked(Increment* increment,
        const DisplayDeviceState& display)
{
    Transaction* transaction(increment->mutable_transaction());
    transaction->set_synchronous(false);
    transaction->set_animation(false);

    addDisplaySurfaceLocked(transaction, display.sequenceId, display.surface);
    addDisplayLayerStackLocked(transaction, display.sequenceId, display.layerStack);
    addDisplaySizeLocked(transaction, display.sequenceId, display.width, display.height);
    addDisplayProjectionLocked(transaction, display.sequenceId, display.orientation,
            display.viewport, display.frame);
}

status_t SurfaceInterceptor::writeProtoFileLocked() {
    ATRACE_CALL();
    std::string output;

    if (!mTrace.IsInitialized()) {
        return NOT_ENOUGH_DATA;
    }
    if (!mTrace.SerializeToString(&output)) {
        return PERMISSION_DENIED;
    }
    if (!android::base::WriteStringToFile(output, mOutputFileName, true)) {
        return PERMISSION_DENIED;
    }

    return NO_ERROR;
}

const sp<const Layer> SurfaceInterceptor::getLayer(const wp<const IBinder>& weakHandle) {
    const sp<const IBinder>& handle(weakHandle.promote());
    const auto layerHandle(static_cast<const Layer::Handle*>(handle.get()));
    const sp<const Layer> layer(layerHandle->owner.promote());
    // layer could be a nullptr at this point
    return layer;
}

const std::string SurfaceInterceptor::getLayerName(const sp<const Layer>& layer) {
    return layer->getName().string();
}

int32_t SurfaceInterceptor::getLayerId(const sp<const Layer>& layer) {
    return layer->sequence;
}

Increment* SurfaceInterceptor::createTraceIncrementLocked() {
    Increment* increment(mTrace.add_increment());
    increment->set_time_stamp(systemTime());
    return increment;
}

SurfaceChange* SurfaceInterceptor::createSurfaceChangeLocked(Transaction* transaction,
        int32_t layerId)
{
    SurfaceChange* change(transaction->add_surface_change());
    change->set_id(layerId);
    return change;
}

DisplayChange* SurfaceInterceptor::createDisplayChangeLocked(Transaction* transaction,
        int32_t sequenceId)
{
    DisplayChange* dispChange(transaction->add_display_change());
    dispChange->set_id(sequenceId);
    return dispChange;
}

void SurfaceInterceptor::setProtoRectLocked(Rectangle* protoRect, const Rect& rect) {
    protoRect->set_left(rect.left);
    protoRect->set_top(rect.top);
    protoRect->set_right(rect.right);
    protoRect->set_bottom(rect.bottom);
}

void SurfaceInterceptor::addPositionLocked(Transaction* transaction, int32_t layerId,
        float x, float y)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    PositionChange* posChange(change->mutable_position());
    posChange->set_x(x);
    posChange->set_y(y);
}

void SurfaceInterceptor::addDepthLocked(Transaction* transaction, int32_t layerId,
        uint32_t z)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    LayerChange* depthChange(change->mutable_layer());
    depthChange->set_layer(z);
}

void SurfaceInterceptor::addSizeLocked(Transaction* transaction, int32_t layerId, uint32_t w,
        uint32_t h)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    SizeChange* sizeChange(change->mutable_size());
    sizeChange->set_w(w);
    sizeChange->set_h(h);
}

void SurfaceInterceptor::addAlphaLocked(Transaction* transaction, int32_t layerId,
        float alpha)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    AlphaChange* alphaChange(change->mutable_alpha());
    alphaChange->set_alpha(alpha);
}

void SurfaceInterceptor::addMatrixLocked(Transaction* transaction, int32_t layerId,
        const layer_state_t::matrix22_t& matrix)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    MatrixChange* matrixChange(change->mutable_matrix());
    matrixChange->set_dsdx(matrix.dsdx);
    matrixChange->set_dtdx(matrix.dtdx);
    matrixChange->set_dsdy(matrix.dsdy);
    matrixChange->set_dtdy(matrix.dtdy);
}

void SurfaceInterceptor::addTransparentRegionLocked(Transaction* transaction,
        int32_t layerId, const Region& transRegion)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    TransparentRegionHintChange* transparentChange(change->mutable_transparent_region_hint());

    for (const auto& rect : transRegion) {
        Rectangle* protoRect(transparentChange->add_region());
        setProtoRectLocked(protoRect, rect);
    }
}

void SurfaceInterceptor::addFlagsLocked(Transaction* transaction, int32_t layerId,
        uint8_t flags)
{
    // There can be multiple flags changed
    if (flags & layer_state_t::eLayerHidden) {
        SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
        HiddenFlagChange* flagChange(change->mutable_hidden_flag());
        flagChange->set_hidden_flag(true);
    }
    if (flags & layer_state_t::eLayerOpaque) {
        SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
        OpaqueFlagChange* flagChange(change->mutable_opaque_flag());
        flagChange->set_opaque_flag(true);
    }
    if (flags & layer_state_t::eLayerSecure) {
        SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
        SecureFlagChange* flagChange(change->mutable_secure_flag());
        flagChange->set_secure_flag(true);
    }
}

void SurfaceInterceptor::addLayerStackLocked(Transaction* transaction, int32_t layerId,
        uint32_t layerStack)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    LayerStackChange* layerStackChange(change->mutable_layer_stack());
    layerStackChange->set_layer_stack(layerStack);
}

void SurfaceInterceptor::addCropLocked(Transaction* transaction, int32_t layerId,
        const Rect& rect)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    CropChange* cropChange(change->mutable_crop());
    Rectangle* protoRect(cropChange->mutable_rectangle());
    setProtoRectLocked(protoRect, rect);
}

void SurfaceInterceptor::addCornerRadiusLocked(Transaction* transaction, int32_t layerId,
                                       float cornerRadius)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    CornerRadiusChange* cornerRadiusChange(change->mutable_corner_radius());
    cornerRadiusChange->set_corner_radius(cornerRadius);
}

void SurfaceInterceptor::addDeferTransactionLocked(Transaction* transaction, int32_t layerId,
        const sp<const Layer>& layer, uint64_t frameNumber)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    if (layer == nullptr) {
        ALOGE("An existing layer could not be retrieved with the handle"
                " for the deferred transaction");
        return;
    }
    DeferredTransactionChange* deferTransaction(change->mutable_deferred_transaction());
    deferTransaction->set_layer_id(getLayerId(layer));
    deferTransaction->set_frame_number(frameNumber);
}

void SurfaceInterceptor::addOverrideScalingModeLocked(Transaction* transaction,
        int32_t layerId, int32_t overrideScalingMode)
{
    SurfaceChange* change(createSurfaceChangeLocked(transaction, layerId));
    OverrideScalingModeChange* overrideChange(change->mutable_override_scaling_mode());
    overrideChange->set_override_scaling_mode(overrideScalingMode);
}

void SurfaceInterceptor::addSurfaceChangesLocked(Transaction* transaction,
        const layer_state_t& state)
{
    const sp<const Layer> layer(getLayer(state.surface));
    if (layer == nullptr) {
        ALOGE("An existing layer could not be retrieved with the surface "
                "from the layer_state_t surface in the update transaction");
        return;
    }

    const int32_t layerId(getLayerId(layer));

    if (state.what & layer_state_t::ePositionChanged) {
        addPositionLocked(transaction, layerId, state.x, state.y);
    }
    if (state.what & layer_state_t::eLayerChanged) {
        addDepthLocked(transaction, layerId, state.z);
    }
    if (state.what & layer_state_t::eSizeChanged) {
        addSizeLocked(transaction, layerId, state.w, state.h);
    }
    if (state.what & layer_state_t::eAlphaChanged) {
        addAlphaLocked(transaction, layerId, state.alpha);
    }
    if (state.what & layer_state_t::eMatrixChanged) {
        addMatrixLocked(transaction, layerId, state.matrix);
    }
    if (state.what & layer_state_t::eTransparentRegionChanged) {
        addTransparentRegionLocked(transaction, layerId, state.transparentRegion);
    }
    if (state.what & layer_state_t::eFlagsChanged) {
        addFlagsLocked(transaction, layerId, state.flags);
    }
    if (state.what & layer_state_t::eLayerStackChanged) {
        addLayerStackLocked(transaction, layerId, state.layerStack);
    }
    if (state.what & layer_state_t::eCropChanged_legacy) {
        addCropLocked(transaction, layerId, state.crop_legacy);
    }
    if (state.what & layer_state_t::eCornerRadiusChanged) {
        addCornerRadiusLocked(transaction, layerId, state.cornerRadius);
    }
    if (state.what & layer_state_t::eDeferTransaction_legacy) {
        sp<Layer> otherLayer = nullptr;
        if (state.barrierHandle_legacy != nullptr) {
            otherLayer =
                    static_cast<Layer::Handle*>(state.barrierHandle_legacy.get())->owner.promote();
        } else if (state.barrierGbp_legacy != nullptr) {
            auto const& gbp = state.barrierGbp_legacy;
            if (mFlinger->authenticateSurfaceTextureLocked(gbp)) {
                otherLayer = (static_cast<MonitoredProducer*>(gbp.get()))->getLayer();
            } else {
                ALOGE("Attempt to defer transaction to to an unrecognized GraphicBufferProducer");
            }
        }
        addDeferTransactionLocked(transaction, layerId, otherLayer, state.frameNumber_legacy);
    }
    if (state.what & layer_state_t::eOverrideScalingModeChanged) {
        addOverrideScalingModeLocked(transaction, layerId, state.overrideScalingMode);
    }
}

void SurfaceInterceptor::addDisplayChangesLocked(Transaction* transaction,
        const DisplayState& state, int32_t sequenceId)
{
    if (state.what & DisplayState::eSurfaceChanged) {
        addDisplaySurfaceLocked(transaction, sequenceId, state.surface);
    }
    if (state.what & DisplayState::eLayerStackChanged) {
        addDisplayLayerStackLocked(transaction, sequenceId, state.layerStack);
    }
    if (state.what & DisplayState::eDisplaySizeChanged) {
        addDisplaySizeLocked(transaction, sequenceId, state.width, state.height);
    }
    if (state.what & DisplayState::eDisplayProjectionChanged) {
        addDisplayProjectionLocked(transaction, sequenceId, state.orientation, state.viewport,
                state.frame);
    }
}

void SurfaceInterceptor::addTransactionLocked(Increment* increment,
        const Vector<ComposerState>& stateUpdates,
        const DefaultKeyedVector< wp<IBinder>, DisplayDeviceState>& displays,
        const Vector<DisplayState>& changedDisplays, uint32_t transactionFlags)
{
    Transaction* transaction(increment->mutable_transaction());
    transaction->set_synchronous(transactionFlags & BnSurfaceComposer::eSynchronous);
    transaction->set_animation(transactionFlags & BnSurfaceComposer::eAnimation);
    for (const auto& compState: stateUpdates) {
        addSurfaceChangesLocked(transaction, compState.state);
    }
    for (const auto& disp: changedDisplays) {
        ssize_t dpyIdx = displays.indexOfKey(disp.token);
        if (dpyIdx >= 0) {
            const DisplayDeviceState& dispState(displays.valueAt(dpyIdx));
            addDisplayChangesLocked(transaction, disp, dispState.sequenceId);
        }
    }
}

void SurfaceInterceptor::addSurfaceCreationLocked(Increment* increment,
        const sp<const Layer>& layer)
{
    SurfaceCreation* creation(increment->mutable_surface_creation());
    creation->set_id(getLayerId(layer));
    creation->set_name(getLayerName(layer));
    creation->set_w(layer->mCurrentState.active_legacy.w);
    creation->set_h(layer->mCurrentState.active_legacy.h);
}

void SurfaceInterceptor::addSurfaceDeletionLocked(Increment* increment,
        const sp<const Layer>& layer)
{
    SurfaceDeletion* deletion(increment->mutable_surface_deletion());
    deletion->set_id(getLayerId(layer));
}

void SurfaceInterceptor::addBufferUpdateLocked(Increment* increment, const sp<const Layer>& layer,
        uint32_t width, uint32_t height, uint64_t frameNumber)
{
    BufferUpdate* update(increment->mutable_buffer_update());
    update->set_id(getLayerId(layer));
    update->set_w(width);
    update->set_h(height);
    update->set_frame_number(frameNumber);
}

void SurfaceInterceptor::addVSyncUpdateLocked(Increment* increment, nsecs_t timestamp) {
    VSyncEvent* event(increment->mutable_vsync_event());
    event->set_when(timestamp);
}

void SurfaceInterceptor::addDisplaySurfaceLocked(Transaction* transaction, int32_t sequenceId,
        const sp<const IGraphicBufferProducer>& surface)
{
    if (surface == nullptr) {
        return;
    }
    uint64_t bufferQueueId = 0;
    status_t err(surface->getUniqueId(&bufferQueueId));
    if (err == NO_ERROR) {
        DisplayChange* dispChange(createDisplayChangeLocked(transaction, sequenceId));
        DispSurfaceChange* surfaceChange(dispChange->mutable_surface());
        surfaceChange->set_buffer_queue_id(bufferQueueId);
        surfaceChange->set_buffer_queue_name(surface->getConsumerName().string());
    }
    else {
        ALOGE("invalid graphic buffer producer received while tracing a display change (%s)",
                strerror(-err));
    }
}

void SurfaceInterceptor::addDisplayLayerStackLocked(Transaction* transaction,
        int32_t sequenceId, uint32_t layerStack)
{
    DisplayChange* dispChange(createDisplayChangeLocked(transaction, sequenceId));
    LayerStackChange* layerStackChange(dispChange->mutable_layer_stack());
    layerStackChange->set_layer_stack(layerStack);
}

void SurfaceInterceptor::addDisplaySizeLocked(Transaction* transaction, int32_t sequenceId,
        uint32_t w, uint32_t h)
{
    DisplayChange* dispChange(createDisplayChangeLocked(transaction, sequenceId));
    SizeChange* sizeChange(dispChange->mutable_size());
    sizeChange->set_w(w);
    sizeChange->set_h(h);
}

void SurfaceInterceptor::addDisplayProjectionLocked(Transaction* transaction,
        int32_t sequenceId, int32_t orientation, const Rect& viewport, const Rect& frame)
{
    DisplayChange* dispChange(createDisplayChangeLocked(transaction, sequenceId));
    ProjectionChange* projectionChange(dispChange->mutable_projection());
    projectionChange->set_orientation(orientation);
    Rectangle* viewportRect(projectionChange->mutable_viewport());
    setProtoRectLocked(viewportRect, viewport);
    Rectangle* frameRect(projectionChange->mutable_frame());
    setProtoRectLocked(frameRect, frame);
}

void SurfaceInterceptor::addDisplayCreationLocked(Increment* increment,
        const DisplayDeviceState& info)
{
    DisplayCreation* creation(increment->mutable_display_creation());
    creation->set_id(info.sequenceId);
    creation->set_name(info.displayName);
    creation->set_is_secure(info.isSecure);
    if (info.displayId) {
        creation->set_display_id(info.displayId->value);
    }
}

void SurfaceInterceptor::addDisplayDeletionLocked(Increment* increment, int32_t sequenceId) {
    DisplayDeletion* deletion(increment->mutable_display_deletion());
    deletion->set_id(sequenceId);
}

void SurfaceInterceptor::addPowerModeUpdateLocked(Increment* increment, int32_t sequenceId,
        int32_t mode)
{
    PowerModeUpdate* powerModeUpdate(increment->mutable_power_mode_update());
    powerModeUpdate->set_id(sequenceId);
    powerModeUpdate->set_mode(mode);
}

void SurfaceInterceptor::saveTransaction(const Vector<ComposerState>& stateUpdates,
        const DefaultKeyedVector< wp<IBinder>, DisplayDeviceState>& displays,
        const Vector<DisplayState>& changedDisplays, uint32_t flags)
{
    if (!mEnabled || (stateUpdates.size() <= 0 && changedDisplays.size() <= 0)) {
        return;
    }
    ATRACE_CALL();
    std::lock_guard<std::mutex> protoGuard(mTraceMutex);
    addTransactionLocked(createTraceIncrementLocked(), stateUpdates, displays, changedDisplays,
            flags);
}

void SurfaceInterceptor::saveSurfaceCreation(const sp<const Layer>& layer) {
    if (!mEnabled || layer == nullptr) {
        return;
    }
    ATRACE_CALL();
    std::lock_guard<std::mutex> protoGuard(mTraceMutex);
    addSurfaceCreationLocked(createTraceIncrementLocked(), layer);
}

void SurfaceInterceptor::saveSurfaceDeletion(const sp<const Layer>& layer) {
    if (!mEnabled || layer == nullptr) {
        return;
    }
    ATRACE_CALL();
    std::lock_guard<std::mutex> protoGuard(mTraceMutex);
    addSurfaceDeletionLocked(createTraceIncrementLocked(), layer);
}

void SurfaceInterceptor::saveBufferUpdate(const sp<const Layer>& layer, uint32_t width,
        uint32_t height, uint64_t frameNumber)
{
    if (!mEnabled || layer == nullptr) {
        return;
    }
    ATRACE_CALL();
    std::lock_guard<std::mutex> protoGuard(mTraceMutex);
    addBufferUpdateLocked(createTraceIncrementLocked(), layer, width, height, frameNumber);
}

void SurfaceInterceptor::saveVSyncEvent(nsecs_t timestamp) {
    if (!mEnabled) {
        return;
    }
    std::lock_guard<std::mutex> protoGuard(mTraceMutex);
    addVSyncUpdateLocked(createTraceIncrementLocked(), timestamp);
}

void SurfaceInterceptor::saveDisplayCreation(const DisplayDeviceState& info) {
    if (!mEnabled) {
        return;
    }
    ATRACE_CALL();
    std::lock_guard<std::mutex> protoGuard(mTraceMutex);
    addDisplayCreationLocked(createTraceIncrementLocked(), info);
}

void SurfaceInterceptor::saveDisplayDeletion(int32_t sequenceId) {
    if (!mEnabled) {
        return;
    }
    ATRACE_CALL();
    std::lock_guard<std::mutex> protoGuard(mTraceMutex);
    addDisplayDeletionLocked(createTraceIncrementLocked(), sequenceId);
}

void SurfaceInterceptor::savePowerModeUpdate(int32_t sequenceId, int32_t mode) {
    if (!mEnabled) {
        return;
    }
    ATRACE_CALL();
    std::lock_guard<std::mutex> protoGuard(mTraceMutex);
    addPowerModeUpdateLocked(createTraceIncrementLocked(), sequenceId, mode);
}

} // namespace impl
} // namespace android
