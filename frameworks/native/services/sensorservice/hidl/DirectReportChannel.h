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

#ifndef ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_DIRECTREPORTCHANNEL_H
#define ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_DIRECTREPORTCHANNEL_H

#include <android/frameworks/sensorservice/1.0/IDirectReportChannel.h>
#include <android/frameworks/sensorservice/1.0/types.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <sensor/SensorManager.h>

namespace android {
namespace frameworks {
namespace sensorservice {
namespace V1_0 {
namespace implementation {

using ::android::frameworks::sensorservice::V1_0::IDirectReportChannel;
using ::android::hardware::sensors::V1_0::RateLevel;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct DirectReportChannel final : public IDirectReportChannel {

    DirectReportChannel(::android::SensorManager& manager, int channelId);
    ~DirectReportChannel();

    // Methods from ::android::frameworks::sensorservice::V1_0::IDirectReportChannel follow.
    Return<void> configure(int32_t sensorHandle, RateLevel rate,
            configure_cb _hidl_cb) override;

private:
    ::android::SensorManager& mManager;
    const int mId;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace sensorservice
}  // namespace frameworks
}  // namespace android

#endif  // ANDROID_FRAMEWORKS_SENSORSERVICE_V1_0_DIRECTREPORTCHANNEL_H
