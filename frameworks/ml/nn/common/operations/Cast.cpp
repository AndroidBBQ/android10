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

#include "Cast.h"
#include "Tracing.h"

namespace android {
namespace nn {
namespace cast {

namespace {

template <typename FromT, typename ToT>
void copyCast(const FromT* in, ToT* out, int numElements) {
    std::transform(in, in + numElements, out, [](FromT a) -> ToT {
        if constexpr (std::is_same_v<ToT, uint8_t>) {
            if (a < 0) return 0;
            if (a > 255) return 255;
        }
        return static_cast<ToT>(a);
    });
}

template <typename FromT>
bool copyToTensor(const FromT* inputData, int numElements, uint8_t* outputData,
                  const Shape& outputShape) {
#define ANDROID_NN_COPY_CAST(operandType, dataType)                                \
    case operandType: {                                                            \
        NNTRACE_COMP("cast::copyCast::" #dataType);                                \
        copyCast(inputData, reinterpret_cast<dataType*>(outputData), numElements); \
        return true;                                                               \
    }

    switch (outputShape.type) {
        ANDROID_NN_COPY_CAST(OperandType::TENSOR_FLOAT16, _Float16);
        ANDROID_NN_COPY_CAST(OperandType::TENSOR_FLOAT32, float);
        ANDROID_NN_COPY_CAST(OperandType::TENSOR_INT32, int32_t);
        ANDROID_NN_COPY_CAST(OperandType::TENSOR_QUANT8_ASYMM, uint8_t);
        default:
            LOG(ERROR) << "Unsupported CAST output type";
            return false;
    }
#undef ANDROID_NN_COPY_CAST
}

}  // namespace

bool prepare(const Shape& input, Shape* output) {
    if (input.dimensions.size() != output->dimensions.size()) {
        return false;
    }
    output->dimensions = input.dimensions;
    return true;
}

bool eval(const uint8_t* inputData, const Shape& inputShape, uint8_t* outputData,
          const Shape& outputShape) {
    NNTRACE_TRANS("cast::eval");
    int numElements = getNumberOfElements(inputShape);

#define ANDROID_NN_COPY_TO_TENSOR(operandType, dataType)                                    \
    case operandType: {                                                                     \
        NNTRACE_TRANS("cast::copyToTensor::" #dataType);                                    \
        copyToTensor(reinterpret_cast<const dataType*>(inputData), numElements, outputData, \
                     outputShape);                                                          \
        return true;                                                                        \
    }

    switch (inputShape.type) {
        ANDROID_NN_COPY_TO_TENSOR(OperandType::TENSOR_FLOAT16, _Float16);
        ANDROID_NN_COPY_TO_TENSOR(OperandType::TENSOR_FLOAT32, float);
        ANDROID_NN_COPY_TO_TENSOR(OperandType::TENSOR_INT32, int32_t);
        ANDROID_NN_COPY_TO_TENSOR(OperandType::TENSOR_QUANT8_ASYMM, uint8_t);
        default:
            LOG(ERROR) << "Unsupported CAST input type";
            return false;
    }
#undef ANDROID_NN_COPY_TO_TENSOR
}

}  // namespace cast
}  // namespace nn
}  // namespace android
