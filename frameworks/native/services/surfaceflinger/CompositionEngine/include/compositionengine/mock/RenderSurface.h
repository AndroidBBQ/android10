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

#include <compositionengine/RenderSurface.h>
#include <gmock/gmock.h>
#include <ui/GraphicBuffer.h>


namespace android::compositionengine::mock {

class RenderSurface : public compositionengine::RenderSurface {
public:
    RenderSurface();
    ~RenderSurface() override;

    MOCK_CONST_METHOD0(isValid, bool());
    MOCK_METHOD0(initialize, void());
    MOCK_CONST_METHOD0(getSize, const ui::Size&());
    MOCK_CONST_METHOD0(isProtected, bool());
    MOCK_CONST_METHOD0(getClientTargetAcquireFence, const sp<Fence>&());
    MOCK_METHOD1(setDisplaySize, void(const ui::Size&));
    MOCK_METHOD1(setProtected, void(bool));
    MOCK_METHOD1(setBufferDataspace, void(ui::Dataspace));
    MOCK_METHOD1(beginFrame, status_t(bool mustRecompose));
    MOCK_METHOD0(prepareFrame, status_t());
    MOCK_METHOD1(dequeueBuffer, sp<GraphicBuffer>(base::unique_fd*));
    MOCK_METHOD1(queueBuffer, void(base::unique_fd&&));
    MOCK_METHOD0(onPresentDisplayCompleted, void());
    MOCK_METHOD0(setViewportAndProjection, void());
    MOCK_METHOD0(flip, void());
    MOCK_CONST_METHOD1(dump, void(std::string& result));
    MOCK_CONST_METHOD0(getPageFlipCount, std::uint32_t());
};

} // namespace android::compositionengine::mock
