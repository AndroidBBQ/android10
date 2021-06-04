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

#include "ActivationFunctor.h"
#include "CpuOperationUtils.h"
#include "OperationResolver.h"

#include "tensorflow/lite/kernels/internal/optimized/legacy_optimized_ops.h"
#include "tensorflow/lite/kernels/internal/optimized/optimized_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {

namespace activation {

constexpr uint32_t kNumInputs = 1;
constexpr uint32_t kInputTensor = 0;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

template <typename T>
bool reluFloat(const T* inputData, const Shape& inputShape, T* outputData, const Shape& outputShape,
               float reluMin = 0.f, float reluMax = std::numeric_limits<float>::max()) {
    NNTRACE_COMP("reluX");
    int numElements = getNumberOfElements(inputShape);
    for (int i = 0; i < numElements; i++, inputData++, outputData++) {
        *outputData = static_cast<T>(
                std::min(std::max(reluMin, static_cast<float>(*inputData)), reluMax));
    }
    return true;
}
template bool reluFloat<float>(const float* inputData, const Shape& inputShape, float* outputData,
                               const Shape& outputShape, float reluMin, float reluMax);
template bool reluFloat<_Float16>(const _Float16* inputData, const Shape& inputShape,
                                  _Float16* outputData, const Shape& outputShape, float reluMin,
                                  float reluMax);

template <typename T>
bool relu1Float(const T* inputData, const Shape& inputShape, T* outputData,
                const Shape& outputShape) {
    return reluFloat(inputData, inputShape, outputData, outputShape, -1.f, 1.f);
}
template bool relu1Float<float>(const float* inputData, const Shape& inputShape, float* outputData,
                                const Shape& outputShape);
template bool relu1Float<_Float16>(const _Float16* inputData, const Shape& inputShape,
                                   _Float16* outputData, const Shape& outputShape);

template <typename T>
bool relu6Float(const T* inputData, const Shape& inputShape, T* outputData,
                const Shape& outputShape) {
    return reluFloat(inputData, inputShape, outputData, outputShape, 0.f, 6.f);
}
template bool relu6Float<float>(const float* inputData, const Shape& inputShape, float* outputData,
                                const Shape& outputShape);
template bool relu6Float<_Float16>(const _Float16* inputData, const Shape& inputShape,
                                   _Float16* outputData, const Shape& outputShape);

bool tanhFloat16(const _Float16* inputData, const Shape& inputShape, _Float16* outputData,
                 const Shape& outputShape) {
    NNTRACE_COMP("tanhFloat16");
    int numElements = getNumberOfElements(inputShape);
    for (int i = 0; i < numElements; i++, inputData++, outputData++) {
        *outputData = static_cast<_Float16>(std::tanh(static_cast<float>(*inputData)));
    }
    return true;
}

bool tanhFloat32(const float* inputData, const Shape& inputShape, float* outputData,
                 const Shape& outputShape) {
    NNTRACE_COMP("tanhFloat32");
    int numElements = getNumberOfElements(inputShape);
    for (int i = 0; i < numElements; i++, inputData++, outputData++) {
        *outputData = std::tanh(*inputData);
    }
    return true;
}

template <typename T>
bool logisticFloat(const T* inputData, const Shape& inputShape, T* outputData,
                   const Shape& outputShape) {
    NNTRACE_COMP("logisticFloat");
    int numElements = getNumberOfElements(inputShape);
    for (int i = 0; i < numElements; i++, inputData++, outputData++) {
        *outputData = static_cast<T>(1.f / (1.f + std::exp(static_cast<float>(-*inputData))));
    }
    return true;
}
template bool logisticFloat<float>(const float* inputData, const Shape& inputShape,
                                   float* outputData, const Shape& outputShape);
template bool logisticFloat<_Float16>(const _Float16* inputData, const Shape& inputShape,
                                      _Float16* outputData, const Shape& outputShape);

#define ANDROID_NN_RELUX_QUANT8(activation)                                           \
    int numElements = getNumberOfElements(inputShape);                                \
    int32_t output_activation_min = 0;                                                \
    int32_t output_activation_max = 0;                                                \
                                                                                      \
    CalculateActivationRangeUint8(activation, inputShape, &output_activation_min,     \
                                  &output_activation_max);                            \
                                                                                      \
    for (int i = 0; i < numElements; i++, inputData++, outputData++) {                \
        *outputData = std::min((uint8_t)output_activation_max,                        \
                               std::max((uint8_t)output_activation_min, *inputData)); \
    }

bool reluQuant8(const uint8_t* inputData, const Shape& inputShape, uint8_t* outputData,
                const Shape& outputShape) {
    NNTRACE_COMP("reluQuant8");
    ANDROID_NN_RELUX_QUANT8(kActivationRelu)
    return true;
}

bool relu1Quant8(const uint8_t* inputData, const Shape& inputShape, uint8_t* outputData,
                 const Shape& outputShape) {
    NNTRACE_COMP("relu1Quant8");
    ANDROID_NN_RELUX_QUANT8(kActivationRelu1)
    return true;
}

bool relu6Quant8(const uint8_t* inputData, const Shape& inputShape, uint8_t* outputData,
                 const Shape& outputShape) {
    NNTRACE_COMP("relu6Quant8");
    ANDROID_NN_RELUX_QUANT8(kActivationRelu6)
    return true;
}

#undef ANDROID_NN_RELUX_QUANT8

bool tanhQuant8(const uint8_t* inputData, const Shape& inputShape, uint8_t* outputData,
                const Shape& outputShape) {
    NNTRACE_TRANS("tanhQuant8");
    if (outputShape.offset != 128 || outputShape.scale != 1.f / 128) {
        LOG(ERROR) << "incorrect scale or offset for TANH output";
        return false;
    }

    int numElements = getNumberOfElements(inputShape);
    static constexpr int kInputIntegerBits = 4;

    const double input_real_multiplier =
            inputShape.scale * static_cast<double>(1 << (31 - kInputIntegerBits));

    int32_t input_multiplier = 0;
    int32_t input_left_shift = 0;
    if (!QuantizeMultiplierGreaterThanOne(input_real_multiplier, &input_multiplier,
                                          &input_left_shift)) {
        return false;
    }
    int32_t input_range_radius = CalculateInputRadius(kInputIntegerBits, input_left_shift);

    NNTRACE_COMP_SWITCH("optimized_ops::Tanh");
    tflite::optimized_ops::Tanh(inputData, convertShapeToTflshape(inputShape), inputShape.offset,
                                input_range_radius, input_multiplier, input_left_shift, outputData,
                                convertShapeToTflshape(outputShape));

    return true;
}

bool logisticQuant8(const uint8_t* inputData, const Shape& inputShape, uint8_t* outputData,
                    const Shape& outputShape) {
    NNTRACE_TRANS("logisticQuant8");
    if (outputShape.offset != 0 || outputShape.scale != 1.f / 256) {
        LOG(ERROR) << "incorrect scale / offset for output";
        return false;
    }

    int numElements = getNumberOfElements(inputShape);
    static constexpr int kInputIntegerBits = 4;

    const double input_real_multiplier =
            inputShape.scale * static_cast<double>(1 << (31 - kInputIntegerBits));

    int32_t input_multiplier = 0;
    int32_t input_left_shift = 0;
    if (!QuantizeMultiplierGreaterThanOne(input_real_multiplier, &input_multiplier,
                                          &input_left_shift)) {
        return false;
    }
    int32_t input_range_radius = CalculateInputRadius(kInputIntegerBits, input_left_shift);

    NNTRACE_COMP_SWITCH("optimized_ops::Logistic");
    tflite::optimized_ops::Logistic(
            inputData, convertShapeToTflshape(inputShape), inputShape.offset, input_range_radius,
            input_multiplier, input_left_shift, outputData, convertShapeToTflshape(outputShape));

    return true;
}

}  // namespace

