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

#include "CpuOperationUtils.h"
#include "Operations.h"

#include <algorithm>
#include <cmath>
#include "tensorflow/lite/kernels/internal/optimized/optimized_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {

inline bool localResponseNormFloat32Impl(const float* inputData, const Shape& inputShape,
                                         int32_t radius, float bias, float alpha, float beta,
                                         int32_t axis, float* outputData,
                                         const Shape& outputShape) {
    NNTRACE_TRANS("localResponseNormFloat32");
    const uint32_t outerSize = getNumberOfElements(inputShape, 0, axis);
    const uint32_t axisSize = getSizeOfDimension(inputShape, axis);
    const uint32_t innerSize =
            getNumberOfElements(inputShape, axis + 1, getNumberOfDimensions(inputShape));
    for (uint32_t outer = 0; outer < outerSize; ++outer) {
        const float* inputBase = inputData + outer * axisSize * innerSize;
        float* outputBase = outputData + outer * axisSize * innerSize;
        for (uint32_t inner = 0; inner < innerSize; ++inner, ++inputBase, ++outputBase) {
            for (int32_t i = 0; i < axisSize; i++) {
                const int32_t dBegin = std::max(0, i - radius);
                // Add 1 on dEnd to comply with optimized_ops in TFLite
                const int32_t dEnd = std::min(static_cast<int32_t>(axisSize), i + radius + 1);
                float sum = 0.0f;
                for (int32_t d = dBegin; d < dEnd; d++) {
                    float val = inputBase[d * innerSize];
                    sum += val * val;
                }
                float multiplier = std::pow(bias + alpha * sum, -beta);
                outputBase[i * innerSize] = inputBase[i * innerSize] * multiplier;
            }
        }
    }
    return true;
}

bool localResponseNormFloat16(const _Float16* inputData, const Shape& inputShape, int32_t radius,
                              float bias, float alpha, float beta, int32_t axis,
                              _Float16* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("localResponseNormFloat16");
    std::vector<float> inputDataFloat32(getNumberOfElements(inputShape));
    convertFloat16ToFloat32(inputData, &inputDataFloat32);
    std::vector<float> outputDataFloat32(getNumberOfElements(outputShape));

    localResponseNormFloat32(inputDataFloat32.data(), inputShape, radius, bias, alpha, beta, axis,
                             outputDataFloat32.data(), outputShape);
    convertFloat32ToFloat16(outputDataFloat32, outputData);

    return true;
}

bool localResponseNormFloat32(const float* inputData, const Shape& inputShape, int32_t radius,
                              float bias, float alpha, float beta, int32_t axis, float* outputData,
                              const Shape& outputShape) {
    int32_t ndim = getNumberOfDimensions(inputShape);
    NN_CHECK(handleNegativeAxis(inputShape, &axis));
    // TFLite optimized implementation only supports computation along the last axis
    if (axis == ndim - 1) {
        NNTRACE_COMP("optimized_ops::LocalResponseNormalization::float");
        tflite::LocalResponseNormalizationParams param = {
                .range = radius, .bias = bias, .alpha = alpha, .beta = beta};
        tflite::optimized_ops::LocalResponseNormalization(
                param, convertShapeToTflshape(inputShape), inputData,
                convertShapeToTflshape(outputShape), outputData);
        return true;
    } else {
        return localResponseNormFloat32Impl(inputData, inputShape, radius, bias, alpha, beta, axis,
                                            outputData, outputShape);
    }
}
}  // namespace nn
}  // namespace android
