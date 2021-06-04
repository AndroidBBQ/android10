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
#include "IndexedShapeWrapper.h"
#include "OperationResolver.h"
#include "OperationsUtils.h"

namespace android {
namespace nn {
namespace select_op {

constexpr uint32_t kNumInputs = 3;
constexpr uint32_t kInputCondition = 0;
constexpr uint32_t kInputTensor1 = 1;
constexpr uint32_t kInputTensor2 = 2;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

template <typename T>
bool compute(const bool8* conditionData, const Shape& conditionShape, const T* aData,
             const Shape& aShape, const T* bData, const Shape& bShape, T* outputData,
             const Shape& outputShape) {
    // The code assumes that condition has the same shape as all other tensors.
    // This should be checked during preparation stage.
    uint32_t size = getNumberOfElements(conditionShape);
    for (uint32_t i = 0; i < size; ++i) {
        T a = aData[i];
        T b = bData[i];
        if (aShape.type == OperandType::TENSOR_QUANT8_ASYMM) {
            a = requantize(a, aShape, outputShape);
            b = requantize(b, bShape, outputShape);
        }
        outputData[i] = conditionData[i] ? a : b;
    }
    return true;
}

template <typename T>
bool executeTyped(IOperationExecutionContext* context) {
    return compute<T>(
            context->getInputBuffer<bool8>(kInputCondition),
            context->getInputShape(kInputCondition), context->getInputBuffer<T>(kInputTensor1),
            context->getInputShape(kInputTensor1), context->getInputBuffer<T>(kInputTensor2),
            context->getInputShape(kInputTensor2), context->getOutputBuffer<T>(kOutputTensor),
            context->getOutputShape(kOutputTensor));
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    OperandType inputType = context->getInputType(kInputTensor1);
    NN_RET_CHECK(
            inputType == OperandType::TENSOR_FLOAT16 || inputType == OperandType::TENSOR_FLOAT32 ||
            inputType == OperandType::TENSOR_INT32 || inputType == OperandType::TENSOR_QUANT8_ASYMM)
            << "Unsupported input operand type for select op: " << toString(inputType);
    NN_RET_CHECK(validateInputTypes(context, {OperandType::TENSOR_BOOL8, inputType, inputType}));
    NN_RET_CHECK(validateOutputTypes(context, {inputType}));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    Shape inputCondition = context->getInputShape(kInputCondition);
    Shape input1 = context->getInputShape(kInputTensor1);
    if (inputCondition.dimensions.size() != input1.dimensions.size()) {
        LOG(ERROR) << "Condition and input tensor dimensions are not equal";
        return false;
    }
    for (int i = 0; i < inputCondition.dimensions.size(); ++i) {
        if (inputCondition.dimensions[i] != input1.dimensions[i]) {
            LOG(ERROR) << "Condition and input tensor dimensions are not equal";
            return false;
        }
    }

    Shape input2 = context->getInputShape(kInputTensor2);
    NN_RET_CHECK(SameShape(input1, input2));

    Shape output = context->getOutputShape(kOutputTensor);
    NN_RET_CHECK(SetShape(input1, &output));
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    switch (context->getInputType(kInputTensor1)) {
        case OperandType::TENSOR_FLOAT16:
            return executeTyped<_Float16>(context);
        case OperandType::TENSOR_FLOAT32:
            return executeTyped<float>(context);
        case OperandType::TENSOR_INT32:
            return executeTyped<int32_t>(context);
        case OperandType::TENSOR_QUANT8_ASYMM:
            return executeTyped<uint8_t>(context);
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for SELECT op.";
    }
}

}  // namespace select_op

NN_REGISTER_OPERATION(SELECT, "SELECT", select_op::validate, select_op::prepare,
                      select_op::execute);

}  // namespace nn
}  // namespace android
