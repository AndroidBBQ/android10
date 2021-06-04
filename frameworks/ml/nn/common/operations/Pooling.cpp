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

#include "tensorflow/lite/kernels/internal/optimized/optimized_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {
namespace pooling {

constexpr uint32_t kInputTensor = 0;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

struct PoolingParam {
    int32_t padding_left, padding_right;
    int32_t padding_top, padding_bottom;
    int32_t stride_width, stride_height;
    int32_t filter_width, filter_height;
    int32_t activation;
    bool useNchw = false;

    bool initialize(const IOperationExecutionContext* context) {
        uint32_t inCount = context->getNumInputs();
        int32_t padding_implicit = 0;
        if (inCount >= 10) {
            padding_left = context->getInputValue<int32_t>(1);
            padding_right = context->getInputValue<int32_t>(2);
            padding_top = context->getInputValue<int32_t>(3);
            padding_bottom = context->getInputValue<int32_t>(4);
            stride_width = context->getInputValue<int32_t>(5);
            stride_height = context->getInputValue<int32_t>(6);
            filter_width = context->getInputValue<int32_t>(7);
            filter_height = context->getInputValue<int32_t>(8);
            activation = context->getInputValue<int32_t>(9);
            if (inCount == 11) {
                useNchw = context->getInputValue<bool>(10);
            }
        } else {
            padding_implicit = context->getInputValue<int32_t>(1);
            stride_width = context->getInputValue<int32_t>(2);
            stride_height = context->getInputValue<int32_t>(3);
            filter_width = context->getInputValue<int32_t>(4);
            filter_height = context->getInputValue<int32_t>(5);
            activation = context->getInputValue<int32_t>(6);
            if (inCount == 8) {
                useNchw = context->getInputValue<bool>(7);
            }
        }
        if (inCount <= 8) {
            Shape inputShape = context->getInputShape(kInputTensor);
            int32_t input_height = getSizeOfDimension(inputShape, useNchw ? 2 : 1);
            int32_t input_width = getSizeOfDimension(inputShape, useNchw ? 3 : 2);
            calculateExplicitPadding(input_width, stride_width, filter_width, padding_implicit,
                                     &padding_left, &padding_right);
            calculateExplicitPadding(input_height, stride_height, filter_height, padding_implicit,
                                     &padding_top, &padding_bottom);
        }
        NN_RET_CHECK_GE(padding_left, 0);
        NN_RET_CHECK_GE(padding_right, 0);
        NN_RET_CHECK_GE(padding_top, 0);
        NN_RET_CHECK_GE(padding_bottom, 0);
        NN_RET_CHECK_GT(stride_width, 0);
        NN_RET_CHECK_GT(stride_height, 0);
        NN_RET_CHECK_GT(filter_width, 0);
        NN_RET_CHECK_GT(filter_height, 0);
        NN_RET_CHECK_GE(activation, 0);
        NN_RET_CHECK_GT(filter_width, padding_left);
        NN_RET_CHECK_GT(filter_width, padding_right);
        NN_RET_CHECK_GT(filter_height, padding_top);
        NN_RET_CHECK_GT(filter_height, padding_bottom);
        return true;
    }

