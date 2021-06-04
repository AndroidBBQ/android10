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

#include "QuantizedLSTM.h"

#include "NeuralNetworksWrapper.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

#include <iostream>

namespace android {
namespace nn {
namespace wrapper {

namespace {

struct OperandTypeParams {
    Type type;
    std::vector<uint32_t> shape;
    float scale;
    int32_t zeroPoint;

    OperandTypeParams(Type type, std::vector<uint32_t> shape, float scale, int32_t zeroPoint)
        : type(type), shape(shape), scale(scale), zeroPoint(zeroPoint) {}
};

}  // namespace

using ::testing::Each;
using ::testing::ElementsAreArray;
using ::testing::FloatNear;
using ::testing::Matcher;

class QuantizedLSTMOpModel {
   public:
    QuantizedLSTMOpModel(const std::vector<OperandTypeParams>& inputOperandTypeParams) {
        std::vector<uint32_t> inputs;

        for (int i = 0; i < NUM_INPUTS; ++i) {
            const auto& curOTP = inputOperandTypeParams[i];
            OperandType curType(curOTP.type, curOTP.shape, curOTP.scale, curOTP.zeroPoint);
            inputs.push_back(model_.addOperand(&curType));
        }

        const uint32_t numBatches = inputOperandTypeParams[0].shape[0];
        inputSize_ = inputOperandTypeParams[0].shape[0];
        const uint32_t outputSize =
                inputOperandTypeParams[QuantizedLSTMCell::kPrevCellStateTensor].shape[1];
        outputSize_ = outputSize;

        std::vector<uint32_t> outputs;
        OperandType cellStateOutOperandType(Type::TENSOR_QUANT16_SYMM, {numBatches, outputSize},
                                            1. / 2048., 0);
        outputs.push_back(model_.addOperand(&cellStateOutOperandType));
        OperandType outputOperandType(Type::TENSOR_QUANT8_ASYMM, {numBatches, outputSize},
                                      1. / 128., 128);
        outputs.push_back(model_.addOperand(&outputOperandType));

        model_.addOperation(ANEURALNETWORKS_QUANTIZED_16BIT_LSTM, inputs, outputs);
        model_.identifyInputsAndOutputs(inputs, outputs);

        initializeInputData(inputOperandTypeParams[QuantizedLSTMCell::kInputTensor], &input_);
        initializeInputData(inputOperandTypeParams[QuantizedLSTMCell::kPrevOutputTensor],
                            &prevOutput_);
        initializeInputData(inputOperandTypeParams[QuantizedLSTMCell::kPrevCellStateTensor],
                            &prevCellState_);

        cellStateOut_.resize(numBatches * outputSize, 0);
        output_.resize(numBatches * outputSize, 0);

        model_.finish();
    }

