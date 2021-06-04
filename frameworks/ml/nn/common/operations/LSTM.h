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

#ifndef FRAMEWORKS_ML_NN_LSTMCELL_H
#define FRAMEWORKS_ML_NN_LSTMCELL_H

#include "ActivationFunctor.h"
#include "HalOperation.h"
#include "tensorflow/lite/kernels/internal/tensor_utils.h"

#include <algorithm>
#include <cmath>

namespace android {
namespace nn {

struct LSTMParams {
    TfLiteFusedActivation activation;
    float cell_clip;
    float proj_clip;
    bool use_cifg;
    bool use_peephole;
    bool use_layer_norm;
    bool use_projection_weight;
    bool use_projection_bias;
    bool merge_outputs;
    bool time_major;
};

struct RunTimeOperandInfo;
struct Shape;

class LSTMCell {
   public:
    LSTMCell(const Operation& operation, std::vector<RunTimeOperandInfo>& operands);

    bool Prepare(const Operation& operation, std::vector<RunTimeOperandInfo>& operands,
                 Shape* scratchShape, Shape* outputStateShape, Shape* cellStateShape,
                 Shape* outputShape);
    bool Eval();

    // Input Tensors of size {n_batch, n_input}
    static constexpr int kInputTensor = 0;

    // Input weight tensors of size: {n_cell, n_input}
    static constexpr int kInputToInputWeightsTensor = 1;  // Optional
    static constexpr int kInputToForgetWeightsTensor = 2;
    static constexpr int kInputToCellWeightsTensor = 3;
    static constexpr int kInputToOutputWeightsTensor = 4;

    // Recurrent weight tensors of size {n_cell, n_output}
    static constexpr int kRecurrentToInputWeightsTensor = 5;  // Optional
    static constexpr int kRecurrentToForgetWeightsTensor = 6;
    static constexpr int kRecurrentToCellWeightsTensor = 7;
    static constexpr int kRecurrentToOutputWeightsTensor = 8;

    // Peephole weights tensors of size {n_cell}, representing a diagonal matrix.
    static constexpr int kCellToInputWeightsTensor = 9;    // Optional
    static constexpr int kCellToForgetWeightsTensor = 10;  // Optional
    static constexpr int kCellToOutputWeightsTensor = 11;  // Optional

    // Gates bias tensors of size {n_cell}
    static constexpr int kInputGateBiasTensor = 12;  // Optional
    static constexpr int kForgetGateBiasTensor = 13;
    static constexpr int kCellGateBiasTensor = 14;
    static constexpr int kOutputGateBiasTensor = 15;

    // Projection weight tensor of size {n_output, n_cell}
    static constexpr int kProjectionWeightsTensor = 16;  // Optional
    // Projection bias tensor of size {n_output}
    static constexpr int kProjectionBiasTensor = 17;  // Optional

    static constexpr int kOutputStateInTensor = 18;
    static constexpr int kCellStateInTensor = 19;

    static constexpr int kActivationParam = 20;
    static constexpr int kCellClipParam = 21;
    static constexpr int kProjClipParam = 22;

    // Layer norm weights tensors of size {n_cell}, representing a diagonal matrix.
    static constexpr int kInputLayerNormWeightsTensor = 23;
    static constexpr int kForgetLayerNormWeightsTensor = 24;
    static constexpr int kCellLayerNormWeightsTensor = 25;
    static constexpr int kOutputLayerNormWeightsTensor = 26;

    // Output tensors.
    static constexpr int kScratchBufferTensor = 0;
    static constexpr int kOutputStateOutTensor = 1;
    static constexpr int kCellStateOutTensor = 2;
    static constexpr int kOutputTensor = 3;

    static constexpr float kLayerNormEpsilon = 1e-8;