    tflite::PoolParams toTfliteParam(const Shape& output) const {
        tflite::PoolParams params = {
                .stride_height = stride_height,
                .stride_width = stride_width,
                .filter_height = filter_height,
                .filter_width = filter_width,
                .padding_values = {.height = static_cast<int16_t>(padding_top),
                                   .width = static_cast<int16_t>(padding_left)}};
        if (output.type == OperandType::TENSOR_QUANT8_ASYMM) {
            int32_t output_activation_min = 0;
            int32_t output_activation_max = 0;
            CalculateActivationRangeUint8(activation, output, &output_activation_min,
                                          &output_activation_max);
            params.quantized_activation_min = output_activation_min;
            params.quantized_activation_max = output_activation_max;
        } else {
            float output_activation_min, output_activation_max;
            CalculateActivationRangeFloat(activation, &output_activation_min,
                                          &output_activation_max);
            params.float_activation_min = output_activation_min;
            params.float_activation_max = output_activation_max;
        }
        return params;
    }
};

bool averagePoolNhwc(const float* inputData, const Shape& inputShape, const PoolingParam& param,
                     float* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("averagePoolFloat32");
    auto op_params = param.toTfliteParam(outputShape);
    NNTRACE_COMP_SWITCH("optimized_ops::AveragePool");
    tflite::optimized_ops::AveragePool(op_params, convertShapeToTflshape(inputShape), inputData,
                                       convertShapeToTflshape(outputShape), outputData);
    return true;
}

bool averagePoolNhwc(const _Float16* inputData, const Shape& inputShape, const PoolingParam& param,
                     _Float16* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("averagePoolFloat16");
    std::vector<float> inputDataFloat32(getNumberOfElements(inputShape));
    std::vector<float> outputDataFloat32(getNumberOfElements(outputShape));

    convertFloat16ToFloat32(inputData, &inputDataFloat32);
    averagePoolNhwc(inputDataFloat32.data(), inputShape, param, outputDataFloat32.data(),
                    outputShape);
    convertFloat32ToFloat16(outputDataFloat32, outputData);
    return true;
}

bool averagePoolNhwc(const uint8_t* inputData, const Shape& inputShape, const PoolingParam& param,
                     uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("averagePoolQuant8");
    auto op_params = param.toTfliteParam(outputShape);
    NNTRACE_COMP_SWITCH("optimized_ops::AveragePool");
    tflite::optimized_ops::AveragePool(op_params, convertShapeToTflshape(inputShape), inputData,
                                       convertShapeToTflshape(outputShape), outputData);
    return true;
}

bool l2PoolNhwc(const float* inputData, const Shape& inputShape, const PoolingParam& param,
                float* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("l2PoolFloat32");
    auto op_params = param.toTfliteParam(outputShape);
    NNTRACE_COMP_SWITCH("optimized_ops::L2Pool");
    tflite::optimized_ops::L2Pool(op_params, convertShapeToTflshape(inputShape), inputData,
                                  convertShapeToTflshape(outputShape), outputData);
    return true;
}

bool l2PoolNhwc(const _Float16* inputData, const Shape& inputShape, const PoolingParam& param,
                _Float16* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("l2PoolFloat16");
    std::vector<float> inputDataFloat32(getNumberOfElements(inputShape));
    std::vector<float> outputDataFloat32(getNumberOfElements(outputShape));

    convertFloat16ToFloat32(inputData, &inputDataFloat32);
    l2PoolNhwc(inputDataFloat32.data(), inputShape, param, outputDataFloat32.data(), outputShape);
    convertFloat32ToFloat16(outputDataFloat32, outputData);
    return true;
}

bool maxPoolNhwc(const float* inputData, const Shape& inputShape, const PoolingParam& param,
                 float* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("maxPoolFloat32");
    auto op_params = param.toTfliteParam(outputShape);
    NNTRACE_COMP_SWITCH("optimized_ops::MaxPool");
    tflite::optimized_ops::MaxPool(op_params, convertShapeToTflshape(inputShape), inputData,
                                   convertShapeToTflshape(outputShape), outputData);
    return true;
}

bool maxPoolNhwc(const uint8_t* inputData, const Shape& inputShape, const PoolingParam& param,
                 uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("maxPoolQuant8");
    auto op_params = param.toTfliteParam(outputShape);
    NNTRACE_COMP_SWITCH("optimized_ops::MaxPool");
    tflite::optimized_ops::MaxPool(op_params, convertShapeToTflshape(inputShape), inputData,
                                   convertShapeToTflshape(outputShape), outputData);
    return true;
}

bool maxPoolNhwc(const _Float16* inputData, const Shape& inputShape, const PoolingParam& param,
                 _Float16* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("maxPoolFloat16");
    std::vector<float> inputData_float32(getNumberOfElements(inputShape));
    std::vector<float> outputData_float32(getNumberOfElements(outputShape));

    convertFloat16ToFloat32(inputData, &inputData_float32);
    maxPoolNhwc(inputData_float32.data(), inputShape, param, outputData_float32.data(),
                outputShape);
    convertFloat32ToFloat16(outputData_float32, outputData);
    return true;
}

template <typename T>
bool averagePool(const T* inputData, const Shape& inputShape, const PoolingParam& param,
                 T* outputData, const Shape& outputShape) {
    InputWithLayout<T> input(param.useNchw);
    OutputWithLayout<T> output(param.useNchw);
    NN_RET_CHECK(input.initialize(inputData, inputShape));
    NN_RET_CHECK(output.initialize(outputData, outputShape));
    NN_RET_CHECK(averagePoolNhwc(input.getNhwcBuffer(), input.getNhwcShape(), param,
                                 output.getNhwcBuffer(), output.getNhwcShape()));
    NN_RET_CHECK(output.commit());
    return true;
}

template <typename T>
bool l2Pool(const T* inputData, const Shape& inputShape, const PoolingParam& param, T* outputData,
            const Shape& outputShape) {
    InputWithLayout<T> input(param.useNchw);
    OutputWithLayout<T> output(param.useNchw);
    NN_RET_CHECK(input.initialize(inputData, inputShape));
    NN_RET_CHECK(output.initialize(outputData, outputShape));
    NN_RET_CHECK(l2PoolNhwc(input.getNhwcBuffer(), input.getNhwcShape(), param,
                            output.getNhwcBuffer(), output.getNhwcShape()));
    NN_RET_CHECK(output.commit());
    return true;
}

template <typename T>
bool maxPool(const T* inputData, const Shape& inputShape, const PoolingParam& param, T* outputData,
             const Shape& outputShape) {
    InputWithLayout<T> input(param.useNchw);
    OutputWithLayout<T> output(param.useNchw);
    NN_RET_CHECK(input.initialize(inputData, inputShape));
    NN_RET_CHECK(output.initialize(outputData, outputShape));
    NN_RET_CHECK(maxPoolNhwc(input.getNhwcBuffer(), input.getNhwcShape(), param,
                             output.getNhwcBuffer(), output.getNhwcShape()));
    NN_RET_CHECK(output.commit());
    return true;
}

}  // namespace

