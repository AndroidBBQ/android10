/*
 * Copyright (C) 2017 The Android Open Source Project
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


#ifndef ANDROID_AUDIO_CLIENT_H
#define ANDROID_AUDIO_CLIENT_H

#include <binder/Parcel.h>
#include <binder/Parcelable.h>
#include <system/audio.h>
#include <utils/String16.h>

namespace android {

class AudioClient : public Parcelable {
 public:
    AudioClient() :
        clientUid(-1), clientPid(-1), clientTid(-1), packageName("") {}

    uid_t clientUid;
    pid_t clientPid;
    pid_t clientTid;
    String16 packageName;

    status_t readFromParcel(const Parcel *parcel) override {
        clientUid = parcel->readInt32();
        clientPid = parcel->readInt32();
        clientTid = parcel->readInt32();
        packageName = parcel->readString16();
        return NO_ERROR;
    }

    status_t writeToParcel(Parcel *parcel) const override {
        parcel->writeInt32(clientUid);
        parcel->writeInt32(clientPid);
        parcel->writeInt32(clientTid);
        parcel->writeString16(packageName);
        return NO_ERROR;
    }
};

}; // namespace android

#endif  // ANDROID_AUDIO_CLIENT_H
