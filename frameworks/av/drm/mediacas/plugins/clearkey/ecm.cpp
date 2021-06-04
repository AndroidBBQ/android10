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
#define LOG_TAG "ecm"

#include "ecm.h"
#include "ecm_generator.h"
#include "protos/license_protos.pb.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/Log.h>

namespace android {
namespace clearkeycas {

Ecm::Ecm()
    : asset_id_(0),
      asset_id_set_(false),
      system_id_(0),
      system_id_set_(false) {}

Ecm::~Ecm() {}

status_t Ecm::Parse(const sp<ABuffer>& buffer_as_binary) {
    if (buffer_as_binary->size() < kSizeBytes) {
        ALOGE("Short Ecm buffer: expected %zu, received %zu.",
                kSizeBytes, buffer_as_binary->size());
        return BAD_VALUE;
    }

    Asset asset;
    ecm_generator::DefaultEcmFields default_fields;
    status_t status = ecm_generator::DecodeECMClearFields(
            buffer_as_binary, &asset, &default_fields);
    if (status != OK) {
        ALOGE("DecodeECMClearFields failed with status %d", status);
        return status;
    }
    set_asset_id(asset.id());
    set_system_id(default_fields.system_id);

    // Save a copy of the buffer_as_binary for a future DecryptEcm call.
    set_buffer(buffer_as_binary);
    return OK;
}

status_t Ecm::Decrypt(
        const sp<ABuffer>& buffer_as_binary,
        const Asset& asset_from_emm) {
    // Invariant: asset has id. These are postconditions for Emm::Decrypt().
    CHECK(asset_from_emm.has_id());

    // DecodeEcm fills in |asset|.id() with the asset_id from the encoded Ecm.
    Asset asset(asset_from_emm);
    ecm_generator::DefaultEcmFields default_fields;
    sp<ABuffer> content_key;
    status_t status = ecm_generator::DecodeECM(
            buffer_as_binary, &asset, &content_key, &default_fields);
    if (status != OK) {
        ALOGE("DecodeECM failed with status %d", status);
        return status;
    }
    if (asset.id() != asset_from_emm.id()) {
        ALOGE("Asset_id from Emm (%llu) does not match asset_id from Ecm (%llu).",
                asset_from_emm.id(), asset.id());
        return CLEARKEY_STATUS_INVALID_PARAMETER;
    }
    set_asset_id(asset.id());
    set_system_id(default_fields.system_id);
    set_content_key(content_key);
    return status;
}

EcmDescriptor::EcmDescriptor() : ecm_set_(false), id_(0), id_set_(false) {}

EcmDescriptor::EcmDescriptor(uint16_t id, const Ecm& ecm)
: ecm_(ecm), ecm_set_(true), id_(id), id_set_(true) {}

EcmDescriptor::~EcmDescriptor() {}

status_t EcmDescriptor::Parse(const sp<ABuffer>& buffer_as_binary) {
    if (buffer_as_binary->size() < kSizeBytes) {
        ALOGE("Short EcmDescriptor buffer: expected %zu, received %zu.",
                kSizeBytes, buffer_as_binary->size());
        return BAD_VALUE;
    }
    sp<ABuffer> id_buffer = new ABuffer(buffer_as_binary->data(), kIdSizeBytes);
    const uint8_t *id_bytes = id_buffer->data();
    uint16_t id = (id_bytes[0] << 8) | id_bytes[1];
    set_id(id);

    // Unmarshall the contained Ecm.
    sp<ABuffer> ecm_buffer = new ABuffer(
            buffer_as_binary->data() + kIdSizeBytes, Ecm::kSizeBytes);
    status_t status = mutable_ecm()->Parse(ecm_buffer);
    if (status != OK) {
        return status;
    }
    return OK;
}

EcmContainer::EcmContainer() : count_(0), count_set_(false) {}

EcmContainer::~EcmContainer() {}

status_t EcmContainer::Add(const EcmDescriptor& descriptor) {
    switch (count_) {
    case 0:
        descriptor_[0] = descriptor;
        count_ = 1;
        break;
    case 1:
        descriptor_[1] = descriptor;
        count_ = 2;
        break;
    case 2:
        descriptor_[0] = descriptor_[1];
        descriptor_[1] = descriptor;
        break;
    default:
        ALOGE("Bad state.");
        return INVALID_OPERATION;
    }
    count_set_ = true;
    return OK;
}

status_t EcmContainer::Parse(const sp<ABuffer>& buffer_as_binary) {
    // EcmContainer can contain 1 or 2 EcmDescriptors so this is a check for
    // minimum size.
    if (buffer_as_binary->size() < kMinimumSizeBytes) {
        ALOGE("Short EcmContainer buffer: expected >= %zu, received %zu.",
                kMinimumSizeBytes, buffer_as_binary->size());
        return BAD_VALUE;
    }

    sp<ABuffer> count_buffer = new ABuffer(
            buffer_as_binary->data(), kCountSizeBytes);
    const uint8_t *count_bytes = count_buffer->data();
    size_t count = (count_bytes[0] << 8) | count_bytes[1];
    // Check that count is a legal value.
    if (!CountLegal(count)) {
        ALOGE("Invalid descriptor count: expected %zu <= count <= %zu, received %zu.",
                kMinDescriptorCount, kMaxDescriptorCount, count);
        return ERROR_OUT_OF_RANGE;
    }
    // If needed, check that buffer_as_binary can hold 2 EcmDescriptors.
    if (count > kMinDescriptorCount) {
        size_t expected_bytes =
                kCountSizeBytes + (count * EcmDescriptor::kSizeBytes);
        if (buffer_as_binary->size() < expected_bytes) {
            ALOGE("Short EcmContainer buffer: expected %zu, received %zu.",
                    expected_bytes, buffer_as_binary->size());
            return BAD_VALUE;
        }
    }
    set_count(count);
    // Unmarshall the contained EcmDescriptors.
    size_t offset = kCountSizeBytes;
    for (size_t i = 0; i < count_; ++i) {
        sp<ABuffer> descriptor_buffer = new ABuffer(
                buffer_as_binary->data() + offset, EcmDescriptor::kSizeBytes);
        status_t status = mutable_descriptor(i)->Parse(descriptor_buffer);
        if (status != OK) {
            return status;
        }
        offset += EcmDescriptor::kSizeBytes;
    }
    return OK;
}

}  // namespace clearkeycas
}  // namespace android