bool validate(OperationType opType, const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    auto inputCount = context->getNumInputs();
    NN_RET_CHECK(inputCount == 11 || inputCount == 10 || inputCount == 8 || inputCount == 7);
    auto inputType = context->getInputType(kInputTensor);
    std::vector<OperandType> inExpectedTypes;
    if (inputType == OperandType::TENSOR_FLOAT32) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
        inExpectedTypes = {
                inputType,          OperandType::INT32, OperandType::INT32, OperandType::INT32,
                OperandType::INT32, OperandType::INT32, OperandType::INT32,
        };
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
        inExpectedTypes = {
                OperandType::TENSOR_FLOAT16, OperandType::INT32, OperandType::INT32,
                OperandType::INT32,          OperandType::INT32, OperandType::INT32,
                OperandType::INT32,
        };
    } else if (opType != OperationType::L2_POOL_2D &&
               inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
        inExpectedTypes = {
                OperandType::TENSOR_QUANT8_ASYMM,
                OperandType::INT32,
                OperandType::INT32,
                OperandType::INT32,
                OperandType::INT32,
                OperandType::INT32,
                OperandType::INT32,
        };
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported input tensor type for operation "
                            << getOperationName(opType);
    }

    if (inputCount >= 10) {
        std::vector<OperandType> explicitScalarTypes(3, OperandType::INT32);
        inExpectedTypes.insert(inExpectedTypes.end(), explicitScalarTypes.begin(),
                               explicitScalarTypes.end());
    }
    if (inputCount == 11 || inputCount == 8) {
        inExpectedTypes.push_back(OperandType::BOOL);
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
    } else {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
    }
    return validateInputTypes(context, inExpectedTypes) &&
           validateOutputTypes(context, {inputType});
}

