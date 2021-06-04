/*
 * Copyright 2018 The Android Open Source Project
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

#ifndef CODEC2_HIDL_V1_0_UTILS_INPUTSURFACE_H
#define CODEC2_HIDL_V1_0_UTILS_INPUTSURFACE_H

#include <codec2/hidl/1.0/ComponentStore.h>

#include <android/hardware/graphics/bufferqueue/2.0/IGraphicBufferProducer.h>
#include <android/hardware/media/c2/1.0/IInputSink.h>
#include <android/hardware/media/c2/1.0/IInputSurface.h>
#include <hidl/Status.h>
#include <media/stagefright/bqhelper/GraphicBufferSource.h>

#include <util/C2InterfaceHelper.h>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct InputSurface : public IInputSurface {

    typedef ::android::hardware::graphics::bufferqueue::V2_0::
            IGraphicBufferProducer HGraphicBufferProducer;

    typedef ::android::
            GraphicBufferSource GraphicBufferSource;

    virtual Return<sp<HGraphicBufferProducer>> getGraphicBufferProducer() override;

    virtual Return<sp<IConfigurable>> getConfigurable() override;

    virtual Return<void> connect(
            const sp<IInputSink>& sink,
            connect_cb _hidl_cb) override;

protected:

    class Interface;
    class ConfigurableIntf;

    sp<ComponentStore> mStore;
    sp<HGraphicBufferProducer> mProducer;
    sp<GraphicBufferSource> mSource;
    std::shared_ptr<Interface> mIntf;
    sp<CachedConfigurable> mConfigurable;

    InputSurface(
            const sp<ComponentStore>& store,
            const std::shared_ptr<C2ReflectorHelper>& reflector,
            const sp<HGraphicBufferProducer>& base,
            const sp<GraphicBufferSource>& source);

    virtual ~InputSurface() override = default;

    friend struct ComponentStore;

};


}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // CODEC2_HIDL_V1_0_UTILS_INPUTSURFACE_H
