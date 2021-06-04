/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef AAUDIO_TIMESTAMP_SCHEDULER_H
#define AAUDIO_TIMESTAMP_SCHEDULER_H

#include <aaudio/AAudio.h>
#include <utility/AudioClock.h>

namespace aaudio {

/**
 * Schedule wakeup time for monitoring the position
 * of an MMAP/NOIRQ buffer.
 *
 * Note that this object is not thread safe. Only call it from a single thread.
 */
class TimestampScheduler
{
public:
    TimestampScheduler() {};
    virtual ~TimestampScheduler() = default;

    /**
     * Start the schedule at the given time.
     */
    void start(int64_t startTime);

    /**
     * Calculate the next time that the read position should be measured.
     */
    int64_t nextAbsoluteTime();

    void setBurstPeriod(int64_t burstPeriod) {
        mBurstPeriod = burstPeriod;
    }

    void setBurstPeriod(int32_t framesPerBurst,
                        int32_t sampleRate) {
        mBurstPeriod = AAUDIO_NANOS_PER_SECOND * framesPerBurst / sampleRate;
    }

    int64_t getBurstPeriod() {
        return mBurstPeriod;
    }

private:
    // Start with an arbitrary default so we do not divide by zero.
    int64_t mBurstPeriod = AAUDIO_NANOS_PER_MILLISECOND;
    int64_t mStartTime = 0;
    int64_t mLastTime = 0;
};

} /* namespace aaudio */

#endif /* AAUDIO_TIMESTAMP_SCHEDULER_H */
