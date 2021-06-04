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

#include "HalInterfaces.h"
#include "OperationResolver.h"
#include "OperationsUtils.h"
#include "Tracing.h"

#include <cmath>

namespace android {
namespace nn {
namespace elementwise {

constexpr uint32_t kNumInputs = 1;
constexpr uint32_t kInputTensor = 0;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

template <typename T>
inline bool compute(float func(float), const T* input, const Shape& shape, T* output) {
    const auto size = getNumberOfElements(shape);
    for (uint32_t i = 0; i < size; ++i) {
        output[i] = static_cast<T>(func(static_cast<float>(input[i])));
    }
    return true;
}

bool execute(IOperationExecutionContext* context, float func(float)) {
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return compute(func, context->getInputBuffer<_Float16>(kInputTensor),
                           context->getInputShape(kInputTensor),
                           context->getOutputBuffer<_Float16>(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return compute(func, context->getInputBuffer<float>(kInputTensor),
                           context->getInputShape(kInputTensor),
                           context->getOutputBuffer<float>(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for elementwise operation";
    }
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    OperandType inputType = context->getInputType(kInputTensor);
    NN_RET_CHECK(inputType == OperandType::TENSOR_FLOAT16 ||
                 inputType == OperandType::TENSOR_FLOAT32)
            << "Unsupported tensor type for elementwise operation";
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

bool executeAbs(IOperationExecutionContext* context) {
    return execute(context, std::abs);
}

bool executeExp(IOperationExecutionContext* context) {
    return execute(context, std::exp);
}

bool executeLog(IOperationExecutionContext* context) {
    return execute(context, std::log);
}

bool executeRsqrt(IOperationExecutionContext* context) {
    return execute(context, [](float x) { return 1.f / std::sqrt(x); });
}

bool executeSin(IOperationExecutionContext* context) {
    return execute(context, std::sin);
}

bool executeSqrt(IOperationExecutionContext* context) {
    return execute(context, std::sqrt);
}

}  // namespace elementwise

NN_REGISTER_OPERATION(ABS, "ABS", elementwise::validate, elementwise::prepare,
                      elementwise::executeAbs);
NN_REGISTER_OPERATION(EXP, "EXP", elementwise::validate, elementwise::prepare,
                      elementwise::executeExp);
NN_REGISTER_OPERATION(LOG, "LOG", elementwise::validate, elementwise::prepare,
                      elementwise::executeLog);
NN_REGISTER_OPERATION(RSQRT, "RSQRT", elementwise::validate, elementwise::prepare,
                      elementwise::executeRsqrt);
NN_REGISTER_OPERATION(SIN, "SIN", elementwise::validate, elementwise::prepare,
                      elementwise::executeSin);
NN_REGISTER_OPERATION(SQRT, "SQRT", elementwise::validate, elementwise::prepare,
                      elementwise::executeSqrt);

}  // namespace nn
}  // namespace android
