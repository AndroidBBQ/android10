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

#include <compositionengine/DisplaySurface.h>
#include <gmock/gmock.h>
#include <utils/String8.h>

namespace android::compositionengine::mock {

class DisplaySurface : public compositionengine::DisplaySurface {
public:
    DisplaySurface();
    ~DisplaySurface() override;

    MOCK_METHOD1(beginFrame, status_t(bool mustRecompose));
    MOCK_METHOD1(prepareFrame, status_t(CompositionType compositionType));
    MOCK_METHOD0(advanceFrame, status_t());
    MOCK_METHOD0(onFrameCommitted, void());
    MOCK_CONST_METHOD1(dumpAsString, void(String8& result));
    MOCK_METHOD2(resizeBuffers, void(uint32_t, uint32_t));
    MOCK_CONST_METHOD0(getClientTargetAcquireFence, const sp<Fence>&());
};

} // namespace android::compositionengine::mock
