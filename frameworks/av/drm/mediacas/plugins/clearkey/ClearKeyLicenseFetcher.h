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

#ifndef CLEAR_KEY_LICENSE_FETCHER_H_
#define CLEAR_KEY_LICENSE_FETCHER_H_

#include "KeyFetcher.h"
#include "LicenseFetcher.h"

namespace android {
namespace clearkeycas {

class ClearKeyLicenseFetcher : public LicenseFetcher {
public:
    ClearKeyLicenseFetcher() {}
    virtual ~ClearKeyLicenseFetcher() {}

    virtual status_t Init(const char *input);

    virtual status_t FetchLicense(uint64_t asset_id, Asset* asset);

private:
    Asset asset_;
};

} // namespace clearkeycas
} // namespace android

#endif  // CLEAR_KEY_LICENSE_FETCHER_H_
