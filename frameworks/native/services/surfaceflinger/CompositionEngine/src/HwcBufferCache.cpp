/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <compositionengine/impl/HwcBufferCache.h>
#include <gui/BufferQueue.h>
#include <ui/GraphicBuffer.h>

namespace android::compositionengine::impl {

HwcBufferCache::HwcBufferCache() {
    std::fill(std::begin(mBuffers), std::end(mBuffers), wp<GraphicBuffer>(nullptr));
}

void HwcBufferCache::getHwcBuffer(int slot, const sp<GraphicBuffer>& buffer, uint32_t* outSlot,
                                  sp<GraphicBuffer>* outBuffer) {
    // default is 0
    if (slot == BufferQueue::INVALID_BUFFER_SLOT || slot < 0 ||
        slot >= BufferQueue::NUM_BUFFER_SLOTS) {
        *outSlot = 0;
    } else {
        *outSlot = slot;
    }

    auto& currentBuffer = mBuffers[*outSlot];
    wp<GraphicBuffer> weakCopy(buffer);
    if (currentBuffer == weakCopy) {
        // already cached in HWC, skip sending the buffer
        *outBuffer = nullptr;
    } else {
        *outBuffer = buffer;

        // update cache
        currentBuffer = buffer;
    }
}

} // namespace android::compositionengine::impl
