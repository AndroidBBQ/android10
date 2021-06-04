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

#include "bcinfo/Wrap/bitcode_wrapperer.h"

#define LOG_TAG "bcinfo"

#include <stdio.h>
#include <sys/stat.h>

#include <log/log.h>

using std::vector;

// The number of bytes in a 32 bit integer.
static const uint32_t kWordSize = 4;

// Number of LLVM-defined fixed fields in the header.
static const uint32_t kLLVMFields = 4;

// Total number of fixed fields in the header.
static const uint32_t kFixedFields = 7;

// The magic number that must exist for bitcode wrappers.
static const uint32_t kWrapperMagicNumber = 0x0B17C0DE;

// The version number associated with a wrapper file.
// Note: llvm currently only allows the value 0. When this changes,
// we should consider making this a command line option.
static const uint32_t kLLVMVersionNumber = 0;

// Fields defined by Android bitcode header.
static const uint32_t kAndroidHeaderVersion = 0;
static const uint32_t kAndroidTargetAPI = 0;
static const uint32_t kAndroidDefaultCompilerVersion = 0;
static const uint32_t kAndroidDefaultOptimizationLevel = 3;

// PNaCl bitcode version number.
static const uint32_t kPnaclBitcodeVersion = 0;

// Max size for variable fields. Currently only used for writing them
// out to files (the parsing works for arbitrary sizes).
static const size_t kMaxVariableFieldSize = 256;

BitcodeWrapperer::BitcodeWrapperer(WrapperInput* infile, WrapperOutput* outfile)
    : infile_(infile),
      outfile_(outfile),
      buffer_size_(0),
      cursor_(0),
      infile_at_eof_(false),
      infile_bc_offset_(0),
      wrapper_bc_offset_(0),
      wrapper_bc_size_(0),
      android_header_version_(kAndroidHeaderVersion),
      android_target_api_(kAndroidTargetAPI),
      android_compiler_version_(kAndroidDefaultCompilerVersion),
      android_optimization_level_(kAndroidDefaultOptimizationLevel),
      pnacl_bc_version_(0),
      error_(false) {
  buffer_.resize(kBitcodeWrappererBufferSize);
  if (IsInputBitcodeWrapper()) {
    ParseWrapperHeader();
  } else if (IsInputBitcodeFile()) {
    wrapper_bc_offset_ = kWordSize * kFixedFields;
    wrapper_bc_size_ = GetInFileSize();
  } else {
    ALOGE("Error: input file is not a bitcode file.\n");
    error_ = true;
  }
}

BitcodeWrapperer::~BitcodeWrapperer() {
  for(size_t i = 0; i < variable_field_data_.size(); i++) {
    delete [] variable_field_data_[i];
  }
}


void BitcodeWrapperer::ClearBuffer() {
  buffer_size_ = 0;
  cursor_ = 0;
  infile_at_eof_ = false;
}

bool BitcodeWrapperer::Seek(uint32_t pos) {
  if (infile_ != nullptr && infile_->Seek(pos)) {
    ClearBuffer();
    return true;
  }
  return false;
}

bool BitcodeWrapperer::CanReadWord() {
  if (GetBufferUnreadBytes() < kWordSize) {
    FillBuffer();
    return GetBufferUnreadBytes() >= kWordSize;
  } else {
    return true;
  }
}

void BitcodeWrapperer::FillBuffer() {
  if (cursor_ > 0) {
    // Before filling, move any remaining bytes to the
    // front of the buffer. This allows us to assume
    // that after the call to FillBuffer, readable
    // text is contiguous.
    if (cursor_ < buffer_size_) {
      size_t i = 0;
      while (cursor_ < buffer_size_) {
        buffer_[i++] = buffer_[cursor_++];
      }
      cursor_ = 0;
      buffer_size_ = i;
    }
  } else {
    // Assume the buffer contents have been used,
    // and we want to completely refill it.
    buffer_size_ = 0;
  }

  // If we don't have an input, we can't refill the buffer at all.
  if (infile_ == nullptr) {
    return;
  }

  // Now fill in remaining space.
  size_t needed = buffer_.size() - buffer_size_;

  while (buffer_.size() > buffer_size_) {
    int actually_read = infile_->Read(&buffer_[buffer_size_], needed);
    if (infile_->AtEof()) {
      infile_at_eof_ = true;
    }
    if (actually_read) {
      buffer_size_ += actually_read;
      needed -= actually_read;
    } else if (infile_at_eof_) {
      break;
    }
  }
}

