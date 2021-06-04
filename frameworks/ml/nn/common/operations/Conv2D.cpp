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
#include "OperationResolver.h"
#include "Operations.h"

#include "Utils.h"
#include "tensorflow/lite/kernels/internal/optimized/legacy_optimized_ops.h"

#include "Tracing.h"

namespace android {
namespace nn {
namespace conv_2d {

constexpr char kOperationName[] = "CONV_2D";

constexpr uint32_t kInputTensor = 0;
constexpr uint32_t kFilterTensor = 1;
constexpr uint32_t kBiasTensor = 2;

constexpr uint32_t kNumOutputs = 1;
constexpr uint32_t kOutputTensor = 0;

namespace {

// If possible we will use this static buffer for the tensor.
constexpr size_t kStaticBufferSize = 1605632;
char static_scratch_buffer[kStaticBufferSize];

// executionMutex is used to protect concurrent access of the static_scratch_buffer
// and other non-threadsafe resources like gemmlowp::GemmContext.
// std::mutex is safe for pthreads on Android.
std::mutex executionMutex;

struct Conv2dParam {
    int32_t padding_left, padding_right;
    int32_t padding_top, padding_bottom;
    int32_t stride_width, stride_height;
    int32_t dilation_width_factor = 1, dilation_height_factor = 1;
    int32_t activation;
    bool useNchw = false;

