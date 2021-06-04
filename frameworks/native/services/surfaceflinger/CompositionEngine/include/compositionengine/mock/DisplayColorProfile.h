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
#include <gmock/gmock.h>
#include <ui/HdrCapabilities.h>

namespace android::compositionengine::mock {

class DisplayColorProfile : public compositionengine::DisplayColorProfile {
public:
    DisplayColorProfile();
    ~DisplayColorProfile() override;

    MOCK_CONST_METHOD0(isValid, bool());

    MOCK_CONST_METHOD1(hasRenderIntent, bool(ui::RenderIntent));
    MOCK_CONST_METHOD1(hasLegacyHdrSupport, bool(ui::Dataspace));
    MOCK_CONST_METHOD5(getBestColorMode,
                       void(ui::Dataspace, ui::RenderIntent, ui::Dataspace*, ui::ColorMode*,
                            ui::RenderIntent*));
    MOCK_CONST_METHOD0(hasWideColorGamut, bool());
    MOCK_CONST_METHOD0(getSupportedPerFrameMetadata, int32_t());
    MOCK_CONST_METHOD0(hasHDR10PlusSupport, bool());
    MOCK_CONST_METHOD0(hasHDR10Support, bool());
    MOCK_CONST_METHOD0(hasHLGSupport, bool());
    MOCK_CONST_METHOD0(hasDolbyVisionSupport, bool());

    MOCK_CONST_METHOD0(getHdrCapabilities, const HdrCapabilities&());

    MOCK_CONST_METHOD1(dump, void(std::string&));
};

} // namespace android::compositionengine::mock
