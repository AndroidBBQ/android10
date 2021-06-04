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

#ifndef C2BUFFER_BASE_H_
#define C2BUFFER_BASE_H_

/// \defgroup allocator Allocation and memory placement
/// @{

/**
 * Buffer/memory usage bits. These shall be used by the allocators to select optimal memory type/
 * pool and buffer layout. Usage bits are conceptually separated into read and write usage, while
 * the buffer use life-cycle is separated into producers (writers) and consumers (readers).
 * These two concepts are related but not equivalent: consumers may only read buffers and only
 * producers may write to buffers; note, however, that buffer producers may also want or need to
 * read the buffers.
 *
 * Read and write buffer usage bits shall be or-ed to arrive at the full buffer usage. Admittedly,
 * this does not account for the amount of reading and writing (e.g. a buffer may have one or more
 * readers); however, the proper information necessary to properly weigh the various usages would be
 * the amount of data read/written for each usage type. This would result in an integer array of
 * size 64 (or the number of distinct usages) for memory usage, and likely such detailed information
 * would not always be available.
 *
 * That platform-agnostic Codec 2.0 API only defines the bare minimum usages. Platforms shall define
 * usage bits that are appropriate for the platform.
 */
struct C2MemoryUsage {
// public:
    /**
     * Buffer read usage.
     */
    enum read_t : uint64_t {
        /** Buffer is read by the CPU. */
        CPU_READ        = 1 << 0,
        /**
         * Buffer shall only be read by trusted hardware. The definition of trusted hardware is
         * platform specific, but this flag is reserved to prevent mapping this block into CPU
         * readable memory resulting in bus fault. This flag can be used when buffer access must be
         * protected.
         */
        READ_PROTECTED  = 1 << 1,
    };

    /**
     * Buffer write usage.
     */
    enum write_t : uint64_t {
        /** Buffer is writted to by the CPU. */
        CPU_WRITE        = 1 << 2,
        /**
         * Buffer shall only be written to by trusted hardware. The definition of trusted hardware
         * is platform specific, but this flag is reserved to prevent mapping this block into CPU
         * writable memory resulting in bus fault. This flag can be used when buffer integrity must
         * be protected.
         */
        WRITE_PROTECTED  = 1 << 3,
    };

    enum : uint64_t {
        /**
         * Buffer usage bits reserved for the platform. We don't separately reserve read and
         * write usages as platforms may have asymmetric distribution between them.
         */
        PLATFORM_MASK     = ~(CPU_READ | CPU_WRITE | READ_PROTECTED | WRITE_PROTECTED),
    };

    /** Create a usage from separate consumer and producer usage mask. \deprecated */
    inline C2MemoryUsage(uint64_t consumer, uint64_t producer)
        : expected(consumer | producer) { }

    inline explicit C2MemoryUsage(uint64_t expected_)
        : expected(expected_) { }

    uint64_t expected; // expected buffer usage
};

/// @}

#endif  // C2BUFFER_BASE_H_

