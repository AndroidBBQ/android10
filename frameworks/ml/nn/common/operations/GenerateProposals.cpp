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
#include <numeric>

#include "Tracing.h"

namespace android {
namespace nn {
namespace bbox_ops {

namespace {

struct BoxEncodingCorner {
    float x1, y1, x2, y2;
};
struct BoxEncodingCenter {
    float w, h, x, y;
};
BoxEncodingCorner toBoxEncodingCorner(const BoxEncodingCenter& ctr) {
    return {.x1 = ctr.x - ctr.w / 2,
            .y1 = ctr.y - ctr.h / 2,
            .x2 = ctr.x + ctr.w / 2,
            .y2 = ctr.y + ctr.h / 2};
}
BoxEncodingCenter toBoxEncodingCenter(const BoxEncodingCorner& cnr) {
    return {.w = cnr.x2 - cnr.x1,
            .h = cnr.y2 - cnr.y1,
            .x = (cnr.x1 + cnr.x2) / 2,
            .y = (cnr.y1 + cnr.y2) / 2};
}

inline bool bboxTransformFloat32(const float* roiData, const Shape& roiShape,
                                 const float* bboxDeltasData, const Shape& bboxDeltasShape,
                                 const int32_t* batchesData, const Shape& batchesShape,
                                 const float* imageInfoData, const Shape& imageInfoDataShape,
                                 float* outputData, const Shape& outputShape) {
    const uint32_t roiLength = 4;
    const uint32_t imageLength = 2;

    uint32_t numClasses = getSizeOfDimension(bboxDeltasShape, 1) / roiLength;
    uint32_t numBatches = getSizeOfDimension(imageInfoDataShape, 0);

    const float* roiDataEnd = roiData + getNumberOfElements(roiShape);
    const float* deltas = bboxDeltasData;
    float* outPtr = outputData;
    uint32_t roiIndex = 0;
    for (const float* roiBase = roiData; roiBase < roiDataEnd; roiBase += roiLength, roiIndex++) {
        uint32_t batchIndex = batchesData[roiIndex];
        // Check for malformed data
        // 1. Invalid batch id
        // 2. Invalid region: x2 < x1 || y2 < y1
        NN_RET_CHECK_GE(batchIndex, 0);
        NN_RET_CHECK_LT(batchIndex, numBatches);
        NN_RET_CHECK_LE(roiBase[0], roiBase[2]);
        NN_RET_CHECK_LE(roiBase[1], roiBase[3]);

        const float* imageInfoBase = imageInfoData + batchIndex * imageLength;
        float imageHeight = imageInfoBase[0];
        float imageWidth = imageInfoBase[1];
        auto roiBefore = toBoxEncodingCenter(
                {.x1 = roiBase[0], .y1 = roiBase[1], .x2 = roiBase[2], .y2 = roiBase[3]});
        for (uint32_t i = 0; i < numClasses; i++) {
            auto roiAfter = toBoxEncodingCorner({.w = std::exp(deltas[2]) * roiBefore.w,
                                                 .h = std::exp(deltas[3]) * roiBefore.h,
                                                 .x = roiBefore.x + deltas[0] * roiBefore.w,
                                                 .y = roiBefore.y + deltas[1] * roiBefore.h});
            BoxEncodingCorner cliped = {.x1 = std::min(std::max(roiAfter.x1, 0.0f), imageWidth),
                                        .y1 = std::min(std::max(roiAfter.y1, 0.0f), imageHeight),
                                        .x2 = std::min(std::max(roiAfter.x2, 0.0f), imageWidth),
                                        .y2 = std::min(std::max(roiAfter.y2, 0.0f), imageHeight)};
            outPtr[0] = cliped.x1;
            outPtr[1] = cliped.y1;
            outPtr[2] = cliped.x2;
            outPtr[3] = cliped.y2;
            deltas += roiLength;
            outPtr += roiLength;
        }
    }
    return true;
}

inline bool bboxTransformFloat16(const _Float16* roiData, const Shape& roiShape,
                                 const _Float16* bboxDeltasData, const Shape& bboxDeltasShape,
                                 const int32_t* batchesData, const Shape& batchesShape,
                                 const _Float16* imageInfoData, const Shape& imageInfoDataShape,
                                 _Float16* outputData, const Shape& outputShape) {
    std::vector<float> roi_float32(getNumberOfElements(roiShape));
    convertFloat16ToFloat32(roiData, &roi_float32);
    std::vector<float> delta_float32(getNumberOfElements(bboxDeltasShape));
    convertFloat16ToFloat32(bboxDeltasData, &delta_float32);
    std::vector<float> imageInfo_float32(getNumberOfElements(imageInfoDataShape));
    convertFloat16ToFloat32(imageInfoData, &imageInfo_float32);
    std::vector<float> output_float32(getNumberOfElements(outputShape));
    NN_RET_CHECK(bboxTransformFloat32(roi_float32.data(), roiShape, delta_float32.data(),
                                      bboxDeltasShape, batchesData, batchesShape,
                                      imageInfo_float32.data(), imageInfoDataShape,
                                      output_float32.data(), outputShape));
    convertFloat32ToFloat16(output_float32, outputData);
    return true;
}

inline bool bboxTransformQuant(const uint16_t* roiData, const Shape& roiShape,
                               const uint8_t* bboxDeltasData, const Shape& bboxDeltasShape,
                               const int32_t* batchesData, const Shape& batchesShape,
                               const uint16_t* imageInfoData, const Shape& imageInfoDataShape,
                               uint16_t* outputData, const Shape& outputShape) {
    std::vector<float> roi_float32(getNumberOfElements(roiShape));
    convertQuantToFloat32(roiData, roiShape.scale, roiShape.offset, &roi_float32);
    std::vector<float> delta_float32(getNumberOfElements(bboxDeltasShape));
    convertQuantToFloat32(bboxDeltasData, bboxDeltasShape.scale, bboxDeltasShape.offset,
                          &delta_float32);
    std::vector<float> imageInfo_float32(getNumberOfElements(imageInfoDataShape));
    convertQuantToFloat32(imageInfoData, imageInfoDataShape.scale, imageInfoDataShape.offset,
                          &imageInfo_float32);
    std::vector<float> output_float32(getNumberOfElements(outputShape));
    NN_RET_CHECK(bboxTransformFloat32(roi_float32.data(), roiShape, delta_float32.data(),
                                      bboxDeltasShape, batchesData, batchesShape,
                                      imageInfo_float32.data(), imageInfoDataShape,
                                      output_float32.data(), outputShape));
    convertFloat32ToQuant(output_float32, outputShape.scale, outputShape.offset, outputData);
    return true;
}

// Taking two indices of bounding boxes, return the intersection-of-union.
float getIoUAxisAligned(const float* roi1, const float* roi2) {
    const float area1 = (roi1[2] - roi1[0]) * (roi1[3] - roi1[1]);
    const float area2 = (roi2[2] - roi2[0]) * (roi2[3] - roi2[1]);
    const float x1 = std::max(roi1[0], roi2[0]);
    const float x2 = std::min(roi1[2], roi2[2]);
    const float y1 = std::max(roi1[1], roi2[1]);
    const float y2 = std::min(roi1[3], roi2[3]);
    const float w = std::max(x2 - x1, 0.0f);
    const float h = std::max(y2 - y1, 0.0f);
    const float areaIntersect = w * h;
    const float areaUnion = area1 + area2 - areaIntersect;
    return areaIntersect / areaUnion;
}

}  // namespace

namespace axis_aligned_bbox_transform {

constexpr char kOperationName[] = "AXIS_ALIGNED_BBOX_TRANSFORM";

constexpr uint32_t kNumInputs = 4;
constexpr uint32_t kRoiTensor = 0;
constexpr uint32_t kDeltaTensor = 1;
constexpr uint32_t kBatchesTensor = 2;
constexpr uint32_t kImageInfoTensor = 3;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    std::vector<OperandType> inExpectedTypes;
    auto inputType = context->getInputType(kRoiTensor);
    if (inputType == OperandType::TENSOR_FLOAT32 || inputType == OperandType::TENSOR_FLOAT16) {
        inExpectedTypes = {inputType, inputType, OperandType::TENSOR_INT32, inputType};
    } else if (inputType == OperandType::TENSOR_QUANT16_ASYMM) {
        inExpectedTypes = {OperandType::TENSOR_QUANT16_ASYMM, OperandType::TENSOR_QUANT8_ASYMM,
                           OperandType::TENSOR_INT32, OperandType::TENSOR_QUANT16_ASYMM};
    } else {
        LOG(ERROR) << "Unsupported input tensor type for operation " << kOperationName;
        return false;
    }
    NN_RET_CHECK(validateInputTypes(context, inExpectedTypes));
    NN_RET_CHECK(validateOutputTypes(context, {inputType}));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    Shape roiShape = context->getInputShape(kRoiTensor);
    Shape bboxDeltasShape = context->getInputShape(kDeltaTensor);
    Shape batchesShape = context->getInputShape(kBatchesTensor);
    Shape imageInfoShape = context->getInputShape(kImageInfoTensor);
    Shape outputShape = context->getOutputShape(kOutputTensor);

    NN_RET_CHECK_EQ(getNumberOfDimensions(roiShape), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(bboxDeltasShape), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(batchesShape), 1);
    NN_RET_CHECK_EQ(getNumberOfDimensions(imageInfoShape), 2);

    // Only numRois can be zero.
    const uint32_t kRoiDim = 4;
    uint32_t numRois = getSizeOfDimension(roiShape, 0);
    uint32_t numClasses = getSizeOfDimension(bboxDeltasShape, 1) / kRoiDim;
    uint32_t numBatches = getSizeOfDimension(imageInfoShape, 0);
    NN_RET_CHECK_GT(numClasses, 0);
    NN_RET_CHECK_GT(numBatches, 0);
    NN_RET_CHECK_EQ(getSizeOfDimension(roiShape, 1), kRoiDim);
    NN_RET_CHECK_EQ(getSizeOfDimension(bboxDeltasShape, 0), numRois);
    NN_RET_CHECK_EQ(getSizeOfDimension(bboxDeltasShape, 1), kRoiDim * numClasses);
    NN_RET_CHECK_EQ(getSizeOfDimension(batchesShape, 0), numRois);
    NN_RET_CHECK_EQ(getSizeOfDimension(imageInfoShape, 1), 2);

    if (roiShape.type == OperandType::TENSOR_QUANT16_ASYMM) {
        NN_RET_CHECK_EQ(roiShape.scale, 0.125f);
        NN_RET_CHECK_EQ(roiShape.offset, 0);
        NN_RET_CHECK_EQ(imageInfoShape.scale, 0.125f);
        NN_RET_CHECK_EQ(imageInfoShape.offset, 0);
    }

