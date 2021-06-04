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

#include "CpuExecutor.h"
#include "CpuOperationUtils.h"
#include "HalInterfaces.h"
#include "OperationsUtils.h"

#include "Tracing.h"
#include "Utils.h"

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

template <typename T>
inline const T* GetOptionalBuffer(const RunTimeOperandInfo* operand) {
    return !IsNullInput(operand) ? reinterpret_cast<const T*>(operand->buffer) : nullptr;
}

}  // anonymous namespace

LSTMCell::LSTMCell(const Operation& operation, std::vector<RunTimeOperandInfo>& operands) {
    input_ = GetInput(operation, operands, kInputTensor);

    input_to_input_weights_ =
            GetInput(operation, operands, kInputToInputWeightsTensor);  // optional
    input_to_forget_weights_ = GetInput(operation, operands, kInputToForgetWeightsTensor);
    input_to_cell_weights_ = GetInput(operation, operands, kInputToCellWeightsTensor);
    input_to_output_weights_ = GetInput(operation, operands, kInputToOutputWeightsTensor);

    recurrent_to_input_weights_ =
            GetInput(operation, operands, kRecurrentToInputWeightsTensor);  // optional
    recurrent_to_forget_weights_ = GetInput(operation, operands, kRecurrentToForgetWeightsTensor);
    recurrent_to_cell_weights_ = GetInput(operation, operands, kRecurrentToCellWeightsTensor);
    recurrent_to_output_weights_ = GetInput(operation, operands, kRecurrentToOutputWeightsTensor);

    cell_to_input_weights_ = GetInput(operation, operands, kCellToInputWeightsTensor);  // optional
    cell_to_forget_weights_ =
            GetInput(operation, operands, kCellToForgetWeightsTensor);  // optional
    cell_to_output_weights_ =
            GetInput(operation, operands, kCellToOutputWeightsTensor);  // optional

    input_gate_bias_ = GetInput(operation, operands, kInputGateBiasTensor);
    forget_gate_bias_ = GetInput(operation, operands, kForgetGateBiasTensor);
    cell_bias_ = GetInput(operation, operands, kCellGateBiasTensor);
    output_gate_bias_ = GetInput(operation, operands, kOutputGateBiasTensor);

    projection_weights_ = GetInput(operation, operands, kProjectionWeightsTensor);  // optional
    projection_bias_ = GetInput(operation, operands, kProjectionBiasTensor);        // optional

    output_state_in_ = GetInput(operation, operands, kOutputStateInTensor);
    cell_state_in_ = GetInput(operation, operands, kCellStateInTensor);

    params_.activation = static_cast<TfLiteFusedActivation>(
            getScalarData<int32_t>(*GetInput(operation, operands, kActivationParam)));
    if (input_->type == OperandType::TENSOR_FLOAT32) {
        params_.cell_clip = getScalarData<float>(*GetInput(operation, operands, kCellClipParam));
        params_.proj_clip = getScalarData<float>(*GetInput(operation, operands, kProjClipParam));
    } else {
        params_.cell_clip = static_cast<float>(
                getScalarData<_Float16>(*GetInput(operation, operands, kCellClipParam)));
        params_.proj_clip = static_cast<float>(
                getScalarData<_Float16>(*GetInput(operation, operands, kProjClipParam)));
    }

    // We check the version of LSTM by checking the number of the inputs to the
    // op. For LSTM version 1.0 there were 23 inputs and for 1.2 there are 27.
    if (operation.inputs.size() == 27) {
        input_layer_norm_weights_ =
                GetInput(operation, operands, kInputLayerNormWeightsTensor);  // optional
        forget_layer_norm_weights_ =
                GetInput(operation, operands, kForgetLayerNormWeightsTensor);  // optional
        cell_layer_norm_weights_ =
                GetInput(operation, operands, kCellLayerNormWeightsTensor);  // optional
        output_layer_norm_weights_ =
                GetInput(operation, operands, kOutputLayerNormWeightsTensor);  // optional
    } else {
        // For LSTM from HAL v1.0 assign operands with no values
        static RunTimeOperandInfo no_value;
        no_value.lifetime = OperandLifeTime::NO_VALUE;

        input_layer_norm_weights_ = &no_value;
        forget_layer_norm_weights_ = &no_value;
        cell_layer_norm_weights_ = &no_value;
        output_layer_norm_weights_ = &no_value;
    }

    output_state_out_ = GetOutput(operation, operands, kOutputStateOutTensor);
    cell_state_out_ = GetOutput(operation, operands, kCellStateOutTensor);
    output_ = GetOutput(operation, operands, kOutputTensor);

    scratch_buffer_ = GetOutput(operation, operands, kScratchBufferTensor);
}

