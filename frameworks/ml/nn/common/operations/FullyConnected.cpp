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

#include "CpuOperationUtils.h"
#include "OperationResolver.h"

#include "tensorflow/lite/kernels/internal/optimized/legacy_optimized_ops.h"
#include "tensorflow/lite/kernels/internal/reference/reference_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {
namespace fully_connected {

constexpr char kOperationName[] = "FULLY_CONNECTED";

constexpr uint32_t kNumInputs = 4;
constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kWeightsTensor = 1;
constexpr uint32_t kBiasTensor = 2;
constexpr uint32_t kActivationScalar = 3;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

// executionMutex is used to protect concurrent access of non-threadsafe resources
// like gemmlowp::GemmContext.
// std::mutex is safe for pthreads on Android.
static std::mutex executionMutex;

bool fullyConnectedFloat32(const float* inputData, const Shape& inputShape,
                           const float* weightsData, const Shape& weightsShape,
                           const float* biasData, const Shape& biasShape, int32_t activation,
                           float* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("fullyConnectedFloat32");
    float output_activation_min, output_activation_max;
    CalculateActivationRangeFloat(activation, &output_activation_min, &output_activation_max);

    // b/80425683, optimized implementation produces incorrect results when the
    // number of input elements is the squre of batch_size.
    uint32_t batch_size = getSizeOfDimension(outputShape, 0);
    uint32_t input_n_elements = getNumberOfElements(inputShape);
    if (batch_size * batch_size == input_n_elements) {
        NNTRACE_COMP_SWITCH("reference_ops::FullyConnected");
        tflite::reference_ops::FullyConnected(inputData, convertShapeToDims(inputShape),
                                              weightsData, convertShapeToDims(weightsShape),
                                              biasData, convertShapeToDims(biasShape),
                                              output_activation_min, output_activation_max,
                                              outputData, convertShapeToDims(outputShape));
    } else {
        NNTRACE_COMP_SWITCH("optimized_ops::FullyConnected");
        tflite::optimized_ops::FullyConnected(inputData, convertShapeToDims(inputShape),
                                              weightsData, convertShapeToDims(weightsShape),
                                              biasData, convertShapeToDims(biasShape),
                                              output_activation_min, output_activation_max,
                                              outputData, convertShapeToDims(outputShape));
    }
    return true;
}

bool fullyConnectedFloat16(const _Float16* inputData, const Shape& inputShape,
                           const _Float16* weightsData, const Shape& weightsShape,
                           const _Float16* biasData, const Shape& biasShape, int32_t activation,
                           _Float16* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("fullyConnectedFloat16");
    std::vector<float> inputDataFloat32(getNumberOfElements(inputShape));
    convertFloat16ToFloat32(inputData, &inputDataFloat32);
    std::vector<float> weightsDataFloat32(getNumberOfElements(weightsShape));
    convertFloat16ToFloat32(weightsData, &weightsDataFloat32);
    std::vector<float> biasDataFloat32(getNumberOfElements(biasShape));
    convertFloat16ToFloat32(biasData, &biasDataFloat32);

    std::vector<float> outputDataFloat32(getNumberOfElements(outputShape));
    fullyConnectedFloat32(inputDataFloat32.data(), inputShape, weightsDataFloat32.data(),
                          weightsShape, biasDataFloat32.data(), biasShape, activation,
                          outputDataFloat32.data(), outputShape);
    convertFloat32ToFloat16(outputDataFloat32, outputData);

    return true;
}

bool fullyConnectedQuant8(const uint8_t* inputData, const Shape& inputShape,
                          const uint8_t* weightsData, const Shape& weightsShape,
                          const int32_t* biasData, const Shape& biasShape, int32_t activation,
                          uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("fullyConnectedQuant8");
    int32_t inputOffset = -inputShape.offset;
    int32_t weightsOffset = -weightsShape.offset;
    int32_t outputOffset = outputShape.offset;

    double realMultiplier = 0.0;
    int32_t outputMultiplier = 0;
    int32_t outputShift = 0;
    int32_t outputActivationMin = 0;
    int32_t outputActivationMax = 0;

    NN_RET_CHECK(GetQuantizedConvolutionMultipler(inputShape, weightsShape, biasShape, outputShape,
                                                  &realMultiplier));
    int exponent;
    NN_RET_CHECK(QuantizeMultiplier(realMultiplier, &outputMultiplier, &exponent));
    outputShift = -exponent;
    CalculateActivationRangeUint8(activation, outputShape, &outputActivationMin,
                                  &outputActivationMax);

    static gemmlowp::GemmContext gemmContext;

    // Prevent concurrent executions that access gemmContext.
    std::unique_lock<std::mutex> lock(executionMutex);
    // Alow gemmlowp automatically decide how many threads to use.
    gemmContext.set_max_num_threads(0);

    NNTRACE_COMP_SWITCH("optimized_ops::FullyConnected");
    tflite::optimized_ops::FullyConnected(inputData, convertShapeToDims(inputShape), inputOffset,
                                          weightsData, convertShapeToDims(weightsShape),
                                          weightsOffset, biasData, convertShapeToDims(biasShape),
                                          outputOffset, outputMultiplier, outputShift,
                                          outputActivationMin, outputActivationMax, outputData,
                                          convertShapeToDims(outputShape), &gemmContext);

    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    auto inputType = context->getInputType(kInputTensor);
    std::vector<OperandType> inExpectedTypes;
    std::vector<OperandType> outExpectedTypes;
    if (inputType == OperandType::TENSOR_FLOAT32) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
        inExpectedTypes = {
                OperandType::TENSOR_FLOAT32,
                OperandType::TENSOR_FLOAT32,
                OperandType::TENSOR_FLOAT32,
                OperandType::INT32,
        };
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
        inExpectedTypes = {
                OperandType::TENSOR_FLOAT16,
                OperandType::TENSOR_FLOAT16,
                OperandType::TENSOR_FLOAT16,
                OperandType::INT32,
        };
    } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        // NeuralNetworks.h specifies that ANEURALNETWORKS_FULLY_CONNECTED's output must
        // meet "outputScale > inputScale * weightsScale" for the operand type
        // ANEURALNETWORKS_TENSOR_QUANT8_ASYMM before API level 29.
        const float inputScale = context->getInputShape(kInputTensor).scale;
        const float weightsScale = context->getInputShape(kWeightsTensor).scale;
        const float outputScale = context->getOutputShape(kOutputTensor).scale;
        bool meetsQuantizedScaleConstraintBeforeV1_2 = (outputScale > inputScale * weightsScale);

