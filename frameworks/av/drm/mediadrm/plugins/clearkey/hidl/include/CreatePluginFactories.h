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

#ifndef CLEARKEY_CREATE_PLUGIN_FACTORIES_H_
#define CLEARKEY_CREATE_PLUGIN_FACTORIES_H_

#include <android/hardware/drm/1.2/ICryptoFactory.h>
#include <android/hardware/drm/1.2/IDrmFactory.h>

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

using ::android::hardware::drm::V1_2::ICryptoFactory;
using ::android::hardware::drm::V1_2::IDrmFactory;

extern "C" {
    IDrmFactory* createDrmFactory();
    ICryptoFactory* createCryptoFactory();
}

}  // namespace clearkey
}  // namespace V1_2
}  // namespace drm
}  // namespace hardware
}  // namespace android
#endif // CLEARKEY_CREATE_PLUGIN_FACTORIES_H_
