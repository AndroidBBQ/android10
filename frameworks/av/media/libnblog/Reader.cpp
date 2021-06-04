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

#include <memory>
#include <stddef.h>
#include <string>
#include <unordered_set>

#include <audio_utils/fifo.h>
#include <binder/IMemory.h>
#include <media/nblog/Entry.h>
#include <media/nblog/Events.h>
#include <media/nblog/Reader.h>
#include <media/nblog/Timeline.h>
#include <utils/Log.h>
#include <utils/String8.h>

namespace android {
namespace NBLog {

Reader::Reader(const void *shared, size_t size, const std::string &name)
    : mName(name),
      mShared((/*const*/ Shared *) shared), /*mIMemory*/
      mFifo(mShared != NULL ?
        new audio_utils_fifo(size, sizeof(uint8_t),
            mShared->mBuffer, mShared->mRear, NULL /*throttlesFront*/) : NULL),
      mFifoReader(mFifo != NULL ? new audio_utils_fifo_reader(*mFifo) : NULL)
{
}

Reader::Reader(const sp<IMemory>& iMemory, size_t size, const std::string &name)
    : Reader(iMemory != 0 ? (Shared *) iMemory->pointer() : NULL, size, name)
{
    mIMemory = iMemory;
}

Reader::~Reader()
{
    delete mFifoReader;
    delete mFifo;
}

// Copies content of a Reader FIFO into its Snapshot
// The Snapshot has the same raw data, but represented as a sequence of entries
// and an EntryIterator making it possible to process the data.
std::unique_ptr<Snapshot> Reader::getSnapshot(bool flush)
{
    if (mFifoReader == NULL) {
        return std::unique_ptr<Snapshot>(new Snapshot());
    }

    // This emulates the behaviour of audio_utils_fifo_reader::read, but without incrementing the
    // reader index. The index is incremented after handling corruption, to after the last complete
    // entry of the buffer
    size_t lost = 0;
    audio_utils_iovec iovec[2];
    const size_t capacity = mFifo->capacity();
    ssize_t availToRead;
    // A call to audio_utils_fifo_reader::obtain() places the read pointer one buffer length
    // before the writer's pointer (since mFifoReader was constructed with flush=false). The
    // do while loop is an attempt to read all of the FIFO's contents regardless of how behind
    // the reader is with respect to the writer. However, the following scheduling sequence is
    // possible and can lead to a starvation situation:
    // - Writer T1 writes, overrun with respect to Reader T2
    // - T2 calls obtain() and gets EOVERFLOW, T2 ptr placed one buffer size behind T1 ptr
    // - T1 write, overrun
    // - T2 obtain(), EOVERFLOW (and so on...)
    // To address this issue, we limit the number of tries for the reader to catch up with
    // the writer.
    int tries = 0;
    size_t lostTemp;
    do {
        availToRead = mFifoReader->obtain(iovec, capacity, NULL /*timeout*/, &lostTemp);
        lost += lostTemp;
    } while (availToRead < 0 || ++tries <= kMaxObtainTries);

    if (availToRead <= 0) {
        ALOGW_IF(availToRead < 0, "NBLog Reader %s failed to catch up with Writer", mName.c_str());
        return std::unique_ptr<Snapshot>(new Snapshot());
    }

    // Change to #if 1 for debugging. This statement is useful for checking buffer fullness levels
    // (as seen by reader) and how much data was lost. If you find that the fullness level is
    // getting close to full, or that data loss is happening to often, then you should
    // probably try some of the following:
    // - log less data
    // - log less often
    // - increase the initial shared memory allocation for the buffer
#if 0
    ALOGD("getSnapshot name=%s, availToRead=%zd, capacity=%zu, fullness=%.3f, lost=%zu",
            name().c_str(), availToRead, capacity, (double)availToRead / (double)capacity, lost);
#endif
    std::unique_ptr<Snapshot> snapshot(new Snapshot(availToRead));
    memcpy(snapshot->mData, (const char *) mFifo->buffer() + iovec[0].mOffset, iovec[0].mLength);
    if (iovec[1].mLength > 0) {
        memcpy(snapshot->mData + (iovec[0].mLength),
                (const char *) mFifo->buffer() + iovec[1].mOffset, iovec[1].mLength);
    }

    // Handle corrupted buffer
    // Potentially, a buffer has corrupted data on both beginning (due to overflow) and end
    // (due to incomplete format entry). But even if the end format entry is incomplete,
    // it ends in a complete entry (which is not an FMT_END). So is safe to traverse backwards.
    // TODO: handle client corruption (in the middle of a buffer)

    const uint8_t *back = snapshot->mData + availToRead;
    const uint8_t *front = snapshot->mData;

    // Find last FMT_END. <back> is sitting on an entry which might be the middle of a FormatEntry.
    // We go backwards until we find an EVENT_FMT_END.
    const uint8_t *lastEnd = findLastValidEntry(front, back, invalidEndTypes);
    if (lastEnd == nullptr) {
        snapshot->mEnd = snapshot->mBegin = EntryIterator(front);
    } else {
        // end of snapshot points to after last FMT_END entry
        snapshot->mEnd = EntryIterator(lastEnd).next();
        // find first FMT_START
        const uint8_t *firstStart = nullptr;
        const uint8_t *firstStartTmp = snapshot->mEnd;
        while ((firstStartTmp = findLastValidEntry(front, firstStartTmp, invalidBeginTypes))
                != nullptr) {
            firstStart = firstStartTmp;
        }
        // firstStart is null if no FMT_START entry was found before lastEnd
        if (firstStart == nullptr) {
            snapshot->mBegin = snapshot->mEnd;
        } else {
            snapshot->mBegin = EntryIterator(firstStart);
        }
    }

    // advance fifo reader index to after last entry read.
    if (flush) {
        mFifoReader->release(snapshot->mEnd - front);
    }

    snapshot->mLost = lost;
    return snapshot;
}

bool Reader::isIMemory(const sp<IMemory>& iMemory) const
{
    return iMemory != 0 && mIMemory != 0 && iMemory->pointer() == mIMemory->pointer();
}

// We make a set of the invalid types rather than the valid types when aligning
// Snapshot EntryIterators to valid entries during log corruption checking.
// This is done in order to avoid the maintenance overhead of adding a new Event
// type to the two sets below whenever a new Event type is created, as it is
// very likely that new types added will be valid types.
// Currently, invalidBeginTypes and invalidEndTypes are used to handle the special
// case of a Format Entry, which consists of a variable number of simple log entries.
// If a new Event is added that consists of a variable number of simple log entries,
// then these sets need to be updated.

// We want the beginning of a Snapshot to point to an entry that is not in
// the middle of a formatted entry and not an FMT_END.
const std::unordered_set<Event> Reader::invalidBeginTypes {
    EVENT_FMT_AUTHOR,
    EVENT_FMT_END,
    EVENT_FMT_FLOAT,
    EVENT_FMT_HASH,
    EVENT_FMT_INTEGER,
    EVENT_FMT_PID,
    EVENT_FMT_STRING,
    EVENT_FMT_TIMESTAMP,
};

// We want the end of a Snapshot to point to an entry that is not in
// the middle of a formatted entry and not a FMT_START.
const std::unordered_set<Event> Reader::invalidEndTypes {
    EVENT_FMT_AUTHOR,
    EVENT_FMT_FLOAT,
    EVENT_FMT_HASH,
    EVENT_FMT_INTEGER,
    EVENT_FMT_PID,
    EVENT_FMT_START,
    EVENT_FMT_STRING,
    EVENT_FMT_TIMESTAMP,
};

const uint8_t *Reader::findLastValidEntry(const uint8_t *front, const uint8_t *back,
                                          const std::unordered_set<Event> &invalidTypes) {
    if (front == nullptr || back == nullptr) {
        return nullptr;
    }
    while (back + Entry::kPreviousLengthOffset >= front) {
        const uint8_t *prev = back - back[Entry::kPreviousLengthOffset] - Entry::kOverhead;
        const Event type = (const Event)prev[offsetof(entry, type)];
        if (prev < front
                || prev + prev[offsetof(entry, length)] + Entry::kOverhead != back
                || type <= EVENT_RESERVED || type >= EVENT_UPPER_BOUND) {
            // prev points to an out of limits or inconsistent entry
            return nullptr;
        }
        // if invalidTypes does not contain the type, then the type is valid.
        if (invalidTypes.find(type) == invalidTypes.end()) {
            return prev;
        }
        back = prev;
    }
    return nullptr; // no entry found
}

// TODO for future compatibility, would prefer to have a dump() go to string, and then go
// to fd only when invoked through binder.
void DumpReader::dump(int fd, size_t indent)
{
    if (fd < 0) return;
    std::unique_ptr<Snapshot> snapshot = getSnapshot(false /*flush*/);
    if (snapshot == nullptr) {
        return;
    }
    String8 timestamp, body;

    // TODO all logged types should have a printable format.
    // TODO can we make the printing generic?
    for (EntryIterator it = snapshot->begin(); it != snapshot->end(); ++it) {
        switch (it->type) {
        case EVENT_FMT_START:
            it = handleFormat(FormatEntry(it), &timestamp, &body);
            break;
        case EVENT_LATENCY: {
            const double latencyMs = it.payload<double>();
            body.appendFormat("EVENT_LATENCY,%.3f", latencyMs);
        } break;
        case EVENT_OVERRUN: {
            const int64_t ts = it.payload<int64_t>();
            body.appendFormat("EVENT_OVERRUN,%lld", static_cast<long long>(ts));
        } break;
        case EVENT_THREAD_INFO: {
            const thread_info_t info = it.payload<thread_info_t>();
            body.appendFormat("EVENT_THREAD_INFO,%d,%s", static_cast<int>(info.id),
                    threadTypeToString(info.type));
        } break;
        case EVENT_UNDERRUN: {
            const int64_t ts = it.payload<int64_t>();
            body.appendFormat("EVENT_UNDERRUN,%lld", static_cast<long long>(ts));
        } break;
        case EVENT_WARMUP_TIME: {
            const double timeMs = it.payload<double>();
            body.appendFormat("EVENT_WARMUP_TIME,%.3f", timeMs);
        } break;
        case EVENT_WORK_TIME: {
            const int64_t monotonicNs = it.payload<int64_t>();
            body.appendFormat("EVENT_WORK_TIME,%lld", static_cast<long long>(monotonicNs));
        } break;
        case EVENT_THREAD_PARAMS: {
            const thread_params_t params = it.payload<thread_params_t>();
            body.appendFormat("EVENT_THREAD_PARAMS,%zu,%u", params.frameCount, params.sampleRate);
        } break;
        case EVENT_FMT_END:
        case EVENT_RESERVED:
        case EVENT_UPPER_BOUND:
            body.appendFormat("warning: unexpected event %d", it->type);
            break;
        default:
            break;
        }
        if (!body.isEmpty()) {
            dprintf(fd, "%.*s%s %s\n", (int)indent, "", timestamp.string(), body.string());
            body.clear();
        }
        timestamp.clear();
    }
}

EntryIterator DumpReader::handleFormat(const FormatEntry &fmtEntry,
        String8 *timestamp, String8 *body)
{
    String8 timestampLocal;
    String8 bodyLocal;
    if (timestamp == nullptr) {
        timestamp = &timestampLocal;
    }
    if (body == nullptr) {
        body = &bodyLocal;
    }

    // log timestamp
    const int64_t ts = fmtEntry.timestamp();
    timestamp->clear();
    timestamp->appendFormat("[%d.%03d]", (int) (ts / (1000 * 1000 * 1000)),
                    (int) ((ts / (1000 * 1000)) % 1000));

    // log unique hash
    log_hash_t hash = fmtEntry.hash();
    // print only lower 16bit of hash as hex and line as int to reduce spam in the log
    body->appendFormat("%.4X-%d ", (int)(hash >> 16) & 0xFFFF, (int) hash & 0xFFFF);

    // log author (if present)
    handleAuthor(fmtEntry, body);

    // log string
    EntryIterator arg = fmtEntry.args();

    const char* fmt = fmtEntry.formatString();
    size_t fmt_length = fmtEntry.formatStringLength();

    for (size_t fmt_offset = 0; fmt_offset < fmt_length; ++fmt_offset) {
        if (fmt[fmt_offset] != '%') {
            body->append(&fmt[fmt_offset], 1); // TODO optimize to write consecutive strings at once
            continue;
        }
        // case "%%""
        if (fmt[++fmt_offset] == '%') {
            body->append("%");
            continue;
        }
        // case "%\0"
        if (fmt_offset == fmt_length) {
            continue;
        }

        Event event = (Event) arg->type;
        size_t length = arg->length;

        // TODO check length for event type is correct

        if (event == EVENT_FMT_END) {
            break;
        }

        // TODO: implement more complex formatting such as %.3f
        const uint8_t *datum = arg->data; // pointer to the current event args
        switch(fmt[fmt_offset])
        {
        case 's': // string
            ALOGW_IF(event != EVENT_FMT_STRING,
                "NBLog Reader incompatible event for string specifier: %d", event);
            body->append((const char*) datum, length);
            break;

        case 't': // timestamp
            ALOGW_IF(event != EVENT_FMT_TIMESTAMP,
                "NBLog Reader incompatible event for timestamp specifier: %d", event);
            appendTimestamp(body, datum);
            break;

        case 'd': // integer
            ALOGW_IF(event != EVENT_FMT_INTEGER,
                "NBLog Reader incompatible event for integer specifier: %d", event);
            appendInt(body, datum);
            break;

        case 'f': // float
            ALOGW_IF(event != EVENT_FMT_FLOAT,
                "NBLog Reader incompatible event for float specifier: %d", event);
            appendFloat(body, datum);
            break;

        case 'p': // pid
            ALOGW_IF(event != EVENT_FMT_PID,
                "NBLog Reader incompatible event for pid specifier: %d", event);
            appendPID(body, datum, length);
            break;

        default:
            ALOGW("NBLog Reader encountered unknown character %c", fmt[fmt_offset]);
        }
        ++arg;
    }
    ALOGW_IF(arg->type != EVENT_FMT_END, "Expected end of format, got %d", arg->type);
    return arg;
}

void DumpReader::appendInt(String8 *body, const void *data)
{
    if (body == nullptr || data == nullptr) {
        return;
    }
    //int x = *((int*) data);
    int x;
    memcpy(&x, data, sizeof(x));
    body->appendFormat("<%d>", x);
}

void DumpReader::appendFloat(String8 *body, const void *data)
{
    if (body == nullptr || data == nullptr) {
        return;
    }
    float f;
    memcpy(&f, data, sizeof(f));
    body->appendFormat("<%f>", f);
}

void DumpReader::appendPID(String8 *body, const void* data, size_t length)
{
    if (body == nullptr || data == nullptr) {
        return;
    }
    pid_t id = *((pid_t*) data);
    char * name = &((char*) data)[sizeof(pid_t)];
    body->appendFormat("<PID: %d, name: %.*s>", id, (int) (length - sizeof(pid_t)), name);
}

void DumpReader::appendTimestamp(String8 *body, const void *data)
{
    if (body == nullptr || data == nullptr) {
        return;
    }
    int64_t ts;
    memcpy(&ts, data, sizeof(ts));
    body->appendFormat("[%d.%03d]", (int) (ts / (1000 * 1000 * 1000)),
                    (int) ((ts / (1000 * 1000)) % 1000));
}

String8 DumpReader::bufferDump(const uint8_t *buffer, size_t size)
{
    String8 str;
    if (buffer == nullptr) {
        return str;
    }
    str.append("[ ");
    for(size_t i = 0; i < size; i++) {
        str.appendFormat("%d ", buffer[i]);
    }
    str.append("]");
    return str;
}

String8 DumpReader::bufferDump(const EntryIterator &it)
{
    return bufferDump(it, it->length + Entry::kOverhead);
}

}   // namespace NBLog
}   // namespace android
