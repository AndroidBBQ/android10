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

#ifndef ANDROID_WIFI_SYSTEM_SUPPLICANT_MANAGER_H
#define ANDROID_WIFI_SYSTEM_SUPPLICANT_MANAGER_H

#include <android-base/macros.h>

namespace android {
namespace wifi_system {

class SupplicantManager {
 public:
  SupplicantManager() = default;
  virtual ~SupplicantManager() = default;

  // Request that supplicant be started.
  // Returns true on success.
  virtual bool StartSupplicant();

  // Request that a running instance of supplicant be stopped.
  // Returns true on success.
  virtual bool StopSupplicant();

  // Returns true iff supplicant is still running.
  virtual bool IsSupplicantRunning();

  // Returns true iff supplicant entropy file exists.
  static bool EnsureEntropyFileExists();

 private:
  DISALLOW_COPY_AND_ASSIGN(SupplicantManager);
};  // class SupplicantManager

}  // namespace wifi_system
}  // namespace android

#endif  // ANDROID_WIFI_SYSTEM_SUPPLICANT_MANAGER_H
