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

#include "BidirectionalSequenceLSTM.h"

#include "CpuExecutor.h"
#include "CpuOperationUtils.h"
#include "HalInterfaces.h"
#include "OperationsUtils.h"

#include "Tracing.h"

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

BidirectionalSequenceLSTM::BidirectionalSequenceLSTM(const Operation& operation,
                                                     std::vector<RunTimeOperandInfo>& operands) {
    input_ = GetInput(operation, operands, kInputTensor);

    fw_input_to_input_weights_ =
            GetInput(operation, operands, kFwInputToInputWeightsTensor);  // optional
    fw_input_to_forget_weights_ = GetInput(operation, operands, kFwInputToForgetWeightsTensor);
    fw_input_to_cell_weights_ = GetInput(operation, operands, kFwInputToCellWeightsTensor);
    fw_input_to_output_weights_ = GetInput(operation, operands, kFwInputToOutputWeightsTensor);

    fw_recurrent_to_input_weights_ =
            GetInput(operation, operands, kFwRecurrentToInputWeightsTensor);  // optional
    fw_recurrent_to_forget_weights_ =
            GetInput(operation, operands, kFwRecurrentToForgetWeightsTensor);
    fw_recurrent_to_cell_weights_ = GetInput(operation, operands, kFwRecurrentToCellWeightsTensor);
    fw_recurrent_to_output_weights_ =
            GetInput(operation, operands, kFwRecurrentToOutputWeightsTensor);

    fw_cell_to_input_weights_ =
            GetInput(operation, operands, kFwCellToInputWeightsTensor);  // optional
    fw_cell_to_forget_weights_ =
            GetInput(operation, operands, kFwCellToForgetWeightsTensor);  // optional
    fw_cell_to_output_weights_ =
            GetInput(operation, operands, kFwCellToOutputWeightsTensor);  // optional

    fw_input_gate_bias_ = GetInput(operation, operands, kFwInputGateBiasTensor);
    fw_forget_gate_bias_ = GetInput(operation, operands, kFwForgetGateBiasTensor);
    fw_cell_bias_ = GetInput(operation, operands, kFwCellGateBiasTensor);
    fw_output_gate_bias_ = GetInput(operation, operands, kFwOutputGateBiasTensor);

    fw_projection_weights_ = GetInput(operation, operands, kFwProjectionWeightsTensor);  // optional
    fw_projection_bias_ = GetInput(operation, operands, kFwProjectionBiasTensor);        // optional

    fw_activation_state_ = GetInput(operation, operands, kFwInputActivationStateTensor);
    fw_cell_state_ = GetInput(operation, operands, kFwInputCellStateTensor);

    bw_input_to_input_weights_ =
            GetInput(operation, operands, kBwInputToInputWeightsTensor);  // optional
    bw_input_to_forget_weights_ = GetInput(operation, operands, kBwInputToForgetWeightsTensor);
    bw_input_to_cell_weights_ = GetInput(operation, operands, kBwInputToCellWeightsTensor);
    bw_input_to_output_weights_ = GetInput(operation, operands, kBwInputToOutputWeightsTensor);

    bw_recurrent_to_input_weights_ =
            GetInput(operation, operands, kBwRecurrentToInputWeightsTensor);  // optional
    bw_recurrent_to_forget_weights_ =
            GetInput(operation, operands, kBwRecurrentToForgetWeightsTensor);
    bw_recurrent_to_cell_weights_ = GetInput(operation, operands, kBwRecurrentToCellWeightsTensor);
    bw_recurrent_to_output_weights_ =
            GetInput(operation, operands, kBwRecurrentToOutputWeightsTensor);

    bw_cell_to_input_weights_ =
            GetInput(operation, operands, kBwCellToInputWeightsTensor);  // optional
    bw_cell_to_forget_weights_ =
            GetInput(operation, operands, kBwCellToForgetWeightsTensor);  // optional
    bw_cell_to_output_weights_ =
            GetInput(operation, operands, kBwCellToOutputWeightsTensor);  // optional

    bw_input_gate_bias_ = GetInput(operation, operands, kBwInputGateBiasTensor);
    bw_forget_gate_bias_ = GetInput(operation, operands, kBwForgetGateBiasTensor);
    bw_cell_bias_ = GetInput(operation, operands, kBwCellGateBiasTensor);
    bw_output_gate_bias_ = GetInput(operation, operands, kBwOutputGateBiasTensor);

    bw_projection_weights_ = GetInput(operation, operands, kBwProjectionWeightsTensor);  // optional
    bw_projection_bias_ = GetInput(operation, operands, kBwProjectionBiasTensor);        // optional

    bw_activation_state_ = GetInput(operation, operands, kBwInputActivationStateTensor);
    bw_cell_state_ = GetInput(operation, operands, kBwInputCellStateTensor);

    aux_input_ = GetInput(operation, operands, kAuxInputTensor);
    fw_aux_input_to_input_weights_ = GetInput(operation, operands, kFwAuxInputToInputWeightsTensor);
    fw_aux_input_to_forget_weights_ =
            GetInput(operation, operands, kFwAuxInputToForgetWeightsTensor);
    fw_aux_input_to_cell_weights_ = GetInput(operation, operands, kFwAuxInputToCellWeightsTensor);
    fw_aux_input_to_output_weights_ =
            GetInput(operation, operands, kFwAuxInputToOutputWeightsTensor);
    bw_aux_input_to_input_weights_ = GetInput(operation, operands, kBwAuxInputToInputWeightsTensor);
    bw_aux_input_to_forget_weights_ =
            GetInput(operation, operands, kBwAuxInputToForgetWeightsTensor);
    bw_aux_input_to_cell_weights_ = GetInput(operation, operands, kBwAuxInputToCellWeightsTensor);
    bw_aux_input_to_output_weights_ =
            GetInput(operation, operands, kBwAuxInputToOutputWeightsTensor);

    fw_input_layer_norm_weights_ = GetInput(operation, operands, kFwInputLayerNormWeightsTensor);
    fw_forget_layer_norm_weights_ = GetInput(operation, operands, kFwForgetLayerNormWeightsTensor);
    fw_cell_layer_norm_weights_ = GetInput(operation, operands, kFwCellLayerNormWeightsTensor);
    fw_output_layer_norm_weights_ = GetInput(operation, operands, kFwOutputLayerNormWeightsTensor);
    bw_input_layer_norm_weights_ = GetInput(operation, operands, kBwInputLayerNormWeightsTensor);
    bw_forget_layer_norm_weights_ = GetInput(operation, operands, kBwForgetLayerNormWeightsTensor);
    bw_cell_layer_norm_weights_ = GetInput(operation, operands, kBwCellLayerNormWeightsTensor);
    bw_output_layer_norm_weights_ = GetInput(operation, operands, kBwOutputLayerNormWeightsTensor);

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
    params_.merge_outputs = getScalarData<bool>(*GetInput(operation, operands, kMergeOutputsParam));
    params_.time_major = getScalarData<bool>(*GetInput(operation, operands, kTimeMajorParam));
    params_.use_layer_norm = !IsNullInput(fw_input_layer_norm_weights_);

    fw_output_ = GetOutput(operation, operands, kFwOutputTensor);
    if (!params_.merge_outputs) {
        bw_output_ = GetOutput(operation, operands, kBwOutputTensor);
    }
}

