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

#ifndef SECURE_BUFFER_H_

#define SECURE_BUFFER_H_

#include <media/ICrypto.h>
#include <media/MediaCodecBuffer.h>

namespace android {

class NativeHandle;

/**
 * Secure MediaCodecBuffer implementation.
 *
 * For classes outside of MediaCodec, this buffer is an opaque buffer only with
 * the size information. For decryption, it exposes underlying handle/pointer
 * and its type, which can be fed to ICrypto::decrypt().
 */
class SecureBuffer : public MediaCodecBuffer {
public:
    SecureBuffer(const sp<AMessage> &format, const void *ptr, size_t size);
    SecureBuffer(const sp<AMessage> &format, const sp<NativeHandle> &handle, size_t size);

    virtual ~SecureBuffer() = default;

    void *getDestinationPointer();
    ICrypto::DestinationType getDestinationType();

private:
    SecureBuffer() = delete;

    const void *mPointer;
    const sp<NativeHandle> mHandle;
};

}  // namespace android

#endif  // SECURE_BUFFER_H_
