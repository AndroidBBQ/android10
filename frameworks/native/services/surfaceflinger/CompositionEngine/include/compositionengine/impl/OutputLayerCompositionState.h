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
#include <optional>
#include <string>

#include <compositionengine/impl/HwcBufferCache.h>
#include <renderengine/Mesh.h>
#include <ui/FloatRect.h>
#include <ui/Rect.h>
#include <ui/Region.h>

#include "DisplayHardware/ComposerHal.h"

namespace HWC2 {
class Layer;
} // namespace HWC2

namespace android {

class HWComposer;

namespace compositionengine::impl {

struct OutputLayerCompositionState {
    // The region of this layer which is visible on this output
    Region visibleRegion;

    // If true, client composition will be used on this output
    bool forceClientComposition{false};

    // If true, when doing client composition, the target may need to be cleared
    bool clearClientTarget{false};

    // The display frame for this layer on this output
    Rect displayFrame;

    // The source crop for this layer on this output
    FloatRect sourceCrop;

    // The buffer transform to use for this layer o on this output.
    Hwc2::Transform bufferTransform{static_cast<Hwc2::Transform>(0)};

    // The Z order index of this layer on this output
    uint32_t z;

    /*
     * HWC state
     */

    struct Hwc {
        explicit Hwc(std::shared_ptr<HWC2::Layer> hwcLayer) : hwcLayer(hwcLayer) {}

        // The HWC Layer backing this layer
        std::shared_ptr<HWC2::Layer> hwcLayer;

        // The HWC composition type for this layer
        Hwc2::IComposerClient::Composition hwcCompositionType{
                Hwc2::IComposerClient::Composition::INVALID};

        // The buffer cache for this layer. This is used to lower the
        // cost of sending reused buffers to the HWC.
        HwcBufferCache hwcBufferCache;
    };

    // The HWC state is optional, and is only set up if there is any potential
    // HWC acceleration possible.
    std::optional<Hwc> hwc;

    // Debugging
    void dump(std::string& result) const;
};

} // namespace compositionengine::impl
} // namespace android