// static
bool LSTMCell::CheckInputTensorDimensions(
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
        const RunTimeOperandInfo* cell_to_output_weights, const RunTimeOperandInfo* input_gate_bias,
        const RunTimeOperandInfo* forget_gate_bias, const RunTimeOperandInfo* cell_bias,
        const RunTimeOperandInfo* output_gate_bias, const RunTimeOperandInfo* projection_weights,
        const RunTimeOperandInfo* projection_bias,
        const RunTimeOperandInfo* input_layer_norm_weights,
        const RunTimeOperandInfo* forget_layer_norm_weights,
        const RunTimeOperandInfo* cell_layer_norm_weights,
        const RunTimeOperandInfo* output_layer_norm_weights, uint32_t n_input, uint32_t n_output,
        uint32_t n_cell, LSTMParams* params) {
    // Making sure clipping parameters have valid values.
    // == 0 means no clipping
    //  > 0 means clipping
    NN_CHECK(params->cell_clip >= 0);
    NN_CHECK(params->proj_clip >= 0);

    if (!IsNullInput(input_to_input_weights)) {
        NN_CHECK_EQ(NumDimensions(input_to_input_weights), 2);
        NN_CHECK_EQ(SizeOfDimension(input_to_input_weights, 0), n_cell);
        NN_CHECK_EQ(SizeOfDimension(input_to_input_weights, 1), n_input);
    }

    NN_CHECK_EQ(NumDimensions(input_to_forget_weights), 2);
    NN_CHECK_EQ(SizeOfDimension(input_to_forget_weights, 0), n_cell);
    NN_CHECK_EQ(SizeOfDimension(input_to_forget_weights, 1), n_input);

    NN_CHECK_EQ(NumDimensions(input_to_cell_weights), 2);
    NN_CHECK_EQ(SizeOfDimension(input_to_cell_weights, 0), n_cell);
    NN_CHECK_EQ(SizeOfDimension(input_to_cell_weights, 1), n_input);

    if (!IsNullInput(recurrent_to_input_weights)) {
        NN_CHECK_EQ(NumDimensions(recurrent_to_input_weights), 2);
        NN_CHECK_EQ(SizeOfDimension(recurrent_to_input_weights, 0), n_cell);
        NN_CHECK_EQ(SizeOfDimension(recurrent_to_input_weights, 1), n_output);
    }

    NN_CHECK_EQ(NumDimensions(recurrent_to_forget_weights), 2);
    NN_CHECK_EQ(SizeOfDimension(recurrent_to_forget_weights, 0), n_cell);
    NN_CHECK_EQ(SizeOfDimension(recurrent_to_forget_weights, 1), n_output);

    NN_CHECK_EQ(NumDimensions(recurrent_to_cell_weights), 2);
    NN_CHECK_EQ(SizeOfDimension(recurrent_to_cell_weights, 0), n_cell);
    NN_CHECK_EQ(SizeOfDimension(recurrent_to_cell_weights, 1), n_output);

    // We make sure the input-gate's parameters are either both present (regular
    // LSTM) or not at all (CIFG-LSTM).
    const bool cifg_weights_all_or_none =
            (!IsNullInput(input_to_input_weights) && !IsNullInput(recurrent_to_input_weights)) ||
            (IsNullInput(input_to_input_weights) && IsNullInput(recurrent_to_input_weights));
    NN_CHECK(cifg_weights_all_or_none);

    if (!IsNullInput(cell_to_input_weights)) {
        NN_CHECK_EQ(NumDimensions(cell_to_input_weights), 1);
        NN_CHECK_EQ(SizeOfDimension(cell_to_input_weights, 0), n_cell);
    }

    if (!IsNullInput(cell_to_forget_weights)) {
        NN_CHECK_EQ(NumDimensions(cell_to_forget_weights), 1);
        NN_CHECK_EQ(SizeOfDimension(cell_to_forget_weights, 0), n_cell);
    }

    if (!IsNullInput(cell_to_output_weights)) {
        NN_CHECK_EQ(NumDimensions(cell_to_output_weights), 1);
        NN_CHECK_EQ(SizeOfDimension(cell_to_output_weights, 0), n_cell);
    }

    // Making sure the peephole weights are there all or none.
    params->use_cifg = IsNullInput(input_to_input_weights);
    const bool peephole_weights_all_or_none =
            ((!IsNullInput(cell_to_input_weights) || params->use_cifg) &&
             !IsNullInput(cell_to_forget_weights) && !IsNullInput(cell_to_output_weights)) ||
            (IsNullInput(cell_to_input_weights) && IsNullInput(cell_to_forget_weights) &&
             IsNullInput(cell_to_output_weights));
    NN_CHECK(peephole_weights_all_or_none);

    // Since we have already checked that weights are all there or none, we can
    // check the existence of only one to the get the condition.
    params->use_peephole = !IsNullInput(cell_to_output_weights);
    // Checking output instead of input layer norm weights because input can be
    // omitted ones can be omited in case CIFG LSTM is used.
    params->use_layer_norm = !IsNullInput(output_layer_norm_weights);

    params->use_projection_weight = (projection_weights->lifetime != OperandLifeTime::NO_VALUE);
    params->use_projection_bias = (projection_bias->lifetime != OperandLifeTime::NO_VALUE);

    // Make sure the input gate bias is present only when not a CIFG-LSTM.
    if (params->use_cifg) {
        NN_CHECK(IsNullInput(input_gate_bias));
    } else {
        NN_CHECK_EQ(NumDimensions(input_gate_bias), 1);
        NN_CHECK_EQ(SizeOfDimension(input_gate_bias, 0), n_cell);
    }

    NN_CHECK_EQ(NumDimensions(forget_gate_bias), 1);
    NN_CHECK_EQ(SizeOfDimension(forget_gate_bias, 0), n_cell);

    NN_CHECK_EQ(NumDimensions(cell_bias), 1);
    NN_CHECK_EQ(SizeOfDimension(cell_bias, 0), n_cell);

    NN_CHECK_EQ(NumDimensions(output_gate_bias), 1);
    NN_CHECK_EQ(SizeOfDimension(output_gate_bias, 0), n_cell);

    if (!IsNullInput(projection_weights)) {
        NN_CHECK_EQ(NumDimensions(projection_weights), 2);
        NN_CHECK_EQ(SizeOfDimension(projection_weights, 0), n_output);
        NN_CHECK_EQ(SizeOfDimension(projection_weights, 1), n_cell);
    }

    if (!IsNullInput(projection_bias)) {
        NN_CHECK_EQ(NumDimensions(projection_bias), 1);
        NN_CHECK_EQ(SizeOfDimension(projection_bias, 0), n_output);
    }

    // Making sure the projection tensors are consistent:
    // 1) If projection weight is not present, then projection bias should not be
    // present.
    // 2) If projection weight is present, then projection bias is optional.
    // TODO: make sure this is correct.
    const bool projecton_tensors_consistent =
            (!IsNullInput(projection_weights) || IsNullInput(projection_bias));
    NN_CHECK(projecton_tensors_consistent == true);

    if (!IsNullInput(input_layer_norm_weights)) {
        NN_CHECK_EQ(NumDimensions(input_layer_norm_weights), 1);
        NN_CHECK_EQ(SizeOfDimension(input_layer_norm_weights, 0), n_cell);
    }
    if (!IsNullInput(forget_layer_norm_weights)) {
        NN_CHECK_EQ(NumDimensions(forget_layer_norm_weights), 1);
        NN_CHECK_EQ(SizeOfDimension(forget_layer_norm_weights, 0), n_cell);
    }
    if (!IsNullInput(cell_layer_norm_weights)) {
        NN_CHECK_EQ(NumDimensions(cell_layer_norm_weights), 1);
        NN_CHECK_EQ(SizeOfDimension(cell_layer_norm_weights, 0), n_cell);
    }
    if (!IsNullInput(output_layer_norm_weights)) {
        NN_CHECK_EQ(NumDimensions(output_layer_norm_weights), 1);
        NN_CHECK_EQ(SizeOfDimension(output_layer_norm_weights, 0), n_cell);
    }

    if (params->use_cifg) {
        NN_RET_CHECK(IsNullInput(input_layer_norm_weights))
                << "input_layer_norm_weights are provided while CIFG is used";
        const bool layer_norm_weights_all_or_none_cifg =
                (IsNullInput(forget_layer_norm_weights) && IsNullInput(cell_layer_norm_weights) &&
                 IsNullInput(output_layer_norm_weights)) ||
                (!IsNullInput(forget_layer_norm_weights) && !IsNullInput(cell_layer_norm_weights) &&
                 !IsNullInput(output_layer_norm_weights));
        NN_RET_CHECK(layer_norm_weights_all_or_none_cifg);
    } else {
        const bool layer_norm_weights_all_or_none =
                (IsNullInput(input_layer_norm_weights) && IsNullInput(forget_layer_norm_weights) &&
                 IsNullInput(cell_layer_norm_weights) && IsNullInput(output_layer_norm_weights)) ||
                (!IsNullInput(input_layer_norm_weights) &&
                 !IsNullInput(forget_layer_norm_weights) && !IsNullInput(cell_layer_norm_weights) &&
                 !IsNullInput(output_layer_norm_weights));
        NN_RET_CHECK(layer_norm_weights_all_or_none);
    }

    return true;
}

