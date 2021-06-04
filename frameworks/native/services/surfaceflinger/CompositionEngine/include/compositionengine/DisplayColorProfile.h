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

#include <ui/GraphicTypes.h>

namespace android {

class HdrCapabilities;

namespace compositionengine {

/**
 * Encapsulates all the state and functionality for how colors should be
 * transformed for a display
 */
class DisplayColorProfile {
public:
    constexpr static float sDefaultMinLumiance = 0.0;
    constexpr static float sDefaultMaxLumiance = 500.0;

    virtual ~DisplayColorProfile();

    // Returns true if the profile is valid. This is meant to be checked post-
    // construction and prior to use, as not everything is set up by the
    // constructor.
    virtual bool isValid() const = 0;

    // Returns true if the profile supports the indicated render intent
    virtual bool hasRenderIntent(ui::RenderIntent) const = 0;

    // Returns true if the profile supports the indicated dataspace
    virtual bool hasLegacyHdrSupport(ui::Dataspace) const = 0;

    // Obtains the best combination of color mode and render intent for the
    // input values
    virtual void getBestColorMode(ui::Dataspace dataspace, ui::RenderIntent intent,
                                  ui::Dataspace* outDataspace, ui::ColorMode* outMode,
                                  ui::RenderIntent* outIntent) const = 0;

    // Returns true if the profile supports a wide color gamut
    virtual bool hasWideColorGamut() const = 0;

    // Returns the per-frame metadata value for this profile
    virtual int32_t getSupportedPerFrameMetadata() const = 0;

    // Returns true if HWC for this profile supports HDR10Plus
    virtual bool hasHDR10PlusSupport() const = 0;

    // Returns true if HWC for this profile supports HDR10
    virtual bool hasHDR10Support() const = 0;

    // Returns true if HWC for this profile supports HLG
    virtual bool hasHLGSupport() const = 0;

    // Returns true if HWC for this profile supports DolbyVision
    virtual bool hasDolbyVisionSupport() const = 0;

    // Gets the supported HDR capabilities for the profile
    virtual const HdrCapabilities& getHdrCapabilities() const = 0;

    // Debugging
    virtual void dump(std::string&) const = 0;
};

} // namespace compositionengine
} // namespace android
