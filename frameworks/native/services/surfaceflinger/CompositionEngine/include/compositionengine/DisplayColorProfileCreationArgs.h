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

#include <ui/GraphicTypes.h>
#include <ui/HdrCapabilities.h>

namespace android::compositionengine {

/**
 * A parameter object for creating DisplayColorProfile instances
 */
struct DisplayColorProfileCreationArgs {
    using HwcColorModes = std::unordered_map<ui::ColorMode, std::vector<ui::RenderIntent>>;

    // True if this display supports a wide color gamut
    bool hasWideColorGamut;

    // The HDR capabilities supported by the HWC
    HdrCapabilities hdrCapabilities;

    // The per-frame metadata supported by the HWC
    int32_t supportedPerFrameMetadata;

    // The mapping of color modes and render intents supported by the HWC
    HwcColorModes hwcColorModes;
};

/**
 * A helper for setting up a DisplayColorProfileCreationArgs value in-line.
 *
 * Prefer this builder over raw structure initialization.
 *
 * Instead of:
 *
 *   DisplayColorProfileCreationArgs{false, HdrCapabilities(), 0,
 *                                   HwcColorModes()}
 *
 * Prefer:
 *
 *  DisplayColorProfileCreationArgsBuilder().setHasWideColorGamut(false)
 *      .setIsVirtual(false).setDisplayId(displayId).Build();
 */
class DisplayColorProfileCreationArgsBuilder {
public:
    DisplayColorProfileCreationArgs Build() { return std::move(mArgs); }

    DisplayColorProfileCreationArgsBuilder& setHasWideColorGamut(bool hasWideColorGamut) {
        mArgs.hasWideColorGamut = hasWideColorGamut;
        return *this;
    }
    DisplayColorProfileCreationArgsBuilder& setHdrCapabilities(HdrCapabilities&& hdrCapabilities) {
        mArgs.hdrCapabilities = std::move(hdrCapabilities);
        return *this;
    }
    DisplayColorProfileCreationArgsBuilder& setSupportedPerFrameMetadata(
            int32_t supportedPerFrameMetadata) {
        mArgs.supportedPerFrameMetadata = supportedPerFrameMetadata;
        return *this;
    }
    DisplayColorProfileCreationArgsBuilder& setHwcColorModes(
            DisplayColorProfileCreationArgs::HwcColorModes&& hwcColorModes) {
        mArgs.hwcColorModes = std::move(hwcColorModes);
        return *this;
    }

private:
    DisplayColorProfileCreationArgs mArgs;
};

} // namespace android::compositionengine
