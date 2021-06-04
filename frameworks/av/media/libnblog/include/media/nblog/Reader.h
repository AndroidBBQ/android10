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

#ifndef ANDROID_MEDIA_NBLOG_READER_H
#define ANDROID_MEDIA_NBLOG_READER_H

#include <memory>
#include <stddef.h>
#include <string>
#include <unordered_set>

#include <media/nblog/Entry.h>
#include <media/nblog/Events.h>
#include <utils/RefBase.h>

class audio_utils_fifo;
class audio_utils_fifo_reader;

namespace android {

class IMemory;
class String8;

namespace NBLog {

struct Shared;

// NBLog Reader API

class Snapshot;     // Forward declaration needed for Reader::getSnapshot()

class Reader : public RefBase {
public:
    // Input parameter 'size' is the desired size of the timeline in byte units.
    // The size of the shared memory must be at least Timeline::sharedSize(size).
    Reader(const void *shared, size_t size, const std::string &name);
    Reader(const sp<IMemory>& iMemory, size_t size, const std::string &name);
    ~Reader() override;

    // get snapshot of readers fifo buffer, effectively consuming the buffer
    std::unique_ptr<Snapshot> getSnapshot(bool flush = true);
    bool     isIMemory(const sp<IMemory>& iMemory) const;
    const std::string &name() const { return mName; }

private:
    // Amount of tries for reader to catch up with writer in getSnapshot().
    static constexpr int kMaxObtainTries = 3;

    // invalidBeginTypes and invalidEndTypes are used to align the Snapshot::begin() and
    // Snapshot::end() EntryIterators to valid entries.
    static const std::unordered_set<Event> invalidBeginTypes;
    static const std::unordered_set<Event> invalidEndTypes;

    // declared as const because audio_utils_fifo() constructor
    sp<IMemory> mIMemory;       // ref-counted version, assigned only in constructor

    const std::string mName;            // name of reader (actually name of writer)
    /*const*/ Shared* const mShared;    // raw pointer to shared memory, actually const but not
    audio_utils_fifo * const mFifo;                 // FIFO itself,
                                                    // non-NULL unless constructor fails
    audio_utils_fifo_reader * const mFifoReader;    // used to read from FIFO,
                                                    // non-NULL unless constructor fails

    // Searches for the last valid entry in the range [front, back)
    // back has to be entry-aligned. Returns nullptr if none enconuntered.
    static const uint8_t *findLastValidEntry(const uint8_t *front, const uint8_t *back,
                                               const std::unordered_set<Event> &invalidTypes);
};

// A snapshot of a readers buffer
// This is raw data. No analysis has been done on it
class Snapshot {
public:
    ~Snapshot() { delete[] mData; }

    // amount of data lost (given by audio_utils_fifo_reader)
    size_t lost() const { return mLost; }

    // iterator to beginning of readable segment of snapshot
    // data between begin and end has valid entries
    EntryIterator begin() const { return mBegin; }

    // iterator to end of readable segment of snapshot
    EntryIterator end() const { return mEnd; }

private:
    Snapshot() = default;
    explicit Snapshot(size_t bufferSize) : mData(new uint8_t[bufferSize]) {}
    friend std::unique_ptr<Snapshot> Reader::getSnapshot(bool flush);

    uint8_t * const       mData = nullptr;
    size_t                mLost = 0;
    EntryIterator         mBegin;
    EntryIterator         mEnd;
};

// TODO move this to MediaLogService?
class DumpReader : public NBLog::Reader {
public:
    DumpReader(const void *shared, size_t size, const std::string &name)
        : Reader(shared, size, name) {}
    DumpReader(const sp<IMemory>& iMemory, size_t size, const std::string &name)
        : Reader(iMemory, size, name) {}
    void dump(int fd, size_t indent = 0);
private:
    void handleAuthor(const AbstractEntry& fmtEntry __unused, String8* body __unused) {}
    EntryIterator handleFormat(const FormatEntry &fmtEntry, String8 *timestamp, String8 *body);

    static void    appendInt(String8 *body, const void *data);
    static void    appendFloat(String8 *body, const void *data);
    static void    appendPID(String8 *body, const void *data, size_t length);
    static void    appendTimestamp(String8 *body, const void *data);

    // The bufferDump functions are used for debugging only.
    static String8 bufferDump(const uint8_t *buffer, size_t size);
    static String8 bufferDump(const EntryIterator &it);
};

}   // namespace NBLog
}   // namespace android

#endif  // ANDROID_MEDIA_NBLOG_READER_H
