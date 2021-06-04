/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef ANDROID_STARTBOOTANIMTHREAD_H
#define ANDROID_STARTBOOTANIMTHREAD_H

#include <stddef.h>

#include <utils/Mutex.h>
#include <utils/Thread.h>

namespace android {

class StartPropertySetThread : public Thread {
// Boot animation is triggered via calls to "property_set()" which can block
// if init's executing slow operation such as 'mount_all --late' (currently
// happening 1/10th with fsck)  concurrently. Running in a separate thread
// allows to pursue the SurfaceFlinger's init process without blocking.
// see b/34499826.
// Any property_set() will block during init stage so need to be offloaded
// to this thread. see b/63844978.
public:
    explicit StartPropertySetThread(bool timestampPropertyValue);
    status_t Start();
private:
    virtual bool threadLoop();
    static constexpr const char* kTimestampProperty = "service.sf.present_timestamp";
    const bool mTimestampPropertyValue;
};

}

#endif // ANDROID_STARTBOOTANIMTHREAD_H
