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

// Define a generic interface to a file/memory region that contains
// a bitcode file, a wrapped bitcode file, or a data file to wrap.

#ifndef LLVM_WRAP_WRAPPER_INPUT_H__
#define LLVM_WRAP_WRAPPER_INPUT_H__

#include <stdint.h>
#include <sys/types.h>

#include "support_macros.h"

// The following is a generic interface to a file/memory region that contains
// a bitcode file, a wrapped bitcode file, or data file to wrap.
class WrapperInput {
 public:
  WrapperInput() {}
  virtual ~WrapperInput() {}
  // Tries to read the requested number of bytes into the buffer. Returns the
  // actual number of bytes read.
  virtual size_t Read(uint8_t* buffer, size_t wanted) = 0;
  // Returns true if at end of input. Note: May return false until
  // Read is called, and returns 0.
  virtual bool AtEof() = 0;
  // Returns the size of the input (in bytes).
  virtual off_t Size() = 0;
  // Moves to the given offset within the input region. Returns false
  // if unable to move to that position.
  virtual bool Seek(uint32_t pos) = 0;
 private:
  DISALLOW_CLASS_COPY_AND_ASSIGN(WrapperInput);
};

#endif  // LLVM_WRAP_WRAPPER_INPUT_H__
