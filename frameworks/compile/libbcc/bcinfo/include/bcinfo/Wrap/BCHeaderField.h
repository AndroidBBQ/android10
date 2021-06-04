/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LLVM_WRAP_BCHEADER_FIELD_H__
#define LLVM_WRAP_BCHEADER_FIELD_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Class representing a variable-size metadata field in the bitcode header.
// Also contains the list of known Tag IDs.
// Contains a pointer to the data but does not own the data, so it can be
// copied with the trivial copy constructor/assignment operator.

// The serialized format has 2 fixed subfields (ID and length) and the
// variable-length data subfield
class BCHeaderField {
 public:
  typedef enum {
    kInvalid = 0,
    kBitcodeHash = 1,
    kAndroidCompilerVersion = 0x4001,
    kAndroidOptimizationLevel = 0x4002
  } Tag;
  typedef uint16_t FixedSubfield;

  BCHeaderField(Tag ID, size_t len, uint8_t* data) :
      ID_(ID), len_(len), data_(data) {}
  size_t GetTotalSize() {
    // Round up to 4 byte alignment
    return (kTagLenSize + len_ + 3) & ~3;
  }

  bool Write(uint8_t* buf, size_t buf_len) {
    size_t fields_len = kTagLenSize + len_;
    size_t pad_len = (4 - (fields_len & 3)) & 3;
    // Ensure buffer is large enough and that length can be represented
    // in 16 bits
    const size_t max_uint16_t = 65535;
    if (buf_len < fields_len + pad_len ||
        len_ > max_uint16_t) return false;

    WriteFixedSubfield(static_cast<FixedSubfield>(ID_), buf);
    WriteFixedSubfield(static_cast<FixedSubfield>(len_),
                       buf + sizeof(FixedSubfield));
    memcpy(buf + kTagLenSize, data_, len_);
    // Pad out to 4 byte alignment
    if (pad_len) {
      memset(buf + fields_len, 0, pad_len);
    }
    return true;
  }

  bool Read(const uint8_t* buf, size_t buf_len) {
    if (buf_len < kTagLenSize) return false;
    FixedSubfield field;
    ReadFixedSubfield(&field, buf);
    ID_ = static_cast<Tag>(field);
    ReadFixedSubfield(&field, buf + sizeof(FixedSubfield));
    len_ = static_cast<size_t>(field);
    if (buf_len < kTagLenSize + len_) return false;
    memcpy(data_, buf + kTagLenSize, len_);
    return true;
  }

  void Print() {
    fprintf(stderr, "Field ID: %d, data length %d, total length %d\n",
            ID_, static_cast<int>(len_), static_cast<int>(GetTotalSize()));
    fprintf(stderr, "Data:");
    for (size_t i = 0; i < len_; i++) fprintf(stderr, "0x%x ", data_[i]);
    fprintf(stderr, "\n");
  }

  // Get the data size from a serialized field to allow allocation
  static size_t GetDataSizeFromSerialized(const uint8_t* buf) {
    FixedSubfield len;
    ReadFixedSubfield(&len, buf + sizeof(FixedSubfield));
    return len;
  }

  Tag getID() const {
    return ID_;
  }

  size_t getLen() const {
    return len_;
  }

 private:
 // Combined size of the fixed subfields
 const static size_t kTagLenSize = 2 * sizeof(FixedSubfield);
  static void WriteFixedSubfield(FixedSubfield value, uint8_t* buf) {
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
  }
  static void ReadFixedSubfield(FixedSubfield* value, const uint8_t* buf) {
    *value = buf[0] | buf[1] << 8;
  }
  Tag ID_;
  size_t len_;
  uint8_t *data_;
};

#endif  // LLVM_WRAP_BCHEADER_FIELD_H__
