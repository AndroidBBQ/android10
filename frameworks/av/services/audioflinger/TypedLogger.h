/*
 *
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_TYPED_LOGGER_H
#define ANDROID_TYPED_LOGGER_H

// This is the client API for the typed logger.

#include <media/nblog/NBLog.h>
#include <algorithm>

/*
Fowler-Noll-Vo (FNV-1a) hash function for the file name.
Hashes at compile time. FNV-1a iterative function:

hash = offset_basis
for each byte to be hashed
        hash = hash xor byte
        hash = hash * FNV_prime
return hash

offset_basis and FNV_prime values depend on the size of the hash output
Following values are defined by FNV and should not be changed arbitrarily
*/

template<typename T>
constexpr T offset_basis();

template<typename T>
constexpr T FNV_prime();

template<>
constexpr uint32_t offset_basis<uint32_t>() {
    return 2166136261u;
}

template<>
constexpr uint32_t FNV_prime<uint32_t>() {
    return 16777619u;
}

template<>
constexpr uint64_t offset_basis<uint64_t>() {
    return 14695981039346656037ull;
}

template<>
constexpr uint64_t FNV_prime<uint64_t>() {
    return 1099511628211ull;
}

template <typename T, size_t n>
__attribute__((no_sanitize("unsigned-integer-overflow")))
constexpr T fnv1a(const char (&file)[n], ssize_t i = (ssize_t)n - 1) {
    return i == -1 ? offset_basis<T>() : (fnv1a<T>(file, i - 1) ^ file[i]) * FNV_prime<T>();
}

template <size_t n>
constexpr uint64_t hash(const char (&file)[n], uint32_t line) {
    // Line numbers over or equal to 2^16 are clamped to 2^16 - 1. This way increases collisions
    // compared to wrapping around, but is easy to identify because it doesn't produce aliasing.
    // It's a very unlikely case anyways.
    return ((fnv1a<uint64_t>(file) << 16) ^ ((fnv1a<uint64_t>(file) >> 32) & 0xFFFF0000)) |
           std::min(line, 0xFFFFu);
}

// TODO Permit disabling of logging at compile-time.

// TODO A non-nullptr dummy implementation that is a nop would be faster than checking for nullptr
//      in the case when logging is enabled at compile-time and enabled at runtime, but it might be
//      slower than nullptr check when logging is enabled at compile-time and disabled at runtime.

// Write formatted entry to log
#define LOGT(fmt, ...) do { NBLog::Writer *x = tlNBLogWriter; if (x != nullptr) \
                                x->logFormat((fmt), hash(__FILE__, __LINE__), ##__VA_ARGS__); } \
                                while (0)

// Write histogram timestamp entry
#define LOG_HIST_TS() do { NBLog::Writer *x = tlNBLogWriter; if (x != nullptr) \
        x->logEventHistTs(NBLog::EVENT_HISTOGRAM_ENTRY_TS, hash(__FILE__, __LINE__)); } while(0)

// Record that audio was turned on/off
#define LOG_AUDIO_STATE() do { NBLog::Writer *x = tlNBLogWriter; if (x != nullptr) \
        x->logEventHistTs(NBLog::EVENT_AUDIO_STATE, hash(__FILE__, __LINE__)); } while(0)

// Log the difference bewteen frames presented by HAL and frames written to HAL output sink,
// divided by the sample rate. Parameter ms is of type double.
#define LOG_LATENCY(ms) do { NBLog::Writer *x = tlNBLogWriter; if (x != nullptr) \
        x->log<NBLog::EVENT_LATENCY>(ms); } while (0)

// Record thread overrun event nanosecond timestamp. Parameter ns is an int64_t.
#define LOG_OVERRUN(ns) do { NBLog::Writer *x = tlNBLogWriter; if (x != nullptr) \
        x->log<NBLog::EVENT_OVERRUN>(ns); } while (0)

// Record thread info. This currently includes type, frameCount, and sampleRate.
// Parameter type is thread_info_t as defined in NBLog.h.
#define LOG_THREAD_INFO(info) do { NBLog::Writer *x = tlNBLogWriter; \
        if (x != nullptr) x->log<NBLog::EVENT_THREAD_INFO>(info); } while (0)

#define LOG_THREAD_PARAMS(params) do {NBLog::Writer *x = tlNBLogWriter; \
        if (x != nullptr) x->log<NBLog::EVENT_THREAD_PARAMS>(params); } while (0)

// Record thread underrun event nanosecond timestamp. Parameter ns is an int64_t.
#define LOG_UNDERRUN(ns) do { NBLog::Writer *x = tlNBLogWriter; if (x != nullptr) \
        x->log<NBLog::EVENT_UNDERRUN>(ns); } while (0)

// Record thread warmup time in milliseconds. Parameter ms is of type double.
#define LOG_WARMUP_TIME(ms) do { NBLog::Writer *x = tlNBLogWriter; if (x != nullptr) \
        x->log<NBLog::EVENT_WARMUP_TIME>(ms); } while (0)

// Record a typed entry that represents a thread's work time in nanoseconds.
// Parameter ns should be of type uint32_t.
#define LOG_WORK_TIME(ns) do { NBLog::Writer *x = tlNBLogWriter; if (x != nullptr) \
        x->log<NBLog::EVENT_WORK_TIME>(ns); } while (0)

namespace android {
extern "C" {
// TODO consider adding a thread_local NBLog::Writer tlDummyNBLogWriter and then
// initialize below tlNBLogWriter to &tlDummyNBLogWriter to remove the need to
// check for nullptr every time. Also reduces the need to add a new logging macro above
// each time we want to log a new type.
extern thread_local NBLog::Writer *tlNBLogWriter;
}
} // namespace android

#endif // ANDROID_TYPED_LOGGER_H
