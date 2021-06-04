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

#include <compositionengine/LayerFECompositionState.h>
#include <renderengine/Mesh.h>

namespace android {

namespace compositionengine::impl {

struct LayerCompositionState {
    /*
     * State intended to be set by LayerFE::getCompositionState
     */

    LayerFECompositionState frontEnd;

    // Debugging
    void dump(std::string& result) const;
};

} // namespace compositionengine::impl
} // namespace android
