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

#include <compositionengine/DisplayColorProfile.h>
#include <compositionengine/Layer.h>
#include <compositionengine/LayerFE.h>
#include <compositionengine/Output.h>
#include <compositionengine/OutputLayer.h>
#include <compositionengine/RenderSurface.h>
#include <compositionengine/impl/OutputCompositionState.h>
#include <gmock/gmock.h>

namespace android::compositionengine::mock {

class Output : public virtual compositionengine::Output {
public:
    Output();
    virtual ~Output();

    MOCK_CONST_METHOD0(isValid, bool());

    MOCK_METHOD1(setCompositionEnabled, void(bool));
    MOCK_METHOD6(setProjection,
                 void(const ui::Transform&, int32_t, const Rect&, const Rect&, const Rect&, bool));
    MOCK_METHOD1(setBounds, void(const ui::Size&));
    MOCK_METHOD2(setLayerStackFilter, void(uint32_t, bool));

    MOCK_METHOD1(setColorTransform, void(const mat4&));
    MOCK_METHOD3(setColorMode, void(ui::ColorMode, ui::Dataspace, ui::RenderIntent));

    MOCK_CONST_METHOD1(dump, void(std::string&));
    MOCK_CONST_METHOD0(getName, const std::string&());
    MOCK_METHOD1(setName, void(const std::string&));

    MOCK_CONST_METHOD0(getDisplayColorProfile, DisplayColorProfile*());
    MOCK_METHOD1(setDisplayColorProfile, void(std::unique_ptr<DisplayColorProfile>));

    MOCK_CONST_METHOD0(getRenderSurface, RenderSurface*());
    MOCK_METHOD1(setRenderSurface, void(std::unique_ptr<RenderSurface>));

    MOCK_CONST_METHOD0(getState, const OutputCompositionState&());
    MOCK_METHOD0(editState, OutputCompositionState&());

    MOCK_CONST_METHOD1(getDirtyRegion, Region(bool));
    MOCK_CONST_METHOD2(belongsInOutput, bool(uint32_t, bool));

    MOCK_CONST_METHOD1(getOutputLayerForLayer,
                       compositionengine::OutputLayer*(compositionengine::Layer*));
    MOCK_METHOD3(getOrCreateOutputLayer,
                 std::unique_ptr<compositionengine::OutputLayer>(
                         std::optional<DisplayId>, std::shared_ptr<compositionengine::Layer>,
                         sp<compositionengine::LayerFE>));
    MOCK_METHOD1(setOutputLayersOrderedByZ, void(OutputLayers&&));
    MOCK_CONST_METHOD0(getOutputLayersOrderedByZ, OutputLayers&());
};

} // namespace android::compositionengine::mock
