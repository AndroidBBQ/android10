/*
 * Copyright 2016, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaCodecBuffer"
#include <utils/Log.h>

#include <media/MediaCodecBuffer.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>

namespace android {

MediaCodecBuffer::MediaCodecBuffer(const sp<AMessage> &format, const sp<ABuffer> &buffer)
    : mMeta(new AMessage),
      mFormat(format),
      mBuffer(buffer) {
}

// ABuffer-like interface
uint8_t *MediaCodecBuffer::base() {
    return mBuffer->base();
}

uint8_t *MediaCodecBuffer::data() {
    return mBuffer->data();
}

size_t MediaCodecBuffer::capacity() const {
    return mBuffer->capacity();
}

size_t MediaCodecBuffer::size() const {
    return mBuffer->size();
}

size_t MediaCodecBuffer::offset() const {
    return mBuffer->offset();
}

status_t MediaCodecBuffer::setRange(size_t offset, size_t size) {
    mBuffer->setRange(offset, size);
    return OK;
}

sp<AMessage> MediaCodecBuffer::meta() {
    return mMeta;
}

sp<AMessage> MediaCodecBuffer::format() {
    return mFormat;
}

void MediaCodecBuffer::setFormat(const sp<AMessage> &format) {
    mMeta->clear();
    mFormat = format;
}

}  // namespace android
