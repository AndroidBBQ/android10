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

#include <img_utils/EndianUtils.h>

namespace android {
namespace img_utils {

EndianOutput::EndianOutput(Output* out, Endianness end)
        : mOffset(0), mOutput(out), mEndian(end) {}

EndianOutput::~EndianOutput() {}

status_t EndianOutput::open() {
    mOffset = 0;
    return mOutput->open();
}

status_t EndianOutput::close() {
    return mOutput->close();
}

void EndianOutput::setEndianness(Endianness end) {
    mEndian = end;
}

uint32_t EndianOutput::getCurrentOffset() const {
    return mOffset;
}

Endianness EndianOutput::getEndianness() const {
    return mEndian;
}

status_t EndianOutput::write(const uint8_t* buf, size_t offset, size_t count) {
    status_t res = OK;
    if((res = mOutput->write(buf, offset, count)) == OK) {
        mOffset += count;
    }
    return res;
}

status_t EndianOutput::write(const int8_t* buf, size_t offset, size_t count) {
    return write(reinterpret_cast<const uint8_t*>(buf), offset, count);
}

#define DEFINE_WRITE(_type_) \
status_t EndianOutput::write(const _type_* buf, size_t offset, size_t count) { \
    return writeHelper<_type_>(buf, offset, count); \
}

DEFINE_WRITE(uint16_t)
DEFINE_WRITE(int16_t)
DEFINE_WRITE(uint32_t)
DEFINE_WRITE(int32_t)
DEFINE_WRITE(uint64_t)
DEFINE_WRITE(int64_t)

status_t EndianOutput::write(const float* buf, size_t offset, size_t count) {
    assert(sizeof(float) == sizeof(uint32_t));
    return writeHelper<uint32_t>(reinterpret_cast<const uint32_t*>(buf), offset, count);
}

status_t EndianOutput::write(const double* buf, size_t offset, size_t count) {
    assert(sizeof(double) == sizeof(uint64_t));
    return writeHelper<uint64_t>(reinterpret_cast<const uint64_t*>(buf), offset, count);
}

} /*namespace img_utils*/
} /*namespace android*/
