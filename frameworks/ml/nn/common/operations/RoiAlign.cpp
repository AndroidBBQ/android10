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
#include "OperationsUtils.h"

#include <cfloat>
#include <cmath>

#include "Tracing.h"
#include "tensorflow/lite/kernels/internal/common.h"

namespace android {
namespace nn {
namespace roi_align {

constexpr char kOperationName[] = "ROI_ALIGN";

constexpr uint32_t kNumInputs = 10;
constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kRoiTensor = 1;
constexpr uint32_t kBatchSplitTensor = 2;
constexpr uint32_t kOutputHeightScalar = 3;
constexpr uint32_t kOutputWidthScalar = 4;
constexpr uint32_t kHeightStrideSalar = 5;
constexpr uint32_t kWidthStrideScalar = 6;
constexpr uint32_t kHeightSamplingRatioScalar = 7;
constexpr uint32_t kWidthSamplingRatioScalar = 8;
constexpr uint32_t kLayoutScalar = 9;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

template <typename T_Input, typename T_Roi>
inline bool roiAlignNhwc(const T_Input* inputData, const Shape& inputShape, const T_Roi* roiData,
                         const Shape& roiShape, const int32_t* batchSplitData,
                         const Shape& batchSplitShape, float heightStride, float widthStride,
                         int32_t heightSamplingRatio, int32_t widthSamplingRatio,
                         T_Input* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("RoiAlign");

    const uint32_t kRoiDim = 4;
    const T_Roi heightScale = 1.0f / heightStride;
    const T_Roi widthScale = 1.0f / widthStride;

    uint32_t numBatches = getSizeOfDimension(inputShape, 0);
    uint32_t inHeight = getSizeOfDimension(inputShape, 1);
    uint32_t inWidth = getSizeOfDimension(inputShape, 2);
    uint32_t inDepth = getSizeOfDimension(inputShape, 3);
    uint32_t outHeight = getSizeOfDimension(outputShape, 1);
    uint32_t outWidth = getSizeOfDimension(outputShape, 2);
    uint32_t numRois = getSizeOfDimension(roiShape, 0);
    uint32_t roiInfoLength = getSizeOfDimension(roiShape, 1);

    T_Input* outPtr = outputData;
    const T_Roi* roiDataEnd = roiData + numRois * roiInfoLength;
    uint32_t roiIndex = 0;
    for (const T_Roi* roiInfo = roiData; roiInfo < roiDataEnd; roiInfo += kRoiDim, roiIndex++) {
        uint32_t batchId = static_cast<uint32_t>(batchSplitData[roiIndex]);
        // Check for malformed data
        // 1. invalid batch id
        // 2. Region out of bound: x1|x2|y1|y2 < 0 || x1|x2 > inWidth || y1|y2 > inHeight
        // 3. Invalid region: x2 < x1 || y2 < y1
        NN_RET_CHECK_GE(batchId, 0);
        NN_RET_CHECK_LT(batchId, numBatches);
        NN_RET_CHECK(roiInfo[0] >= 0);
        NN_RET_CHECK(roiInfo[1] >= 0);
        NN_RET_CHECK(roiInfo[2] >= 0);
        NN_RET_CHECK(roiInfo[3] >= 0);
        NN_RET_CHECK(roiInfo[0] * widthScale <= inWidth);
        NN_RET_CHECK(roiInfo[1] * heightScale <= inHeight);
        NN_RET_CHECK(roiInfo[2] * widthScale <= inWidth);
        NN_RET_CHECK(roiInfo[3] * heightScale <= inHeight);
        NN_RET_CHECK(roiInfo[0] <= roiInfo[2]);
        NN_RET_CHECK(roiInfo[1] <= roiInfo[3]);

        T_Roi wRoiStart = roiInfo[0] * widthScale;
        T_Roi hRoiStart = roiInfo[1] * heightScale;
        T_Roi wRoiEnd = roiInfo[2] * widthScale;
        T_Roi hRoiEnd = roiInfo[3] * heightScale;

        T_Roi roiWidth = std::max(static_cast<float>(wRoiEnd - wRoiStart), 1.0f);
        T_Roi roiHeight = std::max(static_cast<float>(hRoiEnd - hRoiStart), 1.0f);
        T_Roi wStepSize = roiWidth / static_cast<T_Roi>(outWidth);
        T_Roi hStepSize = roiHeight / static_cast<T_Roi>(outHeight);

        // if samplingRatio = 0, use adaptive value of ceil(roiWidth/outWidth), same for height
        uint32_t wSamplingRatio = widthSamplingRatio > 0 ? widthSamplingRatio
                                                         : std::ceil(static_cast<float>(wStepSize));
        uint32_t hSamplingRatio = heightSamplingRatio > 0
                                          ? heightSamplingRatio
                                          : std::ceil(static_cast<float>(hStepSize));
        int32_t numSamplingPoints = wSamplingRatio * hSamplingRatio;
        T_Roi wBinSize = wStepSize / static_cast<T_Roi>(wSamplingRatio);
        T_Roi hBinSize = hStepSize / static_cast<T_Roi>(hSamplingRatio);

        const T_Input* batchBase = inputData + batchId * inHeight * inWidth * inDepth;
        for (uint32_t i = 0; i < outHeight; i++) {
            for (uint32_t j = 0; j < outWidth; j++) {
                T_Roi wStart = wStepSize * j + wRoiStart;
                T_Roi wEnd = wStepSize * (j + 1) + wRoiStart;
                T_Roi hStart = hStepSize * i + hRoiStart;
                T_Roi hEnd = hStepSize * (i + 1) + hRoiStart;

                // initialize output to zero
                for (uint32_t k = 0; k < inDepth; k++) outPtr[k] = 0;

                // calculate the sum of the sampling points
                for (uint32_t yInd = 0; yInd < hSamplingRatio; yInd++) {
                    for (uint32_t xInd = 0; xInd < wSamplingRatio; xInd++) {
                        T_Roi y = hStart + hBinSize / 2 + hBinSize * yInd;
                        T_Roi x = wStart + wBinSize / 2 + wBinSize * xInd;

                        // bilinear interpolation of point (x,y)
                        // w.r.t box [(x1,y1), (x1,y2), (x2,y1), (x2,y2)]
                        uint32_t x1 = std::floor(static_cast<float>(x));
                        uint32_t y1 = std::floor(static_cast<float>(y));
                        uint32_t x2 = x1 + 1, y2 = y1 + 1;
                        T_Roi dx1 = x - static_cast<T_Roi>(x1);
                        T_Roi dy1 = y - static_cast<T_Roi>(y1);

                        // dealing with out of bound samples
                        if (x1 >= inWidth - 1) {
                            x1 = x2 = inWidth - 1;
                            dx1 = 0;
                        }
                        if (y1 >= inHeight - 1) {
                            y1 = y2 = inHeight - 1;
                            dy1 = 0;
                        }

                        T_Roi dx2 = 1.0f - dx1, dy2 = 1.0f - dy1;
                        T_Roi ws[] = {dx2 * dy2, dx1 * dy2, dx2 * dy1, dx1 * dy1};
                        uint32_t offsets[] = {y1 * inWidth * inDepth + x1 * inDepth,
                                              y1 * inWidth * inDepth + x2 * inDepth,
                                              y2 * inWidth * inDepth + x1 * inDepth,
                                              y2 * inWidth * inDepth + x2 * inDepth};

                        for (uint32_t k = 0; k < inDepth; k++) {
                            T_Input interpolation = 0;
                            for (uint32_t c = 0; c < 4; c++) {
                                interpolation += ws[c] * batchBase[offsets[c] + k];
                            }
                            outPtr[k] += interpolation;
                        }
                    }
                }

                // take average
                for (uint32_t k = 0; k < inDepth; k++)
                    outPtr[k] /= static_cast<T_Input>(numSamplingPoints);
                outPtr += inDepth;
            }
        }
    }
    return true;
}

template <>
inline bool roiAlignNhwc<uint8_t, uint16_t>(const uint8_t* inputData, const Shape& inputShape,
                                            const uint16_t* roiData, const Shape& roiShape,
                                            const int32_t* batchSplitData,
                                            const Shape& batchSplitShape, float heightStride,
                                            float widthStride, int32_t heightSamplingRatio,
                                            int32_t widthSamplingRatio, uint8_t* outputData,
                                            const Shape& outputShape) {
    NNTRACE_TRANS("RoiAlignQuant8");

    constexpr float wScale = 1.0f / 255.0f;
    constexpr uint32_t kRoiDim = 4;
    const float heightScale = 1.0f / heightStride;
    const float widthScale = 1.0f / widthStride;

    uint32_t numBatches = getSizeOfDimension(inputShape, 0);
    uint32_t inHeight = getSizeOfDimension(inputShape, 1);
    uint32_t inWidth = getSizeOfDimension(inputShape, 2);
    uint32_t inDepth = getSizeOfDimension(inputShape, 3);
    uint32_t outHeight = getSizeOfDimension(outputShape, 1);
    uint32_t outWidth = getSizeOfDimension(outputShape, 2);
    uint32_t numRois = getSizeOfDimension(roiShape, 0);
    uint32_t roiInfoLength = getSizeOfDimension(roiShape, 1);

    uint8_t* outPtr = outputData;
    const uint16_t* roiDataEnd = roiData + numRois * roiInfoLength;
    uint32_t roiIndex = 0;
    for (const uint16_t* roiInfo = roiData; roiInfo < roiDataEnd; roiInfo += kRoiDim, roiIndex++) {
        uint32_t batchId = static_cast<uint32_t>(batchSplitData[roiIndex]);
        float wRoiStart = static_cast<float>(roiInfo[0]) * widthScale * 0.125f;
        float hRoiStart = static_cast<float>(roiInfo[1]) * heightScale * 0.125f;
        float wRoiEnd = static_cast<float>(roiInfo[2]) * widthScale * 0.125f;
        float hRoiEnd = static_cast<float>(roiInfo[3]) * heightScale * 0.125f;

        // Check for malformed data
        // 1. invalid batch id
        // 2. Region out of bound: x1|x2|y1|y2 < 0 || x1|x2 > inWidth || y1|y2 > inHeight
        // 3. Invalid region: x2 < x1 || y2 < y1
        NN_RET_CHECK_GE(batchId, 0);
        NN_RET_CHECK_LT(batchId, numBatches);
        NN_RET_CHECK(wRoiStart <= inWidth);
        NN_RET_CHECK(hRoiStart <= inHeight);
        NN_RET_CHECK(wRoiEnd <= inWidth);
        NN_RET_CHECK(hRoiEnd <= inHeight);
        NN_RET_CHECK_LE(wRoiStart, wRoiEnd);
        NN_RET_CHECK_LE(hRoiStart, hRoiEnd);

        float roiWidth = std::max(wRoiEnd - wRoiStart, 1.0f);
        float roiHeight = std::max(hRoiEnd - hRoiStart, 1.0f);
        float wStepSize = roiWidth / static_cast<float>(outWidth);
        float hStepSize = roiHeight / static_cast<float>(outHeight);

        // if samplingRatio = 0, use adaptive value of ceil(roiWidth/outWidth), same for height
        uint32_t wSamplingRatio =
                widthSamplingRatio > 0 ? widthSamplingRatio : std::ceil(wStepSize);
        uint32_t hSamplingRatio =
                heightSamplingRatio > 0 ? heightSamplingRatio : std::ceil(hStepSize);
        int32_t numSamplingPoints = wSamplingRatio * hSamplingRatio;
        float wBinSize = wStepSize / static_cast<float>(wSamplingRatio);
        float hBinSize = hStepSize / static_cast<float>(hSamplingRatio);

        float realMultiplier = inputShape.scale * wScale / outputShape.scale / numSamplingPoints;
        int32_t outputMultiplier = 0;
        int32_t outputShift = 0;
        if (!QuantizeMultiplierSmallerThanOne(realMultiplier, &outputMultiplier, &outputShift)) {
            return false;
        }

        const uint8_t* batchBase = inputData + batchId * inHeight * inWidth * inDepth;
        for (uint32_t i = 0; i < outHeight; i++) {
            for (uint32_t j = 0; j < outWidth; j++) {
                float wStart = wStepSize * j + wRoiStart;
                float wEnd = wStepSize * (j + 1) + wRoiStart;
                float hStart = hStepSize * i + hRoiStart;
                float hEnd = hStepSize * (i + 1) + hRoiStart;

                std::vector<int32_t> outTemp(inDepth, 0);
                // calculate the sum of the sampling points
                for (uint32_t yInd = 0; yInd < hSamplingRatio; yInd++) {
                    for (uint32_t xInd = 0; xInd < wSamplingRatio; xInd++) {
                        float y = hStart + hBinSize / 2 + hBinSize * yInd;
                        float x = wStart + wBinSize / 2 + wBinSize * xInd;

                        // bilinear interpolation of point (x,y)
                        // w.r.t box [(x1,y1), (x1,y2), (x2,y1), (x2,y2)]
                        uint32_t x1 = std::floor(x), y1 = std::floor(y);
                        uint32_t x2 = x1 + 1, y2 = y1 + 1;
                        float dx1 = x - static_cast<float>(x1);
                        float dy1 = y - static_cast<float>(y1);

                        // dealing with out of bound samples
                        if (x1 >= inWidth - 1) {
                            x1 = x2 = inWidth - 1;
                            dx1 = 0;
                        }
                        if (y1 >= inHeight - 1) {
                            y1 = y2 = inHeight - 1;
                            dy1 = 0;
                        }

                        float dx2 = 1.0f - dx1, dy2 = 1.0f - dy1;
                        float ws[] = {dx2 * dy2, dx1 * dy2, dx2 * dy1, dx1 * dy1};
                        uint32_t offsets[] = {y1 * inWidth * inDepth + x1 * inDepth,
                                              y1 * inWidth * inDepth + x2 * inDepth,
                                              y2 * inWidth * inDepth + x1 * inDepth,
                                              y2 * inWidth * inDepth + x2 * inDepth};

                        for (uint32_t k = 0; k < inDepth; k++) {
                            int32_t interpolation = 0;
                            for (uint32_t c = 0; c < 4; c++) {
                                int32_t wQuant = static_cast<int32_t>(std::round(ws[c] / wScale));
                                interpolation +=
                                        wQuant * (static_cast<int32_t>(batchBase[offsets[c] + k]) -
                                                  inputShape.offset);
                            }
                            outTemp[k] += interpolation;
                        }
                    }
                }

                // take average and cast to output quantization
                for (uint32_t k = 0; k < inDepth; k++) {
                    int32_t raw_out = tflite::MultiplyByQuantizedMultiplier(
                                              outTemp[k], outputMultiplier, -outputShift) +
                                      outputShape.offset;
                    int32_t clamped_out = std::min(255, std::max(0, raw_out));
                    outPtr[k] = static_cast<uint8_t>(clamped_out);
                }
                outPtr += inDepth;
            }
        }
    }
    return true;
}

template <typename T_Input, typename T_Roi>
inline bool roiAlign(const T_Input* inputData, const Shape& inputShape, const T_Roi* roiData,
                     const Shape& roiShape, const int32_t* batchSplitData,
                     const Shape& batchSplitShape, float heightStride, float widthStride,
                     int32_t heightSamplingRatio, int32_t widthSamplingRatio, bool useNchw,
                     T_Input* outputData, const Shape& outputShape) {
    InputWithLayout<T_Input> input(useNchw);
    OutputWithLayout<T_Input> output(useNchw);
    NN_RET_CHECK(input.initialize(inputData, inputShape));
    NN_RET_CHECK(output.initialize(outputData, outputShape));
    NN_RET_CHECK(roiAlignNhwc(input.getNhwcBuffer(), input.getNhwcShape(), roiData, roiShape,
                              batchSplitData, batchSplitShape, heightStride, widthStride,
                              heightSamplingRatio, widthSamplingRatio, output.getNhwcBuffer(),
                              output.getNhwcShape()));
    NN_RET_CHECK(output.commit());
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    std::vector<OperandType> inExpectedTypes;
    auto inputType = context->getInputType(kInputTensor);
    if (inputType == OperandType::TENSOR_FLOAT32) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT32, OperandType::TENSOR_FLOAT32,
                           OperandType::TENSOR_INT32,   OperandType::INT32,
                           OperandType::INT32,          OperandType::FLOAT32,
                           OperandType::FLOAT32,        OperandType::INT32,
                           OperandType::INT32,          OperandType::BOOL};
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16,
                           OperandType::TENSOR_INT32,   OperandType::INT32,
                           OperandType::INT32,          OperandType::FLOAT16,
                           OperandType::FLOAT16,        OperandType::INT32,
                           OperandType::INT32,          OperandType::BOOL};
    } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                           OperandType::TENSOR_QUANT16_ASYMM,
                           OperandType::TENSOR_INT32,
                           OperandType::INT32,
                           OperandType::INT32,
                           OperandType::FLOAT32,
                           OperandType::FLOAT32,
                           OperandType::INT32,
                           OperandType::INT32,
                           OperandType::BOOL};
    } else {
        LOG(ERROR) << "Unsupported input tensor type for operation " << kOperationName;
        return false;
    }
    NN_RET_CHECK(validateInputTypes(context, inExpectedTypes));
    NN_RET_CHECK(validateOutputTypes(context, {inputType}));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    bool useNchw = context->getInputValue<bool>(kLayoutScalar);
    Shape input = context->getInputShape(kInputTensor);
    Shape roiShape = context->getInputShape(kRoiTensor);
    Shape batchSplitShape = context->getInputShape(kBatchSplitTensor);
    NN_RET_CHECK_EQ(getNumberOfDimensions(input), 4);
    NN_RET_CHECK_EQ(getNumberOfDimensions(roiShape), 2);

    uint32_t numBatches = getSizeOfDimension(input, 0);
    uint32_t inHeight = getSizeOfDimension(input, useNchw ? 2 : 1);
    uint32_t inWidth = getSizeOfDimension(input, useNchw ? 3 : 2);
    uint32_t inDepth = getSizeOfDimension(input, useNchw ? 1 : 3);
    uint32_t numRois = getSizeOfDimension(roiShape, 0);
    // Every dimension must be positive except for numRois.
    NN_RET_CHECK_GT(numBatches, 0);
    NN_RET_CHECK_GT(inHeight, 0);
    NN_RET_CHECK_GT(inWidth, 0);
    NN_RET_CHECK_GT(inDepth, 0);
    NN_RET_CHECK_EQ(getSizeOfDimension(roiShape, 1), 4);
    NN_RET_CHECK_EQ(getSizeOfDimension(batchSplitShape, 0), numRois);

    int32_t outputHeight = context->getInputValue<int32_t>(kOutputHeightScalar);
    int32_t outputWidth = context->getInputValue<int32_t>(kOutputWidthScalar);
    int32_t heightSamplingRatio = context->getInputValue<int32_t>(kHeightSamplingRatioScalar);
    int32_t widthSamplingRatio = context->getInputValue<int32_t>(kWidthSamplingRatioScalar);
    float heightScale, widthScale;
    if (context->getInputType(kInputTensor) == OperandType::TENSOR_FLOAT16) {
        heightScale = context->getInputValue<_Float16>(kHeightStrideSalar);
        widthScale = context->getInputValue<_Float16>(kWidthStrideScalar);
    } else {
        heightScale = context->getInputValue<float>(kHeightStrideSalar);
        widthScale = context->getInputValue<float>(kWidthStrideScalar);
    }
    NN_RET_CHECK_GT(outputHeight, 0);
    NN_RET_CHECK_GT(outputWidth, 0);
    NN_RET_CHECK_GT(heightScale, 0);
    NN_RET_CHECK_GT(widthScale, 0);
    // Sampling ratio can set to 0 for adaptive value.
    NN_RET_CHECK_GE(heightSamplingRatio, 0);
    NN_RET_CHECK_GE(widthSamplingRatio, 0);

    if (roiShape.type == OperandType::TENSOR_QUANT16_ASYMM) {
        NN_RET_CHECK_EQ(roiShape.scale, 0.125f);
        NN_RET_CHECK_EQ(roiShape.offset, 0);
    }

    Shape output = context->getOutputShape(kOutputTensor);
    output.type = input.type;
    if (useNchw) {
        output.dimensions = {numRois, inDepth, static_cast<uint32_t>(outputHeight),
                             static_cast<uint32_t>(outputWidth)};
    } else {
        output.dimensions = {numRois, static_cast<uint32_t>(outputHeight),
                             static_cast<uint32_t>(outputWidth), inDepth};
    }
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getInputShape(kRoiTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return roiAlign(context->getInputBuffer<_Float16>(kInputTensor),
                            context->getInputShape(kInputTensor),
                            context->getInputBuffer<_Float16>(kRoiTensor),
                            context->getInputShape(kRoiTensor),
                            context->getInputBuffer<int32_t>(kBatchSplitTensor),
                            context->getInputShape(kBatchSplitTensor),
                            context->getInputValue<_Float16>(kHeightStrideSalar),
                            context->getInputValue<_Float16>(kWidthStrideScalar),
                            context->getInputValue<int32_t>(kHeightSamplingRatioScalar),
                            context->getInputValue<int32_t>(kWidthSamplingRatioScalar),
                            context->getInputValue<bool>(kLayoutScalar),
                            context->getOutputBuffer<_Float16>(kOutputTensor),
                            context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return roiAlign(context->getInputBuffer<float>(kInputTensor),
                            context->getInputShape(kInputTensor),
                            context->getInputBuffer<float>(kRoiTensor),
                            context->getInputShape(kRoiTensor),
                            context->getInputBuffer<int32_t>(kBatchSplitTensor),
                            context->getInputShape(kBatchSplitTensor),
                            context->getInputValue<float>(kHeightStrideSalar),
                            context->getInputValue<float>(kWidthStrideScalar),
                            context->getInputValue<int32_t>(kHeightSamplingRatioScalar),
                            context->getInputValue<int32_t>(kWidthSamplingRatioScalar),
                            context->getInputValue<bool>(kLayoutScalar),
                            context->getOutputBuffer<float>(kOutputTensor),
                            context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return roiAlign(context->getInputBuffer<uint8_t>(kInputTensor),
                            context->getInputShape(kInputTensor),
                            context->getInputBuffer<uint16_t>(kRoiTensor),
                            context->getInputShape(kRoiTensor),
                            context->getInputBuffer<int32_t>(kBatchSplitTensor),
                            context->getInputShape(kBatchSplitTensor),
                            context->getInputValue<float>(kHeightStrideSalar),
                            context->getInputValue<float>(kWidthStrideScalar),
                            context->getInputValue<int32_t>(kHeightSamplingRatioScalar),
                            context->getInputValue<int32_t>(kWidthSamplingRatioScalar),
                            context->getInputValue<bool>(kLayoutScalar),
                            context->getOutputBuffer<uint8_t>(kOutputTensor),
                            context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace roi_align

NN_REGISTER_OPERATION(ROI_ALIGN, roi_align::kOperationName, roi_align::validate, roi_align::prepare,
                      roi_align::execute, .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
