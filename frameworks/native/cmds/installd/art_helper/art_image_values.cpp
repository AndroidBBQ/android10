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

#include "art_image_values.h"

namespace android {
namespace installd {
namespace art {

uint32_t GetImageBaseAddress() {
    return ART_BASE_ADDRESS;
}
int32_t GetImageMinBaseAddressDelta() {
    return ART_BASE_ADDRESS_MIN_DELTA;
}
int32_t GetImageMaxBaseAddressDelta() {
    return ART_BASE_ADDRESS_MAX_DELTA;
}

static_assert(ART_BASE_ADDRESS_MIN_DELTA < ART_BASE_ADDRESS_MAX_DELTA, "Inconsistent setup");

}  // namespace art
}  // namespace installd
}  // namespace android
