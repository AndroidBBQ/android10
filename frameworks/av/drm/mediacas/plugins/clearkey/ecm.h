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

// Data objects encapsulating the clear key Ecm (Entitlement Control
// Message) and related container messages. Deserialization and decryption
// are handled externally to reduce build-time dependencies.
//
// Simplified typical client-side use:
//   Asset asset; // from the AssetRegistry.
//   uint8[] ecm_buffer; // received over network, contains an EcmContainer.
//   EcmContainer ecm_container;
//   util::Status status = ecm_container.Parse(ecm_buffer);
//   status = ecm_container.descriptor(1).ecm().Decrypt(
//      ecm_container.descriptor(1).ecm().buffer(), asset_key);
//   string content_key;
//   if (ecm_container.descriptor(1).ecm().has_content_key()) {
//     content_key = ecm_container.descriptor(1).ecm().content_key();
//   }
//   // use |content_key| to decrypt content.
//
// Simplified typical server-side use:
//   EcmContainer container;
//   string encoded_ecm;
//   // Use the ecm_generator API to encode and encrypt an ECM from data fields.
//   util::Status status = ecm_generator::EncodeECM(..., &encoded_ecm);
//   // Use |encoded_ecm| to initialized the Ecm from this library.
//   Ecm ecm;
//   util::Status status = ecm.Parse(encoded_ecm);
//   EcmDescriptor descriptor(crypto_period_id, ecm);
//   status = container.Add(descriptor);
//   string serialized_container;
//   status = container.Marshall(&serialized_container);
//   // now |serialized_container| can be sent to the STB.
//
// Due to past overloading of the term "ECM" this library introduces some
// new terminology.
//
// Ecm: the 32-byte message sent from the head end to a packager that contains
// the asset_id, system_id, and content_key (clear).
//
// EcmDescriptor: contains an Ecm and an id (the crypto period id in the case
// of the BroadcastEncryptor). It contains no encrypted fields.
//
// EcmContainer: sent by the server in the video stream using the ECM pid.
// This contains 1 or 2 EcmDescriptors and a count. It contains no
// encrypted fields.
//
// The first EcmContainer sent by the server has only one EcmDescriptor. After
// the first crypto period change, an EcmContainer contains 2 EcmDescriptors.
// One has an odd id and one has an even id. The decrypted content keys from the
// Ecms in the EcmDescriptors are used by the Mpeg2 parser as odd and even
// scrambling keys. As the crypto period changes, the oldest EcmDescriptor is
// dropped from the EcmContainer and the new EcmDescriptor is added.
//
// These classes use a simplified protobuf model. For non-repeating fields,
// - has_foo() indicates whether the field is populated.
// - the accessor foo() returns either a value or a const reference.
// - a mutator sets the value.  Primitive types and strings use
//   set_foo(value) while for objects mutable_foo() returns a pointer.
//
// To prevent null references, objects (like the Asset contained in an Emm)
// are allocated as members and can be accessed via foo() even if they have
// not been populated. The caller must call has_foo() to make sure that the
// object is valid. Calling mutable_foo() to obtain a pointer causes has_foo()
// to return true.
//
// Repeated fields (like the EcmDescriptors contained in an EcmContainer) are
// handled differently.
// - foo_size() returns the number of instances.
// - the accessor foo(index) returns either a value or a const reference to
//   the instance at index. It is illegal to call with |index| >= the value
//   returned by foo_size(). |index| is checked with CHECK.
// - a mutator to change the value of the instance.  Primitive types and
//   strings use set_foo(index, value) while for objects mutable_foo(index)
//   returns a pointer. It is illegal to call with |index| >= the value
//   returned by foo_size(). |index| is checked with CHECK.
//
// Accessing a repeated field with an invalid index causes CHECK to fail.
// Be sure to call EcmContainer::decriptor_size() before calling descriptor()
// or mutable_descriptor()!
//
#ifndef CLEAR_KEY_ECM_H_
#define CLEAR_KEY_ECM_H_

#include <stddef.h>
#include <string>

#include "protos/license_protos.pb.h"

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <utils/Errors.h>

using namespace std;

namespace android {
namespace clearkeycas {

// Entitlement Control Message. It contains clear fields. The asset_id
// and system_id as well as the content_key are clear.
//
// This class is not thread-safe.
class Ecm {
public:
    // Wire size of ECM.
    static constexpr size_t kSizeBytes = 16 + 16; // clear fields + clear key

