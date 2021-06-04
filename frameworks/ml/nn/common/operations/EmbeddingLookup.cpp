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

#include "EmbeddingLookup.h"

#include "CpuExecutor.h"
#include "Operations.h"

#include "Tracing.h"

namespace android {
namespace nn {

EmbeddingLookup::EmbeddingLookup(const Operation& operation,
                                 std::vector<RunTimeOperandInfo>& operands) {
  value_ = GetInput(operation, operands, kValueTensor);
  lookup_ = GetInput(operation, operands, kLookupTensor);

  output_ = GetOutput(operation, operands, kOutputTensor);
}

bool EmbeddingLookup::Eval() {
  NNTRACE_COMP("EmbeddingLookup::Eval");
  const int row_size = value_->shape().dimensions[0];
  const int total_bytes = nonExtensionOperandSizeOfData(value_->type, value_->dimensions);
  const int row_bytes = total_bytes/row_size;

  for (uint32_t i = 0; i < lookup_->shape().dimensions[0]; i++) {
    int idx = (reinterpret_cast<int*>(lookup_->buffer))[i];
    if (idx >= row_size || idx < 0) {
      LOG(ERROR) << "Embedding Lookup: index out of bounds.";
      return false;
    } else {
      memcpy(output_->buffer + i * row_bytes, value_->buffer + idx * row_bytes,
             row_bytes);
    }
  }

  return true;
}

}  // namespace nn
}  // namespace android
