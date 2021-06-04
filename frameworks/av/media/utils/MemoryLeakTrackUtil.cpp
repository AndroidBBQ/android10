/*
 * Copyright 2011, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


//#define LOG_NDEBUG 0
#define LOG_TAG "MemoryLeackTrackUtil"
#include <utils/Log.h>

#include "media/MemoryLeakTrackUtil.h"
#include <sstream>

#include <bionic_malloc.h>

/*
 * The code here originally resided in MediaPlayerService.cpp
 */

// Figure out the abi based on defined macros.
#if defined(__arm__)
#define ABI_STRING "arm"
#elif defined(__aarch64__)
#define ABI_STRING "arm64"
#elif defined(__mips__) && !defined(__LP64__)
#define ABI_STRING "mips"
#elif defined(__mips__) && defined(__LP64__)
#define ABI_STRING "mips64"
#elif defined(__i386__)
#define ABI_STRING "x86"
#elif defined(__x86_64__)
#define ABI_STRING "x86_64"
#else
#error "Unsupported ABI"
#endif

extern std::string backtrace_string(const uintptr_t* frames, size_t frame_count);

namespace android {

std::string dumpMemoryAddresses(size_t limit)
{
    android_mallopt_leak_info_t leak_info;
    if (!android_mallopt(M_GET_MALLOC_LEAK_INFO, &leak_info, sizeof(leak_info))) {
      return "";
    }

    size_t count;
    if (leak_info.buffer == nullptr || leak_info.overall_size == 0 || leak_info.info_size == 0
            || (count = leak_info.overall_size / leak_info.info_size) == 0) {
        ALOGD("no malloc info, libc.debug.malloc.program property should be set");
        return "";
    }

    std::ostringstream oss;
    oss << leak_info.total_memory << " bytes in " << count << " allocations\n";
    oss << "  ABI: '" ABI_STRING "'" << "\n\n";
    if (count > limit) count = limit;

    // The memory is sorted based on total size which is useful for finding
    // worst memory offenders. For diffs, sometimes it is preferable to sort
    // based on the backtrace.
    for (size_t i = 0; i < count; i++) {
        struct AllocEntry {
            size_t size;  // bit 31 is set if this is zygote allocated memory
            size_t allocations;
            uintptr_t backtrace[];
        };

        const AllocEntry * const e = (AllocEntry *)(leak_info.buffer + i * leak_info.info_size);

        oss << (e->size * e->allocations)
                << " bytes ( " << e->size << " bytes * " << e->allocations << " allocations )\n";
        oss << backtrace_string(e->backtrace, leak_info.backtrace_size) << "\n";
    }
    oss << "\n";
    android_mallopt(M_FREE_MALLOC_LEAK_INFO, &leak_info, sizeof(leak_info));
    return oss.str();
}

}  // namespace android
