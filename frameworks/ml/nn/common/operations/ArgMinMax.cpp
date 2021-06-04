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

#include "Operations.h"
#include "CpuOperationUtils.h"

#include "Tracing.h"

namespace android {
namespace nn {

template <typename In, typename Out>
static void argMinMaxImpl(const In* inputData, const Shape& inputShape,
                          int32_t axis, bool isArgMin,
                          Out* outputData, const Shape& outputShape) {
    const int outerSize = getNumberOfElements(inputShape, 0, axis);
    const int axisSize = getSizeOfDimension(inputShape, axis);
    const int innerSize = getNumberOfElements(
            inputShape, axis + 1, getNumberOfDimensions(inputShape));
    for (int outer = 0; outer < outerSize; ++outer) {
        for (int inner = 0; inner < innerSize; ++inner) {
            auto minMaxValue = inputData[outer * axisSize * innerSize + inner];
            int minMaxIndex = 0;
            for (int i = 1; i < axisSize; ++i) {
                const auto& value =
                        inputData[(outer * axisSize + i) * innerSize + inner];
                if ((isArgMin && value < minMaxValue) ||
                    (!isArgMin && value > minMaxValue)) {
                    minMaxValue = value;
                    minMaxIndex = i;
                }
            }
            outputData[outer * innerSize + inner] = minMaxIndex;
        }
    }
}

bool argMinMaxGeneric(const uint8_t* inputData, const Shape& inputShape,
                      int32 axis, bool isArgMin,
                      uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("argMinMaxGeneric");
    NN_CHECK(handleNegativeAxis(inputShape, &axis));

#define NNAPI_IMPL_ARG_MIN_MAX(operandType, dataType)                          \
    if (inputShape.type == operandType) {                                      \
        NNTRACE_COMP_SWITCH("argMinMaxImpl::" #dataType);                      \
        argMinMaxImpl(                                                         \
            reinterpret_cast<const dataType*>(inputData),                      \
            inputShape,                                                        \
            axis,                                                              \
            isArgMin,                                                          \
            reinterpret_cast<int32_t*>(outputData),                            \
            outputShape);                                                      \
        return true;                                                           \
    }

    NNAPI_IMPL_ARG_MIN_MAX(OperandType::TENSOR_FLOAT16, _Float16);
    NNAPI_IMPL_ARG_MIN_MAX(OperandType::TENSOR_FLOAT32, float);
    NNAPI_IMPL_ARG_MIN_MAX(OperandType::TENSOR_INT32, int32_t);
    NNAPI_IMPL_ARG_MIN_MAX(OperandType::TENSOR_QUANT8_ASYMM, uint8_t);
#undef NNAPI_IMPL_ARG_MIN_MAX

    LOG(ERROR) << "Unsupported data type";
    return false;
}

} // namespace nn
} // namespace android
