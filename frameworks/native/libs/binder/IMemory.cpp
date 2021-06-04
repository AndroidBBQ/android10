/*
 * Copyright (C) 2008 The Android Open Source Project
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

#define LOG_TAG "IMemory"

#include <atomic>
#include <stdatomic.h>

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include <binder/IMemory.h>
#include <binder/Parcel.h>
#include <log/log.h>

#include <utils/KeyedVector.h>
#include <utils/threads.h>

#define VERBOSE   0

namespace android {
// ---------------------------------------------------------------------------

class HeapCache : public IBinder::DeathRecipient
{
public:
    HeapCache();
    virtual ~HeapCache();

    virtual void binderDied(const wp<IBinder>& who);

    sp<IMemoryHeap> find_heap(const sp<IBinder>& binder);
    void free_heap(const sp<IBinder>& binder);
    sp<IMemoryHeap> get_heap(const sp<IBinder>& binder);
    void dump_heaps();

private:
    // For IMemory.cpp
    struct heap_info_t {
        sp<IMemoryHeap> heap;
        int32_t         count;
        // Note that this cannot be meaningfully copied.
    };

    void free_heap(const wp<IBinder>& binder);

    Mutex mHeapCacheLock;  // Protects entire vector below.
    KeyedVector< wp<IBinder>, heap_info_t > mHeapCache;
    // We do not use the copy-on-write capabilities of KeyedVector.
    // TODO: Reimplemement based on standard C++ container?
};

static sp<HeapCache> gHeapCache = new HeapCache();

/******************************************************************************/

enum {
    HEAP_ID = IBinder::FIRST_CALL_TRANSACTION
};

class BpMemoryHeap : public BpInterface<IMemoryHeap>
{
public:
    explicit BpMemoryHeap(const sp<IBinder>& impl);
    virtual ~BpMemoryHeap();

    virtual int getHeapID() const;
    virtual void* getBase() const;
    virtual size_t getSize() const;
    virtual uint32_t getFlags() const;
    off_t getOffset() const override;

private:
    friend class IMemory;
    friend class HeapCache;

    // for debugging in this module
    static inline sp<IMemoryHeap> find_heap(const sp<IBinder>& binder) {
        return gHeapCache->find_heap(binder);
    }
    static inline void free_heap(const sp<IBinder>& binder) {
        gHeapCache->free_heap(binder);
    }
    static inline sp<IMemoryHeap> get_heap(const sp<IBinder>& binder) {
        return gHeapCache->get_heap(binder);
    }
    static inline void dump_heaps() {
        gHeapCache->dump_heaps();
    }

    void assertMapped() const;
    void assertReallyMapped() const;

    mutable std::atomic<int32_t> mHeapId;
    mutable void*       mBase;
    mutable size_t      mSize;
    mutable uint32_t    mFlags;
    mutable off_t       mOffset;
    mutable bool        mRealHeap;
    mutable Mutex       mLock;
};

// ----------------------------------------------------------------------------

enum {
    GET_MEMORY = IBinder::FIRST_CALL_TRANSACTION
};

class BpMemory : public BpInterface<IMemory>
{
public:
    explicit BpMemory(const sp<IBinder>& impl);
    virtual ~BpMemory();
    // NOLINTNEXTLINE(google-default-arguments)
    virtual sp<IMemoryHeap> getMemory(ssize_t* offset=nullptr, size_t* size=nullptr) const;

private:
    mutable sp<IMemoryHeap> mHeap;
    mutable ssize_t mOffset;
    mutable size_t mSize;
};

/******************************************************************************/

void* IMemory::fastPointer(const sp<IBinder>& binder, ssize_t offset) const
{
    sp<IMemoryHeap> realHeap = BpMemoryHeap::get_heap(binder);
    void* const base = realHeap->base();
    if (base == MAP_FAILED)
        return nullptr;
    return static_cast<char*>(base) + offset;
}

