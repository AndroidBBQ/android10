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

#ifndef ANDROID_BUFFER_HUB_EVENT_FD_H_
#define ANDROID_BUFFER_HUB_EVENT_FD_H_

#include <android-base/unique_fd.h>
#include <utils/Errors.h>

namespace android {

class BufferHubEventFd {
public:
    /**
     * Constructs a valid event fd.
     */
    BufferHubEventFd();

    /**
     * Constructs from a valid event fd. Caller is responsible for the validity of the fd. Takes
     * ownership.
     */
    BufferHubEventFd(int fd);

    /**
     * Returns whether this BufferHubEventFd holds a valid event_fd.
     */
    bool isValid() const { return get() >= 0; }

    /**
     * Returns the fd number of the BufferHubEventFd object. Note that there is no ownership
     * transfer.
     */
    int get() const { return mFd.get(); }

    /**
     * Signals the eventfd.
     */
    status_t signal() const;

    /**
     * Clears the signal from this eventfd if it is signaled.
     */
    status_t clear() const;

private:
    base::unique_fd mFd;
};

} // namespace android

#endif // ANDROID_BUFFER_HUB_EVENT_FD_H_