bool validate(OperationType opType, const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    auto inputType = context->getInputType(kInputTensor);
    if (inputType == OperandType::TENSOR_FLOAT32) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
    } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        if (opType == OperationType::TANH) {
            NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
        } else {
            NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
        }
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << getOperationName(opType);
    }
    return validateInputTypes(context, {inputType}) && validateOutputTypes(context, {inputType});
}

bool prepare(OperationType opType, IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    NN_RET_CHECK_LE(getNumberOfDimensions(input), 4);
    Shape output = input;
    if (input.type == OperandType::TENSOR_QUANT8_ASYMM) {
        switch (opType) {
            case OperationType::RELU:
            case OperationType::RELU1:
            case OperationType::RELU6:
                break;
            case OperationType::LOGISTIC:
                output.scale = 1.f / 256;
                output.offset = 0;
                break;
            case OperationType::TANH:
                output.scale = 1.f / 128;
                output.offset = 128;
                break;
            default:
                NN_RET_CHECK_FAIL() << "Unsupported operation type";
        }
    }
    return context->setOutputShape(kOutputTensor, output);
}

bool executeRelu(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return reluFloat(context->getInputBuffer<_Float16>(kInputTensor),
                             context->getInputShape(kInputTensor),
                             context->getOutputBuffer<_Float16>(kOutputTensor),
                             context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return reluFloat(context->getInputBuffer<float>(kInputTensor),
                             context->getInputShape(kInputTensor),
                             context->getOutputBuffer<float>(kOutputTensor),
                             context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return reluQuant8(context->getInputBuffer<uint8_t>(kInputTensor),
                              context->getInputShape(kInputTensor),
                              context->getOutputBuffer<uint8_t>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation RELU";
    }
}

bool executeRelu1(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return relu1Float(context->getInputBuffer<_Float16>(kInputTensor),
                              context->getInputShape(kInputTensor),
                              context->getOutputBuffer<_Float16>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return relu1Float(context->getInputBuffer<float>(kInputTensor),
                              context->getInputShape(kInputTensor),
                              context->getOutputBuffer<float>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return relu1Quant8(context->getInputBuffer<uint8_t>(kInputTensor),
                               context->getInputShape(kInputTensor),
                               context->getOutputBuffer<uint8_t>(kOutputTensor),
                               context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation RELU1";
    }
}

bool executeRelu6(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return relu6Float(context->getInputBuffer<_Float16>(kInputTensor),
                              context->getInputShape(kInputTensor),
                              context->getOutputBuffer<_Float16>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return relu6Float(context->getInputBuffer<float>(kInputTensor),
                              context->getInputShape(kInputTensor),
                              context->getOutputBuffer<float>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return relu6Quant8(context->getInputBuffer<uint8_t>(kInputTensor),
                               context->getInputShape(kInputTensor),
                               context->getOutputBuffer<uint8_t>(kOutputTensor),
                               context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation RELU6";
    }
}

bool executeLogistic(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return logisticFloat(context->getInputBuffer<_Float16>(kInputTensor),
                                 context->getInputShape(kInputTensor),
                                 context->getOutputBuffer<_Float16>(kOutputTensor),
                                 context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return logisticFloat(context->getInputBuffer<float>(kInputTensor),
                                 context->getInputShape(kInputTensor),
                                 context->getOutputBuffer<float>(kOutputTensor),
                                 context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return logisticQuant8(context->getInputBuffer<uint8_t>(kInputTensor),
                                  context->getInputShape(kInputTensor),
                                  context->getOutputBuffer<uint8_t>(kOutputTensor),
                                  context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation LOGISTIC";
    }
}

bool executeTanh(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return tanhFloat16(context->getInputBuffer<_Float16>(kInputTensor),
                               context->getInputShape(kInputTensor),
                               context->getOutputBuffer<_Float16>(kOutputTensor),
                               context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return tanhFloat32(context->getInputBuffer<float>(kInputTensor),
                               context->getInputShape(kInputTensor),
                               context->getOutputBuffer<float>(kOutputTensor),
                               context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return tanhQuant8(context->getInputBuffer<uint8_t>(kInputTensor),
                              context->getInputShape(kInputTensor),
                              context->getOutputBuffer<uint8_t>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation TANH";
    }
}

}  // namespace activation

using std::placeholders::_1;
NN_REGISTER_OPERATION(RELU, "RELU", std::bind(activation::validate, OperationType::RELU, _1),
                      std::bind(activation::prepare, OperationType::RELU, _1),
                      activation::executeRelu, .allowZeroSizedInput = true);
NN_REGISTER_OPERATION(RELU1, "RELU1", std::bind(activation::validate, OperationType::RELU1, _1),
                      std::bind(activation::prepare, OperationType::RELU1, _1),
                      activation::executeRelu1, .allowZeroSizedInput = true);
NN_REGISTER_OPERATION(RELU6, "RELU6", std::bind(activation::validate, OperationType::RELU6, _1),
                      std::bind(activation::prepare, OperationType::RELU6, _1),
                      activation::executeRelu6, .allowZeroSizedInput = true);
NN_REGISTER_OPERATION(LOGISTIC, "LOGISTIC",
                      std::bind(activation::validate, OperationType::LOGISTIC, _1),
                      std::bind(activation::prepare, OperationType::LOGISTIC, _1),
                      activation::executeLogistic, .allowZeroSizedInput = true);
NN_REGISTER_OPERATION(TANH, "TANH", std::bind(activation::validate, OperationType::TANH, _1),
                      std::bind(activation::prepare, OperationType::TANH, _1),
                      activation::executeTanh, .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
