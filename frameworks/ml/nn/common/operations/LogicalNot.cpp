/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "Operations"

#include "OperationResolver.h"
#include "OperationsUtils.h"

namespace android {
namespace nn {
namespace logical_not {

constexpr uint32_t kNumInputs = 1;
constexpr uint32_t kInputTensor = 0;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

bool compute(const bool8* input, const Shape& shape, bool8* output) {
    const auto size = getNumberOfElements(shape);
    for (uint32_t i = 0; i < size; ++i) {
        output[i] = input[i] == 0;
    }
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    OperandType inputType = context->getInputType(kInputTensor);
    NN_RET_CHECK(inputType == OperandType::TENSOR_BOOL8)
            << "Unsupported tensor type for LOGICAL_NOT";
    NN_RET_CHECK(validateInputTypes(context, {inputType}));
    NN_RET_CHECK(validateOutputTypes(context, {inputType}));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    Shape output = context->getOutputShape(kOutputTensor);
    NN_RET_CHECK(SetShape(input, &output));
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    return compute(context->getInputBuffer<bool8>(kInputTensor),
                   context->getInputShape(kInputTensor),
                   context->getOutputBuffer<bool8>(kOutputTensor));
}

}  // namespace logical_not

NN_REGISTER_OPERATION(LOGICAL_NOT, "LOGICAL_NOT", logical_not::validate, logical_not::prepare,
                      logical_not::execute);

}  // namespace nn
}  // namespace android