    static bool LSTMEvalFloat32(
            const LSTMParams& params, const float* input_buffer, const Shape& input_shape,
            const float* input_to_input_weights_buffer, const float* input_to_forget_weights_buffer,
            const float* input_to_cell_weights_buffer, const float* input_to_output_weights_buffer,
            const Shape& input_to_output_weights_shape,
            const float* recurrent_to_input_weights_buffer,
            const float* recurrent_to_forget_weights_buffer,
            const float* recurrent_to_cell_weights_buffer,
            const float* recurrent_to_output_weights_buffer,
            const Shape& recurrent_to_output_weights_shape,
            const float* cell_to_input_weights_buffer, const float* cell_to_forget_weights_buffer,
            const float* cell_to_output_weights_buffer, const float* aux_input_buffer,
            const float* aux_input_to_input_weights, const float* aux_input_to_forget_weights,
            const float* aux_input_to_cell_weights, const float* aux_input_to_output_weights,
            const float* input_gate_bias_buffer, const float* forget_gate_bias_buffer,
            const float* cell_bias_buffer, const float* output_gate_bias_buffer,
            const float* projection_weights_buffer, const float* projection_bias_buffer,
            const float* output_state_in_buffer, const float* cell_state_in_buffer,
            const float* input_layer_norm_weights_buffer,
            const float* forget_layer_norm_weights_buffer,
            const float* cell_layer_norm_weights_buffer,
            const float* output_layer_norm_weights_buffer, float* output_state_out_buffer,
            float* cell_state_out_buffer, float* output_buffer, float* scratch_buffer_buffer,
            bool timeMajor = true, bool forwardSequence = true);

    static bool LSTMEvalFloat16(
            const LSTMParams& params, const _Float16* input_buffer, const Shape& input_shape,
            const _Float16* input_to_input_weights_buffer,
            const _Float16* input_to_forget_weights_buffer,
            const _Float16* input_to_cell_weights_buffer,
            const _Float16* input_to_output_weights_buffer,
            const Shape& input_to_output_weights_shape,
            const _Float16* recurrent_to_input_weights_buffer,
            const _Float16* recurrent_to_forget_weights_buffer,
            const _Float16* recurrent_to_cell_weights_buffer,
            const _Float16* recurrent_to_output_weights_buffer,
            const Shape& recurrent_to_output_weights_shape,
            const _Float16* cell_to_input_weights_buffer,
            const _Float16* cell_to_forget_weights_buffer,
            const _Float16* cell_to_output_weights_buffer, const _Float16* aux_input_buffer,
            const _Float16* aux_input_to_input_weights, const _Float16* aux_input_to_forget_weights,
            const _Float16* aux_input_to_cell_weights, const _Float16* aux_input_to_output_weights,
            const _Float16* input_gate_bias_buffer, const _Float16* forget_gate_bias_buffer,
            const _Float16* cell_bias_buffer, const _Float16* output_gate_bias_buffer,
            const _Float16* projection_weights_buffer, const _Float16* projection_bias_buffer,
            const _Float16* output_state_in_buffer, const _Float16* cell_state_in_buffer,
            const _Float16* input_layer_norm_weights_buffer,
            const _Float16* forget_layer_norm_weights_buffer,
            const _Float16* cell_layer_norm_weights_buffer,
            const _Float16* output_layer_norm_weights_buffer, _Float16* output_state_out_buffer,
            _Float16* cell_state_out_buffer, _Float16* output_buffer,
            _Float16* scratch_buffer_buffer, bool timeMajor = true, bool forwardSequence = true);

    static bool LSTMStep(
            const LSTMParams& params, const float* input_buffer, const Shape& input_shape,
            const float* input_to_input_weights_buffer, const float* input_to_forget_weights_buffer,
            const float* input_to_cell_weights_buffer, const float* input_to_output_weights_buffer,
            const Shape& input_to_output_weights_shape,
            const float* recurrent_to_input_weights_buffer,
            const float* recurrent_to_forget_weights_buffer,
            const float* recurrent_to_cell_weights_buffer,
            const float* recurrent_to_output_weights_buffer,
            const Shape& recurrent_to_output_weights_shape,
            const float* cell_to_input_weights_buffer, const float* cell_to_forget_weights_buffer,
            const float* cell_to_output_weights_buffer, const float* aux_input_buffer,
            const float* aux_input_to_input_weights, const float* aux_input_to_forget_weights,
            const float* aux_input_to_cell_weights, const float* aux_input_to_output_weights,
            const float* input_gate_bias_buffer, const float* forget_gate_bias_buffer,
            const float* cell_bias_buffer, const float* output_gate_bias_buffer,
            const float* projection_weights_buffer, const float* projection_bias_buffer,
            const float* output_state_in_buffer, const float* cell_state_in_buffer,
            const float* input_layer_norm_weights_buffer,
            const float* forget_layer_norm_weights_buffer,
            const float* cell_layer_norm_weights_buffer,
            const float* output_layer_norm_weights_buffer, float* output_state_out_buffer,
            float* cell_state_out_buffer, float* output_buffer, float* scratch_buffer_buffer);

