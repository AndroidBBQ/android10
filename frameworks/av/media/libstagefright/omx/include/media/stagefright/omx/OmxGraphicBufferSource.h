/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef OMX_GRAPHIC_BUFFER_SOURCE_H_

#define OMX_GRAPHIC_BUFFER_SOURCE_H_

#include <media/stagefright/bqhelper/GraphicBufferSource.h>
#include <media/stagefright/foundation/ABase.h>

#include <android/BnGraphicBufferSource.h>
#include <android/BnOMXBufferSource.h>

#include "IOmxNodeWrapper.h"

namespace android {

using ::android::binder::Status;

/*
 * This class is used to feed OMX codecs from a Surface via BufferQueue or
 * HW producer.
 *
 * See media/stagefright/bqhelper/GraphicBufferSource.h for documentation.
 */
class OmxGraphicBufferSource : public GraphicBufferSource {
public:
    OmxGraphicBufferSource() = default;
    virtual ~OmxGraphicBufferSource() = default;

    // OmxBufferSource interface
    // ------------------------------

    // This is called when OMX transitions to OMX_StateExecuting, which means
    // we can start handing it buffers.  If we already have buffers of data
    // sitting in the BufferQueue, this will send them to the codec.
    Status onOmxExecuting();

    // This is called when OMX transitions to OMX_StateIdle, indicating that
    // the codec is meant to return all buffers back to the client for them
    // to be freed. Do NOT submit any more buffers to the component.
    Status onOmxIdle();

    // This is called when OMX transitions to OMX_StateLoaded, indicating that
    // we are shutting down.
    Status onOmxLoaded();

    // Rest of the interface in GraphicBufferSource.

    // IGraphicBufferSource interface
    // ------------------------------

    // Configure the buffer source to be used with an OMX node with the default
    // data space.
    status_t configure(
        const sp<IOmxNodeWrapper> &omxNode,
        int32_t dataSpace,
        int32_t bufferCount,
        uint32_t frameWidth,
        uint32_t frameHeight,
        uint32_t consumerUsage);

    // Rest of the interface in GraphicBufferSource.

private:
    DISALLOW_EVIL_CONSTRUCTORS(OmxGraphicBufferSource);
};

}  // namespace android

#endif  // OMX_GRAPHIC_BUFFER_SOURCE_H_
