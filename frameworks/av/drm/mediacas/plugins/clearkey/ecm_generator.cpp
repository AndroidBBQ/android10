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

//#define LOG_NDEBUG 0
#define LOG_TAG "ecm_generator"
#include "ecm_generator.h"

#include <string.h>
#include <algorithm>
#include <endian.h>

#include "protos/license_protos.pb.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaErrors.h>
#include <openssl/aes.h>
#include <utils/Log.h>

namespace android {
namespace clearkeycas {

// These constants are internal to this module.
const uint16_t kEcmClearFieldsSize = 16;
const uint32_t kContentKeyByteSize = 16;
const uint16_t kTotalEcmSize =
        kEcmClearFieldsSize + kContentKeyByteSize; // clear fields + clear key

#define UNALIGNED_LOAD32(_p) (*reinterpret_cast<const uint32_t *>(_p))

static uint32_t Load32(const void *p) {
    return ntohl(UNALIGNED_LOAD32(p));
}

static uint32_t LoadNext32(const uint8_t** pptr) {
    CHECK(pptr);
    CHECK(*pptr);
    const uint8_t* p = *pptr;
    *pptr += sizeof(uint32_t);
    return Load32(p);
}

namespace ecm_generator {

status_t DecodeECM(const sp<ABuffer>& ecm, Asset* asset,
        sp<ABuffer> *content_key, DefaultEcmFields* default_fields) {
    CHECK(asset);
    CHECK(content_key);
    CHECK(default_fields);

    status_t status = DecodeECMClearFields(ecm, asset, default_fields);
    if (status != OK) {
        return status;
    }

    const uint8_t* ptr = ecm->data() + kEcmClearFieldsSize;
    *content_key = new ABuffer(kContentKeyByteSize);
    memcpy((*content_key)->data(), ptr, kContentKeyByteSize);

    return OK;
}

status_t DecodeECMClearFields(const sp<ABuffer>& ecm, Asset* asset,
        DefaultEcmFields* default_fields) {
    CHECK(asset);
    CHECK(default_fields);

    if (ecm->size() < kTotalEcmSize) {
        ALOGE("Short ECM: expected_length=%d, actual_length=%zu",
                kTotalEcmSize, ecm->size());
        return BAD_VALUE;
    }
    const uint8_t* ptr = ecm->data();
    default_fields->old_version = LoadNext32(&ptr);
    default_fields->clear_lead = LoadNext32(&ptr);
    default_fields->system_id = LoadNext32(&ptr);
    // The real ecm version is hidden in the system id.
    default_fields->ecm_version = (default_fields->system_id >> 24) & 0xFF;
    default_fields->system_id &= 0x00FFFFFF;
    if (default_fields->system_id == 0) {
        ALOGE("Ecm has invalid system_id 0");
        return CLEARKEY_STATUS_INVALIDSYSTEMID;
    }
    asset->set_id(LoadNext32(&ptr));
    if (asset->id() == 0) {
        ALOGE("Ecm has invalid asset_id 0");
        return CLEARKEY_STATUS_INVALIDASSETID;
    }
    return OK;
}

}  // namespace ecm_generator
}  // namespace clearkeycas
}  // namespace android
