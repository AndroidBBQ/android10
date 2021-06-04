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

#include <memory>

namespace android {

class HWComposer;

namespace renderengine {
class RenderEngine;
} // namespace renderengine

namespace compositionengine {

class Display;
class Layer;

struct DisplayCreationArgs;
struct LayerCreationArgs;

/**
 * Encapsulates all the interfaces and implementation details for performing
 * display output composition.
 */
class CompositionEngine {
public:
    virtual ~CompositionEngine();

    // Create a composition Display
    virtual std::shared_ptr<Display> createDisplay(DisplayCreationArgs&&) = 0;
    virtual std::shared_ptr<Layer> createLayer(LayerCreationArgs&&) = 0;

    virtual HWComposer& getHwComposer() const = 0;
    virtual void setHwComposer(std::unique_ptr<HWComposer>) = 0;

    virtual renderengine::RenderEngine& getRenderEngine() const = 0;
    virtual void setRenderEngine(std::unique_ptr<renderengine::RenderEngine>) = 0;
};

} // namespace compositionengine
} // namespace android
