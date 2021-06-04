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

#ifndef CLEARKEY_CRYPTO_PLUGIN_H_
#define CLEARKEY_CRYPTO_PLUGIN_H_

#include <android/hardware/drm/1.2/ICryptoPlugin.h>
#include <android/hidl/memory/1.0/IMemory.h>

#include "ClearKeyTypes.h"
#include "Session.h"
#include "Utils.h"

namespace {
    static const size_t KEY_ID_SIZE = 16;
    static const size_t KEY_IV_SIZE = 16;
}

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

namespace drm = ::android::hardware::drm;
using drm::V1_0::DestinationBuffer;
using drm::V1_0::Mode;
using drm::V1_0::Pattern;
using drm::V1_0::SharedBuffer;
using drm::V1_0::Status;
using drm::V1_0::SubSample;

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::sp;

typedef drm::V1_2::Status Status_V1_2;

struct CryptoPlugin : public drm::V1_2::ICryptoPlugin {
    explicit CryptoPlugin(const hidl_vec<uint8_t>& sessionId) {
        mInitStatus = setMediaDrmSession(sessionId);
    }
    virtual ~CryptoPlugin() {}

    Return<bool> requiresSecureDecoderComponent(const hidl_string& mime) {
        UNUSED(mime);
        return false;
    }

    Return<void> notifyResolution(uint32_t width, uint32_t height) {
        UNUSED(width);
        UNUSED(height);
        return Void();
    }

    Return<void> decrypt(
            bool secure,
            const hidl_array<uint8_t, KEY_ID_SIZE>& keyId,
            const hidl_array<uint8_t, KEY_IV_SIZE>& iv,
            Mode mode,
            const Pattern& pattern,
            const hidl_vec<SubSample>& subSamples,
            const SharedBuffer& source,
            uint64_t offset,
            const DestinationBuffer& destination,
            decrypt_cb _hidl_cb);

    Return<void> decrypt_1_2(
            bool secure,
            const hidl_array<uint8_t, KEY_ID_SIZE>& keyId,
            const hidl_array<uint8_t, KEY_IV_SIZE>& iv,
            Mode mode,
            const Pattern& pattern,
            const hidl_vec<SubSample>& subSamples,
            const SharedBuffer& source,
            uint64_t offset,
            const DestinationBuffer& destination,
            decrypt_1_2_cb _hidl_cb);

    Return<void> setSharedBufferBase(const hidl_memory& base,
            uint32_t bufferId);

    Return<Status> setMediaDrmSession(const hidl_vec<uint8_t>& sessionId);

    Return<Status> getInitStatus() const { return mInitStatus; }

private:
    CLEARKEY_DISALLOW_COPY_AND_ASSIGN(CryptoPlugin);

    std::map<uint32_t, sp<IMemory> > mSharedBufferMap;
    sp<Session> mSession;
    Status mInitStatus;
};

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

#endif // CLEARKEY_CRYPTO_PLUGIN_H_
