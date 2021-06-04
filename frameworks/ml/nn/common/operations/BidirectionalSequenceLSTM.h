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

#ifndef FRAMEWORKS_ML_NN_BIDIRECTIONAL_SEQUENCE_LSTM_H
#define FRAMEWORKS_ML_NN_BIDIRECTIONAL_SEQUENCE_LSTM_H

#include "ActivationFunctor.h"
#include "HalOperation.h"
#include "LSTM.h"
#include "OperationsUtils.h"
#include "tensorflow/lite/kernels/internal/tensor_utils.h"

#include <algorithm>
#include <cmath>

namespace android {
namespace nn {

struct RunTimeOperandInfo;

class BidirectionalSequenceLSTM {
   public:
    BidirectionalSequenceLSTM(const Operation& operation,
                              std::vector<RunTimeOperandInfo>& operands);

    bool Prepare(const Operation& operation, std::vector<RunTimeOperandInfo>& operands,
                 Shape* fwOutputShape, Shape* bwOutputShape);
    bool Eval();

    // Input Tensors of size {max_time, n_batch, n_input}
    static constexpr int kInputTensor = 0;

    // Forward LSTM cell tensors.
    // Input weight tensors of size: {n_cell, n_input}
    static constexpr int kFwInputToInputWeightsTensor = 1;  // Optional
    static constexpr int kFwInputToForgetWeightsTensor = 2;
    static constexpr int kFwInputToCellWeightsTensor = 3;
    static constexpr int kFwInputToOutputWeightsTensor = 4;

    // Recurrent weight tensors of size {n_cell, n_output}
    static constexpr int kFwRecurrentToInputWeightsTensor = 5;  // Optional
    static constexpr int kFwRecurrentToForgetWeightsTensor = 6;
    static constexpr int kFwRecurrentToCellWeightsTensor = 7;
    static constexpr int kFwRecurrentToOutputWeightsTensor = 8;

    // Peephole weights tensors of size {n_cell}, representing a diagonal matrix.
    static constexpr int kFwCellToInputWeightsTensor = 9;    // Optional
    static constexpr int kFwCellToForgetWeightsTensor = 10;  // Optional
    static constexpr int kFwCellToOutputWeightsTensor = 11;  // Optional

    // Gates bias tensors of size {n_cell}
    static constexpr int kFwInputGateBiasTensor = 12;  // Optional
    static constexpr int kFwForgetGateBiasTensor = 13;
    static constexpr int kFwCellGateBiasTensor = 14;
    static constexpr int kFwOutputGateBiasTensor = 15;

    // Projection weight tensor of size {n_output, n_cell}
    static constexpr int kFwProjectionWeightsTensor = 16;  // Optional
    // Projection bias tensor of size {n_output}
    static constexpr int kFwProjectionBiasTensor = 17;  // Optional

    // Backward LSTM cell tensors.
    // Input weight tensors of size: {n_cell, n_input}
    static constexpr int kBwInputToInputWeightsTensor = 18;  // Optional
    static constexpr int kBwInputToForgetWeightsTensor = 19;
    static constexpr int kBwInputToCellWeightsTensor = 20;
    static constexpr int kBwInputToOutputWeightsTensor = 21;

    // Recurrent weight tensors of size {n_cell, n_output}
    static constexpr int kBwRecurrentToInputWeightsTensor = 22;  // Optional
    static constexpr int kBwRecurrentToForgetWeightsTensor = 23;
    static constexpr int kBwRecurrentToCellWeightsTensor = 24;
    static constexpr int kBwRecurrentToOutputWeightsTensor = 25;

    // Peephole weights tensors of size {n_cell}, representing a diagonal matrix.
    static constexpr int kBwCellToInputWeightsTensor = 26;   // Optional
    static constexpr int kBwCellToForgetWeightsTensor = 27;  // Optional
    static constexpr int kBwCellToOutputWeightsTensor = 28;  // Optional

