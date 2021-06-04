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

#include <stdlib.h>

#include "bcinfo/Wrap/file_wrapper_output.h"

FileWrapperOutput::FileWrapperOutput(const char* name)
    : _name(name) {
  _file = fopen(name, "wb");
  if (nullptr == _file) {
    fprintf(stderr, "Unable to open: %s\n", name);
    exit(1);
  }
}

FileWrapperOutput::~FileWrapperOutput() {
  fclose(_file);
}

bool FileWrapperOutput::Write(uint8_t byte) {
  return EOF != fputc(byte, _file);
}

bool FileWrapperOutput::Write(const uint8_t* buffer, size_t buffer_size) {
  if (!buffer) {
    return false;
  }

  if (buffer_size > 0) {
    return buffer_size == fwrite(buffer, 1, buffer_size, _file);
  } else {
    return true;
  }
}
