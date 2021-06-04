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

// Defines utility allowing files for bitcode output wrapping.

#ifndef FILE_WRAPPER_OUTPUT_H__
#define FILE_WRAPPER_OUTPUT_H__

#include <stdio.h>

#include "bcinfo/Wrap/support_macros.h"
#include "bcinfo/Wrap/wrapper_output.h"

// Define a class to wrap named files. */
class FileWrapperOutput : public WrapperOutput {
 public:
  explicit FileWrapperOutput(const char* name);
  ~FileWrapperOutput();
  // Writes a single byte, returning false if unable to write.
  virtual bool Write(uint8_t byte);
  // Writes the specified number of bytes in the buffer to
  // output. Returns false if unable to write.
  virtual bool Write(const uint8_t* buffer, size_t buffer_size);
 private:
  // The name of the file
  const char* _name;
  // The corresponding (opened) file.
  FILE* _file;
 private:
  DISALLOW_CLASS_COPY_AND_ASSIGN(FileWrapperOutput);
};
#endif  // FILE_WRAPPER_OUTPUT_H__
