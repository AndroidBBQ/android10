/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <cfloat>
#include <cmath>

#include "Tracing.h"
#include "tensorflow/lite/kernels/internal/common.h"

namespace android {
namespace nn {
namespace transpose_conv_2d {

constexpr char kOperationName[] = "TRANSPOSE_CONV_2D";

constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kFilterTensor = 1;
constexpr uint32_t kBiasTensor = 2;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

// If possible we will use this static buffer for the tensor.
constexpr size_t kStaticBufferSize = 1605632;
char static_scratch_buffer[kStaticBufferSize];

// executionMutex is used to protect concurrent access of the static_scratch_buffer.
// std::mutex is safe for pthreads on Android.
std::mutex executionMutex;

struct TransposeConv2dParam {
    int32_t paddingLeft, paddingRight;
    int32_t paddingTop, paddingBottom;
    int32_t strideWidth, strideHeight;
    int32_t activation;
    bool useNchw = false;

    bool initialize(const IOperationExecutionContext* context) {
        uint32_t inCount = context->getNumInputs();
        int32_t paddingImplicit = 0;
        if (inCount == 9) {
            paddingImplicit = context->getInputValue<int32_t>(4);
            strideWidth = context->getInputValue<int32_t>(5);
            strideHeight = context->getInputValue<int32_t>(6);
            activation = context->getInputValue<int32_t>(7);
            useNchw = context->getInputValue<bool>(8);
            Shape filterShape = context->getInputShape(kFilterTensor);
            int32_t filterWidth = getSizeOfDimension(filterShape, 2);
            int32_t filterHeight = getSizeOfDimension(filterShape, 1);
            NN_RET_CHECK_EQ(getNumberOfDimensions(context->getInputShape(3)), 1);
            NN_RET_CHECK_EQ(getSizeOfDimension(context->getInputShape(3), 0), 4);
            const int32_t* outputShapeData = context->getInputBuffer<int32_t>(3);
            int32_t outputWidth = useNchw ? outputShapeData[3] : outputShapeData[2];
            int32_t outputHeight = useNchw ? outputShapeData[2] : outputShapeData[1];
            calculateExplicitPaddingTransposeConv(outputWidth, strideWidth, filterWidth,
                                                  paddingImplicit, &paddingLeft, &paddingRight);
            calculateExplicitPaddingTransposeConv(outputHeight, strideHeight, filterHeight,
                                                  paddingImplicit, &paddingTop, &paddingBottom);
        } else if (inCount == 11) {
            paddingLeft = context->getInputValue<int32_t>(3);
            paddingRight = context->getInputValue<int32_t>(4);
            paddingTop = context->getInputValue<int32_t>(5);
            paddingBottom = context->getInputValue<int32_t>(6);
            strideWidth = context->getInputValue<int32_t>(7);
            strideHeight = context->getInputValue<int32_t>(8);
            activation = context->getInputValue<int32_t>(9);
            useNchw = context->getInputValue<bool>(10);
        } else {
            NN_RET_CHECK_FAIL() << "Unsupported input spec for operation " << kOperationName;
        }
        // paddingRight and paddingBottom in transpose conv may be less than 0 to resolve the
        // ambiguous output shape issue in the case of stride > 1.
        NN_RET_CHECK_GE(paddingLeft, 0);
        NN_RET_CHECK_GE(paddingTop, 0);
        NN_RET_CHECK_GT(strideWidth, 0);
        NN_RET_CHECK_GT(strideHeight, 0);
        NN_RET_CHECK_GE(activation, 0);
        return true;
    }
};

#define ANDROID_NN_TRANSPOSE_CONV_PARAMETERS                                    \
    uint32_t numBatches = getSizeOfDimension(inputShape, 0);                    \
    uint32_t inputHeight = getSizeOfDimension(inputShape, 1);                   \
    uint32_t inputWidth = getSizeOfDimension(inputShape, 2);                    \
    uint32_t inputDepth = getSizeOfDimension(inputShape, 3);                    \
    uint32_t filterHeight = getSizeOfDimension(filterShape, 1);                 \
    uint32_t filterWidth = getSizeOfDimension(filterShape, 2);                  \
    uint32_t outputHeight = getSizeOfDimension(outputShape, 1);                 \
    uint32_t outputWidth = getSizeOfDimension(outputShape, 2);                  \
    uint32_t outputDepth = getSizeOfDimension(outputShape, 3);                  \
    int32_t paddingLeft = param.paddingLeft, paddingRight = param.paddingRight; \
    int32_t paddingTop = param.paddingTop, paddingBottom = param.paddingBottom; \
    int32_t strideWidth = param.strideWidth, strideHeight = param.strideHeight; \
    int32_t activation = param.activation;

bool transposeConvNhwc(const float* inputData, const Shape& inputShape, const float* filterData,
                       const Shape& filterShape, const float* biasData, const Shape& biasShape,
                       const TransposeConv2dParam& param, float* outputData,
                       const Shape& outputShape) {
    NNTRACE_TRANS("transposeConvFloat32");
    ANDROID_NN_TRANSPOSE_CONV_PARAMETERS

    float outputActivationMin = 0.0f, outputActivationMax = 0.0f;
    CalculateActivationRangeFloat(activation, &outputActivationMin, &outputActivationMax);

    memset(outputData, 0, getNumberOfElements(outputShape) * sizeof(float));

    const float* inputBase = inputData;
    float* outputBase = outputData;
    for (uint32_t b = 0; b < numBatches; b++) {
        for (uint32_t h = 0; h < inputHeight; h++) {
            for (uint32_t w = 0; w < inputWidth; w++) {
                int32_t wOutputOrigin = static_cast<int32_t>(w) * strideWidth - paddingLeft;
                int32_t hOutputOrigin = static_cast<int32_t>(h) * strideHeight - paddingTop;

                const float* filterBase = filterData;
                for (uint32_t k = 0; k < outputDepth; k++) {
                    for (uint32_t i = 0; i < filterHeight; i++) {
                        for (uint32_t j = 0; j < filterWidth; j++, filterBase += inputDepth) {
                            int32_t hOutput = hOutputOrigin + static_cast<int32_t>(i);
                            int32_t wOutput = wOutputOrigin + static_cast<int32_t>(j);
                            if (hOutput >= 0 && hOutput < static_cast<int32_t>(outputHeight) &&
                                wOutput >= 0 && wOutput < static_cast<int32_t>(outputWidth)) {
                                for (uint32_t d = 0; d < inputDepth; d++) {
                                    uint32_t outputIndex = hOutput * outputWidth * outputDepth +
                                                           wOutput * outputDepth + k;
                                    outputBase[outputIndex] += inputBase[d] * filterBase[d];
                                }
                            }
                        }
                    }
                }

                inputBase += inputDepth;
            }
        }
        outputBase += outputHeight * outputWidth * outputDepth;
    }

    const uint32_t outerSize = numBatches * outputHeight * outputWidth;
    float* outPtr = outputData;
    for (uint32_t i = 0; i < outerSize; i++) {
        for (uint32_t d = 0; d < outputDepth; d++, outPtr++) {
            *outPtr += biasData[d];
            *outPtr = std::max(std::min(*outPtr, outputActivationMax), outputActivationMin);
        }
    }

    return true;
}

bool transposeConvNhwc(const uint8_t* inputData, const Shape& inputShape, const uint8_t* filterData,
                       const Shape& filterShape, const int32_t* biasData, const Shape& biasShape,
                       const TransposeConv2dParam& param, uint8_t* outputData,
                       const Shape& outputShape) {
    NNTRACE_TRANS("transposeConvQuant8");
    ANDROID_NN_TRANSPOSE_CONV_PARAMETERS

    int32_t* tempBuffer = nullptr;
    std::unique_ptr<int32_t[]> bufferGuard;
    uint32_t tempBufferByteSize = getNumberOfElements(outputShape) * sizeof(int32_t);
    if (tempBufferByteSize <= kStaticBufferSize) {
        tempBuffer = reinterpret_cast<int32_t*>(static_scratch_buffer);
    } else {
        tempBuffer = new (std::nothrow) int32_t[tempBufferByteSize / sizeof(int32_t)];
        if (tempBuffer == nullptr) {
            LOG(ERROR) << "ConvTranspose size is too large, not enough memory";
            return false;
        }
        bufferGuard.reset(tempBuffer);
    }

    int32_t inputOffset = -inputShape.offset;
    int32_t filterOffset = -filterShape.offset;
    int32_t outputOffset = outputShape.offset;

    double realMultiplier = 0.0;
    int32_t outputMultiplier = 0;
    int32_t outputShift = 0;
    NN_RET_CHECK(GetQuantizedConvolutionMultipler(inputShape, filterShape, biasShape, outputShape,
                                                  &realMultiplier));
    int exponent;
    NN_RET_CHECK(QuantizeMultiplier(realMultiplier, &outputMultiplier, &exponent));
    outputShift = -exponent;

    int32_t outputActivationMin = 0, outputActivationMax = 0;
    CalculateActivationRangeUint8(activation, outputShape, &outputActivationMin,
                                  &outputActivationMax);

    // Prevent concurrent executions that may access the scratch buffer
    std::unique_lock<std::mutex> lock(executionMutex);
    memset(tempBuffer, 0, tempBufferByteSize);

    const uint8_t* inputPtr = inputData;
    int32_t* outputBase = tempBuffer;
    for (uint32_t b = 0; b < numBatches; b++) {
        for (uint32_t h = 0; h < inputHeight; h++) {
            for (uint32_t w = 0; w < inputWidth; w++) {
                for (uint32_t d = 0; d < inputDepth; d++) {
                    int32_t wOutputOrigin = static_cast<int32_t>(w) * strideWidth - paddingLeft;
                    int32_t hOutputOrigin = static_cast<int32_t>(h) * strideHeight - paddingTop;

                    for (uint32_t i = 0; i < filterHeight; i++) {
                        for (uint32_t j = 0; j < filterWidth; j++) {
                            for (uint32_t k = 0; k < outputDepth; k++) {
                                int32_t hOutput = hOutputOrigin + static_cast<int32_t>(i);
                                int32_t wOutput = wOutputOrigin + static_cast<int32_t>(j);
                                if (hOutput >= 0 && hOutput < static_cast<int32_t>(outputHeight) &&
                                    wOutput >= 0 && wOutput < static_cast<int32_t>(outputWidth)) {
                                    uint32_t filterIndex =
                                            k * filterHeight * filterWidth * inputDepth +
                                            i * filterWidth * inputDepth + j * inputDepth + d;
                                    uint32_t outputIndex = hOutput * outputWidth * outputDepth +
                                                           wOutput * outputDepth + k;
                                    outputBase[outputIndex] +=
                                            (static_cast<int32_t>(*inputPtr) + inputOffset) *
                                            (static_cast<int32_t>(filterData[filterIndex]) +
                                             filterOffset);
                                }
                            }
                        }
                    }

                    inputPtr++;
                }
            }
        }
        outputBase += outputHeight * outputWidth * outputDepth;
    }

    const uint32_t outerSize = numBatches * outputHeight * outputWidth;
    int32_t* bufferPtr = tempBuffer;
    uint8_t* outPtr = outputData;
    for (uint32_t i = 0; i < outerSize; i++) {
        for (uint32_t d = 0; d < outputDepth; d++, bufferPtr++, outPtr++) {
            int32_t outVal = *bufferPtr + biasData[d];
            outVal = tflite::MultiplyByQuantizedMultiplier(outVal, outputMultiplier, -outputShift);
            outVal += outputOffset;
            outVal = std::max(std::min(outVal, outputActivationMax), outputActivationMin);
            *outPtr = static_cast<uint8_t>(outVal);
        }
    }

    return true;
}

bool transposeConvNhwc(const _Float16* inputData, const Shape& inputShape,
                       const _Float16* filterData, const Shape& filterShape,
                       const _Float16* biasData, const Shape& biasShape,
                       const TransposeConv2dParam& param, _Float16* outputData,
                       const Shape& outputShape) {
    NNTRACE_TRANS("transposeConvFloat16");
    std::vector<float> inputData_float32(getNumberOfElements(inputShape));
    std::vector<float> filterData_float32(getNumberOfElements(filterShape));
    std::vector<float> biasData_float32(getNumberOfElements(biasShape));
    std::vector<float> outputData_float32(getNumberOfElements(outputShape));

    convertFloat16ToFloat32(inputData, &inputData_float32);
    convertFloat16ToFloat32(filterData, &filterData_float32);
    convertFloat16ToFloat32(biasData, &biasData_float32);

    transposeConvNhwc(inputData_float32.data(), inputShape, filterData_float32.data(), filterShape,
                      biasData_float32.data(), biasShape, param, outputData_float32.data(),
                      outputShape);
    convertFloat32ToFloat16(outputData_float32, outputData);

    return true;
}

template <typename T_Input, typename T_Filter, typename T_Bias>
bool transposeConv(const T_Input* inputData, const Shape& inputShape, const T_Filter* filterData,
                   const Shape& filterShape, const T_Bias* biasData, const Shape& biasShape,
                   const TransposeConv2dParam& param, T_Input* outputData,
                   const Shape& outputShape) {
    InputWithLayout<T_Input> input(param.useNchw);
    OutputWithLayout<T_Input> output(param.useNchw);
    NN_RET_CHECK(input.initialize(inputData, inputShape));
    NN_RET_CHECK(output.initialize(outputData, outputShape));
    NN_RET_CHECK(transposeConvNhwc(input.getNhwcBuffer(), input.getNhwcShape(), filterData,
                                   filterShape, biasData, biasShape, param, output.getNhwcBuffer(),
                                   output.getNhwcShape()));
    NN_RET_CHECK(output.commit());
    return true;
}

bool transposeConvQuant8PerChannelNhwc(const uint8_t* inputData, const Shape& inputShape,
                                       const int8_t* filterData, const Shape& filterShape,
                                       const float* filterScales, const int32_t* biasData,
                                       const Shape& biasShape, const TransposeConv2dParam& param,
                                       uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("transposeConvQuant8PerChannel");
    ANDROID_NN_TRANSPOSE_CONV_PARAMETERS

    int32_t* tempBuffer = nullptr;
    std::unique_ptr<int32_t[]> bufferGuard;
    uint32_t tempBufferByteSize = getNumberOfElements(outputShape) * sizeof(int32_t);
    if (tempBufferByteSize <= kStaticBufferSize) {
        tempBuffer = reinterpret_cast<int32_t*>(static_scratch_buffer);
    } else {
        tempBuffer = new (std::nothrow) int32_t[tempBufferByteSize / sizeof(int32_t)];
        if (tempBuffer == nullptr) {
            LOG(ERROR) << "ConvTranspose size is too large, not enough memory";
            return false;
        }
        bufferGuard.reset(tempBuffer);
    }

    int32_t inputOffset = -inputShape.offset;
    int32_t outputOffset = outputShape.offset;

    std::vector<double> realMultiplier(outputDepth, 0.0);
    std::vector<int32_t> outputMultiplier(outputDepth, 0);
    std::vector<int32_t> outputShift(outputDepth, 0);
    for (int i = 0; i < outputDepth; ++i) {
        Shape filterChannelShape = filterShape;
        filterChannelShape.scale = filterScales[i];
        Shape biasChannelShape = biasShape;
        biasChannelShape.scale = filterScales[i] * inputShape.scale;

        NN_RET_CHECK(GetQuantizedConvolutionMultipler(
                inputShape, filterChannelShape, biasChannelShape, outputShape, &realMultiplier[i]));
        int exponent;
        NN_RET_CHECK(QuantizeMultiplier(realMultiplier[i], &outputMultiplier[i], &exponent));
        outputShift[i] = -exponent;
    }

    int32_t outputActivationMin = 0, outputActivationMax = 0;
    CalculateActivationRangeUint8(activation, outputShape, &outputActivationMin,
                                  &outputActivationMax);

    // Prevent concurrent executions that may access the scratch buffer
    std::unique_lock<std::mutex> lock(executionMutex);
    memset(tempBuffer, 0, tempBufferByteSize);

    const uint8_t* inputPtr = inputData;
    int32_t* outputBase = tempBuffer;
    for (uint32_t b = 0; b < numBatches; b++) {
        for (uint32_t h = 0; h < inputHeight; h++) {
            for (uint32_t w = 0; w < inputWidth; w++) {
                for (uint32_t d = 0; d < inputDepth; d++) {
                    int32_t wOutputOrigin = static_cast<int32_t>(w) * strideWidth - paddingLeft;
                    int32_t hOutputOrigin = static_cast<int32_t>(h) * strideHeight - paddingTop;

                    for (uint32_t i = 0; i < filterHeight; i++) {
                        for (uint32_t j = 0; j < filterWidth; j++) {
                            for (uint32_t k = 0; k < outputDepth; k++) {
                                int32_t hOutput = hOutputOrigin + static_cast<int32_t>(i);
                                int32_t wOutput = wOutputOrigin + static_cast<int32_t>(j);
                                if (hOutput >= 0 && hOutput < static_cast<int32_t>(outputHeight) &&
                                    wOutput >= 0 && wOutput < static_cast<int32_t>(outputWidth)) {
                                    uint32_t filterIndex =
                                            k * filterHeight * filterWidth * inputDepth +
                                            i * filterWidth * inputDepth + j * inputDepth + d;
                                    uint32_t outputIndex = hOutput * outputWidth * outputDepth +
                                                           wOutput * outputDepth + k;
                                    outputBase[outputIndex] +=
                                            (static_cast<int32_t>(*inputPtr) + inputOffset) *
                                            static_cast<int32_t>(filterData[filterIndex]);
                                }
                            }
                        }
                    }

                    inputPtr++;
                }
            }
        }
        outputBase += outputHeight * outputWidth * outputDepth;
    }

    const uint32_t outerSize = numBatches * outputHeight * outputWidth;
    int32_t* bufferPtr = tempBuffer;
    uint8_t* outPtr = outputData;
    for (uint32_t i = 0; i < outerSize; i++) {
        for (uint32_t d = 0; d < outputDepth; d++, bufferPtr++, outPtr++) {
            int32_t outVal = *bufferPtr + biasData[d];
            outVal = tflite::MultiplyByQuantizedMultiplier(outVal, outputMultiplier[d],
                                                           -outputShift[d]);
            outVal += outputOffset;
            outVal = std::max(std::min(outVal, outputActivationMax), outputActivationMin);
            *outPtr = static_cast<uint8_t>(outVal);
        }
    }

    return true;
}

bool transposeConvQuant8PerChannel(const uint8_t* inputData, const Shape& inputShape,
                                   const int8_t* filterData, const Shape& filterShape,
                                   const float* filterScales, const int32_t* biasData,
                                   const Shape& biasShape, const TransposeConv2dParam& param,
                                   uint8_t* outputData, const Shape& outputShape) {
    InputWithLayout<uint8_t> input(param.useNchw);
    OutputWithLayout<uint8_t> output(param.useNchw);
    NN_RET_CHECK(input.initialize(inputData, inputShape));
    NN_RET_CHECK(output.initialize(outputData, outputShape));
    NN_RET_CHECK(transposeConvQuant8PerChannelNhwc(
            input.getNhwcBuffer(), input.getNhwcShape(), filterData, filterShape, filterScales,
            biasData, biasShape, param, output.getNhwcBuffer(), output.getNhwcShape()));
    NN_RET_CHECK(output.commit());
    return true;
}

#undef ANDROID_NN_TRANSPOSE_CONV_PARAMETERS

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    auto inputCount = context->getNumInputs();
    auto inputType = context->getInputType(kInputTensor);
    auto filterType = context->getInputType(kFilterTensor);
    std::vector<OperandType> inExpectedTypes;
    if (inputType == OperandType::TENSOR_FLOAT32 || inputType == OperandType::TENSOR_FLOAT16) {
        inExpectedTypes = {inputType, inputType, inputType};
    } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK(filterType == OperandType::TENSOR_QUANT8_ASYMM ||
                     filterType == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL)
                << "Unsupported filter tensor type for operation " << kOperationName;
        if (filterType == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
            NN_RET_CHECK_EQ(context->getInputExtraParams(kFilterTensor).channelQuant().channelDim,
                            0)
                    << "Unsupported filter tensor channel dimension for operation "
                    << kOperationName;
        }
        inExpectedTypes = {inputType, filterType, OperandType::TENSOR_INT32};
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported input tensor type for operation " << kOperationName;
    }

