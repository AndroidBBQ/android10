/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "SchedulingPolicyService"
//#define LOG_NDEBUG 0

#include <binder/IServiceManager.h>
#include <utils/Mutex.h>
#include "ISchedulingPolicyService.h"
#include "mediautils/SchedulingPolicyService.h"

namespace android {

static sp<ISchedulingPolicyService> sSchedulingPolicyService;
static const String16 _scheduling_policy("scheduling_policy");
static Mutex sMutex;

int requestPriority(pid_t pid, pid_t tid, int32_t prio, bool isForApp, bool asynchronous)
{
    // FIXME merge duplicated code related to service lookup, caching, and error recovery
    int ret;
    for (;;) {
        sMutex.lock();
        sp<ISchedulingPolicyService> sps = sSchedulingPolicyService;
        sMutex.unlock();
        if (sps == 0) {
            sp<IBinder> binder = defaultServiceManager()->checkService(_scheduling_policy);
            if (binder == 0) {
                sleep(1);
                continue;
            }
            sps = interface_cast<ISchedulingPolicyService>(binder);
            sMutex.lock();
            sSchedulingPolicyService = sps;
            sMutex.unlock();
        }
        ret = sps->requestPriority(pid, tid, prio, isForApp, asynchronous);
        if (ret != DEAD_OBJECT) {
            break;
        }
        ALOGW("SchedulingPolicyService died");
        sMutex.lock();
        sSchedulingPolicyService.clear();
        sMutex.unlock();
    }
    return ret;
}

int requestCpusetBoost(bool enable, const sp<IInterface> &client)
{
    int ret;
    sMutex.lock();
    sp<ISchedulingPolicyService> sps = sSchedulingPolicyService;
    sMutex.unlock();
    if (sps == 0) {
        sp<IBinder> binder = defaultServiceManager()->checkService(_scheduling_policy);
        if (binder == 0) {
            return DEAD_OBJECT;
        }
        sps = interface_cast<ISchedulingPolicyService>(binder);
        sMutex.lock();
        sSchedulingPolicyService = sps;
        sMutex.unlock();
    }
    ret = sps->requestCpusetBoost(enable, client);
    if (ret != DEAD_OBJECT) {
        return ret;
    }
    ALOGW("SchedulingPolicyService died");
    sMutex.lock();
    sSchedulingPolicyService.clear();
    sMutex.unlock();
    return ret;
}

}   // namespace android
