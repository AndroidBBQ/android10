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
namespace heatmap_max_keypoint {

constexpr char kOperationName[] = "HEATMAP_MAX_KEYPOINT";

constexpr uint32_t kNumInputs = 3;
constexpr uint32_t kHeatmapTensor = 0;
constexpr uint32_t kBoxesTensor = 1;
constexpr uint32_t kLayoutScalar = 2;

constexpr uint32_t kNumOutputs = 2;
constexpr uint32_t kOutputScoreTensor = 0;
constexpr uint32_t kOutputKeypointTensor = 1;

namespace {

// This function uses Taylor expansion up to the quatratic term to approximate bicubic
// upscaling result.
// 2nd order Taylor expansion: D(x) = D - b'x + 1/2 * x'Ax
// where D = grid[1][1], Taylor expansion center, the original score,
//       x = delta, the correction on max keypoint position,
//       D(x) = deltaScore, the accuracy score after correction
static void solveForDelta(const float grid[3][3], float* delta, float* deltaScore,
                          float fpAtol = 1e-5f, float fpRtol = 1e-5f) {
    // b: negative 1st order derivative at center
    // A: Hessian matrix at center (2nd order derivative)
    float A[2][2], b[2];
    b[0] = -(grid[1][2] - grid[1][0]) / 2.0f;
    b[1] = -(grid[2][1] - grid[0][1]) / 2.0f;
    A[0][0] = grid[1][0] - 2.0f * grid[1][1] + grid[1][2];
    A[0][1] = (grid[2][2] - grid[2][0] - grid[0][2] + grid[0][0]) / 4.0f;
    A[1][0] = A[0][1];
    A[1][1] = grid[0][1] - 2.0f * grid[1][1] + grid[2][1];

    // solve Ax=b, where x=delta -> delta = inv(A) * b
    float crossProd1 = A[0][0] * A[1][1], crossProd2 = A[0][1] * A[1][0];
    float detA = crossProd1 - crossProd2;
    // check if A is invertible
    if (std::abs(detA) < (fpAtol + fpRtol * crossProd1)) return;
    delta[0] = (A[1][1] * b[0] - A[0][1] * b[1]) / detA;
    delta[1] = (A[0][0] * b[1] - A[1][0] * b[0]) / detA;

    // clip out of range delta, i.e. delta > 3/2
    if (std::abs(delta[0]) > 1.5f || std::abs(delta[1]) > 1.5f) {
        float scale = 1.5f / std::max(std::abs(delta[0]), std::abs(delta[1]));
        delta[0] *= scale;
        delta[1] *= scale;
    }

    *deltaScore = grid[1][1] - b[0] * delta[0] - b[1] * delta[1] +
                  ((A[0][0] * delta[0] + A[0][1] * delta[1]) * delta[0] +
                   (A[1][0] * delta[0] + A[1][1] * delta[1]) * delta[1]) /
                          2.0f;
}

inline bool heatmapMaxKeypointFloat32Nhwc(const float* heatmap, const Shape& heatmapShape,
                                          const float* boxes, const Shape& boxesShape,
                                          float* outputScoreData, const Shape& outputScoreShape,
                                          float* outputKeypointData,
                                          const Shape& outputKeypointShape, float fpAtol,
                                          float fpRtol) {
    NNTRACE_TRANS("HeatmapMaxKeypoint");

    uint32_t numBoxes = getSizeOfDimension(heatmapShape, 0);
    uint32_t heatmapSize = getSizeOfDimension(heatmapShape, 1);
    uint32_t numKeypoints = getSizeOfDimension(heatmapShape, 3);
    uint32_t boxInfoLength = getSizeOfDimension(boxesShape, 1);

    const float* heatmapBase = heatmap;
    const float* boxInfoBase = boxes;
    float* outputScoreBase = outputScoreData;
    float* outputKeypointBase = outputKeypointData;
    for (uint32_t i = 0; i < numBoxes; i++) {
        NN_RET_CHECK_LE(boxInfoBase[0], boxInfoBase[2]);
        NN_RET_CHECK_LE(boxInfoBase[1], boxInfoBase[3]);
        for (uint32_t j = 0; j < numKeypoints; j++) {
            // find max score and its index
            uint32_t maxIndex = 0;
            float maxScore = -FLT_MAX;
            for (uint32_t k = 0; k < heatmapSize * heatmapSize; k++) {
                float val = heatmapBase[k * numKeypoints + j];
                if (maxScore < val) {
                    maxScore = val;
                    maxIndex = k;
                }
            }

            uint32_t maxIndexWidth = maxIndex % heatmapSize;
            uint32_t maxIndexHeight = maxIndex / heatmapSize;

            // get local 3x3 grid
            float localGrid[3][3];
            for (int32_t dh = -1; dh <= 1; dh++) {
                for (int32_t dw = -1; dw <= 1; dw++) {
                    // cast uint32_t to int32_t
                    int32_t h = static_cast<int32_t>(maxIndexHeight) + dh;
                    int32_t w = static_cast<int32_t>(maxIndexWidth) + dw;

                    // use mirroring for out of bound indexing
                    // need to ensure heatmapSize >= 2
                    h = h < 0 ? 1 : (h >= heatmapSize ? heatmapSize - 2 : h);
                    w = w < 0 ? 1 : (w >= heatmapSize ? heatmapSize - 2 : w);

                    uint32_t heatmapIndex = static_cast<uint32_t>(h) * heatmapSize * numKeypoints +
                                            static_cast<uint32_t>(w) * numKeypoints + j;
                    localGrid[dh + 1][dw + 1] = heatmapBase[heatmapIndex];
                }
            }

            float delta[2] = {0.0f, 0.0f}, deltaScore = maxScore;
            solveForDelta(localGrid, delta, &deltaScore, fpAtol, fpRtol);

            float wRoiStart = boxInfoBase[0];
            float hRoiStart = boxInfoBase[1];
            float wRoiEnd = boxInfoBase[2];
            float hRoiEnd = boxInfoBase[3];
            float roiWidth = wRoiEnd - wRoiStart;
            float roiHeight = hRoiEnd - hRoiStart;
            float wRelativePos = (static_cast<float>(maxIndexWidth) + delta[0] + 0.5f) /
                                 static_cast<float>(heatmapSize);
            float hRelativePos = (static_cast<float>(maxIndexHeight) + delta[1] + 0.5f) /
                                 static_cast<float>(heatmapSize);
            *outputScoreBase++ = deltaScore;
            outputKeypointBase[0] = wRelativePos * roiWidth + wRoiStart;
            outputKeypointBase[1] = hRelativePos * roiHeight + hRoiStart;
            outputKeypointBase += 2;
        }
        boxInfoBase += boxInfoLength;
        heatmapBase += heatmapSize * heatmapSize * numKeypoints;
    }

    return true;
}

inline bool heatmapMaxKeypointFloat32(const float* heatmap, const Shape& heatmapShape,
                                      const float* boxes, const Shape& boxesShape, bool layout,
                                      float* outputScoreData, const Shape& outputScoreShape,
                                      float* outputKeypointData, const Shape& outputKeypointShape,
                                      float fpAtol, float fpRtol) {
    std::vector<float> heatmap_nhwc;
    Shape heatmapShape_nhwc;
    if (layout) {
        NN_RET_CHECK(convertNchwToNhwc(heatmap, heatmapShape, &heatmap_nhwc, &heatmapShape_nhwc));
    }
    const float* heatmap_tmp = layout ? heatmap_nhwc.data() : heatmap;
    const Shape& heatmapShape_tmp = layout ? heatmapShape_nhwc : heatmapShape;
    return heatmapMaxKeypointFloat32Nhwc(heatmap_tmp, heatmapShape_tmp, boxes, boxesShape,
                                         outputScoreData, outputScoreShape, outputKeypointData,
                                         outputKeypointShape, fpAtol, fpRtol);
}

inline bool heatmapMaxKeypointQuant(const uint8_t* heatmap, const Shape& heatmapShape,
                                    const uint16_t* boxes, const Shape& boxesShape, bool layout,
                                    uint8_t* outputScoreData, const Shape& outputScoreShape,
                                    uint16_t* outputKeypointData, const Shape& outputKeypointShape,
                                    float fpAtol, float fpRtol) {
    std::vector<float> heatmap_float32(getNumberOfElements(heatmapShape));
    convertQuantToFloat32(heatmap, heatmapShape.scale, heatmapShape.offset, &heatmap_float32);
    std::vector<float> boxes_float32(getNumberOfElements(boxesShape));
    convertQuantToFloat32(boxes, boxesShape.scale, boxesShape.offset, &boxes_float32);
    std::vector<float> outputScore_float32(getNumberOfElements(outputScoreShape));
    std::vector<float> outputKeypoint_float32(getNumberOfElements(outputKeypointShape));
    NN_RET_CHECK(heatmapMaxKeypointFloat32(
            heatmap_float32.data(), heatmapShape, boxes_float32.data(), boxesShape, layout,
            outputScore_float32.data(), outputScoreShape, outputKeypoint_float32.data(),
            outputKeypointShape, fpAtol, fpRtol));
    convertFloat32ToQuant(outputScore_float32, outputScoreShape.scale, outputScoreShape.offset,
                          outputScoreData);
    convertFloat32ToQuant(outputKeypoint_float32, outputKeypointShape.scale,
                          outputKeypointShape.offset, outputKeypointData);
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    std::vector<OperandType> inExpectedTypes;
    std::vector<OperandType> outExpectedTypes;
    auto inputType = context->getInputType(kHeatmapTensor);
    if (inputType == OperandType::TENSOR_FLOAT32 || inputType == OperandType::TENSOR_FLOAT16) {
        inExpectedTypes = {inputType, inputType, OperandType::BOOL};
        outExpectedTypes = {inputType, inputType};
    } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM, OperandType::TENSOR_QUANT16_ASYMM,
                           OperandType::BOOL};
        outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM, OperandType::TENSOR_QUANT16_ASYMM};
    } else {
        LOG(ERROR) << "Unsupported input tensor type for operation " << kOperationName;
        return false;
    }
    NN_RET_CHECK(validateInputTypes(context, inExpectedTypes));
    NN_RET_CHECK(validateOutputTypes(context, outExpectedTypes));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    bool layout = context->getInputValue<bool>(kLayoutScalar);
    Shape heatmapShape = context->getInputShape(kHeatmapTensor);
    Shape boxesShape = context->getInputShape(kBoxesTensor);
    NN_RET_CHECK_EQ(getNumberOfDimensions(heatmapShape), 4);
    NN_RET_CHECK_EQ(getNumberOfDimensions(boxesShape), 2);

    uint32_t numBoxes = getSizeOfDimension(heatmapShape, 0);
    uint32_t heatmapSize = getSizeOfDimension(heatmapShape, 2);
    uint32_t numKeypoints = getSizeOfDimension(heatmapShape, layout ? 1 : 3);
    uint32_t boxInfoLength = getSizeOfDimension(boxesShape, 1);
    NN_RET_CHECK_EQ(getSizeOfDimension(heatmapShape, layout ? 3 : 1), heatmapSize);
    NN_RET_CHECK_GE(heatmapSize, 2);
    NN_RET_CHECK_EQ(getSizeOfDimension(boxesShape, 0), numBoxes);
    NN_RET_CHECK_EQ(boxInfoLength, 4);

    if (heatmapShape.type == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK_EQ(boxesShape.scale, 0.125f);
        NN_RET_CHECK_EQ(boxesShape.offset, 0);
    }

    Shape outputScore = context->getOutputShape(kOutputScoreTensor);
    outputScore.type = heatmapShape.type;
    outputScore.dimensions = {numBoxes, numKeypoints};
    NN_RET_CHECK(context->setOutputShape(kOutputScoreTensor, outputScore));

    Shape outputKeypoint = context->getOutputShape(kOutputKeypointTensor);
    outputKeypoint.type = boxesShape.type;
    outputKeypoint.dimensions = {numBoxes, numKeypoints, 2};
    outputKeypoint.offset = 0;
    outputKeypoint.scale = 0.125f;
    NN_RET_CHECK(context->setOutputShape(kOutputKeypointTensor, outputKeypoint));
    return true;
}

