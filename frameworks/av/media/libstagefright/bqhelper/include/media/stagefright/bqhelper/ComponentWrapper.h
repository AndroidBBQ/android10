/*
 * Copyright 2018, The Android Open Source Project
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

#ifndef COMPONENT_WRAPPER_H_
#define COMPONENT_WRAPPER_H_

#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <ui/GraphicBuffer.h>

#include <stdint.h>

namespace android {

struct ComponentWrapper : public RefBase {
    virtual status_t submitBuffer(
            int32_t bufferId, const sp<GraphicBuffer> &buffer = nullptr,
            int64_t timestamp = 0, int fenceFd = -1) = 0;
    virtual status_t submitEos(int32_t bufferId) = 0;
    virtual void dispatchDataSpaceChanged(
            int32_t dataSpace, int32_t aspects, int32_t pixelFormat) = 0;
};

}  // namespace android

#endif  // COMPONENT_WRAPPER_H_
