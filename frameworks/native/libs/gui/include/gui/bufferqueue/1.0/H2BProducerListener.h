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

#ifndef ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_H2BPRODUCERLISTENER_H
#define ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_H2BPRODUCERLISTENER_H

#include <android/hardware/graphics/bufferqueue/1.0/IProducerListener.h>
#include <gui/IProducerListener.h>
#include <hidl/HybridInterface.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V1_0 {
namespace utils {

using HProducerListener = ::android::hardware::graphics::bufferqueue::V1_0::
        IProducerListener;

using BProducerListener = ::android::IProducerListener;

class H2BProducerListener
      : public H2BConverter<HProducerListener, BnProducerListener> {
public:
    H2BProducerListener(sp<HProducerListener> const& base);
    virtual void onBufferReleased() override;
    virtual bool needsReleaseNotify() override;
};

}  // namespace utils
}  // namespace V1_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_H2BPRODUCERLISTENER_H

