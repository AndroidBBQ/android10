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

#include <gui/bufferqueue/2.0/B2HProducerListener.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V2_0 {
namespace utils {

// B2HProducerListener
B2HProducerListener::B2HProducerListener(sp<BProducerListener> const& base)
      : mBase{base},
        mNeedsReleaseNotify{base ? base->needsReleaseNotify() : false} {
}

Return<void> B2HProducerListener::onBuffersReleased(uint32_t count) {
    if (mNeedsReleaseNotify) {
        for (; count > 0; --count) {
            mBase->onBufferReleased();
        }
    }
    return {};
}

}  // namespace utils
}  // namespace V2_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android

