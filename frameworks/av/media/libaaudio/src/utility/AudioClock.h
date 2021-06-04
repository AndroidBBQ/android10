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

#ifndef UTILITY_AUDIO_CLOCK_H
#define UTILITY_AUDIO_CLOCK_H

#include <errno.h>
#include <stdint.h>
#include <time.h>

#include <aaudio/AAudio.h>

// Time conversion constants.
#define AAUDIO_NANOS_PER_MICROSECOND ((int64_t)1000)
#define AAUDIO_NANOS_PER_MILLISECOND (AAUDIO_NANOS_PER_MICROSECOND * 1000)
#define AAUDIO_MILLIS_PER_SECOND     1000
#define AAUDIO_NANOS_PER_SECOND      (AAUDIO_NANOS_PER_MILLISECOND * AAUDIO_MILLIS_PER_SECOND)

class AudioClock {
public:
    static int64_t getNanoseconds(clockid_t clockId = CLOCK_MONOTONIC) {
        struct timespec time;
        int result = clock_gettime(clockId, &time);
        if (result < 0) {
            return -errno;
        }
        return (time.tv_sec * AAUDIO_NANOS_PER_SECOND) + time.tv_nsec;
    }

    /**
     * Sleep until the specified absolute time.
     * Return immediately with AAUDIO_ERROR_ILLEGAL_ARGUMENT if a negative
     * nanoTime is specified.
     *
     * @param nanoTime time to wake up
     * @param clockId CLOCK_MONOTONIC is default
     * @return 0, a negative error, or 1 if the call is interrupted by a signal handler (EINTR)
     */
    static int sleepUntilNanoTime(int64_t nanoTime,
                                  clockid_t clockId = CLOCK_MONOTONIC) {
        if (nanoTime > 0) {
            struct timespec time;
            time.tv_sec = nanoTime / AAUDIO_NANOS_PER_SECOND;
            // Calculate the fractional nanoseconds. Avoids expensive % operation.
            time.tv_nsec = nanoTime - (time.tv_sec * AAUDIO_NANOS_PER_SECOND);
            int err = clock_nanosleep(clockId, TIMER_ABSTIME, &time, nullptr);
            switch (err) {
            case EINTR:
                return 1;
            case 0:
                return 0;
            default:
                // Subtract because clock_nanosleep() returns a positive error number!
                return 0 - err;
            }
        } else {
            return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
        }
    }

    /**
     * Sleep for the specified number of relative nanoseconds in real-time.
     * Return immediately with 0 if a negative nanoseconds is specified.
     *
     * @param nanoseconds time to sleep
     * @param clockId CLOCK_MONOTONIC is default
     * @return 0, a negative error, or 1 if the call is interrupted by a signal handler (EINTR)
     */
    static int sleepForNanos(int64_t nanoseconds, clockid_t clockId = CLOCK_MONOTONIC) {
        if (nanoseconds > 0) {
            struct timespec time;
            time.tv_sec = nanoseconds / AAUDIO_NANOS_PER_SECOND;
            // Calculate the fractional nanoseconds. Avoids expensive % operation.
            time.tv_nsec = nanoseconds - (time.tv_sec * AAUDIO_NANOS_PER_SECOND);
            const int flags = 0; // documented as relative sleep
            int err = clock_nanosleep(clockId, flags, &time, nullptr);
            switch (err) {
            case EINTR:
                return 1;
            case 0:
                return 0;
            default:
                // Subtract because clock_nanosleep() returns a positive error number!
                return 0 - err;
            }
        }
        return 0;
    }
};


#endif // UTILITY_AUDIO_CLOCK_H