bool BitcodeWrapperer::ReadWord(uint32_t& word) {
  if (!CanReadWord()) return false;
  word = (((uint32_t) BufferLookahead(0)) << 0)
      | (((uint32_t) BufferLookahead(1)) << 8)
      | (((uint32_t) BufferLookahead(2)) << 16)
      | (((uint32_t) BufferLookahead(3)) << 24);
  cursor_ += kWordSize;
  return true;
}

bool BitcodeWrapperer::WriteWord(uint32_t value) {
  uint8_t buffer[kWordSize];
  buffer[3] = (value >> 24) & 0xFF;
  buffer[2] = (value >> 16) & 0xFF;
  buffer[1] = (value >> 8)  & 0xFF;
  buffer[0] = (value >> 0)  & 0xFF;
  return outfile_->Write(buffer, kWordSize);
}

bool BitcodeWrapperer::WriteVariableFields() {
  // This buffer may have to be bigger if we start using the fields
  // for larger things.
  uint8_t buffer[kMaxVariableFieldSize];
  for (vector<BCHeaderField>::iterator it = header_fields_.begin();
       it != header_fields_.end(); ++it) {
    if (!it->Write(buffer, kMaxVariableFieldSize) ||
        !outfile_->Write(buffer, it->GetTotalSize())) {
      return false;
    }
  }
  return true;
}

bool BitcodeWrapperer::ParseWrapperHeader() {
  // Make sure LLVM-defined fields have been parsed
  if (!IsInputBitcodeWrapper()) return false;
  // Check the android/pnacl fields
  if (!ReadWord(android_header_version_) ||
      !ReadWord(android_target_api_) || !ReadWord(pnacl_bc_version_)) {
    ALOGW("Error: file not long enough to contain header\n");
    return false;
  }
  if (pnacl_bc_version_ != kPnaclBitcodeVersion) {
    ALOGW("Error: bad PNaCl Bitcode version\n");
    return false;
  }
  int field_data_total = wrapper_bc_offset_ - kWordSize * kFixedFields;
  if (field_data_total > 0) {
    // Read in the variable fields. We need to allocate space for the data.
    int field_data_read = 0;

    while (field_data_read < field_data_total) {
      FillBuffer();
      size_t buffer_needed = BCHeaderField::GetDataSizeFromSerialized(
          &buffer_[cursor_]);
      if (buffer_needed > buffer_.size()) {
        buffer_.resize(buffer_needed +
                       sizeof(BCHeaderField::FixedSubfield) * 2);
        FillBuffer();
      }
      variable_field_data_.push_back(new uint8_t[buffer_needed]);

      BCHeaderField field(BCHeaderField::kInvalid, 0,
                          variable_field_data_.back());
      field.Read(&buffer_[cursor_], buffer_size_);
      header_fields_.push_back(field);
      size_t field_size = field.GetTotalSize();
      cursor_ += field_size;
      field_data_read += field_size;
      if (field_data_read > field_data_total) {
        // We read too much data, the header is corrupted
        ALOGE("Error: raw bitcode offset inconsistent with "
              "variable field data\n");
        return false;
      }

      struct IntFieldHelper {
        BCHeaderField::FixedSubfield tag;
        uint16_t len;
        uint32_t val;
      };
      IntFieldHelper tempIntField;

      switch (field.getID()) {
        case BCHeaderField::kAndroidCompilerVersion:
          if (field.Write((uint8_t*)&tempIntField,
                          sizeof(tempIntField))) {
            android_compiler_version_ = tempIntField.val;
          }
          break;
        case BCHeaderField::kAndroidOptimizationLevel:
          if (field.Write((uint8_t*)&tempIntField,
                          sizeof(tempIntField))) {
            android_optimization_level_ = tempIntField.val;
          }
          break;
        default:
          // Ignore other field types for now
          break;
      }
    }
    Seek(0);
  }
  return true;
}