    std::vector<OperandType> argExpectedTypes;
    if (inputCount == 11) {
        argExpectedTypes = {OperandType::INT32, OperandType::INT32, OperandType::INT32,
                            OperandType::INT32, OperandType::INT32, OperandType::INT32,
                            OperandType::INT32, OperandType::BOOL};
    } else {
        argExpectedTypes = {OperandType::TENSOR_INT32, OperandType::INT32, OperandType::INT32,
                            OperandType::INT32,        OperandType::INT32, OperandType::BOOL};
    }
    inExpectedTypes.insert(inExpectedTypes.end(), argExpectedTypes.begin(), argExpectedTypes.end());
    NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
    return validateInputTypes(context, inExpectedTypes) &&
           validateOutputTypes(context, {inputType});
}

bool prepare(IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    Shape filter = context->getInputShape(kFilterTensor);
    Shape bias = context->getInputShape(kBiasTensor);

    if (filter.type == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
        NN_RET_CHECK(input.type == OperandType::TENSOR_QUANT8_ASYMM);
    } else {
        NN_RET_CHECK(input.type == filter.type);
    }
    if (input.type == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK(bias.type == OperandType::TENSOR_INT32);
    } else {
        NN_RET_CHECK(input.type == bias.type);
    }
    NN_RET_CHECK_EQ(getNumberOfDimensions(input), 4);
    NN_RET_CHECK_EQ(getNumberOfDimensions(filter), 4);
    NN_RET_CHECK_EQ(getNumberOfDimensions(bias), 1);

    TransposeConv2dParam param;
    NN_RET_CHECK(param.initialize(context));

    uint32_t batches = getSizeOfDimension(input, 0);
    uint32_t height = getSizeOfDimension(input, param.useNchw ? 2 : 1);
    uint32_t width = getSizeOfDimension(input, param.useNchw ? 3 : 2);
    uint32_t channels_in = getSizeOfDimension(input, param.useNchw ? 1 : 3);
    uint32_t channels_out = getSizeOfDimension(filter, 0);
    uint32_t filterHeight = getSizeOfDimension(filter, 1);
    uint32_t filterWidth = getSizeOfDimension(filter, 2);
    // Only batches can be zero.
    NN_RET_CHECK_EQ(channels_in, getSizeOfDimension(filter, 3));
    NN_RET_CHECK_EQ(channels_out, getSizeOfDimension(bias, 0));
    NN_RET_CHECK_GT(height, 0);
    NN_RET_CHECK_GT(width, 0);
    NN_RET_CHECK_GT(channels_in, 0);
    NN_RET_CHECK_GT(channels_out, 0);
    NN_RET_CHECK_GT(filterWidth, 0);
    NN_RET_CHECK_GT(filterHeight, 0);

    uint32_t outWidth = computeOutSizeTransposeConv(width, filterWidth, param.strideWidth,
                                                    param.paddingLeft, param.paddingRight);
    uint32_t outHeight = computeOutSizeTransposeConv(height, filterHeight, param.strideHeight,
                                                     param.paddingTop, param.paddingBottom);
    NN_RET_CHECK_GT(outWidth, 0);
    NN_RET_CHECK_GT(outHeight, 0);

    Shape output = context->getOutputShape(kOutputTensor);
    output.type = input.type;
    if (param.useNchw) {
        output.dimensions = {batches, channels_out, outHeight, outWidth};
    } else {
        output.dimensions = {batches, outHeight, outWidth, channels_out};
    }
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    TransposeConv2dParam param;
    NN_RET_CHECK(param.initialize(context));
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT32:
            return transposeConv(context->getInputBuffer<float>(kInputTensor),
                                 context->getInputShape(kInputTensor),
                                 context->getInputBuffer<float>(kFilterTensor),
                                 context->getInputShape(kFilterTensor),
                                 context->getInputBuffer<float>(kBiasTensor),
                                 context->getInputShape(kBiasTensor), param,
                                 context->getOutputBuffer<float>(kOutputTensor),
                                 context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT16:
            return transposeConv(context->getInputBuffer<_Float16>(kInputTensor),
                                 context->getInputShape(kInputTensor),
                                 context->getInputBuffer<_Float16>(kFilterTensor),
                                 context->getInputShape(kFilterTensor),
                                 context->getInputBuffer<_Float16>(kBiasTensor),
                                 context->getInputShape(kBiasTensor), param,
                                 context->getOutputBuffer<_Float16>(kOutputTensor),
                                 context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            if (context->getInputType(kFilterTensor) ==
                OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
                return transposeConvQuant8PerChannel(
                        context->getInputBuffer<uint8_t>(kInputTensor),
                        context->getInputShape(kInputTensor),
                        context->getInputBuffer<int8_t>(kFilterTensor),
                        context->getInputShape(kFilterTensor),
                        context->getInputExtraParams(kFilterTensor).channelQuant().scales.data(),
                        context->getInputBuffer<int32_t>(kBiasTensor),
                        context->getInputShape(kBiasTensor), param,
                        context->getOutputBuffer<uint8_t>(kOutputTensor),
                        context->getOutputShape(kOutputTensor));
            } else if (context->getInputType(kFilterTensor) == OperandType::TENSOR_QUANT8_ASYMM) {
                return transposeConv(context->getInputBuffer<uint8_t>(kInputTensor),
                                     context->getInputShape(kInputTensor),
                                     context->getInputBuffer<uint8_t>(kFilterTensor),
                                     context->getInputShape(kFilterTensor),
                                     context->getInputBuffer<int32_t>(kBiasTensor),
                                     context->getInputShape(kBiasTensor), param,
                                     context->getOutputBuffer<uint8_t>(kOutputTensor),
                                     context->getOutputShape(kOutputTensor));
            } else {
                NN_RET_CHECK_FAIL() << "Unsupported filter type for operation " << kOperationName;
            }
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace transpose_conv_2d

NN_REGISTER_OPERATION(TRANSPOSE_CONV_2D, transpose_conv_2d::kOperationName,
                      transpose_conv_2d::validate, transpose_conv_2d::prepare,
                      transpose_conv_2d::execute, .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
