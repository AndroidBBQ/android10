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

// Defines a generic interface to a file/memory region that
// contains a generated wrapped bitcode file, bitcode file,
// or data file.

#ifndef LLVM_WRAP_WRAPPER_OUTPUT_H__
#define LLVM_WRAP_WRAPPER_OUTPUT_H__

#include <stdint.h>
#include <stddef.h>

#include "bcinfo/Wrap/support_macros.h"

// The following is a generic interface to a file/memory region
// that contains a generated bitcode file, wrapped bitcode file,
// or a data file.
class WrapperOutput {
 public:
  WrapperOutput() {}
  virtual ~WrapperOutput() {}
  // Writes a single byte, returning false if unable to write.
  virtual bool Write(uint8_t byte) = 0;
  // Writes the specified number of bytes in the buffer to
  // output. Returns false if unable to write.
  virtual bool Write(const uint8_t* buffer, size_t buffer_size);
 private:
  DISALLOW_CLASS_COPY_AND_ASSIGN(WrapperOutput);
};

#endif  // LLVM_WRAP_WRAPPER_OUTPUT_H__
