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
namespace logical {

constexpr uint32_t kNumInputs = 2;
constexpr uint32_t kInputTensor1 = 0;
constexpr uint32_t kInputTensor2 = 1;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

bool compute(const std::function<bool(bool, bool)>& func, const bool8* aData, const Shape& aShape,
             const bool8* bData, const Shape& bShape, bool8* outputData, const Shape& outputShape) {
    IndexedShapeWrapper aShapeIndexed(aShape);
    IndexedShapeWrapper bShapeIndexed(bShape);
    IndexedShapeWrapper outputShapeIndexed(outputShape);
    std::vector<uint32_t> curIndex(outputShape.dimensions.size(), 0);
    bool lastIndex = false;
    do {
        uint32_t outputFlatIndex;
        NN_RET_CHECK(outputShapeIndexed.indexToFlatIndex(curIndex, &outputFlatIndex));
        uint32_t aFlatIndex;
        NN_RET_CHECK(aShapeIndexed.broadcastedIndexToFlatIndex(curIndex, &aFlatIndex));
        uint32_t bFlatIndex;
        NN_RET_CHECK(bShapeIndexed.broadcastedIndexToFlatIndex(curIndex, &bFlatIndex));

        outputData[outputFlatIndex] = func(aData[aFlatIndex], bData[bFlatIndex]);

        NN_RET_CHECK(outputShapeIndexed.nextIndexInplace(&curIndex, &lastIndex));
    } while (!lastIndex);
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    OperandType inputType = context->getInputType(kInputTensor1);
    NN_RET_CHECK(inputType == OperandType::TENSOR_BOOL8)
            << "Unsupported tensor type for a logical operation";
    NN_RET_CHECK(validateInputTypes(context, {inputType, inputType}));
    NN_RET_CHECK(validateOutputTypes(context, {inputType}));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    Shape input1 = context->getInputShape(kInputTensor1);
    Shape input2 = context->getInputShape(kInputTensor2);
    Shape output = context->getOutputShape(kOutputTensor);
    NN_RET_CHECK(calculateBroadcastedShape(input1, input2, &output));
    return context->setOutputShape(kOutputTensor, output);
}

bool executeAnd(IOperationExecutionContext* context) {
    return compute(
            std::logical_and<bool>(), context->getInputBuffer<bool8>(kInputTensor1),
            context->getInputShape(kInputTensor1), context->getInputBuffer<bool8>(kInputTensor2),
            context->getInputShape(kInputTensor2), context->getOutputBuffer<bool8>(kOutputTensor),
            context->getOutputShape(kOutputTensor));
}

bool executeOr(IOperationExecutionContext* context) {
    return compute(
            std::logical_or<bool>(), context->getInputBuffer<bool8>(kInputTensor1),
            context->getInputShape(kInputTensor1), context->getInputBuffer<bool8>(kInputTensor2),
            context->getInputShape(kInputTensor2), context->getOutputBuffer<bool8>(kOutputTensor),
            context->getOutputShape(kOutputTensor));
}

}  // namespace logical

NN_REGISTER_OPERATION(LOGICAL_AND, "LOGICAL_AND", logical::validate, logical::prepare,
                      logical::executeAnd);
NN_REGISTER_OPERATION(LOGICAL_OR, "LOGICAL_OR", logical::validate, logical::prepare,
                      logical::executeOr);

}  // namespace nn
}  // namespace android