    // Gates bias tensors of size {n_cell}
    static constexpr int kBwInputGateBiasTensor = 29;  // Optional
    static constexpr int kBwForgetGateBiasTensor = 30;
    static constexpr int kBwCellGateBiasTensor = 31;
    static constexpr int kBwOutputGateBiasTensor = 32;

    // Projection weight tensor of size {n_output, n_cell}
    static constexpr int kBwProjectionWeightsTensor = 33;  // Optional
    // Projection bias tensor of size {n_output}
    static constexpr int kBwProjectionBiasTensor = 34;  // Optional

    // Stateful input tensors that are variables and will be modified by the Op.
    // Activation state tensors of size {n_batch, n_output}
    static constexpr int kFwInputActivationStateTensor = 35;
    // Cell state tensors of size {n_batch, n_cell}
    static constexpr int kFwInputCellStateTensor = 36;
    // Activation state tensors of size {n_batch, n_output}
    static constexpr int kBwInputActivationStateTensor = 37;
    // Cell state tensors of size {n_batch, n_cell}
    static constexpr int kBwInputCellStateTensor = 38;

    // Used as auxiliary input and weights when stacking for
    // tf.contrib.rnn.stack_bidirectional_rnn case (with cross links); Used as input
    // to the backward cell when stacking for tf.nn.static_bidirectional_rnn case
    // (without cross links).
    static constexpr int kAuxInputTensor = 39;  // Optional
    // Forward weights.
    static constexpr int kFwAuxInputToInputWeightsTensor = 40;   // Optional
    static constexpr int kFwAuxInputToForgetWeightsTensor = 41;  // Optional
    static constexpr int kFwAuxInputToCellWeightsTensor = 42;    // Optional
    static constexpr int kFwAuxInputToOutputWeightsTensor = 43;  // Optional
    // Backward weights.
    static constexpr int kBwAuxInputToInputWeightsTensor = 44;   // Optional
    static constexpr int kBwAuxInputToForgetWeightsTensor = 45;  // Optional
    static constexpr int kBwAuxInputToCellWeightsTensor = 46;    // Optional
    static constexpr int kBwAuxInputToOutputWeightsTensor = 47;  // Optional

    static constexpr int kActivationParam = 48;
    static constexpr int kCellClipParam = 49;
    static constexpr int kProjClipParam = 50;
    static constexpr int kMergeOutputsParam = 51;
    static constexpr int kTimeMajorParam = 52;

    // Forward layer norm weights tensors of size {n_cell}, representing a diagonal matrix.
    static constexpr int kFwInputLayerNormWeightsTensor = 53;   // Optional
    static constexpr int kFwForgetLayerNormWeightsTensor = 54;  // Optional
    static constexpr int kFwCellLayerNormWeightsTensor = 55;    // Optional
    static constexpr int kFwOutputLayerNormWeightsTensor = 56;  // Optional
    // Backward layer norm weights tensors of size {n_cell}, representing a diagonal matrix.
    static constexpr int kBwInputLayerNormWeightsTensor = 57;   // Optional
    static constexpr int kBwForgetLayerNormWeightsTensor = 58;  // Optional
    static constexpr int kBwCellLayerNormWeightsTensor = 59;    // Optional
    static constexpr int kBwOutputLayerNormWeightsTensor = 60;  // Optional

    // Output tensors.
    static constexpr int kFwOutputTensor = 0;
    static constexpr int kBwOutputTensor = 1;  // Ignored if merge_outputs is set.

   private:
    LSTMParams params_;
    Shape fw_scratch_shape_;
    Shape bw_scratch_shape_;

    const RunTimeOperandInfo* input_;

