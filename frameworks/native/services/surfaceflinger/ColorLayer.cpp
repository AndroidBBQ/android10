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
#define LOG_TAG "ColorLayer"

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include <compositionengine/CompositionEngine.h>
#include <compositionengine/Display.h>
#include <compositionengine/Layer.h>
#include <compositionengine/LayerCreationArgs.h>
#include <compositionengine/OutputLayer.h>
#include <compositionengine/impl/LayerCompositionState.h>
#include <compositionengine/impl/OutputLayerCompositionState.h>
#include <renderengine/RenderEngine.h>
#include <ui/GraphicBuffer.h>
#include <utils/Errors.h>
#include <utils/Log.h>

#include "ColorLayer.h"
#include "DisplayDevice.h"
#include "SurfaceFlinger.h"

namespace android {
// ---------------------------------------------------------------------------

ColorLayer::ColorLayer(const LayerCreationArgs& args)
      : Layer(args),
        mCompositionLayer{mFlinger->getCompositionEngine().createLayer(
                compositionengine::LayerCreationArgs{this})} {}

ColorLayer::~ColorLayer() = default;

bool ColorLayer::prepareClientLayer(const RenderArea& renderArea, const Region& clip,
                                    bool useIdentityTransform, Region& clearRegion,
                                    const bool supportProtectedContent,
                                    renderengine::LayerSettings& layer) {
    Layer::prepareClientLayer(renderArea, clip, useIdentityTransform, clearRegion,
                              supportProtectedContent, layer);
    half4 color(getColor());
    half3 solidColor(color.r, color.g, color.b);
    layer.source.solidColor = solidColor;
    return true;
}

bool ColorLayer::isVisible() const {
    return !isHiddenByPolicy() && getAlpha() > 0.0f;
}

bool ColorLayer::setColor(const half3& color) {
    if (mCurrentState.color.r == color.r && mCurrentState.color.g == color.g &&
        mCurrentState.color.b == color.b) {
        return false;
    }

    mCurrentState.sequence++;
    mCurrentState.color.r = color.r;
    mCurrentState.color.g = color.g;
    mCurrentState.color.b = color.b;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

bool ColorLayer::setDataspace(ui::Dataspace dataspace) {
    if (mCurrentState.dataspace == dataspace) {
        return false;
    }

    mCurrentState.sequence++;
    mCurrentState.dataspace = dataspace;
    mCurrentState.modified = true;
    setTransactionFlags(eTransactionNeeded);
    return true;
}

void ColorLayer::setPerFrameData(const sp<const DisplayDevice>& display,
                                 const ui::Transform& transform, const Rect& viewport,
                                 int32_t /* supportedPerFrameMetadata */,
                                 const ui::Dataspace targetDataspace) {
    RETURN_IF_NO_HWC_LAYER(display);

    Region visible = transform.transform(visibleRegion.intersect(viewport));

    const auto outputLayer = findOutputLayerForDisplay(display);
    LOG_FATAL_IF(!outputLayer || !outputLayer->getState().hwc);

    auto& hwcLayer = (*outputLayer->getState().hwc).hwcLayer;

    auto error = hwcLayer->setVisibleRegion(visible);
    if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to set visible region: %s (%d)", mName.string(),
              to_string(error).c_str(), static_cast<int32_t>(error));
        visible.dump(LOG_TAG);
    }
    outputLayer->editState().visibleRegion = visible;

    setCompositionType(display, Hwc2::IComposerClient::Composition::SOLID_COLOR);

    const ui::Dataspace dataspace =
            isColorSpaceAgnostic() && targetDataspace != ui::Dataspace::UNKNOWN ? targetDataspace
                                                                                : mCurrentDataSpace;
    error = hwcLayer->setDataspace(dataspace);
    if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to set dataspace %d: %s (%d)", mName.string(), dataspace,
              to_string(error).c_str(), static_cast<int32_t>(error));
    }

    auto& layerCompositionState = getCompositionLayer()->editState().frontEnd;
    layerCompositionState.dataspace = mCurrentDataSpace;

    half4 color = getColor();
    error = hwcLayer->setColor({static_cast<uint8_t>(std::round(255.0f * color.r)),
                                static_cast<uint8_t>(std::round(255.0f * color.g)),
                                static_cast<uint8_t>(std::round(255.0f * color.b)), 255});
    if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to set color: %s (%d)", mName.string(), to_string(error).c_str(),
              static_cast<int32_t>(error));
    }
    layerCompositionState.color = {static_cast<uint8_t>(std::round(255.0f * color.r)),
                                   static_cast<uint8_t>(std::round(255.0f * color.g)),
                                   static_cast<uint8_t>(std::round(255.0f * color.b)), 255};

    // Clear out the transform, because it doesn't make sense absent a source buffer
    error = hwcLayer->setTransform(HWC2::Transform::None);
    if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to clear transform: %s (%d)", mName.string(), to_string(error).c_str(),
              static_cast<int32_t>(error));
    }
    outputLayer->editState().bufferTransform = static_cast<Hwc2::Transform>(0);

    error = hwcLayer->setColorTransform(getColorTransform());
    if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to setColorTransform: %s (%d)", mName.string(),
                to_string(error).c_str(), static_cast<int32_t>(error));
    }
    layerCompositionState.colorTransform = getColorTransform();

    error = hwcLayer->setSurfaceDamage(surfaceDamageRegion);
    if (error != HWC2::Error::None) {
        ALOGE("[%s] Failed to set surface damage: %s (%d)", mName.string(),
              to_string(error).c_str(), static_cast<int32_t>(error));
        surfaceDamageRegion.dump(LOG_TAG);
    }
    layerCompositionState.surfaceDamage = surfaceDamageRegion;
}

void ColorLayer::commitTransaction(const State& stateToCommit) {
    Layer::commitTransaction(stateToCommit);
    mCurrentDataSpace = mDrawingState.dataspace;
}

std::shared_ptr<compositionengine::Layer> ColorLayer::getCompositionLayer() const {
    return mCompositionLayer;
}

// ---------------------------------------------------------------------------

}; // namespace android
