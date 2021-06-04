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

#pragma once

#include <cinttypes>
#include <cstdint>

#include <android/hardware/graphics/common/1.1/types.h>
#include <android/hardware/graphics/common/1.2/types.h>
#include <system/graphics.h>

#define ANDROID_PHYSICAL_DISPLAY_ID_FORMAT PRIu64

namespace android {

using PhysicalDisplayId = uint64_t;

// android::ui::* in this header file will alias different types as
// the HIDL interface is updated.
namespace ui {

using android::hardware::graphics::common::V1_1::RenderIntent;
using android::hardware::graphics::common::V1_2::ColorMode;
using android::hardware::graphics::common::V1_2::Dataspace;
using android::hardware::graphics::common::V1_2::Hdr;
using android::hardware::graphics::common::V1_2::PixelFormat;

}  // namespace ui
}  // namespace android
