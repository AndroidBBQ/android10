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

#ifndef ANDROID_C2BUFFER_H_
#define ANDROID_C2BUFFER_H_

#include <cutils/native_handle.h>
#include <hardware/gralloc.h>

/* Use android native handle for C2Handle */
typedef ::native_handle_t C2Handle;

namespace android {

/**
 * Android platform buffer/memory usage bits.
 */
struct C2AndroidMemoryUsage : public C2MemoryUsage {
    inline C2AndroidMemoryUsage(const C2MemoryUsage &usage) : C2MemoryUsage(usage) { }

// public:
    /**
     * Reuse gralloc flags where possible, as Codec 2.0 API only uses bits 0 and 1.
     */
    enum consumer_t : uint64_t {
        RENDERSCRIPT_READ = GRALLOC_USAGE_RENDERSCRIPT,
        HW_TEXTURE_READ   = GRALLOC_USAGE_HW_TEXTURE,
        HW_COMPOSER_READ  = GRALLOC_USAGE_HW_COMPOSER,
        // gralloc does not define a video decoder read usage flag, so use encoder for
        // now
        HW_CODEC_READ     = GRALLOC_USAGE_HW_VIDEO_ENCODER,
        READ_PROTECTED    = GRALLOC_USAGE_PROTECTED,
    };

    enum producer_t : uint64_t {
        RENDERSCRIPT_WRITE = GRALLOC_USAGE_RENDERSCRIPT,
        HW_TEXTURE_WRITE   = GRALLOC_USAGE_HW_RENDER,
        HW_COMPOSER_WRITE  = GRALLOC_USAGE_HW_COMPOSER | GRALLOC_USAGE_HW_RENDER,
        HW_CODEC_WRITE     = GRALLOC_USAGE_HW_VIDEO_ENCODER,
        // gralloc does not define a write protected usage flag, so use read protected
        // now
        WRITE_PROTECTED    = GRALLOC_USAGE_PROTECTED,
    };

    /**
     * Convert from gralloc usage.
     */
    static C2MemoryUsage FromGrallocUsage(uint64_t usage);

    /**
     * Convert to gralloc usage.
     */
    uint64_t asGrallocUsage() const;
};

}  // namespace android

#endif  // ANDROID_C2BUFFER_H_
