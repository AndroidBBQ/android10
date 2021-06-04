/*
 * Copyright 2019 The Android Open Source Project
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

#include <cstdint>

#include <gui/BufferQueue.h>
#include <gui/HdrMetadata.h>
#include <math/mat4.h>
#include <ui/FloatRect.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicTypes.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/Transform.h>

#include "DisplayHardware/ComposerHal.h"

namespace android::compositionengine {

/*
 * Used by LayerFE::getCompositionState
 */
struct LayerFECompositionState {
    // TODO(lpique): b/121291683 Remove this one we are sure we don't need the
    // value recomputed / set every frame.
    Region geomVisibleRegion;

    /*
     * Geometry state
     */

    bool isSecure{false};
    bool geomUsesSourceCrop{false};
    bool geomBufferUsesDisplayInverseTransform{false};
    uint32_t geomBufferTransform{0};
    ui::Transform geomLayerTransform;
    ui::Transform geomInverseLayerTransform;
    Rect geomBufferSize;
    Rect geomContentCrop;
    Rect geomCrop;
    Region geomActiveTransparentRegion;
    FloatRect geomLayerBounds;

    /*
     * Presentation
     */

    // The blend mode for this layer
    Hwc2::IComposerClient::BlendMode blendMode{Hwc2::IComposerClient::BlendMode::INVALID};

    // The alpha value for this layer
    float alpha{1.f};

    /*
     * Extra metadata
     */

    // The type for this layer
    int type{0};

    // The appId for this layer
    int appId{0};

    /*
     * Per-frame content
     */

    // The type of composition for this layer
    Hwc2::IComposerClient::Composition compositionType{Hwc2::IComposerClient::Composition::INVALID};

    // The buffer and related state
    sp<GraphicBuffer> buffer;
    int bufferSlot{BufferQueue::INVALID_BUFFER_SLOT};
    sp<Fence> acquireFence;
    Region surfaceDamage;

    // The handle to use for a sideband stream for this layer
    sp<NativeHandle> sidebandStream;

    // The color for this layer
    Hwc2::IComposerClient::Color color;

    /*
     * Per-frame presentation state
     */

    // The dataspace for this layer
    ui::Dataspace dataspace{ui::Dataspace::UNKNOWN};

    // The metadata for this layer
    HdrMetadata hdrMetadata;

    // The color transform
    mat4 colorTransform;
};

} // namespace android::compositionengine
