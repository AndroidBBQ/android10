/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_BUFFER_HUB_METADATA_H_
#define ANDROID_BUFFER_HUB_METADATA_H_

#include <android-base/unique_fd.h>
#include <ui/BufferHubDefs.h>

namespace android {

namespace {
using base::unique_fd;
} // namespace

class BufferHubMetadata {
public:
    // Creates a new BufferHubMetadata backed by an ashmem region.
    //
    // @param userMetadataSize Size in bytes of the user defined metadata. The entire metadata
    //        shared memory region to be allocated is the size of canonical
    //        BufferHubDefs::MetadataHeader plus userMetadataSize.
    static BufferHubMetadata create(size_t userMetadataSize);

    // Imports an existing BufferHubMetadata from an ashmem FD.
    //
    // @param ashmemFd Ashmem file descriptor representing an ashmem region.
    static BufferHubMetadata import(unique_fd ashmemFd);

    BufferHubMetadata() = default;

    BufferHubMetadata(BufferHubMetadata&& other) { *this = std::move(other); }

    ~BufferHubMetadata();

    BufferHubMetadata& operator=(BufferHubMetadata&& other) {
        if (this != &other) {
            mUserMetadataSize = other.mUserMetadataSize;
            other.mUserMetadataSize = 0;

            mAshmemFd = std::move(other.mAshmemFd);

            // The old raw mMetadataHeader pointer must be cleared, otherwise the destructor will
            // automatically mummap() the shared memory.
            mMetadataHeader = other.mMetadataHeader;
            other.mMetadataHeader = nullptr;
        }
        return *this;
    }

    // Returns true if the metadata is valid, i.e. the metadata has a valid ashmem fd and the ashmem
    // has been mapped into virtual address space.
    bool isValid() const { return mAshmemFd.get() != -1 && mMetadataHeader != nullptr; }

    size_t userMetadataSize() const { return mUserMetadataSize; }
    size_t metadataSize() const { return mUserMetadataSize + BufferHubDefs::kMetadataHeaderSize; }

    const unique_fd& ashmemFd() const { return mAshmemFd; }
    BufferHubDefs::MetadataHeader* metadataHeader() { return mMetadataHeader; }

private:
    BufferHubMetadata(size_t userMetadataSize, unique_fd ashmemFd,
                      BufferHubDefs::MetadataHeader* metadataHeader);

    BufferHubMetadata(const BufferHubMetadata&) = delete;
    void operator=(const BufferHubMetadata&) = delete;

    size_t mUserMetadataSize = 0;
    unique_fd mAshmemFd;
    BufferHubDefs::MetadataHeader* mMetadataHeader = nullptr;
};

} // namespace android

#endif // ANDROID_BUFFER_HUB_METADATA_H_
