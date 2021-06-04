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

#include <gmock/gmock.h>
#include <renderengine/DisplaySettings.h>
#include <renderengine/LayerSettings.h>
#include <renderengine/Mesh.h>
#include <renderengine/RenderEngine.h>
#include <renderengine/Texture.h>
#include <ui/GraphicBuffer.h>
#include <ui/Region.h>

namespace android {
namespace renderengine {
namespace mock {

class RenderEngine : public renderengine::RenderEngine {
public:
    RenderEngine();
    ~RenderEngine() override;

    MOCK_METHOD0(createFramebuffer, std::unique_ptr<renderengine::Framebuffer>());
    MOCK_METHOD0(createImage, std::unique_ptr<renderengine::Image>());
    MOCK_METHOD0(getFramebufferForDrawing, Framebuffer*());
    MOCK_CONST_METHOD0(primeCache, void());
    MOCK_METHOD1(dump, void(std::string&));
    MOCK_CONST_METHOD0(useNativeFenceSync, bool());
    MOCK_CONST_METHOD0(useWaitSync, bool());
    MOCK_CONST_METHOD0(isCurrent, bool());
    MOCK_METHOD0(flush, base::unique_fd());
    MOCK_METHOD0(finish, bool());
    MOCK_METHOD1(waitFence, bool(base::unique_fd*));
    bool waitFence(base::unique_fd fd) override { return waitFence(&fd); };
    MOCK_METHOD4(clearWithColor, void(float, float, float, float));
    MOCK_METHOD5(fillRegionWithColor, void(const Region&, float, float, float, float));
    MOCK_METHOD2(genTextures, void(size_t, uint32_t*));
    MOCK_METHOD2(deleteTextures, void(size_t, uint32_t const*));
    MOCK_METHOD2(bindExternalTextureImage, void(uint32_t, const renderengine::Image&));
    MOCK_METHOD1(cacheExternalTextureBuffer, void(const sp<GraphicBuffer>&));
    MOCK_METHOD3(bindExternalTextureBuffer,
                 status_t(uint32_t, const sp<GraphicBuffer>&, const sp<Fence>&));
    MOCK_METHOD1(unbindExternalTextureBuffer, void(uint64_t));
    MOCK_CONST_METHOD0(checkErrors, void());
    MOCK_METHOD4(setViewportAndProjection,
                 void(size_t, size_t, Rect, ui::Transform::orientation_flags));
    MOCK_METHOD5(setupLayerBlending, void(bool, bool, bool, const half4&, float));
    MOCK_METHOD1(setupLayerTexturing, void(const Texture&));
    MOCK_METHOD0(setupLayerBlackedOut, void());
    MOCK_METHOD4(setupFillWithColor, void(float, float, float, float));
    MOCK_METHOD2(setupCornerRadiusCropSize, void(float, float));
    MOCK_METHOD1(setColorTransform, void(const mat4&));
    MOCK_METHOD1(setSaturationMatrix, void(const mat4&));
    MOCK_METHOD0(disableTexturing, void());
    MOCK_METHOD0(disableBlending, void());
    MOCK_METHOD1(setSourceY410BT2020, void(bool));
    MOCK_METHOD1(setSourceDataSpace, void(ui::Dataspace));
    MOCK_METHOD1(setOutputDataSpace, void(ui::Dataspace));
    MOCK_METHOD1(setDisplayMaxLuminance, void(const float));
    MOCK_METHOD1(bindFrameBuffer, status_t(renderengine::Framebuffer*));
    MOCK_METHOD1(unbindFrameBuffer, void(renderengine::Framebuffer*));
    MOCK_METHOD1(drawMesh, void(const renderengine::Mesh&));
    MOCK_CONST_METHOD0(getMaxTextureSize, size_t());
    MOCK_CONST_METHOD0(getMaxViewportDims, size_t());
    MOCK_CONST_METHOD0(isProtected, bool());
    MOCK_CONST_METHOD0(supportsProtectedContent, bool());
    MOCK_METHOD1(useProtectedContext, bool(bool));
    MOCK_METHOD6(drawLayers,
                 status_t(const DisplaySettings&, const std::vector<LayerSettings>&,
                          ANativeWindowBuffer*, const bool, base::unique_fd&&, base::unique_fd*));
};

} // namespace mock
} // namespace renderengine
} // namespace android
