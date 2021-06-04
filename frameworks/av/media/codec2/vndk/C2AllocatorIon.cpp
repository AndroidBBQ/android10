/*
 * Copyright (C) 2016 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "C2AllocatorIon"
#include <utils/Log.h>

#include <list>

#include <ion/ion.h>
#include <sys/mman.h>
#include <unistd.h> // getpagesize, size_t, close, dup

#include <C2AllocatorIon.h>
#include <C2Buffer.h>
#include <C2Debug.h>
#include <C2ErrnoUtils.h>

namespace android {

namespace {
    constexpr size_t USAGE_LRU_CACHE_SIZE = 1024;
}

/* size_t <=> int(lo), int(hi) conversions */
constexpr inline int size2intLo(size_t s) {
    return int(s & 0xFFFFFFFF);
}

constexpr inline int size2intHi(size_t s) {
    // cast to uint64_t as size_t may be 32 bits wide
    return int((uint64_t(s) >> 32) & 0xFFFFFFFF);
}

constexpr inline size_t ints2size(int intLo, int intHi) {
    // convert in 2 stages to 64 bits as intHi may be negative
    return size_t(unsigned(intLo)) | size_t(uint64_t(unsigned(intHi)) << 32);
}

/* ========================================= ION HANDLE ======================================== */
/**
 * ION handle
 *
 * There can be only a sole ion client per process, this is captured in the ion fd that is passed
 * to the constructor, but this should be managed by the ion buffer allocator/mapper.
 *
 * ion uses ion_user_handle_t for buffers. We don't store this in the native handle as
 * it requires an ion_free to decref. Instead, we share the buffer to get an fd that also holds
 * a refcount.
 *
 * This handle will not capture mapped fd-s as updating that would require a global mutex.
 */

struct C2HandleIon : public C2Handle {
    // ion handle owns ionFd(!) and bufferFd
    C2HandleIon(int bufferFd, size_t size)
        : C2Handle(cHeader),
          mFds{ bufferFd },
          mInts{ int(size & 0xFFFFFFFF), int((uint64_t(size) >> 32) & 0xFFFFFFFF), kMagic } { }

    static bool isValid(const C2Handle * const o);

    int bufferFd() const { return mFds.mBuffer; }
    size_t size() const {
        return size_t(unsigned(mInts.mSizeLo))
                | size_t(uint64_t(unsigned(mInts.mSizeHi)) << 32);
    }

protected:
    struct {
        int mBuffer; // shared ion buffer
    } mFds;
    struct {
        int mSizeLo; // low 32-bits of size
        int mSizeHi; // high 32-bits of size
        int mMagic;
    } mInts;

private:
    typedef C2HandleIon _type;
    enum {
        kMagic = '\xc2io\x00',
        numFds = sizeof(mFds) / sizeof(int),
        numInts = sizeof(mInts) / sizeof(int),
        version = sizeof(C2Handle)
    };
    //constexpr static C2Handle cHeader = { version, numFds, numInts, {} };
    const static C2Handle cHeader;
};

const C2Handle C2HandleIon::cHeader = {
    C2HandleIon::version,
    C2HandleIon::numFds,
    C2HandleIon::numInts,
    {}
};

// static
bool C2HandleIon::isValid(const C2Handle * const o) {
    if (!o || memcmp(o, &cHeader, sizeof(cHeader))) {
        return false;
    }
    const C2HandleIon *other = static_cast<const C2HandleIon*>(o);
    return other->mInts.mMagic == kMagic;
}

// TODO: is the dup of an ion fd identical to ion_share?

/* ======================================= ION ALLOCATION ====================================== */
class C2AllocationIon : public C2LinearAllocation {
public:
    /* Interface methods */
    virtual c2_status_t map(
        size_t offset, size_t size, C2MemoryUsage usage, C2Fence *fence,
        void **addr /* nonnull */) override;
    virtual c2_status_t unmap(void *addr, size_t size, C2Fence *fenceFd) override;
    virtual ~C2AllocationIon() override;
    virtual const C2Handle *handle() const override;
    virtual id_t getAllocatorId() const override;
    virtual bool equals(const std::shared_ptr<C2LinearAllocation> &other) const override;