        if (!meetsQuantizedScaleConstraintBeforeV1_2) {
            NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
        } else {
            NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
        }

        inExpectedTypes = {
                OperandType::TENSOR_QUANT8_ASYMM,
                OperandType::TENSOR_QUANT8_ASYMM,
                OperandType::TENSOR_INT32,
                OperandType::INT32,
        };
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported input tensor type for operation " << kOperationName;
        return false;
    }
    NN_RET_CHECK(validateInputTypes(context, inExpectedTypes));
    NN_RET_CHECK(validateOutputTypes(context, {inputType}));
    return true;
}

bool prepare(IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    Shape weights = context->getInputShape(kWeightsTensor);
    Shape bias = context->getInputShape(kBiasTensor);

    // Check all the parameters of tensor match within themselves and match the
    // input configuration.
    NN_RET_CHECK(input.type == weights.type);
    if (input.type == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK(bias.type == OperandType::TENSOR_INT32);
    } else {
        NN_RET_CHECK(input.type == bias.type);
    }
    // The Tensorflow fully connected layer specification says that input should
    // be of at least rank 2, so we check. Tflite doesn't check.
    NN_RET_CHECK_GE(getNumberOfDimensions(input), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(weights), 2);
    uint32_t input_n_elements = getNumberOfElements(input);
    uint32_t num_units = getSizeOfDimension(weights, 0);
    uint32_t input_size = getSizeOfDimension(weights, 1);
    // Only batch_size can be 0.
    NN_RET_CHECK_GT(num_units, 0);
    NN_RET_CHECK_GT(input_size, 0);
    uint32_t batch_size = input_n_elements / input_size;
    NN_RET_CHECK_EQ(getSizeOfDimension(bias, 0), num_units);
    NN_RET_CHECK_EQ(input_size * batch_size, input_n_elements);

    Shape output = context->getOutputShape(kOutputTensor);
    output.type = input.type;
    output.dimensions = {batch_size, num_units};
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT32:
            return fullyConnectedFloat32(context->getInputBuffer<float>(kInputTensor),
                                         context->getInputShape(kInputTensor),
                                         context->getInputBuffer<float>(kWeightsTensor),
                                         context->getInputShape(kWeightsTensor),
                                         context->getInputBuffer<float>(kBiasTensor),
                                         context->getInputShape(kBiasTensor),
                                         context->getInputValue<int32_t>(kActivationScalar),
                                         context->getOutputBuffer<float>(kOutputTensor),
                                         context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT16:
            return fullyConnectedFloat16(context->getInputBuffer<_Float16>(kInputTensor),
                                         context->getInputShape(kInputTensor),
                                         context->getInputBuffer<_Float16>(kWeightsTensor),
                                         context->getInputShape(kWeightsTensor),
                                         context->getInputBuffer<_Float16>(kBiasTensor),
                                         context->getInputShape(kBiasTensor),
                                         context->getInputValue<int32_t>(kActivationScalar),
                                         context->getOutputBuffer<_Float16>(kOutputTensor),
                                         context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return fullyConnectedQuant8(context->getInputBuffer<uint8_t>(kInputTensor),
                                        context->getInputShape(kInputTensor),
                                        context->getInputBuffer<uint8_t>(kWeightsTensor),
                                        context->getInputShape(kWeightsTensor),
                                        context->getInputBuffer<int32_t>(kBiasTensor),
                                        context->getInputShape(kBiasTensor),
                                        context->getInputValue<int32_t>(kActivationScalar),
                                        context->getOutputBuffer<uint8_t>(kOutputTensor),
                                        context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace fully_connected

NN_REGISTER_OPERATION(FULLY_CONNECTED, fully_connected::kOperationName, fully_connected::validate,
                      fully_connected::prepare, fully_connected::execute,
                      .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