bool LSTMCell::Prepare(const Operation& operation, std::vector<RunTimeOperandInfo>& operands,
                       Shape* scratchShape, Shape* outputStateShape, Shape* cellStateShape,
                       Shape* outputShape) {
    // Check we have all the inputs and outputs we need.
    NN_CHECK(NumInputsWithValues(operation, operands) >= 15 &&
             NumInputsWithValues(operation, operands) <= 27);
    NN_CHECK_EQ(NumOutputs(operation), 4);

    // Inferring batch size, number of outputs and number of cells from the
    // input tensors.
    NN_CHECK(NumDimensions(input_) > 1);
    const uint32_t n_batch = SizeOfDimension(input_, 0);
    const uint32_t n_input = SizeOfDimension(input_, 1);

    const uint32_t n_cell = SizeOfDimension(input_to_output_weights_, 0);
    NN_CHECK_EQ(NumDimensions(input_to_output_weights_), 2);
    NN_CHECK_EQ(SizeOfDimension(input_to_output_weights_, 1), n_input);

    NN_CHECK_EQ(NumDimensions(recurrent_to_output_weights_), 2);
    NN_CHECK_EQ(SizeOfDimension(recurrent_to_output_weights_, 0), n_cell);
    const uint32_t n_output = SizeOfDimension(recurrent_to_output_weights_, 1);

    // Check that input tensor dimensions matches with each other.
    if (!CheckInputTensorDimensions(
                input_, input_to_input_weights_, input_to_forget_weights_, input_to_cell_weights_,
                input_to_output_weights_, recurrent_to_input_weights_, recurrent_to_forget_weights_,
                recurrent_to_cell_weights_, recurrent_to_output_weights_, cell_to_input_weights_,
                cell_to_forget_weights_, cell_to_output_weights_, input_gate_bias_,
                forget_gate_bias_, cell_bias_, output_gate_bias_, projection_weights_,
                projection_bias_, input_layer_norm_weights_, forget_layer_norm_weights_,
                cell_layer_norm_weights_, output_layer_norm_weights_, n_input, n_output, n_cell,
                &params_)) {
        return false;
    }

    // Resize the output and output_state tensors.
    const Shape& inputShape = input_->shape();

    outputShape->type = inputShape.type;
    outputShape->dimensions = {n_batch, n_output};
    outputShape->offset = inputShape.offset;
    outputShape->scale = inputShape.scale;

    outputStateShape->type = inputShape.type;
    outputStateShape->dimensions = {n_batch, n_output};
    outputStateShape->offset = inputShape.offset;
    outputStateShape->scale = inputShape.scale;

    cellStateShape->type = inputShape.type;
    cellStateShape->dimensions = {n_batch, n_cell};
    cellStateShape->offset = inputShape.offset;
    cellStateShape->scale = inputShape.scale;

    if (params_.use_cifg) {
        // Reserving space for Cell, Forget, Output gates
        scratchShape->dimensions = {n_batch, n_cell * 3};
    } else {
        // Reserving space for Input, Cell, Forget, Output gates
        scratchShape->dimensions = {n_batch, n_cell * 4};
    }
    scratchShape->type = inputShape.type;
    scratchShape->offset = inputShape.offset;
    scratchShape->scale = inputShape.scale;

    return true;
}

// static
bool LSTMCell::LSTMEvalFloat32(
        const LSTMParams& params, const float* input_buffer, const Shape& input_shape,
        const float* input_to_input_weights_buffer, const float* input_to_forget_weights_buffer,
        const float* input_to_cell_weights_buffer, const float* input_to_output_weights_buffer,
        const Shape& input_to_output_weights_shape, const float* recurrent_to_input_weights_buffer,
        const float* recurrent_to_forget_weights_buffer,
        const float* recurrent_to_cell_weights_buffer,
        const float* recurrent_to_output_weights_buffer,
        const Shape& recurrent_to_output_weights_shape, const float* cell_to_input_weights_buffer,
        const float* cell_to_forget_weights_buffer, const float* cell_to_output_weights_buffer,
        const float* aux_input_buffer, const float* aux_input_to_input_weights_buffer,
        const float* aux_input_to_forget_weights_buffer,
        const float* aux_input_to_cell_weights_buffer,
        const float* aux_input_to_output_weights_buffer, const float* input_gate_bias_buffer,
        const float* forget_gate_bias_buffer, const float* cell_bias_buffer,
        const float* output_gate_bias_buffer, const float* projection_weights_buffer,
        const float* projection_bias_buffer, const float* output_state_in_buffer,
        const float* cell_state_in_buffer, const float* input_layer_norm_weights_buffer,
        const float* forget_layer_norm_weights_buffer, const float* cell_layer_norm_weights_buffer,
        const float* output_layer_norm_weights_buffer, float* output_state_out_buffer,
        float* cell_state_out_buffer, float* output_buffer, float* scratch_buffer_buffer,
        bool timeMajor, bool forwardSequence) {
    NNTRACE_COMP("LSTMCell::LSTMEvalFloat32");

    const uint32_t inputRank = getNumberOfDimensions(input_shape);
    NN_CHECK(inputRank == 2 || inputRank == 3);

    const uint32_t maxTime =
            (inputRank == 3) ? getSizeOfDimension(input_shape, timeMajor ? 0 : 1) : 1;
    const uint32_t batchSize = (inputRank == 3) ? getSizeOfDimension(input_shape, timeMajor ? 1 : 0)
                                                : getSizeOfDimension(input_shape, 0);
    const uint32_t inputSize = getSizeOfDimension(input_shape, inputRank - 1);
    const uint32_t numCells = getSizeOfDimension(input_to_output_weights_shape, 0);
    const uint32_t outputSize = getSizeOfDimension(recurrent_to_output_weights_shape, 1);

    Shape batchInputShape = input_shape;
    batchInputShape.dimensions = {batchSize, inputSize};
    const uint32_t batchInputSize = batchSize * inputSize;
    const uint32_t batchOutputSize = batchSize * outputSize;

    std::vector<float> transposedInput;
    const bool hasAuxInput = (aux_input_buffer != nullptr);
    std::vector<float> transposedAuxInput;
    std::vector<float> transposedOutput;
    Shape transposedInputShape;
    Shape transposedOutputShape;
    if (!timeMajor) {
        transposedInput.resize(maxTime * batchInputSize);
        transposeFirstTwoDimensions<float>(input_buffer, input_shape, transposedInput.data());
        if (hasAuxInput) {
            transposedAuxInput.resize(maxTime * batchInputSize);
            transposeFirstTwoDimensions<float>(aux_input_buffer, input_shape,
                                               transposedAuxInput.data());
        }
        transposeFirstTwoDimensions(input_shape, &transposedInputShape);
        transposedOutput.resize(maxTime * batchOutputSize);
        transposedOutputShape = transposedInputShape;
        transposedOutputShape.dimensions[2] = outputSize;
    }
    const float* inputData = timeMajor ? input_buffer : transposedInput.data();
    const float* auxInputData =
            hasAuxInput ? (timeMajor ? aux_input_buffer : transposedAuxInput.data()) : nullptr;
    float* outputData = timeMajor ? output_buffer : transposedOutput.data();

    std::vector<float> outputStateInCurrentTimeStep(
            output_state_in_buffer, output_state_in_buffer + batchSize * outputSize);
    std::vector<float> cellStateInCurrentTimeStep(cell_state_in_buffer,
                                                  cell_state_in_buffer + batchSize * numCells);
    const float* inputCurrentTimeStep =
            inputData + (forwardSequence ? 0 : batchInputSize * (maxTime - 1));
    const float* auxInputCurrentTimeStep =
            hasAuxInput ? (auxInputData + (forwardSequence ? 0 : batchInputSize * (maxTime - 1)))
                        : nullptr;
    float* outputCurrentTimeStep =
            outputData + (forwardSequence ? 0 : batchOutputSize * (maxTime - 1));
    const int batchInputDelta = forwardSequence ? batchInputSize : -batchInputSize;
    const int batchOutputDelta = forwardSequence ? batchOutputSize : -batchOutputSize;

    for (int t = 0; t < maxTime; ++t) {
        LSTMStep(params, inputCurrentTimeStep, batchInputShape, input_to_input_weights_buffer,
                 input_to_forget_weights_buffer, input_to_cell_weights_buffer,
                 input_to_output_weights_buffer, input_to_output_weights_shape,
                 recurrent_to_input_weights_buffer, recurrent_to_forget_weights_buffer,
                 recurrent_to_cell_weights_buffer, recurrent_to_output_weights_buffer,
                 recurrent_to_output_weights_shape, cell_to_input_weights_buffer,
                 cell_to_forget_weights_buffer, cell_to_output_weights_buffer,
                 auxInputCurrentTimeStep, aux_input_to_input_weights_buffer,
                 aux_input_to_forget_weights_buffer, aux_input_to_cell_weights_buffer,
                 aux_input_to_output_weights_buffer, input_gate_bias_buffer,
                 forget_gate_bias_buffer, cell_bias_buffer, output_gate_bias_buffer,
                 projection_weights_buffer, projection_bias_buffer,
                 outputStateInCurrentTimeStep.data(), cellStateInCurrentTimeStep.data(),
                 input_layer_norm_weights_buffer, forget_layer_norm_weights_buffer,
                 cell_layer_norm_weights_buffer, output_layer_norm_weights_buffer,
                 output_state_out_buffer, cell_state_out_buffer, outputCurrentTimeStep,
                 scratch_buffer_buffer);
        inputCurrentTimeStep += batchInputDelta;
        if (hasAuxInput) {
            auxInputCurrentTimeStep += batchInputDelta;
        }
        outputCurrentTimeStep += batchOutputDelta;
        outputStateInCurrentTimeStep.assign(output_state_out_buffer,
                                            output_state_out_buffer + batchSize * outputSize);
        cellStateInCurrentTimeStep.assign(cell_state_out_buffer,
                                          cell_state_out_buffer + batchSize * numCells);
    }

    if (!timeMajor) {
        transposeFirstTwoDimensions<float>(transposedOutput.data(), transposedOutputShape,
                                           output_buffer);
    }

    return true;
}

