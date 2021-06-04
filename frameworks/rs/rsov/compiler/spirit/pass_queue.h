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

#ifndef RSOV_COMPILER_SPIRIT_PASS_QUEUE_H
#define RSOV_COMPILER_SPIRIT_PASS_QUEUE_H

#include "module.h"
#include "pass.h"
#include "stl_util.h"

#include <stdint.h>

#include <memory>
#include <set>
#include <vector>

namespace android {
namespace spirit {

// A FIFO of passes. Passes are appended to the end of the FIFO and run in the
// first-in first-out order. Once appended to a pass queue, Passes are owned by
// the queue.
class PassQueue {
public:
  PassQueue() : mPassesDeleter(mPassSet) {}

  // Appends a pass to the end of the queue
  bool append(Pass *pass);

  // Runs all passes in the queue in the first-in first-out order on a Module.
  // Returns the result Module after all passes have run.
  // If argument error is not null, sets the error code. On a successful run,
  // error code is set to zero.
  Module *run(Module *module, int *error = nullptr);

  // Deserialize the input vector of words into a Module, and runs all passes in
  // the queue in the first-in first-out order on the Module.
  // for a serialized Module.
  // After all the passes have run, returns the words from the serialized result
  // Module.
  // If argument error is not null, sets the error code. On a successful run,
  // error code is set to zero.
  std::vector<uint32_t> run(const std::vector<uint32_t> &spirvWords,
                            int *error = nullptr);

  // Runs all passes in the queue in the first-in first-out order on a Module.
  // After all the passes have run, serializes the result Module, and returns
  // the words as a vector.
  // If argument error is not null, sets the error code. On a successful run,
  // error code is set to zero.
  std::vector<uint32_t> runAndSerialize(Module *module, int *error = nullptr);

private:
  std::vector<Pass *> mPasses;
  // Keep all passes in a set so that we can delete them on destruction without
  // worrying about duplicates
  std::set<Pass *> mPassSet;
  ContainerDeleter<std::set<Pass *>> mPassesDeleter;
};

} // spirit
} // android

#endif // RSOV_COMPILER_SPIRIT_PASS_QUEUE_H
