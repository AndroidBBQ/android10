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

#define LOG_TAG "Operations"

#include "CpuOperationUtils.h"
#include "HalInterfaces.h"
#include "OperationResolver.h"
#include "Tracing.h"

#include "tensorflow/lite/kernels/internal/reference/reference_ops.h"

#include <functional>
#include <vector>

namespace android {
namespace nn {

namespace resize_image {

constexpr uint32_t kNumInputs = 4;
constexpr uint32_t kInputTensor = 0;
// The following two scalars represent output shape if INT32, scale if floating point.
constexpr uint32_t kOutputWidthParamScalar = 1;
constexpr uint32_t kOutputHeightParamScalar = 2;
constexpr uint32_t kLayoutScalar = 3;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

template <typename T>
bool resizeImageOpNhwc(OperationType opType, const T* inputData, const Shape& inputShape,
                       T* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("resizeImageOpNhwc");
    int32_t height = static_cast<int32_t>(getSizeOfDimension(outputShape, 1));
    int32_t width = static_cast<int32_t>(getSizeOfDimension(outputShape, 2));
    // We have to fake a tensor here, to satisfy tflite implementation.
    int32_t outDimData[2] = {height, width};
    Shape outDimShape;
    outDimShape.dimensions = {2};

    if (opType == OperationType::RESIZE_BILINEAR) {
        NNTRACE_COMP_SWITCH("optimized_ops::ResizeBilinear");
        tflite::reference_ops::ResizeBilinear({.align_corners = false},
                                              convertShapeToTflshape(inputShape), inputData,
                                              convertShapeToTflshape(outDimShape), outDimData,
                                              convertShapeToTflshape(outputShape), outputData);
    } else if (opType == OperationType::RESIZE_NEAREST_NEIGHBOR) {
        // Align corners = true is not supported.
        NNTRACE_COMP_SWITCH("optimized_ops::ResizeNearestNeighbor");
        tflite::reference_ops::ResizeNearestNeighbor(
                {.align_corners = false}, convertShapeToTflshape(inputShape), inputData,
                convertShapeToTflshape(outDimShape), outDimData,
                convertShapeToTflshape(outputShape), outputData);
    }
    return true;
}

template <>
bool resizeImageOpNhwc<_Float16>(OperationType opType, const _Float16* inputData,
                                 const Shape& inputShape, _Float16* outputData,
                                 const Shape& outputShape) {
    NNTRACE_TRANS("resizeImageOpNhwcFloat16");
    std::vector<float> inputData_float32(getNumberOfElements(inputShape));
    convertFloat16ToFloat32(inputData, &inputData_float32);
    std::vector<float> outputData_float32(getNumberOfElements(outputShape));
    NN_RET_CHECK(resizeImageOpNhwc(opType, inputData_float32.data(), inputShape,
                                   outputData_float32.data(), outputShape));
    convertFloat32ToFloat16(outputData_float32, outputData);
    return true;
}

template <typename T>
bool resizeImageOp(OperationType opType, const T* inputData, const Shape& inputShape, bool useNchw,
                   T* outputData, const Shape& outputShape) {
    InputWithLayout<T> input(useNchw);
    OutputWithLayout<T> output(useNchw);
    NN_RET_CHECK(input.initialize(inputData, inputShape));
    NN_RET_CHECK(output.initialize(outputData, outputShape));
    NN_RET_CHECK(resizeImageOpNhwc(opType, input.getNhwcBuffer(), input.getNhwcShape(),
                                   output.getNhwcBuffer(), output.getNhwcShape()));
    NN_RET_CHECK(output.commit());
    return true;
}

}  // namespace

bool validate(OperationType opType, const IOperationValidationContext* context) {
    if (opType == OperationType::RESIZE_BILINEAR) {
        NN_RET_CHECK(context->getNumInputs() == kNumInputs ||
                     context->getNumInputs() == kNumInputs - 1);
    } else if (opType == OperationType::RESIZE_NEAREST_NEIGHBOR) {
        NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported operation " << getOperationName(opType);
    }
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    auto inputType = context->getInputType(kInputTensor);
    auto scalarType = context->getInputType(kOutputHeightParamScalar);
    std::vector<OperandType> inExpectedTypes = {inputType, scalarType, scalarType};
    NN_RET_CHECK(inputType == OperandType::TENSOR_FLOAT16 ||
                 inputType == OperandType::TENSOR_FLOAT32 ||
                 inputType == OperandType::TENSOR_QUANT8_ASYMM)
            << "Unsupported tensor type for operation " << getOperationName(opType);
    if (inputType == OperandType::TENSOR_FLOAT16 || inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
    }
    if (scalarType != OperandType::INT32) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
        if (inputType == OperandType::TENSOR_FLOAT32) {
            NN_RET_CHECK(scalarType == OperandType::FLOAT32);
        } else if (inputType == OperandType::TENSOR_FLOAT16) {
            NN_RET_CHECK(scalarType == OperandType::FLOAT16);
        } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
            NN_RET_CHECK(scalarType == OperandType::FLOAT32);
        }
    }
    if (context->getNumInputs() == kNumInputs) {
        inExpectedTypes.push_back(OperandType::BOOL);
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
    } else {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
    }
    return validateInputTypes(context, inExpectedTypes) &&
           validateOutputTypes(context, {inputType});
}

