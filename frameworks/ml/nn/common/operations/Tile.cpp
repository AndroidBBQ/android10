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

#include "Tile.h"
#include "Tracing.h"

namespace android {
namespace nn {
namespace tile {

namespace {

template <typename T>
void CopyMultipleTimes(const T* in_data, int32_t in_size, int32_t multiplier, T* out_data) {
    for (int i = 0; i < multiplier; ++i) {
        const T* in_end = in_data + in_size;
        T* new_out_data = std::copy(in_data, in_end, out_data);
        in_data = out_data;
        out_data = new_out_data;
    }
}

template <typename T, typename M>
std::pair<int, int> TileOneDimension(const Shape& input_shape, const T* in_data,
                                     const M* multipliers, T* out_data, int dimension) {
    const int dimension_size = input_shape.dimensions[dimension];
    if (dimension == input_shape.dimensions.size() - 1) {
        CopyMultipleTimes(in_data, dimension_size, multipliers[dimension], out_data);
        return std::make_pair(dimension_size,
                              dimension_size * static_cast<int>(multipliers[dimension]));
    }
    int total_stride_size = 0, total_tiled_stride_size = 0;
    const T* copy_from_data = in_data;
    T* copy_to_data = out_data;
    for (int i = 0; i < dimension_size; ++i) {
        int stride_size = 0, tiled_stride_size = 0;
        std::tie(stride_size, tiled_stride_size) = TileOneDimension(
                input_shape, copy_from_data, multipliers, copy_to_data, dimension + 1);
        copy_from_data += stride_size;
        copy_to_data += tiled_stride_size;
        total_stride_size += stride_size;
        total_tiled_stride_size += tiled_stride_size;
    }
    CopyMultipleTimes(out_data, total_tiled_stride_size, multipliers[dimension] - 1,
                      out_data + total_tiled_stride_size);
    return std::make_pair(total_stride_size, total_tiled_stride_size * multipliers[dimension]);
}

template <typename T>
void tileImpl(const T* inputData, const Shape& inputShape, const int32_t* multiples, T* outputData,
              const Shape& outputShape) {
    TileOneDimension(inputShape, inputData, multiples, outputData, 0);
}

}  // namespace

bool prepare(const Shape& input, const int32_t* multiples, const Shape& multiplesShape,
             Shape* output) {
    output->type = input.type;
    output->offset = input.offset;
    output->scale = input.scale;

    output->dimensions.assign(input.dimensions.begin(), input.dimensions.end());
    for (size_t i = 0; i < output->dimensions.size(); ++i) {
        output->dimensions[i] *= multiples[i];
    }

    return true;
}

bool eval(const uint8_t* inputData, const Shape& inputShape, const int32_t* multiples,
          uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("tile::eval");
#define ANDROID_NN_IMPL_TILE(operandType, dataType)                                   \
    case operandType: {                                                               \
        NNTRACE_COMP_SWITCH("tileImpl::" #dataType);                                  \
        tileImpl(reinterpret_cast<const dataType*>(inputData), inputShape, multiples, \
                 reinterpret_cast<dataType*>(outputData), outputShape);               \
        return true;                                                                  \
    }

    switch (inputShape.type) {
        ANDROID_NN_IMPL_TILE(OperandType::TENSOR_FLOAT16, _Float16);
        ANDROID_NN_IMPL_TILE(OperandType::TENSOR_FLOAT32, float);
        ANDROID_NN_IMPL_TILE(OperandType::TENSOR_INT32, int32_t);
        ANDROID_NN_IMPL_TILE(OperandType::TENSOR_QUANT8_ASYMM, uint8_t);
        default:
            LOG(ERROR) << "Unsupported data type";
            return false;
    }
#undef ANDROID_NN_IMPL_TILE
}

}  // namespace tile
}  // namespace nn
}  // namespace android
