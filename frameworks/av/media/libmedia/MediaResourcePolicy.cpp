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
#define LOG_TAG "MediaResourcePolicy"
#include <utils/Log.h>
#include <media/MediaResourcePolicy.h>

namespace android {

const char kPolicySupportsMultipleSecureCodecs[] = "supports-multiple-secure-codecs";
const char kPolicySupportsSecureWithNonSecureCodec[] = "supports-secure-with-non-secure-codec";

MediaResourcePolicy::MediaResourcePolicy() {}

MediaResourcePolicy::MediaResourcePolicy(String8 type, String8 value)
        : mType(type),
          mValue(value) {}

void MediaResourcePolicy::readFromParcel(const Parcel &parcel) {
    mType = parcel.readString8();
    mValue = parcel.readString8();
}

void MediaResourcePolicy::writeToParcel(Parcel *parcel) const {
    parcel->writeString8(mType);
    parcel->writeString8(mValue);
}

String8 MediaResourcePolicy::toString() const {
    String8 str;
    str.appendFormat("%s:%s", mType.string(), mValue.string());
    return str;
}

}; // namespace android
