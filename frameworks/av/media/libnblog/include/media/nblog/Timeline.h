/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_MEDIA_NBLOG_TIMELINE_H
#define ANDROID_MEDIA_NBLOG_TIMELINE_H

#include <stddef.h>

#include <audio_utils/fifo_index.h>
#include <utils/RefBase.h>

namespace android {
namespace NBLog {

// Located in shared memory, must be POD.
// Exactly one process must explicitly call the constructor or use placement new.
// Since this is a POD, the destructor is empty and unnecessary to call it explicitly.
struct Shared {
    Shared() /* mRear initialized via default constructor */ {}
    ~Shared() {}

    audio_utils_fifo_index  mRear;  // index one byte past the end of most recent Entry
    char    mBuffer[0];             // circular buffer for entries
};

// FIXME Timeline was intended to wrap Writer and Reader, but isn't actually used yet.
// For now it is just a namespace for sharedSize().
class Timeline : public RefBase {
public:
#if 0
    Timeline(size_t size, void *shared = NULL);
    virtual ~Timeline();
#endif

    // Input parameter 'size' is the desired size of the timeline in byte units.
    // Returns the size rounded up to a power-of-2, plus the constant size overhead for indices.
    static size_t sharedSize(size_t size);

#if 0
private:
    friend class    Writer;
    friend class    Reader;

    const size_t    mSize;      // circular buffer size in bytes, must be a power of 2
    bool            mOwn;       // whether I own the memory at mShared
    Shared* const   mShared;    // pointer to shared memory
#endif
};

}   // namespace NBLog
}   // namespace android

#endif  // ANDROID_MEDIA_NBLOG_TIMELINE_H
