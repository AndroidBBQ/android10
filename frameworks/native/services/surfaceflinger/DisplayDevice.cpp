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

// #define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "DisplayDevice"

#include <android-base/stringprintf.h>
#include <compositionengine/CompositionEngine.h>
#include <compositionengine/Display.h>
#include <compositionengine/DisplayColorProfile.h>
#include <compositionengine/DisplayColorProfileCreationArgs.h>
#include <compositionengine/DisplayCreationArgs.h>
#include <compositionengine/DisplaySurface.h>
#include <compositionengine/RenderSurface.h>
#include <compositionengine/RenderSurfaceCreationArgs.h>
#include <compositionengine/impl/OutputCompositionState.h>
#include <configstore/Utils.h>
#include <log/log.h>
#include <system/window.h>
#include <ui/GraphicTypes.h>

#include "DisplayDevice.h"
#include "Layer.h"
#include "SurfaceFlinger.h"

namespace android {

using android::base::StringAppendF;

/*
 * Initialize the display to the specified values.
 *
 */

uint32_t DisplayDevice::sPrimaryDisplayOrientation = 0;

DisplayDeviceCreationArgs::DisplayDeviceCreationArgs(const sp<SurfaceFlinger>& flinger,
                                                     const wp<IBinder>& displayToken,
                                                     const std::optional<DisplayId>& displayId)
      : flinger(flinger), displayToken(displayToken), displayId(displayId) {}

DisplayDevice::DisplayDevice(DisplayDeviceCreationArgs&& args)
      : mFlinger(args.flinger),
        mDisplayToken(args.displayToken),
        mSequenceId(args.sequenceId),
        mDisplayInstallOrientation(args.displayInstallOrientation),
        mCompositionDisplay{mFlinger->getCompositionEngine().createDisplay(
                compositionengine::DisplayCreationArgs{args.isSecure, args.isVirtual,
                                                       args.displayId})},
        mIsVirtual(args.isVirtual),
        mOrientation(),
        mActiveConfig(0),
        mIsPrimary(args.isPrimary) {
    mCompositionDisplay->createRenderSurface(
            compositionengine::RenderSurfaceCreationArgs{ANativeWindow_getWidth(
                                                                 args.nativeWindow.get()),
                                                         ANativeWindow_getHeight(
                                                                 args.nativeWindow.get()),
                                                         args.nativeWindow, args.displaySurface});

    mCompositionDisplay->createDisplayColorProfile(
            compositionengine::DisplayColorProfileCreationArgs{args.hasWideColorGamut,
                                                               std::move(args.hdrCapabilities),
                                                               args.supportedPerFrameMetadata,
                                                               args.hwcColorModes});

    if (!mCompositionDisplay->isValid()) {
        ALOGE("Composition Display did not validate!");
    }

    mCompositionDisplay->getRenderSurface()->initialize();

    setPowerMode(args.initialPowerMode);

    // initialize the display orientation transform.
    setProjection(DisplayState::eOrientationDefault, Rect::INVALID_RECT, Rect::INVALID_RECT);
}

DisplayDevice::~DisplayDevice() = default;

void DisplayDevice::disconnect() {
    mCompositionDisplay->disconnect();
}

int DisplayDevice::getWidth() const {
    return mCompositionDisplay->getState().bounds.getWidth();
}

int DisplayDevice::getHeight() const {
    return mCompositionDisplay->getState().bounds.getHeight();
}

void DisplayDevice::setDisplayName(const std::string& displayName) {
    if (!displayName.empty()) {
        // never override the name with an empty name
        mDisplayName = displayName;
        mCompositionDisplay->setName(displayName);
    }
}

uint32_t DisplayDevice::getPageFlipCount() const {
    return mCompositionDisplay->getRenderSurface()->getPageFlipCount();
}

// ----------------------------------------------------------------------------

void DisplayDevice::setVisibleLayersSortedByZ(const Vector< sp<Layer> >& layers) {
    mVisibleLayersSortedByZ = layers;
}

const Vector< sp<Layer> >& DisplayDevice::getVisibleLayersSortedByZ() const {
    return mVisibleLayersSortedByZ;
}

void DisplayDevice::setLayersNeedingFences(const Vector< sp<Layer> >& layers) {
    mLayersNeedingFences = layers;
}

const Vector< sp<Layer> >& DisplayDevice::getLayersNeedingFences() const {
    return mLayersNeedingFences;
}

// ----------------------------------------------------------------------------
void DisplayDevice::setPowerMode(int mode) {
    mPowerMode = mode;
    getCompositionDisplay()->setCompositionEnabled(mPowerMode != HWC_POWER_MODE_OFF);
}

int DisplayDevice::getPowerMode()  const {
    return mPowerMode;
}

bool DisplayDevice::isPoweredOn() const {
    return mPowerMode != HWC_POWER_MODE_OFF;
}

// ----------------------------------------------------------------------------
void DisplayDevice::setActiveConfig(int mode) {
    mActiveConfig = mode;
}

int DisplayDevice::getActiveConfig()  const {
    return mActiveConfig;
}

// ----------------------------------------------------------------------------

ui::Dataspace DisplayDevice::getCompositionDataSpace() const {
    return mCompositionDisplay->getState().dataspace;
}

// ----------------------------------------------------------------------------

void DisplayDevice::setLayerStack(uint32_t stack) {
    mCompositionDisplay->setLayerStackFilter(stack, isPrimary());
}

// ----------------------------------------------------------------------------

uint32_t DisplayDevice::displayStateOrientationToTransformOrientation(int orientation) {
    switch (orientation) {
    case DisplayState::eOrientationDefault:
        return ui::Transform::ROT_0;
    case DisplayState::eOrientation90:
        return ui::Transform::ROT_90;
    case DisplayState::eOrientation180:
        return ui::Transform::ROT_180;
    case DisplayState::eOrientation270:
        return ui::Transform::ROT_270;
    default:
        return ui::Transform::ROT_INVALID;
    }
}

status_t DisplayDevice::orientationToTransfrom(int orientation, int w, int h, ui::Transform* tr) {
    uint32_t flags = displayStateOrientationToTransformOrientation(orientation);
    if (flags == ui::Transform::ROT_INVALID) {
        return BAD_VALUE;
    }
    tr->set(flags, w, h);
    return NO_ERROR;
}

void DisplayDevice::setDisplaySize(const int newWidth, const int newHeight) {
    mCompositionDisplay->setBounds(ui::Size(newWidth, newHeight));
}

void DisplayDevice::setProjection(int orientation,
        const Rect& newViewport, const Rect& newFrame) {
    Rect viewport(newViewport);
    Rect frame(newFrame);

    mOrientation = orientation;

    const Rect& displayBounds = getCompositionDisplay()->getState().bounds;
    const int w = displayBounds.width();
    const int h = displayBounds.height();

    ui::Transform R;
    DisplayDevice::orientationToTransfrom(orientation, w, h, &R);

    if (!frame.isValid()) {
        // the destination frame can be invalid if it has never been set,
        // in that case we assume the whole display frame.
        frame = Rect(w, h);
    }

    if (viewport.isEmpty()) {
        // viewport can be invalid if it has never been set, in that case
        // we assume the whole display size.
        // it's also invalid to have an empty viewport, so we handle that
        // case in the same way.
        viewport = Rect(w, h);
        if (R.getOrientation() & ui::Transform::ROT_90) {
            // viewport is always specified in the logical orientation
            // of the display (ie: post-rotation).
            std::swap(viewport.right, viewport.bottom);
        }
    }

    ui::Transform TL, TP, S;
    float src_width  = viewport.width();
    float src_height = viewport.height();
    float dst_width  = frame.width();
    float dst_height = frame.height();
    if (src_width != dst_width || src_height != dst_height) {
        float sx = dst_width  / src_width;
        float sy = dst_height / src_height;
        S.set(sx, 0, 0, sy);
    }

    float src_x = viewport.left;
    float src_y = viewport.top;
    float dst_x = frame.left;
    float dst_y = frame.top;
    TL.set(-src_x, -src_y);
    TP.set(dst_x, dst_y);

    // need to take care of primary display rotation for globalTransform
    // for case if the panel is not installed aligned with device orientation
    if (isPrimary()) {
        DisplayDevice::orientationToTransfrom(
                (orientation + mDisplayInstallOrientation) % (DisplayState::eOrientation270 + 1),
                w, h, &R);
    }

    // The viewport and frame are both in the logical orientation.
    // Apply the logical translation, scale to physical size, apply the
    // physical translation and finally rotate to the physical orientation.
    ui::Transform globalTransform = R * TP * S * TL;

    const uint8_t type = globalTransform.getType();
    const bool needsFiltering =
            (!globalTransform.preserveRects() || (type >= ui::Transform::SCALE));

    Rect scissor = globalTransform.transform(viewport);
    if (scissor.isEmpty()) {
        scissor = displayBounds;
    }

    if (isPrimary()) {
        sPrimaryDisplayOrientation = displayStateOrientationToTransformOrientation(orientation);
    }

    getCompositionDisplay()->setProjection(globalTransform,
                                           displayStateOrientationToTransformOrientation(
                                                   orientation),
                                           frame, viewport, scissor, needsFiltering);
}

uint32_t DisplayDevice::getPrimaryDisplayOrientationTransform() {
    return sPrimaryDisplayOrientation;
}

std::string DisplayDevice::getDebugName() const {
    const auto id = getId() ? to_string(*getId()) + ", " : std::string();
    return base::StringPrintf("DisplayDevice{%s%s%s\"%s\"}", id.c_str(),
                              isPrimary() ? "primary, " : "", isVirtual() ? "virtual, " : "",
                              mDisplayName.c_str());
}

void DisplayDevice::dump(std::string& result) const {
    StringAppendF(&result, "+ %s\n", getDebugName().c_str());

    result.append("   ");
    StringAppendF(&result, "powerMode=%d, ", mPowerMode);
    StringAppendF(&result, "activeConfig=%d, ", mActiveConfig);
    StringAppendF(&result, "numLayers=%zu\n", mVisibleLayersSortedByZ.size());
    getCompositionDisplay()->dump(result);
}

bool DisplayDevice::hasRenderIntent(ui::RenderIntent intent) const {
    return mCompositionDisplay->getDisplayColorProfile()->hasRenderIntent(intent);
}

// ----------------------------------------------------------------------------

const std::optional<DisplayId>& DisplayDevice::getId() const {
    return mCompositionDisplay->getId();
}

bool DisplayDevice::isSecure() const {
    return mCompositionDisplay->isSecure();
}

const Rect& DisplayDevice::getBounds() const {
    return mCompositionDisplay->getState().bounds;
}

const Region& DisplayDevice::getUndefinedRegion() const {
    return mCompositionDisplay->getState().undefinedRegion;
}

bool DisplayDevice::needsFiltering() const {
    return mCompositionDisplay->getState().needsFiltering;
}

uint32_t DisplayDevice::getLayerStack() const {
    return mCompositionDisplay->getState().layerStackId;
}

const ui::Transform& DisplayDevice::getTransform() const {
    return mCompositionDisplay->getState().transform;
}

const Rect& DisplayDevice::getViewport() const {
    return mCompositionDisplay->getState().viewport;
}

const Rect& DisplayDevice::getFrame() const {
    return mCompositionDisplay->getState().frame;
}

const Rect& DisplayDevice::getScissor() const {
    return mCompositionDisplay->getState().scissor;
}

bool DisplayDevice::hasWideColorGamut() const {
    return mCompositionDisplay->getDisplayColorProfile()->hasWideColorGamut();
}

bool DisplayDevice::hasHDR10PlusSupport() const {
    return mCompositionDisplay->getDisplayColorProfile()->hasHDR10PlusSupport();
}

bool DisplayDevice::hasHDR10Support() const {
    return mCompositionDisplay->getDisplayColorProfile()->hasHDR10Support();
}

bool DisplayDevice::hasHLGSupport() const {
    return mCompositionDisplay->getDisplayColorProfile()->hasHLGSupport();
}

bool DisplayDevice::hasDolbyVisionSupport() const {
    return mCompositionDisplay->getDisplayColorProfile()->hasDolbyVisionSupport();
}

int DisplayDevice::getSupportedPerFrameMetadata() const {
    return mCompositionDisplay->getDisplayColorProfile()->getSupportedPerFrameMetadata();
}

const HdrCapabilities& DisplayDevice::getHdrCapabilities() const {
    return mCompositionDisplay->getDisplayColorProfile()->getHdrCapabilities();
}

std::atomic<int32_t> DisplayDeviceState::sNextSequenceId(1);

}  // namespace android