    // internal methods
    C2AllocationIon(int ionFd, size_t size, size_t align, unsigned heapMask, unsigned flags, C2Allocator::id_t id);
    C2AllocationIon(int ionFd, size_t size, int shareFd, C2Allocator::id_t id);

    c2_status_t status() const;

protected:
    class Impl;
    class ImplV2;
    Impl *mImpl;

    // TODO: we could make this encapsulate shared_ptr and copiable
    C2_DO_NOT_COPY(C2AllocationIon);
};

class C2AllocationIon::Impl {
protected:
    /**
     * Constructs an ion allocation.
     *
     * \note We always create an ion allocation, even if the allocation or import fails
     * so that we can capture the error.
     *
     * \param ionFd     ion client (ownership transferred to created object)
     * \param capacity  size of allocation
     * \param bufferFd  buffer handle (ownership transferred to created object). Must be
     *                  invalid if err is not 0.
     * \param buffer    ion buffer user handle (ownership transferred to created object). Must be
     *                  invalid if err is not 0.
     * \param err       errno during buffer allocation or import
     */
    Impl(int ionFd, size_t capacity, int bufferFd, ion_user_handle_t buffer, C2Allocator::id_t id, int err)
        : mIonFd(ionFd),
          mHandle(bufferFd, capacity),
          mBuffer(buffer),
          mId(id),
          mInit(c2_map_errno<ENOMEM, EACCES, EINVAL>(err)),
          mMapFd(-1) {
        if (mInit != C2_OK) {
            // close ionFd now on error
            if (mIonFd >= 0) {
                close(mIonFd);
                mIonFd = -1;
            }
            // C2_CHECK(bufferFd < 0);
            // C2_CHECK(buffer < 0);
        }
    }

public:
    /**
     * Constructs an ion allocation by importing a shared buffer fd.
     *
     * \param ionFd     ion client (ownership transferred to created object)
     * \param capacity  size of allocation
     * \param bufferFd  buffer handle (ownership transferred to created object)
     *
     * \return created ion allocation (implementation) which may be invalid if the
     * import failed.
     */
    static Impl *Import(int ionFd, size_t capacity, int bufferFd, C2Allocator::id_t id);

    /**
     * Constructs an ion allocation by allocating an ion buffer.
     *
     * \param ionFd     ion client (ownership transferred to created object)
     * \param size      size of allocation
     * \param align     desired alignment of allocation
     * \param heapMask  mask of heaps considered
     * \param flags     ion allocation flags
     *
     * \return created ion allocation (implementation) which may be invalid if the
     * allocation failed.
     */
    static Impl *Alloc(int ionFd, size_t size, size_t align, unsigned heapMask, unsigned flags, C2Allocator::id_t id);

    c2_status_t map(size_t offset, size_t size, C2MemoryUsage usage, C2Fence *fence, void **addr) {
        (void)fence; // TODO: wait for fence
        *addr = nullptr;
        if (!mMappings.empty()) {
            ALOGV("multiple map");
            // TODO: technically we should return DUPLICATE here, but our block views don't
            // actually unmap, so we end up remapping an ion buffer multiple times.
            //
            // return C2_DUPLICATE;
        }
        if (size == 0) {
            return C2_BAD_VALUE;
        }

        int prot = PROT_NONE;
        int flags = MAP_SHARED;
        if (usage.expected & C2MemoryUsage::CPU_READ) {
            prot |= PROT_READ;
        }
        if (usage.expected & C2MemoryUsage::CPU_WRITE) {
            prot |= PROT_WRITE;
        }

        size_t alignmentBytes = offset % PAGE_SIZE;
        size_t mapOffset = offset - alignmentBytes;
        size_t mapSize = size + alignmentBytes;
        Mapping map = { nullptr, alignmentBytes, mapSize };

        c2_status_t err = mapInternal(mapSize, mapOffset, alignmentBytes, prot, flags, &(map.addr), addr);
        if (map.addr) {
            mMappings.push_back(map);
        }
        return err;
    }

