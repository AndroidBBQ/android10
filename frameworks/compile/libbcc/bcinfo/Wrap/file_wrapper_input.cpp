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

#include "bcinfo/Wrap/file_wrapper_input.h"

FileWrapperInput::FileWrapperInput(const char* name) :
    _name(name), _at_eof(false), _size_found(false), _size(0) {
  _file = fopen(name, "rb");
  if (_file == nullptr) {
    fprintf(stderr, "Unable to open: %s\n", name);
    exit(1);
  }
}

FileWrapperInput::~FileWrapperInput() {
  fclose(_file);
}

size_t FileWrapperInput::Read(uint8_t* buffer, size_t wanted) {
  size_t found = fread((char*) buffer, 1, wanted, _file);
  if (feof(_file) || ferror(_file)) {
    _at_eof = true;
  }
  return found;
}

bool FileWrapperInput::AtEof() {
  return _at_eof;
}

off_t FileWrapperInput::Size() {
  if (_size_found) return _size;
  struct stat st;
  if (stat(_name, &st) == 0) {
    _size_found = true;
    _size = st.st_size;
    return _size;
  } else {
    fprintf(stderr, "Unable to compute file size: %s\n", _name);
    exit(1);
  }
  // NOT REACHABLE.
  return 0;
}

bool FileWrapperInput::Seek(uint32_t pos) {
  return fseek(_file, (long) pos, SEEK_SET) == 0; // NOLINT
}
