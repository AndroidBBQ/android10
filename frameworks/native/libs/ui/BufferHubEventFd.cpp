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

#include <sys/eventfd.h>

#include <log/log.h>
#include <ui/BufferHubEventFd.h>

namespace android {

BufferHubEventFd::BufferHubEventFd() : mFd(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)) {}

BufferHubEventFd::BufferHubEventFd(int fd) : mFd(fd) {}

status_t BufferHubEventFd::signal() const {
    if (!isValid()) {
        ALOGE("%s: cannot signal an invalid eventfd.", __FUNCTION__);
        return DEAD_OBJECT;
    }

    eventfd_write(mFd.get(), 1);
    return OK;
}

status_t BufferHubEventFd::clear() const {
    if (!isValid()) {
        ALOGE("%s: cannot clear an invalid eventfd.", __FUNCTION__);
        return DEAD_OBJECT;
    }

    eventfd_t value;
    eventfd_read(mFd.get(), &value);
    return OK;
}

} // namespace android
