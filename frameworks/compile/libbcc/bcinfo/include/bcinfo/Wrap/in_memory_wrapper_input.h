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

// Defines utility allowing in-memory buffers for bitcode input wrapping.

#ifndef IN_MEMORY_WRAPPER_INPUT_H__
#define IN_MEMORY_WRAPPER_INPUT_H__

#include <stdio.h>

#include "bcinfo/Wrap/support_macros.h"
#include "bcinfo/Wrap/wrapper_input.h"

// Define a class to wrap named files.
class InMemoryWrapperInput : public WrapperInput {
 public:
  InMemoryWrapperInput(const char* buffer, size_t size);
  ~InMemoryWrapperInput();
  // Tries to read the requested number of bytes into the buffer. Returns the
  // actual number of bytes read.
  virtual size_t Read(uint8_t* buffer, size_t wanted);
  // Returns true if at end of buffer. Note: May return false
  // until Read is called, and returns 0.
  virtual bool AtEof();
  // Returns the size of the buffer (in bytes).
  virtual off_t Size();
  // Moves to the given offset within the buffer. Returns
  // false if unable to move to that position.
  virtual bool Seek(uint32_t pos);
 private:
  // The actual in-memory buffer
  const char* _buffer;
  // The position in the buffer
  size_t _pos;
  // True once eof has been encountered.
  bool _at_eof;
  // The size of the buffer.
  size_t _size;
 private:
  DISALLOW_CLASS_COPY_AND_ASSIGN(InMemoryWrapperInput);
};

#endif // IN_MEMORY_WRAPPER_INPUT_H__
