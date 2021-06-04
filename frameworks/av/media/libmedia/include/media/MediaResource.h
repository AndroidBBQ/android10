/*
 * Copyright 2015 The Android Open Source Project
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


#ifndef ANDROID_MEDIA_RESOURCE_H
#define ANDROID_MEDIA_RESOURCE_H

#include <binder/Parcel.h>
#include <utils/String8.h>

namespace android {

class MediaResource {
public:
    enum Type {
        kUnspecified = 0,
        kSecureCodec,
        kNonSecureCodec,
        kGraphicMemory,
        kCpuBoost,
        kBattery,
    };

    enum SubType {
        kUnspecifiedSubType = 0,
        kAudioCodec,
        kVideoCodec,
    };

    MediaResource();
    MediaResource(Type type, uint64_t value);
    MediaResource(Type type, SubType subType, uint64_t value);

    void readFromParcel(const Parcel &parcel);
    void writeToParcel(Parcel *parcel) const;

    String8 toString() const;

    bool operator==(const MediaResource &other) const;
    bool operator!=(const MediaResource &other) const;

    Type mType;
    SubType mSubType;
    uint64_t mValue;
};

inline static const char *asString(MediaResource::Type i, const char *def = "??") {
    switch (i) {
        case MediaResource::kUnspecified:    return "unspecified";
        case MediaResource::kSecureCodec:    return "secure-codec";
        case MediaResource::kNonSecureCodec: return "non-secure-codec";
        case MediaResource::kGraphicMemory:  return "graphic-memory";
        case MediaResource::kCpuBoost:       return "cpu-boost";
        case MediaResource::kBattery:        return "battery";
        default:                             return def;
    }
}

inline static const char *asString(MediaResource::SubType i, const char *def = "??") {
    switch (i) {
        case MediaResource::kUnspecifiedSubType: return "unspecified";
        case MediaResource::kAudioCodec:         return "audio-codec";
        case MediaResource::kVideoCodec:         return "video-codec";
        default:                                 return def;
    }
}

}; // namespace android

#endif  // ANDROID_MEDIA_RESOURCE_H