// static
bool LSTMCell::LSTMEvalFloat16(
        const LSTMParams& params, const _Float16* input_buffer, const Shape& input_shape,
        const _Float16* input_to_input_weights_buffer,
        const _Float16* input_to_forget_weights_buffer,
        const _Float16* input_to_cell_weights_buffer,
        const _Float16* input_to_output_weights_buffer, const Shape& input_to_output_weights_shape,
        const _Float16* recurrent_to_input_weights_buffer,
        const _Float16* recurrent_to_forget_weights_buffer,
        const _Float16* recurrent_to_cell_weights_buffer,
        const _Float16* recurrent_to_output_weights_buffer,
        const Shape& recurrent_to_output_weights_shape,
        const _Float16* cell_to_input_weights_buffer, const _Float16* cell_to_forget_weights_buffer,
        const _Float16* cell_to_output_weights_buffer, const _Float16* aux_input_buffer,
        const _Float16* aux_input_to_input_weights_buffer,
        const _Float16* aux_input_to_forget_weights_buffer,
        const _Float16* aux_input_to_cell_weights_buffer,
        const _Float16* aux_input_to_output_weights_buffer, const _Float16* input_gate_bias_buffer,
        const _Float16* forget_gate_bias_buffer, const _Float16* cell_bias_buffer,
        const _Float16* output_gate_bias_buffer, const _Float16* projection_weights_buffer,
        const _Float16* projection_bias_buffer, const _Float16* output_state_in_buffer,
        const _Float16* cell_state_in_buffer, const _Float16* input_layer_norm_weights_buffer,
        const _Float16* forget_layer_norm_weights_buffer,
        const _Float16* cell_layer_norm_weights_buffer,
        const _Float16* output_layer_norm_weights_buffer, _Float16* output_state_out_buffer,
        _Float16* cell_state_out_buffer, _Float16* output_buffer, _Float16* scratch_buffer_buffer,
        bool timeMajor, bool forwardSequence) {
    NNTRACE_COMP("LSTMCell::LSTMEvalFloat16");

    const uint32_t inputRank = getNumberOfDimensions(input_shape);
    NN_CHECK(inputRank == 2 || inputRank == 3);

    const uint32_t maxTime =
            (inputRank == 3) ? getSizeOfDimension(input_shape, timeMajor ? 0 : 1) : 1;
    const uint32_t batchSize = (inputRank == 3) ? getSizeOfDimension(input_shape, timeMajor ? 1 : 0)
                                                : getSizeOfDimension(input_shape, 0);
    const uint32_t inputSize = getSizeOfDimension(input_shape, inputRank - 1);
    const uint32_t numCells = getSizeOfDimension(input_to_output_weights_shape, 0);
    const uint32_t outputSize = getSizeOfDimension(recurrent_to_output_weights_shape, 1);

    Shape batchInputShape = input_shape;
    batchInputShape.dimensions = {batchSize, inputSize};
    const uint32_t batchInputSize = batchSize * inputSize;
    const uint32_t batchOutputSize = batchSize * outputSize;

    std::vector<float> input_float32(maxTime * batchInputSize);
    convertFloat16ToFloat32(input_buffer, &input_float32);
    std::vector<float> input_to_input_weights_float32(numCells * inputSize);
    if (input_to_input_weights_buffer != nullptr) {
        convertFloat16ToFloat32(input_to_input_weights_buffer, &input_to_input_weights_float32);
    }
    std::vector<float> input_to_forget_weights_float32(numCells * inputSize);
    convertFloat16ToFloat32(input_to_forget_weights_buffer, &input_to_forget_weights_float32);
    std::vector<float> input_to_cell_weights_float32(numCells * inputSize);
    convertFloat16ToFloat32(input_to_cell_weights_buffer, &input_to_cell_weights_float32);
    std::vector<float> input_to_output_weights_float32(numCells * inputSize);
    convertFloat16ToFloat32(input_to_output_weights_buffer, &input_to_output_weights_float32);

    std::vector<float> recurrent_to_input_weights_float32(numCells * outputSize);
    if (recurrent_to_input_weights_buffer != nullptr) {
        convertFloat16ToFloat32(recurrent_to_input_weights_buffer,
                                &recurrent_to_input_weights_float32);
    }
    std::vector<float> recurrent_to_forget_weights_float32(numCells * outputSize);
    convertFloat16ToFloat32(recurrent_to_forget_weights_buffer,
                            &recurrent_to_forget_weights_float32);
    std::vector<float> recurrent_to_cell_weights_float32(numCells * outputSize);
    convertFloat16ToFloat32(recurrent_to_cell_weights_buffer, &recurrent_to_cell_weights_float32);
    std::vector<float> recurrent_to_output_weights_float32(numCells * outputSize);
    convertFloat16ToFloat32(recurrent_to_output_weights_buffer,
                            &recurrent_to_output_weights_float32);

    std::vector<float> cell_to_input_weights_float32(numCells);
    if (cell_to_input_weights_buffer != nullptr) {
        convertFloat16ToFloat32(cell_to_input_weights_buffer, &cell_to_input_weights_float32);
    }
    std::vector<float> cell_to_forget_weights_float32(numCells);
    if (cell_to_forget_weights_buffer != nullptr) {
        convertFloat16ToFloat32(cell_to_forget_weights_buffer, &cell_to_forget_weights_float32);
    }
    std::vector<float> cell_to_output_weights_float32(numCells);
    if (cell_to_output_weights_buffer != nullptr) {
        convertFloat16ToFloat32(cell_to_output_weights_buffer, &cell_to_output_weights_float32);
    }

    std::vector<float> aux_input_float32(maxTime * batchInputSize);
    if (aux_input_buffer != nullptr) {
        convertFloat16ToFloat32(aux_input_buffer, &aux_input_float32);
    }
    std::vector<float> aux_input_to_input_weights_float32(numCells * inputSize);
    if (aux_input_to_input_weights_buffer != nullptr) {
        convertFloat16ToFloat32(aux_input_to_input_weights_buffer,
                                &aux_input_to_input_weights_float32);
    }
    std::vector<float> aux_input_to_forget_weights_float32(numCells * inputSize);
    if (aux_input_to_forget_weights_buffer != nullptr) {
        convertFloat16ToFloat32(aux_input_to_forget_weights_buffer,
                                &aux_input_to_forget_weights_float32);
    }
    std::vector<float> aux_input_to_cell_weights_float32(numCells * inputSize);
    if (aux_input_to_cell_weights_buffer != nullptr) {
        convertFloat16ToFloat32(aux_input_to_cell_weights_buffer,
                                &aux_input_to_cell_weights_float32);
    }
    std::vector<float> aux_input_to_output_weights_float32(numCells * inputSize);
    if (aux_input_to_output_weights_buffer != nullptr) {
        convertFloat16ToFloat32(aux_input_to_output_weights_buffer,
                                &aux_input_to_output_weights_float32);
    }

    std::vector<float> input_gate_bias_float32(numCells);
    if (input_gate_bias_buffer != nullptr) {
        convertFloat16ToFloat32(input_gate_bias_buffer, &input_gate_bias_float32);
    }
    std::vector<float> forget_gate_bias_float32(numCells);
    convertFloat16ToFloat32(forget_gate_bias_buffer, &forget_gate_bias_float32);
    std::vector<float> cell_bias_float32(numCells);
    convertFloat16ToFloat32(cell_bias_buffer, &cell_bias_float32);
    std::vector<float> output_gate_bias_float32(numCells);
    convertFloat16ToFloat32(output_gate_bias_buffer, &output_gate_bias_float32);

    std::vector<float> projection_weights_float32(numCells * outputSize);
    if (projection_weights_buffer != nullptr) {
        convertFloat16ToFloat32(projection_weights_buffer, &projection_weights_float32);
    }
    std::vector<float> projection_bias_float32(outputSize);
    if (projection_bias_buffer != nullptr) {
        convertFloat16ToFloat32(projection_bias_buffer, &projection_bias_float32);
    }

    std::vector<float> input_layer_norm_weights_float32(numCells);
    if (input_layer_norm_weights_buffer != nullptr) {
        convertFloat16ToFloat32(input_layer_norm_weights_buffer, &input_layer_norm_weights_float32);
    }
    std::vector<float> forget_layer_norm_weights_float32(numCells);
    if (forget_layer_norm_weights_buffer != nullptr) {
        convertFloat16ToFloat32(forget_layer_norm_weights_buffer,
                                &forget_layer_norm_weights_float32);
    }
    std::vector<float> cell_layer_norm_weights_float32(numCells);
    if (cell_layer_norm_weights_buffer != nullptr) {
        convertFloat16ToFloat32(cell_layer_norm_weights_buffer, &cell_layer_norm_weights_float32);
    }
    std::vector<float> output_layer_norm_weights_float32(numCells);
    if (output_layer_norm_weights_buffer != nullptr) {
        convertFloat16ToFloat32(output_layer_norm_weights_buffer,
                                &output_layer_norm_weights_float32);
    }

    std::vector<float> output_state_out_float32(batchOutputSize);
    convertFloat16ToFloat32(output_state_out_buffer, &output_state_out_float32);
    std::vector<float> cell_state_out_float32(batchSize * numCells);
    convertFloat16ToFloat32(cell_state_out_buffer, &cell_state_out_float32);

    std::vector<float> output_float32(maxTime * batchOutputSize);
    convertFloat16ToFloat32(output_buffer, &output_float32);
    std::vector<float> scratch_buffer_float32(params.use_cifg ? 3 * batchSize * numCells
                                                              : 4 * batchSize * numCells);
    convertFloat16ToFloat32(scratch_buffer_buffer, &scratch_buffer_float32);

    std::vector<float> transposedInput;
    const bool hasAuxInput = (aux_input_buffer != nullptr);
    std::vector<float> transposedAuxInput;
    std::vector<float> transposedOutput;
    Shape transposedInputShape;
    Shape transposedOutputShape;
    if (!timeMajor) {
        transposedInput.resize(maxTime * batchInputSize);
        transposeFirstTwoDimensions<float>(input_float32.data(), input_shape,
                                           transposedInput.data());
        if (hasAuxInput) {
            transposedAuxInput.resize(maxTime * batchInputSize);
            transposeFirstTwoDimensions<float>(aux_input_float32.data(), input_shape,
                                               transposedAuxInput.data());
        }
        transposeFirstTwoDimensions(input_shape, &transposedInputShape);
        transposedOutput.resize(maxTime * batchOutputSize);
        transposedOutputShape = transposedInputShape;
        transposedOutputShape.dimensions[2] = outputSize;
    }
    const float* inputData = timeMajor ? input_float32.data() : transposedInput.data();
    const float* auxInputData =
            hasAuxInput ? (timeMajor ? aux_input_float32.data() : transposedAuxInput.data())
                        : nullptr;
    float* outputData = timeMajor ? output_float32.data() : transposedOutput.data();

    std::vector<float> outputStateInCurrentTimeStep(batchSize * outputSize);
    convertFloat16ToFloat32(output_state_in_buffer, &outputStateInCurrentTimeStep);
    std::vector<float> cellStateInCurrentTimeStep(batchSize * numCells);
    convertFloat16ToFloat32(cell_state_in_buffer, &cellStateInCurrentTimeStep);

    const float* inputCurrentTimeStep =
            inputData + (forwardSequence ? 0 : batchInputSize * (maxTime - 1));
    const float* auxInputCurrentTimeStep =
            hasAuxInput ? (auxInputData + (forwardSequence ? 0 : batchInputSize * (maxTime - 1)))
                        : nullptr;
    float* outputCurrentTimeStep =
            outputData + (forwardSequence ? 0 : batchOutputSize * (maxTime - 1));
    const int batchInputDelta = forwardSequence ? batchInputSize : -batchInputSize;
    const int batchOutputDelta = forwardSequence ? batchOutputSize : -batchOutputSize;

    for (int t = 0; t < maxTime; ++t) {
        LSTMStep(params, inputCurrentTimeStep, batchInputShape,
                 input_to_input_weights_float32.data(), input_to_forget_weights_float32.data(),
                 input_to_cell_weights_float32.data(), input_to_output_weights_float32.data(),
                 input_to_output_weights_shape, recurrent_to_input_weights_float32.data(),
                 recurrent_to_forget_weights_float32.data(),
                 recurrent_to_cell_weights_float32.data(),
                 recurrent_to_output_weights_float32.data(), recurrent_to_output_weights_shape,
                 cell_to_input_weights_float32.data(), cell_to_forget_weights_float32.data(),
                 cell_to_output_weights_float32.data(), auxInputCurrentTimeStep,
                 aux_input_to_input_weights_float32.data(),
                 aux_input_to_forget_weights_float32.data(),
                 aux_input_to_cell_weights_float32.data(),
                 aux_input_to_output_weights_float32.data(), input_gate_bias_float32.data(),
                 forget_gate_bias_float32.data(), cell_bias_float32.data(),
                 output_gate_bias_float32.data(), projection_weights_float32.data(),
                 projection_bias_float32.data(), outputStateInCurrentTimeStep.data(),
                 cellStateInCurrentTimeStep.data(), input_layer_norm_weights_float32.data(),
                 forget_layer_norm_weights_float32.data(), cell_layer_norm_weights_float32.data(),
                 output_layer_norm_weights_float32.data(), output_state_out_float32.data(),
                 cell_state_out_float32.data(), outputCurrentTimeStep,
                 scratch_buffer_float32.data());
        inputCurrentTimeStep += batchInputDelta;
        if (hasAuxInput) {
            auxInputCurrentTimeStep += batchInputDelta;
        }
        outputCurrentTimeStep += batchOutputDelta;
        outputStateInCurrentTimeStep = output_state_out_float32;
        cellStateInCurrentTimeStep = cell_state_out_float32;
    }

    if (!timeMajor) {
        transposeFirstTwoDimensions<float>(transposedOutput.data(), transposedOutputShape,
                                           output_float32.data());
    }

    convertFloat32ToFloat16(output_state_out_float32, output_state_out_buffer);
    convertFloat32ToFloat16(cell_state_out_float32, cell_state_out_buffer);
    convertFloat32ToFloat16(output_float32, output_buffer);
    convertFloat32ToFloat16(scratch_buffer_float32, scratch_buffer_buffer);
    return true;
}

