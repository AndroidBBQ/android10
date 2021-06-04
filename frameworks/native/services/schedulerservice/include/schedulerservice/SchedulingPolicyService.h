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
#pragma once

#include <android/frameworks/schedulerservice/1.0/ISchedulingPolicyService.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace android {
namespace frameworks {
namespace schedulerservice {
namespace V1_0 {
namespace implementation {

using ::android::frameworks::schedulerservice::V1_0::ISchedulingPolicyService;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct SchedulingPolicyService : public ISchedulingPolicyService {
    Return<bool> requestPriority(int32_t pid, int32_t tid, int32_t priority) override;
    Return<int32_t> getMaxAllowedPriority() override;
private:
    bool isAllowed();
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace schedulerservice
}  // namespace frameworks
}  // namespace android
