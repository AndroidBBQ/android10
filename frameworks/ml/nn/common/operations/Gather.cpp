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

namespace android {
namespace nn {
namespace gather {

constexpr char kOperationName[] = "GATHER";

constexpr uint32_t kNumInputs = 3;
constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kInputAxis = 1;
constexpr uint32_t kInputIndices = 2;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

template <typename T>
inline bool eval(const T* inputData, const Shape& inputShape, int32_t axis,
                 const int32_t* indicesData, const Shape& indicesShape, T* outputData) {
    const auto outerSize = getNumberOfElements(inputShape, 0, axis);
    const auto axisSize = getSizeOfDimension(inputShape, axis);
    const auto innerSize =
            getNumberOfElements(inputShape, axis + 1, getNumberOfDimensions(inputShape));
    const auto indicesCount = getNumberOfElements(indicesShape);
    for (uint32_t outer = 0; outer < outerSize; ++outer) {
        for (uint32_t outputIndex = 0; outputIndex < indicesCount; ++outputIndex) {
            const auto inputIndex = static_cast<uint32_t>(indicesData[outputIndex]);
            NN_RET_CHECK_LE(0u, inputIndex);
            NN_RET_CHECK_LT(inputIndex, axisSize);
            std::memcpy(outputData + (outer * indicesCount + outputIndex) * innerSize,
                        inputData + (outer * axisSize + inputIndex) * innerSize,
                        sizeof(T) * innerSize);
        }
    }
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    OperandType inputType = context->getInputType(kInputTensor);
    NN_RET_CHECK(
            inputType == OperandType::TENSOR_FLOAT16 || inputType == OperandType::TENSOR_FLOAT32 ||
            inputType == OperandType::TENSOR_INT32 || inputType == OperandType::TENSOR_QUANT8_ASYMM)
            << "Unsupported tensor type for operation " << kOperationName;
    NN_RET_CHECK(validateInputTypes(context,
                                    {inputType, OperandType::INT32, OperandType::TENSOR_INT32}));
    NN_RET_CHECK(validateOutputTypes(context, {inputType}));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    int32_t axis = context->getInputValue<int32_t>(kInputAxis);
    NN_RET_CHECK(handleNegativeAxis(input, &axis));
    Shape indices = context->getInputShape(kInputIndices);
    Shape output = context->getOutputShape(kOutputTensor);

    output.dimensions.clear();
    output.dimensions.reserve(getNumberOfDimensions(input) + getNumberOfDimensions(indices) - 1);
    output.dimensions.insert(output.dimensions.end(), input.dimensions.begin(),
                             input.dimensions.begin() + axis);
    output.dimensions.insert(output.dimensions.end(), indices.dimensions.begin(),
                             indices.dimensions.end());
    output.dimensions.insert(output.dimensions.end(), input.dimensions.begin() + axis + 1,
                             input.dimensions.end());

    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    int32_t axis = context->getInputValue<int32_t>(kInputAxis);
    NN_RET_CHECK(handleNegativeAxis(context->getInputShape(kInputTensor), &axis));
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return eval(context->getInputBuffer<_Float16>(kInputTensor),
                        context->getInputShape(kInputTensor), axis,
                        context->getInputBuffer<int32_t>(kInputIndices),
                        context->getInputShape(kInputIndices),
                        context->getOutputBuffer<_Float16>(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return eval(context->getInputBuffer<float>(kInputTensor),
                        context->getInputShape(kInputTensor), axis,
                        context->getInputBuffer<int32_t>(kInputIndices),
                        context->getInputShape(kInputIndices),
                        context->getOutputBuffer<float>(kOutputTensor));
        case OperandType::TENSOR_INT32:
            return eval(context->getInputBuffer<int32_t>(kInputTensor),
                        context->getInputShape(kInputTensor), axis,
                        context->getInputBuffer<int32_t>(kInputIndices),
                        context->getInputShape(kInputIndices),
                        context->getOutputBuffer<int32_t>(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return eval(context->getInputBuffer<uint8_t>(kInputTensor),
                        context->getInputShape(kInputTensor), axis,
                        context->getInputBuffer<int32_t>(kInputIndices),
                        context->getInputShape(kInputIndices),
                        context->getOutputBuffer<uint8_t>(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace gather

NN_REGISTER_OPERATION(GATHER, gather::kOperationName, gather::validate, gather::prepare,
                      gather::execute);

}  // namespace nn
}  // namespace android
