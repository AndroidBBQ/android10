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
#define LOG_TAG "ClearKeyFetcher"

#include <algorithm>
#include <inttypes.h>
#include <string>

#include "ClearKeyFetcher.h"
#include "ecm.h"
#include "LicenseFetcher.h"

#include <media/stagefright/foundation/ADebug.h>
#include <utils/Log.h>

namespace android {
namespace clearkeycas {

ClearKeyFetcher::ClearKeyFetcher(
        std::unique_ptr<LicenseFetcher> license_fetcher) :
    initialized_(false),
    license_fetcher_(std::move(license_fetcher)) {
    CHECK(license_fetcher_);
}

ClearKeyFetcher::~ClearKeyFetcher() {}

// This is a no-op but other KeyFetcher subclasses require initialization
// so this is necessary to preserve the contract.
status_t ClearKeyFetcher::Init() {
    initialized_ = true;
    return OK;
}

status_t ClearKeyFetcher::ObtainKey(const sp<ABuffer>& buffer,
        uint64_t* asset_id, std::vector<KeyInfo>* keys) {
    CHECK(asset_id);
    CHECK(keys);
    CHECK(initialized_);
    *asset_id = 0;
    keys->clear();

    EcmContainer container;
    status_t status = container.Parse(buffer);
    if (status != OK) {
        return status;
    }
    ALOGV("descriptor_size=%zu", container.descriptor_size());

    // Sanity check to verify that the BroadcastEncryptor is sending a properly
    // formed EcmContainer. If it contains two Ecms, the ids should have different
    // parity (one odd, one even). This does not necessarily affect decryption
    // but indicates a problem with Ecm generation.
    if (container.descriptor_size() == 2) {
        // XOR the least significant bits to verify different parity.
        bool same_parity = (((container.descriptor(0).id() & 0x01) ^
                (container.descriptor(1).id() & 0x01)) == 0);
        if (same_parity) {
            ALOGW("asset_id=%" PRIu64 ": malformed Ecm, "
                    "content keys have same parity, id0=%d, id1=%d",
                    container.descriptor(0).ecm().asset_id(),
                    container.descriptor(0).id(),
                    container.descriptor(1).id());
        }
    }

    *asset_id = container.descriptor(0).ecm().asset_id();

    // Detect asset_id change. This could be caused by a configuration change
    // in the BroadcastEncryptor. This is unusual so log it in case it is an
    // operational mistake. This invalidates the current asset_key causing a
    // new license to be fetched.
    // TODO(rkint): test against BroadcastEncryptor to verify what BE sends on
    // asset_id change. If it sends an EcmContainer with 2 Ecms with different
    // asset_ids (old and new) then it might be best to prefetch the Emm.
    if ((asset_.id() != 0) && (*asset_id != asset_.id())) {
        ALOGW("Asset_id change from %llu to %" PRIu64, asset_.id(), *asset_id);
        asset_.Clear();
    }

    // Fetch license to get asset_id
    if (!asset_.has_id()) {
        status = license_fetcher_->FetchLicense(*asset_id, &asset_);
        if (status != OK) {
            *asset_id = 0;
            return status;
        }
        ALOGV("FetchLicense succeeded, has_id=%d", asset_.has_id());
    }
    keys->resize(container.descriptor_size());

    for (size_t i = 0; i < container.descriptor_size(); ++i) {
        status = container.mutable_descriptor(i)->mutable_ecm()->Decrypt(
                container.descriptor(i).ecm().buffer(), asset_);
        if (status != OK) {
            *asset_id = 0;
            keys->clear();
            return status;
        }
        // TODO: if 2 Ecms have same parity, key from Ecm with higher id
        // should be keys[1].
        KeyInfo key;
        key.key_id = container.descriptor(i).id();
        key.key_bytes = container.descriptor(i).ecm().content_key();

        keys->at(key.key_id & 1) = key;
    }
    return OK;
}

}  // namespace clearkeycas
}  // namespace android
