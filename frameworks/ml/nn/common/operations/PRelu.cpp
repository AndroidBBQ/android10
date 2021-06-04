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

#include "HalInterfaces.h"
#include "IndexedShapeWrapper.h"
#include "OperationResolver.h"
#include "OperationsUtils.h"
#include "Tracing.h"
#include "tensorflow/lite/kernels/internal/optimized/legacy_optimized_ops.h"

namespace android {
namespace nn {
namespace prelu {

constexpr char kOperationName[] = "PRELU";

constexpr uint32_t kNumInputs = 2;
constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kAlphaTensor = 1;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

template <typename T>
inline bool eval(const std::function<T(const T&, const T&)>& func, const T* aData,
                 const Shape& aShape, const T* bData, const Shape& bShape, T* outputData,
                 const Shape& outputShape) {
    IndexedShapeWrapper aShapeIndexed(aShape);
    IndexedShapeWrapper bShapeIndexed(bShape);
    IndexedShapeWrapper outputShapeIndexed(outputShape);
    std::vector<uint32_t> curIndex(outputShape.dimensions.size(), 0);
    bool lastIndex = false;
    do {
        uint32_t outputFlatIndex;
        NN_RET_CHECK(outputShapeIndexed.indexToFlatIndex(curIndex, &outputFlatIndex));
        uint32_t aFlatIndex;
        NN_RET_CHECK(aShapeIndexed.broadcastedIndexToFlatIndex(curIndex, &aFlatIndex));
        uint32_t bFlatIndex;
        NN_RET_CHECK(bShapeIndexed.broadcastedIndexToFlatIndex(curIndex, &bFlatIndex));

        outputData[outputFlatIndex] = func(aData[aFlatIndex], bData[bFlatIndex]);

        NN_RET_CHECK(outputShapeIndexed.nextIndexInplace(&curIndex, &lastIndex));
    } while (!lastIndex);
    return true;
}

bool evalQuant8(const uint8_t* aData, const Shape& aShape, const uint8_t* bData,
                const Shape& bShape, uint8_t* outputData, const Shape& outputShape) {
    const int32_t input_offset = -aShape.offset;
    const int32_t alpha_offset = -bShape.offset;
    const int32_t output_offset = outputShape.offset;
    const double input_product_scale = aShape.scale * bShape.scale;
    const double real_multiplier_pos = aShape.scale / outputShape.scale;
    const double real_multiplier_neg = input_product_scale / outputShape.scale;
    int32_t output_multiplier_pos, output_shift_pos;
    int32_t output_multiplier_neg, output_shift_neg;
    tflite::QuantizeMultiplier(real_multiplier_pos, &output_multiplier_pos, &output_shift_pos);
    tflite::QuantizeMultiplier(real_multiplier_neg, &output_multiplier_neg, &output_shift_neg);
    return eval<uint8_t>(
            [&](const uint8_t& val1, const uint8_t& val2) -> uint8_t {
                const int32_t input = input_offset + static_cast<int32_t>(val1);
                int32_t output_val;
                if (input >= 0) {
                    output_val =
                            output_offset + tflite::MultiplyByQuantizedMultiplier(
                                                    input, output_multiplier_pos, output_shift_pos);
                } else {
                    const int32_t alpha = alpha_offset + static_cast<int32_t>(val2);
                    output_val = output_offset +
                                 tflite::MultiplyByQuantizedMultiplier(
                                         input * alpha, output_multiplier_neg, output_shift_neg);
                }
                output_val = std::max(0, std::min(255, output_val));
                return static_cast<uint8_t>(output_val);
            },
            aData, aShape, bData, bShape, outputData, outputShape);
}

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    auto inputType = context->getInputType(kInputTensor);
    NN_RET_CHECK(inputType == OperandType::TENSOR_FLOAT16 ||
                 inputType == OperandType::TENSOR_FLOAT32 ||
                 inputType == OperandType::TENSOR_QUANT8_ASYMM)
            << "Unsupported tensor type for operation " << kOperationName;
    NN_RET_CHECK(validateInputTypes(context, {inputType, inputType}));
    NN_RET_CHECK(validateOutputTypes(context, {inputType}));
    return validateHalVersion(context, HalVersion::V1_2);
}

bool prepare(IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    Shape alpha = context->getInputShape(kAlphaTensor);
    NN_RET_CHECK(input.type == alpha.type);
    Shape output = context->getOutputShape(kOutputTensor);
    NN_RET_CHECK(calculateBroadcastedShape(input, alpha, &output));
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return eval<_Float16>(
                    [](const _Float16& val1, const _Float16& val2) -> _Float16 {
                        return val1 >= 0.0f ? val1 : val1 * val2;
                    },
                    context->getInputBuffer<_Float16>(kInputTensor),
                    context->getInputShape(kInputTensor),
                    context->getInputBuffer<_Float16>(kAlphaTensor),
                    context->getInputShape(kAlphaTensor),
                    context->getOutputBuffer<_Float16>(kOutputTensor),
                    context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return eval<float>(
                    [](const float& val1, const float& val2) -> float {
                        return val1 >= 0.0f ? val1 : val1 * val2;
                    },
                    context->getInputBuffer<float>(kInputTensor),
                    context->getInputShape(kInputTensor),
                    context->getInputBuffer<float>(kAlphaTensor),
                    context->getInputShape(kAlphaTensor),
                    context->getOutputBuffer<float>(kOutputTensor),
                    context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM: {
            return evalQuant8(context->getInputBuffer<uint8_t>(kInputTensor),
                              context->getInputShape(kInputTensor),
                              context->getInputBuffer<uint8_t>(kAlphaTensor),
                              context->getInputShape(kAlphaTensor),
                              context->getOutputBuffer<uint8_t>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        }
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace prelu

NN_REGISTER_OPERATION(PRELU, prelu::kOperationName, prelu::validate, prelu::prepare,
                      prelu::execute);

}  // namespace nn
}  // namespace android