    // Creates an empty ECM which must be initialized via Parse().
    Ecm();

    ~Ecm();

    // Parses clear fields of Ecm serialized in |buffer_as_binary| and saves
    // a copy of |buffer_as_binary| for a future DecryptEcm call.
    // Returns:
    // - BAD_VALUE if |buffer_as_binary| is too small.
    // - CLEARKEY_STATUS_INVALIDASSETID via ecm_generator::DecodeEcmClearFields if
    //   asset_id is 0.
    // - CLEARKEY_STATUS_INVALIDSYSTEMID via ecm_generator::DecodeEcmClearFields if
    //   system_id is 0.
    // Postconditions:
    // - |asset_id_| and |system_id_| are populated with non-zero values.
    // - |buffer_| contains a copy of the serialized Ecm.
    status_t Parse(const sp<ABuffer>& buffer_as_binary);

    // Parses and decrypts Ecm serialized in |buffer_as_binary| using
    // |asset_from_emm|.asset_key().encryption_key(). It is not necessary to call
    // Parse() first.
    // Returns BAD_VALUE if |buffer_as_binary| is too small.
    // Returns CLEARKEY_STATUS_INVALIDASSETID via
    //   ecm_generator::DecodeEcmClearFields if asset_id is 0.
    // Returns CLEARKEY_STATUS_INVALIDSYSTEMID via
    //   ecm_generator::DecodeEcmClearFields if system_id is 0.
    // Returns CLEARKEY_STATUS_INVALID_PARAMETER if
    // - asset_id in |asset_from_emm| does not match asset_id in serialized Ecm.
    // Preconditions: |asset_from_emm| must contain asset_id and asset_key fields.
    // Postconditions: asset_id() and system_id() are populated with non-zero
    // values, content_key() is populated with the clear content key.
    status_t Decrypt(const sp<ABuffer>& buffer_as_binary,
            const Asset& asset_from_emm);

    // |buffer_| is a serialized copy of the Ecm used for later decryption or
    // for marshalling.
    inline bool has_buffer() const { return buffer_ != NULL; }
    const sp<ABuffer> buffer() const { return buffer_; }
    inline void set_buffer(const sp<ABuffer>& buffer) {
        buffer_ = ABuffer::CreateAsCopy(buffer->data(), buffer->size());
    }

    // |content_key| is the clear, encryption/decryption key generated by the server.
    inline bool has_content_key() const { return content_key_ != NULL; }
    inline void set_content_key(const sp<ABuffer>& value) {
        content_key_ = ABuffer::CreateAsCopy(value->data(), value->size());
    }
    inline const sp<ABuffer> content_key() const { return content_key_; }

    // |asset_id| from the server.
    inline bool has_asset_id() const { return asset_id_set_; }
    inline uint64_t asset_id() const { return asset_id_; }
    inline void set_asset_id(uint64_t value) {
        asset_id_ = value;
        asset_id_set_ = true;
    }

    // |system_id| from the server.
    inline bool has_system_id() const { return system_id_set_; }
    inline uint32_t system_id() const { return system_id_; }
    inline void set_system_id(uint32_t value) {
        system_id_ = value;
        system_id_set_ = true;
    }

private:
    uint64_t asset_id_;
    bool asset_id_set_;
    sp<ABuffer> buffer_;
    sp<ABuffer> content_key_;
    uint32_t system_id_;
    bool system_id_set_;
};

// Contains an Ecm and and Id.
// This class is not thread-safe.
class EcmDescriptor {
public:
    // Wire size of Id field.
    static constexpr size_t kIdSizeBytes = sizeof(uint16_t);
    // Wire size of EcmDescriptor.
    static constexpr size_t kSizeBytes = Ecm::kSizeBytes + kIdSizeBytes;

    // Client-side ctor. Populate from a buffer with Parse().
    EcmDescriptor();

    // Server-side ctor.
    // Args:
    // - |id| is the crypto period ID.
    // - |ecm| is an ECM which must have been intialized with Ecm::Parse().
    EcmDescriptor(uint16_t id, const Ecm& ecm);

    ~EcmDescriptor();

    // Parses EcmDescriptor and its contained Ecm which are serialized in the
    // binary string |buffer_as_binary|.
    // Returns
    // - BAD_VALUE if |buffer_as_binary| is too short to contain a
    //   serialized EcmDescriptor.
    // - Errors returned by Ecm::Parse.
    // Postconditions:
    // - id() is populated. Note that 0 is a legal value.
    // - the clear fields of the contained Ecm have been populated.
    status_t Parse(const sp<ABuffer>& buffer_as_binary);

