/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef ANDROID_UI_BUFFERQUEUEDEFS_H
#define ANDROID_UI_BUFFERQUEUEDEFS_H

namespace android {
    namespace BufferQueueDefs {
        // BufferQueue will keep track of at most this value of buffers.
        // Attempts at runtime to increase the number of buffers past this
        // will fail.
        static constexpr int NUM_BUFFER_SLOTS = 64;

        enum {
            // A flag returned by dequeueBuffer when the client needs to call
            // requestBuffer immediately thereafter.
            BUFFER_NEEDS_REALLOCATION = 0x1,
            // A flag returned by dequeueBuffer when all mirrored slots should be
            // released by the client. This flag should always be processed first.
            RELEASE_ALL_BUFFERS       = 0x2,
        };

    } // namespace BufferQueueDefs
} // namespace android

#endif
