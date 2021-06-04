/*
 * Copyright (C) 2018 The Android Open Source Project
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

#undef LOG_TAG
#define LOG_TAG "DisplayIdentification"

#include <algorithm>
#include <cctype>
#include <numeric>
#include <optional>

#include <log/log.h>

#include "DisplayIdentification.h"

namespace android {
namespace {

using byte_view = std::basic_string_view<uint8_t>;

constexpr size_t kEdidHeaderLength = 5;

constexpr uint16_t kFallbackEdidManufacturerId = 0;
constexpr uint16_t kVirtualEdidManufacturerId = 0xffffu;

std::optional<uint8_t> getEdidDescriptorType(const byte_view& view) {
    if (view.size() < kEdidHeaderLength || view[0] || view[1] || view[2] || view[4]) {
        return {};
    }

    return view[3];
}

std::string_view parseEdidText(const byte_view& view) {
    std::string_view text(reinterpret_cast<const char*>(view.data()), view.size());
    text = text.substr(0, text.find('\n'));

    if (!std::all_of(text.begin(), text.end(), ::isprint)) {
        ALOGW("Invalid EDID: ASCII text is not printable.");
        return {};
    }

    return text;
}

// Big-endian 16-bit value encodes three 5-bit letters where A is 0b00001.
template <size_t I>
char getPnpLetter(uint16_t id) {
    static_assert(I < 3);
    const char letter = 'A' + (static_cast<uint8_t>(id >> ((2 - I) * 5)) & 0b00011111) - 1;
    return letter < 'A' || letter > 'Z' ? '\0' : letter;
}

} // namespace

uint16_t DisplayId::manufacturerId() const {
    return static_cast<uint16_t>(value >> 40);
}

DisplayId DisplayId::fromEdid(uint8_t port, uint16_t manufacturerId, uint32_t displayNameHash) {
    return {(static_cast<Type>(manufacturerId) << 40) | (static_cast<Type>(displayNameHash) << 8) |
            port};
}

bool isEdid(const DisplayIdentificationData& data) {
    const uint8_t kMagic[] = {0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0};
    return data.size() >= sizeof(kMagic) &&
            std::equal(std::begin(kMagic), std::end(kMagic), data.begin());
}

std::optional<Edid> parseEdid(const DisplayIdentificationData& edid) {
    constexpr size_t kMinLength = 128;
    if (edid.size() < kMinLength) {
        ALOGW("Invalid EDID: structure is truncated.");
        // Attempt parsing even if EDID is malformed.
    } else {
        ALOGW_IF(edid[126] != 0, "EDID extensions are currently unsupported.");
        ALOGW_IF(std::accumulate(edid.begin(), edid.begin() + kMinLength, static_cast<uint8_t>(0)),
                 "Invalid EDID: structure does not checksum.");
    }

    constexpr size_t kManufacturerOffset = 8;
    if (edid.size() < kManufacturerOffset + sizeof(uint16_t)) {
        ALOGE("Invalid EDID: manufacturer ID is truncated.");
        return {};
    }

    // Plug and play ID encoded as big-endian 16-bit value.
    const uint16_t manufacturerId =
            (edid[kManufacturerOffset] << 8) | edid[kManufacturerOffset + 1];

    const auto pnpId = getPnpId(manufacturerId);
    if (!pnpId) {
        ALOGE("Invalid EDID: manufacturer ID is not a valid PnP ID.");
        return {};
    }

    constexpr size_t kDescriptorOffset = 54;
    if (edid.size() < kDescriptorOffset) {
        ALOGE("Invalid EDID: descriptors are missing.");
        return {};
    }

    byte_view view(edid.data(), edid.size());
    view.remove_prefix(kDescriptorOffset);

    std::string_view displayName;
    std::string_view serialNumber;
    std::string_view asciiText;

    constexpr size_t kDescriptorCount = 4;
    constexpr size_t kDescriptorLength = 18;

    for (size_t i = 0; i < kDescriptorCount; i++) {
        if (view.size() < kDescriptorLength) {
            break;
        }

        if (const auto type = getEdidDescriptorType(view)) {
            byte_view descriptor(view.data(), kDescriptorLength);
            descriptor.remove_prefix(kEdidHeaderLength);

            switch (*type) {
                case 0xfc:
                    displayName = parseEdidText(descriptor);
                    break;
                case 0xfe:
                    asciiText = parseEdidText(descriptor);
                    break;
                case 0xff:
                    serialNumber = parseEdidText(descriptor);
                    break;
            }
        }

        view.remove_prefix(kDescriptorLength);
    }

    if (displayName.empty()) {
        ALOGW("Invalid EDID: falling back to serial number due to missing display name.");
        displayName = serialNumber;
    }
    if (displayName.empty()) {
        ALOGW("Invalid EDID: falling back to ASCII text due to missing serial number.");
        displayName = asciiText;
    }
    if (displayName.empty()) {
        ALOGE("Invalid EDID: display name and fallback descriptors are missing.");
        return {};
    }

    return Edid{manufacturerId, *pnpId, displayName};
}

std::optional<PnpId> getPnpId(uint16_t manufacturerId) {
    const char a = getPnpLetter<0>(manufacturerId);
    const char b = getPnpLetter<1>(manufacturerId);
    const char c = getPnpLetter<2>(manufacturerId);
    return a && b && c ? std::make_optional(PnpId{a, b, c}) : std::nullopt;
}

std::optional<PnpId> getPnpId(DisplayId displayId) {
    return getPnpId(displayId.manufacturerId());
}

std::optional<DisplayIdentificationInfo> parseDisplayIdentificationData(
        uint8_t port, const DisplayIdentificationData& data) {
    if (!isEdid(data)) {
        ALOGE("Display identification data has unknown format.");
        return {};
    }

    const auto edid = parseEdid(data);
    if (!edid) {
        return {};
    }

    // Hash display name instead of using product code or serial number, since the latter have been
    // observed to change on some displays with multiple inputs.
    const auto hash = static_cast<uint32_t>(std::hash<std::string_view>()(edid->displayName));
    return DisplayIdentificationInfo{DisplayId::fromEdid(port, edid->manufacturerId, hash),
                                     std::string(edid->displayName)};
}

DisplayId getFallbackDisplayId(uint8_t port) {
    return DisplayId::fromEdid(port, kFallbackEdidManufacturerId, 0);
}

DisplayId getVirtualDisplayId(uint32_t id) {
    return DisplayId::fromEdid(0, kVirtualEdidManufacturerId, id);
}

} // namespace android