    // |id| of the contained Ecm. Typically the crypto period id.
    inline bool has_id() const { return id_set_; }
    inline void set_id(uint16_t value) {
        id_ = value;
        id_set_ = true;
    }
    inline uint16_t id() const { return id_; }

    // The contained |ecm|.
    inline bool has_ecm() const { return ecm_set_; }
    inline Ecm* mutable_ecm() {
        ecm_set_ = true;
        return &ecm_;
    }
    inline const Ecm& ecm() const { return ecm_; }

private:
    Ecm ecm_;
    bool ecm_set_;
    uint16_t id_;
    bool id_set_;
};

// Contains a count and 1 or 2 EcmDescriptors. This is included in the video
// stream by the sender in the ECM pid.
// This class is not thread-safe.
class EcmContainer {
public:
    // Wire size of the count field.
    static constexpr size_t kCountSizeBytes = sizeof(uint16_t);
    // Minimum wire size assuming one EcmDescriptor.
    static constexpr size_t kMinimumSizeBytes =
            EcmDescriptor::kSizeBytes + kCountSizeBytes;
    static constexpr size_t kMinDescriptorCount = 1;
    static constexpr size_t kMaxDescriptorCount = 2;

    // Creates an empty EcmContainer which must be populated via Parse()
    // (client-side) or Add() (server-side).
    EcmContainer();

    ~EcmContainer();

    // Adds an EcmDescriptor for server-side applications.
    // If |count_| is 2, |descriptor| replaces the oldest EcmDescriptor.
    //
    // Returns:
    // - INTERNAL if the EcmContainer is in a bad state (count != 0, 1, or 2).
    // Postconditions:
    // - count() is within bounds (1 or 2).
    status_t Add(const EcmDescriptor& descriptor);

    // Parses EcmContainer and its contained EcmDescriptors which are serialized
    // in |buffer_as_binary|.
    // Returns
    // - BAD_VALUE if |buffer_as_binary| is too short to contain a
    //   serialized EcmDescriptor.
    // - ERROR_OUT_OF_RANGE if the count contained in the serialized EcmContainer
    //   is not 1 or 2.
    // - Errors returned by EcmDescriptor::Parse.
    // Postconditions:
    // - count() is within bounds (1 or 2) and.
    // - contained EcmDescriptor(s) parsed and populated.
    status_t Parse(const sp<ABuffer>& buffer_as_binary);

    inline bool has_count() const { return count_set_; }
    // Sets the |count| of contained EcmDecriptors. Illegal values are silently
    // ignored.
    inline void set_count(size_t count) {
        if (!CountLegal(count)) return;
        count_ = count;
        count_set_ = true;
    }
    // Number of contained EcmDecriptors. Only 1 and 2 are legal values.
    inline size_t count() const { return count_; }

    // Returns the number of allowable descriptors. This is redundant but is
    // provided for protobuf compatibility.
    inline size_t descriptor_size() const { return count_; }

    // Returns a pointer to the EcmDescriptor at |index| for valid index values,
    // otherwise calls CHECK and aborts. Always call descriptor_size() first!
    inline EcmDescriptor* mutable_descriptor(size_t index) {
        //CHECK(IndexValid(index));
        return &descriptor_[index];
    }

    // Returns a reference to the EcmDescriptor at |index| for valid index
    // values, otherwise calls CHECK and aborts. Call descriptor_size() first!
    inline const EcmDescriptor& descriptor(size_t index) const {
        //CHECK(IndexValid(index));
        return descriptor_[index];
    }

private:
    // Count value must be 1 or 2.
    inline bool CountLegal(size_t count) const {
        return count <= kMaxDescriptorCount && count >= kMinDescriptorCount;
    }
    // Index must be 0 or 1.
    inline bool IndexLegal(size_t index) const {
        return index < kMaxDescriptorCount;
    }
    // |index| is valid for this object: it is legal and < count_.
    inline bool IndexValid(size_t index) const {
        if (!IndexLegal(index)) return false;
        return index < count_;
    }
    size_t count_;
    bool count_set_;
    EcmDescriptor descriptor_[kMaxDescriptorCount];

    DISALLOW_EVIL_CONSTRUCTORS(EcmContainer);
};

}  // namespace clearkeycas
}  // namespace android

#endif  // CLEAR_KEY_ECM_H_
