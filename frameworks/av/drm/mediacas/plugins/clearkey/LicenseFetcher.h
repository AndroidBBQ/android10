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

#ifndef LICENSE_FETCHER_H_
#define LICENSE_FETCHER_H_

#include "protos/license_protos.pb.h"
#include <media/stagefright/foundation/ABase.h>

namespace android {
namespace clearkeycas {

// Interface for classes which request a license.
class LicenseFetcher {
public:
    LicenseFetcher() {}
    virtual ~LicenseFetcher() {}

    // Initializes resources set in subclass-specific calls. This must be called
    // before threads are started.
    virtual status_t Init(const char *input) = 0;

    // Fetches license based on |asset_id|.  On return, |asset| contains the
    // decrypted asset_key needed to decrypt content keys.
    // |asset| must be non-null.
    virtual status_t FetchLicense(
            uint64_t asset_id, clearkeycas::Asset* asset) = 0;

private:
    DISALLOW_EVIL_CONSTRUCTORS(LicenseFetcher);
};

}  // namespace clearkeycas
}  // namespace android

#endif  // LICENSE_FETCHER_H_
