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

#define LOG_TAG "SharedMemoryProxy"
//#define LOG_NDEBUG 0
#include <log/log.h>

#include <errno.h>
#include <string.h>

#include <aaudio/AAudio.h>
#include "SharedMemoryProxy.h"

using namespace aaudio;

SharedMemoryProxy::~SharedMemoryProxy()
{
    if (mOriginalSharedMemory != nullptr) {
        munmap(mOriginalSharedMemory, mSharedMemorySizeInBytes);
        mOriginalSharedMemory = nullptr;
    }
    if (mProxySharedMemory != nullptr) {
        munmap(mProxySharedMemory, mSharedMemorySizeInBytes);
        close(mProxyFileDescriptor);
        mProxySharedMemory = nullptr;
    }
}

aaudio_result_t SharedMemoryProxy::open(int originalFD, int32_t capacityInBytes) {
    mOriginalFileDescriptor = originalFD;
    mSharedMemorySizeInBytes = capacityInBytes;

    mProxyFileDescriptor = ashmem_create_region("AAudioProxyDataBuffer", mSharedMemorySizeInBytes);
    if (mProxyFileDescriptor < 0) {
        ALOGE("open() ashmem_create_region() failed %d", errno);
        return AAUDIO_ERROR_INTERNAL;
    }
    int err = ashmem_set_prot_region(mProxyFileDescriptor, PROT_READ|PROT_WRITE);
    if (err < 0) {
        ALOGE("open() ashmem_set_prot_region() failed %d", errno);
        close(mProxyFileDescriptor);
        mProxyFileDescriptor = -1;
        return AAUDIO_ERROR_INTERNAL; // TODO convert errno to a better AAUDIO_ERROR;
    }

    // Get original memory address.
    mOriginalSharedMemory = (uint8_t *) mmap(0, mSharedMemorySizeInBytes,
                         PROT_READ|PROT_WRITE,
                         MAP_SHARED,
                         mOriginalFileDescriptor, 0);
    if (mOriginalSharedMemory == MAP_FAILED) {
        ALOGE("open() original mmap(%d) failed %d (%s)",
                mOriginalFileDescriptor, errno, strerror(errno));
        return AAUDIO_ERROR_INTERNAL; // TODO convert errno to a better AAUDIO_ERROR;
    }

    // Map the fd to the same memory addresses.
    mProxySharedMemory = (uint8_t *) mmap(mOriginalSharedMemory, mSharedMemorySizeInBytes,
                         PROT_READ|PROT_WRITE,
                         MAP_SHARED,
                         mProxyFileDescriptor, 0);
    if (mProxySharedMemory != mOriginalSharedMemory) {
        ALOGE("open() proxy mmap(%d) failed %d", mProxyFileDescriptor, errno);
        munmap(mOriginalSharedMemory, mSharedMemorySizeInBytes);
        mOriginalSharedMemory = nullptr;
        close(mProxyFileDescriptor);
        mProxyFileDescriptor = -1;
        return AAUDIO_ERROR_INTERNAL; // TODO convert errno to a better AAUDIO_ERROR;
    }

    return AAUDIO_OK;
}
