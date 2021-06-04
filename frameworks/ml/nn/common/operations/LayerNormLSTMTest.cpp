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
#include "LSTM.h"

#include <android-base/logging.h>

#include "NeuralNetworksWrapper.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

#include <sstream>
#include <string>
#include <vector>

namespace android {
namespace nn {
namespace wrapper {

using ::testing::Each;
using ::testing::FloatNear;
using ::testing::Matcher;

namespace {

std::vector<Matcher<float>> ArrayFloatNear(const std::vector<float>& values,
                                           float max_abs_error = 1.e-6) {
    std::vector<Matcher<float>> matchers;
    matchers.reserve(values.size());
    for (const float& v : values) {
        matchers.emplace_back(FloatNear(v, max_abs_error));
    }
    return matchers;
}

}  // anonymous namespace

#define FOR_ALL_INPUT_AND_WEIGHT_TENSORS(ACTION) \
    ACTION(Input)                                \
    ACTION(InputToInputWeights)                  \
    ACTION(InputToCellWeights)                   \
    ACTION(InputToForgetWeights)                 \
    ACTION(InputToOutputWeights)                 \
    ACTION(RecurrentToInputWeights)              \
    ACTION(RecurrentToCellWeights)               \
    ACTION(RecurrentToForgetWeights)             \
    ACTION(RecurrentToOutputWeights)             \
    ACTION(CellToInputWeights)                   \
    ACTION(CellToForgetWeights)                  \
    ACTION(CellToOutputWeights)                  \
    ACTION(InputGateBias)                        \
    ACTION(CellGateBias)                         \
    ACTION(ForgetGateBias)                       \
    ACTION(OutputGateBias)                       \
    ACTION(ProjectionWeights)                    \
    ACTION(ProjectionBias)                       \
    ACTION(OutputStateIn)                        \
    ACTION(CellStateIn)

#define FOR_ALL_LAYER_NORM_WEIGHTS(ACTION) \
    ACTION(InputLayerNormWeights)          \
    ACTION(ForgetLayerNormWeights)         \
    ACTION(CellLayerNormWeights)           \
    ACTION(OutputLayerNormWeights)

// For all output and intermediate states
#define FOR_ALL_OUTPUT_TENSORS(ACTION) \
    ACTION(ScratchBuffer)              \
    ACTION(OutputStateOut)             \
    ACTION(CellStateOut)               \
    ACTION(Output)

class LayerNormLSTMOpModel {
   public:
    LayerNormLSTMOpModel(uint32_t n_batch, uint32_t n_input, uint32_t n_cell, uint32_t n_output,
                         bool use_cifg, bool use_peephole, bool use_projection_weights,
                         bool use_projection_bias, float cell_clip, float proj_clip,
                         const std::vector<std::vector<uint32_t>>& input_shapes0)
        : n_input_(n_input),
          n_output_(n_output),
          use_cifg_(use_cifg),
          use_peephole_(use_peephole),
          use_projection_weights_(use_projection_weights),
          use_projection_bias_(use_projection_bias),
          activation_(ActivationFn::kActivationTanh),
          cell_clip_(cell_clip),
          proj_clip_(proj_clip) {
        std::vector<uint32_t> inputs;
        std::vector<std::vector<uint32_t>> input_shapes(input_shapes0);

        auto it = input_shapes.begin();

        // Input and weights
#define AddInput(X)                                     \
    CHECK(it != input_shapes.end());                    \
    OperandType X##OpndTy(Type::TENSOR_FLOAT32, *it++); \
    inputs.push_back(model_.addOperand(&X##OpndTy));

        FOR_ALL_INPUT_AND_WEIGHT_TENSORS(AddInput);

        // Parameters
        OperandType ActivationOpndTy(Type::INT32, {});
        inputs.push_back(model_.addOperand(&ActivationOpndTy));
        OperandType CellClipOpndTy(Type::FLOAT32, {});
        inputs.push_back(model_.addOperand(&CellClipOpndTy));
        OperandType ProjClipOpndTy(Type::FLOAT32, {});
        inputs.push_back(model_.addOperand(&ProjClipOpndTy));

        FOR_ALL_LAYER_NORM_WEIGHTS(AddInput);

#undef AddOperand

        // Output and other intermediate state
        std::vector<std::vector<uint32_t>> output_shapes{
                {n_batch, n_cell * (use_cifg ? 3 : 4)},
                {n_batch, n_output},
                {n_batch, n_cell},
                {n_batch, n_output},
        };
        std::vector<uint32_t> outputs;

        auto it2 = output_shapes.begin();

#define AddOutput(X)                                     \
    CHECK(it2 != output_shapes.end());                   \
    OperandType X##OpndTy(Type::TENSOR_FLOAT32, *it2++); \
    outputs.push_back(model_.addOperand(&X##OpndTy));

        FOR_ALL_OUTPUT_TENSORS(AddOutput);

#undef AddOutput

        model_.addOperation(ANEURALNETWORKS_LSTM, inputs, outputs);
        model_.identifyInputsAndOutputs(inputs, outputs);

        Input_.insert(Input_.end(), n_batch * n_input, 0.f);
        OutputStateIn_.insert(OutputStateIn_.end(), n_batch * n_output, 0.f);
        CellStateIn_.insert(CellStateIn_.end(), n_batch * n_cell, 0.f);

        auto multiAll = [](const std::vector<uint32_t>& dims) -> uint32_t {
            uint32_t sz = 1;
            for (uint32_t d : dims) {
                sz *= d;
            }
            return sz;
        };

        it2 = output_shapes.begin();

#define ReserveOutput(X) X##_.insert(X##_.end(), multiAll(*it2++), 0.f);

        FOR_ALL_OUTPUT_TENSORS(ReserveOutput);

#undef ReserveOutput

        model_.finish();
    }

#define DefineSetter(X) \
    void Set##X(const std::vector<float>& f) { X##_.insert(X##_.end(), f.begin(), f.end()); }

    FOR_ALL_INPUT_AND_WEIGHT_TENSORS(DefineSetter);
    FOR_ALL_LAYER_NORM_WEIGHTS(DefineSetter);

#undef DefineSetter

    void ResetOutputState() {
        std::fill(OutputStateIn_.begin(), OutputStateIn_.end(), 0.f);
        std::fill(OutputStateOut_.begin(), OutputStateOut_.end(), 0.f);
    }

    void ResetCellState() {
        std::fill(CellStateIn_.begin(), CellStateIn_.end(), 0.f);
        std::fill(CellStateOut_.begin(), CellStateOut_.end(), 0.f);
    }

    void SetInput(int offset, const float* begin, const float* end) {
        for (; begin != end; begin++, offset++) {
            Input_[offset] = *begin;
        }
    }

    uint32_t num_inputs() const { return n_input_; }
    uint32_t num_outputs() const { return n_output_; }

    const std::vector<float>& GetOutput() const { return Output_; }

    void Invoke() {
        ASSERT_TRUE(model_.isValid());

        OutputStateIn_.swap(OutputStateOut_);
        CellStateIn_.swap(CellStateOut_);

        Compilation compilation(&model_);
        compilation.finish();
        Execution execution(&compilation);
#define SetInputOrWeight(X)                                                                       \
    ASSERT_EQ(                                                                                    \
            execution.setInput(LSTMCell::k##X##Tensor, X##_.data(), sizeof(float) * X##_.size()), \
            Result::NO_ERROR);

        FOR_ALL_INPUT_AND_WEIGHT_TENSORS(SetInputOrWeight);
        FOR_ALL_LAYER_NORM_WEIGHTS(SetInputOrWeight);

#undef SetInputOrWeight

#define SetOutput(X)                                                                               \
    ASSERT_EQ(                                                                                     \
            execution.setOutput(LSTMCell::k##X##Tensor, X##_.data(), sizeof(float) * X##_.size()), \
            Result::NO_ERROR);

        FOR_ALL_OUTPUT_TENSORS(SetOutput);

#undef SetOutput

        if (use_cifg_) {
            execution.setInput(LSTMCell::kInputToInputWeightsTensor, nullptr, 0);
            execution.setInput(LSTMCell::kRecurrentToInputWeightsTensor, nullptr, 0);
        }

        if (use_peephole_) {
            if (use_cifg_) {
                execution.setInput(LSTMCell::kCellToInputWeightsTensor, nullptr, 0);
            }
        } else {
            execution.setInput(LSTMCell::kCellToInputWeightsTensor, nullptr, 0);
            execution.setInput(LSTMCell::kCellToForgetWeightsTensor, nullptr, 0);
            execution.setInput(LSTMCell::kCellToOutputWeightsTensor, nullptr, 0);
        }

        if (use_projection_weights_) {
            if (!use_projection_bias_) {
                execution.setInput(LSTMCell::kProjectionBiasTensor, nullptr, 0);
            }
        } else {
            execution.setInput(LSTMCell::kProjectionWeightsTensor, nullptr, 0);
            execution.setInput(LSTMCell::kProjectionBiasTensor, nullptr, 0);
        }

        ASSERT_EQ(execution.setInput(LSTMCell::kActivationParam, &activation_, sizeof(activation_)),
                  Result::NO_ERROR);
        ASSERT_EQ(execution.setInput(LSTMCell::kCellClipParam, &cell_clip_, sizeof(cell_clip_)),
                  Result::NO_ERROR);
        ASSERT_EQ(execution.setInput(LSTMCell::kProjClipParam, &proj_clip_, sizeof(proj_clip_)),
                  Result::NO_ERROR);

        ASSERT_EQ(execution.compute(), Result::NO_ERROR);
    }

   private:
    Model model_;
    // Execution execution_;
    const uint32_t n_input_;
    const uint32_t n_output_;

    const bool use_cifg_;
    const bool use_peephole_;
    const bool use_projection_weights_;
    const bool use_projection_bias_;

    const int activation_;
    const float cell_clip_;
    const float proj_clip_;

#define DefineTensor(X) std::vector<float> X##_;

    FOR_ALL_INPUT_AND_WEIGHT_TENSORS(DefineTensor);
    FOR_ALL_LAYER_NORM_WEIGHTS(DefineTensor);
    FOR_ALL_OUTPUT_TENSORS(DefineTensor);

#undef DefineTensor
};

TEST(LSTMOpTest, LayerNormNoCifgPeepholeProjectionNoClipping) {
    const int n_batch = 2;
    const int n_input = 5;
    // n_cell and n_output have the same size when there is no projection.
    const int n_cell = 4;
    const int n_output = 3;

    LayerNormLSTMOpModel lstm(n_batch, n_input, n_cell, n_output,
                              /*use_cifg=*/false, /*use_peephole=*/true,
                              /*use_projection_weights=*/true,
                              /*use_projection_bias=*/false,
                              /*cell_clip=*/0.0, /*proj_clip=*/0.0,
                              {
                                      {n_batch, n_input},  // input tensor

                                      {n_cell, n_input},  // input_to_input_weight tensor
                                      {n_cell, n_input},  // input_to_forget_weight tensor
                                      {n_cell, n_input},  // input_to_cell_weight tensor
                                      {n_cell, n_input},  // input_to_output_weight tensor

                                      {n_cell, n_output},  // recurrent_to_input_weight tensor
                                      {n_cell, n_output},  // recurrent_to_forget_weight tensor
                                      {n_cell, n_output},  // recurrent_to_cell_weight tensor
                                      {n_cell, n_output},  // recurrent_to_output_weight tensor

                                      {n_cell},  // cell_to_input_weight tensor
                                      {n_cell},  // cell_to_forget_weight tensor
                                      {n_cell},  // cell_to_output_weight tensor

                                      {n_cell},  // input_gate_bias tensor
                                      {n_cell},  // forget_gate_bias tensor
                                      {n_cell},  // cell_bias tensor
                                      {n_cell},  // output_gate_bias tensor

                                      {n_output, n_cell},  // projection_weight tensor
                                      {0},                 // projection_bias tensor

                                      {n_batch, n_output},  // output_state_in tensor
                                      {n_batch, n_cell},    // cell_state_in tensor

                                      {n_cell},  // input_layer_norm_weights tensor
                                      {n_cell},  // forget_layer_norm_weights tensor
                                      {n_cell},  // cell_layer_norm_weights tensor
                                      {n_cell},  // output_layer_norm_weights tensor
                              });

    lstm.SetInputToInputWeights({0.5,  0.6, 0.7,  -0.8, -0.9, 0.1,  0.2,  0.3,  -0.4, 0.5,
                                 -0.8, 0.7, -0.6, 0.5,  -0.4, -0.5, -0.4, -0.3, -0.2, -0.1});

    lstm.SetInputToForgetWeights({-0.6, -0.1, 0.3,  0.2,  0.9,  -0.5, -0.2, -0.4, 0.3,  -0.8,
                                  -0.4, 0.3,  -0.5, -0.4, -0.6, 0.3,  -0.4, -0.6, -0.5, -0.5});

    lstm.SetInputToCellWeights({-0.4, -0.3, -0.2, -0.1, -0.5, 0.5, -0.2, -0.3, -0.2, -0.6,
                                0.6,  -0.1, -0.4, -0.3, -0.7, 0.7, -0.9, -0.5, 0.8,  0.6});

    lstm.SetInputToOutputWeights({-0.8, -0.4, -0.2, -0.9, -0.1, -0.7, 0.3, -0.3, -0.8, -0.2,
                                  0.6,  -0.2, 0.4,  -0.7, -0.3, -0.5, 0.1, 0.5,  -0.6, -0.4});

    lstm.SetInputGateBias({0.03, 0.15, 0.22, 0.38});

    lstm.SetForgetGateBias({0.1, -0.3, -0.2, 0.1});

    lstm.SetCellGateBias({-0.05, 0.72, 0.25, 0.08});

    lstm.SetOutputGateBias({0.05, -0.01, 0.2, 0.1});

    lstm.SetRecurrentToInputWeights(
            {-0.2, -0.3, 0.4, 0.1, -0.5, 0.9, -0.2, -0.3, -0.7, 0.05, -0.2, -0.6});

    lstm.SetRecurrentToCellWeights(
            {-0.3, 0.2, 0.1, -0.3, 0.8, -0.08, -0.2, 0.3, 0.8, -0.6, -0.1, 0.2});

    lstm.SetRecurrentToForgetWeights(
            {-0.5, -0.3, -0.5, -0.2, 0.6, 0.4, 0.9, 0.3, -0.1, 0.2, 0.5, 0.2});

    lstm.SetRecurrentToOutputWeights(
            {0.3, -0.1, 0.1, -0.2, -0.5, -0.7, -0.2, -0.6, -0.1, -0.4, -0.7, -0.2});

    lstm.SetCellToInputWeights({0.05, 0.1, 0.25, 0.15});
    lstm.SetCellToForgetWeights({-0.02, -0.15, -0.25, -0.03});
    lstm.SetCellToOutputWeights({0.1, -0.1, -0.5, 0.05});

    lstm.SetProjectionWeights({-0.1, 0.2, 0.01, -0.2, 0.1, 0.5, 0.3, 0.08, 0.07, 0.2, -0.4, 0.2});

    lstm.SetInputLayerNormWeights({0.1, 0.2, 0.3, 0.5});
    lstm.SetForgetLayerNormWeights({0.2, 0.2, 0.4, 0.3});
    lstm.SetCellLayerNormWeights({0.7, 0.2, 0.3, 0.8});
    lstm.SetOutputLayerNormWeights({0.6, 0.2, 0.2, 0.5});

    const std::vector<std::vector<float>> lstm_input = {
            {                           // Batch0: 3 (input_sequence_size) * 5 (n_input)
             0.7, 0.8, 0.1, 0.2, 0.3,   // seq 0
             0.8, 0.1, 0.2, 0.4, 0.5,   // seq 1
             0.2, 0.7, 0.7, 0.1, 0.7},  // seq 2

            {                           // Batch1: 3 (input_sequence_size) * 5 (n_input)
             0.3, 0.2, 0.9, 0.8, 0.1,   // seq 0
             0.1, 0.5, 0.2, 0.4, 0.2,   // seq 1
             0.6, 0.9, 0.2, 0.5, 0.7},  // seq 2
    };

    const std::vector<std::vector<float>> lstm_golden_output = {
            {
                    // Batch0: 3 (input_sequence_size) * 3 (n_output)
                    0.0244077, 0.128027, -0.00170918,  // seq 0
                    0.0137642, 0.140751, 0.0395835,    // seq 1
                    -0.00459231, 0.155278, 0.0837377,  // seq 2
            },
            {
                    // Batch1: 3 (input_sequence_size) * 3 (n_output)
                    -0.00692428, 0.0848741, 0.063445,  // seq 0
                    -0.00403912, 0.139963, 0.072681,   // seq 1
                    0.00752706, 0.161903, 0.0561371,   // seq 2
            }};

    // Resetting cell_state and output_state
    lstm.ResetCellState();
    lstm.ResetOutputState();

    const int input_sequence_size = lstm_input[0].size() / n_input;
    for (int i = 0; i < input_sequence_size; i++) {
        for (int b = 0; b < n_batch; ++b) {
            const float* batch_start = lstm_input[b].data() + i * n_input;
            const float* batch_end = batch_start + n_input;

            lstm.SetInput(b * n_input, batch_start, batch_end);
        }

        lstm.Invoke();

        std::vector<float> expected;
        for (int b = 0; b < n_batch; ++b) {
            const float* golden_start = lstm_golden_output[b].data() + i * n_output;
            const float* golden_end = golden_start + n_output;
            expected.insert(expected.end(), golden_start, golden_end);
        }
        EXPECT_THAT(lstm.GetOutput(), ElementsAreArray(ArrayFloatNear(expected)));
    }
}

}  // namespace wrapper
}  // namespace nn
}  // namespace android