void* IMemory::pointer() const {
    ssize_t offset;
    sp<IMemoryHeap> heap = getMemory(&offset);
    void* const base = heap!=nullptr ? heap->base() : MAP_FAILED;
    if (base == MAP_FAILED)
        return nullptr;
    return static_cast<char*>(base) + offset;
}

size_t IMemory::size() const {
    size_t size;
    getMemory(nullptr, &size);
    return size;
}

ssize_t IMemory::offset() const {
    ssize_t offset;
    getMemory(&offset);
    return offset;
}

/******************************************************************************/

BpMemory::BpMemory(const sp<IBinder>& impl)
    : BpInterface<IMemory>(impl), mOffset(0), mSize(0)
{
}

BpMemory::~BpMemory()
{
}

// NOLINTNEXTLINE(google-default-arguments)
sp<IMemoryHeap> BpMemory::getMemory(ssize_t* offset, size_t* size) const
{
    if (mHeap == nullptr) {
        Parcel data, reply;
        data.writeInterfaceToken(IMemory::getInterfaceDescriptor());
        if (remote()->transact(GET_MEMORY, data, &reply) == NO_ERROR) {
            sp<IBinder> heap = reply.readStrongBinder();
            if (heap != nullptr) {
                mHeap = interface_cast<IMemoryHeap>(heap);
                if (mHeap != nullptr) {
                    const int64_t offset64 = reply.readInt64();
                    const uint64_t size64 = reply.readUint64();
                    const ssize_t o = (ssize_t)offset64;
                    const size_t s = (size_t)size64;
                    size_t heapSize = mHeap->getSize();
                    if (s == size64 && o == offset64 // ILP32 bounds check
                            && s <= heapSize
                            && o >= 0
                            && (static_cast<size_t>(o) <= heapSize - s)) {
                        mOffset = o;
                        mSize = s;
                    } else {
                        // Hm.
                        android_errorWriteWithInfoLog(0x534e4554,
                            "26877992", -1, nullptr, 0);
                        mOffset = 0;
                        mSize = 0;
                    }
                }
            }
        }
    }
    if (offset) *offset = mOffset;
    if (size) *size = mSize;
    return (mSize > 0) ? mHeap : nullptr;
}

// ---------------------------------------------------------------------------

IMPLEMENT_META_INTERFACE(Memory, "android.utils.IMemory");

BnMemory::BnMemory() {
}

BnMemory::~BnMemory() {
}

// NOLINTNEXTLINE(google-default-arguments)
status_t BnMemory::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case GET_MEMORY: {
            CHECK_INTERFACE(IMemory, data, reply);
            ssize_t offset;
            size_t size;
            reply->writeStrongBinder( IInterface::asBinder(getMemory(&offset, &size)) );
            reply->writeInt64(offset);
            reply->writeUint64(size);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}


/******************************************************************************/

BpMemoryHeap::BpMemoryHeap(const sp<IBinder>& impl)
    : BpInterface<IMemoryHeap>(impl),
        mHeapId(-1), mBase(MAP_FAILED), mSize(0), mFlags(0), mOffset(0), mRealHeap(false)
{
}

BpMemoryHeap::~BpMemoryHeap() {
    int32_t heapId = mHeapId.load(memory_order_relaxed);
    if (heapId != -1) {
        close(heapId);
        if (mRealHeap) {
            // by construction we're the last one
            if (mBase != MAP_FAILED) {
                sp<IBinder> binder = IInterface::asBinder(this);

                if (VERBOSE) {
                    ALOGD("UNMAPPING binder=%p, heap=%p, size=%zu, fd=%d",
                            binder.get(), this, mSize, heapId);
                }

                munmap(mBase, mSize);
            }
        } else {
            // remove from list only if it was mapped before
            sp<IBinder> binder = IInterface::asBinder(this);
            free_heap(binder);
        }
    }
}

