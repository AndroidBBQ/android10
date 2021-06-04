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

#ifndef ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_B2HPRODUCERLISTENER_H
#define ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_B2HPRODUCERLISTENER_H

#include <android/hidl/base/1.0/IBase.h>
#include <binder/IBinder.h>
#include <gui/IProducerListener.h>
#include <hidl/Status.h>

#include <android/hardware/graphics/bufferqueue/2.0/IProducerListener.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V2_0 {
namespace utils {

using ::android::hardware::Return;

using HProducerListener = ::android::hardware::graphics::bufferqueue::V2_0::
        IProducerListener;

using BProducerListener = ::android::IProducerListener;

struct B2HProducerListener : public HProducerListener {
    explicit B2HProducerListener(sp<BProducerListener> const& base);
    Return<void> onBuffersReleased(uint32_t count) override;
protected:
    sp<BProducerListener> mBase;
    bool mNeedsReleaseNotify;
};

}  // namespace utils
}  // namespace V2_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V2_0_B2HPRODUCERLISTENER_H

