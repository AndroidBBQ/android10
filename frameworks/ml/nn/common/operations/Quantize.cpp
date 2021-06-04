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
#include "Tracing.h"

#include <cmath>

namespace android {
namespace nn {
namespace quantize {

constexpr uint32_t kNumInputs = 1;
constexpr uint32_t kInputTensor = 0;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

bool quantizeFloat32ToQuant8(const float* inputData, uint8_t* outputData,
                             const Shape& outputShape) {
    NNTRACE_COMP("quantizeFloat32ToQuant8");
    uint32_t size = getNumberOfElements(outputShape);
    for (uint32_t i = 0; i < size; ++i) {
        outputData[i] = static_cast<uint8_t>(std::max<float>(
                0, std::min<float>(255, outputShape.offset +
                                                std::round(inputData[i] / outputShape.scale))));
    }
    return true;
}

bool quantizeFloat16ToQuant8(const _Float16* inputData, uint8_t* outputData,
                             const Shape& outputShape) {
    NNTRACE_COMP("quantizeFloat16ToQuant8");
    uint32_t size = getNumberOfElements(outputShape);
    for (uint32_t i = 0; i < size; ++i) {
        outputData[i] = static_cast<uint8_t>(std::max<float>(
                0, std::min<float>(255, outputShape.offset +
                                                std::round(inputData[i] / outputShape.scale))));
    }
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);

    const OperandType inputType = context->getInputType(kInputTensor);
    const OperandType outputType = context->getOutputType(kOutputTensor);

    NN_RET_CHECK(inputType == OperandType::TENSOR_FLOAT16 ||
                 inputType == OperandType::TENSOR_FLOAT32)
            << "Unsupported input operand type for QUANTIZE op: " << toString(inputType);
    NN_RET_CHECK(outputType == OperandType::TENSOR_QUANT8_ASYMM)
            << "Unsupported output operand type for QUANTIZE op: " << toString(outputType);
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
    if (inputType == OperandType::TENSOR_FLOAT32) {
        return quantizeFloat32ToQuant8(context->getInputBuffer<float>(kInputTensor),
                                       context->getOutputBuffer<uint8_t>(kOutputTensor),
                                       context->getOutputShape(kOutputTensor));
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        return quantizeFloat16ToQuant8(context->getInputBuffer<_Float16>(kInputTensor),
                                       context->getOutputBuffer<uint8_t>(kOutputTensor),
                                       context->getOutputShape(kOutputTensor));
    }
    NN_RET_CHECK_FAIL() << "Unsupported tensor types combination for QUANTIZE op. (input type: "
                        << toString(inputType)
                        << " output type: " << toString(context->getOutputType(kOutputTensor))
                        << ")";
}

}  // namespace quantize

NN_REGISTER_OPERATION(QUANTIZE, "QUANTIZE", quantize::validate, quantize::prepare,
                      quantize::execute, .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
