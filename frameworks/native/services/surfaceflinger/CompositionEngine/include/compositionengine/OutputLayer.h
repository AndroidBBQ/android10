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

#include <optional>
#include <string>

#include <utils/StrongPointer.h>

#include "DisplayHardware/DisplayIdentification.h"

namespace android {

namespace compositionengine {

class CompositionEngine;
class Output;
class Layer;
class LayerFE;

namespace impl {
struct OutputLayerCompositionState;
} // namespace impl

/**
 * An output layer contains the output-dependent composition state for a layer
 */
class OutputLayer {
public:
    virtual ~OutputLayer();

    // Gets the output which owns this output layer
    virtual const Output& getOutput() const = 0;

    // Gets the display-independent layer which this output layer represents
    virtual Layer& getLayer() const = 0;

    // Gets the front-end layer interface this output layer represents
    virtual LayerFE& getLayerFE() const = 0;

    using CompositionState = compositionengine::impl::OutputLayerCompositionState;

    // Gets the raw composition state data for the layer
    // TODO(lpique): Make this protected once it is only internally called.
    virtual const CompositionState& getState() const = 0;

    // Allows mutable access to the raw composition state data for the layer.
    // This is meant to be used by the various functions that are part of the
    // composition process.
    // TODO(lpique): Make this protected once it is only internally called.
    virtual CompositionState& editState() = 0;

    // Recalculates the state of the output layer from the output-independent
    // layer. If includeGeometry is false, the geometry state can be skipped.
    virtual void updateCompositionState(bool includeGeometry) = 0;

    // Writes the geometry state to the HWC, or does nothing if this layer does
    // not use the HWC. If includeGeometry is false, the geometry state can be
    // skipped.
    virtual void writeStateToHWC(bool includeGeometry) const = 0;

    // Debugging
    virtual void dump(std::string& result) const = 0;
};

} // namespace compositionengine
} // namespace android
