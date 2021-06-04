/*
 * Copyright (C) 2019 The Android Open Source Project
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

#define LOG_TAG "HeicEncoderInfoManager"
//#define LOG_NDEBUG 0

#include <cstdint>
#include <regex>

#include <cutils/properties.h>
#include <log/log_main.h>
#include <system/graphics.h>

#include <media/stagefright/MediaCodecList.h>
#include <media/stagefright/foundation/MediaDefs.h>
#include <media/stagefright/foundation/ABuffer.h>

#include "HeicEncoderInfoManager.h"

namespace android {
namespace camera3 {

HeicEncoderInfoManager::HeicEncoderInfoManager() :
        mIsInited(false),
        mMinSizeHeic(0, 0),
        mMaxSizeHeic(INT32_MAX, INT32_MAX),
        mHasHEVC(false),
        mHasHEIC(false),
        mDisableGrid(false) {
    if (initialize() == OK) {
        mIsInited = true;
    }
}

HeicEncoderInfoManager::~HeicEncoderInfoManager() {
}

bool HeicEncoderInfoManager::isSizeSupported(int32_t width, int32_t height, bool* useHeic,
        bool* useGrid, int64_t* stall, AString* hevcName) const {
    if (useHeic == nullptr || useGrid == nullptr) {
        ALOGE("%s: invalid parameters: useHeic %p, useGrid %p",
                __FUNCTION__, useHeic, useGrid);
        return false;
    }
    if (!mIsInited) return false;

    bool chooseHeic = false, enableGrid = true;
    if (mHasHEIC && width >= mMinSizeHeic.first &&
            height >= mMinSizeHeic.second && width <= mMaxSizeHeic.first &&
            height <= mMaxSizeHeic.second) {
        chooseHeic = true;
        enableGrid = false;
    } else if (mHasHEVC) {
        bool fullSizeSupportedByHevc = (width >= mMinSizeHevc.first &&
                height >= mMinSizeHevc.second &&
                width <= mMaxSizeHevc.first &&
                height <= mMaxSizeHevc.second);
        if (fullSizeSupportedByHevc && (mDisableGrid ||
                (width <= 1920 && height <= 1080))) {
            enableGrid = false;
        }
        if (hevcName != nullptr) {
            *hevcName = mHevcName;
        }
    } else {
        // No encoder available for the requested size.
        return false;
    }

    if (stall != nullptr) {
        // Find preferred encoder which advertise
        // "measured-frame-rate-WIDTHxHEIGHT-range" key.
        const FrameRateMaps& maps =
                (chooseHeic && mHeicFrameRateMaps.size() > 0) ?
                mHeicFrameRateMaps : mHevcFrameRateMaps;
        const auto& closestSize = findClosestSize(maps, width, height);
        if (closestSize == maps.end()) {
            // The "measured-frame-rate-WIDTHxHEIGHT-range" key is optional.
            // Hardcode to some default value (3.33ms * tile count) based on resolution.
            *stall = 3333333LL * width * height / (kGridWidth * kGridHeight);
            return true;
        }

        // Derive stall durations based on average fps of the closest size.
        constexpr int64_t NSEC_PER_SEC = 1000000000LL;
        int32_t avgFps = (closestSize->second.first + closestSize->second.second)/2;
        float ratio = 1.0f * width * height /
                (closestSize->first.first * closestSize->first.second);
        *stall = ratio * NSEC_PER_SEC / avgFps;
    }

    *useHeic = chooseHeic;
    *useGrid = enableGrid;
    return true;
}

status_t HeicEncoderInfoManager::initialize() {
    mDisableGrid = property_get_bool("camera.heic.disable_grid", false);
    sp<IMediaCodecList> codecsList = MediaCodecList::getInstance();
    if (codecsList == nullptr) {
        // No media codec available.
        return OK;
    }

    sp<AMessage> heicDetails = getCodecDetails(codecsList, MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC);

    if (!getHevcCodecDetails(codecsList, MEDIA_MIMETYPE_VIDEO_HEVC)) {
        if (heicDetails != nullptr) {
            ALOGE("%s: Device must support HEVC codec if HEIC codec is available!",
                    __FUNCTION__);
            return BAD_VALUE;
        }
        return OK;
    }
    mHasHEVC = true;

    // HEIC size range
    if (heicDetails != nullptr) {
        auto res = getCodecSizeRange(MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC,
                heicDetails, &mMinSizeHeic, &mMaxSizeHeic, &mHeicFrameRateMaps);
        if (res != OK) {
            ALOGE("%s: Failed to get HEIC codec size range: %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            return BAD_VALUE;
        }
        mHasHEIC = true;
    }

    return OK;
}

status_t HeicEncoderInfoManager::getFrameRateMaps(sp<AMessage> details, FrameRateMaps* maps) {
    if (details == nullptr || maps == nullptr) {
        ALOGE("%s: Invalid input: details: %p, maps: %p", __FUNCTION__, details.get(), maps);
        return BAD_VALUE;
    }

    for (size_t i = 0; i < details->countEntries(); i++) {
        AMessage::Type type;
        const char* entryName = details->getEntryNameAt(i, &type);
        if (type != AMessage::kTypeString) continue;
        std::regex frameRateNamePattern("measured-frame-rate-([0-9]+)[*x]([0-9]+)-range",
                std::regex_constants::icase);
        std::cmatch sizeMatch;
        if (std::regex_match(entryName, sizeMatch, frameRateNamePattern) &&
                sizeMatch.size() == 3) {
            AMessage::ItemData item = details->getEntryAt(i);
            AString fpsRangeStr;
            if (item.find(&fpsRangeStr)) {
                ALOGV("%s: %s", entryName, fpsRangeStr.c_str());
                std::regex frameRatePattern("([0-9]+)-([0-9]+)");
                std::cmatch fpsMatch;
                if (std::regex_match(fpsRangeStr.c_str(), fpsMatch, frameRatePattern) &&
                        fpsMatch.size() == 3) {
                    maps->emplace(
                            std::make_pair(stoi(sizeMatch[1]), stoi(sizeMatch[2])),
                            std::make_pair(stoi(fpsMatch[1]), stoi(fpsMatch[2])));
                } else {
                    return BAD_VALUE;
                }
            }
        }
    }
    return OK;
}

status_t HeicEncoderInfoManager::getCodecSizeRange(
        const char* codecName,
        sp<AMessage> details,
        std::pair<int32_t, int32_t>* minSize,
        std::pair<int32_t, int32_t>* maxSize,
        FrameRateMaps* frameRateMaps) {
    if (codecName == nullptr || minSize == nullptr || maxSize == nullptr ||
            details == nullptr || frameRateMaps == nullptr) {
        return BAD_VALUE;
    }

    AString sizeRange;
    auto hasItem = details->findString("size-range", &sizeRange);
    if (!hasItem) {
        ALOGE("%s: Failed to query size range for codec %s", __FUNCTION__, codecName);
        return BAD_VALUE;
    }
    ALOGV("%s: %s codec's size range is %s", __FUNCTION__, codecName, sizeRange.c_str());
    std::regex pattern("([0-9]+)[*x]([0-9]+)-([0-9]+)[*x]([0-9]+)");
    std::cmatch match;
    if (std::regex_match(sizeRange.c_str(), match, pattern)) {
        if (match.size() == 5) {
            minSize->first = stoi(match[1]);
            minSize->second = stoi(match[2]);
            maxSize->first = stoi(match[3]);
            maxSize->second = stoi(match[4]);
            if (minSize->first > maxSize->first ||
                    minSize->second > maxSize->second) {
                ALOGE("%s: Invalid %s code size range: %s",
                        __FUNCTION__, codecName, sizeRange.c_str());
                return BAD_VALUE;
            }
        } else {
            return BAD_VALUE;
        }
    }

    auto res = getFrameRateMaps(details, frameRateMaps);
    if (res != OK) {
        return res;
    }

    return OK;
}

HeicEncoderInfoManager::FrameRateMaps::const_iterator HeicEncoderInfoManager::findClosestSize(
        const FrameRateMaps& maps, int32_t width, int32_t height) const {
    int32_t minDiff = INT32_MAX;
    FrameRateMaps::const_iterator closestIter = maps.begin();
    for (auto iter = maps.begin(); iter != maps.end(); iter++) {
        // Use area difference between the sizes to approximate size
        // difference.
        int32_t diff = abs(iter->first.first * iter->first.second - width * height);
        if (diff < minDiff) {
            closestIter = iter;
            minDiff = diff;
        }
    }
    return closestIter;
}

sp<AMessage> HeicEncoderInfoManager::getCodecDetails(
        sp<IMediaCodecList> codecsList, const char* name) {
    ssize_t idx = codecsList->findCodecByType(name, true /*encoder*/);
    if (idx < 0) {
        return nullptr;
    }

    const sp<MediaCodecInfo> info = codecsList->getCodecInfo(idx);
    if (info == nullptr) {
        ALOGE("%s: Failed to get codec info for %s", __FUNCTION__, name);
        return nullptr;
    }
    const sp<MediaCodecInfo::Capabilities> caps =
            info->getCapabilitiesFor(name);
    if (caps == nullptr) {
        ALOGE("%s: Failed to get capabilities for codec %s", __FUNCTION__, name);
        return nullptr;
    }
    const sp<AMessage> details = caps->getDetails();
    if (details == nullptr) {
        ALOGE("%s: Failed to get details for codec %s", __FUNCTION__, name);
        return nullptr;
    }

    return details;
}

