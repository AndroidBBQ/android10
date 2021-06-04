/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef ANDROiD_SERVERS_CAMERA_CAMERA2HEAP_H
#define ANDROiD_SERVERS_CAMERA_CAMERA2HEAP_H

#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>

namespace android {
namespace camera2 {

// Utility class for managing a set of IMemory blocks
class Camera2Heap : public RefBase {
  public:
    explicit Camera2Heap(size_t buf_size, uint_t num_buffers = 1,
            const char *name = NULL) :
            mBufSize(buf_size),
            mNumBufs(num_buffers) {
        mHeap = new MemoryHeapBase(buf_size * num_buffers, 0, name);
        mBuffers = new sp<MemoryBase>[mNumBufs];
        for (uint_t i = 0; i < mNumBufs; i++)
            mBuffers[i] = new MemoryBase(mHeap,
                    i * mBufSize,
                    mBufSize);
    }

    virtual ~Camera2Heap()
    {
        delete [] mBuffers;
    }

    size_t mBufSize;
    uint_t mNumBufs;
    sp<MemoryHeapBase> mHeap;
    sp<MemoryBase> *mBuffers;
};

}; // namespace camera2
}; // namespace android

#endif
