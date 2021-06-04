/*
 * Copyright 2017 The Android Open Source Project
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
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <hwbinder/IPCThreadState.h>
#include <impl/vr_hwc.h>
#include <inttypes.h>

#include "vr_composer.h"

int main() {
  android::ProcessState::self()->startThreadPool();

  // Register the hwbinder HWC HAL service used by SurfaceFlinger while in VR
  // mode.
  android::sp<android::dvr::VrHwc> service = new android::dvr::VrHwc();

  LOG_ALWAYS_FATAL_IF(!service.get(), "Failed to get service");
  LOG_ALWAYS_FATAL_IF(service->isRemote(), "Service is remote");

  const char instance[] = "vr";
  LOG_ALWAYS_FATAL_IF(service->registerAsService(instance) != android::OK,
                      "Failed to register service");

  android::sp<android::dvr::VrComposer> composer =
      new android::dvr::VrComposer(service.get());

  android::sp<android::IServiceManager> sm(android::defaultServiceManager());

  // Register the binder service used by VR Window Manager service to receive
  // frame information from VR HWC HAL.
  android::status_t status = sm->addService(
      android::dvr::VrComposer::SERVICE_NAME(), composer.get(),
      false /* allowIsolated */);
  LOG_ALWAYS_FATAL_IF(status != android::OK,
                      "VrDisplay service failed to start: %" PRId32, status);

  android::hardware::ProcessState::self()->startThreadPool();
  android::hardware::IPCThreadState::self()->joinThreadPool();

  return 0;
}
