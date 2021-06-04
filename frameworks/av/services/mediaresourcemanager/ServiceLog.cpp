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

//#define LOG_NDEBUG 0
#define LOG_TAG "ServiceLog"
#include <utils/Log.h>

#include <time.h>

#include "ServiceLog.h"

static const size_t kDefaultMaxNum = 100;

namespace android {

ServiceLog::ServiceLog() : mMaxNum(kDefaultMaxNum), mLogs(mMaxNum) {}
ServiceLog::ServiceLog(size_t maxNum) : mMaxNum(maxNum), mLogs(mMaxNum) {}

void ServiceLog::add(const String8 &log) {
    Mutex::Autolock lock(mLock);
    time_t now = time(0);
    char buf[64];
    strftime(buf, sizeof(buf), "%m-%d %T", localtime(&now));
    mLogs.add(String8::format("%s %s", buf, log.string()));
}

String8 ServiceLog::toString(const char *linePrefix) const {
    Mutex::Autolock lock(mLock);
    String8 result;
    for (const auto& log : mLogs) {
        addLine(log.string(), linePrefix, &result);
    }
    if (mLogs.size() == mMaxNum) {
        addLine("...", linePrefix, &result);
    } else if (mLogs.size() == 0) {
        addLine("[no events yet]", linePrefix, &result);
    }
    return result;
}

void ServiceLog::addLine(const char *log, const char *prefix, String8 *result) const {
    if (prefix != NULL) {
        result->append(prefix);
    }
    result->append(log);
    result->append("\n");
}

} // namespace android