bool BidirectionalSequenceLSTM::Prepare(const Operation& operation,
                                        std::vector<RunTimeOperandInfo>& operands,
                                        Shape* fwOutputShape, Shape* bwOutputShape) {
    // Inferring batch size, number of outputs and number of cells from the
    // input tensors.
    NN_CHECK(NumDimensions(input_) == 3);
    const uint32_t max_time = SizeOfDimension(input_, params_.time_major ? 0 : 1);
    const uint32_t n_batch = SizeOfDimension(input_, params_.time_major ? 1 : 0);
    const uint32_t n_input = SizeOfDimension(input_, 2);

    const uint32_t n_fw_cell = SizeOfDimension(fw_input_to_output_weights_, 0);
    NN_CHECK_EQ(NumDimensions(fw_input_to_output_weights_), 2);
    NN_CHECK_EQ(SizeOfDimension(fw_input_to_output_weights_, 1), n_input);

    NN_CHECK_EQ(NumDimensions(fw_recurrent_to_output_weights_), 2);
    NN_CHECK_EQ(SizeOfDimension(fw_recurrent_to_output_weights_, 0), n_fw_cell);
    const uint32_t n_fw_output = SizeOfDimension(fw_recurrent_to_output_weights_, 1);

    // Check that input tensor dimensions matches with each other.
    if (!LSTMCell::CheckInputTensorDimensions(
                input_, fw_input_to_input_weights_, fw_input_to_forget_weights_,
                fw_input_to_cell_weights_, fw_input_to_output_weights_,
                fw_recurrent_to_input_weights_, fw_recurrent_to_forget_weights_,
                fw_recurrent_to_cell_weights_, fw_recurrent_to_output_weights_,
                fw_cell_to_input_weights_, fw_cell_to_forget_weights_, fw_cell_to_output_weights_,
                fw_input_gate_bias_, fw_forget_gate_bias_, fw_cell_bias_, fw_output_gate_bias_,
                fw_projection_weights_, fw_projection_bias_, fw_input_layer_norm_weights_,
                fw_forget_layer_norm_weights_, fw_cell_layer_norm_weights_,
                fw_output_layer_norm_weights_, n_input, n_fw_output, n_fw_cell, &params_)) {
        return false;
    }

    const bool aux_inputs_all_or_none =
            (!IsNullInput(aux_input_) && !IsNullInput(fw_aux_input_to_cell_weights_) &&
             !IsNullInput(fw_aux_input_to_forget_weights_) &&
             !IsNullInput(fw_aux_input_to_output_weights_) &&
             !IsNullInput(bw_aux_input_to_cell_weights_) &&
             !IsNullInput(bw_aux_input_to_forget_weights_) &&
             !IsNullInput(bw_aux_input_to_output_weights_)) ||
            (IsNullInput(fw_aux_input_to_cell_weights_) &&
             IsNullInput(fw_aux_input_to_forget_weights_) &&
             IsNullInput(fw_aux_input_to_output_weights_) &&
             IsNullInput(bw_aux_input_to_cell_weights_) &&
             IsNullInput(bw_aux_input_to_forget_weights_) &&
             IsNullInput(bw_aux_input_to_output_weights_));
    NN_CHECK(aux_inputs_all_or_none);
    if (!IsNullInput(aux_input_)) {
        // Check that aux_input has the same dimensions (except last) as the input.
        NN_CHECK_EQ(aux_input_->shape().dimensions[0], input_->shape().dimensions[0]);
        NN_CHECK_EQ(aux_input_->shape().dimensions[1], input_->shape().dimensions[1]);
    }

    const uint32_t n_bw_cell = SizeOfDimension(bw_input_to_output_weights_, 0);
    NN_CHECK_EQ(NumDimensions(bw_input_to_output_weights_), 2);
    NN_CHECK_EQ(SizeOfDimension(bw_input_to_output_weights_, 1), n_input);

    NN_CHECK_EQ(NumDimensions(bw_recurrent_to_output_weights_), 2);
    NN_CHECK_EQ(SizeOfDimension(bw_recurrent_to_output_weights_, 0), n_bw_cell);
    const uint32_t n_bw_output = SizeOfDimension(bw_recurrent_to_output_weights_, 1);

    const Shape& inputShape = input_->shape();
    fwOutputShape->type = inputShape.type;
    fwOutputShape->offset = inputShape.offset;
    fwOutputShape->scale = inputShape.scale;
    fwOutputShape->dimensions.resize(3);
    fwOutputShape->dimensions[0] = params_.time_major ? max_time : n_batch;
    fwOutputShape->dimensions[1] = params_.time_major ? n_batch : max_time;
    fwOutputShape->dimensions[2] = params_.merge_outputs ? n_fw_output + n_bw_output : n_fw_output;

    // Check that input tensor dimensions matches with each other.
    if (!LSTMCell::CheckInputTensorDimensions(
                input_, bw_input_to_input_weights_, bw_input_to_forget_weights_,
                bw_input_to_cell_weights_, bw_input_to_output_weights_,
                bw_recurrent_to_input_weights_, bw_recurrent_to_forget_weights_,
                bw_recurrent_to_cell_weights_, bw_recurrent_to_output_weights_,
                bw_cell_to_input_weights_, bw_cell_to_forget_weights_, bw_cell_to_output_weights_,
                bw_input_gate_bias_, bw_forget_gate_bias_, bw_cell_bias_, bw_output_gate_bias_,
                bw_projection_weights_, bw_projection_bias_, bw_input_layer_norm_weights_,
                bw_forget_layer_norm_weights_, bw_cell_layer_norm_weights_,
                bw_output_layer_norm_weights_, n_input, n_bw_output, n_bw_cell, &params_)) {
        return false;
    }

    if (!params_.merge_outputs) {
        bwOutputShape->type = inputShape.type;
        bwOutputShape->offset = inputShape.offset;
        bwOutputShape->scale = inputShape.scale;
        bwOutputShape->dimensions.resize(3);
        bwOutputShape->dimensions[0] = params_.time_major ? max_time : n_batch;
        bwOutputShape->dimensions[1] = params_.time_major ? n_batch : max_time;
        bwOutputShape->dimensions[2] = n_bw_output;
    }

    if (params_.use_cifg) {
        fw_scratch_shape_.dimensions = {n_batch, n_fw_cell * 3};
        bw_scratch_shape_.dimensions = {n_batch, n_bw_cell * 3};
    } else {
        fw_scratch_shape_.dimensions = {n_batch, n_fw_cell * 4};
        bw_scratch_shape_.dimensions = {n_batch, n_bw_cell * 4};
    }
    fw_scratch_shape_.type = bw_scratch_shape_.type = inputShape.type;
    fw_scratch_shape_.offset = bw_scratch_shape_.offset = inputShape.offset;
    fw_scratch_shape_.scale = bw_scratch_shape_.scale = inputShape.scale;

    return true;
}

