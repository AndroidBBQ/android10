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


#ifndef ANDROID_TIME_CHECK_H
#define ANDROID_TIME_CHECK_H

#include <utils/KeyedVector.h>
#include <utils/Thread.h>


namespace android {

// A class monitoring execution time for a code block (scoped variable) and causing an assert
// if it exceeds a certain time

class TimeCheck {
public:

    // The default timeout is chosen to be less than system server watchdog timeout
    static constexpr uint32_t kDefaultTimeOutMs = 5000;

            TimeCheck(const char *tag, uint32_t timeoutMs = kDefaultTimeOutMs);
            ~TimeCheck();

private:

    class TimeCheckThread : public Thread {
    public:

                            TimeCheckThread() {}
        virtual             ~TimeCheckThread() override;

                nsecs_t     startMonitoring(const char *tag, uint32_t timeoutMs);
                void        stopMonitoring(nsecs_t endTimeNs);

    private:

                // RefBase
        virtual void        onFirstRef() override { run("TimeCheckThread", PRIORITY_URGENT_AUDIO); }

                // Thread
        virtual bool        threadLoop() override;

                Condition           mCond;
                Mutex               mMutex;
                // using the end time in ns as key is OK given the risk is low that two entries
                // are added in such a way that <add time> + <timeout> are the same for both.
                KeyedVector< nsecs_t, const char*>  mMonitorRequests;
    };

    static sp<TimeCheckThread> getTimeCheckThread();

    const           nsecs_t mEndTimeNs;
};

}; // namespace android

#endif  // ANDROID_TIME_CHECK_H
