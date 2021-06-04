/*
 * Copyright (C) 2013 The Android Open Source Project
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

#define LOG_TAG "InputFlinger"

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <binder/IPCThreadState.h>
#include <binder/PermissionCache.h>
#include <hardware/input.h>
#include <log/log.h>
#include <private/android_filesystem_config.h>

#include "InputFlinger.h"
#include "InputDriver.h"

namespace android {

const String16 sAccessInputFlingerPermission("android.permission.ACCESS_INPUT_FLINGER");
const String16 sDumpPermission("android.permission.DUMP");


InputFlinger::InputFlinger() :
        BnInputFlinger() {
    ALOGI("InputFlinger is starting");
    mHost = new InputHost();
    mHost->registerInputDriver(new InputDriver(INPUT_INSTANCE_EVDEV));
}

InputFlinger::~InputFlinger() {
}

status_t InputFlinger::dump(int fd, const Vector<String16>& args) {
    String8 result;
    const IPCThreadState* ipc = IPCThreadState::self();
    const int pid = ipc->getCallingPid();
    const int uid = ipc->getCallingUid();
    if ((uid != AID_SHELL)
            && !PermissionCache::checkPermission(sDumpPermission, pid, uid)) {
        result.appendFormat("Permission Denial: "
                "can't dump InputFlinger from pid=%d, uid=%d\n", pid, uid);
    } else {
        dumpInternal(result);
    }
    write(fd, result.string(), result.size());
    return OK;
}

void InputFlinger::dumpInternal(String8& result) {
    result.append("INPUT FLINGER (dumpsys inputflinger)\n");
    mHost->dump(result);
}

}; // namespace android