void BpMemoryHeap::assertMapped() const
{
    int32_t heapId = mHeapId.load(memory_order_acquire);
    if (heapId == -1) {
        sp<IBinder> binder(IInterface::asBinder(const_cast<BpMemoryHeap*>(this)));
        sp<BpMemoryHeap> heap(static_cast<BpMemoryHeap*>(find_heap(binder).get()));
        heap->assertReallyMapped();
        if (heap->mBase != MAP_FAILED) {
            Mutex::Autolock _l(mLock);
            if (mHeapId.load(memory_order_relaxed) == -1) {
                mBase   = heap->mBase;
                mSize   = heap->mSize;
                mOffset = heap->mOffset;
                int fd = fcntl(heap->mHeapId.load(memory_order_relaxed), F_DUPFD_CLOEXEC, 0);
                ALOGE_IF(fd==-1, "cannot dup fd=%d",
                        heap->mHeapId.load(memory_order_relaxed));
                mHeapId.store(fd, memory_order_release);
            }
        } else {
            // something went wrong
            free_heap(binder);
        }
    }
}

void BpMemoryHeap::assertReallyMapped() const
{
    int32_t heapId = mHeapId.load(memory_order_acquire);
    if (heapId == -1) {

        // remote call without mLock held, worse case scenario, we end up
        // calling transact() from multiple threads, but that's not a problem,
        // only mmap below must be in the critical section.

        Parcel data, reply;
        data.writeInterfaceToken(IMemoryHeap::getInterfaceDescriptor());
        status_t err = remote()->transact(HEAP_ID, data, &reply);
        int parcel_fd = reply.readFileDescriptor();
        const uint64_t size64 = reply.readUint64();
        const int64_t offset64 = reply.readInt64();
        const uint32_t flags = reply.readUint32();
        const size_t size = (size_t)size64;
        const off_t offset = (off_t)offset64;
        if (err != NO_ERROR || // failed transaction
                size != size64 || offset != offset64) { // ILP32 size check
            ALOGE("binder=%p transaction failed fd=%d, size=%zu, err=%d (%s)",
                    IInterface::asBinder(this).get(),
                    parcel_fd, size, err, strerror(-err));
            return;
        }

        Mutex::Autolock _l(mLock);
        if (mHeapId.load(memory_order_relaxed) == -1) {
            int fd = fcntl(parcel_fd, F_DUPFD_CLOEXEC, 0);
            ALOGE_IF(fd == -1, "cannot dup fd=%d, size=%zu, err=%d (%s)",
                    parcel_fd, size, err, strerror(errno));

            int access = PROT_READ;
            if (!(flags & READ_ONLY)) {
                access |= PROT_WRITE;
            }
            mRealHeap = true;
            mBase = mmap(nullptr, size, access, MAP_SHARED, fd, offset);
            if (mBase == MAP_FAILED) {
                ALOGE("cannot map BpMemoryHeap (binder=%p), size=%zu, fd=%d (%s)",
                        IInterface::asBinder(this).get(), size, fd, strerror(errno));
                close(fd);
            } else {
                mSize = size;
                mFlags = flags;
                mOffset = offset;
                mHeapId.store(fd, memory_order_release);
            }
        }
    }
}

int BpMemoryHeap::getHeapID() const {
    assertMapped();
    // We either stored mHeapId ourselves, or loaded it with acquire semantics.
    return mHeapId.load(memory_order_relaxed);
}

void* BpMemoryHeap::getBase() const {
    assertMapped();
    return mBase;
}

size_t BpMemoryHeap::getSize() const {
    assertMapped();
    return mSize;
}

uint32_t BpMemoryHeap::getFlags() const {
    assertMapped();
    return mFlags;
}

off_t BpMemoryHeap::getOffset() const {
    assertMapped();
    return mOffset;
}

// ---------------------------------------------------------------------------

IMPLEMENT_META_INTERFACE(MemoryHeap, "android.utils.IMemoryHeap");

BnMemoryHeap::BnMemoryHeap() {
}

BnMemoryHeap::~BnMemoryHeap() {
}

