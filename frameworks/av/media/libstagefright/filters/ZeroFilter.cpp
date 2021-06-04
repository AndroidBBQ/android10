/*
 * Copyright (C) 2014 The Android Open Source Project
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
#define LOG_TAG "ZeroFilter"

#include <media/MediaCodecBuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>

#include "ZeroFilter.h"

namespace android {

status_t ZeroFilter::setParameters(const sp<AMessage> &msg) {
    sp<AMessage> params;
    CHECK(msg->findMessage("params", &params));

    int32_t invert;
    if (params->findInt32("invert", &invert)) {
        mInvertData = (invert != 0);
    }

    return OK;
}

status_t ZeroFilter::processBuffers(
        const sp<MediaCodecBuffer> &srcBuffer, const sp<MediaCodecBuffer> &outBuffer) {
    // assuming identical input & output buffers, since we're a copy filter
    if (mInvertData) {
        uint32_t* src = (uint32_t*)srcBuffer->data();
        uint32_t* dest = (uint32_t*)outBuffer->data();
        for (size_t i = 0; i < srcBuffer->size() / 4; ++i) {
            *(dest++) = *(src++) ^ 0xFFFFFFFF;
        }
    } else {
        memcpy(outBuffer->data(), srcBuffer->data(), srcBuffer->size());
    }
    outBuffer->setRange(0, srcBuffer->size());

    return OK;
}

}   // namespace android
