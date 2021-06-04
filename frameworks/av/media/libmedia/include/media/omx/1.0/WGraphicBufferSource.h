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

#ifndef ANDROID_HARDWARE_MEDIA_OMX_V1_0_WGRAPHICBUFFERSOURCE_H
#define ANDROID_HARDWARE_MEDIA_OMX_V1_0_WGRAPHICBUFFERSOURCE_H

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <binder/Binder.h>
#include <media/IOMX.h>

#include <android/hardware/graphics/common/1.0/types.h>
#include <android/hardware/media/omx/1.0/IOmxNode.h>
#include <android/hardware/media/omx/1.0/IGraphicBufferSource.h>

#include <android/BnGraphicBufferSource.h>

namespace android {
namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace utils {

using ::android::hardware::graphics::common::V1_0::Dataspace;
using ::android::hardware::media::omx::V1_0::ColorAspects;
using ::android::hardware::media::omx::V1_0::IGraphicBufferSource;
using ::android::hardware::media::omx::V1_0::IOmxNode;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::IOMXNode;

/**
 * Wrapper classes for conversion
 * ==============================
 *
 * Naming convention:
 * - LW = Legacy Wrapper --- It wraps a Treble object inside a legacy object.
 * - TW = Treble Wrapper --- It wraps a legacy object inside a Treble object.
 */

typedef ::android::binder::Status BnStatus;
typedef ::android::BnGraphicBufferSource BnGraphicBufferSource;
typedef ::android::hardware::media::omx::V1_0::IGraphicBufferSource
        TGraphicBufferSource;

struct LWGraphicBufferSource : public BnGraphicBufferSource {
    sp<TGraphicBufferSource> mBase;
    LWGraphicBufferSource(sp<TGraphicBufferSource> const& base);
    BnStatus configure(const sp<IOMXNode>& omxNode, int32_t dataSpace) override;
    BnStatus setSuspend(bool suspend, int64_t timeUs) override;
    BnStatus setRepeatPreviousFrameDelayUs(int64_t repeatAfterUs) override;
    BnStatus setMaxFps(float maxFps) override;
    BnStatus setTimeLapseConfig(double fps, double captureFps) override;
    BnStatus setStartTimeUs(int64_t startTimeUs) override;
    BnStatus setStopTimeUs(int64_t stopTimeUs) override;
    BnStatus getStopTimeOffsetUs(int64_t *stopTimeOffsetUs) override;
    BnStatus setColorAspects(int32_t aspects) override;
    BnStatus setTimeOffsetUs(int64_t timeOffsetsUs) override;
    BnStatus signalEndOfInputStream() override;
};

}  // namespace utils
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_OMX_V1_0_WGRAPHICBUFFERSOURCE_H
