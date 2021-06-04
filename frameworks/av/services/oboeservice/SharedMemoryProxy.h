/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef AAUDIO_SHARED_MEMORY_PROXY_H
#define AAUDIO_SHARED_MEMORY_PROXY_H

#include <stdint.h>
#include <cutils/ashmem.h>
#include <sys/mman.h>

#include <aaudio/AAudio.h>

namespace aaudio {

/**
 * Proxy for sharing memory between two file descriptors.
 */
class SharedMemoryProxy {
public:
    SharedMemoryProxy() {}

    ~SharedMemoryProxy();

    aaudio_result_t open(int fd, int32_t capacityInBytes);

    int getFileDescriptor() const {
        return mProxyFileDescriptor;
    }

private:
    int            mOriginalFileDescriptor = -1;
    int            mProxyFileDescriptor = -1;
    uint8_t       *mOriginalSharedMemory = nullptr;
    uint8_t       *mProxySharedMemory = nullptr;
    int32_t        mSharedMemorySizeInBytes = 0;
};

} /* namespace aaudio */

#endif //AAUDIO_SHARED_MEMORY_PROXY_H
