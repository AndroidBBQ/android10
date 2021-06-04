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

#ifndef ANDROID_AUDIO_FAST_CAPTURE_DUMP_STATE_H
#define ANDROID_AUDIO_FAST_CAPTURE_DUMP_STATE_H

#include <stdint.h>
#include "Configuration.h"
#include "FastThreadDumpState.h"

namespace android {

struct FastCaptureDumpState : FastThreadDumpState {
    FastCaptureDumpState();
    /*virtual*/ ~FastCaptureDumpState();

    void dump(int fd) const;    // should only be called on a stable copy, not the original

    // FIXME by renaming, could pull up many of these to FastThreadDumpState
    uint32_t mReadSequence;     // incremented before and after each read()
    uint32_t mFramesRead;       // total number of frames read successfully
    uint32_t mReadErrors;       // total number of read() errors
    uint32_t mSampleRate;
    size_t   mFrameCount;
};

}   // android

#endif  // ANDROID_AUDIO_FAST_CAPTURE_DUMP_STATE_H