    bool initialize(const IOperationExecutionContext* context) {
        uint32_t inCount = context->getNumInputs();
        int32_t padding_implicit = 0;
        bool useImplicitPadding = false;
        if ((inCount >= 8 && context->getInputType(7) == OperandType::BOOL) || inCount == 7) {
            padding_implicit = context->getInputValue<int32_t>(3);
            stride_width = context->getInputValue<int32_t>(4);
            stride_height = context->getInputValue<int32_t>(5);
            activation = context->getInputValue<int32_t>(6);
            if (inCount >= 8) {
                useNchw = context->getInputValue<bool>(7);
            }
            if (inCount == 10) {
                dilation_width_factor = context->getInputValue<int32_t>(8);
                dilation_height_factor = context->getInputValue<int32_t>(9);
            }
            useImplicitPadding = true;
        } else if (inCount >= 10 && context->getInputType(7) == OperandType::INT32) {
            padding_left = context->getInputValue<int32_t>(3);
            padding_right = context->getInputValue<int32_t>(4);
            padding_top = context->getInputValue<int32_t>(5);
            padding_bottom = context->getInputValue<int32_t>(6);
            stride_width = context->getInputValue<int32_t>(7);
            stride_height = context->getInputValue<int32_t>(8);
            activation = context->getInputValue<int32_t>(9);
            if (inCount >= 11) {
                useNchw = context->getInputValue<bool>(10);
            }
            if (inCount == 13) {
                dilation_width_factor = context->getInputValue<int32_t>(11);
                dilation_height_factor = context->getInputValue<int32_t>(12);
            }
        } else {
            NN_RET_CHECK_FAIL() << "Unsupported input spec for operation " << kOperationName;
        }
        if (useImplicitPadding) {
            Shape inputShape = context->getInputShape(kInputTensor);
            Shape filterShape = context->getInputShape(kFilterTensor);
            int32_t input_width = getSizeOfDimension(inputShape, useNchw ? 3 : 2);
            int32_t input_height = getSizeOfDimension(inputShape, useNchw ? 2 : 1);
            int32_t filter_width = getSizeOfDimension(filterShape, 2);
            int32_t filter_height = getSizeOfDimension(filterShape, 1);
            calculateExplicitPadding(input_width, stride_width, dilation_width_factor, filter_width,
                                     padding_implicit, &padding_left, &padding_right);
            calculateExplicitPadding(input_height, stride_height, dilation_height_factor,
                                     filter_height, padding_implicit, &padding_top,
                                     &padding_bottom);
        }
        NN_RET_CHECK_GE(padding_left, 0);
        NN_RET_CHECK_GE(padding_right, 0);
        NN_RET_CHECK_GE(padding_top, 0);
        NN_RET_CHECK_GE(padding_bottom, 0);
        NN_RET_CHECK_GT(stride_width, 0);
        NN_RET_CHECK_GT(stride_height, 0);
        NN_RET_CHECK_GT(dilation_width_factor, 0);
        NN_RET_CHECK_GT(dilation_height_factor, 0);
        NN_RET_CHECK_GE(activation, 0);
        return true;
    }
};

#define ANDROID_NN_CONV_PARAMETERS(Type)                                        \
    uint32_t height       = getSizeOfDimension(inputShape, 1);                  \
    uint32_t width        = getSizeOfDimension(inputShape, 2);                  \
    uint32_t filterHeight = getSizeOfDimension(filterShape, 1);                 \
    uint32_t filterWidth  = getSizeOfDimension(filterShape, 2);                 \
    uint32_t outHeight    = getSizeOfDimension(outputShape, 1);                 \
    uint32_t outWidth     = getSizeOfDimension(outputShape, 2);                 \
    uint32_t inDepth      = getSizeOfDimension(inputShape, 3);                  \
                                                                                \
    uint32_t paddingHeight = (uint32_t)padding_top;                             \
    uint32_t paddingWidth = (uint32_t)padding_left;                             \
                                                                                \
    tflite::Dims<4> im2colDim;                                                  \
    im2colDim.sizes[3] = (int)getSizeOfDimension(outputShape, 0);               \
    im2colDim.sizes[2] = (int)getSizeOfDimension(outputShape, 1);               \
    im2colDim.sizes[1] = (int)getSizeOfDimension(outputShape, 2);               \
    im2colDim.sizes[0] = (int)inDepth * filterHeight * filterWidth;             \
                                                                                \
    im2colDim.strides[0] = 1;                                                   \
    for (int i=1; i<4; i++) {                                                   \
        im2colDim.strides[i] = im2colDim.strides[i-1] * im2colDim.sizes[i-1];   \
    }                                                                           \
                                                                                \
    Type* im2colData = nullptr;                                                 \
    uint64_t im2colByteSize = sizeof(Type);                                     \
    std::unique_ptr<Type[]> im2colGuard;                                        \
    for (int i=0; i<4; i++) {                                                   \
        im2colByteSize *= im2colDim.sizes[i];                                   \
    }                                                                           \
    /* http://b/77982879, tflite::optimized_ops::Conv uses int for offsets */   \
    if (im2colByteSize >= 0x7fffffff)  {                                        \
        LOG(ERROR) << "Conv size is too large, not enough memory";              \
        return false;                                                           \
    }                                                                           \
    if (im2colByteSize <= kStaticBufferSize) {                                  \
        im2colData = reinterpret_cast<Type *>(static_scratch_buffer);           \
    } else {                                                                    \
        im2colData = new (std::nothrow) Type[im2colByteSize / sizeof(Type)];    \
        if (im2colData == nullptr) {                                            \
            LOG(ERROR) << "Conv size is too large, not enough memory";          \
            return false;                                                       \
        }                                                                       \
        im2colGuard.reset(im2colData);                                          \
    }

bool convNhwc(const float* inputData, const Shape& inputShape, const float* filterData,
              const Shape& filterShape, const float* biasData, const Shape& biasShape,
              int32_t padding_left, int32_t padding_right, int32_t padding_top,
              int32_t padding_bottom, int32_t stride_width, int32_t stride_height,
              int32_t dilation_width_factor, int32_t dilation_height_factor, int32_t activation,
              float* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("convFloat32");

    ANDROID_NN_CONV_PARAMETERS(float)

    float output_activation_min, output_activation_max;
    CalculateActivationRangeFloat(activation, &output_activation_min, &output_activation_max);

    // Prevent concurrent executions that may access the scratch buffer.
    std::unique_lock<std::mutex> lock(executionMutex);
    NNTRACE_COMP_SWITCH("optimized_ops::Conv");
    tflite::optimized_ops::Conv(inputData, convertShapeToDims(inputShape), filterData,
                                convertShapeToDims(filterShape), biasData,
                                convertShapeToDims(biasShape), stride_width, stride_height,
                                dilation_width_factor, dilation_height_factor, paddingWidth,
                                paddingHeight, output_activation_min, output_activation_max,
                                outputData, convertShapeToDims(outputShape), im2colData, im2colDim);
    return true;
}

bool convNhwc(const uint8_t* inputData, const Shape& inputShape, const uint8_t* filterData,
              const Shape& filterShape, const int32_t* biasData, const Shape& biasShape,
              int32_t padding_left, int32_t padding_right, int32_t padding_top,
              int32_t padding_bottom, int32_t stride_width, int32_t stride_height,
              int32_t dilation_width_factor, int32_t dilation_height_factor, int32_t activation,
              uint8_t* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("convQuant8");

    ANDROID_NN_CONV_PARAMETERS(uint8_t)

    int32_t inputOffset = -inputShape.offset;
    int32_t filterOffset = -filterShape.offset;
    int32_t outputOffset = outputShape.offset;

    double real_multiplier = 0.0;
    int32_t output_multiplier = 0;
    int32_t output_shift = 0;
    int32_t output_activation_min = 0;
    int32_t output_activation_max = 0;

    NN_RET_CHECK(GetQuantizedConvolutionMultipler(inputShape, filterShape, biasShape, outputShape,
                                                  &real_multiplier));
    int exponent;
    NN_RET_CHECK(QuantizeMultiplier(real_multiplier, &output_multiplier, &exponent));
    output_shift = -exponent;
    CalculateActivationRangeUint8(activation, outputShape, &output_activation_min,
                                  &output_activation_max);

    static gemmlowp::GemmContext gemm_context;

    // Prevent concurrent executions that may access the scratch buffer and
    // gemm_context.
    std::unique_lock<std::mutex> lock(executionMutex);
    // Alow gemmlowp automatically decide how many threads to use.
    gemm_context.set_max_num_threads(0);

    NNTRACE_COMP_SWITCH("optimized_ops::Conv");
    tflite::optimized_ops::Conv(
            inputData, convertShapeToDims(inputShape), inputOffset, filterData,
            convertShapeToDims(filterShape), filterOffset, biasData, convertShapeToDims(biasShape),
            stride_width, stride_height, dilation_width_factor, dilation_height_factor,
            paddingWidth, paddingHeight, outputOffset, output_multiplier, output_shift,
            output_activation_min, output_activation_max, outputData,
            convertShapeToDims(outputShape), im2colData, im2colDim, &gemm_context);
    return true;
}

bool convNhwc(const _Float16* inputData, const Shape& inputShape, const _Float16* filterData,
              const Shape& filterShape, const _Float16* biasData, const Shape& biasShape,
              int32_t padding_left, int32_t padding_right, int32_t padding_top,
              int32_t padding_bottom, int32_t stride_width, int32_t stride_height,
              int32_t dilation_width_factor, int32_t dilation_height_factor, int32_t activation,
              _Float16* outputData, const Shape& outputShape) {
    NNTRACE_TRANS("convFloat16");

    std::vector<float> inputData_float32(getNumberOfElements(inputShape));
    std::vector<float> filterData_float32(getNumberOfElements(filterShape));
    std::vector<float> biasData_float32(getNumberOfElements(biasShape));
    std::vector<float> outputData_float32(getNumberOfElements(outputShape));

    convertFloat16ToFloat32(inputData, &inputData_float32);
    convertFloat16ToFloat32(filterData, &filterData_float32);
    convertFloat16ToFloat32(biasData, &biasData_float32);

    convNhwc(inputData_float32.data(), inputShape, filterData_float32.data(), filterShape,
             biasData_float32.data(), biasShape, padding_left, padding_right, padding_top,
             padding_bottom, stride_width, stride_height, dilation_width_factor,
             dilation_height_factor, activation, outputData_float32.data(), outputShape);
    convertFloat32ToFloat16(outputData_float32, outputData);

    return true;
}

template <typename T_Input, typename T_Filter, typename T_Bias>
bool conv(const T_Input* inputData, const Shape& inputShape, const T_Filter* filterData,
          const Shape& filterShape, const T_Bias* biasData, const Shape& biasShape,
          int32_t padding_left, int32_t padding_right, int32_t padding_top, int32_t padding_bottom,
          int32_t stride_width, int32_t stride_height, int32_t dilation_width_factor,
          int32_t dilation_height_factor, int32_t activation, bool useNchw, T_Input* outputData,
          const Shape& outputShape) {
    InputWithLayout<T_Input> input(useNchw);
    OutputWithLayout<T_Input> output(useNchw);
    NN_RET_CHECK(input.initialize(inputData, inputShape));
    NN_RET_CHECK(output.initialize(outputData, outputShape));
    NN_RET_CHECK(convNhwc(input.getNhwcBuffer(), input.getNhwcShape(), filterData, filterShape,
                          biasData, biasShape, padding_left, padding_right, padding_top,
                          padding_bottom, stride_width, stride_height, dilation_width_factor,
                          dilation_height_factor, activation, output.getNhwcBuffer(),
                          output.getNhwcShape()));
    NN_RET_CHECK(output.commit());
    return true;
}

bool convQuant8PerChannelNhwc(const uint8_t* inputData, const Shape& inputShape,
                              const int8_t* filterData, const Shape& filterShape,
                              const float* filterScales, const int32_t* biasData,
                              const Shape& biasShape, int32_t paddingLeft, int32_t paddingRight,
                              int32_t paddingTop, int32_t paddingBottom, int32_t strideWidth,
                              int32_t strideHeight, int32_t dilationWidthFactor,
                              int32_t dilationHeightFactor, int32_t activation, uint8_t* outputData,
                              const Shape& outputShape) {
    NNTRACE_TRANS("convQuant8PerChannel");

    uint32_t numBatches = getSizeOfDimension(inputShape, 0);
    uint32_t inputHeight = getSizeOfDimension(inputShape, 1);
    uint32_t inputWidth = getSizeOfDimension(inputShape, 2);
    uint32_t inputDepth = getSizeOfDimension(inputShape, 3);
    uint32_t filterHeight = getSizeOfDimension(filterShape, 1);
    uint32_t filterWidth = getSizeOfDimension(filterShape, 2);
    uint32_t filterDepth = getSizeOfDimension(filterShape, 3);
    uint32_t outputHeight = getSizeOfDimension(outputShape, 1);
    uint32_t outputWidth = getSizeOfDimension(outputShape, 2);
    uint32_t outputDepth = getSizeOfDimension(outputShape, 3);

    int32_t inputOffset = -inputShape.offset;
    int32_t outputOffset = outputShape.offset;

    auto realMultiplier = std::vector<double>(outputDepth, .0f);
    auto outputMultiplier = std::vector<int32_t>(outputDepth, 0);
    auto outputShift = std::vector<int32_t>(outputDepth, .0f);

    for (int i = 0; i < outputDepth; ++i) {
        Shape filterChannelShape = filterShape;
        filterChannelShape.scale = filterScales[i];
        Shape biasChannelShape = biasShape;
        biasChannelShape.scale = filterScales[i] * inputShape.scale;
        NN_RET_CHECK(GetQuantizedConvolutionMultipler(
                inputShape, filterChannelShape, biasChannelShape, outputShape, &realMultiplier[i]));
        int exponent;
        NN_RET_CHECK(QuantizeMultiplier(realMultiplier[i], &outputMultiplier[i], &exponent));
        outputShift[i] = -exponent;
    }

    int32_t output_activation_min = 0, output_activation_max = 0;
    CalculateActivationRangeUint8(activation, outputShape, &output_activation_min,
                                  &output_activation_max);
    const uint8_t* inputBase = inputData;
    uint8_t* outPtr = outputData;
    for (uint32_t b = 0; b < numBatches; b++) {
        for (uint32_t h = 0; h < outputHeight; h++) {
            for (uint32_t w = 0; w < outputWidth; w++) {
                const int8_t* filterBase = filterData;

                for (uint32_t d = 0; d < outputDepth; d++) {
                    int32_t wInputOrigin = static_cast<int32_t>(w) * strideWidth - paddingLeft;
                    int32_t hInputOrigin = static_cast<int32_t>(h) * strideHeight - paddingTop;
                    int32_t sum = 0.0f;

                    for (uint32_t i = 0; i < filterHeight; i++) {
                        for (uint32_t j = 0; j < filterWidth; j++) {
                            for (uint32_t k = 0; k < filterDepth; k++) {
                                int32_t hInput = hInputOrigin +
                                                 dilationHeightFactor * static_cast<int32_t>(i);
                                int32_t wInput = wInputOrigin +
                                                 dilationWidthFactor * static_cast<int32_t>(j);
                                uint32_t dInput = k;
                                if (hInput >= 0 && hInput < static_cast<int32_t>(inputHeight) &&
                                    wInput >= 0 && wInput < static_cast<int32_t>(inputWidth)) {
                                    uint32_t filterIndex =
                                            i * filterWidth * filterDepth + j * filterDepth + k;
                                    uint32_t inputIndex = hInput * inputWidth * inputDepth +
                                                          wInput * inputDepth + dInput;
                                    sum += (static_cast<int32_t>(filterBase[filterIndex])) *
                                           (static_cast<int32_t>(inputBase[inputIndex]) +
                                            inputOffset);
                                }
                            }
                        }
                    }
                    sum += biasData[d];
                    sum = tflite::MultiplyByQuantizedMultiplier(sum, outputMultiplier[d],
                                                                -outputShift[d]);
                    sum += outputOffset;
                    sum = std::max(std::min(sum, output_activation_max), output_activation_min);
                    outPtr[d] = static_cast<uint8_t>(sum);
                    filterBase += filterHeight * filterWidth * filterDepth;
                }
                outPtr += outputDepth;
            }
        }
        inputBase += inputHeight * inputWidth * inputDepth;
    }

    return true;
}

bool convQuant8PerChannel(const uint8_t* inputData, const Shape& inputShape,
                          const int8_t* filterData, const Shape& filterShape,
                          const float* filterScales, const int32_t* biasData,
                          const Shape& biasShape, int32_t paddingLeft, int32_t paddingRight,
                          int32_t paddingTop, int32_t paddingBottom, int32_t strideWidth,
                          int32_t strideHeight, int32_t dilationWidthFactor,
                          int32_t dilationHeightFactor, int32_t activation, bool useNchw,
                          uint8_t* outputData, const Shape& outputShape) {
    InputWithLayout<uint8_t> input(useNchw);
    OutputWithLayout<uint8_t> output(useNchw);
    NN_RET_CHECK(input.initialize(inputData, inputShape));
    NN_RET_CHECK(output.initialize(outputData, outputShape));
    NN_RET_CHECK(convQuant8PerChannelNhwc(
            input.getNhwcBuffer(), input.getNhwcShape(), filterData, filterShape, filterScales,
            biasData, biasShape, paddingLeft, paddingRight, paddingTop, paddingBottom, strideWidth,
            strideHeight, dilationWidthFactor, dilationHeightFactor, activation,
            output.getNhwcBuffer(), output.getNhwcShape()));
    NN_RET_CHECK(output.commit());
    return true;
}

#undef ANDROID_NN_CONV_PARAMETERS

}  // namespace

bool validate(const IOperationValidationContext* context) {
    NN_RET_CHECK_EQ(context->getNumOutputs(), kNumOutputs);
    auto inputCount = context->getNumInputs();
    auto inputType = context->getInputType(kInputTensor);
    auto filterType = context->getInputType(kFilterTensor);
    std::vector<OperandType> inExpectedTypes;
    if (inputType == OperandType::TENSOR_FLOAT32) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT32, OperandType::TENSOR_FLOAT32,
                           OperandType::TENSOR_FLOAT32, OperandType::INT32,
                           OperandType::INT32,          OperandType::INT32,
                           OperandType::INT32};
    } else if (inputType == OperandType::TENSOR_FLOAT16) {
        inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16,
                           OperandType::TENSOR_FLOAT16, OperandType::INT32,
                           OperandType::INT32,          OperandType::INT32,
                           OperandType::INT32};
    } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        if (filterType == OperandType::TENSOR_QUANT8_ASYMM ||
            filterType == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
            inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                               filterType,
                               OperandType::TENSOR_INT32,
                               OperandType::INT32,
                               OperandType::INT32,
                               OperandType::INT32,
                               OperandType::INT32};

