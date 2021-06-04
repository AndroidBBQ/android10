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
#include "Operations.h"

#include <cfloat>
#include <cmath>

#include "Tracing.h"
#include "tensorflow/lite/kernels/internal/common.h"

namespace android {
namespace nn {

#define ANDROID_NN_GROUPED_CONV_PARAMETERS                      \
    uint32_t numBatches = getSizeOfDimension(inputShape, 0);    \
    uint32_t inputHeight = getSizeOfDimension(inputShape, 1);   \
    uint32_t inputWidth = getSizeOfDimension(inputShape, 2);    \
    uint32_t inputDepth = getSizeOfDimension(inputShape, 3);    \
    uint32_t filterHeight = getSizeOfDimension(filterShape, 1); \
    uint32_t filterWidth = getSizeOfDimension(filterShape, 2);  \
    uint32_t filterDepth = getSizeOfDimension(filterShape, 3);  \
    uint32_t outputHeight = getSizeOfDimension(outputShape, 1); \
    uint32_t outputWidth = getSizeOfDimension(outputShape, 2);  \
    uint32_t outputDepth = getSizeOfDimension(outputShape, 3);  \
    uint32_t outputGroupDepth = outputDepth / numGroups;

bool groupedConvFloat32(const float* inputData, const Shape& inputShape, const float* filterData,
                        const Shape& filterShape, const float* biasData, const Shape& biasShape,
                        int32_t padding_left, int32_t padding_right, int32_t padding_top,
                        int32_t padding_bottom, int32_t stride_width, int32_t stride_height,
                        int32_t numGroups, int32_t activation, float* outputData,
                        const Shape& outputShape) {
    NNTRACE_TRANS("groupConvFloat32");
    ANDROID_NN_GROUPED_CONV_PARAMETERS

    float output_activation_min = 0.0f, output_activation_max = 0.0f;
    CalculateActivationRangeFloat(activation, &output_activation_min, &output_activation_max);

    const float* inputBase = inputData;
    float* outPtr = outputData;
    for (uint32_t b = 0; b < numBatches; b++) {
        for (uint32_t h = 0; h < outputHeight; h++) {
            for (uint32_t w = 0; w < outputWidth; w++) {
                const float* filterBase = filterData;
                for (uint32_t g = 0; g < numGroups; g++) {
                    for (uint32_t d = 0; d < outputGroupDepth; d++) {
                        int32_t wInputOrigin =
                                static_cast<int32_t>(w) * stride_width - padding_left;
                        int32_t hInputOrigin =
                                static_cast<int32_t>(h) * stride_height - padding_top;
                        float sum = 0.0f;
                        for (uint32_t i = 0; i < filterHeight; i++) {
                            for (uint32_t j = 0; j < filterWidth; j++) {
                                for (uint32_t k = 0; k < filterDepth; k++) {
                                    int32_t hInput = hInputOrigin + static_cast<int32_t>(i);
                                    int32_t wInput = wInputOrigin + static_cast<int32_t>(j);
                                    uint32_t dInput = filterDepth * g + k;
                                    if (hInput >= 0 && hInput < static_cast<int32_t>(inputHeight) &&
                                        wInput >= 0 && wInput < static_cast<int32_t>(inputWidth)) {
                                        uint32_t filterIndex =
                                                i * filterWidth * filterDepth + j * filterDepth + k;
                                        uint32_t inputIndex = hInput * inputWidth * inputDepth +
                                                              wInput * inputDepth + dInput;
                                        sum += filterBase[filterIndex] * inputBase[inputIndex];
                                    }
                                }
                            }
                        }
                        sum += biasData[g * outputGroupDepth + d];
                        sum = std::max(std::min(sum, output_activation_max), output_activation_min);
                        outPtr[d] = sum;
                        filterBase += filterHeight * filterWidth * filterDepth;
                    }
                    outPtr += outputGroupDepth;
                }
            }
        }
        inputBase += inputHeight * inputWidth * inputDepth;
    }

    return true;
}

bool groupedConvQuant8(const uint8_t* inputData, const Shape& inputShape, const uint8_t* filterData,
                       const Shape& filterShape, const int32_t* biasData, const Shape& biasShape,
                       int32_t padding_left, int32_t padding_right, int32_t padding_top,
                       int32_t padding_bottom, int32_t stride_width, int32_t stride_height,
                       int32_t numGroups, int32_t activation, uint8_t* outputData,
                       const Shape& outputShape) {
    NNTRACE_TRANS("groupConvQuant8");
    ANDROID_NN_GROUPED_CONV_PARAMETERS

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

    int32_t output_activation_min = 0, output_activation_max = 0;
    CalculateActivationRangeUint8(activation, outputShape, &output_activation_min,
                                  &output_activation_max);

    const uint8_t* inputBase = inputData;
    uint8_t* outPtr = outputData;
    for (uint32_t b = 0; b < numBatches; b++) {
        for (uint32_t h = 0; h < outputHeight; h++) {
            for (uint32_t w = 0; w < outputWidth; w++) {
                const uint8_t* filterBase = filterData;
                for (uint32_t g = 0; g < numGroups; g++) {
                    for (uint32_t d = 0; d < outputGroupDepth; d++) {
                        int32_t wInputOrigin =
                                static_cast<int32_t>(w) * stride_width - padding_left;
                        int32_t hInputOrigin =
                                static_cast<int32_t>(h) * stride_height - padding_top;
                        int32_t sum = 0.0f;
                        for (uint32_t i = 0; i < filterHeight; i++) {
                            for (uint32_t j = 0; j < filterWidth; j++) {
                                for (uint32_t k = 0; k < filterDepth; k++) {
                                    int32_t hInput = hInputOrigin + static_cast<int32_t>(i);
                                    int32_t wInput = wInputOrigin + static_cast<int32_t>(j);
                                    uint32_t dInput = filterDepth * g + k;
                                    if (hInput >= 0 && hInput < static_cast<int32_t>(inputHeight) &&
                                        wInput >= 0 && wInput < static_cast<int32_t>(inputWidth)) {
                                        uint32_t filterIndex =
                                                i * filterWidth * filterDepth + j * filterDepth + k;
                                        uint32_t inputIndex = hInput * inputWidth * inputDepth +
                                                              wInput * inputDepth + dInput;
                                        sum += (static_cast<int32_t>(filterBase[filterIndex]) +
                                                filterOffset) *
                                               (static_cast<int32_t>(inputBase[inputIndex]) +
                                                inputOffset);
                                    }
                                }
                            }
                        }
                        sum += biasData[g * outputGroupDepth + d];
                        sum = tflite::MultiplyByQuantizedMultiplier(sum, outputMultiplier,
                                                                    -outputShift);
                        sum += outputOffset;
                        sum = std::max(std::min(sum, output_activation_max), output_activation_min);
                        outPtr[d] = static_cast<uint8_t>(sum);
                        filterBase += filterHeight * filterWidth * filterDepth;
                    }
                    outPtr += outputGroupDepth;
                }
            }
        }
        inputBase += inputHeight * inputWidth * inputDepth;
    }

    return true;
}

bool groupedConvQuant8PerChannel(const uint8_t* inputData, const Shape& inputShape,
                                 const int8_t* filterData, const Shape& filterShape,
                                 const float* filterScales, const int32_t* biasData,
                                 const Shape& biasShape, int32_t padding_left,
                                 int32_t padding_right, int32_t padding_top, int32_t padding_bottom,
                                 int32_t stride_width, int32_t stride_height, int32_t numGroups,
                                 int32_t activation, uint8_t* outputData,
                                 const Shape& outputShape) {
    NNTRACE_TRANS("groupConvQuant8");
    ANDROID_NN_GROUPED_CONV_PARAMETERS

    int32_t inputOffset = -inputShape.offset;
    int32_t outputOffset = outputShape.offset;

    auto realMultiplier = std::vector<double>(outputDepth, .0f);
    auto outputMultiplier = std::vector<int32_t>(outputDepth, 0);
    auto outputShift = std::vector<int32_t>(outputDepth, 0);

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
                const int8_t* filterBase = filterData;
                for (uint32_t g = 0; g < numGroups; g++) {
                    for (uint32_t d = 0; d < outputGroupDepth; d++) {
                        int32_t wInputOrigin =
                                static_cast<int32_t>(w) * stride_width - padding_left;
                        int32_t hInputOrigin =
                                static_cast<int32_t>(h) * stride_height - padding_top;
                        int32_t sum = 0.0f;
                        for (uint32_t i = 0; i < filterHeight; i++) {
                            for (uint32_t j = 0; j < filterWidth; j++) {
                                for (uint32_t k = 0; k < filterDepth; k++) {
                                    int32_t hInput = hInputOrigin + static_cast<int32_t>(i);
                                    int32_t wInput = wInputOrigin + static_cast<int32_t>(j);
                                    uint32_t dInput = filterDepth * g + k;
                                    if (hInput >= 0 && hInput < static_cast<int32_t>(inputHeight) &&
                                        wInput >= 0 && wInput < static_cast<int32_t>(inputWidth)) {
                                        uint32_t filterIndex =
                                                i * filterWidth * filterDepth + j * filterDepth + k;
                                        uint32_t inputIndex = hInput * inputWidth * inputDepth +
                                                              wInput * inputDepth + dInput;
                                        sum += (static_cast<int32_t>(filterBase[filterIndex])) *
                                               (static_cast<int32_t>(inputBase[inputIndex]) +
                                                inputOffset);
                                    }
                                }
                            }
                        }
                        int channelIndex = g * outputGroupDepth + d;
                        sum += biasData[channelIndex];
                        sum = tflite::MultiplyByQuantizedMultiplier(
                                sum, outputMultiplier[channelIndex], -outputShift[channelIndex]);
                        sum += outputOffset;
                        sum = std::max(std::min(sum, output_activation_max), output_activation_min);
                        outPtr[d] = static_cast<uint8_t>(sum);
                        filterBase += filterHeight * filterWidth * filterDepth;
                    }
                    outPtr += outputGroupDepth;
                }
            }
        }
        inputBase += inputHeight * inputWidth * inputDepth;
    }

    return true;
}

