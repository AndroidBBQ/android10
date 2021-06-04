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

// Contains the implementation of the operations.

#define LOG_TAG "Operations"

#include "CpuOperationUtils.h"
#include "Operations.h"

#include "tensorflow/lite/kernels/internal/optimized/legacy_optimized_ops.h"
#include "tensorflow/lite/kernels/internal/reference/reference_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {

bool copyData(const void* inputData, const Shape& inputShape, void* outputData,
              const Shape& outputShape) {
    NNTRACE_COMP("copyData");
    size_t count = nonExtensionOperandSizeOfData(inputShape.type, inputShape.dimensions);
    memcpy(outputData, inputData, count);
    return true;
}

template <typename T>
bool depthToSpaceGeneric(const T* inputData, const Shape& inputShape, int32_t blockSize,
                         T* outputData, const Shape& outputShape) {
    NNTRACE_COMP("optimized_ops::DepthToSpace");
    tflite::optimized_ops::DepthToSpace(inputData, convertShapeToDims(inputShape), blockSize,
                                        outputData, convertShapeToDims(outputShape));
    return true;
}
template bool depthToSpaceGeneric<float>(const float* inputData, const Shape& inputShape,
                                         int32_t blockSize, float* outputData,
                                         const Shape& outputShape);
template bool depthToSpaceGeneric<_Float16>(const _Float16* inputData, const Shape& inputShape,
                                            int32_t blockSize, _Float16* outputData,
                                            const Shape& outputShape);
template bool depthToSpaceGeneric<uint8_t>(const uint8_t* inputData, const Shape& inputShape,
                                           int32_t blockSize, uint8_t* outputData,
                                           const Shape& outputShape);

template <typename T>
bool spaceToDepthGeneric(const T* inputData, const Shape& inputShape, int32_t blockSize,
                         T* outputData, const Shape& outputShape) {
    NNTRACE_COMP("optimized_ops::SpaceToDepth");
    tflite::optimized_ops::SpaceToDepth(inputData, convertShapeToDims(inputShape), blockSize,
                                        outputData, convertShapeToDims(outputShape));
    return true;
}
template bool spaceToDepthGeneric<float>(const float* inputData, const Shape& inputShape,
                                         int32_t blockSize, float* outputData,
                                         const Shape& outputShape);
template bool spaceToDepthGeneric<_Float16>(const _Float16* inputData, const Shape& inputShape,
                                            int32_t blockSize, _Float16* outputData,
                                            const Shape& outputShape);
template bool spaceToDepthGeneric<uint8_t>(const uint8_t* inputData, const Shape& inputShape,
                                           int32_t blockSize, uint8_t* outputData,
                                           const Shape& outputShape);

template <typename T>
bool padGeneric(const T* inputData, const Shape& inputShape, const int32_t* paddings, T padValue,
                T* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("padGeneric");

    // Based on
    // http://google3/third_party/tensorflow/contrib/lite/kernels/internal/optimized/optimized_ops.h?l=6194&rcl=213557260

    // TFLite runtime calls are currently fixed at 4 dimensions. Copy inputs so
    // we can pad them to 4 dims (yes, we are "padding the padding").
    int32_t numInputDims = static_cast<int32_t>(getNumberOfDimensions(inputShape));
    NN_OPS_CHECK(numInputDims <= 4);
    std::vector<int> leftPaddings(4 - numInputDims, 0);
    std::vector<int> rightPaddings(4 - numInputDims, 0);
    for (int32_t i = 0; i < numInputDims; ++i) {
        leftPaddings.push_back(paddings[i * 2]);
        rightPaddings.push_back(paddings[i * 2 + 1]);
    }
    const int leftBPadding = leftPaddings[0];
    const int leftHPadding = leftPaddings[1];
    const int leftWPadding = leftPaddings[2];
    const int leftDPadding = leftPaddings[3];
    const int rightBPadding = rightPaddings[0];
    const int rightHPadding = rightPaddings[1];
    const int rightWPadding = rightPaddings[2];
    const int rightDPadding = rightPaddings[3];

    const auto extInputShape =
            tflite::RuntimeShape::ExtendedShape(4, convertShapeToTflshape(inputShape));
    const auto extOutputShape =
            tflite::RuntimeShape::ExtendedShape(4, convertShapeToTflshape(outputShape));

    const int outputBatch = extOutputShape.Dims(0);
    const int outputHeight = extOutputShape.Dims(1);
    const int outputWidth = extOutputShape.Dims(2);
    const int outputDepth = extOutputShape.Dims(3);

    const int inputDepth = extInputShape.Dims(3);

    NNTRACE_COMP_SWITCH("padGeneric");

    if (leftBPadding != 0) {
        tflite::optimized_ops::TypedMemset<T>(
                outputData, padValue, leftBPadding * outputHeight * outputWidth * outputDepth);
    }
    for (int outB = leftBPadding; outB < outputBatch - rightBPadding; ++outB) {
        if (leftHPadding != 0) {
            tflite::optimized_ops::TypedMemset<T>(
                    outputData + tflite::Offset(extOutputShape, outB, 0, 0, 0), padValue,
                    leftHPadding * outputWidth * outputDepth);
        }
        for (int outH = leftHPadding; outH < outputHeight - rightHPadding; ++outH) {
            if (leftWPadding != 0) {
                tflite::optimized_ops::TypedMemset<T>(
                        outputData + tflite::Offset(extOutputShape, outB, outH, 0, 0), padValue,
                        leftWPadding * outputDepth);
            }
            for (int outW = leftWPadding; outW < outputWidth - rightWPadding; ++outW) {
                if (leftDPadding != 0) {
                    tflite::optimized_ops::TypedMemset<T>(
                            outputData + tflite::Offset(extOutputShape, outB, outH, outW, 0),
                            padValue, leftDPadding);
                }

                T* out =
                        outputData + tflite::Offset(extOutputShape, outB, outH, outW, leftDPadding);
                const T* in =
                        inputData + tflite::Offset(extInputShape, outB - leftBPadding,
                                                   outH - leftHPadding, outW - leftWPadding, 0);
                memcpy(out, in, inputDepth * sizeof(T));

                if (rightDPadding != 0) {
                    tflite::optimized_ops::TypedMemset<T>(
                            outputData + tflite::Offset(extOutputShape, outB, outH, outW,
                                                        outputDepth - rightDPadding),
                            padValue, rightDPadding);
                }
            }
            if (rightWPadding != 0) {
                tflite::optimized_ops::TypedMemset<T>(
                        outputData + tflite::Offset(extOutputShape, outB, outH,
                                                    outputWidth - rightWPadding, 0),
                        padValue, rightWPadding * outputDepth);
            }
        }
        if (rightHPadding != 0) {
            tflite::optimized_ops::TypedMemset<T>(
                    outputData + tflite::Offset(extOutputShape, outB, outputHeight - rightHPadding,
                                                0, 0),
                    padValue, rightHPadding * outputWidth * outputDepth);
        }
    }
    if (rightBPadding != 0) {
        tflite::optimized_ops::TypedMemset<T>(
                outputData + tflite::Offset(extOutputShape, outputBatch - rightBPadding, 0, 0, 0),
                padValue, rightBPadding * outputHeight * outputWidth * outputDepth);
    }

    return true;
}
template bool padGeneric<float>(const float* inputData, const Shape& inputShape,
                                const int32_t* paddings, float padValue, float* outputData,
                                const Shape& outputShape);
