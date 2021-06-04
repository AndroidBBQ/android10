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

#include <compositionengine/Layer.h>
#include <compositionengine/LayerFE.h>
#include <compositionengine/impl/LayerCompositionState.h>
#include <gmock/gmock.h>

namespace android::compositionengine::mock {

class Layer : public compositionengine::Layer {
public:
    Layer();
    virtual ~Layer();

    MOCK_CONST_METHOD0(getLayerFE, sp<LayerFE>());

    MOCK_CONST_METHOD0(getState, const CompositionState&());
    MOCK_METHOD0(editState, CompositionState&());

    MOCK_CONST_METHOD1(dump, void(std::string&));
};

} // namespace android::compositionengine::mock
