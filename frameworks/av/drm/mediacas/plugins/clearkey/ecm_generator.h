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

#ifndef CLEAR_KEY_ECM_GENERATOR_H_
#define CLEAR_KEY_ECM_GENERATOR_H_

#include <string>

#include "protos/license_protos.pb.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/MediaErrors.h>

using namespace std;

namespace android {
namespace clearkeycas {
enum {
    CLEARKEY_STATUS_BASE = ERROR_CAS_VENDOR_MAX,
    CLEARKEY_STATUS_INVALIDASSETID = CLEARKEY_STATUS_BASE - 1,
    CLEARKEY_STATUS_INVALIDSYSTEMID = CLEARKEY_STATUS_BASE - 2,
    CLEARKEY_STATUS_INVALID_PARAMETER = CLEARKEY_STATUS_BASE - 3,
};
class Organization;

namespace ecm_generator {

// Layout of the ECM
// ECM
//  0 -  3 : Old ECM version (deprecated)
//  4 -  7 : Clear lead (milliseconds)
//       8 : ECM Version
//  9 - 11 : System ID
// 12 - 15 : Asset ID
// 16 - 31 : Content Key (clear)
//
// The clear asset ID (bytes 12-15) is compared to the encrypted asset ID
// (bytes 48-51) as a consistency check.

struct DefaultEcmFields {
    uint32_t old_version;
    uint32_t clear_lead;
    uint32_t ecm_version;
    uint32_t system_id;
};

// Decodes a clear key ecm.
// The following fields are decoded from the clear fields portion of the ecm:
//   asset->id
//   default_fields->old_version
//   default_fields->clear_lead
//   default_fields->system_id
//   default_fields->ecm_version
//
// The following fields are decoded from the content key portion of the ecm:
//   content_key
//
// |asset|, |content_key|, |default_fields| are owned by caller and must not
// be NULL.
// Returns failure via ecm_generator::DecodeECMClearFields.
//
// Example usage:
//   Asset asset;
//   string content_key;
//   DefaultEcmFields default_fields;
//   // Get a clear key |ecm|.
//   status_t status = ecm_generator::DecodeECM(ecm, &asset, &content_key, &default_fields);
status_t DecodeECM(const sp<ABuffer>& ecm, Asset* asset,
        sp<ABuffer> *content_key, DefaultEcmFields* default_fields);

// Decodes the following fields from the clear fields portion of the ecm:
//   asset->id
//   default_fields->old_version
//   default_fields->clear_lead
//   default_fields->system_id
//   default_fields->ecm_version
//
// offset, asset and default_fields are owned by caller and must not be NULL.
// offset is updated to show the number of bytes consumed.
// Returns:
// - BAD_VALUE on short ECM, or
// - CLEARKEY_STATUS_INVALIDASSETID via ecm_generator::DecodeEcmClearFields if
//   asset_id is 0, or
// - CLEARKEY_STATUS_INVALIDSYSTEMID via ecm_generator::DecodeEcmClearFields if
//   system_id is 0.
//
// Example usage:
//   Asset asset;
//   DefaultEcmFields default_fields;
//   // Get a clear key ecm.
//   status_t status = ecm_generator::DecodeECMClearFields(ecm, &asset, &default_fields);
status_t DecodeECMClearFields(const sp<ABuffer>& ecm, Asset* asset,
        DefaultEcmFields* default_fields);

}  // namespace ecm_generator
}  // namespace clearkeycas
}  // namespace android
#endif  // CLEAR_KEY_ECM_GENERATOR_H_
