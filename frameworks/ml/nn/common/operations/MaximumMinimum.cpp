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

#define LOG_TAG "Operations"

#include "MaximumMinimum.h"
#include "IndexedShapeWrapper.h"
#include "OperationsUtils.h"
#include "Tracing.h"

namespace android {
namespace nn {
namespace maximum_minimum {

namespace {

template <typename T>
bool evalGeneric(const T* aData, const Shape& aShape, const T* bData, const Shape& bShape,
                 bool isMinimum, T* outputData, const Shape& outputShape) {
    IndexedShapeWrapper aShapeIndexed(aShape);
    IndexedShapeWrapper bShapeIndexed(bShape);
    IndexedShapeWrapper outputShapeIndexed(outputShape);

    std::vector<uint32_t> curIndex(outputShape.dimensions.size(), 0);
    bool lastIndex = false;
    do {
        uint32_t outputFlatIndex;
        NN_CHECK(outputShapeIndexed.indexToFlatIndex(curIndex, &outputFlatIndex));
        uint32_t aFlatIndex;
        NN_CHECK(aShapeIndexed.broadcastedIndexToFlatIndex(curIndex, &aFlatIndex));
        uint32_t bFlatIndex;
        NN_CHECK(bShapeIndexed.broadcastedIndexToFlatIndex(curIndex, &bFlatIndex));

        outputData[outputFlatIndex] = isMinimum ? std::min(aData[aFlatIndex], bData[bFlatIndex])
                                                : std::max(aData[aFlatIndex], bData[bFlatIndex]);

        NN_CHECK(outputShapeIndexed.nextIndexInplace(&curIndex, &lastIndex));
    } while (!lastIndex);

    return true;
}

bool evalQuant8(const uint8_t* aData, const Shape& aShape, const uint8_t* bData,
                const Shape& bShape, bool isMinimum, uint8_t* outputData,
                const Shape& outputShape) {
    IndexedShapeWrapper aShapeIndexed(aShape);
    IndexedShapeWrapper bShapeIndexed(bShape);
    IndexedShapeWrapper outputShapeIndexed(outputShape);

    std::vector<uint32_t> curIndex(outputShape.dimensions.size(), 0);
    bool lastIndex = false;
    do {
        uint32_t outputFlatIndex;
        NN_CHECK(outputShapeIndexed.indexToFlatIndex(curIndex, &outputFlatIndex));
        uint32_t aFlatIndex;
        NN_CHECK(aShapeIndexed.broadcastedIndexToFlatIndex(curIndex, &aFlatIndex));
        uint32_t bFlatIndex;
        NN_CHECK(bShapeIndexed.broadcastedIndexToFlatIndex(curIndex, &bFlatIndex));

        uint8_t aValue = requantize(aData[aFlatIndex], aShape, outputShape);
        uint8_t bValue = requantize(bData[bFlatIndex], bShape, outputShape);
        outputData[outputFlatIndex] =
                isMinimum ? std::min(aValue, bValue) : std::max(aValue, bValue);

        NN_CHECK(outputShapeIndexed.nextIndexInplace(&curIndex, &lastIndex));
    } while (!lastIndex);

    return true;
}

}  // namespace

bool prepare(const Shape& in1, const Shape& in2, Shape* out) {
    NN_CHECK(in1.type == in2.type);
    return calculateBroadcastedShape(in1, in2, out);
}

bool eval(const void* in1, const Shape& shape1, const void* in2, const Shape& shape2,
          bool isMinimum, void* output, const Shape& outputShape) {
    NNTRACE_COMP("maximum_minimum::eval");
    switch (shape1.type) {
        case OperandType::TENSOR_FLOAT16: {
            return evalGeneric(reinterpret_cast<const _Float16*>(in1), shape1,
                               reinterpret_cast<const _Float16*>(in2), shape2, isMinimum,
                               reinterpret_cast<_Float16*>(output), outputShape);
        }
        case OperandType::TENSOR_FLOAT32: {
            return evalGeneric(reinterpret_cast<const float*>(in1), shape1,
                               reinterpret_cast<const float*>(in2), shape2, isMinimum,
                               reinterpret_cast<float*>(output), outputShape);
        }
        case OperandType::TENSOR_INT32: {
            return evalGeneric(reinterpret_cast<const int32_t*>(in1), shape1,
                               reinterpret_cast<const int32_t*>(in2), shape2, isMinimum,
                               reinterpret_cast<int32_t*>(output), outputShape);
        }
        case OperandType::TENSOR_QUANT8_ASYMM: {
            return evalQuant8(reinterpret_cast<const uint8_t*>(in1), shape1,
                              reinterpret_cast<const uint8_t*>(in2), shape2, isMinimum,
                              reinterpret_cast<uint8_t*>(output), outputShape);
        }
        default: {
            LOG(ERROR) << "Unsupported data type: " << toString(shape1.type);
            return false;
        }
    }
}

}  // namespace maximum_minimum
}  // namespace nn
}  // namespace android
