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

#include "OperationsUtils.h"
#define LOG_TAG "Operations"

#include "HalInterfaces.h"
#include "IndexedShapeWrapper.h"
#include "OperationResolver.h"

namespace android {
namespace nn {
namespace dequantize {

constexpr uint32_t kNumInputs = 1;
constexpr uint32_t kInputTensor = 0;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

template <typename InputType, typename OutputType>
bool compute(const InputType* inputData, const Shape& inputShape, OutputType* outputData) {
    const int numElements = getNumberOfElements(inputShape);
    const int32_t zeroPoint = inputShape.offset;
    const float scale = inputShape.scale;
    for (int i = 0; i < numElements; ++i) {
        const int32_t value = inputData[i];
        outputData[i] = static_cast<OutputType>(scale * (value - zeroPoint));
    }
    return true;
}

template <typename OutputType>
bool computePerChannel(const int8_t* inputData, const Shape& inputShape, OutputType* outputData) {
    // First we calculate a stride which is the number of elements we need to
    // skip to change an index along a dimension with different quantization
    // scales.
    const int channelDim = inputShape.extraParams.channelQuant().channelDim;
    int stride = 1;
    for (int i = getNumberOfDimensions(inputShape) - 1; i > channelDim; --i) {
        stride *= getSizeOfDimension(inputShape, i);
    }

    const int numElements = getNumberOfElements(inputShape);
    const int32_t zeroPoint = inputShape.offset;

    for (int i = 0; i < numElements; ++i) {
        // To get current index along the quantized dimension we calculate how
        // many even |strides| we looped through and take this number modulo the
        // size of the dimension (so that we don't have an overflow if the
        // channelDim is not 0).
        const int scaleIndex = (i / stride) % getSizeOfDimension(inputShape, channelDim);
        const float scale = inputShape.extraParams.channelQuant().scales[scaleIndex];
        const int32_t value = inputData[i];
        outputData[i] = static_cast<OutputType>(scale * (value - zeroPoint));
    }
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);

    const OperandType inputType = context->getInputType(kInputTensor);
    const OperandType outputType = context->getOutputType(kOutputTensor);

    if (inputType == OperandType::TENSOR_QUANT8_ASYMM &&
        outputType == OperandType::TENSOR_FLOAT32) {
        return validateHalVersion(context, HalVersion::V1_0);
    }

    NN_RET_CHECK(inputType == OperandType::TENSOR_QUANT8_ASYMM ||
                 inputType == OperandType::TENSOR_QUANT8_SYMM ||
                 inputType == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL)
            << "Unsupported input operand type for DEQUANTIZE op: " << toString(inputType);
    NN_RET_CHECK(outputType == OperandType::TENSOR_FLOAT16 ||
                 outputType == OperandType::TENSOR_FLOAT32)
            << "Unsupported output operand type for DEQUANTIZE op: " << toString(outputType);
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    const Shape& input = context->getInputShape(kInputTensor);
    Shape output = context->getOutputShape(kOutputTensor);
    output.dimensions = input.dimensions;
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;

    const OperandType inputType = context->getInputType(kInputTensor);
    const OperandType outputType = context->getOutputType(kOutputTensor);

    const Shape& inputShape = context->getInputShape(kInputTensor);
    if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        const uint8_t* inputBuffer = context->getInputBuffer<uint8_t>(kInputTensor);
        if (outputType == OperandType::TENSOR_FLOAT16) {
            return compute(inputBuffer, inputShape,
                           context->getOutputBuffer<_Float16>(kOutputTensor));
        } else if (outputType == OperandType::TENSOR_FLOAT32) {
            return compute(inputBuffer, inputShape, context->getOutputBuffer<float>(kOutputTensor));
        }
    } else if (inputType == OperandType::TENSOR_QUANT8_SYMM) {
        const int8_t* inputBuffer = context->getInputBuffer<int8_t>(kInputTensor);
        if (outputType == OperandType::TENSOR_FLOAT16) {
            return compute(inputBuffer, inputShape,
                           context->getOutputBuffer<_Float16>(kOutputTensor));
        } else if (outputType == OperandType::TENSOR_FLOAT32) {
            return compute(inputBuffer, inputShape, context->getOutputBuffer<float>(kOutputTensor));
        }
    } else if (inputType == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
        const int8_t* inputBuffer = context->getInputBuffer<int8_t>(kInputTensor);
        if (outputType == OperandType::TENSOR_FLOAT16) {
            return computePerChannel(inputBuffer, inputShape,
                                     context->getOutputBuffer<_Float16>(kOutputTensor));
        } else if (outputType == OperandType::TENSOR_FLOAT32) {
            return computePerChannel(inputBuffer, inputShape,
                                     context->getOutputBuffer<float>(kOutputTensor));
        }
    }
    NN_RET_CHECK_FAIL() << "Unsupported tensor types combination for dequantize op. (input type: "
                        << toString(inputType) << " output type: " << toString(outputType) << ")";
}

}  // namespace dequantize

NN_REGISTER_OPERATION(DEQUANTIZE, "DEQUANTIZE", dequantize::validate, dequantize::prepare,
                      dequantize::execute, .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
