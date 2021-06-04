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

#include <bufferhub/BufferHubService.h>
#include <hidl/HidlTransportSupport.h>
#include <hwbinder/IPCThreadState.h>
#include <log/log.h>

using android::sp;
using android::frameworks::bufferhub::V1_0::IBufferHub;
using android::frameworks::bufferhub::V1_0::implementation::BufferHubService;

int main(int /*argc*/, char** /*argv*/) {
    ALOGI("Bootstrap bufferhub HIDL service.");

    android::hardware::configureRpcThreadpool(/*numThreads=*/1, /*willJoin=*/true);

    sp<IBufferHub> service = new BufferHubService();
    LOG_ALWAYS_FATAL_IF(service->registerAsService() != android::OK, "Failed to register service");

    android::hardware::joinRpcThreadpool();

    return 0;
}
