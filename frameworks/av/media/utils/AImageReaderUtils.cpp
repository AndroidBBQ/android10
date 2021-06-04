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

#include <cmath>

#include <mediautils/AImageReaderUtils.h>
#include <hidl/HybridInterface.h>

namespace android {
namespace aimg {

using hardware::hidl_vec;

static sp<HGraphicBufferProducer> convertNativeHandleToHGBP (
        const native_handle_t *handle) {
    // Read the size of the halToken vec<uint8_t>
    hidl_vec<uint8_t> halToken;
    halToken.setToExternal(
        reinterpret_cast<uint8_t *>(const_cast<int *>(&(handle->data[1]))),
        handle->data[0]);
    sp<HGraphicBufferProducer> hgbp =
        HGraphicBufferProducer::castFrom(retrieveHalInterface(halToken));
    return hgbp;
}

sp<HGraphicBufferProducer> AImageReader_getHGBPFromHandle(
    const native_handle_t *handle) {
    if (handle == nullptr) {
        return nullptr;
    }
    if (handle->numFds != 0  ||
        handle->numInts < std::ceil(sizeof(size_t) / sizeof(int))) {
        return nullptr;
    }
    return convertNativeHandleToHGBP(handle);
}

} //namespace aimg
} //namespace android