// static
bool LSTMCell::LSTMStep(
        const LSTMParams& params, const float* input_buffer, const Shape& input_shape,
        const float* input_to_input_weights_buffer, const float* input_to_forget_weights_buffer,
        const float* input_to_cell_weights_buffer, const float* input_to_output_weights_buffer,
        const Shape& input_to_output_weights_shape, const float* recurrent_to_input_weights_buffer,
        const float* recurrent_to_forget_weights_buffer,
        const float* recurrent_to_cell_weights_buffer,
        const float* recurrent_to_output_weights_buffer,
        const Shape& recurrent_to_output_weights_shape, const float* cell_to_input_weights_buffer,
        const float* cell_to_forget_weights_buffer, const float* cell_to_output_weights_buffer,
        const float* aux_input_buffer, const float* aux_input_to_input_weights_buffer,
        const float* aux_input_to_forget_weights_buffer,
        const float* aux_input_to_cell_weights_buffer,
        const float* aux_input_to_output_weights_buffer, const float* input_gate_bias_buffer,
        const float* forget_gate_bias_buffer, const float* cell_bias_buffer,
        const float* output_gate_bias_buffer, const float* projection_weights_buffer,
        const float* projection_bias_buffer, const float* output_state_in_buffer,
        const float* cell_state_in_buffer, const float* input_layer_norm_weights_buffer,
        const float* forget_layer_norm_weights_buffer, const float* cell_layer_norm_weights_buffer,
        const float* output_layer_norm_weights_buffer, float* output_state_out_buffer,
        float* cell_state_out_buffer, float* output_buffer, float* scratch_buffer_buffer) {
    NNTRACE_COMP("LSTMCell::LSTMStep");

    const uint32_t n_batch = input_shape.dimensions[0];
    const uint32_t n_input = input_shape.dimensions[1];
    // n_cell and n_output will be the same size when there is no projection.
    const uint32_t n_cell = input_to_output_weights_shape.dimensions[0];
    const uint32_t n_output = recurrent_to_output_weights_shape.dimensions[1];
    const uint32_t n_aux_input = aux_input_buffer == nullptr ? 0 : n_input;

    // Index the scratch buffers pointers to the global scratch buffer.
    float* input_gate_scratch = nullptr;
    float* cell_scratch = nullptr;
    float* forget_gate_scratch = nullptr;
    float* output_gate_scratch = nullptr;
    if (params.use_cifg) {
        cell_scratch = scratch_buffer_buffer;
        forget_gate_scratch = cell_scratch + n_cell * n_batch;
        output_gate_scratch = cell_scratch + 2 * n_cell * n_batch;
    } else {
        input_gate_scratch = scratch_buffer_buffer;
        cell_scratch = input_gate_scratch + n_cell * n_batch;
        forget_gate_scratch = input_gate_scratch + 2 * n_cell * n_batch;
        output_gate_scratch = input_gate_scratch + 3 * n_cell * n_batch;
    }

    if (!params.use_layer_norm) {
        // Initialize scratch buffers with bias.
        if (!params.use_cifg) {
            tflite::tensor_utils::VectorBatchVectorAssign(input_gate_bias_buffer, n_cell, n_batch,
                                                          input_gate_scratch);
        }
        tflite::tensor_utils::VectorBatchVectorAssign(forget_gate_bias_buffer, n_cell, n_batch,
                                                      forget_gate_scratch);
        tflite::tensor_utils::VectorBatchVectorAssign(cell_bias_buffer, n_cell, n_batch,
                                                      cell_scratch);
        tflite::tensor_utils::VectorBatchVectorAssign(output_gate_bias_buffer, n_cell, n_batch,
                                                      output_gate_scratch);
    } else {
        // Initialize scratch buffers with zeroes.
        if (!params.use_cifg) {
            tflite::tensor_utils::ZeroVector(input_gate_scratch, n_cell * n_batch);
        }
        tflite::tensor_utils::ZeroVector(forget_gate_scratch, n_cell * n_batch);
        tflite::tensor_utils::ZeroVector(cell_scratch, n_cell * n_batch);
        tflite::tensor_utils::ZeroVector(output_gate_scratch, n_cell * n_batch);
    }

    // For each batch and cell: compute input_weight * input.
    if (!params.use_cifg) {
        tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
                input_to_input_weights_buffer, n_cell, n_input, input_buffer, n_batch,
                input_gate_scratch, /*result_stride*/ 1);
    }
    tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
            input_to_forget_weights_buffer, n_cell, n_input, input_buffer, n_batch,
            forget_gate_scratch, /*result_stride*/ 1);
    tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(input_to_cell_weights_buffer, n_cell,
                                                              n_input, input_buffer, n_batch,
                                                              cell_scratch, /*result_stride*/ 1);
    tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
            input_to_output_weights_buffer, n_cell, n_input, input_buffer, n_batch,
            output_gate_scratch, /*result_stride*/ 1);

    // If auxiliary input is available then compute aux_input_weight * aux_input
    if (aux_input_buffer != nullptr) {
        if (!params.use_cifg) {
            tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
                    aux_input_to_input_weights_buffer, n_cell, n_aux_input, aux_input_buffer,
                    n_batch, input_gate_scratch,
                    /*result_stride=*/1);
        }

        tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
                aux_input_to_forget_weights_buffer, n_cell, n_aux_input, aux_input_buffer, n_batch,
                forget_gate_scratch, /*result_stride=*/1);
        tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
                aux_input_to_cell_weights_buffer, n_cell, n_aux_input, aux_input_buffer, n_batch,
                cell_scratch, /*result_stride=*/1);
        tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
                aux_input_to_output_weights_buffer, n_cell, n_aux_input, aux_input_buffer, n_batch,
                output_gate_scratch, /*result_stride=*/1);
    }

    // For each batch and cell: compute recurrent_weight * output_state.
    if (!params.use_cifg) {
        tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
                recurrent_to_input_weights_buffer, n_cell, n_output, output_state_in_buffer,
                n_batch, input_gate_scratch,
                /*result_stride*/ 1);
    }
    tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
            recurrent_to_forget_weights_buffer, n_cell, n_output, output_state_in_buffer, n_batch,
            forget_gate_scratch, /*result_stride*/ 1);
    tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
            recurrent_to_cell_weights_buffer, n_cell, n_output, output_state_in_buffer, n_batch,
            cell_scratch, /*result_stride*/ 1);
    tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
            recurrent_to_output_weights_buffer, n_cell, n_output, output_state_in_buffer, n_batch,
            output_gate_scratch, /*result_stride*/ 1);

    // For each batch and cell: update input gate.
    if (!params.use_cifg) {
        if (params.use_peephole) {
            tflite::tensor_utils::VectorBatchVectorCwiseProductAccumulate(
                    cell_to_input_weights_buffer, n_cell, cell_state_in_buffer, n_batch,
                    input_gate_scratch);
        }
        if (params.use_layer_norm) {
            tflite::tensor_utils::MeanStddevNormalization(input_gate_scratch, input_gate_scratch,
                                                          n_cell, n_batch, kLayerNormEpsilon);
            tflite::tensor_utils::VectorBatchVectorCwiseProduct(input_layer_norm_weights_buffer,
                                                                n_cell, input_gate_scratch, n_batch,
                                                                input_gate_scratch);
            tflite::tensor_utils::VectorBatchVectorAdd(input_gate_bias_buffer, n_cell, n_batch,
                                                       input_gate_scratch);
        }
        tflite::tensor_utils::ApplySigmoidToVector(input_gate_scratch, n_cell * n_batch,
                                                   input_gate_scratch);
    }

    // For each batch and cell: update forget gate.
    if (params.use_peephole) {
        tflite::tensor_utils::VectorBatchVectorCwiseProductAccumulate(cell_to_forget_weights_buffer,
                                                                      n_cell, cell_state_in_buffer,
                                                                      n_batch, forget_gate_scratch);
    }
    if (params.use_layer_norm) {
        tflite::tensor_utils::MeanStddevNormalization(forget_gate_scratch, forget_gate_scratch,
                                                      n_cell, n_batch, kLayerNormEpsilon);
        tflite::tensor_utils::VectorBatchVectorCwiseProduct(forget_layer_norm_weights_buffer,
                                                            n_cell, forget_gate_scratch, n_batch,
                                                            forget_gate_scratch);
        tflite::tensor_utils::VectorBatchVectorAdd(forget_gate_bias_buffer, n_cell, n_batch,
                                                   forget_gate_scratch);
    }
    tflite::tensor_utils::ApplySigmoidToVector(forget_gate_scratch, n_cell * n_batch,
                                               forget_gate_scratch);

    // For each batch and cell: update the cell.
    if (params.use_layer_norm) {
        tflite::tensor_utils::MeanStddevNormalization(cell_scratch, cell_scratch, n_cell, n_batch,
                                                      kLayerNormEpsilon);
        tflite::tensor_utils::VectorBatchVectorCwiseProduct(cell_layer_norm_weights_buffer, n_cell,
                                                            cell_scratch, n_batch, cell_scratch);
        tflite::tensor_utils::VectorBatchVectorAdd(cell_bias_buffer, n_cell, n_batch, cell_scratch);
    }
    tflite::tensor_utils::VectorVectorCwiseProduct(forget_gate_scratch, cell_state_in_buffer,
                                                   n_batch * n_cell, cell_state_out_buffer);
    tflite::tensor_utils::ApplyActivationToVector(cell_scratch, n_batch * n_cell, params.activation,
                                                  cell_scratch);
    if (params.use_cifg) {
        tflite::tensor_utils::Sub1Vector(forget_gate_scratch, n_batch * n_cell,
                                         forget_gate_scratch);
        tflite::tensor_utils::VectorVectorCwiseProductAccumulate(
                cell_scratch, forget_gate_scratch, n_batch * n_cell, cell_state_out_buffer);
    } else {
        tflite::tensor_utils::VectorVectorCwiseProductAccumulate(
                cell_scratch, input_gate_scratch, n_batch * n_cell, cell_state_out_buffer);
    }
    if (params.cell_clip > 0.0) {
        tflite::tensor_utils::ClipVector(cell_state_out_buffer, n_batch * n_cell, params.cell_clip,
                                         cell_state_out_buffer);
    }

    // For each batch and cell: update the output gate.
    if (params.use_peephole) {
        tflite::tensor_utils::VectorBatchVectorCwiseProductAccumulate(cell_to_output_weights_buffer,
                                                                      n_cell, cell_state_out_buffer,
                                                                      n_batch, output_gate_scratch);
    }
    if (params.use_layer_norm) {
        tflite::tensor_utils::MeanStddevNormalization(output_gate_scratch, output_gate_scratch,
                                                      n_cell, n_batch, kLayerNormEpsilon);
        tflite::tensor_utils::VectorBatchVectorCwiseProduct(output_layer_norm_weights_buffer,
                                                            n_cell, output_gate_scratch, n_batch,
                                                            output_gate_scratch);
        tflite::tensor_utils::VectorBatchVectorAdd(output_gate_bias_buffer, n_cell, n_batch,
                                                   output_gate_scratch);
    }
    tflite::tensor_utils::ApplySigmoidToVector(output_gate_scratch, n_batch * n_cell,
                                               output_gate_scratch);
    tflite::tensor_utils::ApplyActivationToVector(cell_state_out_buffer, n_batch * n_cell,
                                                  params.activation, cell_scratch);
    tflite::tensor_utils::VectorVectorCwiseProduct(output_gate_scratch, cell_scratch,
                                                   n_batch * n_cell, output_gate_scratch);

    // For each batch: update the projection and output_state.
    if (params.use_projection_weight) {
        if (params.use_projection_bias) {
            tflite::tensor_utils::VectorBatchVectorAssign(projection_bias_buffer, n_output, n_batch,
                                                          output_buffer);
        } else {
            tflite::tensor_utils::ZeroVector(output_buffer, n_batch * n_output);
        }
        tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
                projection_weights_buffer, n_output, n_cell, output_gate_scratch, n_batch,
                output_buffer,
                /*result_stride*/ 1);
        if (params.proj_clip > 0.0) {
            tflite::tensor_utils::ClipVector(output_buffer, n_batch * n_output, params.proj_clip,
                                             output_buffer);
        }
    } else {
        tflite::tensor_utils::CopyVector(output_gate_scratch, n_batch * n_output, output_buffer);
    }
    tflite::tensor_utils::CopyVector(output_buffer, n_batch * n_output, output_state_out_buffer);
    return true;
}

