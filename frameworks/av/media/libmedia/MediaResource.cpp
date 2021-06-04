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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaResource"
#include <utils/Log.h>
#include <media/MediaResource.h>

namespace android {

MediaResource::MediaResource()
        : mType(kUnspecified),
          mSubType(kUnspecifiedSubType),
          mValue(0) {}

MediaResource::MediaResource(Type type, uint64_t value)
        : mType(type),
          mSubType(kUnspecifiedSubType),
          mValue(value) {}

MediaResource::MediaResource(Type type, SubType subType, uint64_t value)
        : mType(type),
          mSubType(subType),
          mValue(value) {}

void MediaResource::readFromParcel(const Parcel &parcel) {
    mType = static_cast<Type>(parcel.readInt32());
    mSubType = static_cast<SubType>(parcel.readInt32());
    mValue = parcel.readUint64();
}

void MediaResource::writeToParcel(Parcel *parcel) const {
    parcel->writeInt32(static_cast<int32_t>(mType));
    parcel->writeInt32(static_cast<int32_t>(mSubType));
    parcel->writeUint64(mValue);
}

String8 MediaResource::toString() const {
    String8 str;
    str.appendFormat("%s/%s:%llu", asString(mType), asString(mSubType), (unsigned long long)mValue);
    return str;
}

bool MediaResource::operator==(const MediaResource &other) const {
    return (other.mType == mType) && (other.mSubType == mSubType) && (other.mValue == mValue);
}

bool MediaResource::operator!=(const MediaResource &other) const {
    return !(*this == other);
}

}; // namespace android
