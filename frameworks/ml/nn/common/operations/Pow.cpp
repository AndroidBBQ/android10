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

#include "Pow.h"
#include "IndexedShapeWrapper.h"
#include "OperationsUtils.h"

#include <cmath>

namespace android {
namespace nn {
namespace pow {

namespace {

template <typename T>
bool evalGeneric(const T* baseData, const Shape& baseShape, const T* exponentData,
                 const Shape& exponentShape, T* outputData, const Shape& outputShape) {
    IndexedShapeWrapper baseShapeIndexed(baseShape);
    IndexedShapeWrapper exponentShapeIndexed(exponentShape);
    IndexedShapeWrapper outputShapeIndexed(outputShape);

    std::vector<uint32_t> curIndex(outputShape.dimensions.size(), 0);
    bool lastIndex = false;
    do {
        uint32_t outputFlatIndex;
        NN_CHECK(outputShapeIndexed.indexToFlatIndex(curIndex, &outputFlatIndex));
        uint32_t baseFlatIndex;
        NN_CHECK(baseShapeIndexed.broadcastedIndexToFlatIndex(curIndex, &baseFlatIndex));
        uint32_t exponentFlatIndex;
        NN_CHECK(exponentShapeIndexed.broadcastedIndexToFlatIndex(curIndex, &exponentFlatIndex));

        outputData[outputFlatIndex] = std::pow(static_cast<float>(baseData[baseFlatIndex]),
                                               static_cast<float>(exponentData[exponentFlatIndex]));

        NN_CHECK(outputShapeIndexed.nextIndexInplace(&curIndex, &lastIndex));
    } while (!lastIndex);

    return true;
}

}  // namespace

bool prepare(const Shape& baseShape, const Shape& exponentShape, Shape* output) {
    NN_OPS_CHECK(baseShape.type == exponentShape.type);
    if (SameShape(baseShape, exponentShape)) {
        return SetShape(baseShape, output);
    }
    return calculateBroadcastedShape(baseShape, exponentShape, output);
}

bool eval(const void* baseData, const Shape& baseShape, const void* exponentData,
          const Shape& exponentShape, void* outputData, const Shape& outputShape) {
    switch (baseShape.type) {
        case OperandType::TENSOR_FLOAT16: {
            return evalGeneric(reinterpret_cast<const _Float16*>(baseData), baseShape,
                               reinterpret_cast<const _Float16*>(exponentData), exponentShape,
                               reinterpret_cast<_Float16*>(outputData), outputShape);
        } break;
        case OperandType::TENSOR_FLOAT32: {
            return evalGeneric(reinterpret_cast<const float*>(baseData), baseShape,
                               reinterpret_cast<const float*>(exponentData), exponentShape,
                               reinterpret_cast<float*>(outputData), outputShape);
        } break;
        default: {
            LOG(ERROR) << "Unsupported data type: " << toString(baseShape.type);
            return false;
        }
    }
}

}  // namespace pow
}  // namespace nn
}  // namespace android
