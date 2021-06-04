/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef AAUDIO_SHARED_RINGBUFFER_H
#define AAUDIO_SHARED_RINGBUFFER_H

#include <android-base/unique_fd.h>
#include <stdint.h>
#include <cutils/ashmem.h>
#include <sys/mman.h>

#include "fifo/FifoBuffer.h"
#include "binding/RingBufferParcelable.h"
#include "binding/AudioEndpointParcelable.h"

namespace aaudio {

// Determine the placement of the counters and data in shared memory.
#define SHARED_RINGBUFFER_READ_OFFSET   0
#define SHARED_RINGBUFFER_WRITE_OFFSET  sizeof(fifo_counter_t)
#define SHARED_RINGBUFFER_DATA_OFFSET   (SHARED_RINGBUFFER_WRITE_OFFSET + sizeof(fifo_counter_t))

/**
 * Atomic FIFO that uses shared memory.
 */
class SharedRingBuffer {
public:
    SharedRingBuffer() {}

    virtual ~SharedRingBuffer();

    aaudio_result_t allocate(android::fifo_frames_t bytesPerFrame, android::fifo_frames_t capacityInFrames);

    void fillParcelable(AudioEndpointParcelable &endpointParcelable,
                        RingBufferParcelable &ringBufferParcelable);

    android::FifoBuffer * getFifoBuffer() {
        return mFifoBuffer;
    }

private:
    android::base::unique_fd  mFileDescriptor;
    android::FifoBuffer      *mFifoBuffer = nullptr;
    uint8_t                  *mSharedMemory = nullptr;
    int32_t                   mSharedMemorySizeInBytes = 0;
    int32_t                   mDataMemorySizeInBytes = 0;
    android::fifo_frames_t    mCapacityInFrames = 0;
};

} /* namespace aaudio */

#endif //AAUDIO_SHARED_RINGBUFFER_H
