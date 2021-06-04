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

#include "pass_queue.h"

#include "module.h"

namespace android {
namespace spirit {

bool PassQueue::append(Pass *pass) {
  mPasses.push_back(pass);
  mPassSet.insert(pass);
  return true;
}

Module *PassQueue::run(Module *module, int *error) {
  if (mPasses.empty()) {
    return module;
  }

  // A unique ptr to keep intermediate modules from leaking
  std::unique_ptr<Module> tempModule;

  for (auto pass : mPasses) {
    int intermediateError = 0;
    Module* newModule = pass->run(module, &intermediateError);
    // Some passes modify the input module in place, while others create a new
    // module. Release memory only when it is necessary.
    if (newModule != module) {
      tempModule.reset(newModule);
    }
    module = newModule;
    if (intermediateError) {
      if (error) {
        *error = intermediateError;
      }
      return nullptr;
    }
    if (!module || !module->resolveIds()) {
      if (error) {
        *error = -1;
      }
      return nullptr;
    }
  }

  if (tempModule == nullptr) {
    return module;
  }

  return tempModule.release();
}

std::vector<uint32_t> PassQueue::run(const std::vector<uint32_t> &spirvWords,
                                     int *error) {
  if (mPasses.empty()) {
    return spirvWords;
  }

  Module *module = Deserialize<Module>(spirvWords);
  if (!module || !module->resolveIds()) {
    return std::vector<uint32_t>();
  }

  return runAndSerialize(module, error);
}

std::vector<uint32_t> PassQueue::runAndSerialize(Module *module, int *error) {
  const int n = mPasses.size();
  if (n < 1) {
    return Serialize<Module>(module);
  }

  // A unique ptr to keep intermediate modules from leaking
  std::unique_ptr<Module> tempModule;

  for (int i = 0; i < n - 1; i++) {
    int intermediateError = 0;
    Module *newModule = mPasses[i]->run(module, &intermediateError);
    // Some passes modify the input module in place, while others create a new
    // module. Release memory only when it is necessary.
    if (newModule != module) {
      tempModule.reset(newModule);
    }
    module = newModule;
    if (intermediateError) {
      if (error) {
        *error = intermediateError;
      }
      return std::vector<uint32_t>();
    }
    if (!module || !module->resolveIds()) {
      if (error) {
        *error = -1;
      }
      return std::vector<uint32_t>();
    }
  }
  return mPasses[n - 1]->runAndSerialize(module, error);
}

} // namespace spirit
} // namespace android