bool prepare(IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    NN_RET_CHECK_EQ(getNumberOfDimensions(input), 4);

    PoolingParam param;
    NN_RET_CHECK(param.initialize(context));

    // Only batches can be zero.
    uint32_t batches = getSizeOfDimension(input, 0);
    uint32_t height = getSizeOfDimension(input, param.useNchw ? 2 : 1);
    uint32_t width = getSizeOfDimension(input, param.useNchw ? 3 : 2);
    uint32_t channels = getSizeOfDimension(input, param.useNchw ? 1 : 3);
    NN_RET_CHECK_GT(height, 0);
    NN_RET_CHECK_GT(width, 0);
    NN_RET_CHECK_GT(channels, 0);

    uint32_t outWidth = computeOutSize(width, param.filter_width, param.stride_width,
                                       param.padding_left, param.padding_right);
    uint32_t outHeight = computeOutSize(height, param.filter_height, param.stride_height,
                                        param.padding_top, param.padding_bottom);

    Shape output = input;
    if (param.useNchw) {
        output.dimensions = {batches, channels, outHeight, outWidth};
    } else {
        output.dimensions = {batches, outHeight, outWidth, channels};
    }
    return context->setOutputShape(kOutputTensor, output);
}

#define POOLING_DISPATCH_INPUT_TYPE(name, type, cppType)              \
    case OperandType::type:                                           \
        return name(context->getInputBuffer<cppType>(kInputTensor),   \
                    context->getInputShape(kInputTensor), param,      \
                    context->getOutputBuffer<cppType>(kOutputTensor), \
                    context->getOutputShape(kOutputTensor))

bool executeAveragePool(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    PoolingParam param;
    NN_RET_CHECK(param.initialize(context));
    switch (context->getInputType(kInputTensor)) {
        POOLING_DISPATCH_INPUT_TYPE(averagePool, TENSOR_FLOAT32, float);
        POOLING_DISPATCH_INPUT_TYPE(averagePool, TENSOR_FLOAT16, _Float16);
        POOLING_DISPATCH_INPUT_TYPE(averagePool, TENSOR_QUANT8_ASYMM, uint8_t);
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation AVERAGE_POOL_2D";
    }
}

bool executeL2Pool(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    PoolingParam param;
    NN_RET_CHECK(param.initialize(context));
    switch (context->getInputType(kInputTensor)) {
        POOLING_DISPATCH_INPUT_TYPE(l2Pool, TENSOR_FLOAT32, float);
        POOLING_DISPATCH_INPUT_TYPE(l2Pool, TENSOR_FLOAT16, _Float16);
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation L2_POOL_2D";
    }
}

bool executeMaxPool(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    PoolingParam param;
    NN_RET_CHECK(param.initialize(context));
    switch (context->getInputType(kInputTensor)) {
        POOLING_DISPATCH_INPUT_TYPE(maxPool, TENSOR_FLOAT32, float);
        POOLING_DISPATCH_INPUT_TYPE(maxPool, TENSOR_FLOAT16, _Float16);
        POOLING_DISPATCH_INPUT_TYPE(maxPool, TENSOR_QUANT8_ASYMM, uint8_t);
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation MAX_POOL_2D";
    }
}

#undef POOLING_DISPATCH_INPUT_TYPE

}  // namespace pooling

using std::placeholders::_1;
NN_REGISTER_OPERATION(AVERAGE_POOL_2D, "AVERAGE_POOL_2D",
                      std::bind(pooling::validate, OperationType::AVERAGE_POOL_2D, _1),
                      pooling::prepare, pooling::executeAveragePool, .allowZeroSizedInput = true);
NN_REGISTER_OPERATION(L2_POOL_2D, "L2_POOL_2D",
                      std::bind(pooling::validate, OperationType::L2_POOL_2D, _1), pooling::prepare,
                      pooling::executeL2Pool, .allowZeroSizedInput = true);
NN_REGISTER_OPERATION(MAX_POOL_2D, "MAX_POOL_2D",
                      std::bind(pooling::validate, OperationType::MAX_POOL_2D, _1),
                      pooling::prepare, pooling::executeMaxPool, .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
