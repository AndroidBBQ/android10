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

#ifndef ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_OBSERVER_H
#define ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_OBSERVER_H

#include <android/hardware/media/bufferpool/2.0/IObserver.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "BufferPoolClient.h"

namespace android {
namespace hardware {
namespace media {
namespace bufferpool {
namespace V2_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Observer : public IObserver {
    // Methods from ::android::hardware::media::bufferpool::V2_0::IObserver follow.
    Return<void> onMessage(int64_t connectionId, uint32_t msgId) override;

    ~Observer();

    void addClient(ConnectionId connectionId,
                   const std::weak_ptr<BufferPoolClient> &wclient);

    void delClient(ConnectionId connectionId);

private:
    Observer();

    friend struct ClientManager;

    std::mutex mLock;
    std::map<ConnectionId, const std::weak_ptr<BufferPoolClient>> mClients;
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace bufferpool
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_BUFFERPOOL_V2_0_OBSERVER_H
