/*
 * Copyright 2018 The Android Open Source Project
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

#include <math/mat4.h>
#include <ui/GraphicTypes.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/Transform.h>

namespace android {
namespace renderengine {

// DisplaySettings contains the settings that are applicable when drawing all
// layers for a given display.
struct DisplaySettings {
    // Rectangle describing the physical display. We will project from the
    // logical clip onto this rectangle.
    Rect physicalDisplay = Rect::INVALID_RECT;

    // Rectangle bounded by the x,y- clipping planes in the logical display, so
    // that the orthographic projection matrix can be computed. When
    // constructing this matrix, z-coordinate bound are assumed to be at z=0 and
    // z=1.
    Rect clip = Rect::INVALID_RECT;

    // Global transform to apply to all layers.
    mat4 globalTransform = mat4();

    // Maximum luminance pulled from the display's HDR capabilities.
    float maxLuminance = 1.0f;

    // Output dataspace that will be populated if wide color gamut is used, or
    // DataSpace::UNKNOWN otherwise.
    ui::Dataspace outputDataspace = ui::Dataspace::UNKNOWN;

    // Additional color transform to apply in linear space after transforming
    // to the output dataspace.
    mat4 colorTransform = mat4();

    // Region that will be cleared to (0, 0, 0, 1) prior to rendering.
    // RenderEngine will transform the clearRegion passed in here, by
    // globalTransform, so that it will be in the same coordinate space as the
    // rendered layers.
    Region clearRegion = Region::INVALID_REGION;

    // The orientation of the physical display.
    uint32_t orientation = ui::Transform::ROT_0;
};

} // namespace renderengine
} // namespace android
