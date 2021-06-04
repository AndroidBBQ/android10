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

#include "CpuExecutor.h"
#include "CpuOperationUtils.h"

#include "Tracing.h"

#include "public/gemmlowp.h"
#include "tensorflow/lite/kernels/internal/reference/legacy_reference_ops.h"

namespace android {
namespace nn {

namespace {

template <typename T>
inline T* GetBuffer(RunTimeOperandInfo* operand) {
    return reinterpret_cast<T*>(operand->buffer);
}

template <typename T>
inline const T* GetBuffer(const RunTimeOperandInfo* operand) {
    return reinterpret_cast<const T*>(operand->buffer);
}

using tflite::Dims;

// The function below is taken from TF Lite implementation in order to decouple
// NN API from TF Lite dependency. Original function, with a description of its
// parameters and types can be found by this link:
// https://github.com/tensorflow/tensorflow/blob/0d697e5fc4c05c699eea0764364104ea500ccc68/tensorflow/contrib/lite/kernels/internal/reference/reference_ops.h#L1926
//
// clang-format off
template <int StateIntegerBits>
void quantizedLstmStep(const uint8_t* input_data_uint8, const Dims<4>& input_dims,
                       const uint8_t* prev_activ_data_uint8,
                       const Dims<4>& prev_activ_dims, const uint8_t* weights_data_uint8,
                       const Dims<4>& weights_dims, const int32_t* bias_data_int32,
                       const Dims<4>& bias_dims, const int16_t* prevCellState_data_int16,
                       const Dims<4>& prevCellState_dims, int16_t* output_state_data_int16,
                       const Dims<4>& output_state_dims, uint8_t* output_activ_data_uint8,
                       const Dims<4>& output_activ_dims, uint8_t* concat_temp_data_uint8,
                       const Dims<4>& concat_temp_dims, int16_t* activ_temp_data_int16,
                       const Dims<4>& activ_temp_dims, int32_t weights_zero_point,
                       int32_t accum_multiplier, int accum_shift) {
  // Gather dimensions information, and perform consistency checks.
  const int outer_size =
      MatchingFlatSizeSkipDim(input_dims, 0, prev_activ_dims, prevCellState_dims,
                              output_state_dims, output_activ_dims);
  TFLITE_CHECK_EQ(ArraySize(weights_dims, 2), 1);
  TFLITE_CHECK_EQ(ArraySize(weights_dims, 3), 1);
  const int input_depth = ArraySize(input_dims, 0);
  const int prev_activ_depth = ArraySize(prev_activ_dims, 0);
  const int total_input_depth = prev_activ_depth + input_depth;
  TFLITE_CHECK_EQ(ArraySize(weights_dims, 0), total_input_depth);
  TFLITE_CHECK_EQ(MatchingArraySize(bias_dims, 1, bias_dims, 2, bias_dims, 3),
                  1);
  const int intern_activ_depth =
      MatchingArraySize(weights_dims, 1, bias_dims, 0);
  TFLITE_CHECK_EQ(intern_activ_depth % 4, 0);
  const int output_depth =
      MatchingArraySize(prevCellState_dims, 0, prev_activ_dims, 0,
                        output_state_dims, 0, output_activ_dims, 0);
  TFLITE_CHECK_EQ(output_depth, intern_activ_depth / 4);
  const int fc_batches = FlatSizeSkipDim(activ_temp_dims, 0);
  const int fc_output_depth =
      MatchingArraySize(weights_dims, 1, activ_temp_dims, 0);
  const int fc_accum_depth = ArraySize(weights_dims, 0);
  TFLITE_CHECK_EQ(fc_output_depth, 4 * output_depth);

  // Depth-concatenate prev_activ and input data together.
  uint8_t const* concat_input_arrays_data[2] = {input_data_uint8,
                                                prev_activ_data_uint8};
  Dims<4> const* concat_input_arrays_dims[2] = {&input_dims, &prev_activ_dims};
  tflite::reference_ops::Concatenation<tflite::FusedActivationFunctionType::kNone, uint8_t>(
      0, concat_input_arrays_data, concat_input_arrays_dims, 2,
      concat_temp_data_uint8, concat_temp_dims);

  // Implementation of the fully connected node inside the LSTM cell.
  // The operands are 8-bit integers, the accumulators are internally 32bit
  // integers, and the output is 16-bit fixed-point with 3 integer bits so
  // the output range is [-2^3, 2^3] == [-8, 8]. The rationale for that
  // is explained in the function comment above.
  for (int b = 0; b < fc_batches; ++b) {
    for (int out_c = 0; out_c < fc_output_depth; ++out_c) {
      // Internal accumulation.
      // Initialize accumulator with the bias-value.
      int32_t accum = bias_data_int32[out_c];
      // Accumulation loop.
      for (int d = 0; d < fc_accum_depth; ++d) {
        int16_t input_val = concat_temp_data_uint8[b * fc_accum_depth + d] - 128;
        int16_t weights_val =
            weights_data_uint8[out_c * fc_accum_depth + d] - weights_zero_point;
        accum += input_val * weights_val;
      }
      // Down-scale the final int32 accumulator to the scale used by our
      // (16-bit, using 3 integer bits) fixed-point format. The quantized
      // multiplier and shift here have been pre-computed offline
      // (e.g. by toco).
      accum =
          tflite::MultiplyByQuantizedMultiplier(accum, accum_multiplier, accum_shift);
      // Saturate, cast to int16, and store to the temporary activations array.
      accum = std::max(-32768, std::min(32767, accum));
      activ_temp_data_int16[out_c + fc_output_depth * b] = accum;
    }
  }

  // Rest of the LSTM cell: tanh and logistic math functions, and some adds
  // and muls, all done in 16-bit fixed-point.
  for (int b = 0; b < outer_size; ++b) {
    for (int c = 0; c < output_depth; ++c) {
      // Define the fixed-point data types that we will use here. All use
      // int16 as the underlying integer type i.e. all are 16-bit fixed-point.
      // They only differ by the number of integral vs. fractional bits,
      // determining the range of values that they can represent.
      //
      // F0 uses 0 integer bits, range [-1, 1].
      // This is the return type of math functions such as tanh, logistic,
      // whose range is in [-1, 1].
      using F0 = gemmlowp::FixedPoint<std::int16_t, 0>;
      // F3 uses 3 integer bits, range [-8, 8].
      // This is the range of the previous fully-connected node's output,
      // which is our input here.
      using F3 = gemmlowp::FixedPoint<std::int16_t, 3>;
      // FS uses StateIntegerBits integer bits, range [-2^StateIntegerBits,
      // 2^StateIntegerBits]. It's used to represent the internal state, whose
      // number of integer bits is currently dictated by the model. See comment
      // on the StateIntegerBits template parameter above.
      using FS = gemmlowp::FixedPoint<std::int16_t, StateIntegerBits>;
      // Implementation of input gate, using fixed-point logistic function.
      F3 input_gate_input = F3::FromRaw(
          activ_temp_data_int16[b * fc_output_depth + 0 * output_depth + c]);
      F0 input_gate_output = gemmlowp::logistic(input_gate_input);
      // Implementation of input modulation gate, using fixed-point tanh
      // function.
      F3 input_modulation_gate_input = F3::FromRaw(
          activ_temp_data_int16[b * fc_output_depth + 1 * output_depth + c]);
      F0 input_modulation_gate_output =
          gemmlowp::tanh(input_modulation_gate_input);
      // Implementation of forget gate, using fixed-point logistic function.
      F3 forget_gate_input = F3::FromRaw(
          activ_temp_data_int16[b * fc_output_depth + 2 * output_depth + c]);
      F0 forget_gate_output = gemmlowp::logistic(forget_gate_input);
      // Implementation of output gate, using fixed-point logistic function.
      F3 output_gate_input = F3::FromRaw(
          activ_temp_data_int16[b * fc_output_depth + 3 * output_depth + c]);
      F0 output_gate_output = gemmlowp::logistic(output_gate_input);
      // Implementation of internal multiplication nodes, still in fixed-point.
      F0 input_times_input_modulation =
          input_gate_output * input_modulation_gate_output;
      FS prevCellState = FS::FromRaw(prevCellState_data_int16[b * output_depth + c]);
      FS prevCellState_times_forget_state = forget_gate_output * prevCellState;
      // Implementation of internal addition node, saturating.
      FS new_state = gemmlowp::SaturatingAdd(
          gemmlowp::Rescale<StateIntegerBits>(input_times_input_modulation),
          prevCellState_times_forget_state);
      // Implementation of last internal Tanh node, still in fixed-point.
      // Since a Tanh fixed-point implementation is specialized for a given
      // number or integer bits, and each specialization can have a substantial
      // code size, and we already used above a Tanh on an input with 3 integer
      // bits, and per the table in the above function comment there is no
      // significant accuracy to be lost by clamping to [-8, +8] for a
      // 3-integer-bits representation, let us just do that. This helps people
      // porting this to targets where code footprint must be minimized.
      F3 new_state_f3 = gemmlowp::Rescale<3>(new_state);
      F0 output_activ_int16 = output_gate_output * gemmlowp::tanh(new_state_f3);
      // Store the new internal state back to memory, as 16-bit integers.
      // Note: here we store the original value with StateIntegerBits, not
      // the rescaled 3-integer-bits value fed to tanh.
      output_state_data_int16[b * output_depth + c] = new_state.raw();
      // Down-scale the output activations to 8-bit integers, saturating,
      // and store back to memory.
      int16_t rescaled_output_activ =
          gemmlowp::RoundingDivideByPOT(output_activ_int16.raw(), 8);
      int16_t clamped_output_activ =
          std::max<int16_t>(-128, std::min<int16_t>(127, rescaled_output_activ));
      output_activ_data_uint8[b * output_depth + c] =
          128 + clamped_output_activ;
    }
  }
}
// clang-format on

// The function assigns a 2D matrix to a submatrix of the weights at a given row
// and column offsets.
void assignWeightsSubmatrix(const RunTimeOperandInfo* submatrix, const int32_t offset_row,
                            const int32_t offset_column, const std::vector<uint32_t>& weightsDims,
                            uint8_t* weights) {
    const uint8_t* submatrixValues = GetBuffer<uint8_t>(submatrix);
    const std::vector<uint32_t> submatrixDims = submatrix->shape().dimensions;
    for (uint32_t i = 0; i < submatrixDims[0] * submatrixDims[1]; ++i) {
        const uint32_t row = i / submatrixDims[1];
        const uint32_t column = i % submatrixDims[1];
        weights[(row + offset_row) * weightsDims[1] + column + offset_column] = submatrixValues[i];
    }
}

}  // namespace

QuantizedLSTMCell::QuantizedLSTMCell(const Operation& operation,
                                     std::vector<RunTimeOperandInfo>& operands) {
    input_ = GetInput(operation, operands, kInputTensor);

    inputToInputWeights_ = GetInput(operation, operands, kInputToInputWeightsTensor);
    inputToForgetWeights_ = GetInput(operation, operands, kInputToForgetWeightsTensor);
    inputToCellWeights_ = GetInput(operation, operands, kInputToCellWeightsTensor);
    inputToOutputWeights_ = GetInput(operation, operands, kInputToOutputWeightsTensor);

    recurrentToInputWeights_ = GetInput(operation, operands, kRecurrentToInputWeightsTensor);
    recurrentToForgetWeights_ = GetInput(operation, operands, kRecurrentToForgetWeightsTensor);
    recurrentToCellWeights_ = GetInput(operation, operands, kRecurrentToCellWeightsTensor);
    recurrentToOutputWeights_ = GetInput(operation, operands, kRecurrentToOutputWeightsTensor);

    inputGateBias_ = GetInput(operation, operands, kInputGateBiasTensor);
    forgetGateBias_ = GetInput(operation, operands, kForgetGateBiasTensor);
    cellGateBias_ = GetInput(operation, operands, kCellGateBiasTensor);
    outputGateBias_ = GetInput(operation, operands, kOutputGateBiasTensor);

    prevCellState_ = GetInput(operation, operands, kPrevCellStateTensor);
    prevOutput_ = GetInput(operation, operands, kPrevOutputTensor);

    cellStateOut_ = GetOutput(operation, operands, kCellStateOutTensor);
    output_ = GetOutput(operation, operands, kOutputTensor);
}

bool QuantizedLSTMCell::prepare(const Operation& operation,
                                std::vector<RunTimeOperandInfo>& operands, Shape* cellStateOutShape,
                                Shape* outputShape) {
    auto input = GetInput(operation, operands, kInputTensor);
    NN_RET_CHECK_EQ(NumDimensions(input), 2);
    NN_RET_CHECK_EQ(input->scale, 1. / 128.0);
    NN_RET_CHECK_EQ(input->zeroPoint, 128);
    const uint32_t numBatches = SizeOfDimension(input, 0);
    const uint32_t inputSize = SizeOfDimension(input, 1);

    auto prevOutput = GetInput(operation, operands, kPrevOutputTensor);
    NN_RET_CHECK_EQ(NumDimensions(prevOutput), 2);
    NN_RET_CHECK_EQ(SizeOfDimension(prevOutput, 0), numBatches);
    NN_RET_CHECK_EQ(prevOutput->scale, 1. / 128.0);
    NN_RET_CHECK_EQ(prevOutput->zeroPoint, 128);
    const uint32_t outputSize = SizeOfDimension(prevOutput, 1);

    auto inputToInputWeights = GetInput(operation, operands, kInputToInputWeightsTensor);
    const float weightsScale = inputToInputWeights->scale;
    NN_RET_CHECK(weightsScale != 0);
    const float weightsZeroPoint = inputToInputWeights->zeroPoint;

    auto checkWeightsShape = [&](const RunTimeOperandInfo* weights, uint32_t columns) -> bool {
        NN_RET_CHECK_EQ(NumDimensions(weights), 2);
        NN_RET_CHECK_EQ(SizeOfDimension(weights, 0), outputSize);
        NN_RET_CHECK_EQ(SizeOfDimension(weights, 1), columns);
        NN_RET_CHECK_EQ(weights->scale, weightsScale);
        NN_RET_CHECK_EQ(weights->zeroPoint, weightsZeroPoint);
        return true;
    };

    auto inputToForgetWeights = GetInput(operation, operands, kInputToForgetWeightsTensor);
    auto inputToCellWeights = GetInput(operation, operands, kInputToCellWeightsTensor);
    auto inputToOutputWeights = GetInput(operation, operands, kInputToOutputWeightsTensor);
    NN_RET_CHECK(checkWeightsShape(inputToInputWeights, inputSize));
    NN_RET_CHECK(checkWeightsShape(inputToForgetWeights, inputSize));
    NN_RET_CHECK(checkWeightsShape(inputToCellWeights, inputSize));
    NN_RET_CHECK(checkWeightsShape(inputToOutputWeights, inputSize));

    auto recurrentToInputWeights = GetInput(operation, operands, kRecurrentToInputWeightsTensor);
    auto recurrentToForgetWeights = GetInput(operation, operands, kRecurrentToForgetWeightsTensor);
    auto recurrentToCellWeights = GetInput(operation, operands, kRecurrentToCellWeightsTensor);
    auto recurrentToOutputWeights = GetInput(operation, operands, kRecurrentToOutputWeightsTensor);
    NN_RET_CHECK(checkWeightsShape(recurrentToInputWeights, outputSize));
    NN_RET_CHECK(checkWeightsShape(recurrentToForgetWeights, outputSize));
    NN_RET_CHECK(checkWeightsShape(recurrentToCellWeights, outputSize));
    NN_RET_CHECK(checkWeightsShape(recurrentToOutputWeights, outputSize));

    auto inputGateBias = GetInput(operation, operands, kInputGateBiasTensor);
    const float biasScale = inputGateBias->scale;
    NN_RET_CHECK_EQ(biasScale, weightsScale / 128.0);
    const float biasZeroPoint = inputGateBias->zeroPoint;
    NN_RET_CHECK_EQ(biasZeroPoint, 0);

    auto checkBiasShape = [&](const RunTimeOperandInfo* bias) -> bool {
        NN_RET_CHECK_EQ(NumDimensions(bias), 1);
        NN_RET_CHECK_EQ(SizeOfDimension(bias, 0), outputSize);
        NN_RET_CHECK_EQ(bias->scale, biasScale);
        NN_RET_CHECK_EQ(bias->zeroPoint, biasZeroPoint);
        return true;
    };

    auto forgetGateBias = GetInput(operation, operands, kForgetGateBiasTensor);
    auto cellGateBias = GetInput(operation, operands, kCellGateBiasTensor);
    auto outputGateBias = GetInput(operation, operands, kOutputGateBiasTensor);
    NN_RET_CHECK(checkBiasShape(inputGateBias));
    NN_RET_CHECK(checkBiasShape(forgetGateBias));
    NN_RET_CHECK(checkBiasShape(cellGateBias));
    NN_RET_CHECK(checkBiasShape(outputGateBias));

    auto prevCellState = GetInput(operation, operands, kPrevCellStateTensor);
    NN_CHECK_EQ(NumDimensions(prevCellState), 2);
    NN_CHECK_EQ(SizeOfDimension(prevCellState, 0), numBatches);
    NN_CHECK_EQ(SizeOfDimension(prevCellState, 1), outputSize);
    NN_CHECK_EQ(prevCellState->zeroPoint, 0);
    // Cell state range for quantized LSTM is a function of StateIntegerBits and
    // can be calculated as:
    // [-2^StateIntegerBits, 2^StateIntegerBits * 32767/32768].
    // Therefore, for a fixed StateIntegerBits parameter, cell state scale is
    // equal to 2^StateIntegerBits * 2^(-15) = 2^(StateIntegerBits - 15) and
    // therefore:
    // StateIntegerBits = log2(cell state scale) + 15
    int stateScaleLog2Rounded;
    NN_CHECK(tflite::CheckedLog2(prevCellState->scale, &stateScaleLog2Rounded));
    const int stateIntegerBits = 15 + stateScaleLog2Rounded;
    // We only support StateIntegerBits == 4
    NN_CHECK(stateIntegerBits == 4);

    *cellStateOutShape = prevCellState->shape();
    *outputShape = prevOutput->shape();
    return true;
}

// The function contatenates 8 input weight matrices into one. Resulting matrix
// has a shape [4 * outputSize, outputSize + inputSize]. The matrix is
// constructed as follows:
// +-----------------------------------+
// | recurrentToInput  | inputToInput  |
// |-------------------+---------------|
// | recurrentToCell   | inputToCell   |
// |-------------------+---------------|
// | recurrentToForget | inputToForget |
// |-------------------+---------------|
// | recurrentToOutput | inputToOutput |
// +-----------------------------------+
void QuantizedLSTMCell::concatenateWeights(const std::vector<uint32_t>& weightsDims,
                                           uint8_t* weights) {
    const int outputSize = SizeOfDimension(inputToInputWeights_, 0);

    assignWeightsSubmatrix(inputToInputWeights_, 0 * outputSize, outputSize, weightsDims, weights);
    assignWeightsSubmatrix(inputToCellWeights_, 1 * outputSize, outputSize, weightsDims, weights);
    assignWeightsSubmatrix(inputToForgetWeights_, 2 * outputSize, outputSize, weightsDims, weights);
    assignWeightsSubmatrix(inputToOutputWeights_, 3 * outputSize, outputSize, weightsDims, weights);
    assignWeightsSubmatrix(recurrentToInputWeights_, 0 * outputSize, 0, weightsDims, weights);
    assignWeightsSubmatrix(recurrentToCellWeights_, 1 * outputSize, 0, weightsDims, weights);
    assignWeightsSubmatrix(recurrentToForgetWeights_, 2 * outputSize, 0, weightsDims, weights);
    assignWeightsSubmatrix(recurrentToOutputWeights_, 3 * outputSize, 0, weightsDims, weights);
}

// The function concatenate four bias vectors of shape [outputSize] into one
// vector of shape [4 * outputSize].
void QuantizedLSTMCell::concatenateBiases(uint32_t outputSize, int32_t* bias) {
    memcpy(bias + 0 * outputSize, GetBuffer<int32_t>(inputGateBias_), sizeof(int32_t) * outputSize);
    memcpy(bias + 1 * outputSize, GetBuffer<int32_t>(cellGateBias_), sizeof(int32_t) * outputSize);
    memcpy(bias + 2 * outputSize, GetBuffer<int32_t>(forgetGateBias_),
           sizeof(int32_t) * outputSize);
    memcpy(bias + 3 * outputSize, GetBuffer<int32_t>(outputGateBias_),
           sizeof(int32_t) * outputSize);
}

bool QuantizedLSTMCell::eval() {
    NNTRACE_COMP("QuantizedLSTM::eval");

    Shape weightsShape;
    weightsShape.dimensions = {4 * SizeOfDimension(prevOutput_, 1),
                               SizeOfDimension(input_, 1) + SizeOfDimension(prevOutput_, 1)};
    std::vector<uint8_t> weights(getNumberOfElements(weightsShape));
    concatenateWeights(weightsShape.dimensions, weights.data());

    Shape biasShape;
    biasShape.dimensions = {getSizeOfDimension(weightsShape, 0)};
    std::vector<int32_t> bias(getNumberOfElements(biasShape));
    concatenateBiases(SizeOfDimension(prevOutput_, 1), bias.data());

    Shape concatTempShape;
    concatTempShape.dimensions = {SizeOfDimension(input_, 0), getSizeOfDimension(weightsShape, 1)};

    Shape activationTempShape;
    activationTempShape.dimensions = {SizeOfDimension(input_, 0),
                                      getSizeOfDimension(weightsShape, 0)};

    std::vector<uint8_t> concatTemp(getNumberOfElements(concatTempShape));
    std::vector<int16_t> activationTemp(getNumberOfElements(activationTempShape));

    // From https://arxiv.org/pdf/1712.05877, for a fully-connected layer,
    // accumulator multiplier is equal to:
    // (input scale) * (weights scale) / (fully-connected output scale)
    // In our case fully-connected output scale is fixed and equal to
    // 2^(-12) (See LSTMCell definition in TF Lite for more details on that).
    // But bias scale is set to (input scale) * (weights scale) (also from the
    // paper), so we can multiply it to an inverse of the fc-output scale to get
    // the multiplier value:
    double realAccumMultiplier = 4096 * inputGateBias_->scale;
    int32_t accumMultiplier;
    int accumShift;
    tflite::QuantizeMultiplier(realAccumMultiplier, &accumMultiplier, &accumShift);
    quantizedLstmStep<4>(
            // Inputs.
            GetBuffer<const uint8_t>(input_), convertShapeToDims(input_->shape()),
            GetBuffer<const uint8_t>(prevOutput_), convertShapeToDims(prevOutput_->shape()),
            weights.data(), convertShapeToDims(weightsShape), bias.data(),
            convertShapeToDims(biasShape), GetBuffer<const int16_t>(prevCellState_),
            convertShapeToDims(prevCellState_->shape()),
            // Outputs.
            GetBuffer<int16_t>(cellStateOut_), convertShapeToDims(cellStateOut_->shape()),
            GetBuffer<uint8_t>(output_), convertShapeToDims(output_->shape()), concatTemp.data(),
            convertShapeToDims(concatTempShape), activationTemp.data(),
            convertShapeToDims(activationTempShape), inputToInputWeights_->zeroPoint,
            accumMultiplier, accumShift);
    return true;
}

}  // namespace nn
}  // namespace android
