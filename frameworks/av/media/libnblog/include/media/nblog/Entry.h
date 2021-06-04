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

#ifndef ANDROID_MEDIA_NBLOG_ENTRY_H
#define ANDROID_MEDIA_NBLOG_ENTRY_H

#include <memory>
#include <stddef.h>
#include <stdint.h>
#include <type_traits>

#include <media/nblog/Events.h>

class audio_utils_fifo_writer;

namespace android {
namespace NBLog {

// entry representation in memory
struct entry {
    const uint8_t type;
    const uint8_t length;
    const uint8_t data[0];
};

// entry tail representation (after data)
struct ending {
    uint8_t length;
    uint8_t next[0];
};

// representation of a single log entry in shared memory
//  byte[0]             mEvent
//  byte[1]             mLength
//  byte[2]             mData[0]
//  ...
//  byte[2+i]           mData[i]
//  ...
//  byte[2+mLength-1]   mData[mLength-1]
//  byte[2+mLength]     duplicate copy of mLength to permit reverse scan
//  byte[3+mLength]     start of next log entry
class Entry {
public:
    Entry(Event event, const void *data, size_t length)
        : mEvent(event), mLength(length), mData(data) {}
    ~Entry() {}

    // used during writing to format Entry information as follows:
    // [type][length][data ... ][length]
    int     copyEntryDataAt(size_t offset) const;

private:
    friend class Writer;
    Event       mEvent;     // event type
    uint8_t     mLength;    // length of additional data, 0 <= mLength <= kMaxLength
    const void *mData;      // event type-specific data
    static const size_t kMaxLength = 255;
public:
    // mEvent, mLength, mData[...], duplicate mLength
    static const size_t kOverhead = sizeof(entry) + sizeof(ending);
    // endind length of previous entry
    static const ssize_t kPreviousLengthOffset = - sizeof(ending) +
        offsetof(ending, length);
};

// entry iterator
class EntryIterator {
public:
    // Used for dummy initialization. Performing operations on a default-constructed
    // EntryIterator other than assigning it to another valid EntryIterator
    // is undefined behavior.
    EntryIterator();
    // Caller's responsibility to make sure entry is not nullptr.
    // Passing in nullptr can result in undefined behavior.
    explicit EntryIterator(const uint8_t *entry);
    EntryIterator(const EntryIterator &other);

    // dereference underlying entry
    const entry&    operator*() const;
    const entry*    operator->() const;
    // advance to next entry
    EntryIterator&  operator++(); // ++i
    // back to previous entry
    EntryIterator&  operator--(); // --i
    // returns an EntryIterator corresponding to the next entry
    EntryIterator   next() const;
    // returns an EntryIterator corresponding to the previous entry
    EntryIterator   prev() const;
    bool            operator!=(const EntryIterator &other) const;
    int             operator-(const EntryIterator &other) const;

    bool            hasConsistentLength() const;
    void            copyTo(std::unique_ptr<audio_utils_fifo_writer> &dst) const;
    void            copyData(uint8_t *dst) const;

    // memcpy preferred to reinterpret_cast to avoid potentially unsupported
    // unaligned memory access.
#if 0
    template<typename T>
    inline const T& payload() {
        return *reinterpret_cast<const T *>(mPtr + offsetof(entry, data));
    }
#else
    template<typename T>
    inline T payload() const {
        static_assert(std::is_trivially_copyable<T>::value
                && !std::is_pointer<T>::value,
                "NBLog::EntryIterator payload must be trivially copyable, non-pointer type.");
        T payload;
        memcpy(&payload, mPtr + offsetof(entry, data), sizeof(payload));
        return payload;
    }
#endif

    inline operator const uint8_t*() const {
        return mPtr;
    }

private:
    const uint8_t  *mPtr;   // Should not be nullptr except for dummy initialization
};

// ---------------------------------------------------------------------------
// The following classes are used for merging into the Merger's buffer.

class AbstractEntry {
public:
    virtual ~AbstractEntry() {}

    // build concrete entry of appropriate class from ptr.
    static std::unique_ptr<AbstractEntry> buildEntry(const uint8_t *ptr);

    // get format entry timestamp
    virtual int64_t       timestamp() const = 0;

    // get format entry's unique id
    virtual log_hash_t    hash() const = 0;

    // entry's author index (-1 if none present)
    // a Merger has a vector of Readers, author simply points to the index of the
    // Reader that originated the entry
    // TODO consider changing to uint32_t
    virtual int           author() const = 0;

    // copy entry, adding author before timestamp, returns iterator to end of entry
    virtual EntryIterator copyWithAuthor(std::unique_ptr<audio_utils_fifo_writer> &dst,
                                            int author) const = 0;

protected:
    // Entry starting in the given pointer, which shall not be nullptr.
    explicit AbstractEntry(const uint8_t *entry) : mEntry(entry) {}
    // copies ordinary entry from src to dst, and returns length of entry
    // size_t      copyEntry(audio_utils_fifo_writer *dst, const iterator &it);
    const uint8_t * const mEntry;
};

// API for handling format entry operations

// a formatted entry has the following structure:
//    * FMT_START entry, containing the format string
//    * TIMESTAMP entry
//    * HASH entry
//    * author entry of the thread that generated it (optional, present in merged log)
//    * format arg1
//    * format arg2
//    * ...
//    * FMT_END entry
class FormatEntry : public AbstractEntry {
public:
    // explicit FormatEntry(const EntryIterator &it);
    explicit FormatEntry(const uint8_t *ptr) : AbstractEntry(ptr) {}
    ~FormatEntry() override = default;

    EntryIterator begin() const;

    // Entry's format string
    const char*   formatString() const;

    // Enrty's format string length
    size_t        formatStringLength() const;

    // Format arguments (excluding format string, timestamp and author)
    EntryIterator args() const;

    // get format entry timestamp
    int64_t       timestamp() const override;

    // get format entry's unique id
    log_hash_t    hash() const override;

    // entry's author index (-1 if none present)
    // a Merger has a vector of Readers, author simply points to the index of the
    // Reader that originated the entry
    int           author() const override;

    // copy entry, adding author before timestamp, returns size of original entry
    EntryIterator copyWithAuthor(std::unique_ptr<audio_utils_fifo_writer> &dst,
                                 int author) const override;
};

class HistogramEntry : public AbstractEntry {
public:
    explicit HistogramEntry(const uint8_t *ptr) : AbstractEntry(ptr) {}
    ~HistogramEntry() override = default;

    int64_t       timestamp() const override;

    log_hash_t    hash() const override;

    int           author() const override;

    EntryIterator copyWithAuthor(std::unique_ptr<audio_utils_fifo_writer> &dst,
                                 int author) const override;
};

}   // namespace NBLog
}   // namespace android

#endif  // ANDROID_MEDIA_NBLOG_ENTRY_H
