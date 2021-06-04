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

#include <displayservice/DisplayService.h>
#include <displayservice/DisplayEventReceiver.h>

namespace android {
namespace frameworks {
namespace displayservice {
namespace V1_0 {
namespace implementation {

Return<sp<IDisplayEventReceiver>> DisplayService::getEventReceiver() {
    return new DisplayEventReceiver();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace displayservice
}  // namespace frameworks
}  // namespace android
