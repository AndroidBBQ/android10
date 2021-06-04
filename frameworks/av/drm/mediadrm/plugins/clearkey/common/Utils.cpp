/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include "Utils.h"

namespace android {

bool operator<(const Vector<uint8_t> &lhs, const Vector<uint8_t> &rhs) {
    if (lhs.size() < rhs.size()) {
        return true;
    } else if (lhs.size() > rhs.size()) {
        return false;
    }
    return memcmp((void *)lhs.array(), (void *)rhs.array(), rhs.size()) < 0;
}

std::string ByteArrayToHexString(const uint8_t* in_buffer, size_t length) {
    static const char kHexChars[] = "0123456789ABCDEF";

    // Each input byte creates two output hex characters.
    std::string out_buffer(length * 2, '\0');

    for (size_t i = 0; i < length; ++i) {
        char byte = in_buffer[i];
        out_buffer[(i * 2)] = kHexChars[(byte >> 4) & 0xf];
        out_buffer[(i * 2) + 1] = kHexChars[byte & 0xf];
    }
    return out_buffer;
}

} // namespace android
