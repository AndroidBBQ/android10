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

#include "RNN.h"

#include "CpuExecutor.h"
#include "CpuOperationUtils.h"
#include "HalInterfaces.h"

#include "Tracing.h"

namespace android {
namespace nn {

RNN::RNN(const Operation& operation,
         std::vector<RunTimeOperandInfo>& operands) {
  NNTRACE_TRANS("RNN::RNN");
  input_ = GetInput(operation, operands, kInputTensor);
  weights_ = GetInput(operation, operands, kWeightsTensor);
  recurrent_weights_ = GetInput(operation, operands, kRecurrentWeightsTensor);
  hidden_state_in_ = GetInput(operation, operands, kHiddenStateInTensor);
  bias_ = GetInput(operation, operands, kBiasTensor);

  activation_ = static_cast<ActivationFn>(
      getScalarData<int32_t>(operands[operation.inputs[kActivationParam]]));

  hidden_state_out_ = GetOutput(operation, operands, kHiddenStateOutTensor);
  output_ = GetOutput(operation, operands, kOutputTensor);
}

bool RNN::Prepare(const Operation &operation,
                  std::vector<RunTimeOperandInfo> &operands,
                  Shape *hiddenStateShape,
                  Shape *outputShape) {
  NNTRACE_TRANS("RNN::Prepare");
  // Check we have all the inputs and outputs we need.
  const int num_inputs = NumInputsWithValues(operation, operands);
  NN_CHECK(num_inputs == 5 || num_inputs == 6);
  NN_CHECK_EQ(NumOutputs(operation), 2);

  const RunTimeOperandInfo *input =
      GetInput(operation, operands, kInputTensor);
  const RunTimeOperandInfo *input_weights =
      GetInput(operation, operands, kWeightsTensor);
  const RunTimeOperandInfo *recurrent_weights =
      GetInput(operation, operands, kRecurrentWeightsTensor);
  const RunTimeOperandInfo *bias =
      GetInput(operation, operands, kBiasTensor);

  // Check all the parameters of tensor match within themselves and match the
  // input configuration.
  const uint32_t batch_size = SizeOfDimension(input, 0);
  const uint32_t num_units = SizeOfDimension(input_weights, 0);
  NN_CHECK_EQ(SizeOfDimension(input, 1), SizeOfDimension(input_weights, 1));
  NN_CHECK_EQ(SizeOfDimension(input_weights, 0), SizeOfDimension(bias, 0));
  NN_CHECK_EQ(SizeOfDimension(recurrent_weights, 0), SizeOfDimension(bias, 0));
  NN_CHECK_EQ(SizeOfDimension(recurrent_weights, 1), SizeOfDimension(bias, 0));

  const Shape &inputShape = input->shape();

  // Resize state.
  hiddenStateShape->type = inputShape.type;
  hiddenStateShape->dimensions = { batch_size, num_units };

  // Resize output.
  outputShape->type = inputShape.type;
  outputShape->dimensions = { batch_size, num_units };

  return true;
}

bool RNN::Eval() {
    switch (input_->type) {
        case OperandType::TENSOR_FLOAT16: {
            RNNStep<_Float16>(reinterpret_cast<_Float16*>(input_->buffer), input_->shape(),
                              reinterpret_cast<_Float16*>(hidden_state_in_->buffer),
                              reinterpret_cast<_Float16*>(bias_->buffer),
                              reinterpret_cast<_Float16*>(weights_->buffer), weights_->shape(),
                              reinterpret_cast<_Float16*>(recurrent_weights_->buffer),
                              recurrent_weights_->shape(), activation_,
                              reinterpret_cast<_Float16*>(output_->buffer));
            memcpy(hidden_state_out_->buffer, output_->buffer,
                   sizeof(_Float16) * getNumberOfElements(output_->shape()));
            break;
        }
        case OperandType::TENSOR_FLOAT32: {
            RNNStep<float>(reinterpret_cast<float*>(input_->buffer), input_->shape(),
                           reinterpret_cast<float*>(hidden_state_in_->buffer),
                           reinterpret_cast<float*>(bias_->buffer),
                           reinterpret_cast<float*>(weights_->buffer), weights_->shape(),
                           reinterpret_cast<float*>(recurrent_weights_->buffer),
                           recurrent_weights_->shape(), activation_,
                           reinterpret_cast<float*>(output_->buffer));
            memcpy(hidden_state_out_->buffer, output_->buffer,
                   sizeof(float) * getNumberOfElements(output_->shape()));
            break;
        }
        default: {
            LOG(ERROR) << "Unsupported data type: " << static_cast<int>(input_->type);
            return false;
        }
    }
    return true;
}

template <typename T>
bool RNN::RNNStep(const T* inputData, const Shape& inputShape, const T* hiddenStateInputData,
                  const T* biasData, const T* weightsData, const Shape& weightsShape,
                  const T* recurrentWeightsData, const Shape& recurrentWeightsShape,
                  const int32_t activation, T* outputData) {
    NNTRACE_COMP("RNN::Eval");

    Shape dummyShape;
    uint32_t numUnits = weightsShape.dimensions[0];
    return RNNStep<T>(inputData, inputShape, /*auxInputData=*/nullptr, /*auxInputShape=*/dummyShape,
                      hiddenStateInputData, biasData, weightsData, weightsShape,
                      /*auxWeightsData=*/nullptr, /*auxWeightsShape=*/dummyShape,
                      recurrentWeightsData, recurrentWeightsShape, activation,
                      /*outputBatchStride=*/numUnits, /*outputBatchOffset=*/0, outputData);
}

// A more general version of the RNNStep function.
// Auxiliary input is treated as if it was concatenated to a regular input and
// the result was multiplied by the weights matrix which was also concatenated
// with auxiliary weights.
template <typename T>
bool RNN::RNNStep(const T* inputData, const Shape& inputShape, const T* auxInputData,
                  const Shape& auxInputShape, const T* hiddenStateInputData, const T* biasData,
                  const T* weightsData, const Shape& weightsShape, const T* auxWeightsData,
                  const Shape& auxWeightsShape, const T* recurrentWeightsData,
                  const Shape& recurrentWeightsShape, const int32_t activation,
                  const uint32_t outputBatchStride, const uint32_t outputBatchOffset, T* outputData,
                  T* hiddenStateOutput) {
    NNTRACE_COMP("RNN::Eval");

    const uint32_t batch_size = inputShape.dimensions[0];
    const uint32_t num_units = weightsShape.dimensions[0];
    const uint32_t input_size = inputShape.dimensions[1];
    const uint32_t input_weights_stride = weightsShape.dimensions[1];
    const uint32_t recurrent_weights_stride = recurrentWeightsShape.dimensions[1];

    uint32_t aux_input_size = 0;
    uint32_t aux_input_weights_stride = 0;
    bool hasAuxInput = (auxInputData != nullptr);
    if (hasAuxInput) {
        aux_input_size = auxInputShape.dimensions[1];
        aux_input_weights_stride = auxWeightsShape.dimensions[1];
    }

    // For each batch
    for (uint32_t b = 0; b < batch_size; b++) {
        // Initialize the pointer to input, output and bias.
        const T* input_ptr_batch = inputData + b * input_size;
        const T* hidden_state_in_ptr_batch = hiddenStateInputData + b * num_units;
        const T* aux_input_ptr_batch = nullptr;
        if (hasAuxInput) {
            aux_input_ptr_batch = auxInputData + b * aux_input_size;
        }
        T* output_ptr_batch = outputData + b * outputBatchStride + outputBatchOffset;

        // Initialize input_weights and recurrent_weights.
        const T* input_weights_ptr = weightsData;
        const T* recurrent_weights_ptr = recurrentWeightsData;
        const T* aux_input_weights_ptr = nullptr;
        if (hasAuxInput) {
            aux_input_weights_ptr = auxWeightsData;
        }

        // Output = bias
        for (uint32_t o = 0; o < num_units; o++) {
            output_ptr_batch[o] = biasData[o];
        }

        // Output += input * input_weights
        for (uint32_t o = 0; o < num_units; o++) {
            for (uint32_t i = 0; i < input_size; i++) {
                output_ptr_batch[o] += input_ptr_batch[i] * input_weights_ptr[i];
            }
            input_weights_ptr += input_weights_stride;
        }

        if (hasAuxInput) {
            // Output += aux_input * aux_input_weights
            for (uint32_t o = 0; o < num_units; o++) {
                for (uint32_t i = 0; i < input_size; i++) {
                    output_ptr_batch[o] += aux_input_ptr_batch[i] * aux_input_weights_ptr[i];
                }
                aux_input_weights_ptr += aux_input_weights_stride;
            }
        }

        // Output += recurrent_weights * hidden_state
        for (uint32_t o = 0; o < num_units; o++) {
            for (uint32_t h = 0; h < num_units; h++) {
                output_ptr_batch[o] += hidden_state_in_ptr_batch[h] * recurrent_weights_ptr[h];
            }
            recurrent_weights_ptr += recurrent_weights_stride;
        }

        // Output = activation(Output)
        for (uint32_t o = 0; o < num_units; o++) {
            output_ptr_batch[o] =
                    (ActivationFunctor(static_cast<ActivationFn>(activation)))(output_ptr_batch[o]);
            if (hiddenStateOutput != nullptr) {
                *hiddenStateOutput = output_ptr_batch[o];
                ++hiddenStateOutput;
            }
        }
    }

    return true;
}

}  // namespace nn
}  // namespace android