    c2_status_t unmap(void *addr, size_t size, C2Fence *fence) {
        if (mMappings.empty()) {
            ALOGD("tried to unmap unmapped buffer");
            return C2_NOT_FOUND;
        }
        for (auto it = mMappings.begin(); it != mMappings.end(); ++it) {
            if (addr != (uint8_t *)it->addr + it->alignmentBytes ||
                    size + it->alignmentBytes != it->size) {
                continue;
            }
            int err = munmap(it->addr, it->size);
            if (err != 0) {
                ALOGD("munmap failed");
                return c2_map_errno<EINVAL>(errno);
            }
            if (fence) {
                *fence = C2Fence(); // not using fences
            }
            (void)mMappings.erase(it);
            ALOGV("successfully unmapped: %d", mHandle.bufferFd());
            return C2_OK;
        }
        ALOGD("unmap failed to find specified map");
        return C2_BAD_VALUE;
    }

    virtual ~Impl() {
        if (!mMappings.empty()) {
            ALOGD("Dangling mappings!");
            for (const Mapping &map : mMappings) {
                (void)munmap(map.addr, map.size);
            }
        }
        if (mMapFd >= 0) {
            close(mMapFd);
            mMapFd = -1;
        }
        if (mInit == C2_OK) {
            if (mBuffer >= 0) {
                (void)ion_free(mIonFd, mBuffer);
            }
            native_handle_close(&mHandle);
        }
        if (mIonFd >= 0) {
            close(mIonFd);
        }
    }

    c2_status_t status() const {
        return mInit;
    }

    const C2Handle *handle() const {
        return &mHandle;
    }

    C2Allocator::id_t getAllocatorId() const {
        return mId;
    }

    virtual ion_user_handle_t ionHandle() const {
        return mBuffer;
    }

protected:
    virtual c2_status_t mapInternal(size_t mapSize, size_t mapOffset, size_t alignmentBytes,
            int prot, int flags, void** base, void** addr) {
        c2_status_t err = C2_OK;
        if (mMapFd == -1) {
            int ret = ion_map(mIonFd, mBuffer, mapSize, prot,
                              flags, mapOffset, (unsigned char**)base, &mMapFd);
            ALOGV("ion_map(ionFd = %d, handle = %d, size = %zu, prot = %d, flags = %d, "
                  "offset = %zu) returned (%d)",
                  mIonFd, mBuffer, mapSize, prot, flags, mapOffset, ret);
            if (ret) {
                mMapFd = -1;
                *base = *addr = nullptr;
                err = c2_map_errno<EINVAL>(-ret);
            } else {
                *addr = (uint8_t *)*base + alignmentBytes;
            }
        } else {
            *base = mmap(nullptr, mapSize, prot, flags, mMapFd, mapOffset);
            ALOGV("mmap(size = %zu, prot = %d, flags = %d, mapFd = %d, offset = %zu) "
                  "returned (%d)",
                  mapSize, prot, flags, mMapFd, mapOffset, errno);
            if (*base == MAP_FAILED) {
                *base = *addr = nullptr;
                err = c2_map_errno<EINVAL>(errno);
            } else {
                *addr = (uint8_t *)*base + alignmentBytes;
            }
        }
        return err;
    }

    int mIonFd;
    C2HandleIon mHandle;
    ion_user_handle_t mBuffer;
    C2Allocator::id_t mId;
    c2_status_t mInit;
    int mMapFd; // only one for now
    struct Mapping {
        void *addr;
        size_t alignmentBytes;
        size_t size;
    };
    std::list<Mapping> mMappings;
};