            if (filterType == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
                NN_RET_CHECK_EQ(
                        context->getInputExtraParams(kFilterTensor).channelQuant().channelDim, 0)
                        << "Unsupported filter tensor channel dimension for operation "
                        << kOperationName;
            }
        } else {
            NN_RET_CHECK_FAIL() << "Unsupported filter tensor type for operation "
                                << kOperationName;
        }
    } else {
        NN_RET_CHECK_FAIL() << "Unsupported input tensor type for operation " << kOperationName;
    }

    // NeuralNetworks.h specifies that ANEURALNETWORKS_CONV_2D's output must
    // meet "outputScale > inputScale * filterScale" for the operand type
    // ANEURALNETWORKS_TENSOR_QUANT8_ASYMM before API level 29. For other
    // operand types (e.g., ANEURALNETWORKS_TENSOR_FLOAT32), this constraint
    // does not apply, so by default the constraint is met.
    bool meetsQuantizedScaleConstraintBeforeV1_2 = true;
    if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
        const float inputScale = context->getInputShape(kInputTensor).scale;
        const float filterScale = context->getInputShape(kFilterTensor).scale;
        const float outputScale = context->getInputShape(kOutputTensor).scale;
        meetsQuantizedScaleConstraintBeforeV1_2 = (outputScale > inputScale * filterScale);
    }

    bool withExplicitPadding = false;
    bool withLayout = false;
    bool withDilation = false;
    if (inputCount >= 8) {
        if (context->getInputType(7) == OperandType::INT32 && inputCount >= 10) {
            std::vector<OperandType> explicitScalarTypes(3, OperandType::INT32);
            inExpectedTypes.insert(inExpectedTypes.end(), explicitScalarTypes.begin(),
                                   explicitScalarTypes.end());
            withExplicitPadding = true;
        }
        int inputOffset = withExplicitPadding ? 3 : 0;
        if (inputCount >= 8 + inputOffset) {
            inExpectedTypes.push_back(OperandType::BOOL);
            withLayout = true;
        }
        NN_RET_CHECK_NE(inputCount, 9 + inputOffset)
                << "Provided only one dilation factor value, two values are requred for operation "
                << kOperationName;
        if (inputCount == 10 + inputOffset) {
            inExpectedTypes.push_back(OperandType::INT32);
            inExpectedTypes.push_back(OperandType::INT32);
            withDilation = true;
        }
    }

    if (filterType == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL || withLayout || withDilation ||
        !meetsQuantizedScaleConstraintBeforeV1_2) {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_2));
    } else {
        NN_RET_CHECK(validateHalVersion(context, HalVersion::V1_0));
    }
    return validateInputTypes(context, inExpectedTypes) &&
           validateOutputTypes(context, {inputType});
}

