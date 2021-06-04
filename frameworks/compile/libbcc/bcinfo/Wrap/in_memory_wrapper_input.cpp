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

#include <sys/stat.h>
#include <stdlib.h>

#include "bcinfo/Wrap/in_memory_wrapper_input.h"

InMemoryWrapperInput::InMemoryWrapperInput(const char* buffer, size_t size) :
    _buffer(buffer), _pos(0), _size(size) {
}

InMemoryWrapperInput::~InMemoryWrapperInput() {
}

size_t InMemoryWrapperInput::Read(uint8_t* buffer, size_t wanted) {
  size_t found = 0;

  if (!buffer) {
    return 0;
  }

  while (found < wanted) {
    if (_pos >= _size) {
      return found;
    }
    buffer[found++] = _buffer[_pos++];
  }
  return found;
}

bool InMemoryWrapperInput::AtEof() {
  return (_pos >= _size);
}

off_t InMemoryWrapperInput::Size() {
  return _size;
}

bool InMemoryWrapperInput::Seek(uint32_t pos) {
  if (pos < _size) {
    _pos = pos;
    return true;
  } else {
    return false;
  }
}
