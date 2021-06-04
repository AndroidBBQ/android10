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

#ifndef CLEAR_KEY_FETCHER_H_
#define CLEAR_KEY_FETCHER_H_

#include <vector>

#include "protos/license_protos.pb.h"

#include <media/stagefright/foundation/ABase.h>
#include "KeyFetcher.h"

namespace android {
namespace clearkeycas {

class LicenseFetcher;

class ClearKeyFetcher : public KeyFetcher {
public:
    // ClearKeyFetcher takes ownership of |license_fetcher|.
    explicit ClearKeyFetcher(
            std::unique_ptr<LicenseFetcher> license_fetcher);

    virtual ~ClearKeyFetcher();

    // Initializes the fetcher. Must be called before ObtainKey.
    status_t Init() override;

    // Obtains the |asset_id| and |keys| from the Ecm contained in |ecm|.
    // Returns
    // - errors returned by EcmContainer::Parse.
    // - errors returned by ClassicLicenseFetcher::FetchLicense.
    // - errors returned by Ecm::Decrypt.
    // |asset_id| and |keys| are owned by the caller and cannot be null.
    // Init() must have been called.
    status_t ObtainKey(const sp<ABuffer>& ecm, uint64_t* asset_id,
            std::vector<KeyInfo>* keys) override;

private:
    clearkeycas::Asset asset_;
    bool initialized_;
    std::unique_ptr<LicenseFetcher> license_fetcher_;

    DISALLOW_EVIL_CONSTRUCTORS(ClearKeyFetcher);
};

} // namespace clearkeycas
} // namespace android

#endif  // CLEAR_KEY_FETCHER_H_
