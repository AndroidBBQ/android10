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

// Contains the implementation of the operations.

#define LOG_TAG "Operations"

#include "CpuOperationUtils.h"
#include "Operations.h"

#include "tensorflow/lite/kernels/internal/reference/legacy_reference_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {

bool stridedSliceGeneric(const uint8_t* inputData, const Shape& inputShape,
                         const int32_t* beginData, const int32_t* endData,
                         const int32_t* stridesData, int32_t beginMask, int32_t endMask,
                         int32_t shrinkAxisMask, uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("stridedSliceGeneric");
    // This Op only supports 1-4D cases and since we use the reference 4D
    // implementation, the 1-3D tensors are mapped to 4D.
    const int kMaxDim = 4;

    std::vector<int> starts;
    std::vector<int> stops;
    std::vector<int> strides;

    int32_t numInputDims = static_cast<int32_t>(getNumberOfDimensions(inputShape));
    for (int32_t idx = numInputDims - 1; idx >= 0; --idx) {
        starts.emplace_back(beginData[idx]);
        stops.emplace_back(endData[idx]);
        strides.emplace_back(stridesData[idx]);
    }

    for (int i = numInputDims; i < kMaxDim; i++) {
        starts.emplace_back(0);
        stops.emplace_back(1);
        strides.emplace_back(1);
    }

    beginMask = ReverseMaskBits(beginMask, numInputDims);
    endMask = ReverseMaskBits(endMask, numInputDims);
    shrinkAxisMask = ReverseMaskBits(shrinkAxisMask, numInputDims);

    if (inputShape.type == OperandType::TENSOR_FLOAT32) {
        NNTRACE_COMP_SWITCH("reference_ops::StridedSlice::float");
        tflite::reference_ops::StridedSlice(
                reinterpret_cast<const float*>(inputData), convertShapeToDims(inputShape),
                beginMask, endMask, shrinkAxisMask, starts, stops, strides,
                reinterpret_cast<float*>(outputData), convertShapeToDims(outputShape));
    } else if (inputShape.type == OperandType::TENSOR_FLOAT16) {
        NNTRACE_COMP_SWITCH("reference_ops::StridedSlice::float16");
        tflite::reference_ops::StridedSlice(
                reinterpret_cast<const _Float16*>(inputData), convertShapeToDims(inputShape),
                beginMask, endMask, shrinkAxisMask, starts, stops, strides,
                reinterpret_cast<_Float16*>(outputData), convertShapeToDims(outputShape));
    } else if (inputShape.type == OperandType::TENSOR_QUANT8_ASYMM) {
        NNTRACE_COMP_SWITCH("reference_ops::StridedSlice::uint8");
        tflite::reference_ops::StridedSlice(
                reinterpret_cast<const uint8_t*>(inputData), convertShapeToDims(inputShape),
                beginMask, endMask, shrinkAxisMask, starts, stops, strides,
                reinterpret_cast<uint8_t*>(outputData), convertShapeToDims(outputShape));
    } else {
        LOG(ERROR) << "Unsupported data type";
        return false;
    }

    return true;
}

}  // namespace nn
}  // namespace android
