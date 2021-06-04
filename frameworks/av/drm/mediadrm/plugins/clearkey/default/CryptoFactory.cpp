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
#define LOG_TAG "ClearKeyCryptoFactory"
#include <utils/Log.h>

#include <utils/Errors.h>
#include <utils/StrongPointer.h>

#include "CryptoFactory.h"

#include "ClearKeyUUID.h"
#include "CryptoPlugin.h"
#include "Session.h"
#include "SessionLibrary.h"

namespace clearkeydrm {

bool CryptoFactory::isCryptoSchemeSupported(const uint8_t uuid[16]) const {
    return isClearKeyUUID(uuid);
}

android::status_t CryptoFactory::createPlugin(
        const uint8_t uuid[16],
        const void* data, size_t size,
        android::CryptoPlugin** plugin) {
    if (!isCryptoSchemeSupported(uuid)) {
        *plugin = NULL;
        return android::BAD_VALUE;
    }

    android::Vector<uint8_t> sessionId;
    sessionId.appendArray(reinterpret_cast<const uint8_t*>(data), size);

    CryptoPlugin *clearKeyPlugin = new CryptoPlugin(sessionId);
    android::status_t result = clearKeyPlugin->getInitStatus();
    if (result == android::OK) {
        *plugin = clearKeyPlugin;
    } else {
        delete clearKeyPlugin;
        *plugin = NULL;
    }
    return result;
}

} // namespace clearkeydrm
