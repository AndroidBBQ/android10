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

#include <compositionengine/LayerFE.h>
#include <compositionengine/LayerFECompositionState.h>
#include <gmock/gmock.h>
#include <ui/Fence.h>

namespace android::compositionengine::mock {

// Defines the interface used by the CompositionEngine to make requests
// of the front-end layer.
class LayerFE : public compositionengine::LayerFE {
public:
    LayerFE();
    virtual ~LayerFE();

    MOCK_CONST_METHOD2(latchCompositionState, void(LayerFECompositionState&, bool));
    MOCK_METHOD1(onLayerDisplayed, void(const sp<Fence>&));

    MOCK_CONST_METHOD0(getDebugName, const char*());
};

} // namespace android::compositionengine::mock