bool BitcodeWrapperer::IsInputBitcodeWrapper() {
  ResetCursor();
  // First make sure that there are enough words (LLVM header)
  // to peek at.
  if (GetBufferUnreadBytes() < kLLVMFields * kWordSize) {
    FillBuffer();
    if (GetBufferUnreadBytes() < kLLVMFields * kWordSize) return false;
  }

  // Now make sure the magic number is right.
  uint32_t first_word;
  if ((!ReadWord(first_word)) ||
      (kWrapperMagicNumber != first_word)) return false;

  // Make sure the version is right.
  uint32_t second_word;
  if ((!ReadWord(second_word)) ||
      (kLLVMVersionNumber != second_word)) return false;

  // Make sure that the offset and size (for llvm) is defined.
  uint32_t bc_offset;
  uint32_t bc_size;
  if (ReadWord(bc_offset) &&
      ReadWord(bc_size)) {
    // Before returning, save the extracted values.
    wrapper_bc_offset_ = bc_offset;
    infile_bc_offset_ = bc_offset;
    wrapper_bc_size_ = bc_size;
    return true;
  }
  // If reached, unable to read wrapped header.
  return false;
}

bool BitcodeWrapperer::IsInputBitcodeFile() {
  ResetCursor();
  // First make sure that there are four bytes to peek at.
  if (GetBufferUnreadBytes() < kWordSize) {
    FillBuffer();
    if (GetBufferUnreadBytes() < kWordSize) return false;
  }
  // If reached, Check if first 4 bytes match bitcode
  // file magic number.
  return (BufferLookahead(0) == 'B') &&
      (BufferLookahead(1) == 'C') &&
      (BufferLookahead(2) == 0xc0) &&
      (BufferLookahead(3) == 0xde);
}

bool BitcodeWrapperer::BufferCopyInToOut(uint32_t size) {
  while (size > 0) {
    // Be sure buffer is non-empty before writing.
    if (0 == buffer_size_) {
      FillBuffer();
      if (0 == buffer_size_) {
        return false;
      }
    }
    // copy the buffer to the output file.
    size_t block = (buffer_size_ < size) ? buffer_size_ : size;
    if (!outfile_->Write(&buffer_[cursor_], block)) return false;
    size -= block;
    buffer_size_ = 0;
  }
  // Be sure that there isn't more bytes on the input stream.
  FillBuffer();
  return buffer_size_ == 0;
}

void BitcodeWrapperer::AddHeaderField(BCHeaderField* field) {
  header_fields_.push_back(*field);
  wrapper_bc_offset_ += field->GetTotalSize();
}

bool BitcodeWrapperer::WriteBitcodeWrapperHeader() {
  return
      // Note: This writes out the 4 word header required by llvm wrapped
      // bitcode.
      WriteWord(kWrapperMagicNumber) &&
      WriteWord(kLLVMVersionNumber) &&
      WriteWord(wrapper_bc_offset_) &&
      WriteWord(wrapper_bc_size_) &&
      // 2 fixed fields defined by Android
      WriteWord(android_header_version_) &&
      WriteWord(android_target_api_) &&
      // PNaClBitcode version
      WriteWord(kPnaclBitcodeVersion) &&
      // Common variable-length fields
      WriteVariableFields();
}

void BitcodeWrapperer::PrintWrapperHeader() {
  if (error_) {
    fprintf(stderr, "Error condition exists: the following"
            "data may not be reliable\n");
  }
  fprintf(stderr, "Wrapper magic:\t\t%x\n", kWrapperMagicNumber);
  fprintf(stderr, "LLVM Bitcode version:\t%d\n", kLLVMVersionNumber);
  fprintf(stderr, "Raw bitcode offset:\t%d\n", wrapper_bc_offset_);
  fprintf(stderr, "Raw bitcode size:\t%d\n", wrapper_bc_size_);
  fprintf(stderr, "Android header version:\t%d\n", android_header_version_);
  fprintf(stderr, "Android target API:\t%d\n", android_target_api_);
  fprintf(stderr, "PNaCl bitcode version:\t%d\n", kPnaclBitcodeVersion);
  for (size_t i = 0; i < header_fields_.size(); i++) header_fields_[i].Print();
}

bool BitcodeWrapperer::GenerateWrappedBitcodeFile() {
  if (!error_ &&
      WriteBitcodeWrapperHeader() &&
      Seek(infile_bc_offset_) &&
      BufferCopyInToOut(wrapper_bc_size_)) {
    off_t dangling = wrapper_bc_size_ & 3;
    if (dangling) {
      return outfile_->Write((const uint8_t*) "\0\0\0\0", 4 - dangling);
    }
    return true;
  }
  return false;
}

bool BitcodeWrapperer::GenerateRawBitcodeFile() {
  return !error_ && Seek(infile_bc_offset_) &&
      BufferCopyInToOut(wrapper_bc_size_);
}