class C2AllocationIon::ImplV2 : public C2AllocationIon::Impl {
public:
    /**
     * Constructs an ion allocation for platforms with new (ion_4.12.h) api
     *
     * \note We always create an ion allocation, even if the allocation or import fails
     * so that we can capture the error.
     *
     * \param ionFd     ion client (ownership transferred to created object)
     * \param capacity  size of allocation
     * \param bufferFd  buffer handle (ownership transferred to created object). Must be
     *                  invalid if err is not 0.
     * \param err       errno during buffer allocation or import
     */
    ImplV2(int ionFd, size_t capacity, int bufferFd, C2Allocator::id_t id, int err)
        : Impl(ionFd, capacity, bufferFd, -1 /*buffer*/, id, err) {
    }

    virtual ~ImplV2() = default;

    virtual ion_user_handle_t ionHandle() const {
        return mHandle.bufferFd();
    }

protected:
    virtual c2_status_t mapInternal(size_t mapSize, size_t mapOffset, size_t alignmentBytes,
            int prot, int flags, void** base, void** addr) {
        c2_status_t err = C2_OK;
        *base = mmap(nullptr, mapSize, prot, flags, mHandle.bufferFd(), mapOffset);
        ALOGV("mmapV2(size = %zu, prot = %d, flags = %d, mapFd = %d, offset = %zu) "
              "returned (%d)",
              mapSize, prot, flags, mHandle.bufferFd(), mapOffset, errno);
        if (*base == MAP_FAILED) {
            *base = *addr = nullptr;
            err = c2_map_errno<EINVAL>(errno);
        } else {
            *addr = (uint8_t *)*base + alignmentBytes;
        }
        return err;
    }

};

C2AllocationIon::Impl *C2AllocationIon::Impl::Import(int ionFd, size_t capacity, int bufferFd,
        C2Allocator::id_t id) {
    int ret = 0;
    if (ion_is_legacy(ionFd)) {
        ion_user_handle_t buffer = -1;
        ret = ion_import(ionFd, bufferFd, &buffer);
        return new Impl(ionFd, capacity, bufferFd, buffer, id, ret);
    } else {
        return new ImplV2(ionFd, capacity, bufferFd, id, ret);
    }
}

C2AllocationIon::Impl *C2AllocationIon::Impl::Alloc(int ionFd, size_t size, size_t align,
        unsigned heapMask, unsigned flags, C2Allocator::id_t id) {
    int bufferFd = -1;
    ion_user_handle_t buffer = -1;
    size_t alignedSize = align == 0 ? size : (size + align - 1) & ~(align - 1);
    int ret;

    if (ion_is_legacy(ionFd)) {
        ret = ion_alloc(ionFd, alignedSize, align, heapMask, flags, &buffer);
        ALOGV("ion_alloc(ionFd = %d, size = %zu, align = %zu, prot = %d, flags = %d) "
              "returned (%d) ; buffer = %d",
              ionFd, alignedSize, align, heapMask, flags, ret, buffer);
        if (ret == 0) {
            // get buffer fd for native handle constructor
            ret = ion_share(ionFd, buffer, &bufferFd);
            if (ret != 0) {
                ion_free(ionFd, buffer);
                buffer = -1;
            }
        }
        return new Impl(ionFd, alignedSize, bufferFd, buffer, id, ret);

    } else {
        ret = ion_alloc_fd(ionFd, alignedSize, align, heapMask, flags, &bufferFd);
        ALOGV("ion_alloc_fd(ionFd = %d, size = %zu, align = %zu, prot = %d, flags = %d) "
              "returned (%d) ; bufferFd = %d",
              ionFd, alignedSize, align, heapMask, flags, ret, bufferFd);

        return new ImplV2(ionFd, alignedSize, bufferFd, id, ret);
    }
}

