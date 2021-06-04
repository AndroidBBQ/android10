/*
 * Copyright 2014 The Android Open Source Project
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

#include <img_utils/ByteArrayOutput.h>

#include <utils/Log.h>

namespace android {
namespace img_utils {

ByteArrayOutput::ByteArrayOutput() {}

ByteArrayOutput::~ByteArrayOutput() {}

status_t ByteArrayOutput::open() {
    return OK;
}

status_t ByteArrayOutput::write(const uint8_t* buf, size_t offset, size_t count) {
    if (mByteArray.appendArray(buf + offset, count) < 0) {
        ALOGE("%s: Failed to write to ByteArrayOutput.", __FUNCTION__);
        return BAD_VALUE;
    }
    return OK;
}

status_t ByteArrayOutput::close() {
    mByteArray.clear();
    return OK;
}

size_t ByteArrayOutput::getSize() const {
    return mByteArray.size();
}

const uint8_t* ByteArrayOutput::getArray() const {
    return mByteArray.array();
}

} /*namespace img_utils*/
} /*namespace android*/