    outputShape.type = roiShape.type;
    outputShape.dimensions = {numRois, numClasses * kRoiDim};
    outputShape.scale = 0.125f;
    outputShape.offset = 0;
    NN_RET_CHECK(context->setOutputShape(kOutputTensor, outputShape));
    return true;
}

bool execute(IOperationExecutionContext* context) {
    NNTRACE_TRANS("axisAlignedBBoxTransform");
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kRoiTensor)) {
        case OperandType::TENSOR_FLOAT16: {
            return bboxTransformFloat16(context->getInputBuffer<_Float16>(kRoiTensor),
                                        context->getInputShape(kRoiTensor),
                                        context->getInputBuffer<_Float16>(kDeltaTensor),
                                        context->getInputShape(kDeltaTensor),
                                        context->getInputBuffer<int32_t>(kBatchesTensor),
                                        context->getInputShape(kBatchesTensor),
                                        context->getInputBuffer<_Float16>(kImageInfoTensor),
                                        context->getInputShape(kImageInfoTensor),
                                        context->getOutputBuffer<_Float16>(kOutputTensor),
                                        context->getOutputShape(kOutputTensor));
        }
        case OperandType::TENSOR_FLOAT32: {
            return bboxTransformFloat32(context->getInputBuffer<float>(kRoiTensor),
                                        context->getInputShape(kRoiTensor),
                                        context->getInputBuffer<float>(kDeltaTensor),
                                        context->getInputShape(kDeltaTensor),
                                        context->getInputBuffer<int32_t>(kBatchesTensor),
                                        context->getInputShape(kBatchesTensor),
                                        context->getInputBuffer<float>(kImageInfoTensor),
                                        context->getInputShape(kImageInfoTensor),
                                        context->getOutputBuffer<float>(kOutputTensor),
                                        context->getOutputShape(kOutputTensor));
        }
        case OperandType::TENSOR_QUANT16_ASYMM: {
            return bboxTransformQuant(context->getInputBuffer<uint16_t>(kRoiTensor),
                                      context->getInputShape(kRoiTensor),
                                      context->getInputBuffer<uint8_t>(kDeltaTensor),
                                      context->getInputShape(kDeltaTensor),
                                      context->getInputBuffer<int32_t>(kBatchesTensor),
                                      context->getInputShape(kBatchesTensor),
                                      context->getInputBuffer<uint16_t>(kImageInfoTensor),
                                      context->getInputShape(kImageInfoTensor),
                                      context->getOutputBuffer<uint16_t>(kOutputTensor),
                                      context->getOutputShape(kOutputTensor));
        }
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace axis_aligned_bbox_transform

namespace box_with_nms_limit {

constexpr char kOperationName[] = "BOX_WITH_NMS_LIMIT";

constexpr uint32_t kNumInputs = 9;
constexpr uint32_t kScoreTensor = 0;
constexpr uint32_t kRoiTensor = 1;
constexpr uint32_t kBatchesTensor = 2;
constexpr uint32_t kScoreThresholdScalar = 3;
constexpr uint32_t kMaxNumDetectionScalar = 4;
constexpr uint32_t kNmsKernelScalar = 5;
constexpr uint32_t kIoUThresholdScalar = 6;
constexpr uint32_t kSigmaScalar = 7;
constexpr uint32_t kNmsScoreThresholdScalar = 8;

constexpr uint32_t kNumOutputs = 4;
constexpr uint32_t kOutputScoreTensor = 0;
constexpr uint32_t kOutputRoiTensor = 1;
constexpr uint32_t kOutputClassTensor = 2;
constexpr uint32_t kOutputBatchesTensor = 3;

namespace {

// TODO(xusongw): Reduce code duplication with hard/soft nms path.

// Inplace hard NMS within range [select, select + selectLength).
uint32_t* hardNmsSingleClass(const float* scoresData, float iouThreshold, int32_t maxNumDetections,
                             std::function<const float*(uint32_t)> getRoiBase, uint32_t* select,
                             uint32_t selectLength) {
    uint32_t *selectStart = select, *selectEnd = select + selectLength, numDetections = 0;
    if (maxNumDetections < 0) {
        maxNumDetections = selectLength;
    }
    while (selectStart < selectEnd && numDetections < maxNumDetections) {
        // find max score and swap to the front
        auto& maxScore = *std::max_element(selectStart, selectEnd,
                                           [&scoresData](const uint32_t& lhs, const uint32_t& rhs) {
                                               return scoresData[lhs] < scoresData[rhs];
                                           });
        std::swap(maxScore, *selectStart);

        // Calculate IoU of the rest, swap to the end (disgard) if needed.
        for (uint32_t* i = selectStart + 1; i < selectEnd; i++) {
            float iou = getIoUAxisAligned(getRoiBase(*i), getRoiBase(*selectStart));
            if (iou >= iouThreshold) {
                std::swap(*i--, *(--selectEnd));
            }
        }
        selectStart++;
        numDetections++;
    }
    return selectStart;
}

void hardNmsMultiClass(const float* scoresData, uint32_t numClasses, uint32_t numRois,
                       float scoreThreshold, float iouThreshold, int32_t maxNumDetections,
                       int32_t maxNumDetectionsPerClass,
                       std::function<const float*(uint32_t)> getRoiBase,
                       std::vector<uint32_t>* select) {
    // Exclude class 0 (background)
    for (uint32_t c = 1; c < numClasses; c++) {
        uint32_t size = select->size();
        for (uint32_t b = 0; b < numRois; b++) {
            const uint32_t index = b * numClasses + c;
            const float score = scoresData[index];
            if (score > scoreThreshold) {
                select->push_back(index);
            }
        }
        uint32_t* selectStart = select->data() + size;
        uint32_t selectLength = select->size() - size;
        uint32_t* selectEnd = hardNmsSingleClass(scoresData, iouThreshold, maxNumDetectionsPerClass,
                                                 getRoiBase, selectStart, selectLength);
        select->resize(selectEnd - select->data());
    }

    // Take top maxNumDetections.
    std::sort(select->begin(), select->end(),
              [&scoresData](const uint32_t& lhs, const uint32_t& rhs) {
                  return scoresData[lhs] > scoresData[rhs];
              });
    if (maxNumDetections < 0 || select->size() <= maxNumDetections) {
        return;
    }
    select->resize(maxNumDetections);
}

// Inplace soft NMS within range [select, select + selectLength).
using SoftNmsKernel = std::function<float(float)>;
uint32_t* softNmsSingleClass(float* scoresData, float scoreThreshold, int32_t maxNumDetections,
                             std::function<const float*(uint32_t)> getRoiBase, SoftNmsKernel kernel,
                             uint32_t* select, uint32_t selectLength) {
    uint32_t *selectStart = select, *selectEnd = select + selectLength, numDetections = 0;
    if (maxNumDetections < 0) {
        maxNumDetections = selectLength;
    }
    while (selectStart < selectEnd && numDetections < maxNumDetections) {
        // find max score and swap to the front
        auto& maxScore = *std::max_element(selectStart, selectEnd,
                                           [&scoresData](const uint32_t& lhs, const uint32_t& rhs) {
                                               return scoresData[lhs] < scoresData[rhs];
                                           });
        std::swap(maxScore, *selectStart);

        // Calculate IoU of the rest, swap to the end (disgard) if needed.
        for (uint32_t* i = selectStart + 1; i < selectEnd; i++) {
            float iou = getIoUAxisAligned(getRoiBase(*i), getRoiBase(*selectStart));
            scoresData[*i] *= kernel(iou);
            if (scoresData[*i] < scoreThreshold) {
                std::swap(*i--, *(--selectEnd));
            }
        }
        selectStart++;
        numDetections++;
    }
    return selectStart;
}

void softNmsMultiClass(float* scoresData, uint32_t numClasses, uint32_t numRois,
                       float scoreThreshold, float nmsScoreThreshold, int32_t maxNumDetections,
                       int32_t maxNumDetectionsPerClass,
                       std::function<const float*(uint32_t)> getRoiBase, SoftNmsKernel kernel,
                       std::vector<uint32_t>* select) {
    // Exclude class 0 (background)
    for (uint32_t c = 1; c < numClasses; c++) {
        uint32_t size = select->size();
        for (uint32_t b = 0; b < numRois; b++) {
            const uint32_t index = b * numClasses + c;
            const float score = scoresData[index];
            if (score > scoreThreshold) {
                select->push_back(index);
            }
        }
        uint32_t* selectStart = select->data() + size;
        uint32_t selectLength = select->size() - size;
        uint32_t* selectEnd =
                softNmsSingleClass(scoresData, nmsScoreThreshold, maxNumDetectionsPerClass,
                                   getRoiBase, kernel, selectStart, selectLength);
        select->resize(selectEnd - select->data());
    }

    // Take top maxNumDetections.
    std::sort(select->begin(), select->end(),
              [&scoresData](const uint32_t& lhs, const uint32_t& rhs) {
                  return scoresData[lhs] > scoresData[rhs];
              });
    if (maxNumDetections < 0 || select->size() <= maxNumDetections) {
        return;
    }
    select->resize(maxNumDetections);
}

bool boxWithNmsLimitFloat32Compute(float* scoresData, const Shape& scoresShape,
                                   const float* roiData, const Shape& roiShape,
                                   const int32_t* batchesData, const Shape& batchesShape,
                                   float scoreThreshold, int32_t maxNumDetections,
                                   int32_t softNmsKernel, float iouThreshold, float sigma,
                                   float nmsScoreThreshold, std::vector<uint32_t>* batchSplitIn,
                                   std::vector<uint32_t>* batchSplitOut,
                                   std::vector<uint32_t>* selected) {
    SoftNmsKernel kernel = nullptr;
    if (softNmsKernel == 0) {
        kernel = [&iouThreshold](float iou) { return iou < iouThreshold ? 1.0f : 0.0f; };
    } else if (softNmsKernel == 1) {
        kernel = [&iouThreshold](float iou) { return iou < iouThreshold ? 1.0f : 1.0f - iou; };
    } else if (softNmsKernel == 2) {
        kernel = [&sigma](float iou) { return std::exp(-1.0f * iou * iou / sigma); };
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported soft NMS kernel " << softNmsKernel;
    }

    const uint32_t kRoiDim = 4;
    uint32_t numRois = getSizeOfDimension(scoresShape, 0);
    uint32_t numClasses = getSizeOfDimension(scoresShape, 1);

    // We assume boxes of the same batch are grouped together.
    std::vector<uint32_t> batch;
    for (uint32_t i = 0, ind = -1; i < numRois; i++) {
        if (batchesData[i] == ind) {
            (batchSplitIn->back())++;
        } else {
            ind = batchesData[i];
            batchSplitIn->push_back(1);
        }
    }

    float* scoresBase = scoresData;
    const float* roiBase = roiData;
    selected->clear();
    for (uint32_t b = 0; b < batchSplitIn->size(); b++) {
        for (uint32_t i = 0; i < batchSplitIn->at(b); i++) {
            const float* roi = roiBase + i * kRoiDim;
            // Check for malformed data: invalid region: x2 < x1 || y2 < y1
            NN_RET_CHECK_LE(roi[0], roi[2]);
            NN_RET_CHECK_LE(roi[1], roi[3]);
        }
        std::vector<uint32_t> result;
        softNmsMultiClass(scoresBase, numClasses, batchSplitIn->at(b), scoreThreshold,
                          nmsScoreThreshold, maxNumDetections, maxNumDetections,
                          [&roiBase](uint32_t ind) { return roiBase + ind * kRoiDim; }, kernel,
                          &result);
        // Sort again by class.
        std::sort(result.begin(), result.end(),
                  [&scoresBase, numClasses](const uint32_t& lhs, const uint32_t& rhs) {
                      uint32_t lhsClass = lhs % numClasses, rhsClass = rhs % numClasses;
                      return lhsClass == rhsClass ? scoresBase[lhs] > scoresBase[rhs]
                                                  : lhsClass < rhsClass;
                  });
        selected->insert(selected->end(), result.begin(), result.end());
        batchSplitOut->push_back(result.size());
        scoresBase += batchSplitIn->at(b) * numClasses;
        roiBase += batchSplitIn->at(b) * numClasses * kRoiDim;
    }
    return true;
}

template <typename T>
T castTo(float val, const Shape&) {
    return val;
}
template <>
uint8_t castTo(float val, const Shape& shape) {
    int32_t intVal = std::round(val / shape.scale + shape.offset);
    intVal = std::min<int32_t>(std::max<int32_t>(intVal, std::numeric_limits<uint8_t>::min()),
                               std::numeric_limits<uint8_t>::max());
    return static_cast<uint8_t>(intVal);
}

template <typename T_Score, typename T_Roi>
bool boxWithNmsLimitWriteOutput(const std::vector<uint32_t>& selected,
                                const std::vector<uint32_t>& batchSplitIn,
                                const std::vector<uint32_t>& batchSplitOut,
                                const std::vector<float>& scores,
                                IOperationExecutionContext* context) {
    const uint32_t kRoiDim = 4;
    Shape scoresShape = context->getInputShape(kScoreTensor);
    uint32_t numClasses = getSizeOfDimension(scoresShape, 1);

    // Set output dimensions.
    uint32_t numOutRois = selected.size();
    if (numOutRois == 0) return true;
    Shape scoresOutShape = context->getOutputShape(kOutputScoreTensor);
    scoresOutShape.dimensions = {numOutRois};
    NN_RET_CHECK(context->setOutputShape(kOutputScoreTensor, scoresOutShape));

    Shape roiOutShape = context->getOutputShape(kOutputRoiTensor);
    roiOutShape.dimensions = {numOutRois, 4};
    NN_RET_CHECK(context->setOutputShape(kOutputRoiTensor, roiOutShape));

    Shape classesOutShape = context->getOutputShape(kOutputClassTensor);
    classesOutShape.dimensions = {numOutRois};
    NN_RET_CHECK(context->setOutputShape(kOutputClassTensor, classesOutShape));

    Shape batchesOutShape = context->getOutputShape(kOutputBatchesTensor);
    batchesOutShape.dimensions = {numOutRois};
    NN_RET_CHECK(context->setOutputShape(kOutputBatchesTensor, batchesOutShape));

    // Write outputs.
    const float* scoresBase = scores.data();
    const T_Roi* roiBase = context->getInputBuffer<T_Roi>(kRoiTensor);
    const int32_t* batchesInPtr = context->getInputBuffer<int32_t>(kBatchesTensor);
    T_Score* scoresOutPtr = context->getOutputBuffer<T_Score>(kOutputScoreTensor);
    T_Roi* roiOutPtr = context->getOutputBuffer<T_Roi>(kOutputRoiTensor);
    int32_t* classesOutPtr = context->getOutputBuffer<int32_t>(kOutputClassTensor);
    int32_t* batchesOutPtr = context->getOutputBuffer<int32_t>(kOutputBatchesTensor);
    uint32_t i = 0;
    for (uint32_t b = 0; b < batchSplitOut.size(); b++) {
        for (uint32_t j = 0; j < batchSplitOut[b]; j++) {
            uint32_t index = selected[i++];
            *scoresOutPtr++ = castTo<T_Score>(scoresBase[index], scoresOutShape);
            memcpy(roiOutPtr, roiBase + index * kRoiDim, kRoiDim * sizeof(T_Roi));
            roiOutPtr += kRoiDim;
            *classesOutPtr++ = index % numClasses;
            *batchesOutPtr++ = *batchesInPtr;
        }
        scoresBase += batchSplitIn[b] * numClasses;
        roiBase += batchSplitIn[b] * numClasses * kRoiDim;
        batchesInPtr += batchSplitIn[b];
    }
    return true;
}

bool boxWithNmsLimitFloat32(const float* scoresData, const Shape& scoresShape, const float* roiData,
                            const Shape& roiShape, const int32_t* batchesData,
                            const Shape& batchesShape, float scoreThreshold,
                            int32_t maxNumDetections, int32_t softNmsKernel, float iouThreshold,
                            float sigma, float nmsScoreThreshold, float* scoresOutData,
                            Shape scoresOutShape, float* roiOutData, Shape roiOutShape,
                            int32_t* classesOutData, Shape classesOutShape, int32_t* batchesOutData,
                            const Shape& batchSplitOutShape, IOperationExecutionContext* context) {
    NNTRACE_TRANS("boxWithNmsLimit");
    std::vector<float> scores_float32(getNumberOfElements(scoresShape));
    for (uint32_t i = 0; i < scores_float32.size(); i++) {
        scores_float32[i] = scoresData[i];
    }
    std::vector<uint32_t> selected, batchSplitIn, batchSplitOut;
    NN_RET_CHECK(boxWithNmsLimitFloat32Compute(
            scores_float32.data(), scoresShape, roiData, roiShape, batchesData, batchesShape,
            scoreThreshold, maxNumDetections, softNmsKernel, iouThreshold, sigma, nmsScoreThreshold,
            &batchSplitIn, &batchSplitOut, &selected));
    return boxWithNmsLimitWriteOutput<float, float>(selected, batchSplitIn, batchSplitOut,
                                                    scores_float32, context);
}

bool boxWithNmsLimitFloat16(const _Float16* scoresData, const Shape& scoresShape,
                            const _Float16* roiData, const Shape& roiShape,
                            const int32_t* batchesData, const Shape& batchesShape,
                            _Float16 scoreThreshold, int32_t maxNumDetections,
                            int32_t softNmsKernel, _Float16 iouThreshold, _Float16 sigma,
                            _Float16 nmsScoreThreshold, _Float16* scoresOutData,
                            const Shape& scoresOutShape, _Float16* roiOutData,
                            const Shape& roiOutShape, int32_t* classesOutData,
                            const Shape& classesOutShape, int32_t* batchesOutData,
                            const Shape& batchSplitOutShape, IOperationExecutionContext* context) {
    std::vector<float> scores_float32(getNumberOfElements(scoresShape));
    convertFloat16ToFloat32(scoresData, &scores_float32);
    std::vector<float> roi_float32(getNumberOfElements(roiShape));
    convertFloat16ToFloat32(roiData, &roi_float32);
    std::vector<uint32_t> selected, batchSplitIn, batchSplitOut;
    NN_RET_CHECK(boxWithNmsLimitFloat32Compute(
            scores_float32.data(), scoresShape, roi_float32.data(), roiShape, batchesData,
            batchesShape, scoreThreshold, maxNumDetections, softNmsKernel, iouThreshold, sigma,
            nmsScoreThreshold, &batchSplitIn, &batchSplitOut, &selected));
    return boxWithNmsLimitWriteOutput<_Float16, _Float16>(selected, batchSplitIn, batchSplitOut,
                                                          scores_float32, context);
}

bool boxWithNmsLimitQuant(const uint8_t* scoresData, const Shape& scoresShape,
                          const uint16_t* roiData, const Shape& roiShape,
                          const int32_t* batchesData, const Shape& batchesShape,
                          float scoreThreshold, int32_t maxNumDetections, int32_t softNmsKernel,
                          float iouThreshold, float sigma, float nmsScoreThreshold,
                          uint8_t* scoresOutData, const Shape& scoresOutShape, uint16_t* roiOutData,
                          const Shape& roiOutShape, int32_t* classesOutData,
                          const Shape& classesOutShape, int32_t* batchesOutData,
                          const Shape& batchSplitOutShape, IOperationExecutionContext* context) {
    std::vector<float> scores_float32(getNumberOfElements(scoresShape));
    convertQuantToFloat32(scoresData, scoresShape.scale, scoresShape.offset, &scores_float32);
    std::vector<float> roi_float32(getNumberOfElements(roiShape));
    convertQuantToFloat32(roiData, roiShape.scale, roiShape.offset, &roi_float32);
    std::vector<uint32_t> selected, batchSplitIn, batchSplitOut;
    NN_RET_CHECK(boxWithNmsLimitFloat32Compute(
            scores_float32.data(), scoresShape, roi_float32.data(), roiShape, batchesData,
            batchesShape, scoreThreshold, maxNumDetections, softNmsKernel, iouThreshold, sigma,
            nmsScoreThreshold, &batchSplitIn, &batchSplitOut, &selected));
    return boxWithNmsLimitWriteOutput<uint8_t, uint16_t>(selected, batchSplitIn, batchSplitOut,
                                                         scores_float32, context);
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    std::vector<OperandType> inExpectedTypes;
    std::vector<OperandType> outExpectedTypes;
    auto inputType = context->getInputType(kScoreTensor);
    if (inputType == OperandType::TENSOR_FLOAT16) {
        inExpectedTypes = {
                OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16, OperandType::TENSOR_INT32,
                OperandType::FLOAT16,        OperandType::INT32,          OperandType::INT32,
                OperandType::FLOAT16,        OperandType::FLOAT16,        OperandType::FLOAT16};
        outExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16,
                            OperandType::TENSOR_INT32, OperandType::TENSOR_INT32};
    } else if (inputType == OperandType::TENSOR_FLOAT32) {
        inExpectedTypes = {
                OperandType::TENSOR_FLOAT32, OperandType::TENSOR_FLOAT32, OperandType::TENSOR_INT32,
                OperandType::FLOAT32,        OperandType::INT32,          OperandType::INT32,
                OperandType::FLOAT32,        OperandType::FLOAT32,        OperandType::FLOAT32};
        outExpectedTypes = {OperandType::TENSOR_FLOAT32, OperandType::TENSOR_FLOAT32,
                            OperandType::TENSOR_INT32, OperandType::TENSOR_INT32};
    } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                           OperandType::TENSOR_QUANT16_ASYMM,
                           OperandType::TENSOR_INT32,
                           OperandType::FLOAT32,
                           OperandType::INT32,
                           OperandType::INT32,
                           OperandType::FLOAT32,
                           OperandType::FLOAT32,
                           OperandType::FLOAT32};
        outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM, OperandType::TENSOR_QUANT16_ASYMM,
                            OperandType::TENSOR_INT32, OperandType::TENSOR_INT32};
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
    NN_RET_CHECK(validateInputTypes(context, inExpectedTypes));
    NN_RET_CHECK(validateOutputTypes(context, outExpectedTypes));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    Shape scoreShape = context->getInputShape(kScoreTensor);
    Shape roiShape = context->getInputShape(kRoiTensor);
    Shape batchesShape = context->getInputShape(kBatchesTensor);
    Shape outputScoreShape = context->getOutputShape(kOutputScoreTensor);
    Shape outputRoiShape = context->getOutputShape(kOutputRoiTensor);
    Shape outputClassShape = context->getOutputShape(kOutputClassTensor);
    Shape outputBatchSplitShape = context->getOutputShape(kOutputBatchesTensor);

    NN_RET_CHECK(getNumberOfDimensions(scoreShape) == 2);
    NN_RET_CHECK(getNumberOfDimensions(roiShape) == 2);
    NN_RET_CHECK(getNumberOfDimensions(batchesShape) == 1);

    // Only numRois can be zero.
    const uint32_t kRoiDim = 4;
    uint32_t numRois = getSizeOfDimension(scoreShape, 0);
    uint32_t numClasses = getSizeOfDimension(scoreShape, 1);
    NN_RET_CHECK(getSizeOfDimension(roiShape, 0) == numRois);
    NN_RET_CHECK(getSizeOfDimension(roiShape, 1) == kRoiDim * numClasses);
    NN_RET_CHECK(getSizeOfDimension(batchesShape, 0) == numRois);
    NN_RET_CHECK_GT(numClasses, 1);

    if (scoreShape.type == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK_EQ(roiShape.scale, 0.125f);
        NN_RET_CHECK_EQ(roiShape.offset, 0);
    }

    outputScoreShape.type = scoreShape.type;
    outputScoreShape.dimensions = {0};
    outputScoreShape.scale = scoreShape.scale;
    outputScoreShape.offset = scoreShape.offset;
    NN_RET_CHECK(context->setOutputShape(kOutputScoreTensor, outputScoreShape));

    outputRoiShape.type = roiShape.type;
    outputRoiShape.dimensions = {0, 4};
    outputRoiShape.scale = 0.125f;
    outputRoiShape.offset = 0;
    NN_RET_CHECK(context->setOutputShape(kOutputRoiTensor, outputRoiShape));

    outputClassShape.type = OperandType::TENSOR_INT32;
    outputClassShape.dimensions = {0};
    NN_RET_CHECK(context->setOutputShape(kOutputClassTensor, outputClassShape));

    outputBatchSplitShape.type = batchesShape.type;
    outputBatchSplitShape.dimensions = {0};
    NN_RET_CHECK(context->setOutputShape(kOutputBatchesTensor, outputBatchSplitShape));
    return true;
}

bool execute(IOperationExecutionContext* context) {
    NNTRACE_TRANS("boxWithNMSLimit");
    // Bypass execution in the case of zero numRois.
    if (getSizeOfDimension(context->getInputShape(kScoreTensor), 0) == 0) return true;
    switch (context->getInputType(kScoreTensor)) {
        case OperandType::TENSOR_FLOAT16: {
            return boxWithNmsLimitFloat16(
                    context->getInputBuffer<_Float16>(kScoreTensor),
                    context->getInputShape(kScoreTensor),
                    context->getInputBuffer<_Float16>(kRoiTensor),
                    context->getInputShape(kRoiTensor),
                    context->getInputBuffer<int32_t>(kBatchesTensor),
                    context->getInputShape(kBatchesTensor),
                    context->getInputValue<_Float16>(kScoreThresholdScalar),
                    context->getInputValue<int32_t>(kMaxNumDetectionScalar),
                    context->getInputValue<int32_t>(kNmsKernelScalar),
                    context->getInputValue<_Float16>(kIoUThresholdScalar),
                    context->getInputValue<_Float16>(kSigmaScalar),
                    context->getInputValue<_Float16>(kNmsScoreThresholdScalar),
                    context->getOutputBuffer<_Float16>(kOutputScoreTensor),
                    context->getOutputShape(kOutputScoreTensor),
                    context->getOutputBuffer<_Float16>(kOutputRoiTensor),
                    context->getOutputShape(kOutputRoiTensor),
                    context->getOutputBuffer<int32_t>(kOutputClassTensor),
                    context->getOutputShape(kOutputClassTensor),
                    context->getOutputBuffer<int32_t>(kOutputBatchesTensor),
                    context->getOutputShape(kOutputBatchesTensor), context);
        }
        case OperandType::TENSOR_FLOAT32: {
            return boxWithNmsLimitFloat32(context->getInputBuffer<float>(kScoreTensor),
                                          context->getInputShape(kScoreTensor),
                                          context->getInputBuffer<float>(kRoiTensor),
                                          context->getInputShape(kRoiTensor),
                                          context->getInputBuffer<int32_t>(kBatchesTensor),
                                          context->getInputShape(kBatchesTensor),
                                          context->getInputValue<float>(kScoreThresholdScalar),
                                          context->getInputValue<int32_t>(kMaxNumDetectionScalar),
                                          context->getInputValue<int32_t>(kNmsKernelScalar),
                                          context->getInputValue<float>(kIoUThresholdScalar),
                                          context->getInputValue<float>(kSigmaScalar),
                                          context->getInputValue<float>(kNmsScoreThresholdScalar),
                                          context->getOutputBuffer<float>(kOutputScoreTensor),
                                          context->getOutputShape(kOutputScoreTensor),
                                          context->getOutputBuffer<float>(kOutputRoiTensor),
                                          context->getOutputShape(kOutputRoiTensor),
                                          context->getOutputBuffer<int32_t>(kOutputClassTensor),
                                          context->getOutputShape(kOutputClassTensor),
                                          context->getOutputBuffer<int32_t>(kOutputBatchesTensor),
                                          context->getOutputShape(kOutputBatchesTensor), context);
        }
        case OperandType::TENSOR_QUANT8_ASYMM: {
            return boxWithNmsLimitQuant(context->getInputBuffer<uint8_t>(kScoreTensor),
                                        context->getInputShape(kScoreTensor),
                                        context->getInputBuffer<uint16_t>(kRoiTensor),
                                        context->getInputShape(kRoiTensor),
                                        context->getInputBuffer<int32_t>(kBatchesTensor),
                                        context->getInputShape(kBatchesTensor),
                                        context->getInputValue<float>(kScoreThresholdScalar),
                                        context->getInputValue<int32_t>(kMaxNumDetectionScalar),
                                        context->getInputValue<int32_t>(kNmsKernelScalar),
                                        context->getInputValue<float>(kIoUThresholdScalar),
                                        context->getInputValue<float>(kSigmaScalar),
                                        context->getInputValue<float>(kNmsScoreThresholdScalar),
                                        context->getOutputBuffer<uint8_t>(kOutputScoreTensor),
                                        context->getOutputShape(kOutputScoreTensor),
                                        context->getOutputBuffer<uint16_t>(kOutputRoiTensor),
                                        context->getOutputShape(kOutputRoiTensor),
                                        context->getOutputBuffer<int32_t>(kOutputClassTensor),
                                        context->getOutputShape(kOutputClassTensor),
                                        context->getOutputBuffer<int32_t>(kOutputBatchesTensor),
                                        context->getOutputShape(kOutputBatchesTensor), context);
        }
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace box_with_nms_limit

namespace generate_proposals {

constexpr char kOperationName[] = "GENERATE_PROPOSALS";

constexpr uint32_t kNumInputs = 11;
constexpr uint32_t kScoreTensor = 0;
constexpr uint32_t kDeltaTensor = 1;
constexpr uint32_t kAnchorTensor = 2;
constexpr uint32_t kImageInfoTensor = 3;
constexpr uint32_t kHeightStrideSalar = 4;
constexpr uint32_t kWidthStrideScalar = 5;
constexpr uint32_t kPreNmsMaxScalar = 6;
constexpr uint32_t kPostNmsMaxScalar = 7;
constexpr uint32_t kIoUThresholdScalar = 8;
constexpr uint32_t kMinSizeScalar = 9;
constexpr uint32_t kLayoutScalar = 10;

constexpr uint32_t kNumOutputs = 3;
constexpr uint32_t kOutputScoreTensor = 0;
constexpr uint32_t kOutputRoiTensor = 1;
constexpr uint32_t kOutputBatchesTensor = 2;

namespace {

void filterBoxes(const float* roiBase, const float* imageInfoBase, float minSize,
                 std::vector<uint32_t>* select) {
    const uint32_t kRoiDim = 4;
    uint32_t i = 0;
    for (uint32_t j = 0; j < select->size(); j++) {
        const float* roiInfo = roiBase + (*select)[j] * kRoiDim;
        float roiWidth, roiHeight, xRoiCenter, yRoiCenter;
        roiWidth = roiInfo[2] - roiInfo[0];
        roiHeight = roiInfo[3] - roiInfo[1];
        xRoiCenter = roiInfo[0] + roiWidth / 2.0f;
        yRoiCenter = roiInfo[1] + roiHeight / 2.0f;
        if (roiWidth > minSize && roiHeight > minSize && xRoiCenter < imageInfoBase[1] &&
            yRoiCenter < imageInfoBase[0]) {
            (*select)[i++] = (*select)[j];
        }
    }
    select->resize(i);
}

bool generateProposalsNhwcFloat32Compute(const float* scoresData, const Shape& scoresShape,
                                         const float* bboxDeltasData, const Shape& bboxDeltasShape,
                                         const float* anchorsData, const Shape& anchorsShape,
                                         const float* imageInfoData, const Shape& imageInfoShape,
                                         float heightStride, float widthStride, int32_t preNmsTopN,
                                         int32_t postNmsTopN, float iouThreshold, float minSize,
                                         std::vector<float>* scoresOutData,
                                         std::vector<float>* roiOutData,
                                         std::vector<int32_t>* batchesOutData) {
    const uint32_t kRoiDim = 4;
    uint32_t numBatches = getSizeOfDimension(scoresShape, 0);
    uint32_t height = getSizeOfDimension(scoresShape, 1);
    uint32_t width = getSizeOfDimension(scoresShape, 2);
    uint32_t numAnchors = getSizeOfDimension(scoresShape, 3);
    uint32_t imageInfoLength = getSizeOfDimension(imageInfoShape, 1);

    uint32_t batchSize = height * width * numAnchors;
    uint32_t roiBufferSize = batchSize * kRoiDim;
    std::vector<float> roiBuffer(roiBufferSize);
    std::vector<float> roiTransformedBuffer(roiBufferSize);
    scoresOutData->clear();
    roiOutData->clear();
    batchesOutData->clear();

    // Compute the roi region for each anchor.
    float* roiBase = roiBuffer.data();
    for (uint32_t h = 0; h < height; h++) {
        float hShift = h * heightStride;
        for (uint32_t w = 0; w < width; w++) {
            const float* anchorsBase = anchorsData;
            float wShift = w * widthStride;
            for (uint32_t a = 0; a < numAnchors; a++, roiBase += kRoiDim, anchorsBase += kRoiDim) {
                roiBase[0] = anchorsBase[0] + wShift;
                roiBase[1] = anchorsBase[1] + hShift;
                roiBase[2] = anchorsBase[2] + wShift;
                roiBase[3] = anchorsBase[3] + hShift;
            }
        }
    }

    const float* scoresBase = scoresData;
    const float* bboxDeltasBase = bboxDeltasData;
    const float* imageInfoBase = imageInfoData;
    // Need to fake some data to satisfy bboxTransform.
    Shape tempRoiShape = anchorsShape;
    tempRoiShape.dimensions = {batchSize, kRoiDim};
    Shape tempBBoxDeltasShape = bboxDeltasShape;
    tempBBoxDeltasShape.dimensions = {batchSize, kRoiDim};
    std::vector<int32_t> tempBatchSplitData(batchSize, 0);
    Shape tempbatchSplitShape = {.dimensions = {batchSize}};
    Shape tempImageInfoShape = imageInfoShape;
    tempImageInfoShape.dimensions = {1, imageInfoLength};

    for (uint32_t b = 0; b < numBatches; b++) {
        // Apply bboxDeltas to anchor locations.
        float tempImageInfo[] = {imageInfoBase[0], imageInfoBase[1]};
        if (!bboxTransformFloat32(roiBuffer.data(), tempRoiShape, bboxDeltasBase,
                                  tempBBoxDeltasShape, tempBatchSplitData.data(),
                                  tempbatchSplitShape, tempImageInfo, tempImageInfoShape,
                                  roiTransformedBuffer.data(), tempRoiShape)) {
            LOG(ERROR) << "BBoxTransform step failed in GENERATE_PROPOSALS op.";
            return false;
        }

        // Find the top preNmsTopN scores.
        std::vector<uint32_t> select(batchSize);
        std::iota(select.begin(), select.end(), 0);
        if (preNmsTopN > 0 && preNmsTopN < select.size()) {
            std::sort(select.begin(), select.end(),
                      [&scoresBase](const uint32_t lhs, const uint32_t rhs) {
                          return scoresBase[lhs] > scoresBase[rhs];
                      });
            select.resize(preNmsTopN);
        }

        // Filter boxes, disgard regions with height or width < minSize.
        filterBoxes(roiTransformedBuffer.data(), imageInfoBase, minSize, &select);

        // Apply hard NMS.
        uint32_t* selectEnd = box_with_nms_limit::hardNmsSingleClass(
                scoresBase, iouThreshold, postNmsTopN,
                [&roiTransformedBuffer](uint32_t ind) {
                    return roiTransformedBuffer.data() + ind * kRoiDim;
                },
                select.data(), select.size());
        uint32_t selectSize = selectEnd - select.data();
        select.resize(selectSize);

        // Write output.
        for (auto i : select) {
            roiOutData->insert(roiOutData->end(), roiTransformedBuffer.begin() + i * kRoiDim,
                               roiTransformedBuffer.begin() + (i + 1) * kRoiDim);
            scoresOutData->push_back(scoresBase[i]);
            batchesOutData->push_back(b);
        }
        scoresBase += batchSize;
        bboxDeltasBase += roiBufferSize;
        imageInfoBase += imageInfoLength;
    }
    return true;
}

bool generateProposalsFloat32Compute(const float* scoresData, const Shape& scoresShape,
                                     const float* bboxDeltasData, const Shape& bboxDeltasShape,
                                     const float* anchorsData, const Shape& anchorsShape,
                                     const float* imageInfoData, const Shape& imageInfoShape,
                                     float heightStride, float widthStride, int32_t preNmsTopN,
                                     int32_t postNmsTopN, float iouThreshold, float minSize,
                                     bool useNchw, std::vector<float>* scoresOutData,
                                     std::vector<float>* roiOutData,
                                     std::vector<int32_t>* batchesOutData) {
    InputWithLayout<float> score_nhwc(useNchw), delta_nhwc(useNchw);
    NN_RET_CHECK(score_nhwc.initialize(scoresData, scoresShape));
    NN_RET_CHECK(delta_nhwc.initialize(bboxDeltasData, bboxDeltasShape));
    return generateProposalsNhwcFloat32Compute(
            score_nhwc.getNhwcBuffer(), score_nhwc.getNhwcShape(), delta_nhwc.getNhwcBuffer(),
            delta_nhwc.getNhwcShape(), anchorsData, anchorsShape, imageInfoData, imageInfoShape,
            heightStride, widthStride, preNmsTopN, postNmsTopN, iouThreshold, minSize,
            scoresOutData, roiOutData, batchesOutData);
}

bool generateProposalsFloat32(const float* scoresData, const Shape& scoresShape,
                              const float* bboxDeltasData, const Shape& bboxDeltasShape,
                              const float* anchorsData, const Shape& anchorsShape,
                              const float* imageInfoData, const Shape& imageInfoShape,
                              float heightStride, float widthStride, int32_t preNmsTopN,
                              int32_t postNmsTopN, float iouThreshold, float minSize, bool useNchw,
                              IOperationExecutionContext* context) {
    std::vector<float> scoresOut_float32, roiOut_float32;
    std::vector<int32_t> batchesOut;
    NN_RET_CHECK(generateProposalsFloat32Compute(
            scoresData, scoresShape, bboxDeltasData, bboxDeltasShape, anchorsData, anchorsShape,
            imageInfoData, imageInfoShape, heightStride, widthStride, preNmsTopN, postNmsTopN,
            iouThreshold, minSize, useNchw, &scoresOut_float32, &roiOut_float32, &batchesOut));

    // Set output dimensions.
    uint32_t numOutRois = scoresOut_float32.size();
    if (numOutRois == 0) return true;
    Shape scoresOutShape = context->getOutputShape(kOutputScoreTensor);
    scoresOutShape.dimensions = {numOutRois};
    NN_RET_CHECK(context->setOutputShape(kOutputScoreTensor, scoresOutShape));
    Shape roiOutShape = context->getOutputShape(kOutputRoiTensor);
    roiOutShape.dimensions = {numOutRois, 4};
    NN_RET_CHECK(context->setOutputShape(kOutputRoiTensor, roiOutShape));
    Shape batchesOutShape = context->getOutputShape(kOutputBatchesTensor);
    batchesOutShape.dimensions = {numOutRois};
    NN_RET_CHECK(context->setOutputShape(kOutputBatchesTensor, batchesOutShape));

    // Write outputs.
    float* scoresOutData = context->getOutputBuffer<float>(kOutputScoreTensor);
    for (uint32_t i = 0; i < scoresOut_float32.size(); i++) {
        scoresOutData[i] = scoresOut_float32[i];
    }
    float* roiOutData = context->getOutputBuffer<float>(kOutputRoiTensor);
    for (uint32_t i = 0; i < roiOut_float32.size(); i++) {
        roiOutData[i] = roiOut_float32[i];
    }
    int32_t* batchesOutData = context->getOutputBuffer<int32_t>(kOutputBatchesTensor);
    for (uint32_t i = 0; i < batchesOut.size(); i++) {
        batchesOutData[i] = batchesOut[i];
    }
    return true;
}

bool generateProposalsFloat16(const _Float16* scoresData, const Shape& scoresShape,
                              const _Float16* bboxDeltasData, const Shape& bboxDeltasShape,
                              const _Float16* anchorsData, const Shape& anchorsShape,
                              const _Float16* imageInfoData, const Shape& imageInfoShape,
                              float heightStride, float widthStride, int32_t preNmsTopN,
                              int32_t postNmsTopN, float iouThreshold, float minSize, bool useNchw,
                              IOperationExecutionContext* context) {
    std::vector<float> score_float32(getNumberOfElements(scoresShape));
    convertFloat16ToFloat32(scoresData, &score_float32);
    std::vector<float> delta_float32(getNumberOfElements(bboxDeltasShape));
    convertFloat16ToFloat32(bboxDeltasData, &delta_float32);
    std::vector<float> anchors_float32(getNumberOfElements(anchorsShape));
    convertFloat16ToFloat32(anchorsData, &anchors_float32);
    std::vector<float> imageInfo_float32(getNumberOfElements(imageInfoShape));
    convertFloat16ToFloat32(imageInfoData, &imageInfo_float32);
    std::vector<float> scoresOut_float32, roiOut_float32;
    std::vector<int32_t> batchesOut;
    NN_RET_CHECK(generateProposalsFloat32Compute(
            score_float32.data(), scoresShape, delta_float32.data(), bboxDeltasShape,
            anchors_float32.data(), anchorsShape, imageInfo_float32.data(), imageInfoShape,
            heightStride, widthStride, preNmsTopN, postNmsTopN, iouThreshold, minSize, useNchw,
            &scoresOut_float32, &roiOut_float32, &batchesOut));

    // Set output dimensions.
    uint32_t numOutRois = scoresOut_float32.size();
    if (numOutRois == 0) return true;
    Shape scoresOutShape = context->getOutputShape(kOutputScoreTensor);
    scoresOutShape.dimensions = {numOutRois};
    NN_RET_CHECK(context->setOutputShape(kOutputScoreTensor, scoresOutShape));
    Shape roiOutShape = context->getOutputShape(kOutputRoiTensor);
    roiOutShape.dimensions = {numOutRois, 4};
    NN_RET_CHECK(context->setOutputShape(kOutputRoiTensor, roiOutShape));
    Shape batchesOutShape = context->getOutputShape(kOutputBatchesTensor);
    batchesOutShape.dimensions = {numOutRois};
    NN_RET_CHECK(context->setOutputShape(kOutputBatchesTensor, batchesOutShape));

    // Write outputs.
    _Float16* scoresOutData = context->getOutputBuffer<_Float16>(kOutputScoreTensor);
    convertFloat32ToFloat16(scoresOut_float32, scoresOutData);
    _Float16* roiOutData = context->getOutputBuffer<_Float16>(kOutputRoiTensor);
    convertFloat32ToFloat16(roiOut_float32, roiOutData);
    int32_t* batchesOutData = context->getOutputBuffer<int32_t>(kOutputBatchesTensor);
    for (uint32_t i = 0; i < batchesOut.size(); i++) {
        batchesOutData[i] = batchesOut[i];
    }
    return true;
}

bool generateProposalsQuant(const uint8_t* scoresData, const Shape& scoresShape,
                            const uint8_t* bboxDeltasData, const Shape& bboxDeltasShape,
                            const int16_t* anchorsData, const Shape& anchorsShape,
                            const uint16_t* imageInfoData, const Shape& imageInfoShape,
                            float heightStride, float widthStride, int32_t preNmsTopN,
                            int32_t postNmsTopN, float iouThreshold, float minSize, bool useNchw,
                            IOperationExecutionContext* context) {
    std::vector<float> score_float32(getNumberOfElements(scoresShape));
    convertQuantToFloat32(scoresData, scoresShape.scale, scoresShape.offset, &score_float32);
    std::vector<float> delta_float32(getNumberOfElements(bboxDeltasShape));
    convertQuantToFloat32(bboxDeltasData, bboxDeltasShape.scale, bboxDeltasShape.offset,
                          &delta_float32);
    std::vector<float> anchors_float32(getNumberOfElements(anchorsShape));
    convertQuantToFloat32(anchorsData, anchorsShape.scale, anchorsShape.offset, &anchors_float32);
    std::vector<float> imageInfo_float32(getNumberOfElements(imageInfoShape));
    convertQuantToFloat32(imageInfoData, imageInfoShape.scale, imageInfoShape.offset,
                          &imageInfo_float32);
    std::vector<float> scoresOut_float32, roiOut_float32;
    std::vector<int32_t> batchesOut;
    NN_RET_CHECK(generateProposalsFloat32Compute(
            score_float32.data(), scoresShape, delta_float32.data(), bboxDeltasShape,
            anchors_float32.data(), anchorsShape, imageInfo_float32.data(), imageInfoShape,
            heightStride, widthStride, preNmsTopN, postNmsTopN, iouThreshold, minSize, useNchw,
            &scoresOut_float32, &roiOut_float32, &batchesOut));

    // Set output dimensions.
    uint32_t numOutRois = scoresOut_float32.size();
    if (numOutRois == 0) return true;
    Shape scoresOutShape = context->getOutputShape(kOutputScoreTensor);
    scoresOutShape.dimensions = {numOutRois};
    NN_RET_CHECK(context->setOutputShape(kOutputScoreTensor, scoresOutShape));
    Shape roiOutShape = context->getOutputShape(kOutputRoiTensor);
    roiOutShape.dimensions = {numOutRois, 4};
    NN_RET_CHECK(context->setOutputShape(kOutputRoiTensor, roiOutShape));
    Shape batchesOutShape = context->getOutputShape(kOutputBatchesTensor);
    batchesOutShape.dimensions = {numOutRois};
    NN_RET_CHECK(context->setOutputShape(kOutputBatchesTensor, batchesOutShape));

    // Write outputs.
    uint8_t* scoresOutData = context->getOutputBuffer<uint8_t>(kOutputScoreTensor);
    convertFloat32ToQuant(scoresOut_float32, scoresOutShape.scale, scoresOutShape.offset,
                          scoresOutData);
    uint16_t* roiOutData = context->getOutputBuffer<uint16_t>(kOutputRoiTensor);
    convertFloat32ToQuant(roiOut_float32, roiOutShape.scale, roiOutShape.offset, roiOutData);
    int32_t* batchesOutData = context->getOutputBuffer<int32_t>(kOutputBatchesTensor);
    for (uint32_t i = 0; i < batchesOut.size(); i++) {
        batchesOutData[i] = batchesOut[i];
    }
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    std::vector<OperandType> inExpectedTypes;
    std::vector<OperandType> outExpectedTypes;
    auto inputType = context->getInputType(kScoreTensor);
    if (inputType == OperandType::TENSOR_FLOAT16) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT16,
                           OperandType::TENSOR_FLOAT16,
                           OperandType::TENSOR_FLOAT16,
                           OperandType::TENSOR_FLOAT16,
                           OperandType::FLOAT16,
                           OperandType::FLOAT16,
                           OperandType::INT32,
                           OperandType::INT32,
                           OperandType::FLOAT16,
                           OperandType::FLOAT16,
                           OperandType::BOOL};
        outExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16,
                            OperandType::TENSOR_INT32};
    } else if (inputType == OperandType::TENSOR_FLOAT32) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT32,
                           OperandType::TENSOR_FLOAT32,
                           OperandType::TENSOR_FLOAT32,
                           OperandType::TENSOR_FLOAT32,
                           OperandType::FLOAT32,
                           OperandType::FLOAT32,
                           OperandType::INT32,
                           OperandType::INT32,
                           OperandType::FLOAT32,
                           OperandType::FLOAT32,
                           OperandType::BOOL};
        outExpectedTypes = {OperandType::TENSOR_FLOAT32, OperandType::TENSOR_FLOAT32,
                            OperandType::TENSOR_INT32};
    } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                           OperandType::TENSOR_QUANT8_ASYMM,
                           OperandType::TENSOR_QUANT16_SYMM,
                           OperandType::TENSOR_QUANT16_ASYMM,
                           OperandType::FLOAT32,
                           OperandType::FLOAT32,
                           OperandType::INT32,
                           OperandType::INT32,
                           OperandType::FLOAT32,
                           OperandType::FLOAT32,
                           OperandType::BOOL};
        outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM, OperandType::TENSOR_QUANT16_ASYMM,
                            OperandType::TENSOR_INT32};
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
    NN_RET_CHECK(validateInputTypes(context, inExpectedTypes));
    NN_RET_CHECK(validateOutputTypes(context, outExpectedTypes));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    bool useNchw = context->getInputValue<bool>(kLayoutScalar);
    Shape scoreShape = context->getInputShape(kScoreTensor);
    Shape bboxDeltasShape = context->getInputShape(kDeltaTensor);
    Shape anchorsShape = context->getInputShape(kAnchorTensor);
    Shape imageInfoDataShape = context->getInputShape(kImageInfoTensor);
    Shape outputScoreShape = context->getOutputShape(kOutputScoreTensor);
    Shape outputRoiShape = context->getOutputShape(kOutputRoiTensor);
    Shape outputBatchSplitShape = context->getOutputShape(kOutputBatchesTensor);

    NN_RET_CHECK_EQ(getNumberOfDimensions(scoreShape), 4);
    NN_RET_CHECK_EQ(getNumberOfDimensions(bboxDeltasShape), 4);
    NN_RET_CHECK_EQ(getNumberOfDimensions(anchorsShape), 2);
    NN_RET_CHECK_EQ(getNumberOfDimensions(imageInfoDataShape), 2);

    const uint32_t kRoiDim = 4;
    uint32_t numBatches = getSizeOfDimension(scoreShape, 0);
    uint32_t height = getSizeOfDimension(scoreShape, useNchw ? 2 : 1);
    uint32_t width = getSizeOfDimension(scoreShape, useNchw ? 3 : 2);
    uint32_t numAnchors = getSizeOfDimension(scoreShape, useNchw ? 1 : 3);

    NN_RET_CHECK_EQ(getSizeOfDimension(bboxDeltasShape, 0), numBatches);
    NN_RET_CHECK_EQ(getSizeOfDimension(bboxDeltasShape, useNchw ? 2 : 1), height);
    NN_RET_CHECK_EQ(getSizeOfDimension(bboxDeltasShape, useNchw ? 3 : 2), width);
    NN_RET_CHECK_EQ(getSizeOfDimension(bboxDeltasShape, useNchw ? 1 : 3), numAnchors * kRoiDim);
    NN_RET_CHECK_EQ(getSizeOfDimension(imageInfoDataShape, 0), numBatches);
    NN_RET_CHECK_EQ(getSizeOfDimension(imageInfoDataShape, 1), 2);
    NN_RET_CHECK_EQ(getSizeOfDimension(anchorsShape, 0), numAnchors);
    NN_RET_CHECK_EQ(getSizeOfDimension(anchorsShape, 1), kRoiDim);

    if (scoreShape.type == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK_EQ(anchorsShape.scale, 0.125f);
        NN_RET_CHECK_EQ(imageInfoDataShape.scale, 0.125f);
        NN_RET_CHECK_EQ(imageInfoDataShape.offset, 0);
    }

    outputScoreShape.type = scoreShape.type;
    outputScoreShape.dimensions = {0};
    outputScoreShape.scale = scoreShape.scale;
    outputScoreShape.offset = scoreShape.offset;
    NN_RET_CHECK(context->setOutputShape(kOutputScoreTensor, outputScoreShape));

    outputRoiShape.dimensions = {0, 4};
    if (scoreShape.type == OperandType::TENSOR_QUANT8_ASYMM) {
        outputRoiShape.scale = 0.125f;
        outputRoiShape.offset = 0;
    }
    NN_RET_CHECK(context->setOutputShape(kOutputRoiTensor, outputRoiShape));

    outputBatchSplitShape.dimensions = {0};
    NN_RET_CHECK(context->setOutputShape(kOutputBatchesTensor, outputBatchSplitShape));
    return true;
}

