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

#include "bcinfo/Wrap/wrapper_output.h"

bool WrapperOutput::Write(const uint8_t* buffer, size_t buffer_size) {
  // Default implementation that uses the byte write routine.
  for (size_t i = 0; i < buffer_size; ++i) {
    if (!Write(buffer[i])) return false;
  }
  return true;
}
