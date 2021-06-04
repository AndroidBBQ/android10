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

#ifndef ANDROID_FRAMEWORKS_BUFFERHUB_V1_0_BUFFER_CLIENT_H
#define ANDROID_FRAMEWORKS_BUFFERHUB_V1_0_BUFFER_CLIENT_H

#include <mutex>

#include <android/frameworks/bufferhub/1.0/IBufferClient.h>
#include <bufferhub/BufferNode.h>

namespace android {
namespace frameworks {
namespace bufferhub {
namespace V1_0 {
namespace implementation {

using hardware::hidl_handle;
using hardware::Return;

// Forward declaration to avoid circular dependency
class BufferHubService;

class BufferClient : public IBufferClient {
public:
    // Creates a server-side buffer client from an existing BufferNode. Note that
    // this function takes ownership of the shared_ptr.
    // Returns a raw pointer to the BufferClient on success, nullptr on failure.
    static BufferClient* create(BufferHubService* service, const std::shared_ptr<BufferNode>& node);

    // Creates a BufferClient from an existing BufferClient. Will share the same BufferNode.
    explicit BufferClient(const BufferClient& other)
          : mService(other.mService), mBufferNode(other.mBufferNode) {}
    ~BufferClient();

    Return<BufferHubStatus> close() override;
    Return<void> duplicate(duplicate_cb _hidl_cb) override;

    // Non-binder functions
    const std::shared_ptr<BufferNode>& getBufferNode() const { return mBufferNode; }

private:
    BufferClient(wp<BufferHubService> service, const std::shared_ptr<BufferNode>& node)
          : mService(service), mBufferNode(node) {}

    sp<BufferHubService> getService();

    wp<BufferHubService> mService;

    std::mutex mClosedMutex;
    bool mClosed GUARDED_BY(mClosedMutex) = false;

    std::shared_ptr<BufferNode> mBufferNode;
};

} // namespace implementation
} // namespace V1_0
} // namespace bufferhub
} // namespace frameworks
} // namespace android

#endif