c2_status_t C2AllocationIon::map(
    size_t offset, size_t size, C2MemoryUsage usage, C2Fence *fence, void **addr) {
    return mImpl->map(offset, size, usage, fence, addr);
}

c2_status_t C2AllocationIon::unmap(void *addr, size_t size, C2Fence *fence) {
    return mImpl->unmap(addr, size, fence);
}

c2_status_t C2AllocationIon::status() const {
    return mImpl->status();
}

C2Allocator::id_t C2AllocationIon::getAllocatorId() const {
    return mImpl->getAllocatorId();
}

bool C2AllocationIon::equals(const std::shared_ptr<C2LinearAllocation> &other) const {
    if (!other || other->getAllocatorId() != getAllocatorId()) {
        return false;
    }
    // get user handle to compare objects
    std::shared_ptr<C2AllocationIon> otherAsIon = std::static_pointer_cast<C2AllocationIon>(other);
    return mImpl->ionHandle() == otherAsIon->mImpl->ionHandle();
}

const C2Handle *C2AllocationIon::handle() const {
    return mImpl->handle();
}

C2AllocationIon::~C2AllocationIon() {
    delete mImpl;
}

C2AllocationIon::C2AllocationIon(int ionFd, size_t size, size_t align,
                                 unsigned heapMask, unsigned flags, C2Allocator::id_t id)
    : C2LinearAllocation(size),
      mImpl(Impl::Alloc(ionFd, size, align, heapMask, flags, id)) { }

C2AllocationIon::C2AllocationIon(int ionFd, size_t size, int shareFd, C2Allocator::id_t id)
    : C2LinearAllocation(size),
      mImpl(Impl::Import(ionFd, size, shareFd, id)) { }

/* ======================================= ION ALLOCATOR ====================================== */
C2AllocatorIon::C2AllocatorIon(id_t id)
    : mInit(C2_OK),
      mIonFd(ion_open()) {
    if (mIonFd < 0) {
        switch (errno) {
        case ENOENT:    mInit = C2_OMITTED; break;
        default:        mInit = c2_map_errno<EACCES>(errno); break;
        }
    } else {
        C2MemoryUsage minUsage = { 0, 0 };
        C2MemoryUsage maxUsage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
        Traits traits = { "android.allocator.ion", id, LINEAR, minUsage, maxUsage };
        mTraits = std::make_shared<Traits>(traits);
        mBlockSize = ::getpagesize();
    }
}

C2AllocatorIon::~C2AllocatorIon() {
    if (mInit == C2_OK) {
        ion_close(mIonFd);
    }
}

C2Allocator::id_t C2AllocatorIon::getId() const {
    std::lock_guard<std::mutex> lock(mUsageMapperLock);
    return mTraits->id;
}

C2String C2AllocatorIon::getName() const {
    std::lock_guard<std::mutex> lock(mUsageMapperLock);
    return mTraits->name;
}

std::shared_ptr<const C2Allocator::Traits> C2AllocatorIon::getTraits() const {
    std::lock_guard<std::mutex> lock(mUsageMapperLock);
    return mTraits;
}

void C2AllocatorIon::setUsageMapper(
        const UsageMapperFn &mapper, uint64_t minUsage, uint64_t maxUsage, uint64_t blockSize) {
    std::lock_guard<std::mutex> lock(mUsageMapperLock);
    mUsageMapperCache.clear();
    mUsageMapperLru.clear();
    mUsageMapper = mapper;
    Traits traits = {
        mTraits->name, mTraits->id, LINEAR,
        C2MemoryUsage(minUsage), C2MemoryUsage(maxUsage)
    };
    mTraits = std::make_shared<Traits>(traits);
    mBlockSize = blockSize;
}

std::size_t C2AllocatorIon::MapperKeyHash::operator()(const MapperKey &k) const {
    return std::hash<uint64_t>{}(k.first) ^ std::hash<size_t>{}(k.second);
}

