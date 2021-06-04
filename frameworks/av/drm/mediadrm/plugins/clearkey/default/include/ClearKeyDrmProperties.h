/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef CLEARKEY_DRM_PROPERTIES_H_
#define CLEARKEY_DRM_PROPERTIES_H_

#include <utils/String8.h>

namespace clearkeydrm {

static const android::String8 kVendorKey("vendor");
static const android::String8 kVendorValue("Google");
static const android::String8 kVersionKey("version");
static const android::String8 kVersionValue("1.0");
static const android::String8 kPluginDescriptionKey("description");
static const android::String8 kPluginDescriptionValue("ClearKey CDM");
static const android::String8 kAlgorithmsKey("algorithms");
static const android::String8 kAlgorithmsValue("");
static const android::String8 kListenerTestSupportKey("listenerTestSupport");
static const android::String8 kListenerTestSupportValue("true");

static const android::String8 kDeviceIdKey("deviceId");
static const uint8_t kTestDeviceIdData[] =
        {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
         0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
} // namespace clearkeydrm

#endif // CLEARKEY_DRM_PROPERTIES_H_
