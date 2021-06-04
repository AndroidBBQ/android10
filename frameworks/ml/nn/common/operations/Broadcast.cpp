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
#include "OperationResolver.h"

#include "tensorflow/lite/kernels/internal/optimized/legacy_optimized_ops.h"
#include "tensorflow/lite/kernels/internal/reference/legacy_reference_ops.h"

#include "Tracing.h"

#include <algorithm>

namespace android {
namespace nn {
namespace broadcast {

constexpr uint32_t kNumInputs = 3;
constexpr uint32_t kInputTensor1 = 0;
constexpr uint32_t kInputTensor2 = 1;
constexpr uint32_t kActivationScalar = 2;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

#define ANDROID_NN_MACRO_DISPATCH(macro)                                \
    switch (activation) {                                               \
        case (int32_t)FusedActivationFunc::NONE:                        \
            macro(kNone);                                               \
            break;                                                      \
        case (int32_t)FusedActivationFunc::RELU:                        \
            macro(kRelu);                                               \
            break;                                                      \
        case (int32_t)FusedActivationFunc::RELU1:                       \
            macro(kRelu1);                                              \
            break;                                                      \
        case (int32_t)FusedActivationFunc::RELU6:                       \
            macro(kRelu6);                                              \
            break;                                                      \
        default:                                                        \
            LOG(ERROR) << "Unsupported fused activation function type"; \
            return false;                                               \
    }

using binaryFunctionFloat32 = std::function<bool(
        const float* in1, const Shape& shape1, const float* in2, const Shape& shape2,
        int32_t activation, float* out, const Shape& shapeOut)>;

bool binaryOperationFloat16(const _Float16* in1, const Shape& shape1, const _Float16* in2,
                            const Shape& shape2, int32_t activation, _Float16* out,
                            const Shape& shapeOut, binaryFunctionFloat32 operationFloat32) {
    std::vector<float> in1_float32(getNumberOfElements(shape1));
    convertFloat16ToFloat32(in1, &in1_float32);
    std::vector<float> in2_float32(getNumberOfElements(shape2));
    convertFloat16ToFloat32(in2, &in2_float32);
    std::vector<float> out_float32(getNumberOfElements(shapeOut));

    operationFloat32(in1_float32.data(), shape1, in2_float32.data(), shape2, activation,
                     out_float32.data(), shapeOut);
    convertFloat32ToFloat16(out_float32, out);

    return true;
}

bool addFloat32(const float* in1, const Shape& shape1, const float* in2, const Shape& shape2,
                int32_t activation, float* out, const Shape& shapeOut) {
    NNTRACE_TRANS("addFloat32");
    bool needBroadcast = !SameShape(shape1, shape2);
    if (needBroadcast) {
        NNTRACE_COMP_SWITCH("optimized_ops::BroadcastAdd");
#define ANDROID_NN_BROADCAST_ADD(activation)                                              \
    tflite::optimized_ops::BroadcastAdd<tflite::FusedActivationFunctionType::activation>( \
            in1, convertShapeToDims(shape1), in2, convertShapeToDims(shape2), out,        \
            convertShapeToDims(shapeOut))

        ANDROID_NN_MACRO_DISPATCH(ANDROID_NN_BROADCAST_ADD)
#undef ANDROID_NN_BROADCAST_ADD
    } else {
        NNTRACE_COMP_SWITCH("optimized_ops::Add");
#define ANDROID_NN_ADD(activation)                                                 \
    tflite::optimized_ops::Add<tflite::FusedActivationFunctionType::activation>(   \
            in1, convertShapeToDims(shape1), in2, convertShapeToDims(shape2), out, \
            convertShapeToDims(shapeOut))

        ANDROID_NN_MACRO_DISPATCH(ANDROID_NN_ADD)
#undef ANDROID_NN_ADD
    }

    return true;
}

bool addFloat16(const _Float16* in1, const Shape& shape1, const _Float16* in2, const Shape& shape2,
                int32_t activation, _Float16* out, const Shape& shapeOut) {
    NNTRACE_TRANS("addFloat16");
    return binaryOperationFloat16(in1, shape1, in2, shape2, activation, out, shapeOut, &addFloat32);
}

bool addQuant8(const uint8_t* in1, const Shape& shape1, const uint8_t* in2, const Shape& shape2,
               int32_t activation, uint8_t* out, const Shape& shapeOut) {
    NNTRACE_TRANS("addQuant8");
    bool needBroadcast = !SameShape(shape1, shape2);

    const int32_t input1_offset = -shape1.offset;
    const int32_t input2_offset = -shape2.offset;
    const int32_t output_offset = shapeOut.offset;
    const int left_shift = 20;
    const double twice_max_input_scale = 2 * std::max(shape1.scale, shape2.scale);
    const double real_input1_multiplier = shape1.scale / twice_max_input_scale;
    const double real_input2_multiplier = shape2.scale / twice_max_input_scale;
    const double real_output_multiplier =
            twice_max_input_scale / ((1 << left_shift) * shapeOut.scale);

    int32_t input1_multiplier;
    int32_t input1_shift;
    if (!QuantizeMultiplierSmallerThanOne(real_input1_multiplier, &input1_multiplier,
                                          &input1_shift)) {
        return false;
    }
    int32_t input2_multiplier;
    int32_t input2_shift;
    if (!QuantizeMultiplierSmallerThanOne(real_input2_multiplier, &input2_multiplier,
                                          &input2_shift)) {
        return false;
    }
    int32_t output_multiplier;
    int32_t output_shift;
    if (!QuantizeMultiplierSmallerThanOne(real_output_multiplier, &output_multiplier,
                                          &output_shift)) {
        return false;
    }
    int32_t output_activation_min;
    int32_t output_activation_max;
    CalculateActivationRangeUint8(activation, shapeOut, &output_activation_min,
                                  &output_activation_max);

    if (needBroadcast) {
        NNTRACE_COMP_SWITCH("optimized_ops::BroadcastAdd");
#define ANDROID_NN_BROADCAST_ADD(activation)                                                     \
    tflite::optimized_ops::BroadcastAdd<tflite::FusedActivationFunctionType::activation>(        \
            left_shift, in1, convertShapeToDims(shape1), input1_offset, input1_multiplier,       \
            input1_shift, in2, convertShapeToDims(shape2), input2_offset, input2_multiplier,     \
            input2_shift, output_offset, output_multiplier, output_shift, output_activation_min, \
            output_activation_max, out, convertShapeToDims(shapeOut))

        ANDROID_NN_MACRO_DISPATCH(ANDROID_NN_BROADCAST_ADD)
#undef ANDROID_NN_BROADCAST_ADD
    } else {
        NNTRACE_COMP_SWITCH("optimized_ops::Add");
#define ANDROID_NN_NORMAL_ADD(activation)                                                        \
    tflite::optimized_ops::Add<tflite::FusedActivationFunctionType::activation>(                 \
            left_shift, in1, convertShapeToDims(shape1), input1_offset, input1_multiplier,       \
            input1_shift, in2, convertShapeToDims(shape2), input2_offset, input2_multiplier,     \
            input2_shift, output_offset, output_multiplier, output_shift, output_activation_min, \
            output_activation_max, out, convertShapeToDims(shapeOut))

        ANDROID_NN_MACRO_DISPATCH(ANDROID_NN_NORMAL_ADD)
#undef ANDROID_NN_NORMAL_ADD
    }

    return true;
}

bool mulFloat32(const float* in1, const Shape& shape1, const float* in2, const Shape& shape2,
                int32_t activation, float* out, const Shape& shapeOut) {
    NNTRACE_TRANS("mulFloat32");
    bool needBroadcast = !SameShape(shape1, shape2);

    if (needBroadcast) {
        NNTRACE_COMP_SWITCH("optimized_ops::BroadcastMul");
#define ANDROID_NN_BROADCAST_MUL(activation)                                              \
    tflite::optimized_ops::BroadcastMul<tflite::FusedActivationFunctionType::activation>( \
            in1, convertShapeToDims(shape1), in2, convertShapeToDims(shape2), out,        \
            convertShapeToDims(shapeOut))

        ANDROID_NN_MACRO_DISPATCH(ANDROID_NN_BROADCAST_MUL)
#undef ANDROID_NN_BROADCAST_MUL
    } else {
        float output_activation_min, output_activation_max;
        CalculateActivationRangeFloat(activation, &output_activation_min, &output_activation_max);

        NNTRACE_COMP_SWITCH("optimized_ops::Mul");
        tflite::optimized_ops::Mul(in1, convertShapeToDims(shape1), in2, convertShapeToDims(shape2),
                                   output_activation_min, output_activation_max, out,
                                   convertShapeToDims(shapeOut));
    }

    return true;
}

bool mulFloat16(const _Float16* in1, const Shape& shape1, const _Float16* in2, const Shape& shape2,
                int32_t activation, _Float16* out, const Shape& shapeOut) {
    NNTRACE_TRANS("mulFloat16");
    return binaryOperationFloat16(in1, shape1, in2, shape2, activation, out, shapeOut, &mulFloat32);
}

bool mulQuant8(const uint8_t* in1, const Shape& shape1, const uint8_t* in2, const Shape& shape2,
               int32_t activation, uint8_t* out, const Shape& shapeOut) {
    NNTRACE_TRANS("mulQuant8");
    const int32_t input1_offset = -shape1.offset;
    const int32_t input2_offset = -shape2.offset;
    const int32_t output_offset = shapeOut.offset;
    const double input_product_scale = shape1.scale * shape2.scale;
    const double real_multiplier = input_product_scale / shapeOut.scale;
    int32 output_multiplier;
    int output_shift;
    if (!QuantizeMultiplierSmallerThanOne(real_multiplier, &output_multiplier, &output_shift)) {
        return false;
    }
    int32_t output_activation_min;
    int32_t output_activation_max;
    CalculateActivationRangeUint8(activation, shapeOut, &output_activation_min,
                                  &output_activation_max);

    // Use BROADCAST version to handle the normal case.
    NNTRACE_COMP_SWITCH("optimized_ops::BroadcastMul");
    tflite::optimized_ops::BroadcastMul(in1, convertShapeToDims(shape1), input1_offset, in2,
                                        convertShapeToDims(shape2), input2_offset, output_offset,
                                        output_multiplier, output_shift, output_activation_min,
                                        output_activation_max, out, convertShapeToDims(shapeOut));

    return true;
}

bool subFloat32(const float* in1, const Shape& shape1, const float* in2, const Shape& shape2,
                int32_t activation, float* out, const Shape& shapeOut) {
    NNTRACE_TRANS("subFloat32");
    NNTRACE_COMP_SWITCH("optimized_ops::Sub");
    tflite::optimized_ops::Sub(in1, convertShapeToDims(shape1), in2, convertShapeToDims(shape2),
                               out, convertShapeToDims(shapeOut));

    // TFLite does not apply activation to broadcast sub.
    float output_activation_min, output_activation_max;
    CalculateActivationRangeFloat(activation, &output_activation_min, &output_activation_max);
    uint32_t numOutputElements = getNumberOfElements(shapeOut);
    for (uint32_t i = 0; i < numOutputElements; i++) {
        out[i] = std::min(std::max(out[i], output_activation_min), output_activation_max);
    }
    return true;
}

bool subFloat16(const _Float16* in1, const Shape& shape1, const _Float16* in2, const Shape& shape2,
                int32_t activation, _Float16* out, const Shape& shapeOut) {
    NNTRACE_TRANS("subFloat16");
    return binaryOperationFloat16(in1, shape1, in2, shape2, activation, out, shapeOut, &subFloat32);
}

bool subQuant8(const uint8_t* in1, const Shape& shape1, const uint8_t* in2, const Shape& shape2,
               int32_t activation, uint8_t* out, const Shape& shapeOut) {
    NNTRACE_TRANS("subQuant8");

    const int32_t input1_offset = -shape1.offset;
    const int32_t input2_offset = -shape2.offset;
    const int32_t output_offset = shapeOut.offset;
    const int left_shift = 20;
    const double twice_max_input_scale = 2 * std::max(shape1.scale, shape2.scale);
    const double real_input1_multiplier = shape1.scale / twice_max_input_scale;
    const double real_input2_multiplier = shape2.scale / twice_max_input_scale;
    const double real_output_multiplier =
            twice_max_input_scale / ((1 << left_shift) * shapeOut.scale);

    int32_t input1_multiplier;
    int32_t input1_shift;
    if (!QuantizeMultiplierSmallerThanOne(real_input1_multiplier, &input1_multiplier,
                                          &input1_shift)) {
        return false;
    }
    int32_t input2_multiplier;
    int32_t input2_shift;
    if (!QuantizeMultiplierSmallerThanOne(real_input2_multiplier, &input2_multiplier,
                                          &input2_shift)) {
        return false;
    }
    input2_multiplier *= -1;
    int32_t output_multiplier;
    int32_t output_shift;
    if (!QuantizeMultiplierSmallerThanOne(real_output_multiplier, &output_multiplier,
                                          &output_shift)) {
        return false;
    }
    int32_t output_activation_min;
    int32_t output_activation_max;
    CalculateActivationRangeUint8(activation, shapeOut, &output_activation_min,
                                  &output_activation_max);

    // We are using tflite::optimized_ops::BroadcastAdd unconditionally here
    // because tflite::optimized_ops::Add fails to pass some of the
    // sub_quantized_different_scales tests.
    NNTRACE_COMP_SWITCH("optimized_ops::BroadcastAdd");
#define ANDROID_NN_BROADCAST_ADD(activation)                                                     \
    tflite::optimized_ops::BroadcastAdd<tflite::FusedActivationFunctionType::activation>(        \
            left_shift, in1, convertShapeToDims(shape1), input1_offset, input1_multiplier,       \
            input1_shift, in2, convertShapeToDims(shape2), input2_offset, input2_multiplier,     \
            input2_shift, output_offset, output_multiplier, output_shift, output_activation_min, \
            output_activation_max, out, convertShapeToDims(shapeOut))

    ANDROID_NN_MACRO_DISPATCH(ANDROID_NN_BROADCAST_ADD)
#undef ANDROID_NN_BROADCAST_ADD

    return true;
}

bool divFloat32(const float* in1, const Shape& shape1, const float* in2, const Shape& shape2,
                int32_t activation, float* out, const Shape& shapeOut) {
    NNTRACE_TRANS("divFloat32");
    float output_activation_min, output_activation_max;
    CalculateActivationRangeFloat(activation, &output_activation_min, &output_activation_max);

    bool needBroadcast = !SameShape(shape1, shape2);
    if (needBroadcast) {
        NNTRACE_COMP_SWITCH("optimized_ops::BroadcastDiv");
        tflite::optimized_ops::BroadcastDiv(
                in1, convertShapeToDims(shape1), in2, convertShapeToDims(shape2),
                output_activation_min, output_activation_max, out, convertShapeToDims(shapeOut));
    } else {
        NNTRACE_COMP_SWITCH("optimized_ops::Div");
        tflite::optimized_ops::Div(in1, convertShapeToDims(shape1), in2, convertShapeToDims(shape2),
                                   output_activation_min, output_activation_max, out,
                                   convertShapeToDims(shapeOut));
    }
    return true;
}

bool divFloat16(const _Float16* in1, const Shape& shape1, const _Float16* in2, const Shape& shape2,
                int32_t activation, _Float16* out, const Shape& shapeOut) {
    NNTRACE_TRANS("divFloat16");
    return binaryOperationFloat16(in1, shape1, in2, shape2, activation, out, shapeOut, &divFloat32);
}

}  // namespace

bool validate(OperationType opType, const IOperationValidationContext* context) {
    const HalVersion opIntroducedAt = (opType == OperationType::DIV || opType == OperationType::SUB)
                                              ? HalVersion::V1_1
                                              : HalVersion::V1_0;
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    auto inputType = context->getInputType(kInputTensor1);
    if (inputType == OperandType::TENSOR_FLOAT32) {
        NN_RET_CHECK(validateHalVersion(context, std::max(HalVersion::V1_0, opIntroducedAt)));
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        NN_RET_CHECK(validateHalVersion(context, std::max(HalVersion::V1_2, opIntroducedAt)));
    } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        if (opType == OperationType::SUB) {
            NN_RET_CHECK(validateHalVersion(context, std::max(HalVersion::V1_2, opIntroducedAt)));
        } else if (opType == OperationType::DIV) {
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation DIV";
        } else if (opType == OperationType::MUL) {
            Shape output = context->getOutputShape(kOutputTensor);
            Shape input1 = context->getInputShape(kInputTensor1);
            Shape input2 = context->getInputShape(kInputTensor2);
            NN_RET_CHECK_GT(output.scale, input1.scale * input2.scale);
            NN_RET_CHECK(validateHalVersion(context, std::max(HalVersion::V1_0, opIntroducedAt)));
        } else {
            NN_RET_CHECK(validateHalVersion(context, std::max(HalVersion::V1_0, opIntroducedAt)));
        }
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << getOperationName(opType);
    }
    return validateInputTypes(context, {inputType, inputType, OperandType::INT32}) &&
           validateOutputTypes(context, {inputType});
}

bool prepare(IOperationExecutionContext* context) {
    Shape input1 = context->getInputShape(kInputTensor1);
    Shape input2 = context->getInputShape(kInputTensor2);
    Shape output = context->getOutputShape(kOutputTensor);
    NN_RET_CHECK_LE(getNumberOfDimensions(input1), 4);
    NN_RET_CHECK_LE(getNumberOfDimensions(input2), 4);
    NN_RET_CHECK(calculateBroadcastedShape(input1, input2, &output));
    return context->setOutputShape(kOutputTensor, output);
}

bool executeAdd(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor1)) {
        case OperandType::TENSOR_FLOAT16:
            return addFloat16(context->getInputBuffer<_Float16>(kInputTensor1),
                              context->getInputShape(kInputTensor1),
                              context->getInputBuffer<_Float16>(kInputTensor2),
                              context->getInputShape(kInputTensor2),
                              context->getInputValue<int32_t>(kActivationScalar),
                              context->getOutputBuffer<_Float16>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return addFloat32(context->getInputBuffer<float>(kInputTensor1),
                              context->getInputShape(kInputTensor1),
                              context->getInputBuffer<float>(kInputTensor2),
                              context->getInputShape(kInputTensor2),
                              context->getInputValue<int32_t>(kActivationScalar),
                              context->getOutputBuffer<float>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return addQuant8(context->getInputBuffer<uint8_t>(kInputTensor1),
                             context->getInputShape(kInputTensor1),
                             context->getInputBuffer<uint8_t>(kInputTensor2),
                             context->getInputShape(kInputTensor2),
                             context->getInputValue<int32_t>(kActivationScalar),
                             context->getOutputBuffer<uint8_t>(kOutputTensor),
                             context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation ADD";
    }
}

bool executeMul(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor1)) {
        case OperandType::TENSOR_FLOAT16:
            return mulFloat16(context->getInputBuffer<_Float16>(kInputTensor1),
                              context->getInputShape(kInputTensor1),
                              context->getInputBuffer<_Float16>(kInputTensor2),
                              context->getInputShape(kInputTensor2),
                              context->getInputValue<int32_t>(kActivationScalar),
                              context->getOutputBuffer<_Float16>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return mulFloat32(context->getInputBuffer<float>(kInputTensor1),
                              context->getInputShape(kInputTensor1),
                              context->getInputBuffer<float>(kInputTensor2),
                              context->getInputShape(kInputTensor2),
                              context->getInputValue<int32_t>(kActivationScalar),
                              context->getOutputBuffer<float>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return mulQuant8(context->getInputBuffer<uint8_t>(kInputTensor1),
                             context->getInputShape(kInputTensor1),
                             context->getInputBuffer<uint8_t>(kInputTensor2),
                             context->getInputShape(kInputTensor2),
                             context->getInputValue<int32_t>(kActivationScalar),
                             context->getOutputBuffer<uint8_t>(kOutputTensor),
                             context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation MUL";
    }
}

bool executeSub(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor1)) {
        case OperandType::TENSOR_FLOAT16:
            return subFloat16(context->getInputBuffer<_Float16>(kInputTensor1),
                              context->getInputShape(kInputTensor1),
                              context->getInputBuffer<_Float16>(kInputTensor2),
                              context->getInputShape(kInputTensor2),
                              context->getInputValue<int32_t>(kActivationScalar),
                              context->getOutputBuffer<_Float16>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return subFloat32(context->getInputBuffer<float>(kInputTensor1),
                              context->getInputShape(kInputTensor1),
                              context->getInputBuffer<float>(kInputTensor2),
                              context->getInputShape(kInputTensor2),
                              context->getInputValue<int32_t>(kActivationScalar),
                              context->getOutputBuffer<float>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return subQuant8(context->getInputBuffer<uint8_t>(kInputTensor1),
                             context->getInputShape(kInputTensor1),
                             context->getInputBuffer<uint8_t>(kInputTensor2),
                             context->getInputShape(kInputTensor2),
                             context->getInputValue<int32_t>(kActivationScalar),
                             context->getOutputBuffer<uint8_t>(kOutputTensor),
                             context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation SUB";
    }
}

bool executeDiv(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    switch (context->getInputType(kInputTensor1)) {
        case OperandType::TENSOR_FLOAT16:
            return divFloat16(context->getInputBuffer<_Float16>(kInputTensor1),
                              context->getInputShape(kInputTensor1),
                              context->getInputBuffer<_Float16>(kInputTensor2),
                              context->getInputShape(kInputTensor2),
                              context->getInputValue<int32_t>(kActivationScalar),
                              context->getOutputBuffer<_Float16>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return divFloat32(context->getInputBuffer<float>(kInputTensor1),
                              context->getInputShape(kInputTensor1),
                              context->getInputBuffer<float>(kInputTensor2),
                              context->getInputShape(kInputTensor2),
                              context->getInputValue<int32_t>(kActivationScalar),
                              context->getOutputBuffer<float>(kOutputTensor),
                              context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation DIV";
    }
}

}  // namespace broadcast

using std::placeholders::_1;
NN_REGISTER_OPERATION(ADD, "ADD", std::bind(broadcast::validate, OperationType::ADD, _1),
                      broadcast::prepare, broadcast::executeAdd, .allowZeroSizedInput = true);
NN_REGISTER_OPERATION(MUL, "MUL", std::bind(broadcast::validate, OperationType::MUL, _1),
                      broadcast::prepare, broadcast::executeMul, .allowZeroSizedInput = true);
NN_REGISTER_OPERATION(SUB, "SUB", std::bind(broadcast::validate, OperationType::SUB, _1),
                      broadcast::prepare, broadcast::executeSub, .allowZeroSizedInput = true);
NN_REGISTER_OPERATION(DIV, "DIV", std::bind(broadcast::validate, OperationType::DIV, _1),
                      broadcast::prepare, broadcast::executeDiv, .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
