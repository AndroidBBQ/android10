/*
 * Copyright 2013 The Android Open Source Project
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

#include <stdint.h>
#include <utils/Errors.h>
#include <utils/Singleton.h>

#ifndef ANDROID_SF_EVENTLOG_H
#define ANDROID_SF_EVENTLOG_H

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

class String8;

class EventLog : public Singleton<EventLog> {

public:
    static void logFrameDurations(const String8& window,
            const int32_t* durations, size_t numDurations);

protected:
    EventLog();

private:
    /*
     * EventLogBuffer is a helper class to construct an in-memory event log
     * tag. In this version the buffer is not dynamic, so write operation can
     * fail if there is not enough space in the temporary buffer.
     * Once constructed, the buffer can be logger by calling the log()
     * method.
     */

    class TagBuffer {
        enum { STORAGE_MAX_SIZE = 128 };
        int32_t mPos;
        int32_t mTag;
        bool mOverflow;
        char mStorage[STORAGE_MAX_SIZE];
    public:
        explicit TagBuffer(int32_t tag);

        // starts list of items
        void startList(int8_t count);
        // terminates the list
        void endList();
        // write a 32-bit integer
        void writeInt32(int32_t value);
        // write a 64-bit integer
        void writeInt64(int64_t value);
        // write a C string
        void writeString8(const String8& value);

        // outputs the the buffer to the log
        void log();
    };

    friend class Singleton<EventLog>;
    EventLog(const EventLog&);
    EventLog& operator =(const EventLog&);

    enum { LOGTAG_SF_FRAME_DUR = 60100 };
    void doLogFrameDurations(const String8& window, const int32_t* durations,
            size_t numDurations);
};

// ---------------------------------------------------------------------------
}// namespace android
// ---------------------------------------------------------------------------

#endif /* ANDROID_SF_EVENTLOG_H */
