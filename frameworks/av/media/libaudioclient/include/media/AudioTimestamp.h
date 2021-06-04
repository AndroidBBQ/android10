/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef ANDROID_AUDIO_TIMESTAMP_H
#define ANDROID_AUDIO_TIMESTAMP_H

#include <string>
#include <sstream>
#include <time.h>

namespace android {

class AudioTimestamp {
public:
    AudioTimestamp() : mPosition(0) {
        mTime.tv_sec = 0;
        mTime.tv_nsec = 0;
    }
    // FIXME change type to match android.media.AudioTrack
    uint32_t        mPosition; // a frame position in AudioTrack::getPosition() units
    struct timespec mTime;     // corresponding CLOCK_MONOTONIC when frame is expected to present
};

struct alignas(8) /* bug 29096183, bug 29108507 */ ExtendedTimestamp {
    enum Location {
        LOCATION_INVALID = -1,
        // Locations in the audio playback / record pipeline.
        LOCATION_CLIENT,   // timestamp of last read frame from client-server track buffer.
        LOCATION_SERVER,   // timestamp of newest frame from client-server track buffer.
        LOCATION_KERNEL,   // timestamp of newest frame in the kernel (alsa) buffer.

        // Historical data: info when the kernel timestamp was OK (prior to the newest frame).
        // This may be useful when the newest frame kernel timestamp is unavailable.
        // Available for playback timestamps.
        LOCATION_SERVER_LASTKERNELOK, // timestamp of server the prior time kernel timestamp OK.
        LOCATION_KERNEL_LASTKERNELOK, // timestamp of kernel the prior time kernel timestamp OK.
        LOCATION_MAX       // for sizing arrays only
    };

    // This needs to be kept in sync with android.media.AudioTimestamp
    enum Timebase {
        TIMEBASE_MONOTONIC,  // Clock monotonic offset (generally 0)
        TIMEBASE_BOOTTIME,
        TIMEBASE_MAX,
    };

    ExtendedTimestamp() {
        clear();
    }

    // mPosition is expressed in frame units.
    // It is generally nonnegative, though we keep this signed for
    // to potentially express algorithmic latency at the start of the stream
    // and to prevent unintentional unsigned integer underflow.
    int64_t mPosition[LOCATION_MAX];

    // mTimeNs is in nanoseconds for the default timebase, monotonic.
    // If this value is -1, then both time and position are invalid.
    // If this value is 0, then the time is not valid but the position is valid.
    int64_t mTimeNs[LOCATION_MAX];

    // mTimebaseOffset is the offset in ns from monotonic when the
    // timestamp was taken.  This may vary due to suspend time
    // or NTP adjustment.
    int64_t mTimebaseOffset[TIMEBASE_MAX];

    // Playback only:
    // mFlushed is number of flushed frames before entering the server mix;
    // hence not included in mPosition. This is used for adjusting server positions
    // information for frames "dropped".
    // FIXME: This variable should be eliminated, with the offset added on the server side
    // before sending to client, but differences in legacy position offset handling
    // and new extended timestamps require this to be maintained as a separate quantity.
    int64_t mFlushed;

    // Call to reset the timestamp to the original (invalid) state
    void clear() {
        memset(mPosition, 0, sizeof(mPosition)); // actually not necessary if time is -1
        for (int i = 0; i < LOCATION_MAX; ++i) {
            mTimeNs[i] = -1;
        }
        memset(mTimebaseOffset, 0, sizeof(mTimebaseOffset));
        mFlushed = 0;
    }

    // Returns the best timestamp as judged from the closest-to-hw stage in the
    // pipeline with a valid timestamp.  If the optional location parameter is non-null,
    // it will be filled with the location where the time was obtained.
    status_t getBestTimestamp(
            int64_t *position, int64_t *time, int timebase, Location *location = nullptr) const {
        if (position == nullptr || time == nullptr
                || timebase < 0 || timebase >= TIMEBASE_MAX) {
            return BAD_VALUE;
        }
        // look for the closest-to-hw stage in the pipeline with a valid timestamp.
        // We omit LOCATION_CLIENT as we prefer at least LOCATION_SERVER based accuracy
        // when getting the best timestamp.
        for (int i = LOCATION_KERNEL; i >= LOCATION_SERVER; --i) {
            if (mTimeNs[i] > 0) {
                *position = mPosition[i];
                *time = mTimeNs[i] + mTimebaseOffset[timebase];
                if (location != nullptr) {
                    *location = (Location)i;
                }
                return OK;
            }
        }
        return INVALID_OPERATION;
    }

    status_t getBestTimestamp(AudioTimestamp *timestamp, Location *location = nullptr) const {
        if (timestamp == nullptr) {
            return BAD_VALUE;
        }
        int64_t position, time;
        if (getBestTimestamp(&position, &time, TIMEBASE_MONOTONIC, location) == OK) {
            timestamp->mPosition = position;
            timestamp->mTime.tv_sec = time / 1000000000;
            timestamp->mTime.tv_nsec = time - timestamp->mTime.tv_sec * 1000000000LL;
            return OK;
        }
        return INVALID_OPERATION;
    }

    double getOutputServerLatencyMs(uint32_t sampleRate) const {
        return getLatencyMs(sampleRate, LOCATION_SERVER, LOCATION_KERNEL);
    }

    double getLatencyMs(uint32_t sampleRate, Location location1, Location location2) const {
        if (sampleRate > 0 && mTimeNs[location1] > 0 && mTimeNs[location2] > 0) {
            const int64_t frameDifference =
                    mPosition[location1] - mPosition[location2];
            const int64_t timeDifferenceNs =
                    mTimeNs[location1] - mTimeNs[location2];
            return ((double)frameDifference * 1e9 / sampleRate - timeDifferenceNs) * 1e-6;
        }
        return 0.;
    }

    // convert fields to a printable string
    std::string toString() const {
        std::stringstream ss;

        ss << "BOOTTIME offset " << mTimebaseOffset[TIMEBASE_BOOTTIME] << "\n";
        for (int i = 0; i < LOCATION_MAX; ++i) {
            ss << "ExtendedTimestamp[" << i << "]  position: "
                    << mPosition[i] << "  time: "  << mTimeNs[i] << "\n";
        }
        return ss.str();
    }
    // TODO:
    // Consider adding buffer status:
    // size, available, algorithmic latency
};

}   // namespace

#endif  // ANDROID_AUDIO_TIMESTAMP_H
