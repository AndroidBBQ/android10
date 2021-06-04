/*
 * Copyright 2019 The Android Open Source Project
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

#pragma once

#include <binder/Parcelable.h>

#include <unordered_map>

namespace android {

enum { METADATA_OWNER_UID = 1, METADATA_WINDOW_TYPE = 2, METADATA_TASK_ID = 3 };

struct LayerMetadata : public Parcelable {
    std::unordered_map<uint32_t, std::vector<uint8_t>> mMap;

    LayerMetadata();
    LayerMetadata(const LayerMetadata& other);
    LayerMetadata(LayerMetadata&& other);
    explicit LayerMetadata(std::unordered_map<uint32_t, std::vector<uint8_t>> map);
    LayerMetadata& operator=(const LayerMetadata& other);
    LayerMetadata& operator=(LayerMetadata&& other);

    // Merges other into this LayerMetadata. If eraseEmpty is true, any entries in
    // in this whose keys are paired with empty values in other will be erased.
    bool merge(const LayerMetadata& other, bool eraseEmpty = false);

    status_t writeToParcel(Parcel* parcel) const override;
    status_t readFromParcel(const Parcel* parcel) override;

    bool has(uint32_t key) const;
    int32_t getInt32(uint32_t key, int32_t fallback) const;
    void setInt32(uint32_t key, int32_t value);

    std::string itemToString(uint32_t key, const char* separator) const;
};

} // namespace android