bool prepare(IOperationExecutionContext* context) {
    Shape input = context->getInputShape(kInputTensor);
    Shape filter = context->getInputShape(kFilterTensor);
    Shape bias = context->getInputShape(kBiasTensor);

    if (filter.type == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
        NN_RET_CHECK(input.type == OperandType::TENSOR_QUANT8_ASYMM);
    } else {
        NN_RET_CHECK(input.type == filter.type);
    }
    if (input.type == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_RET_CHECK(bias.type == OperandType::TENSOR_INT32);
    } else {
        NN_RET_CHECK(input.type == bias.type);
    }
    NN_RET_CHECK_EQ(getNumberOfDimensions(input), 4);
    NN_RET_CHECK_EQ(getNumberOfDimensions(filter), 4);
    NN_RET_CHECK_EQ(getNumberOfDimensions(bias), 1);

    Conv2dParam param;
    NN_RET_CHECK(param.initialize(context));

    uint32_t batches = getSizeOfDimension(input, 0);
    uint32_t height = getSizeOfDimension(input, param.useNchw ? 2 : 1);
    uint32_t width = getSizeOfDimension(input, param.useNchw ? 3 : 2);
    uint32_t channels_in = getSizeOfDimension(input, param.useNchw ? 1 : 3);
    uint32_t channels_out = getSizeOfDimension(filter, 0);
    uint32_t filterHeight = getSizeOfDimension(filter, 1);
    uint32_t filterWidth = getSizeOfDimension(filter, 2);
    // Only batches can be zero.
    NN_RET_CHECK_EQ(channels_in, getSizeOfDimension(filter, 3));
    NN_RET_CHECK_EQ(channels_out, getSizeOfDimension(bias, 0));
    NN_RET_CHECK_GT(height, 0);
    NN_RET_CHECK_GT(width, 0);
    NN_RET_CHECK_GT(channels_in, 0);
    NN_RET_CHECK_GT(channels_out, 0);

    int32_t effectiveFilterWidth = (filterWidth - 1) * param.dilation_width_factor + 1;
    int32_t effectiveFilterHeight = (filterHeight - 1) * param.dilation_height_factor + 1;
    NN_RET_CHECK_GT(effectiveFilterWidth, param.padding_left);
    NN_RET_CHECK_GT(effectiveFilterWidth, param.padding_right);
    NN_RET_CHECK_GT(effectiveFilterHeight, param.padding_top);
    NN_RET_CHECK_GT(effectiveFilterHeight, param.padding_bottom);

    uint32_t outWidth =
            computeOutSize(width, filterWidth, param.stride_width, param.dilation_width_factor,
                           param.padding_left, param.padding_right);
    uint32_t outHeight =
            computeOutSize(height, filterHeight, param.stride_height, param.dilation_height_factor,
                           param.padding_top, param.padding_bottom);

    Shape output = context->getOutputShape(kOutputTensor);
    output.type = input.type;
    if (param.useNchw) {
        output.dimensions = {batches, channels_out, outHeight, outWidth};
    } else {
        output.dimensions = {batches, outHeight, outWidth, channels_out};
    }
    return context->setOutputShape(kOutputTensor, output);
}

