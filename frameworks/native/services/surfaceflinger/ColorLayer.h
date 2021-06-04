/*
 * Copyright (C) 2007 The Android Open Source Project
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

#include <sys/types.h>

#include <cstdint>

#include "Layer.h"

namespace android {

class ColorLayer : public Layer {
public:
    explicit ColorLayer(const LayerCreationArgs&);
    ~ColorLayer() override;

    std::shared_ptr<compositionengine::Layer> getCompositionLayer() const override;

    virtual const char* getTypeId() const { return "ColorLayer"; }
    bool isVisible() const override;

    bool setColor(const half3& color) override;

    bool setDataspace(ui::Dataspace dataspace) override;

    void setPerFrameData(const sp<const DisplayDevice>& display, const ui::Transform& transform,
                         const Rect& viewport, int32_t supportedPerFrameMetadata,
                         const ui::Dataspace targetDataspace) override;

    void commitTransaction(const State& stateToCommit) override;

    bool onPreComposition(nsecs_t /*refreshStartTime*/) override { return false; }

protected:
    virtual bool prepareClientLayer(const RenderArea& renderArea, const Region& clip,
                                    bool useIdentityTransform, Region& clearRegion,
                                    const bool supportProtectedContent,
                                    renderengine::LayerSettings& layer);

private:
    std::shared_ptr<compositionengine::Layer> mCompositionLayer;
};

} // namespace android
