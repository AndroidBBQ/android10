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

#include "Observer.h"

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V2_0 {
namespace implementation {

Observer::Observer() {
}

Observer::~Observer() {
}

// Methods from ::android::hardware::media::bufferpool::V2_0::IObserver follow.
Return<void> Observer::onMessage(int64_t connectionId, uint32_t msgId) {
    std::unique_lock<std::mutex> lock(mLock);
    auto it = mClients.find(connectionId);
    if (it != mClients.end()) {
        const std::shared_ptr<BufferPoolClient> client = it->second.lock();
        if (!client) {
            mClients.erase(it);
        } else {
            lock.unlock();
            client->receiveInvalidation(msgId);
        }
    }
    return Void();
}

void Observer::addClient(ConnectionId connectionId,
                         const std::weak_ptr<BufferPoolClient> &wclient) {
    std::lock_guard<std::mutex> lock(mLock);
    for (auto it = mClients.begin(); it != mClients.end();) {
        if (!it->second.lock() || it->first == connectionId) {
            it = mClients.erase(it);
        } else {
            ++it;
        }
    }
    mClients.insert(std::make_pair(connectionId, wclient));

}

void Observer::delClient(ConnectionId connectionId) {
    std::lock_guard<std::mutex> lock(mLock);
    mClients.erase(connectionId);
}


}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android
