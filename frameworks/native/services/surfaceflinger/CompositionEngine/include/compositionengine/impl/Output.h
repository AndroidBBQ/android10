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
#include <utility>
#include <vector>

#include <compositionengine/Output.h>
#include <compositionengine/impl/OutputCompositionState.h>

namespace android::compositionengine {

class CompositionEngine;
class Layer;
class OutputLayer;

namespace impl {

class Output : public virtual compositionengine::Output {
public:
    Output(const CompositionEngine&);
    ~Output() override;

    bool isValid() const override;

    void setCompositionEnabled(bool) override;
    void setProjection(const ui::Transform&, int32_t orientation, const Rect& frame,
                       const Rect& viewport, const Rect& scissor, bool needsFiltering) override;
    void setBounds(const ui::Size&) override;
    void setLayerStackFilter(uint32_t layerStackId, bool isInternal) override;

    void setColorTransform(const mat4&) override;
    void setColorMode(ui::ColorMode, ui::Dataspace, ui::RenderIntent) override;

    void dump(std::string&) const override;

    const std::string& getName() const override;
    void setName(const std::string&) override;

    compositionengine::DisplayColorProfile* getDisplayColorProfile() const override;
    void setDisplayColorProfile(std::unique_ptr<compositionengine::DisplayColorProfile>) override;

    compositionengine::RenderSurface* getRenderSurface() const override;
    void setRenderSurface(std::unique_ptr<compositionengine::RenderSurface>) override;

    const OutputCompositionState& getState() const override;
    OutputCompositionState& editState() override;

    Region getDirtyRegion(bool repaintEverything) const override;
    bool belongsInOutput(uint32_t, bool) const override;

    compositionengine::OutputLayer* getOutputLayerForLayer(
            compositionengine::Layer*) const override;
    std::unique_ptr<compositionengine::OutputLayer> getOrCreateOutputLayer(
            std::optional<DisplayId>, std::shared_ptr<compositionengine::Layer>,
            sp<LayerFE>) override;
    void setOutputLayersOrderedByZ(OutputLayers&&) override;
    const OutputLayers& getOutputLayersOrderedByZ() const override;

    // Testing
    void setDisplayColorProfileForTest(std::unique_ptr<compositionengine::DisplayColorProfile>);
    void setRenderSurfaceForTest(std::unique_ptr<compositionengine::RenderSurface>);

protected:
    const CompositionEngine& getCompositionEngine() const;
    void dumpBase(std::string&) const;

private:
    void dirtyEntireOutput();

    const CompositionEngine& mCompositionEngine;

    std::string mName;

    OutputCompositionState mState;

    std::unique_ptr<compositionengine::DisplayColorProfile> mDisplayColorProfile;
    std::unique_ptr<compositionengine::RenderSurface> mRenderSurface;

    OutputLayers mOutputLayersOrderedByZ;
};

} // namespace impl
} // namespace android::compositionengine
