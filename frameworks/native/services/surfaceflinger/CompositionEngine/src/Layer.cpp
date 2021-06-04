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

#include <android-base/stringprintf.h>
#include <compositionengine/CompositionEngine.h>
#include <compositionengine/LayerCreationArgs.h>
#include <compositionengine/LayerFE.h>
#include <compositionengine/impl/Layer.h>

namespace android::compositionengine {

Layer::~Layer() = default;

namespace impl {

std::shared_ptr<compositionengine::Layer> createLayer(
        const compositionengine::CompositionEngine& compositionEngine,
        compositionengine::LayerCreationArgs&& args) {
    return std::make_shared<Layer>(compositionEngine, std::move(args));
}

Layer::Layer(const CompositionEngine& compositionEngine, LayerCreationArgs&& args)
      : mCompositionEngine(compositionEngine), mLayerFE(args.layerFE) {
    static_cast<void>(mCompositionEngine); // Temporary use to prevent an unused warning
}

Layer::~Layer() = default;

sp<LayerFE> Layer::getLayerFE() const {
    return mLayerFE.promote();
}

const LayerCompositionState& Layer::getState() const {
    return mState;
}

LayerCompositionState& Layer::editState() {
    return mState;
}

void Layer::dump(std::string& out) const {
    auto layerFE = getLayerFE();
    android::base::StringAppendF(&out, "* compositionengine::Layer %p (%s)\n", this,
                                 layerFE ? layerFE->getDebugName() : "<unknown>");
    mState.dump(out);
}

} // namespace impl
} // namespace android::compositionengine
