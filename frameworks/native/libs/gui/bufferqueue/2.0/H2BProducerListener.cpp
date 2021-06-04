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

//#define LOG_NDEBUG 0
#define LOG_TAG "H2BProducerListener@2.0"

#include <android-base/logging.h>

#include <gui/bufferqueue/2.0/H2BProducerListener.h>
#include <hidl/Status.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V2_0 {
namespace utils {

using ::android::hardware::Return;

H2BProducerListener::H2BProducerListener(sp<HProducerListener> const& base)
      : CBase{base} {
}

void H2BProducerListener::onBufferReleased() {
    if (mBase) {
        Return<void> transResult = mBase->onBuffersReleased(1);
        if (!transResult.isOk()) {
            LOG(ERROR) << "onBuffersReleased: transaction failed.";
        }
    }
}

bool H2BProducerListener::needsReleaseNotify() {
    return static_cast<bool>(mBase);
}

}  // namespace utils
}  // namespace V2_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android

