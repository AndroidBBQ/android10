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

#define LOG_TAG "NBLog"
//#define LOG_NDEBUG 0

#include <stdarg.h>
#include <stddef.h>
#include <sys/prctl.h>

#include <audio_utils/fifo.h>
#include <binder/IMemory.h>
#include <media/nblog/Entry.h>
#include <media/nblog/Events.h>
#include <media/nblog/Timeline.h>
#include <media/nblog/Writer.h>
#include <utils/Log.h>
#include <utils/Mutex.h>

namespace android {
namespace NBLog {

Writer::Writer(void *shared, size_t size)
    : mShared((Shared *) shared),
      mFifo(mShared != NULL ?
        new audio_utils_fifo(size, sizeof(uint8_t),
            mShared->mBuffer, mShared->mRear, NULL /*throttlesFront*/) : NULL),
      mFifoWriter(mFifo != NULL ? new audio_utils_fifo_writer(*mFifo) : NULL),
      mEnabled(mFifoWriter != NULL)
{
    // caching pid and process name
    pid_t id = ::getpid();
    char procName[16];
    int status = prctl(PR_GET_NAME, procName);
    if (status) {  // error getting process name
        procName[0] = '\0';
    }
    size_t length = strlen(procName);
    mPidTagSize = length + sizeof(pid_t);
    mPidTag = new char[mPidTagSize];
    memcpy(mPidTag, &id, sizeof(pid_t));
    memcpy(mPidTag + sizeof(pid_t), procName, length);
}

Writer::Writer(const sp<IMemory>& iMemory, size_t size)
    : Writer(iMemory != 0 ? (Shared *) iMemory->pointer() : NULL, size)
{
    mIMemory = iMemory;
}

Writer::~Writer()
{
    delete mFifoWriter;
    delete mFifo;
    delete[] mPidTag;
}

void Writer::log(const char *string)
{
    if (!mEnabled) {
        return;
    }
    LOG_ALWAYS_FATAL_IF(string == NULL, "Attempted to log NULL string");
    size_t length = strlen(string);
    if (length > Entry::kMaxLength) {
        length = Entry::kMaxLength;
    }
    log(EVENT_STRING, string, length);
}

void Writer::logf(const char *fmt, ...)
{
    if (!mEnabled) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    Writer::logvf(fmt, ap);     // the Writer:: is needed to avoid virtual dispatch for LockedWriter
    va_end(ap);
}

void Writer::logTimestamp()
{
    if (!mEnabled) {
        return;
    }
    struct timespec ts;
    if (!clock_gettime(CLOCK_MONOTONIC, &ts)) {
        log(EVENT_TIMESTAMP, &ts, sizeof(ts));
    }
}

void Writer::logFormat(const char *fmt, log_hash_t hash, ...)
{
    if (!mEnabled) {
        return;
    }
    va_list ap;
    va_start(ap, hash);
    Writer::logVFormat(fmt, hash, ap);
    va_end(ap);
}

void Writer::logEventHistTs(Event event, log_hash_t hash)
{
    if (!mEnabled) {
        return;
    }
    HistTsEntry data;
    data.hash = hash;
    data.ts = systemTime();
    if (data.ts > 0) {
        log(event, &data, sizeof(data));
    } else {
        ALOGE("Failed to get timestamp");
    }
}

bool Writer::isEnabled() const
{
    return mEnabled;
}

bool Writer::setEnabled(bool enabled)
{
    bool old = mEnabled;
    mEnabled = enabled && mShared != NULL;
    return old;
}

void Writer::log(const Entry &etr, bool trusted)
{
    if (!mEnabled) {
        return;
    }
    if (!trusted) {
        log(etr.mEvent, etr.mData, etr.mLength);
        return;
    }
    const size_t need = etr.mLength + Entry::kOverhead; // mEvent, mLength, data[mLength], mLength
                                                        // need = number of bytes written to FIFO

    // FIXME optimize this using memcpy for the data part of the Entry.
    // The Entry could have a method copyTo(ptr, offset, size) to optimize the copy.
    // checks size of a single log Entry: type, length, data pointer and ending
    uint8_t temp[Entry::kMaxLength + Entry::kOverhead];
    // write this data to temp array
    for (size_t i = 0; i < need; i++) {
        temp[i] = etr.copyEntryDataAt(i);
    }
    // write to circular buffer
    mFifoWriter->write(temp, need);
}

void Writer::log(Event event, const void *data, size_t length)
{
    if (!mEnabled) {
        return;
    }
    if (data == NULL || length > Entry::kMaxLength) {
        // TODO Perhaps it makes sense to display truncated data or at least a
        //      message that the data is too long?  The current behavior can create
        //      a confusion for a programmer debugging their code.
        return;
    }
    // Ignore if invalid event
    if (event == EVENT_RESERVED || event >= EVENT_UPPER_BOUND) {
        return;
    }
    Entry etr(event, data, length);
    log(etr, true /*trusted*/);
}

void Writer::logvf(const char *fmt, va_list ap)
{
    if (!mEnabled) {
        return;
    }
    char buffer[Entry::kMaxLength + 1 /*NUL*/];
    int length = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    if (length >= (int) sizeof(buffer)) {
        length = sizeof(buffer) - 1;
        // NUL termination is not required
        // buffer[length] = '\0';
    }
    if (length >= 0) {
        log(EVENT_STRING, buffer, length);
    }
}

void Writer::logStart(const char *fmt)
{
    if (!mEnabled) {
        return;
    }
    size_t length = strlen(fmt);
    if (length > Entry::kMaxLength) {
        length = Entry::kMaxLength;
    }
    log(EVENT_FMT_START, fmt, length);
}

void Writer::logTimestampFormat()
{
    if (!mEnabled) {
        return;
    }
    const nsecs_t ts = systemTime();
    if (ts > 0) {
        log(EVENT_FMT_TIMESTAMP, &ts, sizeof(ts));
    } else {
        ALOGE("Failed to get timestamp");
    }
}

void Writer::logVFormat(const char *fmt, log_hash_t hash, va_list argp)
{
    if (!mEnabled) {
        return;
    }
    Writer::logStart(fmt);
    int i;
    double d;
    float f;
    char* s;
    size_t length;
    int64_t t;
    Writer::logTimestampFormat();
    log(EVENT_FMT_HASH, &hash, sizeof(hash));
    for (const char *p = fmt; *p != '\0'; p++) {
        // TODO: implement more complex formatting such as %.3f
        if (*p != '%') {
            continue;
        }
        switch(*++p) {
        case 's': // string
            s = va_arg(argp, char *);
            length = strlen(s);
            if (length > Entry::kMaxLength) {
                length = Entry::kMaxLength;
            }
            log(EVENT_FMT_STRING, s, length);
            break;

        case 't': // timestamp
            t = va_arg(argp, int64_t);
            log(EVENT_FMT_TIMESTAMP, &t, sizeof(t));
            break;

        case 'd': // integer
            i = va_arg(argp, int);
            log(EVENT_FMT_INTEGER, &i, sizeof(i));
            break;

        case 'f': // float
            d = va_arg(argp, double); // float arguments are promoted to double in vararg lists
            f = (float)d;
            log(EVENT_FMT_FLOAT, &f, sizeof(f));
            break;

        case 'p': // pid
            log(EVENT_FMT_PID, mPidTag, mPidTagSize);
            break;

        // the "%\0" case finishes parsing
        case '\0':
            --p;
            break;

        case '%':
            break;

        default:
            ALOGW("NBLog Writer parsed invalid format specifier: %c", *p);
            break;
        }
    }
    Entry etr(EVENT_FMT_END, nullptr, 0);
    log(etr, true);
}

// ---------------------------------------------------------------------------

LockedWriter::LockedWriter(void *shared, size_t size)
    : Writer(shared, size)
{
}

bool LockedWriter::isEnabled() const
{
    Mutex::Autolock _l(mLock);
    return Writer::isEnabled();
}

bool LockedWriter::setEnabled(bool enabled)
{
    Mutex::Autolock _l(mLock);
    return Writer::setEnabled(enabled);
}

void LockedWriter::log(const Entry &entry, bool trusted) {
    Mutex::Autolock _l(mLock);
    Writer::log(entry, trusted);
}

}   // namespace NBLog
}   // namespace android
