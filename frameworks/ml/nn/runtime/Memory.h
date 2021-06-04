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

#ifndef ANDROID_ML_NN_RUNTIME_MEMORY_H
#define ANDROID_ML_NN_RUNTIME_MEMORY_H

#include "NeuralNetworks.h"
#include "Utils.h"

#include <cutils/native_handle.h>
#include <sys/mman.h>
#include <mutex>
#include <unordered_map>
#include "vndk/hardware_buffer.h"

namespace android {
namespace nn {

class ExecutionBurstController;
class ModelBuilder;

// Represents a memory region.
class Memory {
   public:
    Memory() {}
    virtual ~Memory();

    // Disallow copy semantics to ensure the runtime object can only be freed
    // once. Copy semantics could be enabled if some sort of reference counting
    // or deep-copy system for runtime objects is added later.
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

    // Creates a shared memory object of the size specified in bytes.
    int create(uint32_t size);

    hardware::hidl_memory getHidlMemory() const { return mHidlMemory; }

    // Returns a pointer to the underlying memory of this memory object.
    // The function will fail if the memory is not CPU accessible and nullptr
    // will be returned.
    virtual int getPointer(uint8_t** buffer) const {
        *buffer = static_cast<uint8_t*>(static_cast<void*>(mMemory->getPointer()));
        if (*buffer == nullptr) {
            return ANEURALNETWORKS_BAD_DATA;
        }
        return ANEURALNETWORKS_NO_ERROR;
    }

    virtual bool validateSize(uint32_t offset, uint32_t length) const;

    // Unique key representing this memory object.
    intptr_t getKey() const;

    // Marks a burst object as currently using this memory. When this
    // memory object is destroyed, it will automatically free this memory from
    // the bursts' memory cache.
    void usedBy(const std::shared_ptr<ExecutionBurstController>& burst) const;

   protected:
    // The hidl_memory handle for this shared memory.  We will pass this value when
    // communicating with the drivers.
    hardware::hidl_memory mHidlMemory;
    sp<IMemory> mMemory;

    mutable std::mutex mMutex;
    // mUsedBy is essentially a set of burst objects which use this Memory
    // object. However, std::weak_ptr does not have comparison operations nor a
    // std::hash implementation. This is because it is either a valid pointer
    // (non-null) if the shared object is still alive, or it is null if the
    // object has been freed. To circumvent this, mUsedBy is a map with the raw
    // pointer as the key and the weak_ptr as the value.
    mutable std::unordered_map<const ExecutionBurstController*,
                               std::weak_ptr<ExecutionBurstController>>
            mUsedBy;
};

class MemoryFd : public Memory {
   public:
    MemoryFd() {}
    ~MemoryFd() override;

    // Disallow copy semantics to ensure the runtime object can only be freed
    // once. Copy semantics could be enabled if some sort of reference counting
    // or deep-copy system for runtime objects is added later.
    MemoryFd(const MemoryFd&) = delete;
    MemoryFd& operator=(const MemoryFd&) = delete;

    // Create the native_handle based on input size, prot, and fd.
    // Existing native_handle will be deleted, and mHidlMemory will wrap
    // the newly created native_handle.
    int set(size_t size, int prot, int fd, size_t offset);

    int getPointer(uint8_t** buffer) const override;

   private:
    native_handle_t* mHandle = nullptr;
    mutable uint8_t* mMapping = nullptr;
};

// TODO(miaowang): move function definitions to Memory.cpp
class MemoryAHWB : public Memory {
   public:
    MemoryAHWB() {}
    ~MemoryAHWB() override{};

    // Disallow copy semantics to ensure the runtime object can only be freed
    // once. Copy semantics could be enabled if some sort of reference counting
    // or deep-copy system for runtime objects is added later.
    MemoryAHWB(const MemoryAHWB&) = delete;
    MemoryAHWB& operator=(const MemoryAHWB&) = delete;

    // Keep track of the provided AHardwareBuffer handle.
    int set(const AHardwareBuffer* ahwb) {
        AHardwareBuffer_describe(ahwb, &mBufferDesc);
        const native_handle_t* handle = AHardwareBuffer_getNativeHandle(ahwb);
        mHardwareBuffer = ahwb;
        if (mBufferDesc.format == AHARDWAREBUFFER_FORMAT_BLOB) {
            mHidlMemory = hidl_memory("hardware_buffer_blob", handle, mBufferDesc.width);
        } else {
            // memory size is not used.
            mHidlMemory = hidl_memory("hardware_buffer", handle, 0);
        }
        return ANEURALNETWORKS_NO_ERROR;
    };

    int getPointer(uint8_t** buffer) const override {
        *buffer = nullptr;
        return ANEURALNETWORKS_BAD_DATA;
    };

    // validateSize should only be called for blob mode AHardwareBuffer.
    // Calling it on non-blob mode AHardwareBuffer will result in an error.
    // TODO(miaowang): consider separate blob and non-blob into different classes.
    bool validateSize(uint32_t offset, uint32_t length) const override {
        if (mHardwareBuffer == nullptr) {
            LOG(ERROR) << "MemoryAHWB has not been initialized.";
            return false;
        }
        // validateSize should only be called on BLOB mode buffer.
        if (mBufferDesc.format == AHARDWAREBUFFER_FORMAT_BLOB) {
            if (offset + length > mBufferDesc.width) {
                LOG(ERROR) << "Request size larger than the memory size.";
                return false;
            } else {
                return true;
            }
        } else {
            LOG(ERROR) << "Invalid AHARDWAREBUFFER_FORMAT, must be AHARDWAREBUFFER_FORMAT_BLOB.";
            return false;
        }
    }

   private:
    const AHardwareBuffer* mHardwareBuffer = nullptr;
    AHardwareBuffer_Desc mBufferDesc;
};

// A utility class to accumulate mulitple Memory objects and assign each
// a distinct index number, starting with 0.
//
// The user of this class is responsible for avoiding concurrent calls
// to this class from multiple threads.
class MemoryTracker {
   private:
    // The vector of Memory pointers we are building.
    std::vector<const Memory*> mMemories;
    // A faster way to see if we already have a memory than doing find().
    std::unordered_map<const Memory*, uint32_t> mKnown;

   public:
    // Adds the memory, if it does not already exists.  Returns its index.
    // The memories should survive the tracker.
    uint32_t add(const Memory* memory);
    // Returns the number of memories contained.
    uint32_t size() const { return static_cast<uint32_t>(mKnown.size()); }
    // Returns the ith memory.
    const Memory* operator[](size_t i) const { return mMemories[i]; }
    // Iteration
    decltype(mMemories.begin()) begin() { return mMemories.begin(); }
    decltype(mMemories.end()) end() { return mMemories.end(); }
};

}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_RUNTIME_MEMORY_H
