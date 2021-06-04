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

#define LOG_TAG "MediaUtils"
#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <cutils/properties.h>
#include <sys/resource.h>
#include <unistd.h>

#include <bionic_malloc.h>

#include "MediaUtils.h"

extern "C" void __scudo_set_rss_limit(size_t, int) __attribute__((weak));

namespace android {

void limitProcessMemory(const char *property, size_t numberOfBytes,
                        size_t percentageOfTotalMem) {
    long pageSize = sysconf(_SC_PAGESIZE);
    long numPages = sysconf(_SC_PHYS_PAGES);
    size_t maxMem = SIZE_MAX;

    if (pageSize > 0 && numPages > 0) {
        if (size_t(numPages) < SIZE_MAX / size_t(pageSize)) {
            maxMem = size_t(numPages) * size_t(pageSize);
        }
        ALOGV("physMem: %zu", maxMem);
        if (percentageOfTotalMem > 100) {
            ALOGW("requested %zu%% of total memory, using 100%%", percentageOfTotalMem);
            percentageOfTotalMem = 100;
        }
        maxMem = maxMem / 100 * percentageOfTotalMem;
        if (numberOfBytes < maxMem) {
            maxMem = numberOfBytes;
        }
        ALOGV("requested limit: %zu", maxMem);
    } else {
        ALOGW("couldn't determine total RAM");
    }

    int64_t propVal = property_get_int64(property, maxMem);
    if (propVal > 0 && uint64_t(propVal) <= SIZE_MAX) {
        maxMem = propVal;
    }

    // If Scudo is in use, enforce the hard RSS limit (in MB).
    if (maxMem != SIZE_MAX && &__scudo_set_rss_limit != 0) {
      __scudo_set_rss_limit(maxMem >> 20, 1);
      ALOGV("Scudo hard RSS limit set to %zu MB", maxMem >> 20);
      return;
    }

    if (!android_mallopt(M_SET_ALLOCATION_LIMIT_BYTES, &maxMem,
                         sizeof(maxMem))) {
      ALOGW("couldn't set allocation limit");
    }
}

} // namespace android
