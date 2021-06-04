/*
 * Copyright (C) 2011 The Android Open Source Project
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

#define LOG_TAG "InputApplication"

#include <input/InputApplication.h>

#include <android/log.h>

namespace android {

// --- InputApplicationHandle ---

InputApplicationHandle::InputApplicationHandle() {
}

InputApplicationHandle::~InputApplicationHandle() {
}

InputApplicationInfo InputApplicationInfo::read(const Parcel& from) {
    InputApplicationInfo ret;
    ret.token = from.readStrongBinder();
    ret.name = from.readString8().c_str();
    ret.dispatchingTimeout = from.readInt64();

    return ret;
}

status_t InputApplicationInfo::write(Parcel& output) const {
    output.writeStrongBinder(token);
    output.writeString8(String8(name.c_str()));
    output.writeInt64(dispatchingTimeout);
    
    return OK;
}

} // namespace android