c2_status_t C2AllocatorIon::mapUsage(
        C2MemoryUsage usage, size_t capacity, size_t *align, unsigned *heapMask, unsigned *flags) {
    std::lock_guard<std::mutex> lock(mUsageMapperLock);
    c2_status_t res = C2_OK;
    // align capacity
    capacity = (capacity + mBlockSize - 1) & ~(mBlockSize - 1);
    MapperKey key = std::make_pair(usage.expected, capacity);
    auto entry = mUsageMapperCache.find(key);
    if (entry == mUsageMapperCache.end()) {
        if (mUsageMapper) {
            res = mUsageMapper(usage, capacity, align, heapMask, flags);
        } else {
            *align = 0; // TODO make this 1
            *heapMask = ~0; // default mask
            if (usage.expected & (C2MemoryUsage::CPU_READ | C2MemoryUsage::CPU_WRITE)) {
                *flags = ION_FLAG_CACHED; // cache CPU accessed buffers
            } else {
                *flags = 0;  // default flags
            }
            res = C2_NO_INIT;
        }
        // add usage to cache
        MapperValue value = std::make_tuple(*align, *heapMask, *flags, res);
        mUsageMapperLru.emplace_front(key, value);
        mUsageMapperCache.emplace(std::make_pair(key, mUsageMapperLru.begin()));
        if (mUsageMapperCache.size() > USAGE_LRU_CACHE_SIZE) {
            // remove LRU entry
            MapperKey lruKey = mUsageMapperLru.front().first;
            mUsageMapperCache.erase(lruKey);
            mUsageMapperLru.pop_back();
        }
    } else {
        // move entry to MRU
        mUsageMapperLru.splice(mUsageMapperLru.begin(), mUsageMapperLru, entry->second);
        const MapperValue &value = entry->second->second;
        std::tie(*align, *heapMask, *flags, res) = value;
    }
    return res;
}

c2_status_t C2AllocatorIon::newLinearAllocation(
        uint32_t capacity, C2MemoryUsage usage, std::shared_ptr<C2LinearAllocation> *allocation) {
    if (allocation == nullptr) {
        return C2_BAD_VALUE;
    }

    allocation->reset();
    if (mInit != C2_OK) {
        return mInit;
    }

    size_t align = 0;
    unsigned heapMask = ~0;
    unsigned flags = 0;
    c2_status_t ret = mapUsage(usage, capacity, &align, &heapMask, &flags);
    if (ret && ret != C2_NO_INIT) {
        return ret;
    }

    std::shared_ptr<C2AllocationIon> alloc
        = std::make_shared<C2AllocationIon>(dup(mIonFd), capacity, align, heapMask, flags, mTraits->id);
    ret = alloc->status();
    if (ret == C2_OK) {
        *allocation = alloc;
    }
    return ret;
}

c2_status_t C2AllocatorIon::priorLinearAllocation(
        const C2Handle *handle, std::shared_ptr<C2LinearAllocation> *allocation) {
    *allocation = nullptr;
    if (mInit != C2_OK) {
        return mInit;
    }

    if (!C2HandleIon::isValid(handle)) {
        return C2_BAD_VALUE;
    }

    // TODO: get capacity and validate it
    const C2HandleIon *h = static_cast<const C2HandleIon*>(handle);
    std::shared_ptr<C2AllocationIon> alloc
        = std::make_shared<C2AllocationIon>(dup(mIonFd), h->size(), h->bufferFd(), mTraits->id);
    c2_status_t ret = alloc->status();
    if (ret == C2_OK) {
        *allocation = alloc;
        native_handle_delete(const_cast<native_handle_t*>(
                reinterpret_cast<const native_handle_t*>(handle)));
    }
    return ret;
}

bool C2AllocatorIon::isValid(const C2Handle* const o) {
    return C2HandleIon::isValid(o);
}

} // namespace android

