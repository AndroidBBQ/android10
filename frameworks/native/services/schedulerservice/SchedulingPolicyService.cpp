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
#define LOG_TAG "schedulerservicehidl"

#include "SchedulingPolicyService.h"

#include <private/android_filesystem_config.h> // for AID_CAMERASERVER

#include <log/log.h>
#include <hwbinder/IPCThreadState.h>
#include <mediautils/SchedulingPolicyService.h>

namespace android {
namespace frameworks {
namespace schedulerservice {
namespace V1_0 {
namespace implementation {

bool SchedulingPolicyService::isAllowed() {
    using ::android::hardware::IPCThreadState;

    return IPCThreadState::self()->getCallingUid() == AID_CAMERASERVER;
}

Return<bool> SchedulingPolicyService::requestPriority(int32_t pid, int32_t tid, int32_t priority) {
    if (priority < static_cast<int32_t>(Priority::MIN) ||
            priority > static_cast<int32_t>(Priority::MAX)) {
        return false;
    }

    if (!isAllowed()) {
        return false;
    }

    // TODO(b/37226359): decouple from and remove AIDL service
    // this should always be allowed since we are in system_server.
    int value = ::android::requestPriority(pid, tid, priority, false /* isForApp */);
    return value == 0 /* success */;
}

Return<int32_t> SchedulingPolicyService::getMaxAllowedPriority() {
    if (!isAllowed()) {
        return 0;
    }

    // TODO(b/37226359): decouple from and remove AIDL service
    return 3;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace schedulerservice
}  // namespace frameworks
}  // namespace android
