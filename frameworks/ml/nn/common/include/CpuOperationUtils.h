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

#ifndef ANDROID_ML_NN_COMMON_CPU_OPERATION_UTILS_H
#define ANDROID_ML_NN_COMMON_CPU_OPERATION_UTILS_H

#include "OperationsUtils.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "tensorflow/lite/kernels/internal/types.h"

namespace android {
namespace nn {

// The implementations in tflite/kernels/internal/ take a Dims<4> object
// even if the original tensors were not 4D.
inline tflite::Dims<4> convertShapeToDims(const Shape& shape) {
    nnAssert(shape.dimensions.size() <= 4);
    tflite::Dims<4> dims;

    // The dimensions are reversed in Dims<4>.
    for (int i = 0; i < 4; ++i) {
        int src = static_cast<int>(shape.dimensions.size()) - i - 1;
        if (src >= 0) {
            dims.sizes[i] = static_cast<int>(getSizeOfDimension(shape, src));
        } else {
            dims.sizes[i] = 1;
        }
    }

    dims.strides[0] = 1;
    for (int i = 1; i < 4; i++) {
        dims.strides[i] = dims.strides[i - 1] * dims.sizes[i - 1];
    }
    return dims;
}

inline tflite::RuntimeShape convertShapeToTflshape(const Shape& shape) {
    nnAssert(shape.dimensions.size() <= 4);

    std::vector<int32_t> tflShapeDim(shape.dimensions.begin(), shape.dimensions.end());
    return tflite::RuntimeShape(tflShapeDim.size(), tflShapeDim.data());
}

inline void convertFloat16ToFloat32(const _Float16* input, std::vector<float>* output) {
    CHECK(input != nullptr);
    CHECK(output != nullptr);
    for (int i = 0; i < output->size(); ++i) {
        (*output)[i] = static_cast<float>(input[i]);
    }
}

inline void convertFloat32ToFloat16(const std::vector<float>& input, _Float16* output) {
    CHECK(output != nullptr);
    for (int i = 0; i < input.size(); ++i) {
        output[i] = input[i];
    }
}

template <typename T>
inline void convertQuantToFloat32(const T* input, float scale, int32_t zeroPoint,
                                  std::vector<float>* output) {
    CHECK(input != nullptr);
    CHECK(output != nullptr);
    for (int i = 0; i < output->size(); ++i) {
        (*output)[i] = (static_cast<float>(input[i]) - zeroPoint) * scale;
    }
}

template <typename T>
inline void convertFloat32ToQuant(const std::vector<float>& input, float scale, int32_t zeroPoint,
                                  T* output) {
    CHECK(output != nullptr);
    for (int i = 0; i < input.size(); ++i) {
        int32_t intVal = std::round(input[i] / scale + zeroPoint);
        intVal = std::min<int32_t>(std::max<int32_t>(intVal, std::numeric_limits<T>::min()),
                                   std::numeric_limits<T>::max());
        output[i] = static_cast<T>(intVal);
    }
}

template <typename T>
inline bool convertNchwToNhwc(const T* nchw, const Shape& nchwShape, std::vector<T>* nhwc,
                              Shape* nhwcShape) {
    NN_RET_CHECK_EQ(getNumberOfDimensions(nchwShape), 4)
            << "Error converting a non-4-D tensor to NHWC layout";
    *nhwcShape = nchwShape;
    const auto& fromDim = nchwShape.dimensions;
    nhwcShape->dimensions = {fromDim[0], fromDim[2], fromDim[3], fromDim[1]};
    nhwc->resize(getNumberOfElements(nchwShape));
    auto to = nhwc->data();
    uint32_t spatialSize = fromDim[2] * fromDim[3];
    for (uint32_t n = 0; n < fromDim[0]; n++) {
        for (uint32_t hw = 0; hw < spatialSize; hw++) {
            for (uint32_t c = 0; c < fromDim[1]; c++) {
                uint32_t fromIndex = n * fromDim[1] * spatialSize + c * spatialSize + hw;
                *to++ = nchw[fromIndex];
            }
        }
    }
    return true;
}

template <typename T>
inline bool convertNhwcToNchw(const std::vector<T>& nhwc, const Shape& nhwcShape, T* nchw) {
    NN_RET_CHECK_EQ(getNumberOfDimensions(nhwcShape), 4)
            << "Error converting a non-4-D tensor to NCHW layout";
    const auto& fromDim = nhwcShape.dimensions;
    const auto from = nhwc.data();
    uint32_t spatialSize = fromDim[1] * fromDim[2];
    for (uint32_t n = 0; n < fromDim[0]; n++) {
        for (uint32_t c = 0; c < fromDim[3]; c++) {
            for (uint32_t hw = 0; hw < spatialSize; hw++) {
                uint32_t fromIndex = n * spatialSize * fromDim[3] + hw * fromDim[3] + c;
                *nchw++ = from[fromIndex];
            }
        }
    }
    return true;
}

template <typename T>
class InputWithLayout {
   public:
    InputWithLayout(bool useNchw) : mDataOriginal(nullptr), mUseNchw(useNchw) {}

    bool initialize(const T* data, const Shape& shape) {
        mDataOriginal = data;
        mShape = shape;
        if (mUseNchw) {
            return convertNchwToNhwc(mDataOriginal, shape, &mDataNhwc, &mShape);
        }
        return true;
    }

    const T* getNhwcBuffer() { return mUseNchw ? mDataNhwc.data() : mDataOriginal; }
    const Shape& getNhwcShape() { return mShape; }

   private:
    const T* mDataOriginal;
    std::vector<T> mDataNhwc;
    Shape mShape;
    bool mUseNchw;
};

template <typename T>
class OutputWithLayout {
   public:
    OutputWithLayout(bool useNchw) : mDataOriginal(nullptr), mUseNchw(useNchw) {}

    bool initialize(T* data, const Shape& shape) {
        NN_RET_CHECK_EQ(getNumberOfDimensions(shape), 4);
        mDataOriginal = data;
        mShape = shape;
        if (mUseNchw) {
            const auto& dim = shape.dimensions;
            mShape.dimensions = {dim[0], dim[2], dim[3], dim[1]};
            mDataNhwc.resize(getNumberOfElements(shape));
        }
        return true;
    }

    T* getNhwcBuffer() { return mUseNchw ? mDataNhwc.data() : mDataOriginal; }
    const Shape& getNhwcShape() { return mShape; }
    bool commit() {
        if (mUseNchw) {
            return convertNhwcToNchw(mDataNhwc, mShape, mDataOriginal);
        }
        return true;
    }

   private:
    T* mDataOriginal;
    std::vector<T> mDataNhwc;
    Shape mShape;
    bool mUseNchw;
};

}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_COMMON_CPU_OPERATION_UTILS_H
