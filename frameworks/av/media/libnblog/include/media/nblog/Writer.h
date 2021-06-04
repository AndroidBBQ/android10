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

#ifndef ANDROID_MEDIA_NBLOG_WRITER_H
#define ANDROID_MEDIA_NBLOG_WRITER_H

#include <stdarg.h>
#include <stddef.h>

#include <binder/IMemory.h>
#include <media/nblog/Events.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

class audio_utils_fifo;
class audio_utils_fifo_writer;

namespace android {

class IMemory;

namespace NBLog {

class Entry;
struct Shared;

// NBLog Writer Interface

// Writer is thread-safe with respect to Reader, but not with respect to multiple threads
// calling Writer methods.  If you need multi-thread safety for writing, use LockedWriter.
class Writer : public RefBase {
public:
    Writer() = default;         // dummy nop implementation without shared memory

    // Input parameter 'size' is the desired size of the timeline in byte units.
    // The size of the shared memory must be at least Timeline::sharedSize(size).
    Writer(void *shared, size_t size);
    Writer(const sp<IMemory>& iMemory, size_t size);

    ~Writer() override;

    // FIXME needs comments, and some should be private
    void    log(const char *string);
    void    logf(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
    void    logTimestamp();
    void    logFormat(const char *fmt, log_hash_t hash, ...);
    void    logEventHistTs(Event event, log_hash_t hash);

    // Log data related to Event E. See the event-to-type mapping for the type of data
    // corresponding to the event. For example, if you see a mapping statement:
    //     MAP_TYPE_TO_EVENT(E, T);
    // then the usage of this method would be:
    //     T data = doComputation();
    //     tlNBLogWriter->log<NBLog::E>(data);
    template<Event E>
    void    log(typename get_mapped<E>::type data) {
        log(E, &data, sizeof(data));
    }

    virtual bool    isEnabled() const;

    // return value for all of these is the previous isEnabled()
    virtual bool    setEnabled(bool enabled);   // but won't enable if no shared memory
    bool            enable()  { return setEnabled(true); }
    bool            disable() { return setEnabled(false); }

    sp<IMemory>     getIMemory() const { return mIMemory; }

    // Public logging function implementations should always use one of the
    // two log() function calls below to write to shared memory.
protected:
    // Writes a single Entry to the FIFO if the writer is enabled.
    // This is protected and virtual because LockedWriter uses a lock to protect
    // writing to the FIFO before writing to this function.
    virtual void log(const Entry &entry, bool trusted = false);

private:
    // 0 <= length <= kMaxLength
    // Log a single Entry with corresponding event, data, and length.
    void    log(Event event, const void *data, size_t length);

    void    logvf(const char *fmt, va_list ap);

    // helper functions for logging parts of a formatted entry
    void    logStart(const char *fmt);
    void    logTimestampFormat();
    void    logVFormat(const char *fmt, log_hash_t hash, va_list ap);

    Shared* const   mShared{};          // raw pointer to shared memory
    sp<IMemory>     mIMemory{};         // ref-counted version, initialized in constructor
                                        // and then const
    audio_utils_fifo * const mFifo{};               // FIFO itself, non-NULL
                                                    // unless constructor fails
                                                    // or dummy constructor used
    audio_utils_fifo_writer * const mFifoWriter{};  // used to write to FIFO, non-NULL
                                                    // unless dummy constructor used
    bool            mEnabled = false;   // whether to actually log

    // cached pid and process name to use in %p format specifier
    // total tag length is mPidTagSize and process name is not zero terminated
    char   *mPidTag{};
    size_t  mPidTagSize = 0;
};

// ---------------------------------------------------------------------------

// Similar to Writer, but safe for multiple threads to call concurrently
class LockedWriter : public Writer {
public:
    LockedWriter() = default;
    LockedWriter(void *shared, size_t size);

    bool    isEnabled() const override;
    bool    setEnabled(bool enabled) override;

private:
    // Lock needs to be obtained before writing to FIFO.
    void log(const Entry &entry, bool trusted = false) override;

    mutable Mutex   mLock;
};

}   // namespace NBLog
}   // namespace android

#endif  // ANDROID_MEDIA_NBLOG_WRITER_H
