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

namespace android {
namespace nn {
namespace roi_pooling {

constexpr char kOperationName[] = "ROI_POOLING";

constexpr uint32_t kNumInputs = 8;
constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kRoiTensor = 1;
constexpr uint32_t kBatchSplitTensor = 2;
constexpr uint32_t kOutputHeightScalar = 3;
constexpr uint32_t kOutputWidthScalar = 4;
constexpr uint32_t kHeightStrideSalar = 5;
constexpr uint32_t kWidthStrideScalar = 6;
constexpr uint32_t kLayoutScalar = 7;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

template <typename T_Input, typename T_Roi>
inline bool roiPoolingNhwc(const T_Input* inputData, const Shape& inputShape, const T_Roi* roiData,
                           const Shape& roiShape, const int32_t* batchSplitData,
                           const Shape& batchSplitShape, float heightStride, float widthStride,
                           T_Input* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("RoiPooling");

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
        uint32_t batchId = batchSplitData[roiIndex];
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

        int32_t wRoiStart = std::round(static_cast<float>(roiInfo[0] * widthScale));
        int32_t hRoiStart = std::round(static_cast<float>(roiInfo[1] * heightScale));
        int32_t wRoiEnd = std::round(static_cast<float>(roiInfo[2] * widthScale));
        int32_t hRoiEnd = std::round(static_cast<float>(roiInfo[3] * heightScale));

        // Rois with width/height < 1 are considered malformed and are forced to be 1
        T_Roi roiWidth = static_cast<T_Roi>(std::max(wRoiEnd - wRoiStart + 1, 1));
        T_Roi roiHeight = static_cast<T_Roi>(std::max(hRoiEnd - hRoiStart + 1, 1));
        T_Roi wStepSize = roiWidth / static_cast<T_Roi>(outWidth);
        T_Roi hStepSize = roiHeight / static_cast<T_Roi>(outHeight);

        const T_Input* batchBase = inputData + batchId * inHeight * inWidth * inDepth;
        for (uint32_t i = 0; i < outHeight; i++) {
            for (uint32_t j = 0; j < outWidth; j++) {
                // Take floor on start, ceil on end, start included, end excluded, i.e. [start, end)
                // end is guaranteed to larger than start by at least 1
                uint32_t wStart = std::floor(static_cast<float>(wStepSize * j + wRoiStart));
                uint32_t wEnd = std::ceil(static_cast<float>(wStepSize * (j + 1) + wRoiStart));
                uint32_t hStart = std::floor(static_cast<float>(hStepSize * i + hRoiStart));
                uint32_t hEnd = std::ceil(static_cast<float>(hStepSize * (i + 1) + hRoiStart));

                wStart = std::min(wStart, inWidth);
                wEnd = std::min(wEnd, inWidth);
                hStart = std::min(hStart, inHeight);
                hEnd = std::min(hEnd, inHeight);

                for (uint32_t k = 0; k < inDepth; k++) {
                    T_Input maxValue = static_cast<T_Input>(inputShape.offset);
                    bool first = true;
                    for (uint32_t h = hStart; h < hEnd; h++) {
                        for (uint32_t w = wStart; w < wEnd; w++) {
                            T_Input inputValue = batchBase[h * inWidth * inDepth + w * inDepth + k];
                            if (first || inputValue > maxValue) {
                                maxValue = inputValue;
                                first = false;
                            }
                        }
                    }
                    outPtr[k] = maxValue;
                }
                outPtr += inDepth;
            }
        }
    }
    return true;
}

template <typename T_Input, typename T_Roi>
inline bool roiPooling(const T_Input* inputData, const Shape& inputShape, const T_Roi* roiData,
                       const Shape& roiShape, const int32_t* batchSplitData,
                       const Shape& batchSplitShape, float heightStride, float widthStride,
                       bool useNchw, T_Input* outputData, const Shape& outputShape) {
    InputWithLayout<T_Input> input(useNchw);
    OutputWithLayout<T_Input> output(useNchw);
    NN_RET_CHECK(input.initialize(inputData, inputShape));
    NN_RET_CHECK(output.initialize(outputData, outputShape));
    NN_RET_CHECK(roiPoolingNhwc(input.getNhwcBuffer(), input.getNhwcShape(), roiData, roiShape,
                                batchSplitData, batchSplitShape, heightStride, widthStride,
                                output.getNhwcBuffer(), output.getNhwcShape()));
    NN_RET_CHECK(output.commit());
    return true;
}

template <>
inline bool roiPooling<uint8_t, uint16_t>(const uint8_t* inputData, const Shape& inputShape,
                                          const uint16_t* roiData, const Shape& roiShape,
                                          const int32_t* batchSplitData,
                                          const Shape& batchSplitShape, float heightStride,
                                          float widthStride, bool useNchw, uint8_t* outputData,
                                          const Shape& outputShape) {
    std::vector<float> roi_float32(getNumberOfElements(roiShape));
    convertQuantToFloat32(roiData, roiShape.scale, roiShape.offset, &roi_float32);
    NN_RET_CHECK(roiPooling(inputData, inputShape, roi_float32.data(), roiShape, batchSplitData,
                            batchSplitShape, heightStride, widthStride, useNchw, outputData,
                            outputShape));
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
                           OperandType::FLOAT32,        OperandType::BOOL};
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16,
                           OperandType::TENSOR_INT32,   OperandType::INT32,
                           OperandType::INT32,          OperandType::FLOAT16,
                           OperandType::FLOAT16,        OperandType::BOOL};
    } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                           OperandType::TENSOR_QUANT16_ASYMM,
                           OperandType::TENSOR_INT32,
                           OperandType::INT32,
                           OperandType::INT32,
                           OperandType::FLOAT32,
                           OperandType::FLOAT32,
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
    NN_RET_CHECK_EQ(getSizeOfDimension(roiShape, 1), 4);
    NN_RET_CHECK_EQ(getSizeOfDimension(batchSplitShape, 0), numRois);

    auto outputHeight = context->getInputValue<int32_t>(kOutputHeightScalar);
    auto outputWidth = context->getInputValue<int32_t>(kOutputWidthScalar);
    float heightStride, widthStride;
    if (context->getInputType(kInputTensor) == OperandType::TENSOR_FLOAT16) {
        heightStride = context->getInputValue<_Float16>(kHeightStrideSalar);
        widthStride = context->getInputValue<_Float16>(kWidthStrideScalar);
    } else {
        heightStride = context->getInputValue<float>(kHeightStrideSalar);
        widthStride = context->getInputValue<float>(kWidthStrideScalar);
    }
    NN_RET_CHECK_GT(outputHeight, 0);
    NN_RET_CHECK_GT(outputWidth, 0);
    NN_RET_CHECK_GT(heightStride, 0);
    NN_RET_CHECK_GT(widthStride, 0);

    if (roiShape.type == OperandType::TENSOR_QUANT16_ASYMM) {
        NN_RET_CHECK_EQ(roiShape.scale, 0.125f);
        NN_RET_CHECK_EQ(roiShape.offset, 0);
    }

    Shape output = input;
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
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return roiPooling(context->getInputBuffer<_Float16>(kInputTensor),
                              context->getInputShape(kInputTensor),
                              context->getInputBuffer<_Float16>(kRoiTensor),
                              context->getInputShape(kRoiTensor),
                              context->getInputBuffer<int32_t>(kBatchSplitTensor),
                              context->getInputShape(kBatchSplitTensor),
                              context->getInputValue<_Float16>(kHeightStrideSalar),
                              context->getInputValue<_Float16>(kWidthStrideScalar),
                              context->getInputValue<bool>(kLayoutScalar),
                              context->getOutputBuffer<_Float16>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return roiPooling(context->getInputBuffer<float>(kInputTensor),
                              context->getInputShape(kInputTensor),
                              context->getInputBuffer<float>(kRoiTensor),
                              context->getInputShape(kRoiTensor),
                              context->getInputBuffer<int32_t>(kBatchSplitTensor),
                              context->getInputShape(kBatchSplitTensor),
                              context->getInputValue<float>(kHeightStrideSalar),
                              context->getInputValue<float>(kWidthStrideScalar),
                              context->getInputValue<bool>(kLayoutScalar),
                              context->getOutputBuffer<float>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return roiPooling(context->getInputBuffer<uint8_t>(kInputTensor),
                              context->getInputShape(kInputTensor),
                              context->getInputBuffer<uint16_t>(kRoiTensor),
                              context->getInputShape(kRoiTensor),
                              context->getInputBuffer<int32_t>(kBatchSplitTensor),
                              context->getInputShape(kBatchSplitTensor),
                              context->getInputValue<float>(kHeightStrideSalar),
                              context->getInputValue<float>(kWidthStrideScalar),
                              context->getInputValue<bool>(kLayoutScalar),
                              context->getOutputBuffer<uint8_t>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace roi_pooling

NN_REGISTER_OPERATION(ROI_POOLING, roi_pooling::kOperationName, roi_pooling::validate,
                      roi_pooling::prepare, roi_pooling::execute);

}  // namespace nn
}  // namespace android
