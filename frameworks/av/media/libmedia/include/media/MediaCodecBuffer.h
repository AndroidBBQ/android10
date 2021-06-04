/*
 * Copyright 2016, The Android Open Source Project
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

#ifndef MEDIA_CODEC_BUFFER_H_

#define MEDIA_CODEC_BUFFER_H_

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>

namespace android {

struct ABuffer;
struct AMessage;
class MediaBufferBase;

/**
 * Buffers used by MediaCodec.
 */
class MediaCodecBuffer : public RefBase {
public:
    MediaCodecBuffer(const sp<AMessage> &format, const sp<ABuffer> &buffer);

    /**
     * MediaCodec will release all references to the buffer when it's done using
     * it, so the destructor should return the buffer to the owner, such as OMX
     * components, buffer allocators, surfaces, etc.
     */
    virtual ~MediaCodecBuffer() = default;

    // ABuffer-like interface
    uint8_t *base();
    uint8_t *data();
    size_t capacity() const;
    size_t size() const;
    size_t offset() const;
    // Default implementation calls ABuffer::setRange() and returns OK.
    virtual status_t setRange(size_t offset, size_t size);

    // TODO: Specify each field for meta/format.
    sp<AMessage> meta();
    sp<AMessage> format();

    void setFormat(const sp<AMessage> &format);

private:
    MediaCodecBuffer() = delete;

    const sp<AMessage> mMeta;
    sp<AMessage> mFormat;
    const sp<ABuffer> mBuffer;
};

}  // namespace android

#endif  // MEDIA_CODEC_BUFFER_H_