    const RunTimeOperandInfo* aux_input_;
    const RunTimeOperandInfo* fw_aux_input_to_input_weights_;
    const RunTimeOperandInfo* fw_aux_input_to_forget_weights_;
    const RunTimeOperandInfo* fw_aux_input_to_cell_weights_;
    const RunTimeOperandInfo* fw_aux_input_to_output_weights_;
    const RunTimeOperandInfo* bw_aux_input_to_input_weights_;
    const RunTimeOperandInfo* bw_aux_input_to_forget_weights_;
    const RunTimeOperandInfo* bw_aux_input_to_cell_weights_;
    const RunTimeOperandInfo* bw_aux_input_to_output_weights_;

    const RunTimeOperandInfo* fw_input_to_input_weights_;
    const RunTimeOperandInfo* fw_input_to_forget_weights_;
    const RunTimeOperandInfo* fw_input_to_cell_weights_;
    const RunTimeOperandInfo* fw_input_to_output_weights_;

    const RunTimeOperandInfo* fw_recurrent_to_input_weights_;
    const RunTimeOperandInfo* fw_recurrent_to_forget_weights_;
    const RunTimeOperandInfo* fw_recurrent_to_cell_weights_;
    const RunTimeOperandInfo* fw_recurrent_to_output_weights_;

    const RunTimeOperandInfo* fw_cell_to_input_weights_;
    const RunTimeOperandInfo* fw_cell_to_forget_weights_;
    const RunTimeOperandInfo* fw_cell_to_output_weights_;

    const RunTimeOperandInfo* fw_input_gate_bias_;
    const RunTimeOperandInfo* fw_forget_gate_bias_;
    const RunTimeOperandInfo* fw_cell_bias_;
    const RunTimeOperandInfo* fw_output_gate_bias_;

    const RunTimeOperandInfo* fw_projection_weights_;
    const RunTimeOperandInfo* fw_projection_bias_;

    const RunTimeOperandInfo* fw_input_layer_norm_weights_;
    const RunTimeOperandInfo* fw_forget_layer_norm_weights_;
    const RunTimeOperandInfo* fw_cell_layer_norm_weights_;
    const RunTimeOperandInfo* fw_output_layer_norm_weights_;

    RunTimeOperandInfo* fw_activation_state_;
    RunTimeOperandInfo* fw_cell_state_;
    RunTimeOperandInfo* fw_output_;

    const RunTimeOperandInfo* bw_input_to_input_weights_;
    const RunTimeOperandInfo* bw_input_to_forget_weights_;
    const RunTimeOperandInfo* bw_input_to_cell_weights_;
    const RunTimeOperandInfo* bw_input_to_output_weights_;

    const RunTimeOperandInfo* bw_recurrent_to_input_weights_;
    const RunTimeOperandInfo* bw_recurrent_to_forget_weights_;
    const RunTimeOperandInfo* bw_recurrent_to_cell_weights_;
    const RunTimeOperandInfo* bw_recurrent_to_output_weights_;

    const RunTimeOperandInfo* bw_cell_to_input_weights_;
    const RunTimeOperandInfo* bw_cell_to_forget_weights_;
    const RunTimeOperandInfo* bw_cell_to_output_weights_;

    const RunTimeOperandInfo* bw_input_gate_bias_;
    const RunTimeOperandInfo* bw_forget_gate_bias_;
    const RunTimeOperandInfo* bw_cell_bias_;
    const RunTimeOperandInfo* bw_output_gate_bias_;

    const RunTimeOperandInfo* bw_projection_weights_;
    const RunTimeOperandInfo* bw_projection_bias_;

    const RunTimeOperandInfo* bw_input_layer_norm_weights_;
    const RunTimeOperandInfo* bw_forget_layer_norm_weights_;
    const RunTimeOperandInfo* bw_cell_layer_norm_weights_;
    const RunTimeOperandInfo* bw_output_layer_norm_weights_;

    RunTimeOperandInfo* bw_activation_state_;
    RunTimeOperandInfo* bw_cell_state_;
    RunTimeOperandInfo* bw_output_;
};

}  // namespace nn
}  // namespace android

#endif  // FRAMEWORKS_ML_NN_BIDIRECTIONAL_SEQUENCE_LSTM_H
