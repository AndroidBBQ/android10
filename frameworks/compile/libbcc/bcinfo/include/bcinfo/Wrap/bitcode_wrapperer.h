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

// Define utility class to wrap/unwrap bitcode files. Does wrapping/unwrapping
// in such a way that the wrappered bitcode file is still a bitcode file.

#ifndef LLVM_WRAP_BITCODE_WRAPPERER_H__
#define LLVM_WRAP_BITCODE_WRAPPERER_H__

#include <stdint.h>
#include <stddef.h>
#include <vector>

#include "bcinfo/Wrap/support_macros.h"
#include "bcinfo/Wrap/BCHeaderField.h"
#include "bcinfo/Wrap/wrapper_input.h"
#include "bcinfo/Wrap/wrapper_output.h"

// The bitcode wrapper header is the following 7 fixed 4-byte fields:
//      1) 0B17C0DE - The magic number expected by llvm for wrapped bitcodes
//      2) Version # 0 - The current version of wrapped bitcode files
//      3) (raw) bitcode offset
//      4) (raw) bitcode size
//      5) Android header version
//      6) Android target API
//      7) PNaCl Bitcode version
//      plus 0 or more variable-length fields (consisting of ID, length, data)

// Initial buffer size. It is expanded if needed to hold large variable-size
// fields.
static const size_t kBitcodeWrappererBufferSize = 1024;

// Support class for outputting a wrapped bitcode file from a raw bitcode
// file (and optionally additional header fields), or for outputting a raw
// bitcode file from a wrapped one.
class BitcodeWrapperer {
 public:
  // Create a bitcode wrapperer using the following
  // input and output files.
  BitcodeWrapperer(WrapperInput* infile, WrapperOutput* outfile);

  // Returns true if the input file begins with a bitcode
  // wrapper magic number. As a side effect, _wrapper_ fields are set.
  bool IsInputBitcodeWrapper();

  // Returns true if the input file begins with a bitcode
  // file magic number.
  bool IsInputBitcodeFile();

  // Add a variable-length field to the header. The caller is responsible
  // for freeing the data pointed to by the BCHeaderField.
  void AddHeaderField(BCHeaderField* field);

  // Generate a wrapped bitcode file from the input bitcode file
  // and the current header data. Return true on success.
  bool GenerateWrappedBitcodeFile();

  // Unwrap the wrapped bitcode file, to the corresponding
  // outfile. Return true on success.
  bool GenerateRawBitcodeFile();

  // Print current wrapper header fields to stderr for debugging.
  void PrintWrapperHeader();

  uint32_t getAndroidHeaderVersion() {
    return android_header_version_;
  }

  uint32_t getAndroidTargetAPI() {
    return android_target_api_;
  }

  uint32_t getAndroidCompilerVersion() {
    return android_compiler_version_;
  }

  uint32_t getAndroidOptimizationLevel() {
    return android_optimization_level_;
  }

  ~BitcodeWrapperer();

 private:
  DISALLOW_CLASS_COPY_AND_ASSIGN(BitcodeWrapperer);

  // Refills the buffer with more bytes. Does this in a way
  // such that it is maximally filled.
  void FillBuffer();

  // Returns the number of bytes in infile.
  off_t GetInFileSize() {
    if (infile_ != nullptr) {
      return infile_->Size();
    } else {
      return 0;
    }
  }

  // Returns the offset of bitcode (i.e. the size of the wrapper header)
  // if the output file were to be written now.
  size_t BitcodeOffset();

  // Returns true if we can read a word. If necessary, fills the buffer
  // with enough characters so that there are at least a 32-bit value
  // in the buffer. Returns false if there isn't a 32-bit value
  // to read from the input file.
  bool CanReadWord();

  // Read a (32-bit) word from the input. Return true
  // if able to read the word.
  bool ReadWord(uint32_t& word);

  // Write a (32-bit) word to the output. Return true if successful
  bool WriteWord(uint32_t word);

  // Write all variable-sized header fields to the output. Return true
  // if successful.
  bool WriteVariableFields();

  // Parse the bitcode wrapper header in the infile, if any. Return true
  // if successful.
  bool ParseWrapperHeader();

  // Returns the i-th character in front of the cursor in the buffer.
  uint8_t BufferLookahead(int i) { return buffer_[cursor_ + i]; }

  // Returns how many unread bytes are in the buffer.
  size_t GetBufferUnreadBytes() { return buffer_size_ - cursor_; }


  // Backs up the read cursor to the beginning of the input buffer.
  void ResetCursor() {
    cursor_ = 0;
  }

  // Generates the header sequence for the wrapped bitcode being
  // generated.
  bool WriteBitcodeWrapperHeader();

  // Copies size bytes of infile to outfile, using the buffer.
  bool BufferCopyInToOut(uint32_t size);

  // Discards the old infile and replaces it with the given file.
  void ReplaceInFile(WrapperInput* new_infile);

  // Discards the old outfile and replaces it with the given file.
  void ReplaceOutFile(WrapperOutput* new_outfile);

  // Moves to the given position in the input file. Returns false
  // if unsuccessful.
  bool Seek(uint32_t pos);

  // Clear the buffer of all contents.
  void ClearBuffer();

  // The input file being processed. Can be either
  // a bitcode file, a wrappered bitcode file, or a secondary
  // file to be wrapped.
  WrapperInput* infile_;

  // The output file being generated. Can be either
  // a bitcode file, a wrappered bitcode file, or a secondary
  // unwrapped file.
  WrapperOutput* outfile_;

  // A buffer of bytes read from the input file.
  std::vector<uint8_t> buffer_;

  // The number of bytes that were read from the input file
  // into the buffer.
  size_t buffer_size_;

  // The index to the current read point within the buffer.
  size_t cursor_;

  // True when eof of input is reached.
  bool infile_at_eof_;

  // The 32-bit value defining the offset of the raw bitcode in the input file.
  uint32_t infile_bc_offset_;

  // The 32-bit value defining the generated offset of the wrapped bitcode.
  // This value changes as new fields are added with AddHeaderField
  uint32_t wrapper_bc_offset_;

  // The 32-bit value defining the size of the raw wrapped bitcode.
  uint32_t wrapper_bc_size_;

  // Android header version and target API
  uint32_t android_header_version_;
  uint32_t android_target_api_;
  uint32_t android_compiler_version_;
  uint32_t android_optimization_level_;

  // PNaCl bitcode version
  uint32_t pnacl_bc_version_;

  // Vector of variable header fields
  std::vector<BCHeaderField> header_fields_;
  // If any bufferdata from header fields is owned, it is stored here and
  // freed on destruction.
  std::vector<uint8_t*> variable_field_data_;

  // True if there was an error condition (e.g. the file is not bitcode)
  bool error_;
};

#endif  // LLVM_WRAP_BITCODE_WRAPPERER_H__
