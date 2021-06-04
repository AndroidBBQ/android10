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

#ifndef ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_TYPES_H
#define ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_TYPES_H

#include <android/hardware/graphics/bufferqueue/2.0/types.h>
#include <android/hardware/graphics/common/1.2/types.h>
#include <hidl/HidlSupport.h>
#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V2_0 {
namespace utils {

// Status
// ======

using HStatus = ::android::hardware::graphics::bufferqueue::V2_0::
        Status;

// A status_t value may have flags encoded. These flags are decoded into boolean
// values if their corresponding output pointers are not null.
bool b2h(status_t from, HStatus* to,
         bool* bufferNeedsReallocation = nullptr,
         bool* releaseAllBuffers = nullptr);
// Simple 1-to-1 mapping. If BUFFER_NEEDS_REALLOCATION or RELEASE_ALL_BUFFERS
// needs to be added, it must be done manually afterwards.
bool h2b(HStatus from, status_t* to);

// Fence
// =====

using BFence = ::android::Fence;
// This class manages the lifetime of a copied handle. Its destructor calls
// native_handle_delete() but not native_handle_close().
struct HFenceWrapper {
    HFenceWrapper() = default;
    // Sets mHandle to a new value.
    HFenceWrapper(native_handle_t* h);
    // Deletes mHandle without closing.
    ~HFenceWrapper();
    // Deletes mHandle without closing, then sets mHandle to a new value.
    HFenceWrapper& set(native_handle_t* h);
    HFenceWrapper& operator=(native_handle_t* h);
    // Returns a non-owning hidl_handle pointing to mHandle.
    hidl_handle getHandle() const;
    operator hidl_handle() const;
protected:
    native_handle_t* mHandle{nullptr};
};

// Does not clone the fd---only copy the fd. The returned HFenceWrapper should
// not outlive the input Fence object.
bool b2h(sp<BFence> const& from, HFenceWrapper* to);
// Clones the fd and puts it in a new Fence object.
bool h2b(native_handle_t const* from, sp<BFence>* to);

// ConnectionType
// ==============

using HConnectionType = ::android::hardware::graphics::bufferqueue::V2_0::
        ConnectionType;

bool b2h(int from, HConnectionType* to);
bool h2b(HConnectionType from, int* to);

// Rect
// ====

using BRect = ::android::Rect;
using HRect = ::android::hardware::graphics::common::V1_2::Rect;

bool b2h(BRect const& from, HRect* to);
bool h2b(HRect const& from, BRect* to);

// Region
// ======

using BRegion = ::android::Region;
using HRegion = ::android::hardware::hidl_vec<HRect>;

bool b2h(BRegion const& from, HRegion* to);
bool h2b(HRegion const& from, BRegion* to);

// GraphicBuffer
// =============

using HardwareBuffer = ::android::hardware::graphics::common::V1_2::
        HardwareBuffer;
using HardwareBufferDescription = ::android::hardware::graphics::common::V1_2::
        HardwareBufferDescription;

// Does not clone the handle. The returned HardwareBuffer should not outlive the
// input GraphicBuffer. Note that HardwareBuffer does not carry the generation
// number, so this function needs another output argument.
bool b2h(sp<GraphicBuffer> const& from, HardwareBuffer* to,
         uint32_t* toGenerationNumber = nullptr);
// Clones the handle and creates a new GraphicBuffer from the cloned handle.
// Note that the generation number of the GraphicBuffer has to be set manually
// afterwards because HardwareBuffer does not have such information.
bool h2b(HardwareBuffer const& from, sp<GraphicBuffer>* to);

}  // namespace utils
}  // namespace V2_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_TYPES_H

