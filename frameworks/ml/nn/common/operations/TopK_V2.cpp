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
#include "TopK_V2.h"

#include "OperationsUtils.h"

#include <algorithm>

namespace android {
namespace nn {
namespace topk_v2 {

namespace {

template <typename T>
bool evalGeneric(const T* inputData, const Shape& inputShape, const int32_t k, T* valuesData,
                 const Shape& /*valuesShape*/, int32_t* indicesData,
                 const Shape& /*indicesShape*/) {
    const int rowSize = inputShape.dimensions.back();
    const int totalSize = getNumberOfElements(inputShape);
    std::vector<std::pair<T, int32_t>> values(rowSize);
    T* curOutputValue = valuesData;
    int32_t* curOutputIndex = indicesData;
    for (int rowBegin = 0; rowBegin < totalSize; rowBegin += rowSize) {
        for (int i = 0; i < rowSize; ++i) {
            values[i] = std::make_pair(inputData[rowBegin + i], i);
        }
        std::nth_element(values.begin(), values.begin() + (rowSize - k), values.end());
        std::sort(values.begin() + (rowSize - k), values.end());
        std::reverse(values.begin(), values.end());
        for (int i = 0; i < k; ++i) {
            *curOutputValue = values[i].first;
            *curOutputIndex = values[i].second;
            curOutputValue++;
            curOutputIndex++;
        }
    }
    return true;
}

}  // namespace

bool prepare(const Shape& input, int32_t k, Shape* values, Shape* indices) {
    NN_CHECK(k > 0);
    NN_CHECK(k <= input.dimensions.back());

    values->dimensions = input.dimensions;
    values->dimensions.back() = k;
    indices->dimensions = input.dimensions;
    indices->dimensions.back() = k;
    return true;
}

bool eval(const void* inputData, const Shape& inputShape, const int32_t k, void* valuesData,
          const Shape& valuesShape, void* indicesData, const Shape& indicesShape) {
    switch (inputShape.type) {
        case OperandType::TENSOR_FLOAT16: {
            return evalGeneric(reinterpret_cast<const _Float16*>(inputData), inputShape, k,
                               reinterpret_cast<_Float16*>(valuesData), valuesShape,
                               reinterpret_cast<int32_t*>(indicesData), indicesShape);
        } break;
        case OperandType::TENSOR_FLOAT32: {
            return evalGeneric(reinterpret_cast<const float*>(inputData), inputShape, k,
                               reinterpret_cast<float*>(valuesData), valuesShape,
                               reinterpret_cast<int32_t*>(indicesData), indicesShape);
        } break;
        case OperandType::TENSOR_INT32: {
            return evalGeneric(reinterpret_cast<const int32_t*>(inputData), inputShape, k,
                               reinterpret_cast<int32_t*>(valuesData), valuesShape,
                               reinterpret_cast<int32_t*>(indicesData), indicesShape);
        } break;
        case OperandType::TENSOR_QUANT8_ASYMM: {
            return evalGeneric(reinterpret_cast<const uint8_t*>(inputData), inputShape, k,
                               reinterpret_cast<uint8_t*>(valuesData), valuesShape,
                               reinterpret_cast<int32_t*>(indicesData), indicesShape);
        } break;
        default: {
            LOG(ERROR) << "Unsupported data type: " << toString(inputShape.type);
            return false;
        }
    }
}

}  // namespace topk_v2
}  // namespace nn
}  // namespace android