template bool padGeneric<_Float16>(const _Float16* inputData, const Shape& inputShape,
                                   const int32_t* paddings, _Float16 padValue, _Float16* outputData,
                                   const Shape& outputShape);
template bool padGeneric<uint8_t>(const uint8_t* inputData, const Shape& inputShape,
                                  const int32_t* paddings, uint8_t padValue, uint8_t* outputData,
                                  const Shape& outputShape);

template <typename T>
bool batchToSpaceGeneric(const T* inputData, const Shape& inputShape, const int32_t* blockSize,
                         T* outputData, const Shape& outputShape) {
    // Needed by low level implementation, but not really used.
    tflite::Dims<4> blockSizeDim, cropsDim;
    const int32 crops[4] = {0, 0, 0, 0};
    NNTRACE_COMP("optimized_ops::BatchToSpaceND");
    tflite::optimized_ops::BatchToSpaceND(inputData, convertShapeToDims(inputShape), blockSize,
                                          blockSizeDim, crops, cropsDim, outputData,
                                          convertShapeToDims(outputShape));
    return true;
}
template bool batchToSpaceGeneric<float>(const float* inputData, const Shape& inputShape,
                                         const int32_t* blockSize, float* outputData,
                                         const Shape& outputShape);
template bool batchToSpaceGeneric<_Float16>(const _Float16* inputData, const Shape& inputShape,
                                            const int32_t* blockSize, _Float16* outputData,
                                            const Shape& outputShape);
template bool batchToSpaceGeneric<uint8_t>(const uint8_t* inputData, const Shape& inputShape,
                                           const int32_t* blockSize, uint8_t* outputData,
                                           const Shape& outputShape);

template <typename T>
bool spaceToBatchGeneric(const T* inputData, const Shape& inputShape, const int32_t* blockSize,
                         const int32_t* padding, const Shape& paddingShape, T* outputData,
                         const Shape& outputShape) {
    // Needed by low level implementation, but not really used.
    tflite::RuntimeShape blockSizeDim;
    NNTRACE_COMP("optimized_ops::SpaceToBatchND");
    tflite::optimized_ops::SpaceToBatchND(
            {.output_offset = outputShape.offset}, convertShapeToTflshape(inputShape), inputData,
            blockSizeDim, blockSize, convertShapeToTflshape(paddingShape), padding,
            convertShapeToTflshape(outputShape), outputData);
    return true;
}
template bool spaceToBatchGeneric<float>(const float* inputData, const Shape& inputShape,
                                         const int32_t* blockSize, const int32_t* padding,
                                         const Shape& paddingShape, float* outputData,
                                         const Shape& outputShape);
template bool spaceToBatchGeneric<_Float16>(const _Float16* inputData, const Shape& inputShape,
                                            const int32_t* blockSize, const int32_t* padding,
                                            const Shape& paddingShape, _Float16* outputData,
                                            const Shape& outputShape);
template bool spaceToBatchGeneric<uint8_t>(const uint8_t* inputData, const Shape& inputShape,
                                           const int32_t* blockSize, const int32_t* padding,
                                           const Shape& paddingShape, uint8_t* outputData,
                                           const Shape& outputShape);

}  // namespace nn
}  // namespace android