bool BidirectionalSequenceLSTM::Eval() {
    const uint32_t n_fw_output = SizeOfDimension(fw_recurrent_to_output_weights_, 1);
    const uint32_t n_bw_output = SizeOfDimension(bw_recurrent_to_output_weights_, 1);
    std::vector<uint32_t> fw_output_dims = input_->shape().dimensions;
    fw_output_dims[2] = n_fw_output;
    std::vector<uint32_t> bw_output_dims = fw_output_dims;
    bw_output_dims[2] = n_bw_output;
    const uint32_t n_fw_output_elements = fw_output_dims[0] * fw_output_dims[1] * fw_output_dims[2];
    const uint32_t n_output_elements =
            fw_output_dims[0] * fw_output_dims[1] * (fw_output_dims[2] + bw_output_dims[2]);

    switch (input_->type) {
        case OperandType::TENSOR_FLOAT32: {
            std::vector<float> fw_scratch_buffer(getNumberOfElements(fw_scratch_shape_));
            const bool kForwardSequence = true;
            LSTMCell::LSTMEvalFloat32(
                    params_, GetBuffer<const float>(input_), input_->shape(),
                    GetBuffer<const float>(fw_input_to_input_weights_),
                    GetBuffer<const float>(fw_input_to_forget_weights_),
                    GetBuffer<const float>(fw_input_to_cell_weights_),
                    GetBuffer<const float>(fw_input_to_output_weights_),
                    fw_input_to_output_weights_->shape(),
                    GetBuffer<const float>(fw_recurrent_to_input_weights_),
                    GetBuffer<const float>(fw_recurrent_to_forget_weights_),
                    GetBuffer<const float>(fw_recurrent_to_cell_weights_),
                    GetBuffer<const float>(fw_recurrent_to_output_weights_),
                    fw_recurrent_to_output_weights_->shape(),
                    GetBuffer<const float>(fw_cell_to_input_weights_),
                    GetBuffer<const float>(fw_cell_to_forget_weights_),
                    GetBuffer<const float>(fw_cell_to_output_weights_),
                    GetOptionalBuffer<const float>(aux_input_),
                    GetOptionalBuffer<const float>(fw_aux_input_to_input_weights_),
                    GetOptionalBuffer<const float>(fw_aux_input_to_forget_weights_),
                    GetOptionalBuffer<const float>(fw_aux_input_to_cell_weights_),
                    GetOptionalBuffer<const float>(fw_aux_input_to_output_weights_),
                    GetBuffer<const float>(fw_input_gate_bias_),
                    GetBuffer<const float>(fw_forget_gate_bias_),
                    GetBuffer<const float>(fw_cell_bias_),
                    GetBuffer<const float>(fw_output_gate_bias_),
                    GetBuffer<const float>(fw_projection_weights_),
                    GetBuffer<const float>(fw_projection_bias_),
                    GetBuffer<const float>(fw_activation_state_),
                    GetBuffer<const float>(fw_cell_state_),
                    GetOptionalBuffer<const float>(fw_input_layer_norm_weights_),
                    GetOptionalBuffer<const float>(fw_forget_layer_norm_weights_),
                    GetOptionalBuffer<const float>(fw_cell_layer_norm_weights_),
                    GetOptionalBuffer<const float>(fw_output_layer_norm_weights_),
                    GetBuffer<float>(fw_activation_state_), GetBuffer<float>(fw_cell_state_),
                    GetBuffer<float>(fw_output_), fw_scratch_buffer.data(), params_.time_major,
                    kForwardSequence);

            std::vector<float> bw_scratch_buffer(getNumberOfElements(bw_scratch_shape_));
            const bool kBackwardSequence = false;
            LSTMCell::LSTMEvalFloat32(
                    params_, GetBuffer<const float>(input_), input_->shape(),
                    GetBuffer<const float>(bw_input_to_input_weights_),
                    GetBuffer<const float>(bw_input_to_forget_weights_),
                    GetBuffer<const float>(bw_input_to_cell_weights_),
                    GetBuffer<const float>(bw_input_to_output_weights_),
                    bw_input_to_output_weights_->shape(),
                    GetBuffer<const float>(bw_recurrent_to_input_weights_),
                    GetBuffer<const float>(bw_recurrent_to_forget_weights_),
                    GetBuffer<const float>(bw_recurrent_to_cell_weights_),
                    GetBuffer<const float>(bw_recurrent_to_output_weights_),
                    bw_recurrent_to_output_weights_->shape(),
                    GetBuffer<const float>(bw_cell_to_input_weights_),
                    GetBuffer<const float>(bw_cell_to_forget_weights_),
                    GetBuffer<const float>(bw_cell_to_output_weights_),
                    GetOptionalBuffer<const float>(aux_input_),
                    GetOptionalBuffer<const float>(bw_aux_input_to_input_weights_),
                    GetOptionalBuffer<const float>(bw_aux_input_to_forget_weights_),
                    GetOptionalBuffer<const float>(bw_aux_input_to_cell_weights_),
                    GetOptionalBuffer<const float>(bw_aux_input_to_output_weights_),
                    GetBuffer<const float>(bw_input_gate_bias_),
                    GetBuffer<const float>(bw_forget_gate_bias_),
                    GetBuffer<const float>(bw_cell_bias_),
                    GetBuffer<const float>(bw_output_gate_bias_),
                    GetBuffer<const float>(bw_projection_weights_),
                    GetBuffer<const float>(bw_projection_bias_),
                    GetBuffer<const float>(bw_activation_state_),
                    GetBuffer<const float>(bw_cell_state_),
                    GetOptionalBuffer<const float>(bw_input_layer_norm_weights_),
                    GetOptionalBuffer<const float>(bw_forget_layer_norm_weights_),
                    GetOptionalBuffer<const float>(bw_cell_layer_norm_weights_),
                    GetOptionalBuffer<const float>(bw_output_layer_norm_weights_),
                    GetBuffer<float>(bw_activation_state_), GetBuffer<float>(bw_cell_state_),
                    params_.merge_outputs ? GetBuffer<float>(fw_output_) + n_fw_output_elements
                                          : GetBuffer<float>(bw_output_),
                    bw_scratch_buffer.data(), params_.time_major, kBackwardSequence);
            if (params_.merge_outputs) {
                std::vector<float> temp(n_output_elements);
                mergeThirdDimension(GetBuffer<float>(fw_output_), fw_output_dims,
                                    GetBuffer<float>(fw_output_) + n_fw_output_elements,
                                    bw_output_dims, temp.data());
                std::copy(temp.data(), temp.data() + n_output_elements,
                          GetBuffer<float>(fw_output_));
            }
        } break;
        case OperandType::TENSOR_FLOAT16: {
            std::vector<_Float16> fw_scratch_buffer(getNumberOfElements(fw_scratch_shape_));
            const bool kForwardSequence = true;
            LSTMCell::LSTMEvalFloat16(
                    params_, GetBuffer<const _Float16>(input_), input_->shape(),
                    GetOptionalBuffer<const _Float16>(fw_input_to_input_weights_),
                    GetBuffer<const _Float16>(fw_input_to_forget_weights_),
                    GetBuffer<const _Float16>(fw_input_to_cell_weights_),
                    GetBuffer<const _Float16>(fw_input_to_output_weights_),
                    fw_input_to_output_weights_->shape(),
                    GetOptionalBuffer<const _Float16>(fw_recurrent_to_input_weights_),
                    GetBuffer<const _Float16>(fw_recurrent_to_forget_weights_),
                    GetBuffer<const _Float16>(fw_recurrent_to_cell_weights_),
                    GetBuffer<const _Float16>(fw_recurrent_to_output_weights_),
                    fw_recurrent_to_output_weights_->shape(),
                    GetOptionalBuffer<const _Float16>(fw_cell_to_input_weights_),
                    GetOptionalBuffer<const _Float16>(fw_cell_to_forget_weights_),
                    GetOptionalBuffer<const _Float16>(fw_cell_to_output_weights_),
                    GetOptionalBuffer<const _Float16>(aux_input_),
                    GetOptionalBuffer<const _Float16>(fw_aux_input_to_input_weights_),
                    GetOptionalBuffer<const _Float16>(fw_aux_input_to_forget_weights_),
                    GetOptionalBuffer<const _Float16>(fw_aux_input_to_cell_weights_),
                    GetOptionalBuffer<const _Float16>(fw_aux_input_to_output_weights_),
                    GetOptionalBuffer<const _Float16>(fw_input_gate_bias_),
                    GetBuffer<const _Float16>(fw_forget_gate_bias_),
                    GetBuffer<const _Float16>(fw_cell_bias_),
                    GetBuffer<const _Float16>(fw_output_gate_bias_),
                    GetOptionalBuffer<const _Float16>(fw_projection_weights_),
                    GetOptionalBuffer<const _Float16>(fw_projection_bias_),
                    GetBuffer<const _Float16>(fw_activation_state_),
                    GetBuffer<const _Float16>(fw_cell_state_),
                    GetOptionalBuffer<const _Float16>(fw_input_layer_norm_weights_),
                    GetOptionalBuffer<const _Float16>(fw_forget_layer_norm_weights_),
                    GetOptionalBuffer<const _Float16>(fw_cell_layer_norm_weights_),
                    GetOptionalBuffer<const _Float16>(fw_output_layer_norm_weights_),
                    GetBuffer<_Float16>(fw_activation_state_), GetBuffer<_Float16>(fw_cell_state_),
                    GetBuffer<_Float16>(fw_output_), fw_scratch_buffer.data(), params_.time_major,
                    kForwardSequence);

            std::vector<_Float16> bw_scratch_buffer(getNumberOfElements(bw_scratch_shape_));
            const bool kBackwardSequence = false;
            LSTMCell::LSTMEvalFloat16(
                    params_, GetBuffer<const _Float16>(input_), input_->shape(),
                    GetOptionalBuffer<const _Float16>(bw_input_to_input_weights_),
                    GetBuffer<const _Float16>(bw_input_to_forget_weights_),
                    GetBuffer<const _Float16>(bw_input_to_cell_weights_),
                    GetBuffer<const _Float16>(bw_input_to_output_weights_),
                    bw_input_to_output_weights_->shape(),
                    GetOptionalBuffer<const _Float16>(bw_recurrent_to_input_weights_),
                    GetBuffer<const _Float16>(bw_recurrent_to_forget_weights_),
                    GetBuffer<const _Float16>(bw_recurrent_to_cell_weights_),
                    GetBuffer<const _Float16>(bw_recurrent_to_output_weights_),
                    bw_recurrent_to_output_weights_->shape(),
                    GetOptionalBuffer<const _Float16>(bw_cell_to_input_weights_),
                    GetOptionalBuffer<const _Float16>(bw_cell_to_forget_weights_),
                    GetOptionalBuffer<const _Float16>(bw_cell_to_output_weights_),
                    GetOptionalBuffer<const _Float16>(aux_input_),
                    GetOptionalBuffer<const _Float16>(bw_aux_input_to_input_weights_),
                    GetOptionalBuffer<const _Float16>(bw_aux_input_to_forget_weights_),
                    GetOptionalBuffer<const _Float16>(bw_aux_input_to_cell_weights_),
                    GetOptionalBuffer<const _Float16>(bw_aux_input_to_output_weights_),
                    GetOptionalBuffer<const _Float16>(bw_input_gate_bias_),
                    GetBuffer<const _Float16>(bw_forget_gate_bias_),
                    GetBuffer<const _Float16>(bw_cell_bias_),
                    GetBuffer<const _Float16>(bw_output_gate_bias_),
                    GetOptionalBuffer<const _Float16>(bw_projection_weights_),
                    GetOptionalBuffer<const _Float16>(bw_projection_bias_),
                    GetBuffer<const _Float16>(bw_activation_state_),
                    GetBuffer<const _Float16>(bw_cell_state_),
                    GetOptionalBuffer<const _Float16>(bw_input_layer_norm_weights_),
                    GetOptionalBuffer<const _Float16>(bw_forget_layer_norm_weights_),
                    GetOptionalBuffer<const _Float16>(bw_cell_layer_norm_weights_),
                    GetOptionalBuffer<const _Float16>(bw_output_layer_norm_weights_),
                    GetBuffer<_Float16>(bw_activation_state_), GetBuffer<_Float16>(bw_cell_state_),
                    params_.merge_outputs ? GetBuffer<_Float16>(fw_output_) + n_fw_output_elements
                                          : GetBuffer<_Float16>(bw_output_),
                    bw_scratch_buffer.data(), params_.time_major, kBackwardSequence);
            if (params_.merge_outputs) {
                std::vector<_Float16> temp(n_output_elements);
                mergeThirdDimension(GetBuffer<_Float16>(fw_output_), fw_output_dims,
                                    GetBuffer<_Float16>(fw_output_) + n_fw_output_elements,
                                    bw_output_dims, temp.data());
                std::copy(temp.data(), temp.data() + n_output_elements,
                          GetBuffer<_Float16>(fw_output_));
            }
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
