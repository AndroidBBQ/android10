/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRAMEWORKS_ML_NN_HASHTABLE_LOOKUP_H
#define FRAMEWORKS_ML_NN_HASHTABLE_LOOKUP_H

#include "HalOperation.h"

#include <vector>

namespace android {
namespace nn {

struct RunTimeOperandInfo;

class HashtableLookup {
 public:
  HashtableLookup(
      const Operation &operation,
      std::vector<RunTimeOperandInfo> &operands);

  bool Eval();

  static constexpr int kLookupTensor = 0;
  static constexpr int kKeyTensor = 1;
  static constexpr int kValueTensor = 2;

  static constexpr int kOutputTensor = 0;
  static constexpr int kHitsTensor = 1;

 private:
  const RunTimeOperandInfo *lookup_;
  const RunTimeOperandInfo *key_;
  const RunTimeOperandInfo *value_;

  RunTimeOperandInfo *output_;
  RunTimeOperandInfo *hits_;
};

}  // namespace nn
}  // namespace android

#endif  // FRAMEWORKS_ML_NN_HASHTABLE_LOOKUP_H
