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

#ifndef ZERO_FILTER_H_
#define ZERO_FILTER_H_

#include "SimpleFilter.h"

namespace android {

struct ZeroFilter : public SimpleFilter {
public:
    ZeroFilter() : mInvertData(false) {};

    virtual status_t start() { return OK; };
    virtual void reset() {};
    virtual status_t setParameters(const sp<AMessage> &msg);
    virtual status_t processBuffers(
            const sp<MediaCodecBuffer> &srcBuffer, const sp<MediaCodecBuffer> &outBuffer);

protected:
    virtual ~ZeroFilter() {};

private:
    bool mInvertData;
};

}   // namespace android

#endif  // ZERO_FILTER_H_