    void invoke() {
        ASSERT_TRUE(model_.isValid());

        Compilation compilation(&model_);
        compilation.finish();
        Execution execution(&compilation);

        // Set all the inputs.
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kInputTensor, input_),
                  Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kInputToInputWeightsTensor,
                                 inputToInputWeights_),
                  Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kInputToForgetWeightsTensor,
                                 inputToForgetWeights_),
                  Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kInputToCellWeightsTensor,
                                 inputToCellWeights_),
                  Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kInputToOutputWeightsTensor,
                                 inputToOutputWeights_),
                  Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kRecurrentToInputWeightsTensor,
                                 recurrentToInputWeights_),
                  Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kRecurrentToForgetWeightsTensor,
                                 recurrentToForgetWeights_),
                  Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kRecurrentToCellWeightsTensor,
                                 recurrentToCellWeights_),
                  Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kRecurrentToOutputWeightsTensor,
                                 recurrentToOutputWeights_),
                  Result::NO_ERROR);
        ASSERT_EQ(
                setInputTensor(&execution, QuantizedLSTMCell::kInputGateBiasTensor, inputGateBias_),
                Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kForgetGateBiasTensor,
                                 forgetGateBias_),
                  Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kCellGateBiasTensor, cellGateBias_),
                  Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kOutputGateBiasTensor,
                                 outputGateBias_),
                  Result::NO_ERROR);
        ASSERT_EQ(
                setInputTensor(&execution, QuantizedLSTMCell::kPrevCellStateTensor, prevCellState_),
                Result::NO_ERROR);
        ASSERT_EQ(setInputTensor(&execution, QuantizedLSTMCell::kPrevOutputTensor, prevOutput_),
                  Result::NO_ERROR);
        // Set all the outputs.
        ASSERT_EQ(
                setOutputTensor(&execution, QuantizedLSTMCell::kCellStateOutTensor, &cellStateOut_),
                Result::NO_ERROR);
        ASSERT_EQ(setOutputTensor(&execution, QuantizedLSTMCell::kOutputTensor, &output_),
                  Result::NO_ERROR);

        ASSERT_EQ(execution.compute(), Result::NO_ERROR);

        // Put state outputs into inputs for the next step
        prevOutput_ = output_;
        prevCellState_ = cellStateOut_;
    }

    int inputSize() { return inputSize_; }

    int outputSize() { return outputSize_; }

    void setInput(const std::vector<uint8_t>& input) { input_ = input; }

    void setWeightsAndBiases(std::vector<uint8_t> inputToInputWeights,
                             std::vector<uint8_t> inputToForgetWeights,
                             std::vector<uint8_t> inputToCellWeights,
                             std::vector<uint8_t> inputToOutputWeights,
                             std::vector<uint8_t> recurrentToInputWeights,
                             std::vector<uint8_t> recurrentToForgetWeights,
                             std::vector<uint8_t> recurrentToCellWeights,
                             std::vector<uint8_t> recurrentToOutputWeights,
                             std::vector<int32_t> inputGateBias,
                             std::vector<int32_t> forgetGateBias,
                             std::vector<int32_t> cellGateBias,  //
                             std::vector<int32_t> outputGateBias) {
        inputToInputWeights_ = inputToInputWeights;
        inputToForgetWeights_ = inputToForgetWeights;
        inputToCellWeights_ = inputToCellWeights;
        inputToOutputWeights_ = inputToOutputWeights;
        recurrentToInputWeights_ = recurrentToInputWeights;
        recurrentToForgetWeights_ = recurrentToForgetWeights;
        recurrentToCellWeights_ = recurrentToCellWeights;
        recurrentToOutputWeights_ = recurrentToOutputWeights;
        inputGateBias_ = inputGateBias;
        forgetGateBias_ = forgetGateBias;
        cellGateBias_ = cellGateBias;
        outputGateBias_ = outputGateBias;
    }

    template <typename T>
    void initializeInputData(OperandTypeParams params, std::vector<T>* vec) {
        int size = 1;
        for (int d : params.shape) {
            size *= d;
        }
        vec->clear();
        vec->resize(size, params.zeroPoint);
    }

    std::vector<uint8_t> getOutput() { return output_; }

   private:
    static constexpr int NUM_INPUTS = 15;
    static constexpr int NUM_OUTPUTS = 2;

    Model model_;
    // Inputs
    std::vector<uint8_t> input_;
    std::vector<uint8_t> inputToInputWeights_;
    std::vector<uint8_t> inputToForgetWeights_;
    std::vector<uint8_t> inputToCellWeights_;
    std::vector<uint8_t> inputToOutputWeights_;
    std::vector<uint8_t> recurrentToInputWeights_;
    std::vector<uint8_t> recurrentToForgetWeights_;
    std::vector<uint8_t> recurrentToCellWeights_;
    std::vector<uint8_t> recurrentToOutputWeights_;
    std::vector<int32_t> inputGateBias_;
    std::vector<int32_t> forgetGateBias_;
    std::vector<int32_t> cellGateBias_;
    std::vector<int32_t> outputGateBias_;
    std::vector<int16_t> prevCellState_;
    std::vector<uint8_t> prevOutput_;
    // Outputs
    std::vector<int16_t> cellStateOut_;
    std::vector<uint8_t> output_;

    int inputSize_;
    int outputSize_;

    template <typename T>
    Result setInputTensor(Execution* execution, int tensor, const std::vector<T>& data) {
        return execution->setInput(tensor, data.data(), sizeof(T) * data.size());
    }
    template <typename T>
    Result setOutputTensor(Execution* execution, int tensor, std::vector<T>* data) {
        return execution->setOutput(tensor, data->data(), sizeof(T) * data->size());
    }
};

