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

#include <compositionengine/impl/CompositionEngine.h>
#include <compositionengine/impl/Display.h>
#include <compositionengine/impl/Layer.h>
#include <renderengine/RenderEngine.h>

#include "DisplayHardware/HWComposer.h"

namespace android::compositionengine {

CompositionEngine::~CompositionEngine() = default;

namespace impl {

std::unique_ptr<compositionengine::CompositionEngine> createCompositionEngine() {
    return std::make_unique<CompositionEngine>();
}

CompositionEngine::CompositionEngine() = default;
CompositionEngine::~CompositionEngine() = default;

std::shared_ptr<compositionengine::Display> CompositionEngine::createDisplay(
        DisplayCreationArgs&& args) {
    return compositionengine::impl::createDisplay(*this, std::move(args));
}

std::shared_ptr<compositionengine::Layer> CompositionEngine::createLayer(LayerCreationArgs&& args) {
    return compositionengine::impl::createLayer(*this, std::move(args));
}

HWComposer& CompositionEngine::getHwComposer() const {
    return *mHwComposer.get();
}

void CompositionEngine::setHwComposer(std::unique_ptr<HWComposer> hwComposer) {
    mHwComposer = std::move(hwComposer);
}

renderengine::RenderEngine& CompositionEngine::getRenderEngine() const {
    return *mRenderEngine.get();
}

void CompositionEngine::setRenderEngine(std::unique_ptr<renderengine::RenderEngine> renderEngine) {
    mRenderEngine = std::move(renderEngine);
}

} // namespace impl
} // namespace android::compositionengine
