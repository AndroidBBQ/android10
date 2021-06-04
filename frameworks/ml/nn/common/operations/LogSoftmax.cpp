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
namespace log_softmax {

constexpr char kOperationName[] = "LOG_SOFTMAX";

constexpr uint32_t kNumInputs = 3;
constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kInputBeta = 1;
constexpr uint32_t kInputAxis = 2;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

template <typename T>
inline bool compute(const T* input, const Shape& shape, T beta, uint32_t axis, T* output) {
    const uint32_t outerSize = getNumberOfElements(shape, 0, axis);
    const uint32_t axisSize = getSizeOfDimension(shape, axis);
    const uint32_t innerSize = getNumberOfElements(shape, axis + 1, getNumberOfDimensions(shape));
    for (uint32_t outer = 0; outer < outerSize; ++outer) {
        for (uint32_t inner = 0; inner < innerSize; ++inner) {
            // We subtract the maximum value from each element to ensure
            // numerical stability, taking advantage of the following equality:
            // exp(x[i])/sum(exp(x[i])) == exp(x[i]+C)/sum(exp(x[i]+C))
            T maxValue = input[outer * axisSize * innerSize + inner];
            for (uint32_t i = 1; i < axisSize; ++i) {
                maxValue = std::max(maxValue, input[(outer * axisSize + i) * innerSize + inner]);
            }

            T sum = 0;
            for (uint32_t i = 0; i < axisSize; ++i) {
                sum += std::exp(static_cast<double>(
                        (input[(outer * axisSize + i) * innerSize + inner] - maxValue) * beta));
            }

            const T logSum = std::log(static_cast<double>(sum));
            for (uint32_t i = 0; i < axisSize; ++i) {
                output[(outer * axisSize + i) * innerSize + inner] =
                        (input[(outer * axisSize + i) * innerSize + inner] - maxValue) * beta -
                        logSum;
            }
        }
    }
    return true;
}

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    OperandType inputType = context->getInputType(kInputTensor);
    std::vector<OperandType> inExpectedTypes;
    std::vector<OperandType> outExpectedTypes;
    if (inputType == OperandType::TENSOR_FLOAT32) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT32, OperandType::FLOAT32, OperandType::INT32};
        outExpectedTypes = {OperandType::TENSOR_FLOAT32};
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::FLOAT16, OperandType::INT32};
        outExpectedTypes = {OperandType::TENSOR_FLOAT16};
    } else {
        LOG(ERROR) << "Unsupported input tensor type for operation " << kOperationName;
        return false;
    }
    NN_RET_CHECK(validateInputTypes(context, inExpectedTypes));
    NN_RET_CHECK(validateOutputTypes(context, outExpectedTypes));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    return context->setOutputShape(kOutputTensor, context->getInputShape(kInputTensor));
}

bool execute(IOperationExecutionContext* context) {
    int32_t axis = context->getInputValue<int32_t>(kInputAxis);
    NN_RET_CHECK(handleNegativeAxis(context->getInputShape(kInputTensor), &axis));
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return compute(context->getInputBuffer<_Float16>(kInputTensor),
                           context->getInputShape(kInputTensor),
                           context->getInputValue<_Float16>(kInputBeta), axis,
                           context->getOutputBuffer<_Float16>(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return compute(context->getInputBuffer<float>(kInputTensor),
                           context->getInputShape(kInputTensor),
                           context->getInputValue<float>(kInputBeta), axis,
                           context->getOutputBuffer<float>(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace log_softmax

NN_REGISTER_OPERATION(LOG_SOFTMAX, log_softmax::kOperationName, log_softmax::validate,
                      log_softmax::prepare, log_softmax::execute);

}  // namespace nn
}  // namespace android
