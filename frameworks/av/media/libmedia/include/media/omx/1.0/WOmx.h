/*
 * Copyright 2016, The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_MEDIA_OMX_V1_0_WOMX_H
#define ANDROID_HARDWARE_MEDIA_OMX_V1_0_WOMX_H

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <media/IOMX.h>

#include <hidl/HybridInterface.h>
#include <android/hardware/media/omx/1.0/IOmx.h>

namespace android {
namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace utils {

using ::android::hardware::media::omx::V1_0::IOmx;
using ::android::hardware::media::omx::V1_0::IOmxNode;
using ::android::hardware::media::omx::V1_0::IOmxObserver;
using ::android::hardware::media::omx::V1_0::Status;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using ::android::List;
using ::android::IOMX;

/**
 * Wrapper classes for conversion
 * ==============================
 *
 * Naming convention:
 * - LW = Legacy Wrapper --- It wraps a Treble object inside a legacy object.
 * - TW = Treble Wrapper --- It wraps a legacy object inside a Treble object.
 */

struct LWOmx : public IOMX {
    sp<IOmx> mBase;
    LWOmx(sp<IOmx> const& base);
    status_t listNodes(List<IOMX::ComponentInfo>* list) override;
    status_t allocateNode(
            char const* name,
            sp<IOMXObserver> const& observer,
            sp<IOMXNode>* omxNode) override;
    status_t createInputSurface(
            sp<::android::IGraphicBufferProducer>* bufferProducer,
            sp<::android::IGraphicBufferSource>* bufferSource) override;
};

}  // namespace utils
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_OMX_V1_0_WOMX_H
