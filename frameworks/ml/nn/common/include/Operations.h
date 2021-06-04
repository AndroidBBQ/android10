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

#ifndef ANDROID_ML_NN_COMMON_OPERATIONS_H
#define ANDROID_ML_NN_COMMON_OPERATIONS_H

#include "operations/BidirectionalSequenceLSTM.h"
#include "operations/Cast.h"
#include "operations/EmbeddingLookup.h"
#include "operations/ExpandDims.h"
#include "operations/HashtableLookup.h"
#include "operations/LSHProjection.h"
#include "operations/LSTM.h"
#include "operations/MaximumMinimum.h"
#include "operations/Multinomial.h"
#include "operations/Pow.h"
#include "operations/QuantizedLSTM.h"
#include "operations/RNN.h"
#include "operations/SVDF.h"
#include "operations/Tile.h"
#include "operations/TopK_V2.h"

#include <stddef.h>

#include <cstdint>
#include <vector>

namespace android {
namespace nn {

struct Shape;

bool floorFloat16(const _Float16* inputData, _Float16* outputData, const Shape& shape);
bool floorFloat32(const float* inputData, float* outputData, const Shape& shape);

bool depthwiseConvFloat16(const _Float16* inputData, const Shape& inputShape,
                          const _Float16* filterData, const Shape& filterShape,
                          const _Float16* biasData, const Shape& biasShape, int32_t paddingLeft,
                          int32_t paddingRight, int32_t paddingTop, int32_t paddingBottom,
                          int32_t strideWidth, int32_t strideHeight, int32_t dilationWidthFactor,
                          int32_t dilationHeightFactor, int32_t depthMultiplier, int32_t activation,
                          _Float16* outputData, const Shape& outputShape);
bool depthwiseConvFloat32(const float* inputData, const Shape& inputShape, const float* filterData,
                          const Shape& filterShape, const float* biasData, const Shape& biasShape,
                          int32_t paddingLeft, int32_t paddingRight, int32_t paddingTop,
                          int32_t paddingBottom, int32_t strideWidth, int32_t strideHeight,
                          int32_t dilationWidthFactor, int32_t dilationHeightFactor,
                          int32_t depthMultiplier, int32_t activation, float* outputData,
                          const Shape& outputShape);
bool depthwiseConvQuant8(const uint8_t* inputData, const Shape& inputShape,
                         const uint8_t* filterData, const Shape& filterShape,
                         const int32_t* biasData, const Shape& biasShape, int32_t paddingLeft,
                         int32_t paddingRight, int32_t paddingTop, int32_t paddingBottom,
                         int32_t strideWidth, int32_t strideHeight, int32_t dilationWidthFactor,
                         int32_t dilationHeightFactor, int32_t depthMultiplier, int32_t activation,
                         uint8_t* outputData, const Shape& outputShape);
bool depthwiseConvQuant8PerChannel(const uint8_t* inputData, const Shape& inputShape,
                                   const int8_t* filterData, const Shape& filterShape,
                                   const float* filterScales, const int32_t* biasData,
                                   const Shape& biasShape, int32_t paddingLeft,
                                   int32_t paddingRight, int32_t paddingTop, int32_t paddingBottom,
                                   int32_t strideWidth, int32_t strideHeight,
                                   int32_t dilationWidthFactor, int32_t dilationHeightFactor,
                                   int32_t depthMultiplier, int32_t activation, uint8_t* outputData,
                                   const Shape& outputShape);

bool localResponseNormFloat16(const _Float16* inputData, const Shape& inputShape, int32_t radius,
                              float bias, float alpha, float beta, int32_t axis,
                              _Float16* outputData, const Shape& outputShape);
bool localResponseNormFloat32(const float* inputData, const Shape& inputShape, int32_t radius,
                              float bias, float alpha, float beta, int32_t axis, float* outputData,
                              const Shape& outputShape);

bool copyData(const void* inputData, const Shape& inputShape, void* outputData,
              const Shape& outputShape);

template <typename T>
bool depthToSpaceGeneric(const T* inputData, const Shape& inputShape, int32_t blockSize,
                         T* outputData, const Shape& outputShape);
template <typename T>
bool spaceToDepthGeneric(const T* inputData, const Shape& inputShape, int32_t blockSize,
                         T* outputData, const Shape& outputShape);

template <typename T>
bool padGeneric(const T* inputData, const Shape& inputShape, const int32_t* paddings, T pad_value,
                T* outputData, const Shape& outputShape);

template <typename T>
bool batchToSpaceGeneric(const T* inputData, const Shape& inputShape, const int32_t* blockSize,
                         T* outputData, const Shape& outputShape);

template <typename T>
bool spaceToBatchGeneric(const T* inputData, const Shape& inputShape, const int32_t* blockSize,
                         const int32_t* padding, const Shape& paddingShape, T* outputData,
                         const Shape& outputShape);

bool meanFloat16(_Float16* inputData, const Shape& inputShape, const int32_t* axis,
                 const Shape& axisShape, bool keepDims, _Float16* outputData,
                 const Shape& outputShape);
template <typename T, typename U>
bool meanGeneric(T* inputData, const Shape& inputShape, const int32_t* axis, const Shape& axisShape,
                 bool keepDims, T* outputData, const Shape& outputShape);

bool stridedSliceGeneric(const uint8_t* inputData, const Shape& inputShape,
                         const int32_t* beginData, const int32_t* endData,
                         const int32_t* stridesData, int32_t beginMask, int32_t endMask,
                         int32_t shrinkAxisMask, uint8_t* outputData, const Shape& outputShape);

bool argMinMaxGeneric(const uint8_t* inputData, const Shape& inputShape, int32_t axis,
                      bool isArgMin, uint8_t* outputData, const Shape& outputShape);

bool splitFloat16(const _Float16* inputData, const Shape& inputShape, int32_t axis,
                  const std::vector<_Float16*>* outputDataPtrs,
                  const std::vector<Shape>& outputShapes);

bool splitFloat32(const float* inputData, const Shape& inputShape, const int32_t axis,
                  const std::vector<float*>* outputDataPtrs,
                  const std::vector<Shape>& outputShapes);

bool splitInt32(const int32_t* inputData, const Shape& inputShape, const int32_t axis,
                const std::vector<int32_t*>* outputDataPtrs,
                const std::vector<Shape>& outputShapes);

bool splitQuant8(const uint8_t* inputData, const Shape& inputShape, const int32_t axis,
                 const std::vector<uint8_t*>* outputDataPtrs,
                 const std::vector<Shape>& outputShapes);

bool groupedConvFloat16(const _Float16* inputData, const Shape& inputShape,
                        const _Float16* filterData, const Shape& filterShape,
                        const _Float16* biasData, const Shape& biasShape, int32_t numGroups,
                        int32_t padding_left, int32_t padding_right, int32_t padding_top,
                        int32_t padding_bottom, int32_t stride_width, int32_t stride_height,
                        int32_t activation, _Float16* outputData, const Shape& outputShape);

bool groupedConvFloat32(const float* inputData, const Shape& inputShape, const float* filterData,
                        const Shape& filterShape, const float* biasData, const Shape& biasShape,
                        int32_t numGroups, int32_t padding_left, int32_t padding_right,
                        int32_t padding_top, int32_t padding_bottom, int32_t stride_width,
                        int32_t stride_height, int32_t activation, float* outputData,
                        const Shape& outputShape);

bool groupedConvQuant8(const uint8_t* inputData, const Shape& inputShape, const uint8_t* filterData,
                       const Shape& filterShape, const int32_t* biasData, const Shape& biasShape,
                       int32_t numGroups, int32_t padding_left, int32_t padding_right,
                       int32_t padding_top, int32_t padding_bottom, int32_t stride_width,
                       int32_t stride_height, int32_t activation, uint8_t* outputData,
                       const Shape& outputShape);

bool groupedConvQuant8PerChannel(const uint8_t* inputData, const Shape& inputShape,
                                 const int8_t* filterData, const Shape& filterShape,
                                 const float* filterScales, const int32_t* biasData,
                                 const Shape& biasShape, int32_t padding_left,
                                 int32_t padding_right, int32_t padding_top, int32_t padding_bottom,
                                 int32_t stride_width, int32_t stride_height, int32_t numGroups,
                                 int32_t activation, uint8_t* outputData, const Shape& outputShape);

bool channelShuffleGeneric(const uint8_t* inputData, const Shape& inputShape, int32_t numGroups,
                           int32_t axis, uint8_t* outputData, const Shape& outputShape);
}  // namespace nn
}  // namespace android
#endif  // ANDROID_ML_NN_COMMON_OPERATIONS_H
