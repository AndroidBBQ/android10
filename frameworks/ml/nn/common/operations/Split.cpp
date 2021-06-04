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

#include "Operations.h"
#include "OperationsUtils.h"

#include "Tracing.h"

namespace android {
namespace nn {

template <typename Scalar>
bool splitGeneric(const Scalar* inputData, const Shape& inputShape, int32_t axis,
                  const std::vector<Scalar*>* outputDataPtrs,
                  const std::vector<Shape>& outputShapes) {
    NN_CHECK(handleNegativeAxis(inputShape, &axis));
    int outerSize = 1;
    for (int i = 0; i < axis; ++i) {
        outerSize *= inputShape.dimensions[i];
    }
    int baseInnerSize = 1;
    int concatDimensions = getNumberOfDimensions(inputShape);
    for (int i = axis + 1; i < concatDimensions; ++i) {
        baseInnerSize *= inputShape.dimensions[i];
    }

    const Scalar* inputPtr = inputData;
    for (int k = 0; k < outerSize; k++) {
        for (int i = 0; i < outputDataPtrs->size(); ++i) {
            const int copySize = outputShapes[i].dimensions[axis] * baseInnerSize;
            memcpy(outputDataPtrs->at(i) + k * copySize, inputPtr, copySize * sizeof(Scalar));
            inputPtr += copySize;
        }
    }

    return true;
}

bool splitFloat16(const _Float16* inputData, const Shape& inputShape, int32_t axis,
                  const std::vector<_Float16*>* outputDataPtrs,
                  const std::vector<Shape>& outputShapes) {
    NNTRACE_COMP("splitFloat16");
    return splitGeneric<_Float16>(inputData, inputShape, axis, outputDataPtrs, outputShapes);
}

bool splitFloat32(const float* inputData, const Shape& inputShape, int32_t axis,
                  const std::vector<float*>* outputDataPtrs,
                  const std::vector<Shape>& outputShapes) {
    NNTRACE_COMP("splitFloat32");
    return splitGeneric<float>(inputData, inputShape, axis, outputDataPtrs, outputShapes);
}

bool splitQuant8(const uint8_t* inputData, const Shape& inputShape, int32_t axis,
                 const std::vector<uint8_t*>* outputDataPtrs,
                 const std::vector<Shape>& outputShapes) {
    NNTRACE_COMP("splitQuant8");
    return splitGeneric<uint8_t>(inputData, inputShape, axis, outputDataPtrs, outputShapes);
}

bool splitInt32(const int32_t* inputData, const Shape& inputShape, int32_t axis,
                const std::vector<int32_t*>* outputDataPtrs,
                const std::vector<Shape>& outputShapes) {
    NNTRACE_COMP("splitInt32");
    return splitGeneric<int32_t>(inputData, inputShape, axis, outputDataPtrs, outputShapes);
}

}  // namespace nn
}  // namespace android
