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

#include <memory>
#include <string>

#include <compositionengine/OutputLayer.h>
#include <compositionengine/impl/OutputLayerCompositionState.h>
#include <ui/FloatRect.h>
#include <ui/Rect.h>

#include "DisplayHardware/DisplayIdentification.h"

namespace android::compositionengine::impl {

class OutputLayer : public compositionengine::OutputLayer {
public:
    OutputLayer(const compositionengine::Output&, std::shared_ptr<compositionengine::Layer>,
                sp<compositionengine::LayerFE>);
    ~OutputLayer() override;

    void initialize(const CompositionEngine&, std::optional<DisplayId>);

    const compositionengine::Output& getOutput() const override;
    compositionengine::Layer& getLayer() const override;
    compositionengine::LayerFE& getLayerFE() const override;

    const OutputLayerCompositionState& getState() const override;
    OutputLayerCompositionState& editState() override;

    void updateCompositionState(bool) override;
    void writeStateToHWC(bool) const override;

    void dump(std::string& result) const override;

    virtual FloatRect calculateOutputSourceCrop() const;
    virtual Rect calculateOutputDisplayFrame() const;
    virtual uint32_t calculateOutputRelativeBufferTransform() const;

private:
    Rect calculateInitialCrop() const;

    const compositionengine::Output& mOutput;
    std::shared_ptr<compositionengine::Layer> mLayer;
    sp<compositionengine::LayerFE> mLayerFE;

    OutputLayerCompositionState mState;
};

std::unique_ptr<compositionengine::OutputLayer> createOutputLayer(
        const CompositionEngine&, std::optional<DisplayId>, const compositionengine::Output&,
        std::shared_ptr<compositionengine::Layer>, sp<compositionengine::LayerFE>);

} // namespace android::compositionengine::impl
