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

#ifndef ANDROID_MEDIA_NBLOG_EVENTS_H
#define ANDROID_MEDIA_NBLOG_EVENTS_H

#include <stddef.h>
#include <stdint.h>
#include <system/audio.h>
#include <type_traits>

namespace android {
namespace NBLog {

// TODO have a comment somewhere explaining the whole process for adding a new EVENT_

// NBLog Event types. The Events are named to provide contextual meaning for what is logged.
// If adding a new standalone Event here, update the event-to-type mapping by adding a
// MAP_EVENT_TO_TYPE statement below.
// XXX Note that as of the current design, Events should not be renumbered (i.e. reordered)
// if they ever leave memory (for example, written to file, uploaded to cloud, etc.).
// TODO make some sort of interface to keep these "contract" constants.
enum Event : uint8_t {
    EVENT_RESERVED,
    EVENT_STRING,               // ASCII string, not NUL-terminated
                                // TODO: make timestamp optional
    EVENT_TIMESTAMP,            // clock_gettime(CLOCK_MONOTONIC)

    // Types for Format Entry, i.e. formatted entry
    EVENT_FMT_START,            // logFormat start event: entry includes format string,
                                // following entries contain format arguments
    // format arguments
    EVENT_FMT_AUTHOR,           // author index (present in merged logs) tracks entry's
                                // original log
    EVENT_FMT_FLOAT,            // floating point value entry
    EVENT_FMT_HASH,             // unique HASH of log origin, originates from hash of file name
                                // and line number
    EVENT_FMT_INTEGER,          // integer value entry
    EVENT_FMT_PID,              // process ID and process name
    EVENT_FMT_STRING,           // string value entry
    EVENT_FMT_TIMESTAMP,        // timestamp value entry
    // end of format arguments
    EVENT_FMT_END,              // end of logFormat argument list

    // Types for wakeup timestamp histograms
    EVENT_AUDIO_STATE,          // audio on/off event: logged on FastMixer::onStateChange call
    EVENT_HISTOGRAM_ENTRY_TS,   // single datum for timestamp histogram

    // Types representing audio performance metrics
    EVENT_LATENCY,              // difference between frames presented by HAL and frames
                                // written to HAL output sink, divided by sample rate.
    EVENT_OVERRUN,              // predicted thread overrun event timestamp
    EVENT_THREAD_INFO,          // see thread_info_t below
    EVENT_UNDERRUN,             // predicted thread underrun event timestamp
    EVENT_WARMUP_TIME,          // thread warmup time
    EVENT_WORK_TIME,            // the time a thread takes to do work, e.g. read, write, etc.
    EVENT_THREAD_PARAMS,        // see thread_params_t below

    EVENT_UPPER_BOUND,          // to check for invalid events
};

// NBLog custom-defined structs. Some NBLog Event types map to these structs.

using log_hash_t = uint64_t;

// used for EVENT_HISTOGRAM_ENTRY_TS (not mapped)
struct HistTsEntry {
    log_hash_t hash;
    int64_t ts;
}; //TODO __attribute__((packed));

// used for EVENT_HISTOGRAM_ENTRY_TS (not mapped)
struct HistTsEntryWithAuthor {
    log_hash_t hash;
    int64_t ts;
    int author;
}; //TODO __attribute__((packed));

enum ThreadType {
    UNKNOWN,
    MIXER,
    CAPTURE,
    FASTMIXER,
    FASTCAPTURE,
};

inline const char *threadTypeToString(ThreadType type) {
    switch (type) {
    case MIXER:
        return "MIXER";
    case CAPTURE:
        return "CAPTURE";
    case FASTMIXER:
        return "FASTMIXER";
    case FASTCAPTURE:
        return "FASTCAPTURE";
    case UNKNOWN:
    default:
        return "UNKNOWN";
    }
}

// mapped from EVENT_THREAD_INFO
// These fields always stay the same throughout a thread's lifetime and
// should only need to be logged once upon thread initialization.
// There is currently no recovery mechanism if the log event corresponding
// to this type is lost.
// TODO add this information when adding a reader to MediaLogService?
struct thread_info_t {
    audio_io_handle_t id = -1;      // Thread I/O handle
    ThreadType type = UNKNOWN;      // See enum ThreadType above
};

// mapped from EVENT_THREAD_PARAMS
// These fields are not necessarily constant throughout a thread's lifetime and
// can be logged whenever a thread receives new configurations or parameters.
struct thread_params_t {
    size_t frameCount = 0;          // number of frames per read or write buffer
    unsigned sampleRate = 0;        // in frames per second
};

template <Event E> struct get_mapped;
#define MAP_EVENT_TO_TYPE(E, T) \
template<> struct get_mapped<E> { \
    static_assert(std::is_trivially_copyable<T>::value \
            && !std::is_pointer<T>::value, \
            "NBLog::Event must map to trivially copyable, non-pointer type."); \
    typedef T type; \
}

// Maps an NBLog Event type to a C++ POD type.
MAP_EVENT_TO_TYPE(EVENT_LATENCY, double);
MAP_EVENT_TO_TYPE(EVENT_OVERRUN, int64_t);
MAP_EVENT_TO_TYPE(EVENT_THREAD_INFO, thread_info_t);
MAP_EVENT_TO_TYPE(EVENT_UNDERRUN, int64_t);
MAP_EVENT_TO_TYPE(EVENT_WARMUP_TIME, double);
MAP_EVENT_TO_TYPE(EVENT_WORK_TIME, int64_t);
MAP_EVENT_TO_TYPE(EVENT_THREAD_PARAMS, thread_params_t);

}   // namespace NBLog
}   // namespace android

#endif  // ANDROID_MEDIA_NBLOG_EVENTS_H
