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

#include "CpuOperationUtils.h"
#include "OperationResolver.h"

#include "tensorflow/lite/kernels/internal/optimized/optimized_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {
namespace l2_norm {

constexpr char kOperationName[] = "L2_NORMALIZATION";

constexpr uint32_t kNumInputs = 2;
constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kAxisScalar = 1;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

inline bool l2normFloat32Impl(const float* inputData, const Shape& inputShape, int32_t axis,
                              float* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("l2normFloat32");
    const uint32_t outerSize = getNumberOfElements(inputShape, 0, axis);
    const uint32_t axisSize = getSizeOfDimension(inputShape, axis);
    const uint32_t innerSize =
            getNumberOfElements(inputShape, axis + 1, getNumberOfDimensions(inputShape));
    for (uint32_t outer = 0; outer < outerSize; ++outer) {
        const float* inputBeg = inputData + outer * axisSize * innerSize;
        const float* inputEnd = inputBeg + axisSize * innerSize;
        float* outputBeg = outputData + outer * axisSize * innerSize;
        for (uint32_t inner = 0; inner < innerSize; ++inner, ++inputBeg, ++inputEnd, ++outputBeg) {
            float sum = 0.0f;
            for (const float* p = inputBeg; p < inputEnd; p += innerSize) {
                float val = *p;
                sum += val * val;
            }
            float l2_norm = std::sqrt(sum);
            float* pOut = outputBeg;
            for (const float* p = inputBeg; p < inputEnd; p += innerSize, pOut += innerSize) {
                *pOut = *p / l2_norm;
            }
        }
    }
    return true;
}

inline bool l2normQuant8Impl(const uint8_t* inputData, const Shape& inputShape, int32_t axis,
                             uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("l2normQuant8");
    const uint32_t outerSize = getNumberOfElements(inputShape, 0, axis);
    const uint32_t axisSize = getSizeOfDimension(inputShape, axis);
    const uint32_t innerSize =
            getNumberOfElements(inputShape, axis + 1, getNumberOfDimensions(inputShape));
    for (uint32_t outer = 0; outer < outerSize; ++outer) {
        const uint8_t* inputBeg = inputData + outer * axisSize * innerSize;
        const uint8_t* inputEnd = inputBeg + axisSize * innerSize;
        uint8_t* outputBeg = outputData + outer * axisSize * innerSize;
        for (uint32_t inner = 0; inner < innerSize; ++inner, ++inputBeg, ++inputEnd, ++outputBeg) {
            int32_t sum = 0;
            for (const uint8_t* p = inputBeg; p < inputEnd; p += innerSize) {
                int32_t val = static_cast<int32_t>(*p) - inputShape.offset;
                sum += val * val;
            }
            int32_t invMultiplier, invShift;
            tflite::GetInvSqrtQuantizedMultiplierExp(sum, -1, &invMultiplier, &invShift);
            uint8_t* pOut = outputBeg;
            for (const uint8_t* p = inputBeg; p < inputEnd; p += innerSize, pOut += innerSize) {
                int32_t val = static_cast<int32_t>(*p) - inputShape.offset;
                int32_t scaledVal = tflite::MultiplyByQuantizedMultiplierSmallerThanOneExp(
                                            val * 128, invMultiplier, invShift) +
                                    128;
                *pOut = static_cast<uint8_t>(std::min(std::max(scaledVal, 0), 255));
            }
        }
    }
    return true;
}

bool l2normFloat32(const float* inputData, const Shape& inputShape, int32_t axis, float* outputData,
                   const Shape& outputShape) {
    int32_t ndim = getNumberOfDimensions(inputShape);
    NN_CHECK(handleNegativeAxis(inputShape, &axis));
    // TFLite optimized implementation only supports computation along the last axis
    if (axis == ndim - 1) {
        NNTRACE_COMP("optimized_ops::L2Normalization::float");
        tflite::L2NormalizationParams param = {.input_zero_point = 0};
        tflite::optimized_ops::L2Normalization(param, convertShapeToTflshape(inputShape), inputData,
                                               convertShapeToTflshape(outputShape), outputData);
        return true;
    } else {
        return l2normFloat32Impl(inputData, inputShape, axis, outputData, outputShape);
    }
}

bool l2normFloat16(const _Float16* inputData, const Shape& inputShape, int32_t axis,
                   _Float16* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("l2normFloat16");
    std::vector<float> inputDataFloat32(getNumberOfElements(inputShape));
    convertFloat16ToFloat32(inputData, &inputDataFloat32);
    std::vector<float> outputDataFloat32(getNumberOfElements(outputShape));

    l2normFloat32(inputDataFloat32.data(), inputShape, axis, outputDataFloat32.data(), outputShape);
    convertFloat32ToFloat16(outputDataFloat32, outputData);

    return true;
}

bool l2normQuant8(const uint8_t* inputData, const Shape& inputShape, int32_t axis,
                  uint8_t* outputData, const Shape& outputShape) {
    int32_t ndim = getNumberOfDimensions(inputShape);
    NN_CHECK(handleNegativeAxis(inputShape, &axis));
    // TFLite optimized implementation only supports computation along the last axis
    if (axis == ndim - 1) {
        NNTRACE_COMP("optimized_ops::L2Normalization::uint8");
        tflite::L2NormalizationParams param = {.input_zero_point = inputShape.offset};
        tflite::optimized_ops::L2Normalization(param, convertShapeToTflshape(inputShape), inputData,
                                               convertShapeToTflshape(outputShape), outputData);
        return true;
    } else {
        return l2normQuant8Impl(inputData, inputShape, axis, outputData, outputShape);
    }
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK(context->getNumInputs() == kNumInputs ||
                 context->getNumInputs() == kNumInputs - 1);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);