    static bool CheckInputTensorDimensions(
            const RunTimeOperandInfo* input_, const RunTimeOperandInfo* input_to_input_weights,
            const RunTimeOperandInfo* input_to_forget_weights,
            const RunTimeOperandInfo* input_to_cell_weights,
            const RunTimeOperandInfo* input_to_output_weights,
            const RunTimeOperandInfo* recurrent_to_input_weights,
            const RunTimeOperandInfo* recurrent_to_forget_weights,
            const RunTimeOperandInfo* recurrent_to_cell_weights,
            const RunTimeOperandInfo* recurrent_to_output_weights,
            const RunTimeOperandInfo* cell_to_input_weights,
            const RunTimeOperandInfo* cell_to_forget_weights,
            const RunTimeOperandInfo* cell_to_output_weights,
            const RunTimeOperandInfo* input_gate_bias, const RunTimeOperandInfo* forget_gate_bias,
            const RunTimeOperandInfo* cell_bias, const RunTimeOperandInfo* output_gate_bias,
            const RunTimeOperandInfo* projection_weights, const RunTimeOperandInfo* projection_bias,
            const RunTimeOperandInfo* input_layer_norm_weights,
            const RunTimeOperandInfo* forget_layer_norm_weights,
            const RunTimeOperandInfo* cell_layer_norm_weights,
            const RunTimeOperandInfo* output_layer_norm_weights, uint32_t n_input,
            uint32_t n_output, uint32_t n_cell, LSTMParams* params);

   private:
    LSTMParams params_;
    const RunTimeOperandInfo* input_;

    const RunTimeOperandInfo* input_to_input_weights_;
    const RunTimeOperandInfo* input_to_forget_weights_;
    const RunTimeOperandInfo* input_to_cell_weights_;
    const RunTimeOperandInfo* input_to_output_weights_;

    const RunTimeOperandInfo* recurrent_to_input_weights_;
    const RunTimeOperandInfo* recurrent_to_forget_weights_;
    const RunTimeOperandInfo* recurrent_to_cell_weights_;
    const RunTimeOperandInfo* recurrent_to_output_weights_;

    const RunTimeOperandInfo* cell_to_input_weights_;
    const RunTimeOperandInfo* cell_to_forget_weights_;
    const RunTimeOperandInfo* cell_to_output_weights_;

    const RunTimeOperandInfo* input_gate_bias_;
    const RunTimeOperandInfo* forget_gate_bias_;
    const RunTimeOperandInfo* cell_bias_;
    const RunTimeOperandInfo* output_gate_bias_;

    const RunTimeOperandInfo* projection_weights_;
    const RunTimeOperandInfo* projection_bias_;

    const RunTimeOperandInfo* output_state_in_;
    const RunTimeOperandInfo* cell_state_in_;

    const RunTimeOperandInfo* input_layer_norm_weights_;
    const RunTimeOperandInfo* forget_layer_norm_weights_;
    const RunTimeOperandInfo* cell_layer_norm_weights_;
    const RunTimeOperandInfo* output_layer_norm_weights_;

    RunTimeOperandInfo* output_state_out_;
    RunTimeOperandInfo* cell_state_out_;
    RunTimeOperandInfo* output_;

    RunTimeOperandInfo* scratch_buffer_;
};

}  // namespace nn
}  // namespace android

#endif  // FRAMEWORKS_ML_NN_LSTMCELL_H
