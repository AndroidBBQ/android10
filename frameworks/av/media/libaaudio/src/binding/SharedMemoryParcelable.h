/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef ANDROID_AAUDIO_SHARED_MEMORY_PARCELABLE_H
#define ANDROID_AAUDIO_SHARED_MEMORY_PARCELABLE_H

#include <stdint.h>
#include <sys/mman.h>

#include <android-base/unique_fd.h>
#include <binder/Parcel.h>
#include <binder/Parcelable.h>

namespace aaudio {

// Arbitrary limits for sanity checks. TODO remove after debugging.
#define MAX_SHARED_MEMORIES (32)
#define MAX_MMAP_OFFSET_BYTES (32 * 1024 * 8)
#define MAX_MMAP_SIZE_BYTES (32 * 1024 * 8)

/**
 * This is a parcelable description of a shared memory referenced by a file descriptor.
 * It may be divided into several regions.
 * The memory can be shared using Binder or simply shared between threads.
 */
class SharedMemoryParcelable : public android::Parcelable {
public:
    SharedMemoryParcelable();
    virtual ~SharedMemoryParcelable();

    /**
     * Make a dup() of the fd and store it for later use.
     *
     * @param fd
     * @param sizeInBytes
     */
    void setup(const android::base::unique_fd& fd, int32_t sizeInBytes);

    virtual android::status_t writeToParcel(android::Parcel* parcel) const override;

    virtual android::status_t readFromParcel(const android::Parcel* parcel) override;

    // mmap() shared memory
    aaudio_result_t resolve(int32_t offsetInBytes, int32_t sizeInBytes, void **regionAddressPtr);

    // munmap() any mapped memory
    aaudio_result_t close();

    int32_t getSizeInBytes();

    void dump();

protected:

#define MMAP_UNRESOLVED_ADDRESS    reinterpret_cast<uint8_t*>(MAP_FAILED)

    aaudio_result_t resolveSharedMemory(const android::base::unique_fd& fd);

    android::base::unique_fd   mFd;
    int32_t                    mSizeInBytes = 0;
    uint8_t                   *mResolvedAddress = MMAP_UNRESOLVED_ADDRESS;

private:

    aaudio_result_t validate() const;

};

} /* namespace aaudio */

#endif //ANDROID_AAUDIO_SHARED_MEMORY_PARCELABLE_H
