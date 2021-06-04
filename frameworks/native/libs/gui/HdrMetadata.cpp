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

#include <gui/HdrMetadata.h>
#include <limits>

namespace android {

size_t HdrMetadata::getFlattenedSize() const {
    size_t size = sizeof(validTypes);
    if (validTypes & SMPTE2086) {
        size += sizeof(smpte2086);
    }
    if (validTypes & CTA861_3) {
        size += sizeof(cta8613);
    }
    if (validTypes & HDR10PLUS) {
        size += sizeof(size_t);
        size += hdr10plus.size();
    }
    return size;
}

status_t HdrMetadata::flatten(void* buffer, size_t size) const {
    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }

    FlattenableUtils::write(buffer, size, validTypes);
    if (validTypes & SMPTE2086) {
        FlattenableUtils::write(buffer, size, smpte2086);
    }
    if (validTypes & CTA861_3) {
        FlattenableUtils::write(buffer, size, cta8613);
    }
    if (validTypes & HDR10PLUS) {
        size_t metadataSize = hdr10plus.size();
        FlattenableUtils::write(buffer, size, metadataSize);
        memcpy(buffer, hdr10plus.data(), metadataSize);
        FlattenableUtils::advance(buffer, size, metadataSize);
    }

    return NO_ERROR;
}

status_t HdrMetadata::unflatten(void const* buffer, size_t size) {
    if (size < sizeof(validTypes)) {
        return NO_MEMORY;
    }
    FlattenableUtils::read(buffer, size, validTypes);
    if (validTypes & SMPTE2086) {
        if (size < sizeof(smpte2086)) {
            return NO_MEMORY;
        }
        FlattenableUtils::read(buffer, size, smpte2086);
    }
    if (validTypes & CTA861_3) {
        if (size < sizeof(cta8613)) {
            return NO_MEMORY;
        }
        FlattenableUtils::read(buffer, size, cta8613);
    }
    if (validTypes & HDR10PLUS) {
        if (size < sizeof(size_t)) {
            return NO_MEMORY;
        }

        size_t metadataSize;
        FlattenableUtils::read(buffer, size, metadataSize);

        if (size < metadataSize) {
            return NO_MEMORY;
        }

        hdr10plus.resize(metadataSize);
        memcpy(hdr10plus.data(), buffer, metadataSize);
        FlattenableUtils::advance(buffer, size, metadataSize);
    }

    return NO_ERROR;
}

bool HdrMetadata::operator==(const HdrMetadata& rhs) const {
    if (validTypes != rhs.validTypes) return false;

    if ((validTypes & SMPTE2086) == SMPTE2086) {
        if (smpte2086.displayPrimaryRed.x != rhs.smpte2086.displayPrimaryRed.x ||
            smpte2086.displayPrimaryRed.y != rhs.smpte2086.displayPrimaryRed.y ||
            smpte2086.displayPrimaryGreen.x != rhs.smpte2086.displayPrimaryGreen.x ||
            smpte2086.displayPrimaryGreen.y != rhs.smpte2086.displayPrimaryGreen.y ||
            smpte2086.displayPrimaryBlue.x != rhs.smpte2086.displayPrimaryBlue.x ||
            smpte2086.displayPrimaryBlue.y != rhs.smpte2086.displayPrimaryBlue.y ||
            smpte2086.whitePoint.x != rhs.smpte2086.whitePoint.x ||
            smpte2086.whitePoint.y != rhs.smpte2086.whitePoint.y ||
            smpte2086.maxLuminance != rhs.smpte2086.maxLuminance ||
            smpte2086.minLuminance != rhs.smpte2086.minLuminance) {
            return false;
        }
    }

    if ((validTypes & CTA861_3) == CTA861_3) {
        if (cta8613.maxFrameAverageLightLevel != rhs.cta8613.maxFrameAverageLightLevel ||
            cta8613.maxContentLightLevel != rhs.cta8613.maxContentLightLevel) {
            return false;
        }
    }

    if ((validTypes & HDR10PLUS) == HDR10PLUS) {
        if (hdr10plus != rhs.hdr10plus) return false;
    }

    return true;
}

} // namespace android
