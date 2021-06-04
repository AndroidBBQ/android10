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

#ifndef ANDROID_FRAMEWORKS_BUFFERHUB_V1_0_BUFFER_HUB_SERVICE_H
#define ANDROID_FRAMEWORKS_BUFFERHUB_V1_0_BUFFER_HUB_SERVICE_H

#include <map>
#include <mutex>
#include <set>
#include <vector>

#include <android/frameworks/bufferhub/1.0/IBufferHub.h>
#include <bufferhub/BufferClient.h>
#include <bufferhub/BufferHubIdGenerator.h>
#include <utils/Mutex.h>

namespace android {
namespace frameworks {
namespace bufferhub {
namespace V1_0 {
namespace implementation {

using hardware::hidl_handle;
using hardware::hidl_string;
using hardware::hidl_vec;
using hardware::Return;
using hardware::graphics::common::V1_2::HardwareBufferDescription;

class BufferHubService : public IBufferHub {
public:
    BufferHubService();

    Return<void> allocateBuffer(const HardwareBufferDescription& description,
                                const uint32_t userMetadataSize,
                                allocateBuffer_cb _hidl_cb) override;
    Return<void> importBuffer(const hidl_handle& tokenHandle, importBuffer_cb _hidl_cb) override;

    Return<void> debug(const hidl_handle& fd, const hidl_vec<hidl_string>& args) override;

    // Non-binder functions
    // Internal help function for IBufferClient::duplicate.
    hidl_handle registerToken(const wp<BufferClient>& client);

    void onClientClosed(const BufferClient* client);

private:
    // Helper function to build BufferTraits.bufferInfo handle
    hidl_handle buildBufferInfo(char* bufferInfoStorage, int bufferId, uint32_t clientBitMask,
                                uint32_t userMetadataSize, int metadataFd, int eventFd);

    // Helper function to remove all the token belongs to a specific client.
    void removeTokenByClient(const BufferClient* client);

    // List of active BufferClient for bookkeeping.
    std::mutex mClientSetMutex;
    std::set<wp<BufferClient>> mClientSet GUARDED_BY(mClientSetMutex);

    // Token generation related
    // A random number used as private key for HMAC
    uint64_t mKey;
    static constexpr size_t kKeyLen = sizeof(uint64_t);

    std::mutex mTokenMutex;
    // The first TokenId will be 1. TokenId could be negative.
    int mLastTokenId GUARDED_BY(mTokenMutex) = 0;
    static constexpr size_t mTokenIdSize = sizeof(int);
    // A map from token id to the token-buffer_client pair. Using token id as the key to reduce
    // looking up time
    std::map<int, std::pair<std::vector<uint8_t>, const wp<BufferClient>>> mTokenMap
            GUARDED_BY(mTokenMutex);
};

} // namespace implementation
} // namespace V1_0
} // namespace bufferhub
} // namespace frameworks
} // namespace android

#endif // ANDROID_FRAMEWORKS_BUFFERHUB_V1_0_BUFFER_HUB_SERVICE_H
