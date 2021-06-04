/*
 * Copyright 2018, The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_MEDIA_C2_V1_0_INPUT_SURFACE_H
#define ANDROID_HARDWARE_MEDIA_C2_V1_0_INPUT_SURFACE_H

#include <memory>

#include <C2Component.h>
#include <media/stagefright/codec2/1.0/InputSurfaceConnection.h>

namespace android {

class GraphicBufferSource;

namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace implementation {

using ::android::sp;

typedef ::android::hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer
        HGraphicBufferProducer;
typedef ::android::IGraphicBufferProducer BGraphicBufferProducer;

// TODO: ::android::TWGraphicBufferProducer<IInputSurface>
typedef ::android::TWGraphicBufferProducer<HGraphicBufferProducer> InputSurfaceBase;

class InputSurface : public InputSurfaceBase {
public:
    virtual ~InputSurface() = default;

    // Methods from IInputSurface
    sp<InputSurfaceConnection> connectToComponent(
            const std::shared_ptr<::C2Component> &comp);
    // TODO: intf()

    static sp<InputSurface> Create();

private:
    InputSurface(
            const sp<BGraphicBufferProducer> &base,
            const sp<::android::GraphicBufferSource> &source);

    sp<::android::GraphicBufferSource> mSource;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_C2_V1_0_INPUT_SURFACE_H
