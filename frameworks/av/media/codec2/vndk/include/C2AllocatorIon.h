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

#ifndef STAGEFRIGHT_CODEC2_ALLOCATOR_ION_H_
#define STAGEFRIGHT_CODEC2_ALLOCATOR_ION_H_

#include <functional>
#include <list>
#include <mutex>
#include <tuple>
#include <unordered_map>

#include <C2Buffer.h>

namespace android {

class C2AllocatorIon : public C2Allocator {
public:
    // Usage mapper function used by the allocator
    //   (usage, capacity) => (align, heapMask, flags)
    //
    // capacity is aligned to the default block-size (defaults to page size) to reduce caching
    // overhead
    typedef std::function<c2_status_t(C2MemoryUsage, size_t,
                      /* => */ size_t*, unsigned*, unsigned*)> UsageMapperFn;

    virtual id_t getId() const override;

    virtual C2String getName() const override;

    virtual std::shared_ptr<const Traits> getTraits() const override;

    virtual c2_status_t newLinearAllocation(
            uint32_t capacity, C2MemoryUsage usage,
            std::shared_ptr<C2LinearAllocation> *allocation) override;

    virtual c2_status_t priorLinearAllocation(
            const C2Handle *handle,
            std::shared_ptr<C2LinearAllocation> *allocation) override;

    C2AllocatorIon(id_t id);

    virtual c2_status_t status() const { return mInit; }

    virtual ~C2AllocatorIon() override;

    static bool isValid(const C2Handle* const o);

    /**
     * Updates the usage mapper for subsequent new allocations, as well as the supported
     * minimum and maximum usage masks and default block-size to use for the mapper.
     *
     * \param mapper this method is called to map Codec 2.0 buffer usage to ion flags
     *        required by the ion device
     * \param minUsage minimum buffer usage required for supported allocations (defaults to 0)
     * \param maxUsage maximum buffer usage supported by the ion allocator (defaults to SW_READ
     *        | SW_WRITE)
     * \param blockSize alignment used prior to calling |mapper| for the buffer capacity.
     *        This also helps reduce the size of cache required for caching mapper results.
     *        (defaults to the page size)
     */
    void setUsageMapper(
            const UsageMapperFn &mapper, uint64_t minUsage, uint64_t maxUsage, uint64_t blockSize);

private:
    c2_status_t mapUsage(C2MemoryUsage usage, size_t size,
                     /* => */ size_t *align, unsigned *heapMask, unsigned *flags);

    c2_status_t mInit;
    int mIonFd;

    // this locks mTraits, mBlockSize, mUsageMapper, mUsageMapperLru and mUsageMapperCache
    mutable std::mutex mUsageMapperLock;
    std::shared_ptr<const Traits> mTraits;
    size_t mBlockSize;
    UsageMapperFn mUsageMapper;
    typedef std::pair<uint64_t, size_t> MapperKey;
    struct MapperKeyHash {
        std::size_t operator()(const MapperKey &) const;
    };
    typedef std::tuple<size_t, unsigned, unsigned, c2_status_t> MapperValue;
    typedef std::pair<MapperKey, MapperValue> MapperKeyValue;
    typedef std::list<MapperKeyValue>::iterator MapperKeyValuePointer;
    std::list<MapperKeyValue> mUsageMapperLru;
    std::unordered_map<MapperKey, MapperKeyValuePointer, MapperKeyHash> mUsageMapperCache;
};

} // namespace android

#endif // STAGEFRIGHT_CODEC2_ALLOCATOR_ION_H_
