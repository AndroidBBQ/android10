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

#include "CameraThreadState.h"
#include <binder/IPCThreadState.h>
#include <hwbinder/IPCThreadState.h>
#include <unistd.h>

namespace android {

int CameraThreadState::getCallingUid() {
    if (hardware::IPCThreadState::self()->isServingCall()) {
        return hardware::IPCThreadState::self()->getCallingUid();
    }
    return IPCThreadState::self()->getCallingUid();
}

int CameraThreadState::getCallingPid() {
    if (hardware::IPCThreadState::self()->isServingCall()) {
        return hardware::IPCThreadState::self()->getCallingPid();
    }
    return IPCThreadState::self()->getCallingPid();
}

int64_t CameraThreadState::clearCallingIdentity() {
    if (hardware::IPCThreadState::self()->isServingCall()) {
        return hardware::IPCThreadState::self()->clearCallingIdentity();
    }
    return IPCThreadState::self()->clearCallingIdentity();
}

void CameraThreadState::restoreCallingIdentity(int64_t token) {
    if (hardware::IPCThreadState::self()->isServingCall()) {
        hardware::IPCThreadState::self()->restoreCallingIdentity(token);
    } else {
        IPCThreadState::self()->restoreCallingIdentity(token);
    }
    return;
}

} // android
