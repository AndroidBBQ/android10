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
#include "Operations.h"

#include "tensorflow/lite/kernels/internal/optimized/depthwiseconv_float.h"
#include "tensorflow/lite/kernels/internal/optimized/depthwiseconv_uint8.h"

#include "Tracing.h"

namespace android {
namespace nn {

bool depthwiseConvFloat16(const _Float16* inputData, const Shape& inputShape,
                          const _Float16* filterData, const Shape& filterShape,
                          const _Float16* biasData, const Shape& biasShape, int32_t paddingLeft,
                          int32_t paddingRight, int32_t paddingTop, int32_t paddingBottom,
                          int32_t strideWidth, int32_t strideHeight, int32_t dilationWidthFactor,
                          int32_t dilationHeightFactor, int32_t depthMultiplier, int32_t activation,
                          _Float16* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("depthwiseConvFloat16");
    std::vector<float> inputDataFloat32(getNumberOfElements(inputShape));
    convertFloat16ToFloat32(inputData, &inputDataFloat32);
    std::vector<float> filterDataFloat32(getNumberOfElements(filterShape));
    convertFloat16ToFloat32(filterData, &filterDataFloat32);
    std::vector<float> biasDataFloat32(getNumberOfElements(biasShape));
    convertFloat16ToFloat32(biasData, &biasDataFloat32);

    std::vector<float> outputDataFloat32(getNumberOfElements(outputShape));
    depthwiseConvFloat32(inputDataFloat32.data(), inputShape, filterDataFloat32.data(), filterShape,
                         biasDataFloat32.data(), biasShape, paddingLeft, paddingRight, paddingTop,
                         paddingBottom, strideWidth, strideHeight, dilationWidthFactor,
                         dilationHeightFactor, depthMultiplier, activation,
                         outputDataFloat32.data(), outputShape);

    convertFloat32ToFloat16(outputDataFloat32, outputData);
    return true;
}

#define ANDROID_NN_DEPTHWISE_CONV_PARAMETERS                    \
    uint32_t height = getSizeOfDimension(inputShape, 1);        \
    uint32_t width = getSizeOfDimension(inputShape, 2);         \
    uint32_t filterHeight = getSizeOfDimension(filterShape, 1); \
    uint32_t filterWidth = getSizeOfDimension(filterShape, 2);  \
    uint32_t outHeight = getSizeOfDimension(outputShape, 1);    \
    uint32_t outWidth = getSizeOfDimension(outputShape, 2);     \
                                                                \
    uint32_t paddingHeight = (uint32_t)paddingTop;              \
    uint32_t paddingWidth = (uint32_t)paddingLeft;

bool depthwiseConvFloat32(const float* inputData, const Shape& inputShape, const float* filterData,
                          const Shape& filterShape, const float* biasData, const Shape& biasShape,
                          int32_t paddingLeft, int32_t paddingRight, int32_t paddingTop,
                          int32_t paddingBottom, int32_t strideWidth, int32_t strideHeight,
                          int32_t dilationWidthFactor, int32_t dilationHeightFactor,
                          int32_t depthMultiplier, int32_t activation, float* outputData,
                          const Shape& outputShape) {
    NNTRACE_TRANS("depthwiseConvFloat32");

    ANDROID_NN_DEPTHWISE_CONV_PARAMETERS

    float output_activation_min, output_activation_max;
    CalculateActivationRangeFloat(activation, &output_activation_min, &output_activation_max);

    tflite::DepthwiseParams params{
            .padding_values = {static_cast<int16>(paddingWidth), static_cast<int16>(paddingHeight)},
            .stride_width = static_cast<int16>(strideWidth),
            .stride_height = static_cast<int16>(strideHeight),
            .depth_multiplier = static_cast<int16>(depthMultiplier),
            .float_activation_min = output_activation_min,
            .float_activation_max = output_activation_max,
            .dilation_width_factor = static_cast<int16>(dilationWidthFactor),
            .dilation_height_factor = static_cast<int16>(dilationHeightFactor),
    };
    NNTRACE_COMP_SWITCH("optimized_ops::DepthwiseConv");
    tflite::optimized_ops::DepthwiseConv(params, convertShapeToTflshape(inputShape), inputData,
                                         convertShapeToTflshape(filterShape), filterData,
                                         convertShapeToTflshape(biasShape), biasData,
                                         convertShapeToTflshape(outputShape), outputData);

    return true;
}

bool depthwiseConvQuant8(const uint8_t* inputData, const Shape& inputShape,
                         const uint8_t* filterData, const Shape& filterShape,
                         const int32_t* biasData, const Shape& biasShape, int32_t paddingLeft,
                         int32_t paddingRight, int32_t paddingTop, int32_t paddingBottom,
                         int32_t strideWidth, int32_t strideHeight, int32_t dilationWidthFactor,
                         int32_t dilationHeightFactor, int32_t depthMultiplier, int32_t activation,
                         uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("depthwiseConvQuant8");

    ANDROID_NN_DEPTHWISE_CONV_PARAMETERS

    double real_multiplier = 0.0;
    int32_t output_multiplier = 0;
    int32_t output_shift = 0;
    int32_t output_activation_min = 0;
    int32_t output_activation_max = 0;

    NN_RET_CHECK(GetQuantizedConvolutionMultipler(inputShape, filterShape, biasShape, outputShape,
                                                  &real_multiplier));
    int exponent;
    NN_RET_CHECK(QuantizeMultiplier(real_multiplier, &output_multiplier, &exponent));
    output_shift = -exponent;
    CalculateActivationRangeUint8(activation, outputShape, &output_activation_min,
                                  &output_activation_max);

    tflite::DepthwiseParams params{
            .padding_values = {static_cast<int16>(paddingWidth), static_cast<int16>(paddingHeight)},
            .stride_width = static_cast<int16>(strideWidth),
            .stride_height = static_cast<int16>(strideHeight),
            .depth_multiplier = static_cast<int16>(depthMultiplier),
            .quantized_activation_min = output_activation_min,
            .quantized_activation_max = output_activation_max,
            .dilation_width_factor = static_cast<int16>(dilationWidthFactor),
            .dilation_height_factor = static_cast<int16>(dilationHeightFactor),
            .input_offset = -inputShape.offset,
            .weights_offset = -filterShape.offset,
            .output_offset = outputShape.offset,
            .output_shift = -output_shift,
            .output_multiplier = output_multiplier,
    };
    NNTRACE_COMP_SWITCH("optimized_ops::DepthwiseConv");
    tflite::optimized_ops::DepthwiseConv(params, convertShapeToTflshape(inputShape), inputData,
                                         convertShapeToTflshape(filterShape), filterData,
                                         convertShapeToTflshape(biasShape), biasData,
                                         convertShapeToTflshape(outputShape), outputData);
    return true;
}

bool depthwiseConvQuant8PerChannel(const uint8_t* inputData, const Shape& inputShape,
                                   const int8_t* filterData, const Shape& filterShape,
                                   const float* filterScales, const int32_t* biasData,
                                   const Shape& biasShape, int32_t paddingLeft,
                                   int32_t paddingRight, int32_t paddingTop, int32_t paddingBottom,
                                   int32_t strideWidth, int32_t strideHeight,
                                   int32_t dilationWidthFactor, int32_t dilationHeightFactor,

                                   int32_t depthMultiplier, int32_t activation, uint8_t* outputData,
                                   const Shape& outputShape) {
    NNTRACE_TRANS("depthwiseConvQuant8");

    uint32_t paddingHeight = (uint32_t)paddingTop;
    uint32_t paddingWidth = (uint32_t)paddingLeft;

    uint32_t numBatches = getSizeOfDimension(inputShape, 0);
    uint32_t inputHeight = getSizeOfDimension(inputShape, 1);
    uint32_t inputWidth = getSizeOfDimension(inputShape, 2);
    uint32_t inputDepth = getSizeOfDimension(inputShape, 3);
    uint32_t filterHeight = getSizeOfDimension(filterShape, 1);
    uint32_t filterWidth = getSizeOfDimension(filterShape, 2);
    uint32_t filterDepth = getSizeOfDimension(filterShape, 3);
    uint32_t outputHeight = getSizeOfDimension(outputShape, 1);
    uint32_t outputWidth = getSizeOfDimension(outputShape, 2);
    uint32_t outputDepth = getSizeOfDimension(outputShape, 3);

    int32_t inputOffset = -inputShape.offset;
    int32_t outputOffset = outputShape.offset;

    auto realMultiplier = std::vector<double>(outputDepth, .0f);
    auto outputMultiplier = std::vector<int32_t>(outputDepth, 0);
    auto outputShift = std::vector<int32_t>(outputDepth, .0f);

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

    int32_t output_activation_min = 0, output_activation_max = 0;
    CalculateActivationRangeUint8(activation, outputShape, &output_activation_min,
                                  &output_activation_max);

    const uint8_t* inputBase = inputData;
    uint8_t* outPtr = outputData;
    for (uint32_t b = 0; b < numBatches; b++) {
        for (uint32_t h = 0; h < outputHeight; h++) {
            for (uint32_t w = 0; w < outputWidth; w++) {
                for (uint32_t ic = 0; ic < inputDepth; ic++) {
                    for (uint32_t m = 0; m < depthMultiplier; m++) {
                        int32_t wInputOrigin = static_cast<int32_t>(w) * strideWidth - paddingLeft;
                        int32_t hInputOrigin = static_cast<int32_t>(h) * strideHeight - paddingTop;
                        const int oc = m + ic * depthMultiplier;

                        int32_t sum = 0.0f;
                        for (uint32_t i = 0; i < filterHeight; i++) {
                            for (uint32_t j = 0; j < filterWidth; j++) {
                                int32_t hInput = hInputOrigin +
                                                 dilationHeightFactor * static_cast<int32_t>(i);
                                int32_t wInput = wInputOrigin +
                                                 dilationWidthFactor * static_cast<int32_t>(j);

                                if (hInput >= 0 && hInput < static_cast<int32_t>(inputHeight) &&
                                    wInput >= 0 && wInput < static_cast<int32_t>(inputWidth)) {
                                    uint32_t filterIndex =
                                            i * filterWidth * filterDepth + j * filterDepth + oc;
                                    uint32_t inputIndex = hInput * inputWidth * inputDepth +
                                                          wInput * inputDepth + ic;
                                    sum += (static_cast<int32_t>(filterData[filterIndex])) *
                                           (static_cast<int32_t>(inputBase[inputIndex]) +
                                            inputOffset);
                                }
                            }
                        }

                        sum += biasData[oc];
                        sum = tflite::MultiplyByQuantizedMultiplier(sum, outputMultiplier[oc],
                                                                    -outputShift[oc]);
                        sum += outputOffset;
                        sum = std::max(std::min(sum, output_activation_max), output_activation_min);
                        outPtr[m] = static_cast<uint8_t>(sum);
                    }
                    outPtr += depthMultiplier;
                }
            }
        }
        inputBase += inputHeight * inputWidth * inputDepth;
    }

    return true;
}

#undef ANDROID_NN_DEPTHWISE_CONV_PARAMETERS
}  // namespace nn
}  // namespace android
