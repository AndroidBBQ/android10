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

#ifndef ANDROID_AUDIO_FAST_CAPTURE_STATE_H
#define ANDROID_AUDIO_FAST_CAPTURE_STATE_H

#include <media/nbaio/NBAIO.h>
#include <media/AudioBufferProvider.h>
#include "FastThreadState.h"
#include <private/media/AudioTrackShared.h>

namespace android {

// Represent a single state of the fast capture
struct FastCaptureState : FastThreadState {
                FastCaptureState();
    /*virtual*/ ~FastCaptureState();

    // all pointer fields use raw pointers; objects are owned and ref-counted by RecordThread
    NBAIO_Source*   mInputSource;       // HAL input device, must already be negotiated
    // FIXME by renaming, could pull up these fields to FastThreadState
    int             mInputSourceGen;    // increment when mInputSource is assigned
    NBAIO_Sink*     mPipeSink;          // after reading from input source, write to this pipe sink
    int             mPipeSinkGen;       // increment when mPipeSink is assigned
    size_t          mFrameCount;        // number of frames per fast capture buffer
    audio_track_cblk_t* mCblk;          // control block for the single fast client, or NULL

    audio_format_t  mFastPatchRecordFormat = AUDIO_FORMAT_INVALID;
    AudioBufferProvider* mFastPatchRecordBufferProvider = nullptr;   // a reference to a patch
                                                                     // record in fast mode

    // Extends FastThreadState::Command
    static const Command
        // The following commands also process configuration changes, and can be "or"ed:
        READ = 0x8,                     // read from input source
        WRITE = 0x10,                   // write to pipe sink
        READ_WRITE = 0x18;              // read from input source and write to pipe sink

    // never returns NULL; asserts if command is invalid
    static const char *commandToString(Command command);
};  // struct FastCaptureState

}   // namespace android

#endif  // ANDROID_AUDIO_FAST_CAPTURE_STATE_H