bool prepare(OperationType opType, IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    NN_RET_CHECK_EQ(getNumberOfDimensions(input), 4);
    bool useNchw = false;
    if (context->getNumInputs() > kLayoutScalar) {
        useNchw = context->getInputValue<bool>(kLayoutScalar);
    }

    // Only batches can be zero.
    uint32_t batches = getSizeOfDimension(input, 0);
    uint32_t inHeight = getSizeOfDimension(input, useNchw ? 2 : 1);
    uint32_t inWidth = getSizeOfDimension(input, useNchw ? 3 : 2);
    uint32_t channels = getSizeOfDimension(input, useNchw ? 1 : 3);
    NN_RET_CHECK_GT(inHeight, 0);
    NN_RET_CHECK_GT(inWidth, 0);
    NN_RET_CHECK_GT(channels, 0);

    int32_t height, width;
    auto scalarType = context->getInputType(kOutputHeightParamScalar);
    if (scalarType == OperandType::INT32) {
        height = context->getInputValue<int32_t>(kOutputHeightParamScalar);
        width = context->getInputValue<int32_t>(kOutputWidthParamScalar);
    } else if (scalarType == OperandType::FLOAT32) {
        height = std::floor(static_cast<float>(inHeight) *
                            context->getInputValue<float>(kOutputHeightParamScalar));
        width = std::floor(static_cast<float>(inWidth) *
                           context->getInputValue<float>(kOutputWidthParamScalar));
    } else if (scalarType == OperandType::FLOAT16) {
        height = std::floor(
                static_cast<float>(inHeight) *
                static_cast<float>(context->getInputValue<_Float16>(kOutputHeightParamScalar)));
        width = std::floor(
                static_cast<float>(inWidth) *
                static_cast<float>(context->getInputValue<_Float16>(kOutputWidthParamScalar)));
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported scalar type for operation " << getOperationName(opType);
    }
    NN_RET_CHECK_GT(height, 0);
    NN_RET_CHECK_GT(width, 0);

    Shape output = input;
    if (useNchw) {
        output.dimensions = {batches, channels, (uint32_t)height, (uint32_t)width};
    } else {
        output.dimensions = {batches, (uint32_t)height, (uint32_t)width, channels};
    }
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(OperationType opType, IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    bool useNchw = false;
    if (context->getNumInputs() > kLayoutScalar) {
        useNchw = context->getInputValue<bool>(kLayoutScalar);
    }
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return resizeImageOp(opType, context->getInputBuffer<_Float16>(kInputTensor),
                                 context->getInputShape(kInputTensor), useNchw,
                                 context->getOutputBuffer<_Float16>(kOutputTensor),
                                 context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return resizeImageOp(opType, context->getInputBuffer<float>(kInputTensor),
                                 context->getInputShape(kInputTensor), useNchw,
                                 context->getOutputBuffer<float>(kOutputTensor),
                                 context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return resizeImageOp(opType, context->getInputBuffer<uint8_t>(kInputTensor),
                                 context->getInputShape(kInputTensor), useNchw,
                                 context->getOutputBuffer<uint8_t>(kOutputTensor),
                                 context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation "
                                << getOperationName(opType);
    }
}

}  // namespace resize_image

using std::placeholders::_1;

NN_REGISTER_OPERATION(RESIZE_BILINEAR, "RESIZE_BILINEAR",
                      std::bind(resize_image::validate, OperationType::RESIZE_BILINEAR, _1),
                      std::bind(resize_image::prepare, OperationType::RESIZE_BILINEAR, _1),
                      std::bind(resize_image::execute, OperationType::RESIZE_BILINEAR, _1),
                      .allowZeroSizedInput = true);

NN_REGISTER_OPERATION(RESIZE_NEAREST_NEIGHBOR, "RESIZE_NEAREST_NEIGHBOR",
                      std::bind(resize_image::validate, OperationType::RESIZE_NEAREST_NEIGHBOR, _1),
                      std::bind(resize_image::prepare, OperationType::RESIZE_NEAREST_NEIGHBOR, _1),
                      std::bind(resize_image::execute, OperationType::RESIZE_NEAREST_NEIGHBOR, _1),
                      .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
