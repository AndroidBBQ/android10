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

#include <gui/bufferqueue/1.0/B2HProducerListener.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V1_0 {
namespace utils {

// B2HProducerListener
B2HProducerListener::B2HProducerListener(
        sp<BProducerListener> const& base):
    mBase(base) {
}

Return<void> B2HProducerListener::onBufferReleased() {
    mBase->onBufferReleased();
    return Void();
}

Return<bool> B2HProducerListener::needsReleaseNotify() {
    return mBase->needsReleaseNotify();
}

}  // namespace utils
}  // namespace V1_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android
