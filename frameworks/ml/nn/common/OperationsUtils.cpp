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

#define LOG_TAG "OperationsUtils"

#include "OperationsUtils.h"
#include "Operations.h"
#include "Utils.h"

#include <cmath>

namespace android {
namespace nn {

namespace {

bool validateOperandTypes(const std::vector<OperandType>& expectedTypes, const char* tag,
                          uint32_t operandCount,
                          std::function<OperandType(uint32_t)> getOperandType) {
    NN_RET_CHECK_EQ(operandCount, expectedTypes.size());
    for (uint32_t i = 0; i < operandCount; ++i) {
        OperandType type = getOperandType(i);
        NN_RET_CHECK(type == expectedTypes[i])
                << "Invalid " << tag << " tensor type " << toString(type) << " for " << tag << " "
                << i << ", expected " << toString(expectedTypes[i]);
    }
    return true;
}

}  // namespace

bool validateInputTypes(const IOperationValidationContext* context,
                        const std::vector<OperandType>& expectedTypes) {
    return validateOperandTypes(expectedTypes, "input", context->getNumInputs(),
                                [context](uint32_t index) { return context->getInputType(index); });
}

bool validateOutputTypes(const IOperationValidationContext* context,
                         const std::vector<OperandType>& expectedTypes) {
    return validateOperandTypes(
            expectedTypes, "output", context->getNumOutputs(),
            [context](uint32_t index) { return context->getOutputType(index); });
}

bool validateHalVersion(const IOperationValidationContext* context,
                        HalVersion minSupportedHalVersion) {
    if (context->getHalVersion() < minSupportedHalVersion) {
        NN_RET_CHECK_FAIL() << "The given inputs and outputs are only supported in "
                            << toString(minSupportedHalVersion) << " and later (validating using "
                            << toString(context->getHalVersion()) << ")";
    }
    return true;
}

bool SameShape(const Shape& in1, const Shape& in2) {
    if (in1.type != in2.type || in1.dimensions.size() != in2.dimensions.size()) {
        return false;
    }
    for (size_t i = 0; i < in1.dimensions.size(); i++) {
        if (in1.dimensions[i] != in2.dimensions[i]) {
            return false;
        }
    }
    return true;
}

bool SetShape(const Shape& in, Shape* out) {
    if (in.type != out->type) {
        return false;
    }
    out->dimensions = in.dimensions;
    return true;
}

bool combineDimensions(const std::vector<uint32_t>& lhs, const std::vector<uint32_t>& rhs,
                       std::vector<uint32_t>* combined) {
    if (rhs.empty()) {
        *combined = lhs;
        return true;
    }
    if (lhs.empty()) {
        *combined = rhs;
        return true;
    }
    NN_RET_CHECK_EQ(lhs.size(), rhs.size()) << "incompatible ranks";
    combined->resize(lhs.size());
    for (uint32_t i = 0; i < lhs.size(); i++) {
        if (lhs[i] == 0) {
            (*combined)[i] = rhs[i];
            continue;
        }
        if (rhs[i] == 0) {
            (*combined)[i] = lhs[i];
            continue;
        }
        NN_RET_CHECK_EQ(lhs[i], rhs[i]) << "incompatible dimension: " << i;
        (*combined)[i] = lhs[i];
    }
    return true;
}

uint32_t getNumberOfElements(const Shape& shape) {
    uint32_t count = 1;
    for (size_t i = 0; i < shape.dimensions.size(); i++) {
        count *= shape.dimensions[i];
    }
    return count;
}

uint32_t getNumberOfElements(const Shape& shape,
                             size_t firstAxisInclusive,
                             size_t lastAxisExclusive) {
    nnAssert(0 <= firstAxisInclusive);
    nnAssert(firstAxisInclusive <= lastAxisExclusive);
    nnAssert(lastAxisExclusive <= shape.dimensions.size());
    uint32_t count = 1;
    for (size_t i = firstAxisInclusive; i < lastAxisExclusive; i++) {
        count *= shape.dimensions[i];
    }
    return count;
}

uint32_t getNumberOfDimensions(const Shape& shape) {
    return shape.dimensions.size();
}

uint32_t getSizeOfDimension(const Shape& shape, uint32_t dimensionIdx) {
    nnAssert(0 <= dimensionIdx && dimensionIdx < shape.dimensions.size());
    return shape.dimensions[dimensionIdx];
}

bool handleNegativeAxis(int32_t numberOfDimensions, int32_t* axis) {
    NN_CHECK(-numberOfDimensions <= *axis && *axis < numberOfDimensions);
    if (*axis < 0) {
        *axis += numberOfDimensions;
    }
    return true;
}

bool QuantizeMultiplier(double double_multiplier, int32_t* quantized_multiplier, int* shift) {
    if (double_multiplier == 0.) {
        *quantized_multiplier = 0;
        *shift = 0;
        return true;
    }
    const double q = std::frexp(double_multiplier, shift);
    auto q_fixed = static_cast<int64_t>(std::round(q * (1ll << 31)));
    NN_RET_CHECK(q_fixed <= (1ll << 31));
    if (q_fixed == (1ll << 31)) {
        q_fixed /= 2;
        ++*shift;
    }
    NN_RET_CHECK_LE(q_fixed, std::numeric_limits<int32_t>::max());
    *quantized_multiplier = static_cast<int32_t>(q_fixed);
    return true;
}

bool QuantizeMultiplierSmallerThanOne(double double_multiplier,
                                      int32_t* quantized_multiplier,
                                      int32_t* right_shift) {
    NN_OPS_CHECK(double_multiplier >= 0.);
    NN_OPS_CHECK(double_multiplier < 1.);
    if (double_multiplier == 0.) {
        *quantized_multiplier = 0;
        *right_shift = 0;
        return true;
    }
    NN_OPS_CHECK(double_multiplier > 0.);
    const double q = std::frexp(double_multiplier, right_shift);
    *right_shift *= -1;
    int64_t q_fixed = static_cast<int64_t>(std::round(q * (1LL << 31)));
    NN_OPS_CHECK(q_fixed <= (1LL << 31));
    if (q_fixed == (1LL << 31)) {
        q_fixed /= 2;
        --*right_shift;
    }
    NN_OPS_CHECK(*right_shift >= 0);
    NN_OPS_CHECK(q_fixed <= std::numeric_limits<int32_t>::max());
    *quantized_multiplier = static_cast<int32_t>(q_fixed);
    return true;
}

bool QuantizeMultiplierGreaterThanOne(double double_multiplier,
                                      int32_t* quantized_multiplier,
                                      int* left_shift) {
    NN_OPS_CHECK(double_multiplier > 1.);
    const double q = std::frexp(double_multiplier, left_shift);
    int64_t q_fixed = static_cast<int64_t>(std::round(q * (1LL << 31)));
    NN_OPS_CHECK(q_fixed <= (1LL << 31));
    if (q_fixed == (1LL << 31)) {
        q_fixed /= 2;
        ++*left_shift;
    }
    NN_OPS_CHECK(*left_shift >= 0);
    NN_OPS_CHECK(q_fixed <= std::numeric_limits<int32_t>::max());
    *quantized_multiplier = static_cast<int32_t>(q_fixed);
    return true;
}

bool GetQuantizedConvolutionMultipler(const Shape& inputShape, const Shape& filterShape,
                                      const Shape& biasShape, const Shape& outputShape,
                                      double* multiplier) {
    // Upcast bias and input_product to double
    const double input_product_scale = inputShape.scale * filterShape.scale;
    const double bias_scale = biasShape.scale;

    // The following conditions must be guaranteed by the training pipeline.
    NN_OPS_CHECK(std::abs(input_product_scale - bias_scale) <=
              1e-6 * std::min(input_product_scale, bias_scale));
    NN_OPS_CHECK(input_product_scale >= 0);
    *multiplier = input_product_scale / outputShape.scale;
    return true;
}

void CalculateActivationRangeUint8(int32_t activation,
                                   const Shape& outputShape,
                                   int32_t* act_min,
                                   int32_t* act_max) {
    const int32_t qmin = std::numeric_limits<uint8_t>::min();
    const int32_t qmax = std::numeric_limits<uint8_t>::max();

    const auto scale = outputShape.scale;
    const auto zero_point = outputShape.offset;

    auto quantize = [scale, zero_point](float f) {
        return zero_point + static_cast<int32_t>(std::round(f / scale));
    };

    if (activation == kActivationRelu) {
        *act_min = std::max(qmin, quantize(0.0));
        *act_max = qmax;
    } else if (activation == kActivationRelu6) {
        *act_min = std::max(qmin, quantize(0.0));
        *act_max = std::min(qmax, quantize(6.0));
    } else if (activation == kActivationRelu1) {
        *act_min = std::max(qmin, quantize(-1.0));
        *act_max = std::min(qmax, quantize(1.0));
    } else if (activation == kActivationNone){
        *act_min = qmin;
        *act_max = qmax;
    } else {
        LOG(ERROR) << "Unsupported fused activation function.";
    }
}

void CalculateActivationRangeFloat(int32_t activation,
                                   float* activation_min,
                                   float* activation_max) {
    if (activation == kActivationRelu) {
        *activation_min = 0.f;
        *activation_max = std::numeric_limits<float>::max();
    } else if (activation == kActivationRelu6) {
        *activation_min = 0.f;
        *activation_max = 6.f;
    } else if (activation == kActivationRelu1) {
        *activation_min = -1.f;
        *activation_max = 1.f;
    } else if (activation == kActivationNone){
        *activation_min = std::numeric_limits<float>::lowest();
        *activation_max = std::numeric_limits<float>::max();
    } else {
        LOG(ERROR) << "Unsupported fused activation function.";
    }
}

int32_t CalculateInputRadius(int input_integer_bits, int input_left_shift) {
    const double max_input_rescaled = 1.0 * ((1 << input_integer_bits) - 1) *
                                      (1LL << (31 - input_integer_bits)) /
                                      (1LL << input_left_shift);
    // Tighten bound using floor.  Suppose that we could use the exact value.
    // After scaling the difference, the result would be at the maximum.  Thus we
    // must ensure that our value has lower magnitude.
    return static_cast<int32_t>(std::floor(max_input_rescaled));
}

void calculateExplicitPaddingImpl(int32_t in_size, int32_t stride, int32_t dilation_factor,
                                  int32_t filter_size, int32_t padding_implicit,
                                  bool isTransposeConv, int32_t* padding_head,
                                  int32_t* padding_tail) {
    *padding_head = 0;
    *padding_tail = 0;

    int32_t effective_filter_size = (filter_size - 1) * dilation_factor + 1;

    if (padding_implicit == kPaddingSame) {
        int32_t out_size = (in_size + stride - 1) / stride;
        int32_t tmp = (out_size - 1) * stride + effective_filter_size;
        if (tmp > in_size) {
            *padding_head = (tmp - in_size) / 2;
            *padding_tail = (tmp - in_size) - *padding_head;
        }
        // For transpose conv, make padding tail fit tightly to the end of the last stride.
        if (isTransposeConv) {
            *padding_tail = (tmp - in_size) - *padding_head;
        }
    }
}

bool calculateBroadcastedShape(const Shape& in1, const Shape& in2, Shape* out) {
    NN_RET_CHECK(in1.type == in2.type);
    uint32_t numberOfDims1 = getNumberOfDimensions(in1);
    uint32_t numberOfDims2 = getNumberOfDimensions(in2);
    uint32_t maxDims = std::max(numberOfDims1, numberOfDims2);
    out->dimensions = std::vector<uint32_t>(maxDims);
    for (uint32_t i = 1; i <= maxDims; i++) {
        uint32_t dim1 = 1;
        if (i <= numberOfDims1) {
            dim1 = getSizeOfDimension(in1, numberOfDims1 - i);
        }
        uint32_t dim2 = 1;
        if (i <= numberOfDims2) {
            dim2 = getSizeOfDimension(in2, numberOfDims2 - i);
        }
        if (dim1 != dim2 && dim1 != 1 && dim2 != 1) {
            LOG(ERROR) << "Dimensions mismatch for broadcast:\n"
                       << "First tensor: dimension " << numberOfDims1 - i << " of size " << dim1
                       << "\nSecond tensor: dimension " << numberOfDims2 - i << "of size " << dim2;
            return false;
        }
        out->dimensions[maxDims - i] = (dim1 == 1) ? dim2 : dim1;
    }
    return true;
}

uint8_t requantize(uint8_t value, const Shape& oldShape, const Shape& newShape) {
    double doubleValue = (value - oldShape.offset) * oldShape.scale;
    double doubleRet = doubleValue / newShape.scale + newShape.offset;
    if (doubleRet < 0) return 0;
    if (doubleRet > 255) return 255;
    return static_cast<uint8_t>(std::round(doubleRet));
}

bool floorPrepare(const Shape& input, Shape* output) {
    return SetShape(input, output);
}

bool depthwiseConvPrepare(const Shape& input, const Shape& filter, const Shape& bias,
                          int32_t padding_left, int32_t padding_right, int32_t padding_top,
                          int32_t padding_bottom, int32_t stride_width, int32_t stride_height,
                          int32_t depth_multiplier, int32_t dilation_width_factor,
                          int32_t dilation_height_factor, Shape* output) {
    if (filter.type == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
        NN_OPS_CHECK(input.type == OperandType::TENSOR_QUANT8_ASYMM);
    } else {
        NN_OPS_CHECK(input.type == filter.type);
    }
    if (input.type == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_OPS_CHECK(bias.type == OperandType::TENSOR_INT32);
    } else {
        NN_OPS_CHECK(input.type == bias.type);
    }
    NN_OPS_CHECK(getNumberOfDimensions(input) == 4);
    NN_OPS_CHECK(getNumberOfDimensions(filter) == 4);
    NN_OPS_CHECK(getNumberOfDimensions(bias) == 1);

    NN_OPS_CHECK(getSizeOfDimension(filter, 3) == getSizeOfDimension(bias, 0));

    uint32_t channels_out = getSizeOfDimension(filter, 3);
    uint32_t channels_in = getSizeOfDimension(input, 3);
    uint32_t width        = getSizeOfDimension(input, 2);
    uint32_t height       = getSizeOfDimension(input, 1);
    uint32_t filterWidth  = getSizeOfDimension(filter, 2);
    uint32_t filterHeight = getSizeOfDimension(filter, 1);
    uint32_t batches      = getSizeOfDimension(input, 0);

    NN_OPS_CHECK(depth_multiplier * channels_in == channels_out);
    int32_t effectiveFilterWidth = (filterWidth - 1) * dilation_width_factor + 1;
    int32_t effectiveFilterHeight = (filterHeight - 1) * dilation_height_factor + 1;
    NN_RET_CHECK_GT(effectiveFilterWidth, padding_left);
    NN_RET_CHECK_GT(effectiveFilterWidth, padding_right);
    NN_RET_CHECK_GT(effectiveFilterHeight, padding_top);
    NN_RET_CHECK_GT(effectiveFilterHeight, padding_bottom);

    uint32_t outWidth = computeOutSize(width, filterWidth, stride_width, dilation_width_factor,
                                       padding_left, padding_right);
    uint32_t outHeight = computeOutSize(height, filterHeight, stride_height, dilation_height_factor,
                                        padding_top, padding_bottom);

    output->type = input.type;
    output->dimensions = {batches, outHeight, outWidth, channels_out};
    return true;
}

bool genericActivationPrepare(const Shape& input,
                              Shape* output) {
    NN_OPS_CHECK(getNumberOfDimensions(input) <= 4);
    return SetShape(input, output);
}

bool genericNormalizationPrepare(const Shape& input, Shape* output) {
    return SetShape(input, output);
}

bool reshapePrepare(const Shape& input,
                    const int32_t* targetDims,
                    const int32_t targetDimsSize,
                    Shape* output) {
    // Reshape allows one of the targetDims components to have the
    // special -1 value, meaning it will be calculated automatically based on the
    // input. Here we calculate what that dimension should be so that the number
    // of output elements in the same as the number of input elements.
    int32_t numInputElements = (int32_t) getNumberOfElements(input);

    std::vector<uint32_t> outDims(targetDimsSize);
    int32_t numOutputElements = 1;
    int32_t strechDim = -1;
    for (int32_t i = 0; i < targetDimsSize; ++i) {
        int32_t value = targetDims[i];
        if (value == -1) {
            NN_OPS_CHECK(strechDim == -1);
            strechDim = i;
        } else {
            numOutputElements *= value;
            outDims[i] = (uint32_t)value;
        }
    }
    if (strechDim != -1) {
        int32_t strechValue = numInputElements / numOutputElements;
        outDims[strechDim] = (uint32_t) strechValue;
        numOutputElements *= strechValue;
    }

    NN_OPS_CHECK(numInputElements == numOutputElements);

    output->type = input.type;
    output->dimensions = outDims;
    output->offset = input.offset;
    output->scale = input.scale;

    return true;
}

bool depthToSpacePrepare(const Shape& input,
                         int32_t blockSize,
                         Shape* output) {
    NN_OPS_CHECK(getNumberOfDimensions(input) == 4);
    NN_OPS_CHECK(blockSize > 0);

    uint32_t batches  = getSizeOfDimension(input, 0);
    uint32_t height   = getSizeOfDimension(input, 1);
    uint32_t width    = getSizeOfDimension(input, 2);
    uint32_t channels = getSizeOfDimension(input, 3);

    NN_OPS_CHECK(channels % (blockSize * blockSize) == 0);
    output->type = input.type;
    output->dimensions = {batches,
                          height * blockSize,
                          width * blockSize,
                          channels / (blockSize * blockSize)};
    output->offset = input.offset;
    output->scale = input.scale;

    return true;
}

bool spaceToDepthPrepare(const Shape& input,
                         int32_t blockSize,
                         Shape* output) {
    NN_OPS_CHECK(getNumberOfDimensions(input) == 4);
    NN_OPS_CHECK(blockSize > 0);

    uint32_t batches  = getSizeOfDimension(input, 0);
    uint32_t height   = getSizeOfDimension(input, 1);
    uint32_t width    = getSizeOfDimension(input, 2);
    uint32_t channels = getSizeOfDimension(input, 3);

    NN_OPS_CHECK(height % blockSize == 0);
    NN_OPS_CHECK(width % blockSize == 0);

    output->type = input.type;
    output->dimensions = {batches,
                          height / blockSize,
                          width / blockSize,
                          channels * (blockSize * blockSize)};
    output->offset = input.offset;
    output->scale = input.scale;

    return true;
}

bool embeddingLookupPrepare(const Shape &valueShape,
                            const Shape &lookupShape,
                            Shape *outputShape) {
    NN_OPS_CHECK(getNumberOfDimensions(valueShape) >= 2);
    NN_OPS_CHECK(getNumberOfDimensions(lookupShape) == 1);

    const uint32_t rows     = getSizeOfDimension(valueShape, 0);
    const uint32_t columns  = getSizeOfDimension(valueShape, 1);

    const uint32_t lookups  = getSizeOfDimension(lookupShape, 0);

    outputShape->type = valueShape.type;
    outputShape->dimensions = { lookups, columns };
    for (uint32_t i = 2; i < getNumberOfDimensions(valueShape); i++) {
        outputShape->dimensions.push_back(getSizeOfDimension(valueShape, i));
    }
    outputShape->offset = valueShape.offset;
    outputShape->scale = valueShape.scale;

    return true;
}

bool hashtableLookupPrepare(const Shape &lookupShape,
                            const Shape &keyShape,
                            const Shape &valueShape,
                            Shape *outputShape,
                            Shape *hitShape) {
    NN_OPS_CHECK(getNumberOfDimensions(lookupShape) == 1);
    NN_OPS_CHECK(getNumberOfDimensions(keyShape) == 1);
    NN_OPS_CHECK(getNumberOfDimensions(valueShape) >= 1);

    const uint32_t lookups  = getSizeOfDimension(lookupShape, 0);
    const uint32_t keys     = getSizeOfDimension(keyShape, 0);
    const uint32_t rows     = getSizeOfDimension(valueShape, 0);
    outputShape->type = valueShape.type;
    outputShape->dimensions = { lookups };
    for (uint32_t i = 1; i < getNumberOfDimensions(valueShape); i++) {
        outputShape->dimensions.push_back(getSizeOfDimension(valueShape, i));
    }
    outputShape->offset = valueShape.offset;
    outputShape->scale = valueShape.scale;

    hitShape->type = OperandType::TENSOR_QUANT8_ASYMM;
    hitShape->dimensions = { lookups };
    hitShape->offset = 0;
    hitShape->scale = 1.f;

    return true;
}

bool padPrepare(const Shape& input,
                const int32_t* paddingsData,
                const Shape& paddingsShape,
                Shape* output) {
    uint32_t numInputDims = getNumberOfDimensions(input);

    // paddings need to be provided as a 2-D int32 tensor.
    NN_OPS_CHECK(paddingsShape.type == OperandType::TENSOR_INT32);
    NN_OPS_CHECK(getNumberOfDimensions(paddingsShape) == 2);
    NN_OPS_CHECK(getSizeOfDimension(paddingsShape, 0) == numInputDims);
    NN_OPS_CHECK(getSizeOfDimension(paddingsShape, 1) == 2);

    std::vector<uint32_t> outDims(numInputDims);
    for (uint32_t i = 0; i < numInputDims; ++i) {
        int32_t beforePadding = *paddingsData++;
        int32_t afterPadding = *paddingsData++;
        // Pad value has to be greater than equal to 0.
        NN_OPS_CHECK(beforePadding >= 0 && afterPadding >= 0);
        outDims[i] = beforePadding + getSizeOfDimension(input, i) + afterPadding;
    }
    output->type = input.type;
    output->dimensions = outDims;
    output->offset = input.offset;
    output->scale = input.scale;

    return true;
}

bool batchToSpacePrepare(const Shape& input,
                         const int32_t* blockSizeData,
                         const Shape& blockSizeShape,
                         Shape* output) {
    // Only 4D NHWC tensors are supported.
    NN_OPS_CHECK(getNumberOfDimensions(input) == 4);

    // blockSize need to be provided as a 1-D int32 tensor.
    NN_OPS_CHECK(blockSizeShape.type == OperandType::TENSOR_INT32);
    NN_OPS_CHECK(getNumberOfDimensions(blockSizeShape) == 1);
    // Only applies to spatial dimensions.
    NN_OPS_CHECK(getSizeOfDimension(blockSizeShape, 0) == 2);

    uint32_t batches  = getSizeOfDimension(input, 0);
    uint32_t height   = getSizeOfDimension(input, 1);
    uint32_t width    = getSizeOfDimension(input, 2);
    uint32_t channels = getSizeOfDimension(input, 3);

    NN_OPS_CHECK(batches % (blockSizeData[0] * blockSizeData[1]) == 0);
    output->type = input.type;
    output->dimensions = {batches / (blockSizeData[0] * blockSizeData[1]),
                          height * blockSizeData[0],
                          width * blockSizeData[1],
                          channels};
    output->offset = input.offset;
    output->scale = input.scale;

    return true;
}

bool spaceToBatchPrepare(const Shape& input,
                         const int32_t* blockSizeData,
                         const Shape& blockSizeShape,
                         const int32_t* paddingsData,
                         const Shape& paddingsShape,
                         Shape* output) {
    // Only 4D NHWC tensors are supported.
    NN_OPS_CHECK(getNumberOfDimensions(input) == 4);

    // blockSize need to be provided as a 1-D int32 tensor.
    NN_OPS_CHECK(blockSizeShape.type == OperandType::TENSOR_INT32);
    NN_OPS_CHECK(getNumberOfDimensions(blockSizeShape) == 1);
    // Only applies to spatial dimensions.
    NN_OPS_CHECK(getSizeOfDimension(blockSizeShape, 0) == 2);

    // paddings need to be provided as a 2-D int32 tensor.
    NN_OPS_CHECK(paddingsShape.type == OperandType::TENSOR_INT32);
    NN_OPS_CHECK(getNumberOfDimensions(paddingsShape) == 2);
    NN_OPS_CHECK(getSizeOfDimension(paddingsShape, 0) == 2);
    NN_OPS_CHECK(getSizeOfDimension(paddingsShape, 1) == 2);

    uint32_t batches  = getSizeOfDimension(input, 0);
    uint32_t height   = getSizeOfDimension(input, 1);
    uint32_t width    = getSizeOfDimension(input, 2);
    uint32_t channels = getSizeOfDimension(input, 3);

    uint32_t paddedHeight = paddingsData[0] + height + paddingsData[1];
    uint32_t paddedWidth = paddingsData[2] + width + paddingsData[3];

    NN_OPS_CHECK(paddedHeight % blockSizeData[0] == 0);
    NN_OPS_CHECK(paddedWidth % blockSizeData[1] == 0);

    output->type = input.type;
    output->dimensions = {batches * (blockSizeData[0] * blockSizeData[1]),
                          paddedHeight / blockSizeData[0],
                          paddedWidth / blockSizeData[1],
                          channels};
    output->offset = input.offset;
    output->scale = input.scale;

    return true;
}

bool squeezePrepare(const Shape& input,
                    const int32_t* squeezeDims,
                    const Shape& squeezeDimsShape,
                    Shape* output) {
    int32_t numInputDims = static_cast<int32_t>(getNumberOfDimensions(input));

    // squeezeDims need to be provided as a 1-D int32 tensor.
    NN_OPS_CHECK(squeezeDimsShape.type == OperandType::TENSOR_INT32);
    NN_OPS_CHECK(getNumberOfDimensions(squeezeDimsShape) == 1);

    int32_t squeezeDimsSize = static_cast<int32_t>(getSizeOfDimension(squeezeDimsShape, 0));
    std::vector<bool> shouldSqueeze(numInputDims, false);
    int32_t numDimsSqueezed = 0;

    if (squeezeDimsSize == 0) {
        // If squeezeDimsSize is 0, all dims with value 1 will be squeezed.
        for (int32_t idx = 0; idx < numInputDims; ++idx) {
            if (getSizeOfDimension(input, idx) == 1) {
                shouldSqueeze[idx] = true;
                ++numDimsSqueezed;
            }
        }
    } else {
        for (int32_t idx = 0; idx < squeezeDimsSize; ++idx) {
            int32_t current = squeezeDims[idx] < 0 ? squeezeDims[idx] + numInputDims
                                               : squeezeDims[idx];
            NN_OPS_CHECK(current >= 0 && current < numInputDims &&
                         getSizeOfDimension(input, current) == 1);
            if (!shouldSqueeze[current]) ++numDimsSqueezed;
            shouldSqueeze[current] = true;
      }
    }

    // Sets output dimensions.
    std::vector<uint32_t> outDims(numInputDims - numDimsSqueezed);
    for (int32_t inIdx = 0, outIdx = 0; inIdx < numInputDims; ++inIdx) {
        if (!shouldSqueeze[inIdx]) {
            outDims[outIdx++] = getSizeOfDimension(input, inIdx);
        }
    }

    output->type = input.type;
    output->dimensions = outDims;
    output->offset = input.offset;
    output->scale = input.scale;

    return true;
}

bool meanPrepare(const Shape& input,
                 const int32_t* axisData,
                 const Shape& axisShape,
                 bool keepDims,
                 Shape* output) {

    // perm need to be provided as a 1-D int32 tensor.
    NN_OPS_CHECK(axisShape.type == OperandType::TENSOR_INT32);
    NN_OPS_CHECK(getNumberOfDimensions(axisShape) == 1);

    int32_t numInputDims = static_cast<int32_t>(getNumberOfDimensions(input));
    int32_t axisSize = static_cast<int32_t>(getSizeOfDimension(axisShape, 0));

    // Determines size of output tensor.
    if (keepDims) {
        std::vector<uint32_t> outDims(numInputDims);
        for (int32_t idx = 0; idx < numInputDims; ++idx) {
            bool isAxis = false;
            for (int32_t axisIdx = 0; axisIdx < axisSize; ++axisIdx) {
                if (axisData[axisIdx] == idx || axisData[axisIdx] + numInputDims == idx) {
                    isAxis = true;
                    break;
                }
            }
            if (isAxis) {
                outDims[idx] = 1;
            } else {
                outDims[idx] = getSizeOfDimension(input, idx);
            }
        }
        output->dimensions = outDims;
    } else {
        // Calculates size of reducing axis.
        int32_t numReduceAxis = axisSize;
        for (int32_t i = 0; i < axisSize; ++i) {
            int32_t current = axisData[i];
            if (current < 0) {
                current += numInputDims;
            }
            NN_OPS_CHECK(current >= 0 && current < numInputDims);
            for (int32_t j = 0; j < i; ++j) {
                int32_t previous = axisData[j];
                if (previous < 0) {
                    previous += numInputDims;
                }
                if (current == previous) {
                    --numReduceAxis;
                    break;
                }
            }
        }
        // Determines output dimensions.
        std::vector<uint32_t> outDims(numInputDims - numReduceAxis);
        int32_t numSkipAxis = 0;
        for (int32_t idx = 0; idx < numInputDims; ++idx) {
            bool isAxis = false;
            for (int32_t axisIdx = 0; axisIdx < axisSize; ++axisIdx) {
                if (axisData[axisIdx] == idx || axisData[axisIdx] + numInputDims == idx) {
                    ++numSkipAxis;
                    isAxis = true;
                    break;
                }
            }
            if (!isAxis) {
                outDims[idx - numSkipAxis] = getSizeOfDimension(input, idx);
            }
        }
        output->dimensions = outDims;
    }

    output->type = input.type;
    output->offset = input.offset;
    output->scale = input.scale;

    return true;
}

bool stridedSlicePrepare(const Shape& input,
                         const int32_t* beginData, const Shape& beginShape,
                         const int32_t* endData, const Shape& endShape,
                         const int32_t* stridesData, const Shape& stridesShape,
                         int32_t beginMask, int32_t endMask, int32_t shrinkAxisMask,
                         Shape* output) {
    uint32_t numInputDims = getNumberOfDimensions(input);
    // StridedSlice op only supports 1D-4D input arrays.
    NN_OPS_CHECK(numInputDims <= 4);

    NN_OPS_CHECK(getNumberOfDimensions(beginShape) == 1);
    NN_OPS_CHECK(getNumberOfDimensions(endShape) == 1);
    NN_OPS_CHECK(getNumberOfDimensions(stridesShape) == 1);

    NN_OPS_CHECK(getSizeOfDimension(beginShape, 0) == numInputDims);
    NN_OPS_CHECK(getSizeOfDimension(endShape, 0) == numInputDims);
    NN_OPS_CHECK(getSizeOfDimension(stridesShape, 0) == numInputDims);

    NN_OPS_CHECK(beginShape.type == OperandType::TENSOR_INT32);
    NN_OPS_CHECK(endShape.type == OperandType::TENSOR_INT32);
    NN_OPS_CHECK(stridesShape.type == OperandType::TENSOR_INT32);

    // Determine size of output tensor and map indices
    std::vector<uint32_t> outDims;
    for (int32_t idx = 0; idx < static_cast<int32_t>(numInputDims); idx++) {
      int32_t dim = static_cast<int32_t>(getSizeOfDimension(input, idx));
      int32_t stride = stridesData[idx];
      // stride value has to be non-zero
      NN_OPS_CHECK(stride != 0);
      bool positiveStride = stride > 0;

      int32_t begin = beginMask & (1 << idx)
              ? positiveStride ? 0 : dim - 1
              : ClampedIndex(beginData[idx], dim, positiveStride);
      int32_t end = endMask & (1 << idx)
              ? positiveStride ? dim : -1
              : ClampedIndex(endData[idx], dim, positiveStride);

      // This is valid for both positive and negative strides
      int32_t outDim = ceil((end - begin) / static_cast<float>(stride));
      outDim = outDim < 0 ? 0 : static_cast<uint32_t>(outDim);
      if (!(shrinkAxisMask & (1 << idx))) {
          outDims.push_back(outDim);
      } else {
          if (outDim != 1) {
              LOG(ERROR) << "Outdim " << idx << " is " << outDim << ", expected 1";
              NN_OPS_CHECK(outDim == 1);
          }
      }
    }

    output->type = input.type;
    output->dimensions = outDims;
    output->offset = input.offset;
    output->scale = input.scale;

    return true;
}

bool argMinMaxPrepare(const Shape& input, int32_t axis, Shape* output) {
    NN_CHECK(handleNegativeAxis(input, &axis));

    output->type = OperandType::TENSOR_INT32;

    // Copy the input dimensions, omitting the axis dimension.
    output->dimensions.clear();
    output->dimensions.reserve(getNumberOfDimensions(input) - 1);
    output->dimensions.insert(output->dimensions.end(),
                              input.dimensions.begin(),
                              input.dimensions.begin() + axis);
    output->dimensions.insert(output->dimensions.end(),
                              input.dimensions.begin() + axis + 1,
                              input.dimensions.end());

    return true;
}

bool splitPrepare(const Shape& input, int32_t axis, int32_t numOutputs,
                  std::vector<Shape>* output) {
    NN_CHECK(handleNegativeAxis(input, &axis));

    const int32_t sizeOfAxisToSplit = input.dimensions[axis];
    NN_OPS_CHECK(sizeOfAxisToSplit % numOutputs == 0);
    const int32_t sliceSize = sizeOfAxisToSplit / numOutputs;

    for (int i = 0; i < numOutputs; ++i) {
        output->at(i).type = input.type;
        output->at(i).dimensions = input.dimensions;
        output->at(i).dimensions[axis] = sliceSize;
        output->at(i).offset = input.offset;
        output->at(i).scale = input.scale;
    }
    return true;
}

bool groupedConvPrepare(const Shape& input, const Shape& filter, const Shape& bias,
                        int32_t padding_left, int32_t padding_right, int32_t padding_top,
                        int32_t padding_bottom, int32_t stride_width, int32_t stride_height,
                        int32_t numGroups, Shape* output) {
    if (filter.type == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
        NN_OPS_CHECK(input.type == OperandType::TENSOR_QUANT8_ASYMM);
    } else {
        NN_OPS_CHECK(input.type == filter.type);
    }
    if (input.type == OperandType::TENSOR_QUANT8_ASYMM) {
        NN_OPS_CHECK(bias.type == OperandType::TENSOR_INT32);
    } else {
        NN_OPS_CHECK(input.type == bias.type);
    }
    NN_OPS_CHECK(getNumberOfDimensions(input) == 4);
    NN_OPS_CHECK(getNumberOfDimensions(filter) == 4);
    NN_OPS_CHECK(getNumberOfDimensions(bias) == 1);

    NN_OPS_CHECK(getSizeOfDimension(filter, 0) == getSizeOfDimension(bias, 0));

    NN_OPS_CHECK(getSizeOfDimension(filter, 3) * numGroups == getSizeOfDimension(input, 3));
    NN_OPS_CHECK(getSizeOfDimension(filter, 0) % numGroups == 0);

    uint32_t channels_out = getSizeOfDimension(filter, 0);
    uint32_t width = getSizeOfDimension(input, 2);
    uint32_t height = getSizeOfDimension(input, 1);
    uint32_t filterWidth = getSizeOfDimension(filter, 2);
    uint32_t filterHeight = getSizeOfDimension(filter, 1);
    uint32_t batches = getSizeOfDimension(input, 0);

    NN_RET_CHECK_GT(static_cast<int32_t>(filterWidth), padding_left);
    NN_RET_CHECK_GT(static_cast<int32_t>(filterWidth), padding_right);
    NN_RET_CHECK_GT(static_cast<int32_t>(filterHeight), padding_top);
    NN_RET_CHECK_GT(static_cast<int32_t>(filterHeight), padding_bottom);

    uint32_t outWidth =
            computeOutSize(width, filterWidth, stride_width, padding_left, padding_right);
    uint32_t outHeight =
            computeOutSize(height, filterHeight, stride_height, padding_top, padding_bottom);

    output->type = input.type;
    output->dimensions = {batches, outHeight, outWidth, channels_out};
    return true;
}

} // namespace nn
} // namespace android