bool execute(IOperationExecutionContext* context) {
    NNTRACE_TRANS("generateProposals");
    switch (context->getInputType(kScoreTensor)) {
        case OperandType::TENSOR_FLOAT16: {
            return generateProposalsFloat16(context->getInputBuffer<_Float16>(kScoreTensor),
                                            context->getInputShape(kScoreTensor),
                                            context->getInputBuffer<_Float16>(kDeltaTensor),
                                            context->getInputShape(kDeltaTensor),
                                            context->getInputBuffer<_Float16>(kAnchorTensor),
                                            context->getInputShape(kAnchorTensor),
                                            context->getInputBuffer<_Float16>(kImageInfoTensor),
                                            context->getInputShape(kImageInfoTensor),
                                            context->getInputValue<_Float16>(kHeightStrideSalar),
                                            context->getInputValue<_Float16>(kWidthStrideScalar),
                                            context->getInputValue<int32_t>(kPreNmsMaxScalar),
                                            context->getInputValue<int32_t>(kPostNmsMaxScalar),
                                            context->getInputValue<_Float16>(kIoUThresholdScalar),
                                            context->getInputValue<_Float16>(kMinSizeScalar),
                                            context->getInputValue<bool>(kLayoutScalar), context);
        }
        case OperandType::TENSOR_FLOAT32: {
            return generateProposalsFloat32(context->getInputBuffer<float>(kScoreTensor),
                                            context->getInputShape(kScoreTensor),
                                            context->getInputBuffer<float>(kDeltaTensor),
                                            context->getInputShape(kDeltaTensor),
                                            context->getInputBuffer<float>(kAnchorTensor),
                                            context->getInputShape(kAnchorTensor),
                                            context->getInputBuffer<float>(kImageInfoTensor),
                                            context->getInputShape(kImageInfoTensor),
                                            context->getInputValue<float>(kHeightStrideSalar),
                                            context->getInputValue<float>(kWidthStrideScalar),
                                            context->getInputValue<int32_t>(kPreNmsMaxScalar),
                                            context->getInputValue<int32_t>(kPostNmsMaxScalar),
                                            context->getInputValue<float>(kIoUThresholdScalar),
                                            context->getInputValue<float>(kMinSizeScalar),
                                            context->getInputValue<bool>(kLayoutScalar), context);
        }
        case OperandType::TENSOR_QUANT8_ASYMM: {
            return generateProposalsQuant(context->getInputBuffer<uint8_t>(kScoreTensor),
                                          context->getInputShape(kScoreTensor),
                                          context->getInputBuffer<uint8_t>(kDeltaTensor),
                                          context->getInputShape(kDeltaTensor),
                                          context->getInputBuffer<int16_t>(kAnchorTensor),
                                          context->getInputShape(kAnchorTensor),
                                          context->getInputBuffer<uint16_t>(kImageInfoTensor),
                                          context->getInputShape(kImageInfoTensor),
                                          context->getInputValue<float>(kHeightStrideSalar),
                                          context->getInputValue<float>(kWidthStrideScalar),
                                          context->getInputValue<int32_t>(kPreNmsMaxScalar),
                                          context->getInputValue<int32_t>(kPostNmsMaxScalar),
                                          context->getInputValue<float>(kIoUThresholdScalar),
                                          context->getInputValue<float>(kMinSizeScalar),
                                          context->getInputValue<bool>(kLayoutScalar), context);
        }
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace generate_proposals

namespace detection_postprocess {

constexpr char kOperationName[] = "DETECTION_POSTPROCESS";

constexpr uint32_t kNumInputs = 14;
constexpr uint32_t kScoreTensor = 0;
constexpr uint32_t kDeltaTensor = 1;
constexpr uint32_t kAnchorTensor = 2;
constexpr uint32_t kScaleYScalar = 3;
constexpr uint32_t kScaleXScalar = 4;
constexpr uint32_t kScaleHScalar = 5;
constexpr uint32_t kScaleWScalar = 6;
constexpr uint32_t kUseRegularNmsScalar = 7;
constexpr uint32_t kMaxNumDetectionScalar = 8;
constexpr uint32_t kMaxClassesPerDetectionScalar = 9;
constexpr uint32_t kMaxNumDetectionPerClassScalar = 10;
constexpr uint32_t kScoreThresholdScalar = 11;
constexpr uint32_t kIoUThresholdScalar = 12;
constexpr uint32_t kIsBGInLabelScalar = 13;

constexpr uint32_t kNumOutputs = 4;
constexpr uint32_t kOutputScoreTensor = 0;
constexpr uint32_t kOutputRoiTensor = 1;
constexpr uint32_t kOutputClassTensor = 2;
constexpr uint32_t kOutputDetectionTensor = 3;

namespace {

bool detectionPostprocessFloat32(
        const float* scoreData, const Shape& scoreShape, const float* deltaData,
        const Shape& deltaShape, const float* anchorData, const Shape& anchorShape, float scaleY,
        float scaleX, float scaleH, float scaleW, bool useRegularNms, int32_t maxNumDetections,
        int32_t maxClassesPerDetection, int32_t maxNumDetectionsPerClass, float iouThreshold,
        float scoreThreshold, bool isBGInLabel, float* scoreOutData, const Shape& scoreOutShape,
        float* roiOutData, const Shape& roiOutShape, int32_t* classOutData,
        const Shape& classOutShape, int32_t* detectionOutData, const Shape& detectionOutShape) {
    const uint32_t kRoiDim = 4;
    uint32_t numBatches = getSizeOfDimension(scoreShape, 0);
    uint32_t numAnchors = getSizeOfDimension(scoreShape, 1);
    uint32_t numClasses = getSizeOfDimension(scoreShape, 2);
    uint32_t lengthBoxEncoding = getSizeOfDimension(deltaShape, 2);
    uint32_t numOutDetection = getSizeOfDimension(scoreOutShape, 1);

    memset(scoreOutData, 0, getNumberOfElements(scoreOutShape) * sizeof(float));
    memset(roiOutData, 0, getNumberOfElements(roiOutShape) * sizeof(float));
    memset(classOutData, 0, getNumberOfElements(classOutShape) * sizeof(int32_t));
    memset(detectionOutData, 0, getNumberOfElements(detectionOutShape) * sizeof(int32_t));

    const float* scoreBase = scoreData;
    const float* deltaBase = deltaData;
    float* scoreOutBase = scoreOutData;
    float* roiOutBase = roiOutData;
    int32_t* classOutBase = classOutData;
    std::vector<float> roiBuffer(numAnchors * kRoiDim);
    std::vector<float> scoreBuffer(numAnchors);
    for (uint32_t b = 0; b < numBatches; b++) {
        const float* anchorBase = anchorData;
        for (uint32_t a = 0; a < numAnchors; a++) {
            float yCtr = anchorBase[0] + anchorBase[2] * deltaBase[0] / scaleY;
            float xCtr = anchorBase[1] + anchorBase[3] * deltaBase[1] / scaleX;
            float hHalf = anchorBase[2] * std::exp(deltaBase[2] / scaleH) * 0.5f;
            float wHalf = anchorBase[3] * std::exp(deltaBase[3] / scaleW) * 0.5f;
            roiBuffer[a * kRoiDim] = yCtr - hHalf;
            roiBuffer[a * kRoiDim + 1] = xCtr - wHalf;
            roiBuffer[a * kRoiDim + 2] = yCtr + hHalf;
            roiBuffer[a * kRoiDim + 3] = xCtr + wHalf;
            anchorBase += kRoiDim;
            deltaBase += lengthBoxEncoding;
        }

        if (useRegularNms) {
            std::vector<uint32_t> select;
            box_with_nms_limit::hardNmsMultiClass(
                    scoreBase, numClasses, numAnchors, scoreThreshold, iouThreshold,
                    maxNumDetections, maxNumDetectionsPerClass,
                    [&roiBuffer, numClasses](uint32_t ind) {
                        return roiBuffer.data() + (ind / numClasses) * kRoiDim;
                    },
                    &select);
            for (uint32_t i = 0; i < select.size(); i++) {
                uint32_t ind = select[i];
                scoreOutBase[i] = scoreBase[ind];
                memcpy(roiOutBase + i * kRoiDim, &roiBuffer[(ind / numClasses) * kRoiDim],
                       kRoiDim * sizeof(float));
                classOutBase[i] = (ind % numClasses) - (isBGInLabel ? 0 : 1);
            }
            *detectionOutData++ = select.size();
        } else {
            uint32_t numOutClasses = std::min<uint32_t>(numClasses - 1, maxClassesPerDetection);
            std::vector<float> maxScores(numAnchors);
            for (uint32_t a = 0; a < numAnchors; a++) {
                maxScores[a] = *std::max_element(scoreBase + a * numClasses + 1,
                                                 scoreBase + (a + 1) * numClasses);
            }
            std::vector<uint32_t> select;
            for (uint32_t a = 0; a < numAnchors; a++) {
                if (maxScores[a] > scoreThreshold) {
                    select.push_back(a);
                }
            }
            uint32_t* selectEnd = box_with_nms_limit::hardNmsSingleClass(
                    maxScores.data(), iouThreshold, maxNumDetections,
                    [&roiBuffer](uint32_t ind) { return roiBuffer.data() + ind * kRoiDim; },
                    select.data(), select.size());
            select.resize(selectEnd - select.data());
            float* scoreOutPtr = scoreOutBase;
            float* roiOutPtr = roiOutBase;
            int32_t* classOutPtr = classOutBase;
            for (auto i : select) {
                const float* score = scoreBase + i * numClasses;
                std::vector<uint32_t> scoreInds(numClasses - 1);
                std::iota(scoreInds.begin(), scoreInds.end(), 1);
                std::sort(scoreInds.begin(), scoreInds.end(),
                          [&score](const uint32_t lhs, const uint32_t rhs) {
                              return score[lhs] > score[rhs];
                          });
                for (uint32_t c = 0; c < numOutClasses; c++) {
                    *scoreOutPtr++ = score[scoreInds[c]];
                    memcpy(roiOutPtr, &roiBuffer[i * kRoiDim], kRoiDim * sizeof(float));
                    roiOutPtr += kRoiDim;
                    *classOutPtr++ = scoreInds[c] - (isBGInLabel ? 0 : 1);
                }
            }
            *detectionOutData++ = select.size() * numOutClasses;
        }
        scoreBase += numAnchors * numClasses;
        scoreOutBase += numOutDetection;
        roiOutBase += numOutDetection * kRoiDim;
        classOutBase += numOutDetection;
    }
    return true;
}

bool detectionPostprocessFloat16(
        const _Float16* scoreData, const Shape& scoreShape, const _Float16* deltaData,
        const Shape& deltaShape, const _Float16* anchorData, const Shape& anchorShape, float scaleY,
        float scaleX, float scaleH, float scaleW, bool useRegularNms, int32_t maxNumDetections,
        int32_t maxClassesPerDetection, int32_t maxNumDetectionsPerClass, float iouThreshold,
        float scoreThreshold, bool isBGInLabel, _Float16* scoreOutData, const Shape& scoreOutShape,
        _Float16* roiOutData, const Shape& roiOutShape, int32_t* classOutData,
        const Shape& classOutShape, int32_t* detectionOutData, const Shape& detectionOutShape) {
    std::vector<float> scores_float32(getNumberOfElements(scoreShape));
    convertFloat16ToFloat32(scoreData, &scores_float32);
    std::vector<float> delta_float32(getNumberOfElements(deltaShape));
    convertFloat16ToFloat32(deltaData, &delta_float32);
    std::vector<float> anchor_float32(getNumberOfElements(anchorShape));
    convertFloat16ToFloat32(anchorData, &anchor_float32);
    std::vector<float> outputScore_float32(getNumberOfElements(scoreOutShape));
    std::vector<float> outputRoi_float32(getNumberOfElements(roiOutShape));
    NN_RET_CHECK(detectionPostprocessFloat32(
            scores_float32.data(), scoreShape, delta_float32.data(), deltaShape,
            anchor_float32.data(), anchorShape, scaleY, scaleX, scaleH, scaleW, useRegularNms,
            maxNumDetections, maxClassesPerDetection, maxNumDetectionsPerClass, iouThreshold,
            scoreThreshold, isBGInLabel, outputScore_float32.data(), scoreOutShape,
            outputRoi_float32.data(), roiOutShape, classOutData, classOutShape, detectionOutData,
            detectionOutShape));
    convertFloat32ToFloat16(outputScore_float32, scoreOutData);
    convertFloat32ToFloat16(outputRoi_float32, roiOutData);
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    std::vector<OperandType> inExpectedTypes;
    std::vector<OperandType> outExpectedTypes;
    auto inputType = context->getInputType(kScoreTensor);
    if (inputType == OperandType::TENSOR_FLOAT16) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16,
                           OperandType::TENSOR_FLOAT16, OperandType::FLOAT16,
                           OperandType::FLOAT16,        OperandType::FLOAT16,
                           OperandType::FLOAT16,        OperandType::BOOL,
                           OperandType::INT32,          OperandType::INT32,
                           OperandType::INT32,          OperandType::FLOAT16,
                           OperandType::FLOAT16,        OperandType::BOOL};
    } else if (inputType == OperandType::TENSOR_FLOAT32) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT32, OperandType::TENSOR_FLOAT32,
                           OperandType::TENSOR_FLOAT32, OperandType::FLOAT32,
                           OperandType::FLOAT32,        OperandType::FLOAT32,
                           OperandType::FLOAT32,        OperandType::BOOL,
                           OperandType::INT32,          OperandType::INT32,
                           OperandType::INT32,          OperandType::FLOAT32,
                           OperandType::FLOAT32,        OperandType::BOOL};
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
    NN_RET_CHECK(validateInputTypes(context, inExpectedTypes));
    NN_RET_CHECK(validateOutputTypes(
            context, {inputType, inputType, OperandType::TENSOR_INT32, OperandType::TENSOR_INT32}));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    Shape scoreShape = context->getInputShape(kScoreTensor);
    Shape deltasShape = context->getInputShape(kDeltaTensor);
    Shape anchorsShape = context->getInputShape(kAnchorTensor);
    Shape outputScoreShape = context->getOutputShape(kOutputScoreTensor);
    Shape outputRoiShape = context->getOutputShape(kOutputRoiTensor);
    Shape outputClassShape = context->getOutputShape(kOutputClassTensor);
    Shape outputDetectionShape = context->getOutputShape(kOutputDetectionTensor);

    NN_RET_CHECK_EQ(getNumberOfDimensions(scoreShape), 3);
    NN_RET_CHECK_EQ(getNumberOfDimensions(deltasShape), 3);
    NN_RET_CHECK_EQ(getNumberOfDimensions(anchorsShape), 2);

    const uint32_t kRoiDim = 4;
    uint32_t numBatches = getSizeOfDimension(scoreShape, 0);
    uint32_t numAnchors = getSizeOfDimension(scoreShape, 1);
    uint32_t numClasses = getSizeOfDimension(scoreShape, 2);
    uint32_t lengthBoxEncoding = getSizeOfDimension(deltasShape, 2);
    uint32_t maxNumDetections = context->getInputValue<int32_t>(kMaxNumDetectionScalar);
    uint32_t maxClassesPerDetection =
            context->getInputValue<int32_t>(kMaxClassesPerDetectionScalar);
    uint32_t numOutDetections = maxNumDetections;

    NN_RET_CHECK_EQ(getSizeOfDimension(deltasShape, 0), numBatches);
    NN_RET_CHECK_EQ(getSizeOfDimension(deltasShape, 1), numAnchors);
    NN_RET_CHECK_EQ(getSizeOfDimension(anchorsShape, 0), numAnchors);
    NN_RET_CHECK_EQ(getSizeOfDimension(anchorsShape, 1), kRoiDim);

    if (scoreShape.type == OperandType::TENSOR_FLOAT32) {
        NN_RET_CHECK_GT(context->getInputValue<float>(kScaleYScalar), 0);
        NN_RET_CHECK_GT(context->getInputValue<float>(kScaleXScalar), 0);
        NN_RET_CHECK_GT(context->getInputValue<float>(kScaleHScalar), 0);
        NN_RET_CHECK_GT(context->getInputValue<float>(kScaleWScalar), 0);
        NN_RET_CHECK_GE(context->getInputValue<float>(kScoreThresholdScalar), 0);
        NN_RET_CHECK_GE(context->getInputValue<float>(kIoUThresholdScalar), 0);
    } else if (scoreShape.type == OperandType::TENSOR_FLOAT16) {
        NN_RET_CHECK(context->getInputValue<_Float16>(kScaleYScalar) > 0);
        NN_RET_CHECK(context->getInputValue<_Float16>(kScaleXScalar) > 0);
        NN_RET_CHECK(context->getInputValue<_Float16>(kScaleHScalar) > 0);
        NN_RET_CHECK(context->getInputValue<_Float16>(kScaleWScalar) > 0);
        NN_RET_CHECK(context->getInputValue<_Float16>(kScoreThresholdScalar) >= 0);
        NN_RET_CHECK(context->getInputValue<_Float16>(kIoUThresholdScalar) >= 0);
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
    NN_RET_CHECK_GT(numClasses, 1);
    NN_RET_CHECK_GE(lengthBoxEncoding, 4);
    NN_RET_CHECK_GT(maxNumDetections, 0);
    if (context->getInputValue<bool>(kUseRegularNmsScalar)) {
        NN_RET_CHECK_GT(context->getInputValue<int32_t>(kMaxNumDetectionPerClassScalar), 0);
    } else {
        NN_RET_CHECK_GT(maxClassesPerDetection, 0);
        numOutDetections *= maxClassesPerDetection;
    }

    outputScoreShape.type = scoreShape.type;
    outputScoreShape.dimensions = {numBatches, numOutDetections};
    NN_RET_CHECK(context->setOutputShape(kOutputScoreTensor, outputScoreShape));

    outputRoiShape.type = anchorsShape.type;
    outputRoiShape.dimensions = {numBatches, numOutDetections, 4};
    NN_RET_CHECK(context->setOutputShape(kOutputRoiTensor, outputRoiShape));

    outputClassShape.type = OperandType::TENSOR_INT32;
    outputClassShape.dimensions = {numBatches, numOutDetections};
    NN_RET_CHECK(context->setOutputShape(kOutputClassTensor, outputClassShape));

    outputDetectionShape.type = OperandType::TENSOR_INT32;
    outputDetectionShape.dimensions = {numBatches};
    NN_RET_CHECK(context->setOutputShape(kOutputDetectionTensor, outputDetectionShape));
    return true;
}

bool execute(IOperationExecutionContext* context) {
    NNTRACE_TRANS("detectionPostProcess");
    switch (context->getInputType(kScoreTensor)) {
        case OperandType::TENSOR_FLOAT16: {
            return detectionPostprocessFloat16(
                    context->getInputBuffer<_Float16>(kScoreTensor),
                    context->getInputShape(kScoreTensor),
                    context->getInputBuffer<_Float16>(kDeltaTensor),
                    context->getInputShape(kDeltaTensor),
                    context->getInputBuffer<_Float16>(kAnchorTensor),
                    context->getInputShape(kAnchorTensor),
                    context->getInputValue<_Float16>(kScaleYScalar),
                    context->getInputValue<_Float16>(kScaleXScalar),
                    context->getInputValue<_Float16>(kScaleHScalar),
                    context->getInputValue<_Float16>(kScaleWScalar),
                    context->getInputValue<bool>(kUseRegularNmsScalar),
                    context->getInputValue<int32_t>(kMaxNumDetectionScalar),
                    context->getInputValue<int32_t>(kMaxClassesPerDetectionScalar),
                    context->getInputValue<int32_t>(kMaxNumDetectionPerClassScalar),
                    context->getInputValue<_Float16>(kIoUThresholdScalar),
                    context->getInputValue<_Float16>(kScoreThresholdScalar),
                    context->getInputValue<bool>(kIsBGInLabelScalar),
                    context->getOutputBuffer<_Float16>(kOutputScoreTensor),
                    context->getOutputShape(kOutputScoreTensor),
                    context->getOutputBuffer<_Float16>(kOutputRoiTensor),
                    context->getOutputShape(kOutputRoiTensor),
                    context->getOutputBuffer<int32_t>(kOutputClassTensor),
                    context->getOutputShape(kOutputClassTensor),
                    context->getOutputBuffer<int32_t>(kOutputDetectionTensor),
                    context->getOutputShape(kOutputDetectionTensor));
        }
        case OperandType::TENSOR_FLOAT32: {
            return detectionPostprocessFloat32(
                    context->getInputBuffer<float>(kScoreTensor),
                    context->getInputShape(kScoreTensor),
                    context->getInputBuffer<float>(kDeltaTensor),
                    context->getInputShape(kDeltaTensor),
                    context->getInputBuffer<float>(kAnchorTensor),
                    context->getInputShape(kAnchorTensor),
                    context->getInputValue<float>(kScaleYScalar),
                    context->getInputValue<float>(kScaleXScalar),
                    context->getInputValue<float>(kScaleHScalar),
                    context->getInputValue<float>(kScaleWScalar),
                    context->getInputValue<bool>(kUseRegularNmsScalar),
                    context->getInputValue<int32_t>(kMaxNumDetectionScalar),
                    context->getInputValue<int32_t>(kMaxClassesPerDetectionScalar),
                    context->getInputValue<int32_t>(kMaxNumDetectionPerClassScalar),
                    context->getInputValue<float>(kIoUThresholdScalar),
                    context->getInputValue<float>(kScoreThresholdScalar),
                    context->getInputValue<bool>(kIsBGInLabelScalar),
                    context->getOutputBuffer<float>(kOutputScoreTensor),
                    context->getOutputShape(kOutputScoreTensor),
                    context->getOutputBuffer<float>(kOutputRoiTensor),
                    context->getOutputShape(kOutputRoiTensor),
                    context->getOutputBuffer<int32_t>(kOutputClassTensor),
                    context->getOutputShape(kOutputClassTensor),
                    context->getOutputBuffer<int32_t>(kOutputDetectionTensor),
                    context->getOutputShape(kOutputDetectionTensor));
        }
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace detection_postprocess

}  // namespace bbox_ops

NN_REGISTER_OPERATION(AXIS_ALIGNED_BBOX_TRANSFORM,
                      bbox_ops::axis_aligned_bbox_transform::kOperationName,
                      bbox_ops::axis_aligned_bbox_transform::validate,
                      bbox_ops::axis_aligned_bbox_transform::prepare,
                      bbox_ops::axis_aligned_bbox_transform::execute, .allowZeroSizedInput = true);

NN_REGISTER_OPERATION(BOX_WITH_NMS_LIMIT, bbox_ops::box_with_nms_limit::kOperationName,
                      bbox_ops::box_with_nms_limit::validate, bbox_ops::box_with_nms_limit::prepare,
                      bbox_ops::box_with_nms_limit::execute, .allowZeroSizedInput = true);

NN_REGISTER_OPERATION(GENERATE_PROPOSALS, bbox_ops::generate_proposals::kOperationName,
                      bbox_ops::generate_proposals::validate, bbox_ops::generate_proposals::prepare,
                      bbox_ops::generate_proposals::execute);

NN_REGISTER_OPERATION(DETECTION_POSTPROCESSING, bbox_ops::detection_postprocess::kOperationName,
                      bbox_ops::detection_postprocess::validate,
                      bbox_ops::detection_postprocess::prepare,
                      bbox_ops::detection_postprocess::execute);
}  // namespace nn
}  // namespace android
