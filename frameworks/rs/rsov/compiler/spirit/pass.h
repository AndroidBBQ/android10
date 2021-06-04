/*
 * Copyright 2017, The Android Open Source Project
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

#ifndef RSOV_COMPILER_SPIRIT_PASS_H
#define RSOV_COMPILER_SPIRIT_PASS_H

#include <stdint.h>

#include <vector>

namespace android {
namespace spirit {

class Module;

// The base class for a pass, either an analysis or a transformation of a
// Module. An instanace of a derived class can be added to a PassQueue and
// applied to a Module, and produce a result Module with other passes.
class Pass {
public:
  virtual ~Pass() {}

  // Runs the pass  on the input module and returns the result module.
  // If argument error is not null, set the error code. On a successful run,
  // error code is set to zero.
  virtual Module *run(Module *module, int *error);

  // Runs the pass  on the input module, serializes the result module, and
  // returns the words as a vector.
  // If argument error is not null, set the error code. On a successful run,
  // error code is set to zero.
  virtual std::vector<uint32_t> runAndSerialize(Module *module, int *error);
};

} // namespace spirit
} // namespace android

#endif // RSOV_COMPILER_SPIRIT_PASS_H
