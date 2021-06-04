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
#include <unordered_map>
#include <vector>

#include <compositionengine/DisplayColorProfile.h>
#include <compositionengine/DisplayColorProfileCreationArgs.h>
#include <ui/HdrCapabilities.h>

namespace android::compositionengine {

class CompositionEngine;
class Display;

namespace impl {

class DisplayColorProfile : public compositionengine::DisplayColorProfile {
public:
    DisplayColorProfile(DisplayColorProfileCreationArgs&&);
    ~DisplayColorProfile() override;

    bool isValid() const override;

    bool hasRenderIntent(ui::RenderIntent intent) const override;
    bool hasLegacyHdrSupport(ui::Dataspace dataspace) const override;
    void getBestColorMode(ui::Dataspace dataspace, ui::RenderIntent intent,
                          ui::Dataspace* outDataspace, ui::ColorMode* outMode,
                          ui::RenderIntent* outIntent) const override;

    bool hasWideColorGamut() const override;
    int32_t getSupportedPerFrameMetadata() const override;

    // Whether h/w composer has native support for specific HDR type.
    bool hasHDR10PlusSupport() const override;
    bool hasHDR10Support() const override;
    bool hasHLGSupport() const override;
    bool hasDolbyVisionSupport() const override;

    const HdrCapabilities& getHdrCapabilities() const override;

    void dump(std::string&) const override;

private:
    void populateColorModes(const DisplayColorProfileCreationArgs::HwcColorModes& hwcColorModes);
    void addColorMode(const DisplayColorProfileCreationArgs::HwcColorModes& hwcColorModes,
                      const ui::ColorMode mode, const ui::RenderIntent intent);

    // Mappings from desired Dataspace/RenderIntent to the supported
    // Dataspace/ColorMode/RenderIntent.
    using ColorModeKey = uint64_t;
    struct ColorModeValue {
        ui::Dataspace dataspace;
        ui::ColorMode colorMode;
        ui::RenderIntent renderIntent;
    };

    static ColorModeKey getColorModeKey(ui::Dataspace dataspace, ui::RenderIntent intent) {
        return (static_cast<uint64_t>(dataspace) << 32) | static_cast<uint32_t>(intent);
    }

    // Need to know if display is wide-color capable or not.
    // Initialized by SurfaceFlinger when the DisplayDevice is created.
    // Fed to RenderEngine during composition.
    bool mHasWideColorGamut{false};
    int32_t mSupportedPerFrameMetadata{0};
    bool mHasHdr10Plus{false};
    bool mHasHdr10{false};
    bool mHasHLG{false};
    bool mHasDolbyVision{false};
    HdrCapabilities mHdrCapabilities;
    std::unordered_map<ColorModeKey, ColorModeValue> mColorModes;
};

std::unique_ptr<compositionengine::DisplayColorProfile> createDisplayColorProfile(
        DisplayColorProfileCreationArgs&&);

} // namespace impl
} // namespace android::compositionengine
