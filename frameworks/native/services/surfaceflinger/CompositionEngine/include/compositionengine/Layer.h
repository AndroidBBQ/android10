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
#include <string>

#include <utils/StrongPointer.h>

namespace android {

typedef int64_t nsecs_t;

namespace compositionengine {

class Display;
class LayerFE;

namespace impl {
struct LayerCompositionState;
} // namespace impl

/**
 * A layer contains the output-independent composition state for a front-end
 * Layer
 */
class Layer {
public:
    virtual ~Layer();

    // Gets the front-end interface for this layer.  Can return nullptr if the
    // front-end layer no longer exists.
    virtual sp<LayerFE> getLayerFE() const = 0;

    using CompositionState = impl::LayerCompositionState;

    // Gets the raw composition state data for the layer
    // TODO(lpique): Make this protected once it is only internally called.
    virtual const CompositionState& getState() const = 0;

    // Allows mutable access to the raw composition state data for the layer.
    // This is meant to be used by the various functions that are part of the
    // composition process.
    // TODO(lpique): Make this protected once it is only internally called.
    virtual CompositionState& editState() = 0;

    // Debugging
    virtual void dump(std::string& result) const = 0;
};

} // namespace compositionengine
} // namespace android