bool execute(IOperationExecutionContext* context) {
    // Bypass execution in the case of zero-sized input.
    if (getNumberOfElements(context->getOutputShape(kOutputTensor)) == 0) return true;
    Conv2dParam param;
    NN_RET_CHECK(param.initialize(context));
    switch (context->getInputType(kInputTensor)) {
        case OperandType::TENSOR_FLOAT32:
            return conv(context->getInputBuffer<float>(kInputTensor),
                        context->getInputShape(kInputTensor),
                        context->getInputBuffer<float>(kFilterTensor),
                        context->getInputShape(kFilterTensor),
                        context->getInputBuffer<float>(kBiasTensor),
                        context->getInputShape(kBiasTensor), param.padding_left,
                        param.padding_right, param.padding_top, param.padding_bottom,
                        param.stride_width, param.stride_height, param.dilation_width_factor,
                        param.dilation_height_factor, param.activation, param.useNchw,
                        context->getOutputBuffer<float>(kOutputTensor),
                        context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_FLOAT16:
            return conv(context->getInputBuffer<_Float16>(kInputTensor),
                        context->getInputShape(kInputTensor),
                        context->getInputBuffer<_Float16>(kFilterTensor),
                        context->getInputShape(kFilterTensor),
                        context->getInputBuffer<_Float16>(kBiasTensor),
                        context->getInputShape(kBiasTensor), param.padding_left,
                        param.padding_right, param.padding_top, param.padding_bottom,
                        param.stride_width, param.stride_height, param.dilation_width_factor,
                        param.dilation_height_factor, param.activation, param.useNchw,
                        context->getOutputBuffer<_Float16>(kOutputTensor),
                        context->getOutputShape(kOutputTensor));
        case OperandType::TENSOR_QUANT8_ASYMM:
            if (context->getInputType(kFilterTensor) ==
                OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
                return convQuant8PerChannel(
                        context->getInputBuffer<uint8_t>(kInputTensor),
                        context->getInputShape(kInputTensor),
                        context->getInputBuffer<int8_t>(kFilterTensor),
                        context->getInputShape(kFilterTensor),
                        context->getInputExtraParams(kFilterTensor).channelQuant().scales.data(),
                        context->getInputBuffer<int32_t>(kBiasTensor),
                        context->getInputShape(kBiasTensor), param.padding_left,
                        param.padding_right, param.padding_top, param.padding_bottom,
                        param.stride_width, param.stride_height, param.dilation_width_factor,
                        param.dilation_height_factor, param.activation, param.useNchw,
                        context->getOutputBuffer<uint8_t>(kOutputTensor),
                        context->getOutputShape(kOutputTensor));
            } else if (context->getInputType(kFilterTensor) == OperandType::TENSOR_QUANT8_ASYMM) {
                return conv(context->getInputBuffer<uint8_t>(kInputTensor),
                            context->getInputShape(kInputTensor),
                            context->getInputBuffer<uint8_t>(kFilterTensor),
                            context->getInputShape(kFilterTensor),
                            context->getInputBuffer<int32_t>(kBiasTensor),
                            context->getInputShape(kBiasTensor), param.padding_left,
                            param.padding_right, param.padding_top, param.padding_bottom,
                            param.stride_width, param.stride_height, param.dilation_width_factor,
                            param.dilation_height_factor, param.activation, param.useNchw,
                            context->getOutputBuffer<uint8_t>(kOutputTensor),
                            context->getOutputShape(kOutputTensor));
            } else {
                NN_RET_CHECK_FAIL() << "Unsupported filter type for operation " << kOperationName;
            }
        default:
            NN_RET_CHECK_FAIL() << "Unsupported tensor type for operation " << kOperationName;
    }
}

}  // namespace conv_2d

NN_REGISTER_OPERATION(CONV_2D, conv_2d::kOperationName, conv_2d::validate, conv_2d::prepare,
                      conv_2d::execute, .allowZeroSizedInput = true);

}  // namespace nn
}  // namespace android
