/*
 * Copyright (C) 2019 The Android Open Source Project
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
namespace bidirectional_sequence_rnn {

constexpr uint32_t kNumInputs = 15;
constexpr uint32_t kInputTensor = 0;
// Forward cell tensors
constexpr uint32_t kFwWeightsTensor = 1;
constexpr uint32_t kFwRecurrentWeightsTensor = 2;
constexpr uint32_t kFwBiasTensor = 3;
constexpr uint32_t kFwHiddenStateTensor = 4;
// Backward cell tensors
constexpr uint32_t kBwWeightsTensor = 5;
constexpr uint32_t kBwRecurrentWeightsTensor = 6;
constexpr uint32_t kBwBiasTensor = 7;
constexpr uint32_t kBwHiddenStateTensor = 8;
// Auxiliary inputs
constexpr uint32_t kAuxInputTensor = 9;       // optional
constexpr uint32_t kFwAuxWeightsTensor = 10;  // optional
constexpr uint32_t kBwAuxWeightsTensor = 11;  // optional
// Cell parameters
constexpr uint32_t kActivationParam = 12;
constexpr uint32_t kTimeMajorParam = 13;
constexpr uint32_t kMergeOutputsParam = 14;

constexpr uint32_t kFwOutputTensor = 0;
constexpr uint32_t kBwOutputTensor = 1;  // Only if mergeOutputs parameter is false

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

Shape removeFirstDim(const Shape& input) {
    Shape output = input;
    output.dimensions.resize(input.dimensions.size() - 1);
    for (int i = 0; i < input.dimensions.size() - 1; ++i) {
        output.dimensions[i] = input.dimensions[i + 1];
    }
    return output;
}

template <typename T>
bool executeTyped(IOperationExecutionContext* context) {
    const T* input = context->getInputBuffer<T>(kInputTensor);
    Shape inputShape = context->getInputShape(kInputTensor);

    const T* fwWeights = context->getInputBuffer<T>(kFwWeightsTensor);
    Shape fwWeightsShape = context->getInputShape(kFwWeightsTensor);
    const T* fwRecurrentWeights = context->getInputBuffer<T>(kFwRecurrentWeightsTensor);
    Shape fwRecurrentWeightsShape = context->getInputShape(kFwRecurrentWeightsTensor);
    const T* fwBias = context->getInputBuffer<T>(kFwBiasTensor);
    const T* fwHiddenState = context->getInputBuffer<T>(kFwHiddenStateTensor);

    const T* bwWeights = context->getInputBuffer<T>(kBwWeightsTensor);
    Shape bwWeightsShape = context->getInputShape(kBwWeightsTensor);
    const T* bwRecurrentWeights = context->getInputBuffer<T>(kBwRecurrentWeightsTensor);
    Shape bwRecurrentWeightsShape = context->getInputShape(kBwRecurrentWeightsTensor);
    const T* bwBias = context->getInputBuffer<T>(kBwBiasTensor);
    const T* bwHiddenState = context->getInputBuffer<T>(kBwHiddenStateTensor);

    const T* auxInput = nullptr;
    const T* fwAuxWeights = nullptr;
    const T* bwAuxWeights = nullptr;
    const bool hasAuxInputs = !context->isOmittedInput(kAuxInputTensor);
    if (hasAuxInputs) {
        auxInput = context->getInputBuffer<T>(kAuxInputTensor);
        fwAuxWeights = context->getInputBuffer<T>(kFwAuxWeightsTensor);
        bwAuxWeights = context->getInputBuffer<T>(kBwAuxWeightsTensor);
    }
    Shape auxInputShape = context->getInputShape(kAuxInputTensor);
    Shape fwAuxWeightsShape = context->getInputShape(kFwAuxWeightsTensor);
    Shape bwAuxWeightsShape = context->getInputShape(kBwAuxWeightsTensor);

    int32_t activation = context->getInputValue<int32_t>(kActivationParam);
    int32_t timeMajor = context->getInputValue<bool>(kTimeMajorParam);
    int32_t mergeOutputs = context->getInputValue<bool>(kMergeOutputsParam);

    T* fwOutput = context->getOutputBuffer<T>(kFwOutputTensor);
    Shape fwOutputShape = context->getOutputShape(kFwOutputTensor);
    T* bwOutput = nullptr;
    Shape bwOutputShape;
    if (!mergeOutputs) {
        bwOutputShape = context->getOutputShape(kBwOutputTensor);
        bwOutput = context->getOutputBuffer<T>(kBwOutputTensor);
    }

    // If the input tensors are not in time major format, we transpose the first
    // two dimensions, and set input and output pointers to temporary vectors
    // which are transposed back after the RNN is applied.
    std::vector<T> inputTransposed;
    std::vector<T> auxInputTransposed;
    std::vector<T> fwOutputTransposed;
    std::vector<T> bwOutputTransposed;
    if (!timeMajor) {
        // First, resize temporary buffers to accommodate for transposed tensors.
        inputTransposed.resize(getNumberOfElements(inputShape));
        if (hasAuxInputs) {
            auxInputTransposed.resize(getNumberOfElements(auxInputShape));
        }
        fwOutputTransposed.resize(getNumberOfElements(fwOutputShape));
        if (!mergeOutputs) {
            bwOutputTransposed.resize(getNumberOfElements(bwOutputShape));
        }

        // Transpose the input tensors.
        transposeFirstTwoDims(input, inputShape, inputTransposed.data());
        if (hasAuxInputs) {
            transposeFirstTwoDims(auxInput, auxInputShape, auxInputTransposed.data());
        }

        // Change input and output pointers to the temporary buffers.
        input = inputTransposed.data();
        if (hasAuxInputs) {
            auxInput = auxInputTransposed.data();
        }
        fwOutput = fwOutputTransposed.data();
        if (!mergeOutputs) {
            bwOutput = bwOutputTransposed.data();
        }

        // Swap the first two dimensions in the Shapes to reflect the
        // transposition.
        std::swap(inputShape.dimensions[0], inputShape.dimensions[1]);
        if (hasAuxInputs) {
            std::swap(auxInputShape.dimensions[0], auxInputShape.dimensions[1]);
        }
        std::swap(fwOutputShape.dimensions[0], fwOutputShape.dimensions[1]);
        if (!mergeOutputs) {
            std::swap(bwOutputShape.dimensions[0], bwOutputShape.dimensions[1]);
        }
    }

    const uint32_t maxTime = getSizeOfDimension(inputShape, 0);
    const uint32_t batchSize = getSizeOfDimension(inputShape, 1);
    const uint32_t inputSize = getSizeOfDimension(inputShape, 2);
    uint32_t auxInputSize = 0;
    if (hasAuxInputs) {
        auxInputSize = getSizeOfDimension(auxInputShape, 2);
    }
    const uint32_t fwNumUnits = getSizeOfDimension(fwWeightsShape, 0);
    const uint32_t bwNumUnits = getSizeOfDimension(bwWeightsShape, 0);

    Shape fixedTimeInputShape = removeFirstDim(inputShape);
    Shape fixedTimeAuxInputShape = auxInputShape;
    if (hasAuxInputs) {
        fixedTimeAuxInputShape = removeFirstDim(auxInputShape);
    }

    // Create an additional buffer to store a hidden state between steps.
    std::vector<T> tempHiddenState(batchSize * fwNumUnits);
    // Forward pass
    for (int i = 0; i < maxTime; ++i) {
        const T* inputBatchPtr = input + i * batchSize * inputSize;
        const T* auxInputBatchPtr = nullptr;
        if (hasAuxInputs) {
            auxInputBatchPtr = auxInput + i * batchSize * auxInputSize;
        }
        const uint32_t fwOutputBatchStride = mergeOutputs ? (fwNumUnits + bwNumUnits) : fwNumUnits;
        T* fwOutputBatchPtr = fwOutput + i * batchSize * fwOutputBatchStride;

        RNN::RNNStep<T>(inputBatchPtr, fixedTimeInputShape, auxInputBatchPtr,
                        fixedTimeAuxInputShape, fwHiddenState, fwBias, fwWeights, fwWeightsShape,
                        fwAuxWeights, fwAuxWeightsShape, fwRecurrentWeights,
                        fwRecurrentWeightsShape, activation, fwOutputBatchStride,
                        /*outputBatchOffset=*/0, fwOutputBatchPtr, tempHiddenState.data());

        fwHiddenState = tempHiddenState.data();
    }

    tempHiddenState.resize(batchSize * bwNumUnits);
    // Backward pass
    for (int i = maxTime - 1; i >= 0; --i) {
        const T* inputBatchPtr = input + i * batchSize * inputSize;
        const T* auxInputBatchPtr = nullptr;
        if (hasAuxInputs) {
            auxInputBatchPtr = auxInput + i * batchSize * auxInputSize;
        }
        T* bwOutputBatchPtr;
        uint32_t bwOutputBatchOffset = 0;
        uint32_t bwOutputBatchStride;
        if (mergeOutputs) {
            bwOutputBatchStride = fwNumUnits + bwNumUnits;
            bwOutputBatchOffset = fwNumUnits;
            bwOutputBatchPtr = fwOutput + i * batchSize * bwOutputBatchStride;
        } else {
            bwOutputBatchStride = bwNumUnits;
            bwOutputBatchPtr = bwOutput + i * batchSize * bwOutputBatchStride;
        }

        RNN::RNNStep<T>(inputBatchPtr, fixedTimeInputShape, auxInputBatchPtr,
                        fixedTimeAuxInputShape, bwHiddenState, bwBias, bwWeights, bwWeightsShape,
                        bwAuxWeights, bwAuxWeightsShape, bwRecurrentWeights,
                        bwRecurrentWeightsShape, activation, bwOutputBatchStride,
                        bwOutputBatchOffset, bwOutputBatchPtr, tempHiddenState.data());

        bwHiddenState = tempHiddenState.data();
    }

    // If the inputs were in batch major format, transpose data in temporary
    // buffers and write to the output(s).
    if (!timeMajor) {
        transposeFirstTwoDims(fwOutputTransposed.data(), fwOutputShape,
                              context->getOutputBuffer<T>(kFwOutputTensor));
        if (!mergeOutputs) {
            transposeFirstTwoDims(bwOutputTransposed.data(), bwOutputShape,
                                  context->getOutputBuffer<T>(kBwOutputTensor));
        }
    }
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    // Exact number is dependent on the mergeOutputs parameter and checked
    // during preparation.
    NN_RET_CHECK(context->getNumOutputs() == 1 || context->getNumOutputs() == 2);
    OperandType inputType = context->getInputType(kInputTensor);
    if (inputType != OperandType::TENSOR_FLOAT16 && inputType != OperandType::TENSOR_FLOAT32) {
        LOG(ERROR) << "Unsupported input operand type for UNIDIRECTIONAL_SEQUENCE_RNN op: "
                   << toString(inputType);
        return false;
    }
    NN_RET_CHECK(validateInputTypes(
            context, {inputType, inputType, inputType, inputType, inputType, inputType, inputType,
                      inputType, inputType, inputType, inputType, inputType, OperandType::INT32,
                      OperandType::BOOL, OperandType::BOOL}));
    if (context->getNumOutputs() == 1) {
        NN_RET_CHECK(validateOutputTypes(context, {inputType}));
    } else {
        NN_RET_CHECK(validateOutputTypes(context, {inputType, inputType}));
    }
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    int32_t mergeOutputs = context->getInputValue<bool>(kMergeOutputsParam);
    if (mergeOutputs) {
        NN_RET_CHECK_EQ(context->getNumOutputs(), 1);
    } else {
        NN_RET_CHECK_EQ(context->getNumOutputs(), 2);
    }

    // Check that none of the required inputs are omitted.
    const std::vector<int> requiredInputs = {
            kInputTensor,         kFwWeightsTensor, kFwRecurrentWeightsTensor, kFwBiasTensor,
            kFwHiddenStateTensor, kBwWeightsTensor, kBwRecurrentWeightsTensor, kBwBiasTensor,
            kBwHiddenStateTensor, kActivationParam, kTimeMajorParam,           kMergeOutputsParam,
    };
    for (const int requiredInput : requiredInputs) {
        NN_RET_CHECK(!context->isOmittedInput(requiredInput))
                << "required input " << requiredInput << " is omitted";
    }

    Shape input = context->getInputShape(kInputTensor);
    Shape fwWeights = context->getInputShape(kFwWeightsTensor);
    Shape fwRecurrentWeights = context->getInputShape(kFwRecurrentWeightsTensor);
    Shape fwBias = context->getInputShape(kFwBiasTensor);
    Shape fwHiddenState = context->getInputShape(kFwHiddenStateTensor);
    Shape bwWeights = context->getInputShape(kBwWeightsTensor);
    Shape bwRecurrentWeights = context->getInputShape(kBwRecurrentWeightsTensor);
    Shape bwBias = context->getInputShape(kBwBiasTensor);
    Shape bwHiddenState = context->getInputShape(kBwHiddenStateTensor);

    Shape auxInput = context->getInputShape(kAuxInputTensor);
    Shape fwAuxWeights = context->getInputShape(kFwAuxWeightsTensor);
    Shape bwAuxWeights = context->getInputShape(kBwAuxWeightsTensor);

    const bool auxInputsAllOrNone = (context->isOmittedInput(kAuxInputTensor) &&
                                     context->isOmittedInput(kFwAuxWeightsTensor) &&
                                     context->isOmittedInput(kBwAuxWeightsTensor)) ||
                                    (!context->isOmittedInput(kAuxInputTensor) &&
                                     !context->isOmittedInput(kFwAuxWeightsTensor) &&
                                     !context->isOmittedInput(kBwAuxWeightsTensor));
    NN_RET_CHECK(auxInputsAllOrNone);
    const bool hasAuxInputs = !context->isOmittedInput(kAuxInputTensor);

    int32_t timeMajor = context->getInputValue<bool>(kTimeMajorParam);
    const uint32_t batchSize =
            timeMajor ? getSizeOfDimension(input, 1) : getSizeOfDimension(input, 0);
    const uint32_t maxTime =
            timeMajor ? getSizeOfDimension(input, 0) : getSizeOfDimension(input, 1);
    const uint32_t fwNumUnits = getSizeOfDimension(fwWeights, 0);
    const uint32_t bwNumUnits = getSizeOfDimension(bwWeights, 0);
    const uint32_t inputSize = getSizeOfDimension(input, 2);

    NN_RET_CHECK_EQ(getNumberOfDimensions(input), 3);
    NN_RET_CHECK_EQ(getNumberOfDimensions(fwWeights), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(fwRecurrentWeights), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(fwBias), 1);
    NN_RET_CHECK_EQ(getNumberOfDimensions(fwHiddenState), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(bwWeights), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(bwRecurrentWeights), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(bwBias), 1);
    NN_RET_CHECK_EQ(getNumberOfDimensions(bwHiddenState), 2);

    NN_RET_CHECK_EQ(inputSize, getSizeOfDimension(fwWeights, 1));
    NN_RET_CHECK_EQ(fwNumUnits, getSizeOfDimension(fwBias, 0));
    NN_RET_CHECK_EQ(fwNumUnits, getSizeOfDimension(fwRecurrentWeights, 0));
    NN_RET_CHECK_EQ(fwNumUnits, getSizeOfDimension(fwRecurrentWeights, 1));
    NN_RET_CHECK_EQ(batchSize, getSizeOfDimension(fwHiddenState, 0));
    NN_RET_CHECK_EQ(fwNumUnits, getSizeOfDimension(fwHiddenState, 1));

    NN_RET_CHECK_EQ(inputSize, getSizeOfDimension(bwWeights, 1));
    NN_RET_CHECK_EQ(bwNumUnits, getSizeOfDimension(bwBias, 0));
    NN_RET_CHECK_EQ(bwNumUnits, getSizeOfDimension(bwRecurrentWeights, 0));
    NN_RET_CHECK_EQ(bwNumUnits, getSizeOfDimension(bwRecurrentWeights, 1));
    NN_RET_CHECK_EQ(batchSize, getSizeOfDimension(bwHiddenState, 0));
    NN_RET_CHECK_EQ(bwNumUnits, getSizeOfDimension(bwHiddenState, 1));

    if (hasAuxInputs) {
        NN_RET_CHECK_EQ(getNumberOfDimensions(auxInput), 3);
        NN_RET_CHECK_EQ(getNumberOfDimensions(fwAuxWeights), 2);
        NN_RET_CHECK_EQ(getNumberOfDimensions(bwAuxWeights), 2);

        NN_RET_CHECK_EQ(getSizeOfDimension(auxInput, 0), getSizeOfDimension(input, 0));
        NN_RET_CHECK_EQ(getSizeOfDimension(auxInput, 1), getSizeOfDimension(input, 1));
        NN_RET_CHECK_EQ(getSizeOfDimension(fwAuxWeights, 0), fwNumUnits);
        NN_RET_CHECK_EQ(getSizeOfDimension(fwAuxWeights, 1), getSizeOfDimension(auxInput, 2));
        NN_RET_CHECK_EQ(getSizeOfDimension(bwAuxWeights, 0), bwNumUnits);
        NN_RET_CHECK_EQ(getSizeOfDimension(bwAuxWeights, 1), getSizeOfDimension(auxInput, 2));
    }

    Shape fwOutput = context->getOutputShape(kFwOutputTensor);
    fwOutput.dimensions.resize(3);
    fwOutput.dimensions[0] = timeMajor ? maxTime : batchSize;
    fwOutput.dimensions[1] = timeMajor ? batchSize : maxTime;
    fwOutput.dimensions[2] = mergeOutputs ? fwNumUnits + bwNumUnits : fwNumUnits;
    NN_RET_CHECK(context->setOutputShape(kFwOutputTensor, fwOutput));
    if (!mergeOutputs) {
        Shape bwOutput = context->getOutputShape(kBwOutputTensor);
        bwOutput.dimensions.resize(3);
        bwOutput.dimensions[0] = timeMajor ? maxTime : batchSize;
        bwOutput.dimensions[1] = timeMajor ? batchSize : maxTime;
        bwOutput.dimensions[2] = bwNumUnits;
        NN_RET_CHECK(context->setOutputShape(kBwOutputTensor, bwOutput));
    }

    return true;
}

bool execute(IOperationExecutionContext* context) {
    if (context->getInputType(kInputTensor) == OperandType::TENSOR_FLOAT16) {
        executeTyped<_Float16>(context);
    } else {
        executeTyped<float>(context);
    }
    return true;
}

}  // namespace bidirectional_sequence_rnn

NN_REGISTER_OPERATION(BIDIRECTIONAL_SEQUENCE_RNN, "BIDIRECTIONAL_SEQUENCE_RNN",
                      bidirectional_sequence_rnn::validate, bidirectional_sequence_rnn::prepare,
                      bidirectional_sequence_rnn::execute, .allowOmittedOperand = true);

}  // namespace nn
}  // namespace android
