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

#ifndef ANDROID_WIFI_SYSTEM_INTERFACE_TOOL_H
#define ANDROID_WIFI_SYSTEM_INTERFACE_TOOL_H

#include <array>
#include <cstdint>
#include <linux/if_ether.h>

namespace android {
namespace wifi_system {

class InterfaceTool {
 public:
  InterfaceTool() = default;
  virtual ~InterfaceTool() = default;

  // Get the interface state of |if_name|.
  // Returns true iff the interface is up.
  virtual bool GetUpState(const char* if_name);

  // Set the interface named by |if_name| up or down.
  // Returns true on success, false otherwise.
  virtual bool SetUpState(const char* if_name, bool request_up);

  // A helpful alias for SetUpState() that assumes there is a single system
  // WiFi interface.  Prefer this form if you're hardcoding "wlan0" to help us
  // identify all the places we are hardcoding the name of the wifi interface.
  virtual bool SetWifiUpState(bool request_up);

  // Set the MAC address of the |if_name| interface
  // Returns true on success, false otherwise.
  virtual bool SetMacAddress(const char* if_name,
                             const std::array<uint8_t, ETH_ALEN>& address);

  // Get the factory MAC address of the |if_name| interface
  virtual std::array<uint8_t, ETH_ALEN> GetFactoryMacAddress(const char* if_name);
};  // class InterfaceTool

}  // namespace wifi_system
}  // namespace android

#endif  // ANDROID_WIFI_SYSTEM_INTERFACE_TOOL_H
