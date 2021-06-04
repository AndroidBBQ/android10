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

#ifndef ANDROID_BUFFER_HUB_BUFFER_H_
#define ANDROID_BUFFER_HUB_BUFFER_H_

#include <android/frameworks/bufferhub/1.0/IBufferClient.h>
#include <android/hardware_buffer.h>
#include <cutils/native_handle.h>
#include <ui/BufferHubDefs.h>
#include <ui/BufferHubEventFd.h>
#include <ui/BufferHubMetadata.h>
#include <utils/NativeHandle.h>

namespace android {

class BufferHubBuffer {
public:
    // Allocates a standalone BufferHubBuffer.
    static std::unique_ptr<BufferHubBuffer> create(uint32_t width, uint32_t height,
                                                   uint32_t layerCount, uint32_t format,
                                                   uint64_t usage, size_t userMetadataSize);

    // Imports the given token to a BufferHubBuffer. Not taking ownership of the token.
    static std::unique_ptr<BufferHubBuffer> import(const sp<NativeHandle>& token);

    BufferHubBuffer(const BufferHubBuffer&) = delete;
    void operator=(const BufferHubBuffer&) = delete;

    virtual ~BufferHubBuffer();

    // Gets ID of the buffer client. All BufferHubBuffer clients derived from the same buffer in
    // BufferHub share the same buffer id.
    int id() const { return mId; }

    // Returns the buffer description, which is guaranteed to be faithful values from BufferHub.
    const AHardwareBuffer_Desc& desc() const { return mBufferDesc; }

    // Duplicate the underlying Gralloc buffer handle. Caller is responsible to free the handle
    // after use.
    native_handle_t* duplicateHandle() {
        return native_handle_clone(mBufferHandle.getNativeHandle());
    }

    const BufferHubEventFd& eventFd() const { return mEventFd; }

    // Returns the current value of MetadataHeader::bufferState.
    uint32_t bufferState() const { return mBufferState->load(std::memory_order_acquire); }

    // A state mask which is unique to a buffer hub client among all its siblings sharing the same
    // concrete graphic buffer.
    uint32_t clientStateMask() const { return mClientStateMask; }

    size_t userMetadataSize() const { return mMetadata.userMetadataSize(); }

    // Returns true if the BufferClient is still alive.
    bool isConnected() const { return mBufferClient->ping().isOk(); }

    // Returns true if the buffer is valid: non-null buffer handle, valid id, valid client bit mask,
    // valid metadata and valid buffer client
    bool isValid() const;

    // Gains the buffer for exclusive write permission. Read permission is implied once a buffer is
    // gained.
    // The buffer can be gained as long as there is no other client in acquired or gained state.
    int gain();

    // Posts the gained buffer for other buffer clients to use the buffer.
    // The buffer can be posted iff the buffer state for this client is gained.
    // After posting the buffer, this client is put to released state and does not have access to
    // the buffer for this cycle of the usage of the buffer.
    int post();

    // Acquires the buffer for shared read permission.
    // The buffer can be acquired iff the buffer state for this client is posted.
    int acquire();

    // Releases the buffer.
    // The buffer can be released from any buffer state.
    // After releasing the buffer, this client no longer have any permissions to the buffer for the
    // current cycle of the usage of the buffer.
    int release();

    // Returns whether the buffer is released by all active clients or not.
    bool isReleased() const;

    // Creates a token that stands for this BufferHubBuffer client and could be used for Import to
    // create another BufferHubBuffer. The new BufferHubBuffer will share the same underlying
    // gralloc buffer and ashmem region for metadata. Not taking ownership of the token.
    // Returns a valid token on success, nullptr on failure.
    sp<NativeHandle> duplicate();

private:
    BufferHubBuffer(uint32_t width, uint32_t height, uint32_t layerCount, uint32_t format,
                    uint64_t usage, size_t userMetadataSize);

    BufferHubBuffer(const sp<NativeHandle>& token);

    int initWithBufferTraits(const frameworks::bufferhub::V1_0::BufferTraits& bufferTraits);

    // Global id for the buffer that is consistent across processes.
    int mId = 0;

    // Client state mask of this BufferHubBuffer object. It is unique amoung all
    // clients/users of the buffer.
    uint32_t mClientStateMask = 0U;

    // Stores ground truth of the buffer.
    AHardwareBuffer_Desc mBufferDesc;

    // Wraps the gralloc buffer handle of this buffer.
    hardware::hidl_handle mBufferHandle;

    // Event fd used for signalling buffer state changes. Shared by all clients of the same buffer.
    BufferHubEventFd mEventFd;

    // An ashmem-based metadata object. The same shared memory are mapped to the
    // bufferhubd daemon and all buffer clients.
    BufferHubMetadata mMetadata;
    // Shortcuts to the atomics inside the header of mMetadata.
    std::atomic<uint32_t>* mBufferState = nullptr;
    std::atomic<uint32_t>* mFenceState = nullptr;
    std::atomic<uint32_t>* mActiveClientsBitMask = nullptr;

    // HwBinder backend
    sp<frameworks::bufferhub::V1_0::IBufferClient> mBufferClient;
};

} // namespace android

#endif // ANDROID_BUFFER_HUB_BUFFER_H_
