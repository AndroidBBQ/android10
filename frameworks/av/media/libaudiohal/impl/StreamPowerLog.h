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

#ifndef ANDROID_HARDWARE_STREAM_POWER_LOG_H
#define ANDROID_HARDWARE_STREAM_POWER_LOG_H

#include <audio_utils/clock.h>
#include <audio_utils/PowerLog.h>
#include <cutils/properties.h>
#include <system/audio.h>

namespace android {
namespace CPP_VERSION {

class StreamPowerLog {
public:
    StreamPowerLog() :
        mIsUserDebugOrEngBuild(is_userdebug_or_eng_build()),
        mPowerLog(nullptr),
        mFrameSize(0) {
        // use init() to set up the power log.
    }

    ~StreamPowerLog() {
        power_log_destroy(mPowerLog); // OK for null mPowerLog
        mPowerLog = nullptr;
    }

    // A one-time initialization (do not call twice) before using StreamPowerLog.
    void init(uint32_t sampleRate, audio_channel_mask_t channelMask, audio_format_t format) {
        if (mPowerLog == nullptr) {
            // Note: A way to get channel count for both input and output channel masks
            // but does not check validity of the channel mask.
            const uint32_t channelCount = popcount(audio_channel_mask_get_bits(channelMask));
            mFrameSize = channelCount * audio_bytes_per_sample(format);
            if (mFrameSize > 0) {
                const size_t kPowerLogFramesPerEntry =
                        (long long)sampleRate * kPowerLogSamplingIntervalMs / 1000;
                mPowerLog = power_log_create(
                        sampleRate,
                        channelCount,
                        format,
                        kPowerLogEntries,
                        kPowerLogFramesPerEntry);
            }
        }
        // mPowerLog may be NULL (not the right build, format not accepted, etc.).
    }

    // Dump the power log to fd.
    void dump(int fd) const {
        // OK for null mPowerLog
        (void)power_log_dump(
                mPowerLog, fd, "      " /* prefix */, kPowerLogLines, 0 /* limit_ns */);
    }

    // Log the audio data contained in buffer.
    void log(const void *buffer, size_t sizeInBytes) const {
        if (mPowerLog != nullptr) { // mFrameSize is always nonzero if mPowerLog exists.
            power_log_log(
                    mPowerLog, buffer, sizeInBytes / mFrameSize, audio_utils_get_real_time_ns());
        }
    }

    bool isUserDebugOrEngBuild() const {
        return mIsUserDebugOrEngBuild;
    }

private:

    static inline bool is_userdebug_or_eng_build() {
        char value[PROPERTY_VALUE_MAX];
        (void)property_get("ro.build.type", value, "unknown"); // ignore actual length
        return strcmp(value, "userdebug") == 0 || strcmp(value, "eng") == 0;
    }

    // Audio signal power log configuration.
    static const size_t kPowerLogLines = 40;
    static const size_t kPowerLogSamplingIntervalMs = 50;
    static const size_t kPowerLogEntries = (1 /* minutes */ * 60 /* seconds */ * 1000 /* msec */
            / kPowerLogSamplingIntervalMs);

    const bool mIsUserDebugOrEngBuild;
    power_log_t *mPowerLog;
    size_t mFrameSize;
};

} // namespace CPP_VERSION
} // namespace android

#endif // ANDROID_HARDWARE_STREAM_POWER_LOG_H
