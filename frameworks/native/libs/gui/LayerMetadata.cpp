/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <android-base/stringprintf.h>
#include <binder/Parcel.h>
#include <gui/LayerMetadata.h>

using android::base::StringPrintf;

namespace android {

LayerMetadata::LayerMetadata() = default;

LayerMetadata::LayerMetadata(std::unordered_map<uint32_t, std::vector<uint8_t>> map)
      : mMap(std::move(map)) {}

LayerMetadata::LayerMetadata(const LayerMetadata& other) = default;

LayerMetadata::LayerMetadata(LayerMetadata&& other) = default;

bool LayerMetadata::merge(const LayerMetadata& other, bool eraseEmpty) {
    bool changed = false;
    for (const auto& entry : other.mMap) {
        auto it = mMap.find(entry.first);
        if (it != mMap.cend() && it->second != entry.second) {
            if (eraseEmpty && entry.second.empty()) {
                mMap.erase(it);
            } else {
                it->second = entry.second;
            }
            changed = true;
        } else if (it == mMap.cend() && !entry.second.empty()) {
            mMap[entry.first] = entry.second;
            changed = true;
        }
    }
    return changed;
}

status_t LayerMetadata::writeToParcel(Parcel* parcel) const {
    parcel->writeInt32(static_cast<int>(mMap.size()));
    status_t status = OK;
    for (const auto& entry : mMap) {
        status = parcel->writeUint32(entry.first);
        if (status != OK) {
            break;
        }
        status = parcel->writeByteVector(entry.second);
        if (status != OK) {
            break;
        }
    }
    return status;
}

status_t LayerMetadata::readFromParcel(const Parcel* parcel) {
    int size = parcel->readInt32();
    status_t status = OK;
    mMap.clear();
    for (int i = 0; i < size; ++i) {
        uint32_t key = parcel->readUint32();
        status = parcel->readByteVector(&mMap[key]);
        if (status != OK) {
            break;
        }
    }
    return status;
}

LayerMetadata& LayerMetadata::operator=(const LayerMetadata& other) {
    mMap = other.mMap;
    return *this;
}

LayerMetadata& LayerMetadata::operator=(LayerMetadata&& other) {
    mMap = std::move(other.mMap);
    return *this;
}

bool LayerMetadata::has(uint32_t key) const {
    return mMap.count(key);
}

int32_t LayerMetadata::getInt32(uint32_t key, int32_t fallback) const {
    if (!has(key)) return fallback;
    const std::vector<uint8_t>& data = mMap.at(key);
    if (data.size() < sizeof(uint32_t)) return fallback;
    Parcel p;
    p.setData(data.data(), data.size());
    return p.readInt32();
}

void LayerMetadata::setInt32(uint32_t key, int32_t value) {
    std::vector<uint8_t>& data = mMap[key];
    Parcel p;
    p.writeInt32(value);
    data.resize(p.dataSize());
    memcpy(data.data(), p.data(), p.dataSize());
}

std::string LayerMetadata::itemToString(uint32_t key, const char* separator) const {
    if (!has(key)) return std::string();
    switch (key) {
        case METADATA_OWNER_UID:
            return StringPrintf("ownerUID%s%d", separator, getInt32(key, 0));
        case METADATA_WINDOW_TYPE:
            return StringPrintf("windowType%s%d", separator, getInt32(key, 0));
        case METADATA_TASK_ID:
            return StringPrintf("taskId%s%d", separator, getInt32(key, 0));
        default:
            return StringPrintf("%d%s%dbytes", key, separator,
                                static_cast<int>(mMap.at(key).size()));
    }
}

} // namespace android
