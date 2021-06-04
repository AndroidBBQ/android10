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

#ifndef _ANDROID_SCHEDULING_POLICY_SERVICE_H
#define _ANDROID_SCHEDULING_POLICY_SERVICE_H

#include <utils/RefBase.h>

namespace android {

class IInterface;
// Request elevated priority for thread tid, whose thread group leader must be pid.
// The priority parameter is currently restricted to either 1 or 2.
// The asynchronous parameter should be 'true' to return immediately,
// after the request is enqueued but not necessarily executed.
// The default value 'false' means to return after request has been enqueued and executed.
int requestPriority(pid_t pid, pid_t tid, int32_t prio, bool isForApp, bool asynchronous = false);

// Request to move media.codec process between SP_FOREGROUND and SP_TOP_APP.
// When 'enable' is 'true', server will attempt to move media.codec process
// from SP_FOREGROUND into SP_TOP_APP cpuset. A valid 'client' must be provided
// for the server to receive death notifications. When 'enable' is 'false', server
// will attempt to move media.codec process back to the original cpuset, and
// 'client' is ignored in this case.
int requestCpusetBoost(bool enable, const sp<IInterface> &client);

}   // namespace android

#endif  // _ANDROID_SCHEDULING_POLICY_SERVICE_H
