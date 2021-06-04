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

#include "HashtableLookup.h"

#include "CpuExecutor.h"
#include "Operations.h"

#include "Tracing.h"

namespace android {
namespace nn {

namespace {

int greater(const void* a, const void* b) {
  return *static_cast<const int*>(a) - *static_cast<const int*>(b);
}

}  // anonymous namespace

HashtableLookup::HashtableLookup(const Operation& operation,
                                 std::vector<RunTimeOperandInfo>& operands) {
  lookup_ = GetInput(operation, operands, kLookupTensor);
  key_ = GetInput(operation, operands, kKeyTensor);
  value_ = GetInput(operation, operands, kValueTensor);

  output_ = GetOutput(operation, operands, kOutputTensor);
  hits_ = GetOutput(operation, operands, kHitsTensor);
}

bool HashtableLookup::Eval() {
  NNTRACE_COMP("HashtableLookup::Eval");
  const int num_rows = value_->shape().dimensions[0];
  const int row_bytes = nonExtensionOperandSizeOfData(value_->type, value_->dimensions) / num_rows;
  void* pointer = nullptr;

  for (int i = 0; i < static_cast<int>(lookup_->shape().dimensions[0]); i++) {
    int idx = -1;
    pointer = bsearch(lookup_->buffer + sizeof(int) * i, key_->buffer,
                      num_rows, sizeof(int), greater);
    if (pointer != nullptr) {
      idx =
          (reinterpret_cast<uint8_t*>(pointer) - key_->buffer) / sizeof(float);
    }

    if (idx >= num_rows || idx < 0) {
      memset(output_->buffer + i * row_bytes, 0, row_bytes);
      hits_->buffer[i] = 0;
    } else {
      memcpy(output_->buffer + i * row_bytes, value_->buffer + idx * row_bytes,
             row_bytes);
      hits_->buffer[i] = 1;
    }
  }

  return true;
}

}  // namespace nn
}  // namespace android