bool groupedConvFloat16(const _Float16* inputData, const Shape& inputShape,
                        const _Float16* filterData, const Shape& filterShape,
                        const _Float16* biasData, const Shape& biasShape, int32_t padding_left,
                        int32_t padding_right, int32_t padding_top, int32_t padding_bottom,
                        int32_t stride_width, int32_t stride_height, int32_t numGroups,
                        int32_t activation, _Float16* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("groupConvFloat16");

    std::vector<float> inputData_float32(getNumberOfElements(inputShape));
    std::vector<float> filterData_float32(getNumberOfElements(filterShape));
    std::vector<float> biasData_float32(getNumberOfElements(biasShape));
    std::vector<float> outputData_float32(getNumberOfElements(outputShape));

    convertFloat16ToFloat32(inputData, &inputData_float32);
    convertFloat16ToFloat32(filterData, &filterData_float32);
    convertFloat16ToFloat32(biasData, &biasData_float32);

    groupedConvFloat32(inputData_float32.data(), inputShape, filterData_float32.data(), filterShape,
                       biasData_float32.data(), biasShape, padding_left, padding_right, padding_top,
                       padding_bottom, stride_width, stride_height, numGroups, activation,
                       outputData_float32.data(), outputShape);
    convertFloat32ToFloat16(outputData_float32, outputData);

    return true;
}

#undef ANDROID_NN_GROUPED_CONV_PARAMETERS
}  // namespace nn
}  // namespace android
