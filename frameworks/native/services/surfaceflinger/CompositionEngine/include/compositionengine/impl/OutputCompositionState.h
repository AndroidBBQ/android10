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

#include <math/mat4.h>
#include <ui/GraphicTypes.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/Transform.h>

namespace android {

namespace compositionengine::impl {

struct OutputCompositionState {
    // If false, composition will not per performed for this display
    bool isEnabled{false};

    // If false, this output is not considered secure
    bool isSecure{false};

    // If true, this output displays layers that are internal-only
    bool layerStackInternal{false};

    // The layer stack to display on this display
    uint32_t layerStackId{~0u};

    // The physical space screen bounds
    Rect bounds;

    // The logical to physical transformation to use
    ui::Transform transform;

    // The physical orientation of the display, expressed as ui::Transform
    // orientation flags.
    uint32_t orientation{0};

    // The logical space user visible bounds
    Rect frame;

    // The logical space user viewport rectangle
    Rect viewport;

    // The physical space scissor rectangle
    Rect scissor;

    // If true, RenderEngine filtering should be enabled
    bool needsFiltering{false};

    // The logical coordinates for the dirty region for the display.
    // dirtyRegion is semi-persistent state. Dirty rectangles are added to it
    // by the FE until composition happens, at which point it is cleared.
    Region dirtyRegion;

    // The logical coordinates for the undefined region for the display.
    // The undefined region is internal to the composition engine. It is
    // updated every time the geometry changes.
    Region undefinedRegion;

    // True if the last composition frame had visible layers
    bool lastCompositionHadVisibleLayers{false};

    // The color transform to apply
    android_color_transform_t colorTransform{HAL_COLOR_TRANSFORM_IDENTITY};

    // The color transform matrix to apply, corresponding with colorTransform.
    mat4 colorTransformMat;

    // Current active color mode
    ui::ColorMode colorMode{ui::ColorMode::NATIVE};

    // Current active render intent
    ui::RenderIntent renderIntent{ui::RenderIntent::COLORIMETRIC};

    // Current active dstaspace
    ui::Dataspace dataspace{ui::Dataspace::UNKNOWN};

    // Debugging
    void dump(std::string& result) const;
};

} // namespace compositionengine::impl
} // namespace android