class QuantizedLstmTest : public ::testing::Test {
   protected:
    void VerifyGoldens(const std::vector<std::vector<uint8_t>>& input,
                       const std::vector<std::vector<uint8_t>>& output,
                       QuantizedLSTMOpModel* lstm) {
        const int numBatches = input.size();
        EXPECT_GT(numBatches, 0);
        const int inputSize = lstm->inputSize();
        EXPECT_GT(inputSize, 0);
        const int inputSequenceSize = input[0].size() / inputSize;
        EXPECT_GT(inputSequenceSize, 0);
        for (int i = 0; i < inputSequenceSize; ++i) {
            std::vector<uint8_t> inputStep;
            for (int b = 0; b < numBatches; ++b) {
                const uint8_t* batchStart = input[b].data() + i * inputSize;
                const uint8_t* batchEnd = batchStart + inputSize;
                inputStep.insert(inputStep.end(), batchStart, batchEnd);
            }
            lstm->setInput(inputStep);
            lstm->invoke();

            const int outputSize = lstm->outputSize();
            std::vector<float> expected;
            for (int b = 0; b < numBatches; ++b) {
                const uint8_t* goldenBatchStart = output[b].data() + i * outputSize;
                const uint8_t* goldenBatchEnd = goldenBatchStart + outputSize;
                expected.insert(expected.end(), goldenBatchStart, goldenBatchEnd);
            }
            EXPECT_THAT(lstm->getOutput(), ElementsAreArray(expected));
        }
    }
};

