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

#define LOG_TAG "Memory"

#include "Memory.h"

#include "ExecutionBurstController.h"
#include "HalInterfaces.h"
#include "Utils.h"

namespace android {
namespace nn {

Memory::~Memory() {
    for (const auto [ptr, weakBurst] : mUsedBy) {
        if (const std::shared_ptr<ExecutionBurstController> burst = weakBurst.lock()) {
            burst->freeMemory(getKey());
        }
    }
}

int Memory::create(uint32_t size) {
    mHidlMemory = allocateSharedMemory(size);
    mMemory = mapMemory(mHidlMemory);
    if (mMemory == nullptr) {
        LOG(ERROR) << "Memory::create failed";
        return ANEURALNETWORKS_OUT_OF_MEMORY;
    }
    return ANEURALNETWORKS_NO_ERROR;
}

bool Memory::validateSize(uint32_t offset, uint32_t length) const {
    if (offset + length > mHidlMemory.size()) {
        LOG(ERROR) << "Request size larger than the memory size.";
        return false;
    } else {
        return true;
    }
}

intptr_t Memory::getKey() const {
    return reinterpret_cast<intptr_t>(this);
}

void Memory::usedBy(const std::shared_ptr<ExecutionBurstController>& burst) const {
    std::lock_guard<std::mutex> guard(mMutex);
    mUsedBy.emplace(burst.get(), burst);
}

MemoryFd::~MemoryFd() {
    // Unmap the memory.
    if (mMapping) {
        munmap(mMapping, mHidlMemory.size());
    }
    // Delete the native_handle.
    if (mHandle) {
        int fd = mHandle->data[0];
        if (fd != -1) {
            close(fd);
        }
        native_handle_delete(mHandle);
    }
}

int MemoryFd::set(size_t size, int prot, int fd, size_t offset) {
    if (size == 0 || fd < 0) {
        LOG(ERROR) << "Invalid size or fd";
        return ANEURALNETWORKS_BAD_DATA;
    }
    int dupfd = dup(fd);
    if (dupfd == -1) {
        LOG(ERROR) << "Failed to dup the fd";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    if (mMapping) {
        if (munmap(mMapping, mHidlMemory.size()) != 0) {
            LOG(ERROR) << "Failed to remove the existing mapping";
            // This is not actually fatal.
        }
        mMapping = nullptr;
    }
    if (mHandle) {
        native_handle_delete(mHandle);
    }
    mHandle = native_handle_create(1, 3);
    if (mHandle == nullptr) {
        LOG(ERROR) << "Failed to create native_handle";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    mHandle->data[0] = dupfd;
    mHandle->data[1] = prot;
    mHandle->data[2] = (int32_t)(uint32_t)(offset & 0xffffffff);
#if defined(__LP64__)
    mHandle->data[3] = (int32_t)(uint32_t)(offset >> 32);
#else
    mHandle->data[3] = 0;
#endif
    mHidlMemory = hidl_memory("mmap_fd", mHandle, size);
    return ANEURALNETWORKS_NO_ERROR;
}

int MemoryFd::getPointer(uint8_t** buffer) const {
    if (mMapping) {
        *buffer = mMapping;
        return ANEURALNETWORKS_NO_ERROR;
    }

    if (mHandle == nullptr) {
        LOG(ERROR) << "Memory not initialized";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    int fd = mHandle->data[0];
    int prot = mHandle->data[1];
    size_t offset = getSizeFromInts(mHandle->data[2], mHandle->data[3]);
    void* data = mmap(nullptr, mHidlMemory.size(), prot, MAP_SHARED, fd, offset);
    if (data == MAP_FAILED) {
        LOG(ERROR) << "MemoryFd::getPointer(): Can't mmap the file descriptor.";
        return ANEURALNETWORKS_UNMAPPABLE;
    } else {
        mMapping = *buffer = static_cast<uint8_t*>(data);
        return ANEURALNETWORKS_NO_ERROR;
    }
}

uint32_t MemoryTracker::add(const Memory* memory) {
    VLOG(MODEL) << __func__ << "(" << SHOW_IF_DEBUG(memory) << ")";
    // See if we already have this memory. If so,
    // return its index.
    auto i = mKnown.find(memory);
    if (i != mKnown.end()) {
        return i->second;
    }
    VLOG(MODEL) << "It's new";
    // It's a new one.  Save it an assign an index to it.
    size_t next = mKnown.size();
    if (next > 0xFFFFFFFF) {
        LOG(ERROR) << "ANeuralNetworks more than 2^32 memories.";
        return ANEURALNETWORKS_BAD_DATA;
    }
    uint32_t idx = static_cast<uint32_t>(next);
    mKnown[memory] = idx;
    mMemories.push_back(memory);
    return idx;
}

} // namespace nn
} // namespace android
