/*
 * Copyright 2019 The Android Open Source Project
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

#include <cutils/native_handle.h>
#include <ui/BufferQueueDefs.h>
#include <types.h>
#include <system/window.h>
#include <vndk/hardware_buffer.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V2_0 {
namespace utils {

// TODO: move this into ui/BufferQueueDefs.h so that we don't need
// to include headers from libgui.
enum {
    // The API number used to indicate the currently connected producer
    CURRENTLY_CONNECTED_API = -1,
    // The API number used to indicate that no producer is connected
    NO_CONNECTED_API        = 0,
};

// Status
// ======

bool b2h(status_t from, HStatus* to,
         bool* bufferNeedsReallocation, bool* releaseAllBuffers) {
    switch (from) {
    case OK:
        *to = HStatus::OK; break;
    case NO_MEMORY:
        *to = HStatus::NO_MEMORY; break;
    case NO_INIT:
        *to = HStatus::NO_INIT; break;
    case BAD_VALUE:
        *to = HStatus::BAD_VALUE; break;
    case DEAD_OBJECT:
        *to = HStatus::DEAD_OBJECT; break;
    case INVALID_OPERATION:
        *to = HStatus::INVALID_OPERATION; break;
    case TIMED_OUT:
        *to = HStatus::TIMED_OUT; break;
    case WOULD_BLOCK:
        *to = HStatus::WOULD_BLOCK; break;
    case UNKNOWN_ERROR:
        *to = HStatus::UNKNOWN_ERROR; break;
    default:
        status_t mask =
                (bufferNeedsReallocation ? BufferQueueDefs::BUFFER_NEEDS_REALLOCATION : 0)
                | (releaseAllBuffers ? BufferQueueDefs::RELEASE_ALL_BUFFERS : 0);
        if (from & ~mask) {
            *to = static_cast<HStatus>(from);
        } else {
            *to = HStatus::OK;
            if (bufferNeedsReallocation) {
                *bufferNeedsReallocation = from & BufferQueueDefs::BUFFER_NEEDS_REALLOCATION;
            }
            if (releaseAllBuffers) {
                *releaseAllBuffers = from & BufferQueueDefs::RELEASE_ALL_BUFFERS;
            }
        }
    }
    return true;
}

bool h2b(HStatus from, status_t* to) {
    switch (from) {
    case HStatus::OK:
        *to = OK; break;
    case HStatus::NO_MEMORY:
        *to = NO_MEMORY; break;
    case HStatus::NO_INIT:
        *to = NO_INIT; break;
    case HStatus::BAD_VALUE:
        *to = BAD_VALUE; break;
    case HStatus::DEAD_OBJECT:
        *to = DEAD_OBJECT; break;
    case HStatus::INVALID_OPERATION:
        *to = INVALID_OPERATION; break;
    case HStatus::TIMED_OUT:
        *to = TIMED_OUT; break;
    case HStatus::WOULD_BLOCK:
        *to = WOULD_BLOCK; break;
    case HStatus::UNKNOWN_ERROR:
        *to = UNKNOWN_ERROR; break;
    default:
        *to = static_cast<status_t>(from);
    }
    return true;
}

// Fence
// =====

HFenceWrapper::HFenceWrapper(native_handle_t* h) : mHandle{h} {
}

HFenceWrapper::~HFenceWrapper() {
    native_handle_delete(mHandle);
}

HFenceWrapper& HFenceWrapper::set(native_handle_t* h) {
    native_handle_delete(mHandle);
    mHandle = h;
    return *this;
}

HFenceWrapper& HFenceWrapper::operator=(native_handle_t* h) {
    return set(h);
}

hidl_handle HFenceWrapper::getHandle() const {
    return hidl_handle{mHandle};
}

HFenceWrapper::operator hidl_handle() const {
    return getHandle();
}

bool b2h(sp<BFence> const& from, HFenceWrapper* to) {
    if (!from) {
        to->set(nullptr);
        return true;
    }
    int fenceFd = from->get();
    if (fenceFd == -1) {
        to->set(nullptr);
        return true;
    }
    native_handle_t* nh = native_handle_create(1, 0);
    if (!nh) {
        return false;
    }
    nh->data[0] = fenceFd;
    to->set(nh);
    return true;
}

bool h2b(native_handle_t const* from, sp<BFence>* to) {
    if (!from || from->numFds == 0) {
        *to = new ::android::Fence();
        return true;
    }
    if (from->numFds != 1 || from->numInts != 0) {
        return false;
    }
    *to = new BFence(dup(from->data[0]));
    return true;
}

// ConnectionType
// ==============

bool b2h(int from, HConnectionType* to) {
    *to = static_cast<HConnectionType>(from);
    switch (from) {
    case CURRENTLY_CONNECTED_API:
        *to = HConnectionType::CURRENTLY_CONNECTED; break;
    case NATIVE_WINDOW_API_EGL:
        *to = HConnectionType::EGL; break;
    case NATIVE_WINDOW_API_CPU:
        *to = HConnectionType::CPU; break;
    case NATIVE_WINDOW_API_MEDIA:
        *to = HConnectionType::MEDIA; break;
    case NATIVE_WINDOW_API_CAMERA:
        *to = HConnectionType::CAMERA; break;
    }
    return true;
}

bool h2b(HConnectionType from, int* to) {
    *to = static_cast<int>(from);
    switch (from) {
    case HConnectionType::CURRENTLY_CONNECTED:
        *to = CURRENTLY_CONNECTED_API; break;
    case HConnectionType::EGL:
        *to = NATIVE_WINDOW_API_EGL; break;
    case HConnectionType::CPU:
        *to = NATIVE_WINDOW_API_CPU; break;
    case HConnectionType::MEDIA:
        *to = NATIVE_WINDOW_API_MEDIA; break;
    case HConnectionType::CAMERA:
        *to = NATIVE_WINDOW_API_CAMERA; break;
    }
    return true;
}

// Rect
// ====

bool b2h(BRect const& from, HRect* to) {
    BRect* dst = reinterpret_cast<BRect*>(to->data());
    dst->left = from.left;
    dst->top = from.top;
    dst->right = from.right;
    dst->bottom = from.bottom;
    return true;
}

bool h2b(HRect const& from, BRect* to) {
    BRect const* src = reinterpret_cast<BRect const*>(from.data());
    to->left = src->left;
    to->top = src->top;
    to->right = src->right;
    to->bottom = src->bottom;
    return true;
}

// Region
// ======

bool b2h(BRegion const& from, HRegion* to) {
    size_t numRects;
    BRect const* rectArray = from.getArray(&numRects);
    to->resize(numRects);
    for (size_t i = 0; i < numRects; ++i) {
        if (!b2h(rectArray[i], &(*to)[i])) {
            return false;
        }
    }
    return true;
}

bool h2b(HRegion const& from, BRegion* to) {
    if (from.size() > 0) {
        BRect bRect;
        if (!h2b(from[0], &bRect)) {
            return false;
        }
        to->set(bRect);
        for (size_t i = 1; i < from.size(); ++i) {
            if (!h2b(from[i], &bRect)) {
                return false;
            }
            to->addRectUnchecked(
                    static_cast<int>(bRect.left),
                    static_cast<int>(bRect.top),
                    static_cast<int>(bRect.right),
                    static_cast<int>(bRect.bottom));
        }
    } else {
        to->clear();
    }
    return true;
}

// GraphicBuffer
// =============

// The handle is not cloned. Its lifetime is tied to the original GraphicBuffer.
bool b2h(sp<GraphicBuffer> const& from, HardwareBuffer* to,
         uint32_t* toGenerationNumber) {
    if (!from) {
        return false;
    }
    AHardwareBuffer* hwBuffer = from->toAHardwareBuffer();
    to->nativeHandle.setTo(
          const_cast<native_handle_t*>(
              AHardwareBuffer_getNativeHandle(hwBuffer)),
          false);
    AHardwareBuffer_describe(
            hwBuffer,
            reinterpret_cast<AHardwareBuffer_Desc*>(to->description.data()));
    if (toGenerationNumber) {
        *toGenerationNumber = from->getGenerationNumber();
    }
    return true;
}

// The handle is cloned.
bool h2b(HardwareBuffer const& from, sp<GraphicBuffer>* to) {
    AHardwareBuffer_Desc const* desc =
            reinterpret_cast<AHardwareBuffer_Desc const*>(
            from.description.data());
    native_handle_t const* handle = from.nativeHandle;
    AHardwareBuffer* hwBuffer;
    if (AHardwareBuffer_createFromHandle(
            desc, handle, AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_CLONE,
            &hwBuffer) != OK) {
        return false;
    }
    *to = GraphicBuffer::fromAHardwareBuffer(hwBuffer);
    AHardwareBuffer_release(hwBuffer);
    return true;
}

}  // namespace utils
}  // namespace V2_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android