// Inputs and weights in this test are random and the test only checks that the
// outputs are equal to outputs obtained from running TF Lite version of
// quantized LSTM on the same inputs.
TEST_F(QuantizedLstmTest, BasicQuantizedLstmTest) {
    const int numBatches = 2;
    const int inputSize = 2;
    const int outputSize = 4;

    float weightsScale = 0.00408021;
    int weightsZeroPoint = 100;
    // OperandType biasOperandType(Type::TENSOR_INT32, input_shapes[3],
    // weightsScale / 128., 0);
    // inputs.push_back(model_.addOperand(&biasOperandType));
    // OperandType prevCellStateOperandType(Type::TENSOR_QUANT16_SYMM, input_shapes[4],
    // 1. / 2048., 0);
    // inputs.push_back(model_.addOperand(&prevCellStateOperandType));

    QuantizedLSTMOpModel lstm({
            // input
            OperandTypeParams(Type::TENSOR_QUANT8_ASYMM, {numBatches, inputSize}, 1. / 128., 128),
            // inputToInputWeights
            // inputToForgetWeights
            // inputToCellWeights
            // inputToOutputWeights
            OperandTypeParams(Type::TENSOR_QUANT8_ASYMM, {outputSize, inputSize}, weightsScale,
                              weightsZeroPoint),
            OperandTypeParams(Type::TENSOR_QUANT8_ASYMM, {outputSize, inputSize}, weightsScale,
                              weightsZeroPoint),
            OperandTypeParams(Type::TENSOR_QUANT8_ASYMM, {outputSize, inputSize}, weightsScale,
                              weightsZeroPoint),
            OperandTypeParams(Type::TENSOR_QUANT8_ASYMM, {outputSize, inputSize}, weightsScale,
                              weightsZeroPoint),
            // recurrentToInputWeights
            // recurrentToForgetWeights
            // recurrentToCellWeights
            // recurrentToOutputWeights
            OperandTypeParams(Type::TENSOR_QUANT8_ASYMM, {outputSize, outputSize}, weightsScale,
                              weightsZeroPoint),
            OperandTypeParams(Type::TENSOR_QUANT8_ASYMM, {outputSize, outputSize}, weightsScale,
                              weightsZeroPoint),
            OperandTypeParams(Type::TENSOR_QUANT8_ASYMM, {outputSize, outputSize}, weightsScale,
                              weightsZeroPoint),
            OperandTypeParams(Type::TENSOR_QUANT8_ASYMM, {outputSize, outputSize}, weightsScale,
                              weightsZeroPoint),
            // inputGateBias
            // forgetGateBias
            // cellGateBias
            // outputGateBias
            OperandTypeParams(Type::TENSOR_INT32, {outputSize}, weightsScale / 128., 0),
            OperandTypeParams(Type::TENSOR_INT32, {outputSize}, weightsScale / 128., 0),
            OperandTypeParams(Type::TENSOR_INT32, {outputSize}, weightsScale / 128., 0),
            OperandTypeParams(Type::TENSOR_INT32, {outputSize}, weightsScale / 128., 0),
            // prevCellState
            OperandTypeParams(Type::TENSOR_QUANT16_SYMM, {numBatches, outputSize}, 1. / 2048., 0),
            // prevOutput
            OperandTypeParams(Type::TENSOR_QUANT8_ASYMM, {numBatches, outputSize}, 1. / 128., 128),
    });

    lstm.setWeightsAndBiases(
            // inputToInputWeights
            {146, 250, 235, 171, 10, 218, 171, 108},
            // inputToForgetWeights
            {24, 50, 132, 179, 158, 110, 3, 169},
            // inputToCellWeights
            {133, 34, 29, 49, 206, 109, 54, 183},
            // inputToOutputWeights
            {195, 187, 11, 99, 109, 10, 218, 48},
            // recurrentToInputWeights
            {254, 206, 77, 168, 71, 20, 215, 6, 223, 7, 118, 225, 59, 130, 174, 26},
            // recurrentToForgetWeights
            {137, 240, 103, 52, 68, 51, 237, 112, 0, 220, 89, 23, 69, 4, 207, 253},
            // recurrentToCellWeights
            {172, 60, 205, 65, 14, 0, 140, 168, 240, 223, 133, 56, 142, 64, 246, 216},
            // recurrentToOutputWeights
            {106, 214, 67, 23, 59, 158, 45, 3, 119, 132, 49, 205, 129, 218, 11, 98},
            // inputGateBias
            {-7876, 13488, -726, 32839},
            // forgetGateBias
            {9206, -46884, -11693, -38724},
            // cellGateBias
            {39481, 48624, 48976, -21419},
            // outputGateBias
            {-58999, -17050, -41852, -40538});

    // LSTM input is stored as numBatches x (sequenceLength x inputSize) vector.
    std::vector<std::vector<uint8_t>> lstmInput;
    // clang-format off
    lstmInput = {{154, 166,
                  166, 179,
                  141, 141},
                 {100, 200,
                  50,  150,
                  111, 222}};
    // clang-format on

    // LSTM output is stored as numBatches x (sequenceLength x outputSize) vector.
    std::vector<std::vector<uint8_t>> lstmGoldenOutput;
    // clang-format off
    lstmGoldenOutput = {{136, 150, 140, 115,
                         140, 151, 146, 112,
                         139, 153, 146, 114},
                        {135, 152, 138, 112,
                         136, 156, 142, 112,
                         141, 154, 146, 108}};
    // clang-format on
    VerifyGoldens(lstmInput, lstmGoldenOutput, &lstm);
};

}  // namespace wrapper
}  // namespace nn
}  // namespace android
