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

#ifndef AIMAGE_READER_UTILS_H
#define AIMAGE_READER_UTILS_H

#include <cutils/native_handle.h>
#include <android/hardware/graphics/bufferqueue/1.0/IGraphicBufferProducer.h>

namespace android {
namespace aimg {

using HGraphicBufferProducer = hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer;

// Retrieves HGraphicBufferProducer corresponding to the native_handle_t
// provided (this native handle MUST have been obtained by AImageReader_getWindowNativeHandle()).
sp<HGraphicBufferProducer> AImageReader_getHGBPFromHandle(const native_handle_t *handle);

}// namespace aimg
}// namespace android

#endif //AIMAGE_READER_UTILS_H
