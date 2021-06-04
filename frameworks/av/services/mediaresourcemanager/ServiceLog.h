/*
**
** Copyright 2015, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_SERVICELOG_H
#define ANDROID_SERVICELOG_H

#include <utils/Errors.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <utils/Vector.h>

#include "media/RingBuffer.h"

namespace android {

class ServiceLog : public RefBase {
public:
    ServiceLog();
    explicit ServiceLog(size_t maxNum);

    void add(const String8 &log);
    String8 toString(const char *linePrefix = NULL) const;

private:
    size_t mMaxNum;
    mutable Mutex mLock;
    RingBuffer<String8> mLogs;

    void addLine(const char *log, const char *prefix, String8 *result) const;
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_SERVICELOG_H
