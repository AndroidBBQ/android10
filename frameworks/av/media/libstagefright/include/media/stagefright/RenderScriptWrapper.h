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

#ifndef RENDERSCRIPT_WRAPPER_H_
#define RENDERSCRIPT_WRAPPER_H_

#include <RenderScript.h>

namespace android {

struct RenderScriptWrapper : public RefBase {
public:
    struct RSFilterCallback : public RefBase {
    public:
        // called by RSFilter to process each input buffer
        virtual status_t processBuffers(
                RSC::Allocation* inBuffer,
                RSC::Allocation* outBuffer) = 0;

        virtual status_t handleSetParameters(const sp<AMessage> &msg) = 0;
    };

    sp<RSFilterCallback> mCallback;
    RSC::sp<RSC::RS> mContext;
};

}   // namespace android

#endif  // RENDERSCRIPT_WRAPPER_H_