bool execute(IOperationExecutionContext* context) {
    bool layout = context->getInputValue<bool>(kLayoutScalar);
    switch (context->getInputType(kHeatmapTensor)) {
        case OperandType::TENSOR_FLOAT16: {
            const auto heatmap = context->getInputBuffer<_Float16>(kHeatmapTensor);
            const auto heatmapShape = context->getInputShape(kHeatmapTensor);
            const auto boxes = context->getInputBuffer<_Float16>(kBoxesTensor);
            const auto boxesShape = context->getInputShape(kBoxesTensor);
            auto outputScoreData = context->getOutputBuffer<_Float16>(kOutputScoreTensor);
            const auto outputScoreShape = context->getOutputShape(kOutputScoreTensor);
            auto outputKeypointData = context->getOutputBuffer<_Float16>(kOutputKeypointTensor);
            const auto outputKeypointShape = context->getOutputShape(kOutputKeypointTensor);
            std::vector<float> heatmap_float32(getNumberOfElements(heatmapShape));
            convertFloat16ToFloat32(heatmap, &heatmap_float32);
            std::vector<float> boxes_float32(getNumberOfElements(boxesShape));
            convertFloat16ToFloat32(boxes, &boxes_float32);
            std::vector<float> outputScore_float32(getNumberOfElements(outputScoreShape));
            std::vector<float> outputKeypoint_float32(getNumberOfElements(outputKeypointShape));
            NN_RET_CHECK(heatmapMaxKeypointFloat32(
                    heatmap_float32.data(), heatmapShape, boxes_float32.data(), boxesShape, layout,
                    outputScore_float32.data(), outputScoreShape, outputKeypoint_float32.data(),
                    outputKeypointShape, 1e-3f, 1e-3f));
            convertFloat32ToFloat16(outputScore_float32, outputScoreData);
            convertFloat32ToFloat16(outputKeypoint_float32, outputKeypointData);
            return true;
        }
        case OperandType::TENSOR_FLOAT32: {
            return heatmapMaxKeypointFloat32(context->getInputBuffer<float>(kHeatmapTensor),
                                             context->getInputShape(kHeatmapTensor),
                                             context->getInputBuffer<float>(kBoxesTensor),
                                             context->getInputShape(kBoxesTensor), layout,
                                             context->getOutputBuffer<float>(kOutputScoreTensor),
                                             context->getOutputShape(kOutputScoreTensor),
                                             context->getOutputBuffer<float>(kOutputKeypointTensor),
                                             context->getOutputShape(kOutputKeypointTensor), 1e-5f,
                                             1e-5f);
        }
        case OperandType::TENSOR_QUANT8_ASYMM: {
            return heatmapMaxKeypointQuant(
                    context->getInputBuffer<uint8_t>(kHeatmapTensor),
                    context->getInputShape(kHeatmapTensor),
                    context->getInputBuffer<uint16_t>(kBoxesTensor),
                    context->getInputShape(kBoxesTensor), layout,
                    context->getOutputBuffer<uint8_t>(kOutputScoreTensor),
                    context->getOutputShape(kOutputScoreTensor),
                    context->getOutputBuffer<uint16_t>(kOutputKeypointTensor),
                    context->getOutputShape(kOutputKeypointTensor), 1e-5f, 1e-5f);
        }
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace heatmap_max_keypoint

NN_REGISTER_OPERATION(HEATMAP_MAX_KEYPOINT, heatmap_max_keypoint::kOperationName,
                      heatmap_max_keypoint::validate, heatmap_max_keypoint::prepare,
                      heatmap_max_keypoint::execute);

}  // namespace nn
}  // namespace android