// NOLINTNEXTLINE(google-default-arguments)
status_t BnMemoryHeap::onTransact(
        uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
       case HEAP_ID: {
            CHECK_INTERFACE(IMemoryHeap, data, reply);
            reply->writeFileDescriptor(getHeapID());
            reply->writeUint64(getSize());
            reply->writeInt64(getOffset());
            reply->writeUint32(getFlags());
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

/*****************************************************************************/

HeapCache::HeapCache()
    : DeathRecipient()
{
}

HeapCache::~HeapCache()
{
}

void HeapCache::binderDied(const wp<IBinder>& binder)
{
    //ALOGD("binderDied binder=%p", binder.unsafe_get());
    free_heap(binder);
}

sp<IMemoryHeap> HeapCache::find_heap(const sp<IBinder>& binder)
{
    Mutex::Autolock _l(mHeapCacheLock);
    ssize_t i = mHeapCache.indexOfKey(binder);
    if (i>=0) {
        heap_info_t& info = mHeapCache.editValueAt(i);
        ALOGD_IF(VERBOSE,
                "found binder=%p, heap=%p, size=%zu, fd=%d, count=%d",
                binder.get(), info.heap.get(),
                static_cast<BpMemoryHeap*>(info.heap.get())->mSize,
                static_cast<BpMemoryHeap*>(info.heap.get())
                    ->mHeapId.load(memory_order_relaxed),
                info.count);
        ++info.count;
        return info.heap;
    } else {
        heap_info_t info;
        info.heap = interface_cast<IMemoryHeap>(binder);
        info.count = 1;
        //ALOGD("adding binder=%p, heap=%p, count=%d",
        //      binder.get(), info.heap.get(), info.count);
        mHeapCache.add(binder, info);
        return info.heap;
    }
}

void HeapCache::free_heap(const sp<IBinder>& binder)  {
    free_heap( wp<IBinder>(binder) );
}

void HeapCache::free_heap(const wp<IBinder>& binder)
{
    sp<IMemoryHeap> rel;
    {
        Mutex::Autolock _l(mHeapCacheLock);
        ssize_t i = mHeapCache.indexOfKey(binder);
        if (i>=0) {
            heap_info_t& info(mHeapCache.editValueAt(i));
            if (--info.count == 0) {
                ALOGD_IF(VERBOSE,
                        "removing binder=%p, heap=%p, size=%zu, fd=%d, count=%d",
                        binder.unsafe_get(), info.heap.get(),
                        static_cast<BpMemoryHeap*>(info.heap.get())->mSize,
                        static_cast<BpMemoryHeap*>(info.heap.get())
                            ->mHeapId.load(memory_order_relaxed),
                        info.count);
                rel = mHeapCache.valueAt(i).heap;
                mHeapCache.removeItemsAt(i);
            }
        } else {
            ALOGE("free_heap binder=%p not found!!!", binder.unsafe_get());
        }
    }
}

sp<IMemoryHeap> HeapCache::get_heap(const sp<IBinder>& binder)
{
    sp<IMemoryHeap> realHeap;
    Mutex::Autolock _l(mHeapCacheLock);
    ssize_t i = mHeapCache.indexOfKey(binder);
    if (i>=0)   realHeap = mHeapCache.valueAt(i).heap;
    else        realHeap = interface_cast<IMemoryHeap>(binder);
    return realHeap;
}

void HeapCache::dump_heaps()
{
    Mutex::Autolock _l(mHeapCacheLock);
    int c = mHeapCache.size();
    for (int i=0 ; i<c ; i++) {
        const heap_info_t& info = mHeapCache.valueAt(i);
        BpMemoryHeap const* h(static_cast<BpMemoryHeap const *>(info.heap.get()));
        ALOGD("hey=%p, heap=%p, count=%d, (fd=%d, base=%p, size=%zu)",
                mHeapCache.keyAt(i).unsafe_get(),
                info.heap.get(), info.count,
                h->mHeapId.load(memory_order_relaxed), h->mBase, h->mSize);
    }
}


// ---------------------------------------------------------------------------
}; // namespace android
