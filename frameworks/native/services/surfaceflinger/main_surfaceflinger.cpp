/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include <sys/resource.h>

#include <sched.h>

#include <android/frameworks/displayservice/1.0/IDisplayService.h>
#include <android/hardware/configstore/1.0/ISurfaceFlingerConfigs.h>
#include <android/hardware/graphics/allocator/2.0/IAllocator.h>
#include <android/hardware/graphics/allocator/3.0/IAllocator.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <configstore/Utils.h>
#include <displayservice/DisplayService.h>
#include <hidl/LegacySupport.h>
#include <processgroup/sched_policy.h>
#include "SurfaceFlinger.h"
#include "SurfaceFlingerFactory.h"
#include "SurfaceFlingerProperties.h"

using namespace android;

static status_t startGraphicsAllocatorService() {
    using android::hardware::configstore::getBool;
    using android::hardware::configstore::V1_0::ISurfaceFlingerConfigs;
    if (!android::sysprop::start_graphics_allocator_service(false)) {
        return OK;
    }

    status_t result = hardware::registerPassthroughServiceImplementation<
            android::hardware::graphics::allocator::V3_0::IAllocator>();
    if (result == OK) {
        return OK;
    }

    result = hardware::registerPassthroughServiceImplementation<
            android::hardware::graphics::allocator::V2_0::IAllocator>();
    if (result != OK) {
        ALOGE("could not start graphics allocator service");
        return result;
    }

    return OK;
}

static status_t startDisplayService() {
    using android::frameworks::displayservice::V1_0::implementation::DisplayService;
    using android::frameworks::displayservice::V1_0::IDisplayService;

    sp<IDisplayService> displayservice = new DisplayService();
    status_t err = displayservice->registerAsService();

    if (err != OK) {
        ALOGE("Could not register IDisplayService service.");
    }

    return err;
}

int main(int, char**) {
    signal(SIGPIPE, SIG_IGN);

    hardware::configureRpcThreadpool(1 /* maxThreads */,
            false /* callerWillJoin */);

    startGraphicsAllocatorService();

    // When SF is launched in its own process, limit the number of
    // binder threads to 4.
    ProcessState::self()->setThreadPoolMaxThreadCount(4);

    // start the thread pool
    sp<ProcessState> ps(ProcessState::self());
    ps->startThreadPool();

    // instantiate surfaceflinger
    sp<SurfaceFlinger> flinger = surfaceflinger::createSurfaceFlinger();

    setpriority(PRIO_PROCESS, 0, PRIORITY_URGENT_DISPLAY);

    set_sched_policy(0, SP_FOREGROUND);

    // Put most SurfaceFlinger threads in the system-background cpuset
    // Keeps us from unnecessarily using big cores
    // Do this after the binder thread pool init
    if (cpusets_enabled()) set_cpuset_policy(0, SP_SYSTEM);

    // initialize before clients can connect
    flinger->init();

    // publish surface flinger
    sp<IServiceManager> sm(defaultServiceManager());
    sm->addService(String16(SurfaceFlinger::getServiceName()), flinger, false,
                   IServiceManager::DUMP_FLAG_PRIORITY_CRITICAL | IServiceManager::DUMP_FLAG_PROTO);

    startDisplayService(); // dependency on SF getting registered above

    struct sched_param param = {0};
    param.sched_priority = 2;
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        ALOGE("Couldn't set SCHED_FIFO");
    }

    // run surface flinger in this thread
    flinger->run();

    return 0;
}
