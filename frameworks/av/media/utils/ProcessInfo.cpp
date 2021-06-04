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

//#define LOG_NDEBUG 0
#define LOG_TAG "ProcessInfo"
#include <utils/Log.h>

#include <media/stagefright/ProcessInfo.h>

#include <binder/IPCThreadState.h>
#include <binder/IProcessInfoService.h>
#include <binder/IServiceManager.h>

namespace android {

ProcessInfo::ProcessInfo() {}

bool ProcessInfo::getPriority(int pid, int* priority) {
    sp<IBinder> binder = defaultServiceManager()->getService(String16("processinfo"));
    sp<IProcessInfoService> service = interface_cast<IProcessInfoService>(binder);

    size_t length = 1;
    int32_t state;
    static const int32_t INVALID_ADJ = -10000;
    static const int32_t NATIVE_ADJ = -1000;
    int32_t score = INVALID_ADJ;
    status_t err = service->getProcessStatesAndOomScoresFromPids(length, &pid, &state, &score);
    if (err != OK) {
        ALOGE("getProcessStatesAndOomScoresFromPids failed");
        return false;
    }
    ALOGV("pid %d state %d score %d", pid, state, score);
    if (score <= NATIVE_ADJ) {
        ALOGE("pid %d invalid OOM adjustments value %d", pid, score);
        return false;
    }

    // Use OOM adjustments value as the priority. Lower the value, higher the priority.
    *priority = score;
    return true;
}

bool ProcessInfo::isValidPid(int pid) {
    int callingPid = IPCThreadState::self()->getCallingPid();
    // Trust it if this is called from the same process otherwise pid has to match the calling pid.
    return (callingPid == getpid()) || (callingPid == pid);
}

ProcessInfo::~ProcessInfo() {}

}  // namespace android
