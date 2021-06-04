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

#include "SVDF.h"

#include "CpuExecutor.h"
#include "CpuOperationUtils.h"
#include "HalInterfaces.h"

#include "Tracing.h"

namespace android {
namespace nn {

SVDF::SVDF(const Operation& operation,
           std::vector<RunTimeOperandInfo>& operands) {
    NNTRACE_TRANS("SVDF::SVDF");
    input_ = GetInput(operation, operands, kInputTensor);
    weights_feature_ = GetInput(operation, operands, kWeightsFeatureTensor);
    weights_time_ = GetInput(operation, operands, kWeightsTimeTensor);
    bias_ = GetInput(operation, operands, kBiasTensor);
    state_in_ = GetInput(operation, operands, kStateInTensor);

    params_.rank_ = getScalarData<int>(*GetInput(operation, operands, kRankParam));
    params_.activation_ = static_cast<TfLiteFusedActivation>(getScalarData<int>(
        *GetInput(operation, operands, kActivationParam)));

    state_out_ = GetOutput(operation, operands, kStateOutTensor);
    output_ = GetOutput(operation, operands, kOutputTensor);
}

bool SVDF::Prepare(const Operation &operation,
                   std::vector<RunTimeOperandInfo> &operands,
                   Shape *stateShape,
                   Shape *outputShape) {
  NNTRACE_TRANS("SVDF::Prepare");
  // Check we have all the inputs and outputs we need.
  const int num_inputs = NumInputsWithValues(operation, operands);

  NN_CHECK(num_inputs == 6 || num_inputs == 7);
  NN_CHECK_EQ(NumOutputs(operation), 2);

  const RunTimeOperandInfo *input =
      GetInput(operation, operands, SVDF::kInputTensor);
  const RunTimeOperandInfo *weights_feature =
      GetInput(operation, operands, SVDF::kWeightsFeatureTensor);
  const RunTimeOperandInfo *weights_time =
      GetInput(operation, operands, SVDF::kWeightsTimeTensor);

  // Check all the parameters of tensor match within themselves and match the
  // input configuration.
  const int rank = getScalarData<int>(*GetInput(operation, operands, kRankParam));
  const uint32_t batch_size = SizeOfDimension(input, 0);
  const uint32_t num_filters = SizeOfDimension(weights_feature, 0);
  NN_CHECK_EQ(num_filters % rank, 0);
  const uint32_t num_units = num_filters / rank;
  const uint32_t memory_size = SizeOfDimension(weights_time, 1);
  NN_CHECK_EQ(SizeOfDimension(input, 1), SizeOfDimension(weights_feature, 1));
  NN_CHECK_EQ(SizeOfDimension(weights_time, 0), num_filters);

  const RunTimeOperandInfo *bias =
      GetInput(operation, operands, kBiasTensor);
  if (!IsNullInput(bias)) {
    NN_CHECK_EQ(SizeOfDimension(bias, 0), num_units);
  }

  // Resize state.
  const Shape &inputShape = input->shape();
  stateShape->type = inputShape.type;
  stateShape->dimensions = { batch_size, memory_size * num_filters };
  stateShape->offset = inputShape.offset;
  stateShape->scale = inputShape.scale;

  // Resize output.
  outputShape->type = inputShape.type;
  outputShape->dimensions = { batch_size, num_units };
  outputShape->offset = inputShape.offset;
  outputShape->scale = inputShape.scale;

  return true;
}

bool SVDF::Eval() {
    NNTRACE_TRANS("SVDF::Eval");
    switch (input_->type) {
        case OperandType::TENSOR_FLOAT16: {
            std::vector<float> inputDataFloat32(getNumberOfElements(input_->shape()));
            convertFloat16ToFloat32(reinterpret_cast<_Float16*>(input_->buffer), &inputDataFloat32);
            std::vector<float> inputStateDataFloat32(getNumberOfElements(state_in_->shape()));
            convertFloat16ToFloat32(reinterpret_cast<_Float16*>(state_in_->buffer),
                                    &inputStateDataFloat32);
            std::vector<float> biasDataFloat32(getNumberOfElements(bias_->shape()));
            if (!IsNullInput(bias_)) {
                convertFloat16ToFloat32(reinterpret_cast<_Float16*>(bias_->buffer),
                                        &biasDataFloat32);
            }
            std::vector<float> weightsFeatureDataFloat32(
                    getNumberOfElements(weights_feature_->shape()));
            convertFloat16ToFloat32(reinterpret_cast<_Float16*>(weights_feature_->buffer),
                                    &weightsFeatureDataFloat32);
            std::vector<float> weightsTimeDataFloat32(getNumberOfElements(weights_time_->shape()));
            convertFloat16ToFloat32(reinterpret_cast<_Float16*>(weights_time_->buffer),
                                    &weightsTimeDataFloat32);
            std::vector<float> outputDataFloat32(getNumberOfElements(output_->shape()));
            std::vector<float> outputStateDataFloat32(getNumberOfElements(state_out_->shape()));

            EvalFloat32(inputDataFloat32.data(), inputStateDataFloat32.data(),
                        biasDataFloat32.data(), weightsFeatureDataFloat32.data(),
                        weightsTimeDataFloat32.data(), outputDataFloat32.data(),
                        outputStateDataFloat32.data());
            convertFloat32ToFloat16(outputDataFloat32,
                                    reinterpret_cast<_Float16*>(output_->buffer));
            convertFloat32ToFloat16(outputStateDataFloat32,
                                    reinterpret_cast<_Float16*>(state_out_->buffer));
            break;
        }
        case OperandType::TENSOR_FLOAT32: {
            EvalFloat32(reinterpret_cast<float*>(input_->buffer),
                        reinterpret_cast<float*>(state_in_->buffer),
                        reinterpret_cast<float*>(bias_->buffer),
                        reinterpret_cast<float*>(weights_feature_->buffer),
                        reinterpret_cast<float*>(weights_time_->buffer),
                        reinterpret_cast<float*>(output_->buffer),
                        reinterpret_cast<float*>(state_out_->buffer));
            break;
        }
        default: {
            LOG(ERROR) << "Unsupported data type: " << static_cast<int>(input_->type);
            return false;
        }
    }
    return true;
}

void SVDF::EvalFloat32(const float* inputData, const float* inputStateData, const float* biasData,
                       const float* weightsFeatureData, const float* weightsTimeData,
                       float* outputData, float* outputStateData) {
    NNTRACE_COMP("SVDF::EvalFloat32");

    const int rank = params_.rank_;
    const int batch_size = SizeOfDimension(input_, 0);
    const int input_size = SizeOfDimension(input_, 1);
    const int num_filters = SizeOfDimension(weights_feature_, 0);
    const int num_units = num_filters / rank;
    const int memory_size = SizeOfDimension(weights_time_, 1);

    memcpy(outputStateData, inputStateData, sizeof(float) * batch_size * memory_size * num_filters);
    // Compute conv1d(inputs, weights_feature).
    for (int b = 0; b < batch_size; b++) {
        float* state_ptr_batch = outputStateData + b * memory_size * num_filters;
        for (int c = 0; c < num_filters; c++) {
            float* state_ptr = state_ptr_batch + c * memory_size;
            state_ptr[memory_size - 1] = 0.0;
        }
    }
    // The state left most column is used to save current cycle activation. This
    // is achieved by starting at state->data.f[memory_size - 1] and having the
    // stride equal to memory_size.
    tflite::tensor_utils::MatrixBatchVectorMultiplyAccumulate(
            weightsFeatureData, num_filters, input_size, inputData, batch_size,
            &outputStateData[memory_size - 1], memory_size);

    // Compute matmul(state, weights_time).
    // The right most column is used to save temporary output (with the size of
    // num_filters). This is achieved by starting at state->data.f and having the
    // stride equal to memory_size.
    float scratch[batch_size * num_filters];
    for (int b = 0; b < batch_size; b++) {
        float* state_out_ptr_batch = outputStateData + b * memory_size * num_filters;
        float* scratch_ptr_batch = scratch + b * num_filters;
        tflite::tensor_utils::BatchVectorBatchVectorDotProduct(
                weightsTimeData, state_out_ptr_batch, memory_size, num_filters, scratch_ptr_batch,
                /*result_stride=*/1);
    }

    // Initialize output with bias if provided.
    if (!IsNullInput(bias_)) {
        tflite::tensor_utils::VectorBatchVectorAssign(biasData, num_units, batch_size, outputData);
    } else {
        tflite::tensor_utils::ZeroVector(outputData, batch_size * num_units);
    }

    // Reduction sum
    for (int b = 0; b < batch_size; b++) {
        float* output_ptr_batch = outputData + b * num_units;
        float* scratch_ptr_batch = scratch + b * num_filters;
        tflite::tensor_utils::ReductionSumVector(scratch_ptr_batch, output_ptr_batch, num_units,
                                                 rank);
    }

    // Apply activation.
    for (int b = 0; b < batch_size; b++) {
        float* output_ptr_batch = outputData + b * num_units;
        tflite::tensor_utils::ApplyActivationToVector(output_ptr_batch, num_units,
                                                      params_.activation_, output_ptr_batch);
    }

    // Right shift the state.
    for (int b = 0; b < batch_size; b++) {
        float* state_out_ptr_batch = outputStateData + b * memory_size * num_filters;
        for (int f = 0; f < num_filters; f++) {
            tflite::tensor_utils::VectorShiftLeft(state_out_ptr_batch, memory_size,
                                                  /*shift_value=*/0.0);
            state_out_ptr_batch += memory_size;
        }
    }
}

}  // namespace nn
}  // namespace android
