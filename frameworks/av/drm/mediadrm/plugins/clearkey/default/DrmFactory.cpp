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
#define LOG_TAG "ClearKeyDrmFactory"
#include <utils/Log.h>

#include <utils/Errors.h>

#include "DrmFactory.h"

#include "DrmPlugin.h"
#include "ClearKeyUUID.h"
#include "MimeType.h"
#include "SessionLibrary.h"

namespace clearkeydrm {

bool DrmFactory::isCryptoSchemeSupported(const uint8_t uuid[16]) {
    return isClearKeyUUID(uuid);
}

bool DrmFactory::isContentTypeSupported(const android::String8 &type) {
    // This should match the types handed by InitDataParser.
    return type == kIsoBmffVideoMimeType ||
        type == kIsoBmffAudioMimeType ||
        type == kCencInitDataFormat ||
        type == kWebmVideoMimeType ||
        type == kWebmAudioMimeType ||
        type == kWebmInitDataFormat;
}

android::status_t DrmFactory::createDrmPlugin(
        const uint8_t uuid[16],
        android::DrmPlugin** plugin) {
    if (!isCryptoSchemeSupported(uuid)) {
        *plugin = NULL;
        return android::BAD_VALUE;
    }

    *plugin = new DrmPlugin(SessionLibrary::get());
    return android::OK;
}

} // namespace clearkeydrm
