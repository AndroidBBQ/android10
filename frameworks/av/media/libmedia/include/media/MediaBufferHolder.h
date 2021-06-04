/*
 * Copyright 2018, The Android Open Source Project
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

#ifndef MEDIA_BUFFER_HOLDER_H_

#define MEDIA_BUFFER_HOLDER_H_

#include <media/stagefright/MediaBuffer.h>
#include <utils/RefBase.h>

namespace android {

struct MediaBufferHolder : public RefBase {
    MediaBufferHolder(MediaBufferBase* buffer)
        : mMediaBuffer(buffer) {
        if (mMediaBuffer != nullptr) {
            mMediaBuffer->add_ref();
        }
    }

    virtual ~MediaBufferHolder() {
        if (mMediaBuffer != nullptr) {
            mMediaBuffer->release();
        }
    }

    MediaBufferBase* mediaBuffer() { return mMediaBuffer; }

private:
    MediaBufferBase* const mMediaBuffer;
};

}  // android

#endif  // MEDIA_BUFFER_HOLDER_H_
