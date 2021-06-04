/*
**
** Copyright 2009, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "GraphicBufferAllocator"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <ui/GraphicBufferAllocator.h>

#include <limits.h>
#include <stdio.h>

#include <grallocusage/GrallocUsageConversion.h>

#include <android-base/stringprintf.h>
#include <log/log.h>
#include <utils/Singleton.h>
#include <utils/Trace.h>

#include <ui/Gralloc.h>
#include <ui/Gralloc2.h>
#include <ui/Gralloc3.h>
#include <ui/GraphicBufferMapper.h>

namespace android {
// ---------------------------------------------------------------------------

using base::StringAppendF;

ANDROID_SINGLETON_STATIC_INSTANCE( GraphicBufferAllocator )

Mutex GraphicBufferAllocator::sLock;
KeyedVector<buffer_handle_t,
    GraphicBufferAllocator::alloc_rec_t> GraphicBufferAllocator::sAllocList;

GraphicBufferAllocator::GraphicBufferAllocator() : mMapper(GraphicBufferMapper::getInstance()) {
    mAllocator = std::make_unique<const Gralloc3Allocator>(
            reinterpret_cast<const Gralloc3Mapper&>(mMapper.getGrallocMapper()));
    if (!mAllocator->isLoaded()) {
        mAllocator = std::make_unique<const Gralloc2Allocator>(
                reinterpret_cast<const Gralloc2Mapper&>(mMapper.getGrallocMapper()));
    }

    if (!mAllocator->isLoaded()) {
        LOG_ALWAYS_FATAL("gralloc-allocator is missing");
    }
}

GraphicBufferAllocator::~GraphicBufferAllocator() {}

size_t GraphicBufferAllocator::getTotalSize() const {
    Mutex::Autolock _l(sLock);
    size_t total = 0;
    for (size_t i = 0; i < sAllocList.size(); ++i) {
        total += sAllocList.valueAt(i).size;
    }
    return total;
}

void GraphicBufferAllocator::dump(std::string& result) const {
    Mutex::Autolock _l(sLock);
    KeyedVector<buffer_handle_t, alloc_rec_t>& list(sAllocList);
    size_t total = 0;
    result.append("Allocated buffers:\n");
    const size_t c = list.size();
    for (size_t i=0 ; i<c ; i++) {
        const alloc_rec_t& rec(list.valueAt(i));
        if (rec.size) {
            StringAppendF(&result,
                          "%10p: %7.2f KiB | %4u (%4u) x %4u | %4u | %8X | 0x%" PRIx64 " | %s\n",
                          list.keyAt(i), rec.size / 1024.0, rec.width, rec.stride, rec.height,
                          rec.layerCount, rec.format, rec.usage, rec.requestorName.c_str());
        } else {
            StringAppendF(&result,
                          "%10p: unknown     | %4u (%4u) x %4u | %4u | %8X | 0x%" PRIx64 " | %s\n",
                          list.keyAt(i), rec.width, rec.stride, rec.height, rec.layerCount,
                          rec.format, rec.usage, rec.requestorName.c_str());
        }
        total += rec.size;
    }
    StringAppendF(&result, "Total allocated (estimate): %.2f KB\n", total / 1024.0);

    result.append(mAllocator->dumpDebugInfo());
}

void GraphicBufferAllocator::dumpToSystemLog()
{
    std::string s;
    GraphicBufferAllocator::getInstance().dump(s);
    ALOGD("%s", s.c_str());
}

status_t GraphicBufferAllocator::allocate(uint32_t width, uint32_t height,
        PixelFormat format, uint32_t layerCount, uint64_t usage,
        buffer_handle_t* handle, uint32_t* stride,
        uint64_t /*graphicBufferId*/, std::string requestorName)
{
    ATRACE_CALL();

    // make sure to not allocate a N x 0 or 0 x N buffer, since this is
    // allowed from an API stand-point allocate a 1x1 buffer instead.
    if (!width || !height)
        width = height = 1;

    const uint32_t bpp = bytesPerPixel(format);
    if (std::numeric_limits<size_t>::max() / width / height < static_cast<size_t>(bpp)) {
        ALOGE("Failed to allocate (%u x %u) layerCount %u format %d "
              "usage %" PRIx64 ": Requesting too large a buffer size",
              width, height, layerCount, format, usage);
        return BAD_VALUE;
    }

    // Ensure that layerCount is valid.
    if (layerCount < 1)
        layerCount = 1;

    // TODO(b/72323293, b/72703005): Remove these invalid bits from callers
    usage &= ~static_cast<uint64_t>((1 << 10) | (1 << 13));

    status_t error =
            mAllocator->allocate(width, height, format, layerCount, usage, 1, stride, handle);
    size_t bufSize;

    // if stride has no meaning or is too large,
    // approximate size with the input width instead
    if ((*stride) != 0 &&
        std::numeric_limits<size_t>::max() / height / (*stride) < static_cast<size_t>(bpp)) {
        bufSize = static_cast<size_t>(width) * height * bpp;
    } else {
        bufSize = static_cast<size_t>((*stride)) * height * bpp;
    }

    if (error == NO_ERROR) {
        Mutex::Autolock _l(sLock);
        KeyedVector<buffer_handle_t, alloc_rec_t>& list(sAllocList);
        alloc_rec_t rec;
        rec.width = width;
        rec.height = height;
        rec.stride = *stride;
        rec.format = format;
        rec.layerCount = layerCount;
        rec.usage = usage;
        rec.size = bufSize;
        rec.requestorName = std::move(requestorName);
        list.add(*handle, rec);

        return NO_ERROR;
    } else {
        ALOGE("Failed to allocate (%u x %u) layerCount %u format %d "
                "usage %" PRIx64 ": %d",
                width, height, layerCount, format, usage,
                error);
        return NO_MEMORY;
    }
}

status_t GraphicBufferAllocator::free(buffer_handle_t handle)
{
    ATRACE_CALL();

    // We allocated a buffer from the allocator and imported it into the
    // mapper to get the handle.  We just need to free the handle now.
    mMapper.freeBuffer(handle);

    Mutex::Autolock _l(sLock);
    KeyedVector<buffer_handle_t, alloc_rec_t>& list(sAllocList);
    list.removeItem(handle);

    return NO_ERROR;
}

// ---------------------------------------------------------------------------
}; // namespace android
