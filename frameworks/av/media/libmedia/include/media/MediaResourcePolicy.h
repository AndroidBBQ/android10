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


#ifndef ANDROID_MEDIA_RESOURCE_POLICY_H
#define ANDROID_MEDIA_RESOURCE_POLICY_H

#include <binder/Parcel.h>
#include <utils/String8.h>

namespace android {

extern const char kPolicySupportsMultipleSecureCodecs[];
extern const char kPolicySupportsSecureWithNonSecureCodec[];

class MediaResourcePolicy {
public:
    MediaResourcePolicy();
    MediaResourcePolicy(String8 type, String8 value);

    void readFromParcel(const Parcel &parcel);
    void writeToParcel(Parcel *parcel) const;

    String8 toString() const;

    String8 mType;
    String8 mValue;
};

}; // namespace android

#endif  // ANDROID_MEDIA_RESOURCE_POLICY_H
