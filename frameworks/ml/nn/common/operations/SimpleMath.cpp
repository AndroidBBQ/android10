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
#include "tensorflow/lite/kernels/internal/reference/legacy_reference_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {

bool floorFloat16(const _Float16* inputData, _Float16* outputData, const Shape& shape) {
    NNTRACE_TRANS("floorFloat16");
    std::vector<float> inputDataFloat32(getNumberOfElements(shape));
    convertFloat16ToFloat32(inputData, &inputDataFloat32);

    std::vector<float> outputDataFloat32(getNumberOfElements(shape));
    floorFloat32(inputDataFloat32.data(), outputDataFloat32.data(), shape);
    convertFloat32ToFloat16(outputDataFloat32, outputData);
    return true;
}

bool floorFloat32(const float* inputData, float* outputData, const Shape& shape) {
    NNTRACE_TRANS("floorFloat32");
    tflite::Dims<4> dim = convertShapeToDims(shape);
    NNTRACE_COMP_SWITCH("optimized_ops::Floor");
    tflite::optimized_ops::Floor(inputData, dim, outputData, dim);
    return true;
}

bool meanFloat16(_Float16* inputData, const Shape& inputShape, const int32_t* axis,
                 const Shape& axisShape, bool keepDims, _Float16* outputData,
                 const Shape& outputShape) {
    NNTRACE_TRANS("meanFloat16");
    std::vector<float> inputDataFloat32(getNumberOfElements(inputShape));
    convertFloat16ToFloat32(inputData, &inputDataFloat32);

    std::vector<float> outputDataFloat32(getNumberOfElements(outputShape));
    meanGeneric<float, float>(inputDataFloat32.data(), inputShape, axis, axisShape, keepDims,
                              outputDataFloat32.data(), outputShape);
    convertFloat32ToFloat16(outputDataFloat32, outputData);
    return true;
}

template <typename T, typename U>
bool meanGeneric(T* inputData, const Shape& inputShape, const int32_t* axis, const Shape& axisShape,
                 bool keepDims, T* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("meanGeneric");
    // Creates a temp index to iterate through input data.
    int32_t* scratchBuffer = new int32_t[getNumberOfDimensions(inputShape)];

    // Creates a temp tensor to store resolved axis given input data.
    int32_t axisSize = static_cast<int32_t>(getSizeOfDimension(axisShape, 0));
    int32_t* resolvedAxis = new int32_t[axisSize];

    bool result = true;
    U* tempSumBuffer = new (std::nothrow) U[getNumberOfElements(outputShape)];
    if (!tempSumBuffer) {
        LOG(ERROR) << "Failed to allocate tempSumBuffer for MEAN";
        result = false;
    } else {
        NNTRACE_COMP_SWITCH("optimized_ops::Mean");
        tflite::reference_ops::Mean<T, U>(
                inputData, reinterpret_cast<const int*>(inputShape.dimensions.data()),
                getNumberOfDimensions(inputShape), outputData,
                reinterpret_cast<const int*>(outputShape.dimensions.data()),
                getNumberOfDimensions(outputShape), axis, axisSize, keepDims, scratchBuffer,
                resolvedAxis, tempSumBuffer);
        delete[] tempSumBuffer;
    }
    delete[] scratchBuffer;
    delete[] resolvedAxis;
    return result;
}
template bool meanGeneric<float, float>(float* inputData, const Shape& inputShape,
                                        const int32_t* axis, const Shape& axisShape, bool keepDims,
                                        float* outputData, const Shape& outputShape);
template bool meanGeneric<uint8_t, int32_t>(uint8_t* inputData, const Shape& inputShape,
                                            const int32_t* axis, const Shape& axisShape,
                                            bool keepDims, uint8_t* outputData,
                                            const Shape& outputShape);

}  // namespace nn
}  // namespace android