bool HeicEncoderInfoManager::getHevcCodecDetails(
        sp<IMediaCodecList> codecsList, const char* mime) {
    bool found = false;
    ssize_t idx = 0;
    while ((idx = codecsList->findCodecByType(mime, true /*encoder*/, idx)) >= 0) {
        const sp<MediaCodecInfo> info = codecsList->getCodecInfo(idx++);
        if (info == nullptr) {
            ALOGE("%s: Failed to get codec info for %s", __FUNCTION__, mime);
            break;
        }

        // Filter out software ones as they may be too slow
        if (!(info->getAttributes() & MediaCodecInfo::kFlagIsHardwareAccelerated)) {
            continue;
        }

        const sp<MediaCodecInfo::Capabilities> caps =
                info->getCapabilitiesFor(mime);
        if (caps == nullptr) {
            ALOGE("%s: [%s] Failed to get capabilities", __FUNCTION__,
                    info->getCodecName());
            break;
        }
        const sp<AMessage> details = caps->getDetails();
        if (details == nullptr) {
            ALOGE("%s: [%s] Failed to get details", __FUNCTION__,
                    info->getCodecName());
            break;
        }

        // Check CQ mode
        AString bitrateModes;
        auto hasItem = details->findString("feature-bitrate-modes", &bitrateModes);
        if (!hasItem) {
            ALOGE("%s: [%s] Failed to query bitrate modes", __FUNCTION__,
                    info->getCodecName());
            break;
        }
        ALOGV("%s: [%s] feature-bitrate-modes value is %d, %s",
                __FUNCTION__, info->getCodecName(), hasItem, bitrateModes.c_str());
        std::regex pattern("(^|,)CQ($|,)", std::regex_constants::icase);
        if (!std::regex_search(bitrateModes.c_str(), pattern)) {
            continue; // move on to next encoder
        }

        std::pair<int32_t, int32_t> minSizeHevc, maxSizeHevc;
        FrameRateMaps hevcFrameRateMaps;
        auto res = getCodecSizeRange(MEDIA_MIMETYPE_VIDEO_HEVC,
                details, &minSizeHevc, &maxSizeHevc, &hevcFrameRateMaps);
        if (res != OK) {
            ALOGE("%s: [%s] Failed to get size range: %s (%d)", __FUNCTION__,
                    info->getCodecName(), strerror(-res), res);
            break;
        }
        if (kGridWidth < minSizeHevc.first
                || kGridWidth > maxSizeHevc.first
                || kGridHeight < minSizeHevc.second
                || kGridHeight > maxSizeHevc.second) {
            continue; // move on to next encoder
        }

        // Found: save name, size, frame rate
        mHevcName = info->getCodecName();
        mMinSizeHevc = minSizeHevc;
        mMaxSizeHevc = maxSizeHevc;
        mHevcFrameRateMaps = hevcFrameRateMaps;

        found = true;
        break;
    }

    return found;
}

} //namespace camera3
} // namespace android
