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

#ifndef CLIENT_BLOCK_HELPER_H
#define CLIENT_BLOCK_HELPER_H

#include <gui/IGraphicBufferProducer.h>
#include <codec2/hidl/1.0/types.h>
#include <C2Work.h>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

// BufferQueue-Based Block Operations
// ==================================

// Manage BufferQueue and graphic blocks for both component and codec.
// Manage graphic blocks ownership consistently during surface change.
struct OutputBufferQueue {

    OutputBufferQueue();

    ~OutputBufferQueue();

    // Configure a new surface to render graphic blocks.
    // Graphic blocks from older surface will be migrated to new surface.
    bool configure(const sp<IGraphicBufferProducer>& igbp,
                   uint32_t generation,
                   uint64_t bqId);

    // Render a graphic block to current surface.
    status_t outputBuffer(
            const C2ConstGraphicBlock& block,
            const BnGraphicBufferProducer::QueueBufferInput& input,
            BnGraphicBufferProducer::QueueBufferOutput* output);

    // Call holdBufferQueueBlock() on output blocks in the given workList.
    // The OutputBufferQueue will take the ownership of output blocks.
    //
    // Note: This function should be called after WorkBundle has been received
    // from another process.
    void holdBufferQueueBlocks(
            const std::list<std::unique_ptr<C2Work>>& workList);

private:

    class Impl;
    std::unique_ptr<Impl> mImpl;
};

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // CLIENT_BLOCK_HELPER_H
