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

#ifndef ANDROID_FRAMEWORKS_DISPLAYSERVICE_V1_0_DISPLAYSERVICE_H
#define ANDROID_FRAMEWORKS_DISPLAYSERVICE_V1_0_DISPLAYSERVICE_H

#include <android/frameworks/displayservice/1.0/IDisplayService.h>
#include <hidl/Status.h>

namespace android {
namespace frameworks {
namespace displayservice {
namespace V1_0 {
namespace implementation {

using ::android::hardware::Return;
using ::android::hardware::Void;

struct DisplayService : public IDisplayService {
    Return<sp<IDisplayEventReceiver>> getEventReceiver() override;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace displayservice
}  // namespace frameworks
}  // namespace android

#endif  // ANDROID_FRAMEWORKS_DISPLAYSERVICE_V1_0_DISPLAYSERVICE_H
