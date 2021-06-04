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
#include "RNN.h"

namespace android {
namespace nn {
namespace unidirectional_sequence_rnn {

constexpr uint32_t kNumInputs = 7;
constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kWeightsTensor = 1;
constexpr uint32_t kRecurrentWeightsTensor = 2;
constexpr uint32_t kBiasTensor = 3;
constexpr uint32_t kHiddenStateTensor = 4;
constexpr uint32_t kActivationParam = 5;
constexpr uint32_t kTimeMajorParam = 6;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

template <typename T>
void transposeFirstTwoDims(const T* input, const Shape& inputShape, T* output) {
    const uint32_t firstDimSize = getSizeOfDimension(inputShape, 0);
    const uint32_t secondDimSize = getSizeOfDimension(inputShape, 1);
    const uint32_t inputSize = getSizeOfDimension(inputShape, 2);
    for (int f = 0; f < firstDimSize; ++f) {
        for (int s = 0; s < secondDimSize; ++s) {
            for (int i = 0; i < inputSize; ++i) {
                const uint32_t inputIndex = f * secondDimSize * inputSize + s * inputSize + i;
                const uint32_t outputIndex = s * firstDimSize * inputSize + f * inputSize + i;
                output[outputIndex] = input[inputIndex];
            }
        }
    }
}

template <typename T>
bool executeTyped(IOperationExecutionContext* context) {
    const T* input = context->getInputBuffer<T>(kInputTensor);
    Shape inputShape = context->getInputShape(kInputTensor);
    const T* weights = context->getInputBuffer<T>(kWeightsTensor);
    Shape weightsShape = context->getInputShape(kWeightsTensor);
    const T* recurrentWeights = context->getInputBuffer<T>(kRecurrentWeightsTensor);
    Shape recurrentWeightsShape = context->getInputShape(kRecurrentWeightsTensor);
    const T* bias = context->getInputBuffer<T>(kBiasTensor);
    const T* hiddenState = context->getInputBuffer<T>(kHiddenStateTensor);
    int32_t activation = context->getInputValue<int32_t>(kActivationParam);

    T* output = context->getOutputBuffer<T>(kOutputTensor);
    Shape outputShape = context->getOutputShape(kOutputTensor);

    int32_t timeMajor = context->getInputValue<int32_t>(kTimeMajorParam);
    // If the input tensors are not in time major format, we transpose the first
    // two dimensions, and set input and output pointers to temporary vectors
    // which are transposed back after the RNN is applied.
    std::vector<T> inputTransposed;
    std::vector<T> outputTransposed;
    if (!timeMajor) {
        // Convert input and output to time major format.
        inputTransposed.resize(getNumberOfElements(inputShape));
        outputTransposed.resize(getNumberOfElements(outputShape));
        transposeFirstTwoDims(input, inputShape, inputTransposed.data());
        input = inputTransposed.data();
        output = outputTransposed.data();
        std::swap(inputShape.dimensions[0], inputShape.dimensions[1]);
        std::swap(outputShape.dimensions[0], outputShape.dimensions[1]);
    }

    const uint32_t maxTime = getSizeOfDimension(inputShape, 0);
    const uint32_t batchSize = getSizeOfDimension(inputShape, 1);
    const uint32_t inputSize = getSizeOfDimension(inputShape, 2);
    const uint32_t numUnits = getSizeOfDimension(weightsShape, 0);

    // A shape at a fixed step (removed time dimension).
    Shape fixedTimeInputShape = inputShape;
    fixedTimeInputShape.dimensions.resize(2);
    fixedTimeInputShape.dimensions[0] = inputShape.dimensions[1];
    fixedTimeInputShape.dimensions[1] = inputShape.dimensions[2];

    for (int i = 0; i < maxTime; ++i) {
        RNN::RNNStep<T>(input, fixedTimeInputShape, hiddenState, bias, weights, weightsShape,
                        recurrentWeights, recurrentWeightsShape, activation, output);
        input += batchSize * inputSize;
        hiddenState = output;
        output += batchSize * numUnits;
    }

    if (!timeMajor) {
        transposeFirstTwoDims(outputTransposed.data(), outputShape,
                              context->getOutputBuffer<T>(kOutputTensor));
    }
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    OperandType inputType = context->getInputType(kInputTensor);
    if (inputType != OperandType::TENSOR_FLOAT16 && inputType != OperandType::TENSOR_FLOAT32) {
        LOG(ERROR) << "Unsupported input operand type for UNIDIRECTIONAL_SEQUENCE_RNN op: "
                   << toString(inputType);
        return false;
    }
    NN_RET_CHECK(validateInputTypes(context, {inputType, inputType, inputType, inputType, inputType,
                                              OperandType::INT32, OperandType::INT32}));
    NN_RET_CHECK(validateOutputTypes(context, {inputType}));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    Shape weights = context->getInputShape(kWeightsTensor);
    Shape recurrentWeights = context->getInputShape(kRecurrentWeightsTensor);
    Shape bias = context->getInputShape(kBiasTensor);
    Shape hiddenState = context->getInputShape(kHiddenStateTensor);

    int32_t timeMajor = context->getInputValue<int32_t>(kTimeMajorParam);
    NN_RET_CHECK(timeMajor == 0 || timeMajor == 1);
    const uint32_t batchSize =
            timeMajor ? getSizeOfDimension(input, 1) : getSizeOfDimension(input, 0);
    const uint32_t maxTime =
            timeMajor ? getSizeOfDimension(input, 0) : getSizeOfDimension(input, 1);
    const uint32_t numUnits = getSizeOfDimension(weights, 0);
    const uint32_t inputSize = getSizeOfDimension(input, 2);

    NN_RET_CHECK_EQ(getNumberOfDimensions(input), 3);
    NN_RET_CHECK_EQ(getNumberOfDimensions(weights), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(recurrentWeights), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(bias), 1);
    NN_RET_CHECK_EQ(getNumberOfDimensions(hiddenState), 2);

    NN_RET_CHECK_EQ(inputSize, getSizeOfDimension(weights, 1));
    NN_RET_CHECK_EQ(numUnits, getSizeOfDimension(bias, 0));
    NN_RET_CHECK_EQ(numUnits, getSizeOfDimension(recurrentWeights, 0));
    NN_RET_CHECK_EQ(numUnits, getSizeOfDimension(recurrentWeights, 1));
    NN_RET_CHECK_EQ(batchSize, getSizeOfDimension(hiddenState, 0));
    NN_RET_CHECK_EQ(numUnits, getSizeOfDimension(hiddenState, 1));

    Shape output = context->getOutputShape(kOutputTensor);
    output.dimensions[0] = timeMajor ? maxTime : batchSize;
    output.dimensions[1] = timeMajor ? batchSize : maxTime;
    output.dimensions[2] = numUnits;

    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    if (context->getInputType(kInputTensor) == OperandType::TENSOR_FLOAT16) {
        executeTyped<_Float16>(context);
    } else {
        executeTyped<float>(context);
    }
    return true;
}

}  // namespace unidirectional_sequence_rnn

NN_REGISTER_OPERATION(UNIDIRECTIONAL_SEQUENCE_RNN, "UNIDIRECTIONAL_SEQUENCE_RNN",
                      unidirectional_sequence_rnn::validate, unidirectional_sequence_rnn::prepare,
                      unidirectional_sequence_rnn::execute);

}  // namespace nn
}  // namespace android
