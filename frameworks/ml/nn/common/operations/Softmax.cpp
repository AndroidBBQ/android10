/*
 * Copyright (C) 2019 The Android Open Source Project
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
#include "OperationResolver.h"

#include "tensorflow/lite/kernels/internal/optimized/legacy_optimized_ops.h"
#include "tensorflow/lite/kernels/internal/optimized/optimized_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {

namespace softmax {

constexpr char kOperationName[] = "SOFTMAX";

constexpr uint32_t kNumInputs = 3;
constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kBetaScalar = 1;
constexpr uint32_t kAxisScalar = 2;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

inline bool softmaxSlowFloat32(const float* inputData, const Shape& inputShape, const float beta,
                               int32_t axis, float* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("softmaxFloatSlow32");
    const uint32_t outerSize = getNumberOfElements(inputShape, 0, axis);
    const uint32_t axisSize = getSizeOfDimension(inputShape, axis);
    const uint32_t innerSize =
            getNumberOfElements(inputShape, axis + 1, getNumberOfDimensions(inputShape));
    for (uint32_t outer = 0; outer < outerSize; ++outer) {
        const float* inputBeg = inputData + outer * axisSize * innerSize;
        const float* inputEnd = inputBeg + axisSize * innerSize;
        float* outputBeg = outputData + outer * axisSize * innerSize;
        for (uint32_t inner = 0; inner < innerSize; ++inner, ++inputBeg, ++inputEnd, ++outputBeg) {
            // Find max
            float maxValue = -FLT_MAX;
            for (const float* p = inputBeg; p < inputEnd; p += innerSize) {
                maxValue = std::max(maxValue, *p);
            }
            // Compute sum
            float sum = 0.0f;
            for (const float* p = inputBeg; p < inputEnd; p += innerSize) {
                sum += std::exp((*p - maxValue) * beta);
            }
            // Compute result
            float* pOut = outputBeg;
            for (const float* p = inputBeg; p < inputEnd; p += innerSize, pOut += innerSize) {
                *pOut = std::exp((*p - maxValue) * beta) / sum;
            }
        }
    }
    return true;
}

bool softmaxFloat32(const float* inputData, const Shape& inputShape, const float beta, int32_t axis,
                    float* outputData, const Shape& outputShape) {
    int32_t ndim = getNumberOfDimensions(inputShape);
    NN_CHECK(handleNegativeAxis(inputShape, &axis));
    // TFLite optimized implementation only supports computation along the last axis
    if (axis == ndim - 1) {
        NNTRACE_COMP("optimized_ops::Softmax::float");
        tflite::SoftmaxParams param = {.beta = beta};
        tflite::optimized_ops::Softmax(param, convertShapeToTflshape(inputShape), inputData,
                                       convertShapeToTflshape(outputShape), outputData);
        return true;
    } else {
        return softmaxSlowFloat32(inputData, inputShape, beta, axis, outputData, outputShape);
    }
}

bool softmaxFloat16(const _Float16* inputData, const Shape& inputShape, const float beta,
                    int32_t axis, _Float16* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("softmaxFloat16");
    std::vector<float> inputData_float32(getNumberOfElements(inputShape));
    convertFloat16ToFloat32(inputData, &inputData_float32);
    std::vector<float> outputData_float32(getNumberOfElements(outputShape));

    softmaxFloat32(inputData_float32.data(), inputShape, beta, axis, outputData_float32.data(),
                   outputShape);
    convertFloat32ToFloat16(outputData_float32, outputData);

    return true;
}

bool softmaxQuant8Impl(const uint8_t* inputData, const Shape& inputShape, const float beta,
                       int32_t axis, int32_t inputMultiplier, int32_t inputLeftShift, float diffMin,
                       uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("softmaxQuant8");
    // The representation chosen for the input to the exp() function is Q5.26.
    // We need to leave extra space since values that we skip might be as large as
    // -32 before multiplying by input_beta_multiplier, and therefore as large as
    // -16 afterwards.  Note that exp(-8) is definitely not insignificant to
    // accumulation, but exp(-16) definitely is.
    static const int32_t kScaledDiffIntegerBits = 5;
    static const int kAccumulationIntegerBits = 12;
    using FixedPointScaledDiff = gemmlowp::FixedPoint<int32_t, kScaledDiffIntegerBits>;
    using FixedPointAccum = gemmlowp::FixedPoint<int32_t, kAccumulationIntegerBits>;
    using FixedPoint0 = gemmlowp::FixedPoint<int32_t, 0>;

    const uint32_t outerSize = getNumberOfElements(inputShape, 0, axis);
    const uint32_t axisSize = getSizeOfDimension(inputShape, axis);
    const uint32_t innerSize =
            getNumberOfElements(inputShape, axis + 1, getNumberOfDimensions(inputShape));
    for (uint32_t outer = 0; outer < outerSize; ++outer) {
        const uint8_t* inputBeg = inputData + outer * axisSize * innerSize;
        const uint8_t* inputEnd = inputBeg + axisSize * innerSize;
        uint8_t* outputBeg = outputData + outer * axisSize * innerSize;
        for (uint32_t inner = 0; inner < innerSize; ++inner, ++inputBeg, ++inputEnd, ++outputBeg) {
            // Find max
            uint8_t maxValue = 0;
            for (const uint8_t* p = inputBeg; p < inputEnd; p += innerSize) {
                maxValue = std::max(maxValue, *p);
            }

            // Compute sum
            FixedPointAccum sum_of_exps = FixedPointAccum::Zero();
            for (const uint8_t* p = inputBeg; p < inputEnd; p += innerSize) {
                int32_t input_diff = static_cast<int32_t>(*p) - maxValue;
                if (input_diff >= diffMin) {
                    const int32_t input_diff_rescaled =
                            tflite::MultiplyByQuantizedMultiplierGreaterThanOne(
                                    input_diff, inputMultiplier, inputLeftShift);
                    const auto scaled_diff_f8 = FixedPointScaledDiff::FromRaw(input_diff_rescaled);
                    sum_of_exps = sum_of_exps + gemmlowp::Rescale<kAccumulationIntegerBits>(
                                                        exp_on_negative_values(scaled_diff_f8));
                }
            }

            uint32_t fixed_sum_of_exps = static_cast<uint32_t>(sum_of_exps.raw());
            int32_t headroom_plus_one = tflite::CountLeadingZeros(fixed_sum_of_exps);
            // This is the number of bits to the left of the binary point above 1.0.
            // Consider fixed_sum_of_exps=1.25.  In that case shifted_scale=0.8 and
            // no later adjustment will be needed.
            int32_t num_bits_over_unit = kAccumulationIntegerBits - headroom_plus_one;
            int32_t shifted_sum_minus_one = static_cast<int32_t>(
                    (fixed_sum_of_exps << headroom_plus_one) - (static_cast<uint32_t>(1) << 31));

            FixedPoint0 shifted_scale = gemmlowp::one_over_one_plus_x_for_x_in_0_1(
                    FixedPoint0::FromRaw(shifted_sum_minus_one));

            // Compute result
            uint8_t* pOut = outputBeg;
            for (const uint8_t* p = inputBeg; p < inputEnd; p += innerSize, pOut += innerSize) {
                int32_t input_diff = static_cast<int32_t>(*p) - maxValue;
                if (input_diff >= diffMin) {
                    const int32_t input_diff_rescaled =
                            tflite::MultiplyByQuantizedMultiplierGreaterThanOne(
                                    input_diff, inputMultiplier, inputLeftShift);
                    const auto scaled_diff_f8 = FixedPointScaledDiff::FromRaw(input_diff_rescaled);

                    FixedPoint0 exp_in_0 = exp_on_negative_values(scaled_diff_f8);
                    int32_t unsat_output = gemmlowp::RoundingDivideByPOT(
                            (shifted_scale * exp_in_0).raw(), num_bits_over_unit + 31 - 8);

                    *pOut = static_cast<uint8_t>(
                            std::max(std::min(unsat_output, static_cast<int32_t>(255)), 0));

                } else {
                    *pOut = 0;
                }
            }
        }
    }
    return true;
}

bool softmaxQuant8(const uint8_t* inputData, const Shape& inputShape, const float beta,
                   int32_t axis, uint8_t* outputData, const Shape& outputShape) {
    int32_t ndim = getNumberOfDimensions(inputShape);
    NN_CHECK(handleNegativeAxis(inputShape, &axis));

    if (outputShape.offset != 0 || outputShape.scale != 1.f / 256) {
        LOG(ERROR) << "incorrect scale / offset for output";
        return false;
    }

    static const int32_t kScaledDiffIntegerBits = 5;
    const double input_beta_real_multiplier =
            std::min(1.0 * beta * inputShape.scale * (1 << (31 - kScaledDiffIntegerBits)),
                     (1LL << 31) - 1.0);

    int32_t inputMultiplier = 0, inputLeftShift = 0;
    if (!QuantizeMultiplierGreaterThanOne(input_beta_real_multiplier, &inputMultiplier,
                                          &inputLeftShift)) {
        return false;
    }
    int32_t diffMin = -CalculateInputRadius(kScaledDiffIntegerBits, inputLeftShift);

    // TFLite optimized implementation only supports computation along the last axis
    if (axis == ndim - 1) {
        NNTRACE_COMP("optimized_ops::Softmax::uint8");
        tflite::SoftmaxParams param = {.beta = beta,
                                       .input_multiplier = inputMultiplier,
                                       .input_left_shift = inputLeftShift,
                                       .diff_min = diffMin};
        tflite::optimized_ops::Softmax(param, convertShapeToTflshape(inputShape), inputData,
                                       convertShapeToTflshape(outputShape), outputData);
        return true;
    } else {
        return softmaxQuant8Impl(inputData, inputShape, beta, axis, inputMultiplier, inputLeftShift,
                                 diffMin, outputData, outputShape);
    }
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK(context->getNumInputs() == kNumInputs ||
                 context->getNumInputs() == kNumInputs - 1);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    auto inputType = context->getInputType(kInputTensor);
    std::vector<OperandType> inExpectedTypes;
    if (inputType == OperandType::TENSOR_FLOAT32 || inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
        inExpectedTypes = {inputType, OperandType::FLOAT32};
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
        inExpectedTypes = {inputType, OperandType::FLOAT16};
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
    if (context->getNumInputs() == kNumInputs) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
        inExpectedTypes.push_back(OperandType::INT32);
    } else {
        const size_t ndim = context->getInputShape(kInputTensor).dimensions.size();
        if (ndim != 2 && ndim != 4 && ndim != 0) {
            NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
        }
    }
    return validateInputTypes(context, inExpectedTypes) &&
           validateOutputTypes(context, {inputType});
}

bool prepare(IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    float beta = (input.type == OperandType::TENSOR_FLOAT16)
                         ? context->getInputValue<_Float16>(kBetaScalar)
                         : context->getInputValue<float>(kBetaScalar);
    NN_RET_CHECK_LE(getNumberOfDimensions(input), 4);
    NN_RET_CHECK_GT(beta, 0.0f);
    Shape output = context->getOutputShape(kOutputTensor);
    output.dimensions = input.dimensions;
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    int32_t axis = (context->getNumInputs() == kNumInputs)
                           ? context->getInputValue<int32_t>(kAxisScalar)
                           : -1;
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT16:
            return softmaxFloat16(context->getInputBuffer<_Float16>(kInputTensor),
                                  context->getInputShape(kInputTensor),
                                  context->getInputValue<_Float16>(kBetaScalar), axis,
                                  context->getOutputBuffer<_Float16>(kOutputTensor),
                                  context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT32:
            return softmaxFloat32(context->getInputBuffer<float>(kInputTensor),
                                  context->getInputShape(kInputTensor),
                                  context->getInputValue<float>(kBetaScalar), axis,
                                  context->getOutputBuffer<float>(kOutputTensor),
                                  context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return softmaxQuant8(context->getInputBuffer<uint8_t>(kInputTensor),
                                 context->getInputShape(kInputTensor),
                                 context->getInputValue<float>(kBetaScalar), axis,
                                 context->getOutputBuffer<uint8_t>(kOutputTensor),
                                 context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace softmax

NN_REGISTER_OPERATION(SOFTMAX, "SOFTMAX", softmax::validate, softmax::prepare, softmax::execute,
                      .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
