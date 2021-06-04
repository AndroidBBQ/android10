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


#include <utils/Log.h>
#include <media/TimeCheck.h>
#include <media/EventLog.h>

namespace android {

/* static */
sp<TimeCheck::TimeCheckThread> TimeCheck::getTimeCheckThread()
{
    static sp<TimeCheck::TimeCheckThread> sTimeCheckThread = new TimeCheck::TimeCheckThread();
    return sTimeCheckThread;
}

TimeCheck::TimeCheck(const char *tag, uint32_t timeoutMs)
    : mEndTimeNs(getTimeCheckThread()->startMonitoring(tag, timeoutMs))
{
}

TimeCheck::~TimeCheck() {
    getTimeCheckThread()->stopMonitoring(mEndTimeNs);
}

TimeCheck::TimeCheckThread::~TimeCheckThread()
{
    AutoMutex _l(mMutex);
    requestExit();
    mMonitorRequests.clear();
    mCond.signal();
}

nsecs_t TimeCheck::TimeCheckThread::startMonitoring(const char *tag, uint32_t timeoutMs) {
    Mutex::Autolock _l(mMutex);
    nsecs_t endTimeNs = systemTime() + milliseconds(timeoutMs);
    for (; mMonitorRequests.indexOfKey(endTimeNs) >= 0; ++endTimeNs);
    mMonitorRequests.add(endTimeNs, tag);
    mCond.signal();
    return endTimeNs;
}

void TimeCheck::TimeCheckThread::stopMonitoring(nsecs_t endTimeNs) {
    Mutex::Autolock _l(mMutex);
    mMonitorRequests.removeItem(endTimeNs);
    mCond.signal();
}

bool TimeCheck::TimeCheckThread::threadLoop()
{
    status_t status = TIMED_OUT;
    const char *tag;
    {
        AutoMutex _l(mMutex);

        if (exitPending()) {
            return false;
        }

        nsecs_t endTimeNs = INT64_MAX;
        // KeyedVector mMonitorRequests is ordered so take first entry as next timeout
        if (mMonitorRequests.size() != 0) {
            endTimeNs = mMonitorRequests.keyAt(0);
            tag = mMonitorRequests.valueAt(0);
        }

        const nsecs_t waitTimeNs = endTimeNs - systemTime();
        if (waitTimeNs > 0) {
            status = mCond.waitRelative(mMutex, waitTimeNs);
        }
        if (status != NO_ERROR) {
            LOG_EVENT_STRING(LOGTAG_AUDIO_BINDER_TIMEOUT, tag);
            LOG_ALWAYS_FATAL("TimeCheck timeout for %s", tag);
        }
    }
    return true;
}

}; // namespace android
