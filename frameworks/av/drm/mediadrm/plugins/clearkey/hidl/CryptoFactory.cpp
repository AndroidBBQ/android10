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

//#define LOG_NDEBUG 0
#define LOG_TAG "hidl_ClearKeyCryptoFactory"
#include <utils/Log.h>

#include "CryptoFactory.h"

#include "ClearKeyUUID.h"
#include "CryptoPlugin.h"
#include "TypeConvert.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

Return<bool> CryptoFactory::isCryptoSchemeSupported(
    const hidl_array<uint8_t, 16> &uuid)
{
    return clearkeydrm::isClearKeyUUID(uuid.data());
}

Return<void> CryptoFactory::createPlugin(
    const hidl_array<uint8_t, 16> &uuid,
    const hidl_vec<uint8_t> &initData,
    createPlugin_cb _hidl_cb) {

    if (!isCryptoSchemeSupported(uuid.data())) {
        ALOGE("Clearkey Drm HAL: failed to create clearkey plugin, " \
                "invalid crypto scheme");
        _hidl_cb(Status::BAD_VALUE, nullptr);
        return Void();
    }

    CryptoPlugin *cryptoPlugin = new CryptoPlugin(initData);
    Status status = cryptoPlugin->getInitStatus();
    if (status == Status::OK) {
        _hidl_cb(Status::OK, cryptoPlugin);
    } else {
        delete cryptoPlugin;
        _hidl_cb(status, nullptr);
    }
    return Void();
}

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

