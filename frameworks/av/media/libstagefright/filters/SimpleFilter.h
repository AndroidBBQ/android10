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

#ifndef SIMPLE_FILTER_H_
#define SIMPLE_FILTER_H_

#include <stdint.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>

namespace android {

struct AMessage;
class MediaCodecBuffer;

struct SimpleFilter : public RefBase {
public:
    SimpleFilter() : mWidth(0), mHeight(0), mStride(0), mSliceHeight(0),
            mColorFormatIn(0), mColorFormatOut(0) {};

    virtual status_t configure(const sp<AMessage> &msg);

    virtual status_t start() = 0;
    virtual void reset() = 0;
    virtual status_t setParameters(const sp<AMessage> &msg) = 0;
    virtual status_t processBuffers(
            const sp<MediaCodecBuffer> &srcBuffer, const sp<MediaCodecBuffer> &outBuffer) = 0;

protected:
    int32_t mWidth, mHeight;
    int32_t mStride, mSliceHeight;
    int32_t mColorFormatIn, mColorFormatOut;

    virtual ~SimpleFilter() {};
};

}   // namespace android

#endif  // SIMPLE_FILTER_H_
