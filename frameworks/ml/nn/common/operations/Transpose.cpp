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
#include "tensorflow/lite/kernels/internal/reference/reference_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {
namespace transpose {

constexpr char kOperationName[] = "TRANSPOSE";

constexpr uint32_t kNumInputs = 2;
constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kPermTensor = 1;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

template <typename T>
bool transposeGeneric(const T* inputData, const Shape& inputShape, const int32_t* perm,
                      const Shape& permShape, T* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("transposeGeneric");
    // Reverse the permuted axes and convert to 4D due to the way Dims are
    // constructed.
    const int32_t kOutputDimensionNum = 4;

    // permData can be NO_VALUE representing a regular 2D matrix transpose
    int32_t permSize = perm == nullptr ? 2 : static_cast<int32_t>(getSizeOfDimension(permShape, 0));
    int32_t perm_tmp[2] = {1, 0};
    if (perm == nullptr) {
        perm = perm_tmp;
    }
    int32_t reversed_perm[kOutputDimensionNum];
    for (int32_t output_k = 0, input_k = permSize - 1; output_k < permSize; ++output_k, --input_k) {
        reversed_perm[output_k] = permSize - perm[input_k] - 1;
    }
    for (int32_t k = permSize; k < kOutputDimensionNum; ++k) {
        reversed_perm[k] = k;
    }
    NNTRACE_COMP_SWITCH("reference_ops::Transpose");
    tflite::reference_ops::Transpose(inputData, convertShapeToDims(inputShape), outputData,
                                     convertShapeToDims(outputShape), reversed_perm);
    return true;
}

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumInputs(), kNumInputs);
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);

    const OperandType inputType = context->getInputType(kInputTensor);
    if (inputType == OperandType::TENSOR_FLOAT32 || inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_1));
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
    return validateInputTypes(context, {inputType, OperandType::TENSOR_INT32}) &&
           validateOutputTypes(context, {inputType});
}

bool prepare(IOperationExecutionContext* context) {
    // Only the permutation tensor can be omitted.
    NN_RET_CHECK(!context->isOmittedInput(kInputTensor));
    NN_RET_CHECK(!context->isOmittedOutput(kOutputTensor));

    const Shape& input = context->getInputShape(kInputTensor);
    uint32_t numInputDims = getNumberOfDimensions(input);
    Shape output = context->getOutputShape(kOutputTensor);
    output.type = input.type;
    output.offset = input.offset;
    output.scale = input.scale;

    // permData can be NO_VALUE representing a regular 2D matrix transpose
    if (context->isOmittedInput(kPermTensor)) {
        NN_RET_CHECK_EQ(numInputDims, 2);
        output.dimensions = {getSizeOfDimension(input, 1), getSizeOfDimension(input, 0)};
    } else {
        const Shape& permShape = context->getInputShape(kPermTensor);
        const int32_t* permData = context->getInputBuffer<int32_t>(kPermTensor);

        // Transpose op only supports 1D-4D input arrays.
        NN_RET_CHECK_LE(numInputDims, 4);

        // perm need to be provided as a 1-D int32 tensor.
        NN_RET_CHECK(permShape.type == OperandType::TENSOR_INT32);
        NN_RET_CHECK_EQ(getNumberOfDimensions(permShape), 1);
        NN_RET_CHECK_EQ(numInputDims, getSizeOfDimension(permShape, 0));

        std::vector<uint32_t> outDims(numInputDims);
        for (int32_t idx = 0; idx < static_cast<int32_t>(numInputDims); ++idx) {
            NN_RET_CHECK(permData[idx] >= 0 && permData[idx] < static_cast<int32_t>(numInputDims));
            outDims[idx] = getSizeOfDimension(input, permData[idx]);
        }
        output.dimensions = outDims;
    }
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;

    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT32:
            return transposeGeneric(context->getInputBuffer<float>(kInputTensor),
                                    context->getInputShape(kInputTensor),
                                    context->getInputBuffer<int32_t>(kPermTensor),
                                    context->getInputShape(kPermTensor),
                                    context->getOutputBuffer<float>(kOutputTensor),
                                    context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT16:
            return transposeGeneric(context->getInputBuffer<_Float16>(kInputTensor),
                                    context->getInputShape(kInputTensor),
                                    context->getInputBuffer<int32_t>(kPermTensor),
                                    context->getInputShape(kPermTensor),
                                    context->getOutputBuffer<_Float16>(kOutputTensor),
                                    context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            return transposeGeneric(context->getInputBuffer<uint8_t>(kInputTensor),
                                    context->getInputShape(kInputTensor),
                                    context->getInputBuffer<int32_t>(kPermTensor),
                                    context->getInputShape(kPermTensor),
                                    context->getOutputBuffer<uint8_t>(kOutputTensor),
                                    context->getOutputShape(kOutputTensor));
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace transpose

NN_REGISTER_OPERATION(TRANSPOSE, transpose::kOperationName, transpose::validate, transpose::prepare,
                      transpose::execute, .allowOmittedOperand = true, .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