    const OperandType inputType = context->getInputType(kInputTensor);
    std::vector<OperandType> inExpectedTypes = {inputType};
    if (inputType == OperandType::TENSOR_FLOAT16 || inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
    } else if (inputType == OperandType::TENSOR_FLOAT32) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
    if (context->getNumInputs() == kNumInputs) {
        inExpectedTypes.push_back(OperandType::INT32);
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
    } else if (context->getInputShape(kInputTensor).dimensions.size() != 4) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
    }
    return validateInputTypes(context, inExpectedTypes) &&
           validateOutputTypes(context, {inputType});
}

bool prepare(IOperationExecutionContext* context) {
    const Shape& input = context->getInputShape(kInputTensor);
    int32_t numDimensions = getNumberOfDimensions(input);
    int32_t axis = context->getNumInputs() == kNumInputs
                           ? context->getInputValue<int32_t>(kAxisScalar)
                           : -1;
    NN_RET_CHECK_GE(axis, -numDimensions);
    NN_RET_CHECK_LT(axis, numDimensions);
    Shape output = context->getOutputShape(kOutputTensor);
    output.type = input.type;
    output.dimensions = input.dimensions;
    if (output.type == OperandType::TENSOR_QUANT8_ASYMM) {
        output.scale = 1.0f / 128.0f;
        output.offset = 128;
    } else {
        output.scale = 0;
        output.offset = 0;
    }
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    int32_t axis = context->getNumInputs() == kNumInputs
                           ? context->getInputValue<int32_t>(kAxisScalar)
                           : -1;
    NN_RET_CHECK(handleNegativeAxis(context->getInputShape(kInputTensor), &axis));
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT32:
            return l2normFloat32(context->getInputBuffer<float>(kInputTensor),
                                 context->getInputShape(kInputTensor), axis,
                                 context->getOutputBuffer<float>(kOutputTensor),
                                 context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT16:
            return l2normFloat16(context->getInputBuffer<_Float16>(kInputTensor),
                                 context->getInputShape(kInputTensor), axis,
                                 context->getOutputBuffer<_Float16>(kOutputTensor),
                                 context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return l2normQuant8(context->getInputBuffer<uint8_t>(kInputTensor),
                                context->getInputShape(kInputTensor), axis,
                                context->getOutputBuffer<uint8_t>(kOutputTensor),
                                context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace l2_norm

NN_REGISTER_OPERATION(L2_NORMALIZATION, l2_norm::kOperationName, l2_norm::validate,
                      l2_norm::prepare, l2_norm::execute);

}  // namespace nn
}  // namespace android
