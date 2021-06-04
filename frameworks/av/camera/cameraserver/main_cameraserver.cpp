/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "cameraserver"
//#define LOG_NDEBUG 0

#include "CameraService.h"
#include <hidl/HidlTransportSupport.h>

using namespace android;

int main(int argc __unused, char** argv __unused)
{
    signal(SIGPIPE, SIG_IGN);

    // Set 5 threads for HIDL calls. Now cameraserver will serve HIDL calls in
    // addition to consuming them from the Camera HAL as well.
    hardware::configureRpcThreadpool(5, /*willjoin*/ false);

    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();
    ALOGI("ServiceManager: %p", sm.get());
    CameraService::instantiate();
    ALOGI("ServiceManager: %p done instantiate", sm.get());
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();
}
