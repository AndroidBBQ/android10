/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "wifi_system/hostapd_manager.h"

#include <android-base/logging.h>
#include <cutils/properties.h>

namespace android {
namespace wifi_system {
const char kHostapdServiceName[] = "hostapd";

bool HostapdManager::StartHostapd() {
  if (property_set("ctl.start", kHostapdServiceName) != 0) {
    LOG(ERROR) << "Failed to start SoftAP";
    return false;
  }

  LOG(DEBUG) << "SoftAP started successfully";
  return true;
}

bool HostapdManager::StopHostapd() {
  LOG(DEBUG) << "Stopping the SoftAP service...";

  if (property_set("ctl.stop", kHostapdServiceName) < 0) {
    LOG(ERROR) << "Failed to stop hostapd service!";
    return false;
  }

  LOG(DEBUG) << "SoftAP stopped successfully";
  return true;
}
}  // namespace wifi_system
}  // namespace android
