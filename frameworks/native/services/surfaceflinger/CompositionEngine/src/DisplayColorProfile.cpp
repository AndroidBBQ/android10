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

#include <array>
#include <unordered_set>

#include <compositionengine/CompositionEngine.h>
#include <compositionengine/Display.h>
#include <compositionengine/DisplayColorProfileCreationArgs.h>
#include <compositionengine/RenderSurface.h>
#include <compositionengine/impl/DisplayColorProfile.h>
#include <compositionengine/impl/DumpHelpers.h>
#include <log/log.h>
#include <ui/DebugUtils.h>

#include "DisplayHardware/HWComposer.h"

namespace android::compositionengine {

DisplayColorProfile::~DisplayColorProfile() = default;

namespace impl {
namespace {

using ui::ColorMode;
using ui::Dataspace;
using ui::RenderIntent;

// ordered list of known SDR color modes
const std::array<ColorMode, 3> sSdrColorModes = {
        ColorMode::DISPLAY_BT2020,
        ColorMode::DISPLAY_P3,
        ColorMode::SRGB,
};

// ordered list of known HDR color modes
const std::array<ColorMode, 2> sHdrColorModes = {
        ColorMode::BT2100_PQ,
        ColorMode::BT2100_HLG,
};

// ordered list of known SDR render intents
const std::array<RenderIntent, 2> sSdrRenderIntents = {
        RenderIntent::ENHANCE,
        RenderIntent::COLORIMETRIC,
};

// ordered list of known HDR render intents
const std::array<RenderIntent, 2> sHdrRenderIntents = {
        RenderIntent::TONE_MAP_ENHANCE,
        RenderIntent::TONE_MAP_COLORIMETRIC,
};

// map known color mode to dataspace
Dataspace colorModeToDataspace(ColorMode mode) {
    switch (mode) {
        case ColorMode::SRGB:
            return Dataspace::V0_SRGB;
        case ColorMode::DISPLAY_P3:
            return Dataspace::DISPLAY_P3;
        case ColorMode::DISPLAY_BT2020:
            return Dataspace::DISPLAY_BT2020;
        case ColorMode::BT2100_HLG:
            return Dataspace::BT2020_HLG;
        case ColorMode::BT2100_PQ:
            return Dataspace::BT2020_PQ;
        default:
            return Dataspace::UNKNOWN;
    }
}

// Return a list of candidate color modes.
std::vector<ColorMode> getColorModeCandidates(ColorMode mode) {
    std::vector<ColorMode> candidates;

    // add mode itself
    candidates.push_back(mode);

    // check if mode is HDR
    bool isHdr = false;
    for (auto hdrMode : sHdrColorModes) {
        if (hdrMode == mode) {
            isHdr = true;
            break;
        }
    }

    // add other HDR candidates when mode is HDR
    if (isHdr) {
        for (auto hdrMode : sHdrColorModes) {
            if (hdrMode != mode) {
                candidates.push_back(hdrMode);
            }
        }
    }

    // add other SDR candidates
    for (auto sdrMode : sSdrColorModes) {
        if (sdrMode != mode) {
            candidates.push_back(sdrMode);
        }
    }

    return candidates;
}

// Return a list of candidate render intents.
std::vector<RenderIntent> getRenderIntentCandidates(RenderIntent intent) {
    std::vector<RenderIntent> candidates;

    // add intent itself
    candidates.push_back(intent);

    // check if intent is HDR
    bool isHdr = false;
    for (auto hdrIntent : sHdrRenderIntents) {
        if (hdrIntent == intent) {
            isHdr = true;
            break;
        }
    }

    if (isHdr) {
        // add other HDR candidates when intent is HDR
        for (auto hdrIntent : sHdrRenderIntents) {
            if (hdrIntent != intent) {
                candidates.push_back(hdrIntent);
            }
        }
    } else {
        // add other SDR candidates when intent is SDR
        for (auto sdrIntent : sSdrRenderIntents) {
            if (sdrIntent != intent) {
                candidates.push_back(sdrIntent);
            }
        }
    }

    return candidates;
}

// Return the best color mode supported by HWC.
ColorMode getHwcColorMode(
        const std::unordered_map<ColorMode, std::vector<RenderIntent>>& hwcColorModes,
        ColorMode mode) {
    std::vector<ColorMode> candidates = getColorModeCandidates(mode);
    for (auto candidate : candidates) {
        auto iter = hwcColorModes.find(candidate);
        if (iter != hwcColorModes.end()) {
            return candidate;
        }
    }

    return ColorMode::NATIVE;
}

// Return the best render intent supported by HWC.
RenderIntent getHwcRenderIntent(const std::vector<RenderIntent>& hwcIntents, RenderIntent intent) {
    std::vector<RenderIntent> candidates = getRenderIntentCandidates(intent);
    for (auto candidate : candidates) {
        for (auto hwcIntent : hwcIntents) {
            if (candidate == hwcIntent) {
                return candidate;
            }
        }
    }

    return RenderIntent::COLORIMETRIC;
}

} // anonymous namespace

std::unique_ptr<compositionengine::DisplayColorProfile> createDisplayColorProfile(
        DisplayColorProfileCreationArgs&& args) {
    return std::make_unique<DisplayColorProfile>(std::move(args));
}

DisplayColorProfile::DisplayColorProfile(DisplayColorProfileCreationArgs&& args)
      : mHasWideColorGamut(args.hasWideColorGamut),
        mSupportedPerFrameMetadata(args.supportedPerFrameMetadata) {
    populateColorModes(args.hwcColorModes);

    std::vector<ui::Hdr> types = args.hdrCapabilities.getSupportedHdrTypes();
    for (ui::Hdr hdrType : types) {
        switch (hdrType) {
            case ui::Hdr::HDR10_PLUS:
                mHasHdr10Plus = true;
                break;
            case ui::Hdr::HDR10:
                mHasHdr10 = true;
                break;
            case ui::Hdr::HLG:
                mHasHLG = true;
                break;
            case ui::Hdr::DOLBY_VISION:
                mHasDolbyVision = true;
                break;
            default:
                ALOGE("UNKNOWN HDR capability: %d", static_cast<int32_t>(hdrType));
        }
    }

    float minLuminance = args.hdrCapabilities.getDesiredMinLuminance();
    float maxLuminance = args.hdrCapabilities.getDesiredMaxLuminance();
    float maxAverageLuminance = args.hdrCapabilities.getDesiredMaxAverageLuminance();

    minLuminance = minLuminance <= 0.0 ? sDefaultMinLumiance : minLuminance;
    maxLuminance = maxLuminance <= 0.0 ? sDefaultMaxLumiance : maxLuminance;
    maxAverageLuminance = maxAverageLuminance <= 0.0 ? sDefaultMaxLumiance : maxAverageLuminance;
    if (args.hasWideColorGamut) {
        // insert HDR10/HLG as we will force client composition for HDR10/HLG
        // layers
        if (!hasHDR10Support()) {
            types.push_back(ui::Hdr::HDR10);
        }

        if (!hasHLGSupport()) {
            types.push_back(ui::Hdr::HLG);
        }
    }

    mHdrCapabilities = HdrCapabilities(types, maxLuminance, maxAverageLuminance, minLuminance);
}

DisplayColorProfile::~DisplayColorProfile() = default;

bool DisplayColorProfile::isValid() const {
    return true;
}

bool DisplayColorProfile::hasWideColorGamut() const {
    return mHasWideColorGamut;
}

int32_t DisplayColorProfile::getSupportedPerFrameMetadata() const {
    return mSupportedPerFrameMetadata;
}

bool DisplayColorProfile::hasHDR10PlusSupport() const {
    return mHasHdr10Plus;
}

bool DisplayColorProfile::hasHDR10Support() const {
    return mHasHdr10;
}

bool DisplayColorProfile::hasHLGSupport() const {
    return mHasHLG;
}

bool DisplayColorProfile::hasDolbyVisionSupport() const {
    return mHasDolbyVision;
}

const HdrCapabilities& DisplayColorProfile::getHdrCapabilities() const {
    return mHdrCapabilities;
}

void DisplayColorProfile::populateColorModes(
        const DisplayColorProfileCreationArgs::HwcColorModes& hwcColorModes) {
    if (!hasWideColorGamut()) {
        return;
    }

    // collect all known SDR render intents
    std::unordered_set<RenderIntent> sdrRenderIntents(sSdrRenderIntents.begin(),
                                                      sSdrRenderIntents.end());
    auto iter = hwcColorModes.find(ColorMode::SRGB);
    if (iter != hwcColorModes.end()) {
        for (auto intent : iter->second) {
            sdrRenderIntents.insert(intent);
        }
    }

    // add all known SDR combinations
    for (auto intent : sdrRenderIntents) {
        for (auto mode : sSdrColorModes) {
            addColorMode(hwcColorModes, mode, intent);
        }
    }

    // collect all known HDR render intents
    std::unordered_set<RenderIntent> hdrRenderIntents(sHdrRenderIntents.begin(),
                                                      sHdrRenderIntents.end());
    iter = hwcColorModes.find(ColorMode::BT2100_PQ);
    if (iter != hwcColorModes.end()) {
        for (auto intent : iter->second) {
            hdrRenderIntents.insert(intent);
        }
    }

    // add all known HDR combinations
    for (auto intent : hdrRenderIntents) {
        for (auto mode : sHdrColorModes) {
            addColorMode(hwcColorModes, mode, intent);
        }
    }
}

// Map dataspace/intent to the best matched dataspace/colorMode/renderIntent
// supported by HWC.
void DisplayColorProfile::addColorMode(
        const DisplayColorProfileCreationArgs::HwcColorModes& hwcColorModes, const ColorMode mode,
        const RenderIntent intent) {
    // find the best color mode
    const ColorMode hwcColorMode = getHwcColorMode(hwcColorModes, mode);

    // find the best render intent
    auto iter = hwcColorModes.find(hwcColorMode);
    const auto& hwcIntents =
            iter != hwcColorModes.end() ? iter->second : std::vector<RenderIntent>();
    const RenderIntent hwcIntent = getHwcRenderIntent(hwcIntents, intent);

    const Dataspace dataspace = colorModeToDataspace(mode);
    const Dataspace hwcDataspace = colorModeToDataspace(hwcColorMode);

    ALOGV("DisplayColorProfile: map (%s, %s) to (%s, %s, %s)",
          dataspaceDetails(static_cast<android_dataspace_t>(dataspace)).c_str(),
          decodeRenderIntent(intent).c_str(),
          dataspaceDetails(static_cast<android_dataspace_t>(hwcDataspace)).c_str(),
          decodeColorMode(hwcColorMode).c_str(), decodeRenderIntent(hwcIntent).c_str());

    mColorModes[getColorModeKey(dataspace, intent)] = {hwcDataspace, hwcColorMode, hwcIntent};
}

bool DisplayColorProfile::hasRenderIntent(RenderIntent intent) const {
    // assume a render intent is supported when SRGB supports it; we should
    // get rid of that assumption.
    auto iter = mColorModes.find(getColorModeKey(Dataspace::V0_SRGB, intent));
    return iter != mColorModes.end() && iter->second.renderIntent == intent;
}

bool DisplayColorProfile::hasLegacyHdrSupport(Dataspace dataspace) const {
    if ((dataspace == Dataspace::BT2020_PQ && hasHDR10Support()) ||
        (dataspace == Dataspace::BT2020_HLG && hasHLGSupport())) {
        auto iter =
                mColorModes.find(getColorModeKey(dataspace, RenderIntent::TONE_MAP_COLORIMETRIC));
        return iter == mColorModes.end() || iter->second.dataspace != dataspace;
    }

    return false;
}

void DisplayColorProfile::getBestColorMode(Dataspace dataspace, RenderIntent intent,
                                           Dataspace* outDataspace, ColorMode* outMode,
                                           RenderIntent* outIntent) const {
    auto iter = mColorModes.find(getColorModeKey(dataspace, intent));
    if (iter != mColorModes.end()) {
        *outDataspace = iter->second.dataspace;
        *outMode = iter->second.colorMode;
        *outIntent = iter->second.renderIntent;
    } else {
        // this is unexpected on a WCG display
        if (hasWideColorGamut()) {
            ALOGE("map unknown (%s)/(%s) to default color mode",
                  dataspaceDetails(static_cast<android_dataspace_t>(dataspace)).c_str(),
                  decodeRenderIntent(intent).c_str());
        }

        *outDataspace = Dataspace::UNKNOWN;
        *outMode = ColorMode::NATIVE;
        *outIntent = RenderIntent::COLORIMETRIC;
    }
}

void DisplayColorProfile::dump(std::string& out) const {
    out.append("   Composition Display Color State:");

    out.append("\n   HWC Support: ");

    dumpVal(out, "wideColorGamut", hasWideColorGamut());
    dumpVal(out, "hdr10plus", hasHDR10PlusSupport());
    dumpVal(out, "hdr10", hasHDR10Support());
    dumpVal(out, "hlg", hasHLGSupport());
    dumpVal(out, "dv", hasDolbyVisionSupport());
    dumpVal(out, "metadata", getSupportedPerFrameMetadata());

    out.append("\n");
}

} // namespace impl
} // namespace android::compositionengine
