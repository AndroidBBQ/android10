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

#include "DirectReportChannel.h"
#include "utils.h"

namespace android {
namespace frameworks {
namespace sensorservice {
namespace V1_0 {
namespace implementation {

DirectReportChannel::DirectReportChannel(::android::SensorManager& manager, int channelId)
        : mManager(manager), mId(channelId) {}

DirectReportChannel::~DirectReportChannel() {
    mManager.destroyDirectChannel(mId);
}

// Methods from ::android::frameworks::sensorservice::V1_0::IDirectReportChannel follow.
Return<void> DirectReportChannel::configure(int32_t sensorHandle, RateLevel rate,
        configure_cb _hidl_cb) {
    int token = mManager.configureDirectChannel(mId,
            static_cast<int>(sensorHandle), static_cast<int>(rate));
    _hidl_cb(token <= 0 ? 0 : token,
             token <= 0 ? convertResult(token) : Result::OK);
    return Void();
}


}  // namespace implementation
}  // namespace V1_0
}  // namespace sensorservice
}  // namespace frameworks
}  // namespace android
