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
#include <stdint.h>

#include <audio_utils/fifo.h>
#include <media/nblog/Entry.h>
#include <media/nblog/Events.h>
#include <utils/Log.h>

namespace android {
namespace NBLog {

int Entry::copyEntryDataAt(size_t offset) const
{
    // FIXME This is too slow
    if (offset == 0) {
        return mEvent;
    } else if (offset == 1) {
        return mLength;
    } else if (offset < (size_t) (mLength + 2)) {
        return (int) ((char *) mData)[offset - 2];
    } else if (offset == (size_t) (mLength + 2)) {
        return mLength;
    } else {
        return 0;   // FIXME is this an error?
    }
}

EntryIterator::EntryIterator()   // Dummy initialization.
    : mPtr(nullptr)
{
}

EntryIterator::EntryIterator(const uint8_t *entry)
    : mPtr(entry)
{
}

EntryIterator::EntryIterator(const EntryIterator &other)
    : mPtr(other.mPtr)
{
}

const entry& EntryIterator::operator*() const
{
    return *(entry*) mPtr;
}

const entry* EntryIterator::operator->() const
{
    return (entry*) mPtr;
}

EntryIterator& EntryIterator::operator++()
{
    mPtr += mPtr[offsetof(entry, length)] + Entry::kOverhead;
    return *this;
}

EntryIterator& EntryIterator::operator--()
{
    mPtr -= mPtr[Entry::kPreviousLengthOffset] + Entry::kOverhead;
    return *this;
}

EntryIterator EntryIterator::next() const
{
    EntryIterator aux(*this);
    return ++aux;
}

EntryIterator EntryIterator::prev() const
{
    EntryIterator aux(*this);
    return --aux;
}

bool EntryIterator::operator!=(const EntryIterator &other) const
{
    return mPtr != other.mPtr;
}

int EntryIterator::operator-(const EntryIterator &other) const
{
    return mPtr - other.mPtr;
}

bool EntryIterator::hasConsistentLength() const
{
    return mPtr[offsetof(entry, length)] == mPtr[mPtr[offsetof(entry, length)] +
        Entry::kOverhead + Entry::kPreviousLengthOffset];
}

void EntryIterator::copyTo(std::unique_ptr<audio_utils_fifo_writer> &dst) const
{
    size_t length = mPtr[offsetof(entry, length)] + Entry::kOverhead;
    dst->write(mPtr, length);
}

void EntryIterator::copyData(uint8_t *dst) const
{
    memcpy((void*) dst, mPtr + offsetof(entry, data), mPtr[offsetof(entry, length)]);
}

// ---------------------------------------------------------------------------

std::unique_ptr<AbstractEntry> AbstractEntry::buildEntry(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return nullptr;
    }
    const uint8_t type = EntryIterator(ptr)->type;
    switch (type) {
    case EVENT_FMT_START:
        return std::make_unique<FormatEntry>(FormatEntry(ptr));
    case EVENT_AUDIO_STATE:
    case EVENT_HISTOGRAM_ENTRY_TS:
        return std::make_unique<HistogramEntry>(HistogramEntry(ptr));
    default:
        ALOGW("Tried to create AbstractEntry of type %d", type);
        return nullptr;
    }
}

EntryIterator FormatEntry::begin() const
{
    return EntryIterator(mEntry);
}

const char *FormatEntry::formatString() const
{
    return (const char*) mEntry + offsetof(entry, data);
}

size_t FormatEntry::formatStringLength() const
{
    return mEntry[offsetof(entry, length)];
}

EntryIterator FormatEntry::args() const
{
    auto it = begin();
    ++it; // skip start fmt
    ++it; // skip timestamp
    ++it; // skip hash
    // Skip author if present
    if (it->type == EVENT_FMT_AUTHOR) {
        ++it;
    }
    return it;
}

int64_t FormatEntry::timestamp() const
{
    auto it = begin();
    ++it; // skip start fmt
    return it.payload<int64_t>();
}

log_hash_t FormatEntry::hash() const
{
    auto it = begin();
    ++it; // skip start fmt
    ++it; // skip timestamp
    // unaligned 64-bit read not supported
    log_hash_t hash;
    memcpy(&hash, it->data, sizeof(hash));
    return hash;
}

int FormatEntry::author() const
{
    auto it = begin();
    ++it; // skip start fmt
    ++it; // skip timestamp
    ++it; // skip hash
    // if there is an author entry, return it, return -1 otherwise
    return it->type == EVENT_FMT_AUTHOR ? it.payload<int>() : -1;
}

EntryIterator FormatEntry::copyWithAuthor(
        std::unique_ptr<audio_utils_fifo_writer> &dst, int author) const
{
    auto it = begin();
    it.copyTo(dst);     // copy fmt start entry
    (++it).copyTo(dst); // copy timestamp
    (++it).copyTo(dst); // copy hash
    // insert author entry
    size_t authorEntrySize = Entry::kOverhead + sizeof(author);
    uint8_t authorEntry[authorEntrySize];
    authorEntry[offsetof(entry, type)] = EVENT_FMT_AUTHOR;
    authorEntry[offsetof(entry, length)] =
        authorEntry[authorEntrySize + Entry::kPreviousLengthOffset] =
        sizeof(author);
    *(int*) (&authorEntry[offsetof(entry, data)]) = author;
    dst->write(authorEntry, authorEntrySize);
    // copy rest of entries
    while ((++it)->type != EVENT_FMT_END) {
        it.copyTo(dst);
    }
    it.copyTo(dst);
    ++it;
    return it;
}

int64_t HistogramEntry::timestamp() const
{
    return EntryIterator(mEntry).payload<HistTsEntry>().ts;
}

log_hash_t HistogramEntry::hash() const
{
    return EntryIterator(mEntry).payload<HistTsEntry>().hash;
}

int HistogramEntry::author() const
{
    EntryIterator it(mEntry);
    return it->length == sizeof(HistTsEntryWithAuthor)
            ? it.payload<HistTsEntryWithAuthor>().author : -1;
}

EntryIterator HistogramEntry::copyWithAuthor(
        std::unique_ptr<audio_utils_fifo_writer> &dst, int author) const
{
    // Current histogram entry has {type, length, struct HistTsEntry, length}.
    // We now want {type, length, struct HistTsEntryWithAuthor, length}
    uint8_t buffer[Entry::kOverhead + sizeof(HistTsEntryWithAuthor)];
    // Copy content until the point we want to add the author
    memcpy(buffer, mEntry, sizeof(entry) + sizeof(HistTsEntry));
    // Copy the author
    *(int*) (buffer + sizeof(entry) + sizeof(HistTsEntry)) = author;
    // Update lengths
    buffer[offsetof(entry, length)] = sizeof(HistTsEntryWithAuthor);
    buffer[offsetof(entry, data) + sizeof(HistTsEntryWithAuthor) + offsetof(ending, length)]
        = sizeof(HistTsEntryWithAuthor);
    // Write new buffer into FIFO
    dst->write(buffer, sizeof(buffer));
    return EntryIterator(mEntry).next();
}

}   // namespace NBLog
}   // namespace android