bool LSTMCell::Eval() {
    switch (input_->type) {
        case OperandType::TENSOR_FLOAT32: {
            LSTMEvalFloat32(params_, GetBuffer<const float>(input_), input_->shape(),
                            GetBuffer<const float>(input_to_input_weights_),
                            GetBuffer<const float>(input_to_forget_weights_),
                            GetBuffer<const float>(input_to_cell_weights_),
                            GetBuffer<const float>(input_to_output_weights_),
                            input_to_output_weights_->shape(),
                            GetBuffer<const float>(recurrent_to_input_weights_),
                            GetBuffer<const float>(recurrent_to_forget_weights_),
                            GetBuffer<const float>(recurrent_to_cell_weights_),
                            GetBuffer<const float>(recurrent_to_output_weights_),
                            recurrent_to_output_weights_->shape(),
                            GetBuffer<const float>(cell_to_input_weights_),
                            GetBuffer<const float>(cell_to_forget_weights_),
                            GetBuffer<const float>(cell_to_output_weights_),
                            /*aux_input_buffer=*/nullptr,
                            /*aux_input_to_input_weights_buffer=*/nullptr,
                            /*aux_input_to_forget_weights_buffer=*/nullptr,
                            /*aux_input_to_cell_weights_buffer=*/nullptr,
                            /*aux_input_to_output_weights_buffer=*/nullptr,
                            GetBuffer<const float>(input_gate_bias_),
                            GetBuffer<const float>(forget_gate_bias_),
                            GetBuffer<const float>(cell_bias_),
                            GetBuffer<const float>(output_gate_bias_),
                            GetBuffer<const float>(projection_weights_),
                            GetBuffer<const float>(projection_bias_),
                            GetBuffer<const float>(output_state_in_),
                            GetBuffer<const float>(cell_state_in_),
                            GetBuffer<const float>(input_layer_norm_weights_),
                            GetBuffer<const float>(forget_layer_norm_weights_),
                            GetBuffer<const float>(cell_layer_norm_weights_),
                            GetBuffer<const float>(output_layer_norm_weights_),
                            GetBuffer<float>(output_state_out_), GetBuffer<float>(cell_state_out_),
                            GetBuffer<float>(output_), GetBuffer<float>(scratch_buffer_));
        } break;
        case OperandType::TENSOR_FLOAT16: {
            LSTMEvalFloat16(params_, GetBuffer<const _Float16>(input_), input_->shape(),
                            GetOptionalBuffer<const _Float16>(input_to_input_weights_),
                            GetBuffer<const _Float16>(input_to_forget_weights_),
                            GetBuffer<const _Float16>(input_to_cell_weights_),
                            GetBuffer<const _Float16>(input_to_output_weights_),
                            input_to_output_weights_->shape(),
                            GetOptionalBuffer<const _Float16>(recurrent_to_input_weights_),
                            GetBuffer<const _Float16>(recurrent_to_forget_weights_),
                            GetBuffer<const _Float16>(recurrent_to_cell_weights_),
                            GetBuffer<const _Float16>(recurrent_to_output_weights_),
                            recurrent_to_output_weights_->shape(),
                            GetOptionalBuffer<const _Float16>(cell_to_input_weights_),
                            GetOptionalBuffer<const _Float16>(cell_to_forget_weights_),
                            GetOptionalBuffer<const _Float16>(cell_to_output_weights_),
                            /*aux_input_buffer=*/nullptr,
                            /*aux_input_to_input_weights_buffer=*/nullptr,
                            /*aux_input_to_forget_weights_buffer=*/nullptr,
                            /*aux_input_to_cell_weights_buffer=*/nullptr,
                            /*aux_input_to_output_weights_buffer=*/nullptr,
                            GetOptionalBuffer<const _Float16>(input_gate_bias_),
                            GetBuffer<const _Float16>(forget_gate_bias_),
                            GetBuffer<const _Float16>(cell_bias_),
                            GetBuffer<const _Float16>(output_gate_bias_),
                            GetOptionalBuffer<const _Float16>(projection_weights_),
                            GetOptionalBuffer<const _Float16>(projection_bias_),
                            GetBuffer<const _Float16>(output_state_in_),
                            GetBuffer<const _Float16>(cell_state_in_),
                            GetOptionalBuffer<const _Float16>(input_layer_norm_weights_),
                            GetOptionalBuffer<const _Float16>(forget_layer_norm_weights_),
                            GetOptionalBuffer<const _Float16>(cell_layer_norm_weights_),
                            GetOptionalBuffer<const _Float16>(output_layer_norm_weights_),
                            GetBuffer<_Float16>(output_state_out_),
                            GetBuffer<_Float16>(cell_state_out_), GetBuffer<_Float16>(output_),
                            GetBuffer<_Float16>(scratch_buffer_));
        } break;
        default: {
            LOG(ERROR) << "Unsupported data type: " << static_cast<int>(input_->type);
            return false;
        }
    }
    return true;
}

}  // namespace nn
}  // namespace android
