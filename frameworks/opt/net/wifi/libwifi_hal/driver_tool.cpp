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

#include <grp.h>
#include <pwd.h>
#include <sys/types.h>

#include "wifi_hal/driver_tool.h"

#include <android-base/logging.h>

#include "hardware_legacy/wifi.h"

namespace android {
namespace wifi_hal {

const int DriverTool::kFirmwareModeSta = WIFI_GET_FW_PATH_STA;
const int DriverTool::kFirmwareModeAp = WIFI_GET_FW_PATH_AP;
const int DriverTool::kFirmwareModeP2p = WIFI_GET_FW_PATH_P2P;

bool DriverTool::LoadDriver() {
  return ::wifi_load_driver() == 0;
}

bool DriverTool::UnloadDriver() {
  return ::wifi_unload_driver() == 0;
}

bool DriverTool::IsDriverLoaded() {
  return ::is_wifi_driver_loaded() != 0;
}

bool DriverTool::IsFirmwareModeChangeNeeded(int mode) {
  return (wifi_get_fw_path(mode) != nullptr);
}

bool DriverTool::ChangeFirmwareMode(int mode) {
  const char* fwpath = wifi_get_fw_path(mode);
  if (!fwpath) {
    return true;  // HAL doesn't think we need to load firmware for this mode.
  }
  if (wifi_change_fw_path(fwpath) != 0) {
    // Not all devices actually require firmware reloads, but
    // failure to change the firmware path when it is defined is an error.
    return false;
  }
  return true;
}

}  // namespace wifi_hal
}  // namespace android
