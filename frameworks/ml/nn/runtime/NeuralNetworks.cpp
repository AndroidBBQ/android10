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

// Contains all the entry points to the C Neural Networks API.
// We do basic validation of the operands and then call the class
// that implements the functionality.

#define LOG_TAG "NeuralNetworks"

#include "NeuralNetworks.h"

#include "BurstBuilder.h"
#include "Callbacks.h"
#include "CompilationBuilder.h"
#include "ExecutionBuilder.h"
#include "Manager.h"
#include "Memory.h"
#include "ModelBuilder.h"
#include "NeuralNetworksExtensions.h"
#include "NeuralNetworksOEM.h"
#include "Tracing.h"
#include "Utils.h"

#include "vndk/hardware_buffer.h"

#include <cstddef>
#include <memory>
#include <vector>

// Make sure the constants defined in the header files have not changed values.
// IMPORTANT: When adding new values, update kNumberOfDataTypes or kNumberOfDataTypesOEM
// in Utils.h.
static_assert(ANEURALNETWORKS_FLOAT32 == 0, "ANEURALNETWORKS_FLOAT32 has changed");
static_assert(ANEURALNETWORKS_INT32 == 1, "ANEURALNETWORKS_INT32 has changed");
static_assert(ANEURALNETWORKS_UINT32 == 2, "ANEURALNETWORKS_UINT32 has changed");
static_assert(ANEURALNETWORKS_TENSOR_FLOAT32 == 3, "ANEURALNETWORKS_TENSOR_FLOAT32 has changed");
static_assert(ANEURALNETWORKS_TENSOR_INT32 == 4, "ANEURALNETWORKS_TENSOR_INT32 has changed");
static_assert(ANEURALNETWORKS_TENSOR_QUANT8_ASYMM == 5,
              "ANEURALNETWORKS_TENSOR_QUANT8_ASYMM has changed");
static_assert(ANEURALNETWORKS_BOOL == 6, "ANEURALNETWORKS_BOOL has changed");
static_assert(ANEURALNETWORKS_TENSOR_QUANT16_SYMM == 7,
              "ANEURALNETWORKS_TENSOR_QUANT16_SYMM has changed");
static_assert(ANEURALNETWORKS_TENSOR_FLOAT16 == 8, "ANEURALNETWORKS_TENSOR_FLOAT16 has changed");
static_assert(ANEURALNETWORKS_TENSOR_BOOL8 == 9, "ANEURALNETWORKS_TENSOR_BOOL8 has changed");
static_assert(ANEURALNETWORKS_FLOAT16 == 10, "ANEURALNETWORKS_FLOAT16 has changed");
static_assert(ANEURALNETWORKS_TENSOR_QUANT8_SYMM_PER_CHANNEL == 11,
              "ANEURALNETWORKS_TENSOR_QUANT8_SYMM_PER_CHANNEL has changed");
static_assert(ANEURALNETWORKS_TENSOR_QUANT16_ASYMM == 12,
              "ANEURALNETWORKS_TENSOR_QUANT16_ASYMM has changed");
static_assert(ANEURALNETWORKS_TENSOR_QUANT8_SYMM == 13,
              "ANEURALNETWORKS_TENSOR_QUANT8_SYMM has changed");
static_assert(ANEURALNETWORKS_OEM_SCALAR == 10000, "ANEURALNETWORKS_OEM_SCALAR has changed");
static_assert(ANEURALNETWORKS_TENSOR_OEM_BYTE == 10001,
              "ANEURALNETWORKS_TENSOR_OEM_BYTE has changed");

// IMPORTANT: When adding new values, update kNumberOfOperationTypes or
// kNumberOfOperationTypesOEMin Utils.h.
static_assert(ANEURALNETWORKS_ADD == 0, "ANEURALNETWORKS_ADD has changed");
static_assert(ANEURALNETWORKS_AVERAGE_POOL_2D == 1, "ANEURALNETWORKS_AVERAGE_POOL_2D has changed");
static_assert(ANEURALNETWORKS_CONCATENATION == 2, "ANEURALNETWORKS_CONCATENATION has changed");
static_assert(ANEURALNETWORKS_CONV_2D == 3, "ANEURALNETWORKS_CONV_2D has changed");
static_assert(ANEURALNETWORKS_DEPTHWISE_CONV_2D == 4,
              "ANEURALNETWORKS_DEPTHWISE_CONV_2D has changed");
static_assert(ANEURALNETWORKS_DEPTH_TO_SPACE == 5, "ANEURALNETWORKS_DEPTH_TO_SPACE has changed");
static_assert(ANEURALNETWORKS_DEQUANTIZE == 6, "ANEURALNETWORKS_DEQUANTIZE has changed");
static_assert(ANEURALNETWORKS_EMBEDDING_LOOKUP == 7,
              "ANEURALNETWORKS_EMBEDDING_LOOKUP has changed");
static_assert(ANEURALNETWORKS_FLOOR == 8, "ANEURALNETWORKS_FLOOR has changed");
static_assert(ANEURALNETWORKS_FULLY_CONNECTED == 9, "ANEURALNETWORKS_FULLY_CONNECTED has changed");
static_assert(ANEURALNETWORKS_HASHTABLE_LOOKUP == 10,
              "ANEURALNETWORKS_HASHTABLE_LOOKUP has changed");
static_assert(ANEURALNETWORKS_L2_NORMALIZATION == 11,
              "ANEURALNETWORKS_L2_NORMALIZATION has changed");
static_assert(ANEURALNETWORKS_L2_POOL_2D == 12, "ANEURALNETWORKS_L2_POOL has changed");
static_assert(ANEURALNETWORKS_LOCAL_RESPONSE_NORMALIZATION == 13,
              "ANEURALNETWORKS_LOCAL_RESPONSE_NORMALIZATION has changed");
static_assert(ANEURALNETWORKS_LOGISTIC == 14, "ANEURALNETWORKS_LOGISTIC has changed");
static_assert(ANEURALNETWORKS_LSH_PROJECTION == 15, "ANEURALNETWORKS_LSH_PROJECTION has changed");
static_assert(ANEURALNETWORKS_LSTM == 16, "ANEURALNETWORKS_LSTM has changed");
static_assert(ANEURALNETWORKS_MAX_POOL_2D == 17, "ANEURALNETWORKS_MAX_POOL has changed");
static_assert(ANEURALNETWORKS_MUL == 18, "ANEURALNETWORKS_MUL has changed");
static_assert(ANEURALNETWORKS_RELU == 19, "ANEURALNETWORKS_RELU has changed");
static_assert(ANEURALNETWORKS_RELU1 == 20, "ANEURALNETWORKS_RELU1 has changed");
static_assert(ANEURALNETWORKS_RELU6 == 21, "ANEURALNETWORKS_RELU6 has changed");
static_assert(ANEURALNETWORKS_RESHAPE == 22, "ANEURALNETWORKS_RESHAPE has changed");
static_assert(ANEURALNETWORKS_RESIZE_BILINEAR == 23, "ANEURALNETWORKS_RESIZE_BILINEAR has changed");
static_assert(ANEURALNETWORKS_RNN == 24, "ANEURALNETWORKS_RNN has changed");
static_assert(ANEURALNETWORKS_SOFTMAX == 25, "ANEURALNETWORKS_SOFTMAX has changed");
static_assert(ANEURALNETWORKS_SPACE_TO_DEPTH == 26, "ANEURALNETWORKS_SPACE_TO_DEPTH has changed");
static_assert(ANEURALNETWORKS_SVDF == 27, "ANEURALNETWORKS_SVDF has changed");
static_assert(ANEURALNETWORKS_TANH == 28, "ANEURALNETWORKS_TANH has changed");

static_assert(ANEURALNETWORKS_BATCH_TO_SPACE_ND == 29,
              "ANEURALNETWORKS_BATCH_TO_SPACE_ND has changed");
static_assert(ANEURALNETWORKS_DIV == 30, "ANEURALNETWORKS_DIV has changed");
static_assert(ANEURALNETWORKS_MEAN == 31, "ANEURALNETWORKS_MEAN has changed");
static_assert(ANEURALNETWORKS_PAD == 32, "ANEURALNETWORKS_PAD has changed");
static_assert(ANEURALNETWORKS_SPACE_TO_BATCH_ND == 33,
              "ANEURALNETWORKS_SPACE_TO_BATCH_ND has changed");
static_assert(ANEURALNETWORKS_SQUEEZE == 34, "ANEURALNETWORKS_SQUEEZE has changed");
static_assert(ANEURALNETWORKS_STRIDED_SLICE == 35, "ANEURALNETWORKS_STRIDED_SLICE has changed");
static_assert(ANEURALNETWORKS_SUB == 36, "ANEURALNETWORKS_TANH has changed");
static_assert(ANEURALNETWORKS_TRANSPOSE == 37, "ANEURALNETWORKS_TRANSPOSE has changed");

static_assert(ANEURALNETWORKS_ABS == 38, "ANEURALNETWORKS_ABS has changed");
static_assert(ANEURALNETWORKS_ARGMAX == 39, "ANEURALNETWORKS_ARGMAX has changed");
static_assert(ANEURALNETWORKS_ARGMIN == 40, "ANEURALNETWORKS_ARGMIN has changed");
static_assert(ANEURALNETWORKS_AXIS_ALIGNED_BBOX_TRANSFORM == 41,
              "ANEURALNETWORKS_AXIS_ALIGNED_BBOX_TRANSFORM has changed");
static_assert(ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_LSTM == 42,
              "ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_LSTM has changed");
static_assert(ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_RNN == 43,
              "ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_RNN has changed");
static_assert(ANEURALNETWORKS_BOX_WITH_NMS_LIMIT == 44,
              "ANEURALNETWORKS_BOX_WITH_NMS_LIMIT has changed");
static_assert(ANEURALNETWORKS_CAST == 45, "ANEURALNETWORKS_CAST has changed");
static_assert(ANEURALNETWORKS_CHANNEL_SHUFFLE == 46, "ANEURALNETWORKS_CHANNEL_SHUFFLE has changed");
static_assert(ANEURALNETWORKS_DETECTION_POSTPROCESSING == 47,
              "ANEURALNETWORKS_DETECTION_POSTPROCESSING has changed");
static_assert(ANEURALNETWORKS_EQUAL == 48, "ANEURALNETWORKS_EQUAL has changed");
static_assert(ANEURALNETWORKS_EXP == 49, "ANEURALNETWORKS_EXP has changed");
static_assert(ANEURALNETWORKS_EXPAND_DIMS == 50, "ANEURALNETWORKS_EXPAND_DIMS has changed");
static_assert(ANEURALNETWORKS_GATHER == 51, "ANEURALNETWORKS_GATHER has changed");
static_assert(ANEURALNETWORKS_GENERATE_PROPOSALS == 52,
              "ANEURALNETWORKS_GENERATE_PROPOSALS has changed");
static_assert(ANEURALNETWORKS_GREATER == 53, "ANEURALNETWORKS_GREATER has changed");
static_assert(ANEURALNETWORKS_GREATER_EQUAL == 54, "ANEURALNETWORKS_GREATER_EQUAL has changed");
static_assert(ANEURALNETWORKS_GROUPED_CONV_2D == 55, "ANEURALNETWORKS_GROUPED_CONV_2D has changed");
static_assert(ANEURALNETWORKS_HEATMAP_MAX_KEYPOINT == 56,
              "ANEURALNETWORKS_HEATMAP_MAX_KEYPOINT has changed");
static_assert(ANEURALNETWORKS_INSTANCE_NORMALIZATION == 57,
              "ANEURALNETWORKS_INSTANCE_NORMALIZATION has changed");
static_assert(ANEURALNETWORKS_LESS == 58, "ANEURALNETWORKS_LESS has changed");
static_assert(ANEURALNETWORKS_LESS_EQUAL == 59, "ANEURALNETWORKS_LESS_EQUAL has changed");
static_assert(ANEURALNETWORKS_LOG == 60, "ANEURALNETWORKS_LOG has changed");
static_assert(ANEURALNETWORKS_LOGICAL_AND == 61, "ANEURALNETWORKS_LOGICAL_AND has changed");
static_assert(ANEURALNETWORKS_LOGICAL_NOT == 62, "ANEURALNETWORKS_LOGICAL_NOT has changed");
static_assert(ANEURALNETWORKS_LOGICAL_OR == 63, "ANEURALNETWORKS_LOGICAL_OR has changed");
static_assert(ANEURALNETWORKS_LOG_SOFTMAX == 64, "ANEURALNETWORKS_LOG_SOFTMAX has changed");
static_assert(ANEURALNETWORKS_MAXIMUM == 65, "ANEURALNETWORKS_MAXIMUM has changed");
static_assert(ANEURALNETWORKS_MINIMUM == 66, "ANEURALNETWORKS_MINIMUM has changed");
static_assert(ANEURALNETWORKS_NEG == 67, "ANEURALNETWORKS_NEG has changed");
static_assert(ANEURALNETWORKS_NOT_EQUAL == 68, "ANEURALNETWORKS_NOT_EQUAL has changed");
static_assert(ANEURALNETWORKS_PAD_V2 == 69, "ANEURALNETWORKS_PAD_V2 has changed");
static_assert(ANEURALNETWORKS_POW == 70, "ANEURALNETWORKS_POW has changed");
static_assert(ANEURALNETWORKS_PRELU == 71, "ANEURALNETWORKS_PRELU has changed");
static_assert(ANEURALNETWORKS_QUANTIZE == 72, "ANEURALNETWORKS_QUANTIZE has changed");
static_assert(ANEURALNETWORKS_QUANTIZED_16BIT_LSTM == 73,
              "ANEURALNETWORKS_QUANTIZED_16BIT_LSTM has changed");
static_assert(ANEURALNETWORKS_RANDOM_MULTINOMIAL == 74,
              "ANEURALNETWORKS_RANDOM_MULTINOMIAL has changed");
static_assert(ANEURALNETWORKS_REDUCE_ALL == 75, "ANEURALNETWORKS_REDUCE_ALL has changed");
static_assert(ANEURALNETWORKS_REDUCE_ANY == 76, "ANEURALNETWORKS_REDUCE_ANY has changed");
static_assert(ANEURALNETWORKS_REDUCE_MAX == 77, "ANEURALNETWORKS_REDUCE_MAX has changed");
static_assert(ANEURALNETWORKS_REDUCE_MIN == 78, "ANEURALNETWORKS_REDUCE_MIN has changed");
static_assert(ANEURALNETWORKS_REDUCE_PROD == 79, "ANEURALNETWORKS_REDUCE_PROD has changed");
static_assert(ANEURALNETWORKS_REDUCE_SUM == 80, "ANEURALNETWORKS_REDUCE_SUM has changed");
static_assert(ANEURALNETWORKS_ROI_ALIGN == 81, "ANEURALNETWORKS_ROI_ALIGN has changed");
static_assert(ANEURALNETWORKS_ROI_POOLING == 82, "ANEURALNETWORKS_ROI_POOLING has changed");
static_assert(ANEURALNETWORKS_RSQRT == 83, "ANEURALNETWORKS_RSQRT has changed");
static_assert(ANEURALNETWORKS_SELECT == 84, "ANEURALNETWORKS_SELECT has changed");
static_assert(ANEURALNETWORKS_SIN == 85, "ANEURALNETWORKS_SIN has changed");
static_assert(ANEURALNETWORKS_SLICE == 86, "ANEURALNETWORKS_SLICE has changed");
static_assert(ANEURALNETWORKS_SPLIT == 87, "ANEURALNETWORKS_SPLIT has changed");
static_assert(ANEURALNETWORKS_SQRT == 88, "ANEURALNETWORKS_SQRT has changed");
static_assert(ANEURALNETWORKS_TILE == 89, "ANEURALNETWORKS_TILE has changed");
static_assert(ANEURALNETWORKS_TOPK_V2 == 90, "ANEURALNETWORKS_TOPK_V2 has changed");
static_assert(ANEURALNETWORKS_TRANSPOSE_CONV_2D == 91,
              "ANEURALNETWORKS_TRANSPOSE_CONV_2D has changed");
static_assert(ANEURALNETWORKS_UNIDIRECTIONAL_SEQUENCE_LSTM == 92,
              "ANEURALNETWORKS_UNIDIRECTIONAL_SEQUENCE_LSTM has changed");
static_assert(ANEURALNETWORKS_UNIDIRECTIONAL_SEQUENCE_RNN == 93,
              "ANEURALNETWORKS_UNIDIRECTIONAL_SEQUENCE_RNN has changed");

static_assert(ANEURALNETWORKS_OEM_OPERATION == 10000, "ANEURALNETWORKS_OEM_OPERATION has changed");

static_assert(ANEURALNETWORKS_FUSED_NONE == 0, "ANEURALNETWORKS_FUSED_NONE has changed");
static_assert(ANEURALNETWORKS_FUSED_RELU == 1, "ANEURALNETWORKS_FUSED_RELU has changed");
static_assert(ANEURALNETWORKS_FUSED_RELU1 == 2, "ANEURALNETWORKS_FUSED_RELU1 has changed");
static_assert(ANEURALNETWORKS_FUSED_RELU6 == 3, "ANEURALNETWORKS_FUSED_RELU6 has changed");

static_assert(ANEURALNETWORKS_PREFER_LOW_POWER == 0,
              "ANEURALNETWORKS_PREFER_LOW_POWER has changed");
static_assert(ANEURALNETWORKS_PREFER_FAST_SINGLE_ANSWER == 1,
              "ANEURALNETWORKS_PREFER_FAST_SINGLE_ANSWER has changed");
static_assert(ANEURALNETWORKS_PREFER_SUSTAINED_SPEED == 2,
              "ANEURALNETWORKS_PREFER_SUSTAINED_SPEED has changed");

static_assert(ANEURALNETWORKS_NO_ERROR == 0, "ANEURALNETWORKS_NO_ERROR has changed");
static_assert(ANEURALNETWORKS_OUT_OF_MEMORY == 1, "ANEURALNETWORKS_OUT_OF_MEMORY has changed");
static_assert(ANEURALNETWORKS_INCOMPLETE == 2, "ANEURALNETWORKS_INCOMPLETE has changed");
static_assert(ANEURALNETWORKS_UNEXPECTED_NULL == 3, "ANEURALNETWORKS_UNEXPECTED_NULL has changed");
static_assert(ANEURALNETWORKS_BAD_DATA == 4, "ANEURALNETWORKS_BAD_DATA has changed");
static_assert(ANEURALNETWORKS_OP_FAILED == 5, "ANEURALNETWORKS_OP_FAILED has changed");
static_assert(ANEURALNETWORKS_BAD_STATE == 6, "ANEURALNETWORKS_BAD_STATE has changed");
static_assert(ANEURALNETWORKS_UNMAPPABLE == 7, "ANEURALNETWORKS_UNMAPPABLE has changed");
static_assert(ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE == 8,
              "ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE has changed");
static_assert(ANEURALNETWORKS_UNAVAILABLE_DEVICE == 9,
              "ANEURALNETWORKS_UNAVAILABLE_DEVICE has changed");

static_assert(ANEURALNETWORKS_MAX_SIZE_OF_IMMEDIATELY_COPIED_VALUES == 128,
              "ANEURALNETWORKS_MAX_SIZE_OF_IMMEDIATELY_COPIED_VALUES has changed");

static_assert(ANEURALNETWORKS_DEVICE_UNKNOWN == 0, "ANEURALNETWORKS_DEVICE_UNKNOWN has changed");
static_assert(ANEURALNETWORKS_DEVICE_OTHER == 1, "ANEURALNETWORKS_DEVICE_OTHER has changed");
static_assert(ANEURALNETWORKS_DEVICE_CPU == 2, "ANEURALNETWORKS_DEVICE_CPU has changed");
static_assert(ANEURALNETWORKS_DEVICE_GPU == 3, "ANEURALNETWORKS_DEVICE_GPU has changed");
static_assert(ANEURALNETWORKS_DEVICE_ACCELERATOR == 4,
              "ANEURALNETWORKS_DEVICE_ACCELERATOR has changed");

static_assert(ANEURALNETWORKS_DURATION_ON_HARDWARE == 0,
              "ANEURALNETWORKS_DURATION_ON_HARDWARE has changed");
static_assert(ANEURALNETWORKS_DURATION_IN_DRIVER == 1,
              "ANEURALNETWORKS_DURATION_IN_DRIVER has changed");

// Make sure that the constants are compatible with the values defined in
// hardware/interfaces/neuralnetworks/1.0/types.hal.
static_assert(static_cast<int32_t>(OperandType::OEM) == ANEURALNETWORKS_OEM_SCALAR,
              "OEM != ANEURALNETWORKS_OEM");
static_assert(static_cast<int32_t>(OperandType::FLOAT32) == ANEURALNETWORKS_FLOAT32,
              "FLOAT32 != ANEURALNETWORKS_FLOAT32");
static_assert(static_cast<int32_t>(OperandType::INT32) == ANEURALNETWORKS_INT32,
              "INT32 != ANEURALNETWORKS_INT32");
static_assert(static_cast<int32_t>(OperandType::UINT32) == ANEURALNETWORKS_UINT32,
              "UINT32 != ANEURALNETWORKS_UINT32");
static_assert(static_cast<int32_t>(OperandType::TENSOR_OEM_BYTE) == ANEURALNETWORKS_TENSOR_OEM_BYTE,
              "TENSOR_OEM_BYTE != ANEURALNETWORKS_TENSOR_OEM_BYTE");
static_assert(static_cast<int32_t>(OperandType::TENSOR_FLOAT32) == ANEURALNETWORKS_TENSOR_FLOAT32,
              "TENSOR_FLOAT32 != ANEURALNETWORKS_TENSOR_FLOAT32");
static_assert(static_cast<int32_t>(OperandType::TENSOR_QUANT8_ASYMM) ==
                      ANEURALNETWORKS_TENSOR_QUANT8_ASYMM,
              "TENSOR_QUANT8_ASYMM != ANEURALNETWORKS_TENSOR_QUANT8_ASYMM");

static_assert(static_cast<int32_t>(OperationType::ADD) == ANEURALNETWORKS_ADD,
              "OperationType::ADD != ANEURALNETWORKS_ADD");
static_assert(static_cast<int32_t>(OperationType::AVERAGE_POOL_2D) ==
                      ANEURALNETWORKS_AVERAGE_POOL_2D,
              "OperationType::AVERAGE_POOL_2D != ANEURALNETWORKS_AVERAGE_POOL_2D");
static_assert(static_cast<int32_t>(OperationType::CONV_2D) == ANEURALNETWORKS_CONV_2D,
              "OperationType::CONV_2D != ANEURALNETWORKS_CONV_2D");
static_assert(static_cast<int32_t>(OperationType::DEPTHWISE_CONV_2D) ==
                      ANEURALNETWORKS_DEPTHWISE_CONV_2D,
              "OperationType::DEPTHWISE_CONV_2D != ANEURALNETWORKS_DEPTHWISE_CONV_2D");
static_assert(static_cast<int32_t>(OperationType::DEPTH_TO_SPACE) == ANEURALNETWORKS_DEPTH_TO_SPACE,
              "OperationType::DEPTH_TO_SPACE != ANEURALNETWORKS_DEPTH_TO_SPACE");
static_assert(static_cast<int32_t>(OperationType::DEQUANTIZE) == ANEURALNETWORKS_DEQUANTIZE,
              "OperationType::DEQUANTIZE != ANEURALNETWORKS_DEQUANTIZE");
static_assert(static_cast<int32_t>(OperationType::EMBEDDING_LOOKUP) ==
                      ANEURALNETWORKS_EMBEDDING_LOOKUP,
              "OperationType::EMBEDDING_LOOKUP != ANEURALNETWORKS_EMBEDDING_LOOKUP");
static_assert(static_cast<int32_t>(OperationType::FLOOR) == ANEURALNETWORKS_FLOOR,
              "OperationType::FLOOR != ANEURALNETWORKS_FLOOR");
static_assert(static_cast<int32_t>(OperationType::FULLY_CONNECTED) ==
                      ANEURALNETWORKS_FULLY_CONNECTED,
              "OperationType::FULLY_CONNECTED != ANEURALNETWORKS_FULLY_CONNECTED");
static_assert(static_cast<int32_t>(OperationType::HASHTABLE_LOOKUP) ==
                      ANEURALNETWORKS_HASHTABLE_LOOKUP,
              "OperationType::HASHTABLE_LOOKUP != ANEURALNETWORKS_HASHTABLE_LOOKUP");
static_assert(static_cast<int32_t>(OperationType::L2_NORMALIZATION) ==
                      ANEURALNETWORKS_L2_NORMALIZATION,
              "OperationType::L2_NORMALIZATION != ANEURALNETWORKS_L2_NORMALIZATION");
static_assert(static_cast<int32_t>(OperationType::L2_POOL_2D) == ANEURALNETWORKS_L2_POOL_2D,
              "OperationType::L2_POOL_2D != ANEURALNETWORKS_L2_POOL_2D");
static_assert(static_cast<int32_t>(OperationType::LOCAL_RESPONSE_NORMALIZATION) ==
                      ANEURALNETWORKS_LOCAL_RESPONSE_NORMALIZATION,
              "OperationType::LOCAL_RESPONSE_NORMALIZATION != "
              "ANEURALNETWORKS_LOCAL_RESPONSE_NORMALIZATION");
static_assert(static_cast<int32_t>(OperationType::LOGISTIC) == ANEURALNETWORKS_LOGISTIC,
              "OperationType::LOGISTIC != ANEURALNETWORKS_LOGISTIC");
static_assert(static_cast<int32_t>(OperationType::LSH_PROJECTION) == ANEURALNETWORKS_LSH_PROJECTION,
              "OperationType::LSH_PROJECTION != ANEURALNETWORKS_LSH_PROJECTION");
static_assert(static_cast<int32_t>(OperationType::LSTM) == ANEURALNETWORKS_LSTM,
              "OperationType::LSTM != ANEURALNETWORKS_LSTM");
static_assert(static_cast<int32_t>(OperationType::MAX_POOL_2D) == ANEURALNETWORKS_MAX_POOL_2D,
              "OperationType::MAX_POOL_2D != ANEURALNETWORKS_MAX_POOL_2D");
static_assert(static_cast<int32_t>(OperationType::MUL) == ANEURALNETWORKS_MUL,
              "OperationType::MUL != ANEURALNETWORKS_MUL");
static_assert(static_cast<int32_t>(OperationType::RELU) == ANEURALNETWORKS_RELU,
              "OperationType::RELU != ANEURALNETWORKS_RELU");
static_assert(static_cast<int32_t>(OperationType::RELU1) == ANEURALNETWORKS_RELU1,
              "OperationType::RELU1 != ANEURALNETWORKS_RELU1");
static_assert(static_cast<int32_t>(OperationType::RELU6) == ANEURALNETWORKS_RELU6,
              "OperationType::RELU6 != ANEURALNETWORKS_RELU6");
static_assert(static_cast<int32_t>(OperationType::RESHAPE) == ANEURALNETWORKS_RESHAPE,
              "OperationType::RESHAPE != ANEURALNETWORKS_RESHAPE");
static_assert(static_cast<int32_t>(OperationType::RESIZE_BILINEAR) ==
                      ANEURALNETWORKS_RESIZE_BILINEAR,
              "OperationType::RESIZE_BILINEAR != ANEURALNETWORKS_RESIZE_BILINEAR");
static_assert(static_cast<int32_t>(OperationType::RNN) == ANEURALNETWORKS_RNN,
              "OperationType::RNN != ANEURALNETWORKS_RNN");
static_assert(static_cast<int32_t>(OperationType::SOFTMAX) == ANEURALNETWORKS_SOFTMAX,
              "OperationType::SOFTMAX != ANEURALNETWORKS_SOFTMAX");
static_assert(static_cast<int32_t>(OperationType::SPACE_TO_DEPTH) == ANEURALNETWORKS_SPACE_TO_DEPTH,
              "OperationType::SPACE_TO_DEPTH != ANEURALNETWORKS_SPACE_TO_DEPTH");
static_assert(static_cast<int32_t>(OperationType::SVDF) == ANEURALNETWORKS_SVDF,
              "OperationType::SVDF != ANEURALNETWORKS_SVDF");
static_assert(static_cast<int32_t>(OperationType::TANH) == ANEURALNETWORKS_TANH,
              "OperationType::TANH != ANEURALNETWORKS_TANH");

static_assert(static_cast<int32_t>(FusedActivationFunc::NONE) == ANEURALNETWORKS_FUSED_NONE,
              "FusedActivationFunc::NONE != ANEURALNETWORKS_FUSED_NONE");
static_assert(static_cast<int32_t>(FusedActivationFunc::RELU) == ANEURALNETWORKS_FUSED_RELU,
              "FusedActivationFunc::RELU != ANEURALNETWORKS_FUSED_RELU");
static_assert(static_cast<int32_t>(FusedActivationFunc::RELU1) == ANEURALNETWORKS_FUSED_RELU1,
              "FusedActivationFunc::RELU1 != ANEURALNETWORKS_FUSED_RELU1");
static_assert(static_cast<int32_t>(FusedActivationFunc::RELU6) == ANEURALNETWORKS_FUSED_RELU6,
              "FusedActivationFunc::RELU6 != ANEURALNETWORKS_FUSED_RELU6");

// Make sure that the constants are compatible with the values defined in
// hardware/interfaces/neuralnetworks/1.1/types.hal.
static_assert(static_cast<int32_t>(OperationType::BATCH_TO_SPACE_ND) ==
                      ANEURALNETWORKS_BATCH_TO_SPACE_ND,
              "OperationType::BATCH_TO_SPACE_ND != ANEURALNETWORKS_BATCH_TO_SPACE_ND");
static_assert(static_cast<int32_t>(OperationType::DIV) == ANEURALNETWORKS_DIV,
              "OperationType::DIV != ANEURALNETWORKS_DIV");
static_assert(static_cast<int32_t>(OperationType::MEAN) == ANEURALNETWORKS_MEAN,
              "OperationType::MEAN != ANEURALNETWORKS_MEAN");
static_assert(static_cast<int32_t>(OperationType::PAD) == ANEURALNETWORKS_PAD,
              "OperationType::PAD != ANEURALNETWORKS_PAD");
static_assert(static_cast<int32_t>(OperationType::SPACE_TO_BATCH_ND) ==
                      ANEURALNETWORKS_SPACE_TO_BATCH_ND,
              "OperationType::SPACE_TO_BATCH_ND != ANEURALNETWORKS_SPACE_TO_BATCH_ND");
static_assert(static_cast<int32_t>(OperationType::SQUEEZE) == ANEURALNETWORKS_SQUEEZE,
              "OperationType::SQUEEZE != ANEURALNETWORKS_SQUEEZE");
static_assert(static_cast<int32_t>(OperationType::STRIDED_SLICE) == ANEURALNETWORKS_STRIDED_SLICE,
              "OperationType::STRIDED_SLICE != ANEURALNETWORKS_STRIDED_SLICE");
static_assert(static_cast<int32_t>(OperationType::SUB) == ANEURALNETWORKS_SUB,
              "OperationType::SUB != ANEURALNETWORKS_SUB");
static_assert(static_cast<int32_t>(OperationType::TRANSPOSE) == ANEURALNETWORKS_TRANSPOSE,
              "OperationType::TRANSPOSE != ANEURALNETWORKS_TRANSPOSE");

// Make sure that the constants are compatible with the values defined in
// hardware/interfaces/neuralnetworks/1.2/types.hal.
static_assert(static_cast<int32_t>(OperandType::BOOL) == ANEURALNETWORKS_BOOL,
              "BOOL != ANEURALNETWORKS_BOOL");
static_assert(static_cast<int32_t>(OperandType::TENSOR_QUANT16_SYMM) ==
                      ANEURALNETWORKS_TENSOR_QUANT16_SYMM,
              "TENSOR_QUANT16_SYMM != ANEURALNETWORKS_TENSOR_QUANT16_SYMM");
static_assert(static_cast<int32_t>(OperandType::TENSOR_FLOAT16) == ANEURALNETWORKS_TENSOR_FLOAT16,
              "TENSOR_FLOAT16 != ANEURALNETWORKS_TENSOR_FLOAT16");
static_assert(static_cast<int32_t>(OperandType::TENSOR_BOOL8) == ANEURALNETWORKS_TENSOR_BOOL8,
              "TENSOR_BOOL8 != ANEURALNETWORKS_TENSOR_BOOL8");
static_assert(static_cast<int32_t>(OperandType::FLOAT16) == ANEURALNETWORKS_FLOAT16,
              "FLOAT16 != ANEURALNETWORKS_FLOAT16");
static_assert(static_cast<int32_t>(OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) ==
                      ANEURALNETWORKS_TENSOR_QUANT8_SYMM_PER_CHANNEL,
              "TENSOR_QUANT8_SYMM_PER_CHANNEL != ANEURALNETWORKS_TENSOR_QUANT8_SYMM_PER_CHANNEL");
static_assert(static_cast<int32_t>(OperandType::TENSOR_QUANT16_ASYMM) ==
                      ANEURALNETWORKS_TENSOR_QUANT16_ASYMM,
              "TENSOR_QUANT16_ASYMM != ANEURALNETWORKS_TENSOR_QUANT16_ASYMM");
static_assert(static_cast<int32_t>(OperandType::TENSOR_QUANT8_SYMM) ==
                      ANEURALNETWORKS_TENSOR_QUANT8_SYMM,
              "TENSOR_QUANT8_SYMM != ANEURALNETWORKS_TENSOR_QUANT8_SYMM");

static_assert(static_cast<int32_t>(OperationType::ABS) == ANEURALNETWORKS_ABS,
              "OperationType::ABS != ANEURALNETWORKS_ABS");
static_assert(static_cast<int32_t>(OperationType::ARGMAX) == ANEURALNETWORKS_ARGMAX,
              "OperationType::ARGMAX != ANEURALNETWORKS_ARGMAX");
static_assert(static_cast<int32_t>(OperationType::ARGMIN) == ANEURALNETWORKS_ARGMIN,
              "OperationType::ARGMIN != ANEURALNETWORKS_ARGMIN");
static_assert(static_cast<int32_t>(OperationType::AXIS_ALIGNED_BBOX_TRANSFORM) ==
                      ANEURALNETWORKS_AXIS_ALIGNED_BBOX_TRANSFORM,
              "OperationType::AXIS_ALIGNED_BBOX_TRANSFORM != "
              "ANEURALNETWORKS_AXIS_ALIGNED_BBOX_TRANSFORM");
static_assert(static_cast<int32_t>(OperationType::BIDIRECTIONAL_SEQUENCE_LSTM) ==
                      ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_LSTM,
              "OperationType::BIDIRECTIONAL_SEQUENCE_LSTM != "
              "ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_LSTM");
static_assert(
        static_cast<int32_t>(OperationType::BIDIRECTIONAL_SEQUENCE_RNN) ==
                ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_RNN,
        "OperationType::BIDIRECTIONAL_SEQUENCE_RNN != ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_RNN");
static_assert(static_cast<int32_t>(OperationType::BOX_WITH_NMS_LIMIT) ==
                      ANEURALNETWORKS_BOX_WITH_NMS_LIMIT,
              "OperationType::BOX_WITH_NMS_LIMIT != ANEURALNETWORKS_BOX_WITH_NMS_LIMIT");
static_assert(static_cast<int32_t>(OperationType::CAST) == ANEURALNETWORKS_CAST,
              "OperationType::CAST != ANEURALNETWORKS_CAST");
static_assert(static_cast<int32_t>(OperationType::CHANNEL_SHUFFLE) ==
                      ANEURALNETWORKS_CHANNEL_SHUFFLE,
              "OperationType::CHANNEL_SHUFFLE != ANEURALNETWORKS_CHANNEL_SHUFFLE");
static_assert(
        static_cast<int32_t>(OperationType::DETECTION_POSTPROCESSING) ==
                ANEURALNETWORKS_DETECTION_POSTPROCESSING,
        "OperationType::DETECTION_POSTPROCESSING != ANEURALNETWORKS_DETECTION_POSTPROCESSING");
static_assert(static_cast<int32_t>(OperationType::EQUAL) == ANEURALNETWORKS_EQUAL,
              "OperationType::EQUAL != ANEURALNETWORKS_EQUAL");
static_assert(static_cast<int32_t>(OperationType::EXP) == ANEURALNETWORKS_EXP,
              "OperationType::EXP != ANEURALNETWORKS_EXP");
static_assert(static_cast<int32_t>(OperationType::EXPAND_DIMS) == ANEURALNETWORKS_EXPAND_DIMS,
              "OperationType::EXPAND_DIMS != ANEURALNETWORKS_EXPAND_DIMS");
static_assert(static_cast<int32_t>(OperationType::GATHER) == ANEURALNETWORKS_GATHER,
              "OperationType::GATHER != ANEURALNETWORKS_GATHER");
static_assert(static_cast<int32_t>(OperationType::GENERATE_PROPOSALS) ==
                      ANEURALNETWORKS_GENERATE_PROPOSALS,
              "OperationType::GENERATE_PROPOSALS != ANEURALNETWORKS_GENERATE_PROPOSALS");
static_assert(static_cast<int32_t>(OperationType::GREATER) == ANEURALNETWORKS_GREATER,
              "OperationType::GREATER != ANEURALNETWORKS_GREATER");
static_assert(static_cast<int32_t>(OperationType::GREATER_EQUAL) == ANEURALNETWORKS_GREATER_EQUAL,
              "OperationType::GREATER_EQUAL != ANEURALNETWORKS_GREATER_EQUAL");
static_assert(static_cast<int32_t>(OperationType::GROUPED_CONV_2D) ==
                      ANEURALNETWORKS_GROUPED_CONV_2D,
              "OperationType::GROUPED_CONV_2D != ANEURALNETWORKS_GROUPED_CONV_2D");
static_assert(static_cast<int32_t>(OperationType::HEATMAP_MAX_KEYPOINT) ==
                      ANEURALNETWORKS_HEATMAP_MAX_KEYPOINT,
              "OperationType::HEATMAP_MAX_KEYPOINT != ANEURALNETWORKS_HEATMAP_MAX_KEYPOINT");
static_assert(static_cast<int32_t>(OperationType::INSTANCE_NORMALIZATION) ==
                      ANEURALNETWORKS_INSTANCE_NORMALIZATION,
              "OperationType::INSTANCE_NORMALIZATION != ANEURALNETWORKS_INSTANCE_NORMALIZATION");
static_assert(static_cast<int32_t>(OperationType::LESS) == ANEURALNETWORKS_LESS,
              "OperationType::LESS != ANEURALNETWORKS_LESS");
static_assert(static_cast<int32_t>(OperationType::LESS_EQUAL) == ANEURALNETWORKS_LESS_EQUAL,
              "OperationType::LESS_EQUAL != ANEURALNETWORKS_LESS_EQUAL");
static_assert(static_cast<int32_t>(OperationType::LOG) == ANEURALNETWORKS_LOG,
              "OperationType::LOG != ANEURALNETWORKS_LOG");
static_assert(static_cast<int32_t>(OperationType::LOGICAL_AND) == ANEURALNETWORKS_LOGICAL_AND,
              "OperationType::LOGICAL_AND != ANEURALNETWORKS_LOGICAL_AND");
static_assert(static_cast<int32_t>(OperationType::LOGICAL_NOT) == ANEURALNETWORKS_LOGICAL_NOT,
              "OperationType::LOGICAL_NOT != ANEURALNETWORKS_LOGICAL_NOT");
static_assert(static_cast<int32_t>(OperationType::LOGICAL_OR) == ANEURALNETWORKS_LOGICAL_OR,
              "OperationType::LOGICAL_OR != ANEURALNETWORKS_LOGICAL_OR");
static_assert(static_cast<int32_t>(OperationType::LOG_SOFTMAX) == ANEURALNETWORKS_LOG_SOFTMAX,
              "OperationType::LOG_SOFTMAX != ANEURALNETWORKS_LOG_SOFTMAX");
static_assert(static_cast<int32_t>(OperationType::MAXIMUM) == ANEURALNETWORKS_MAXIMUM,
              "OperationType::MAXIMUM != ANEURALNETWORKS_MAXIMUM");
static_assert(static_cast<int32_t>(OperationType::MINIMUM) == ANEURALNETWORKS_MINIMUM,
              "OperationType::MINIMUM != ANEURALNETWORKS_MINIMUM");
static_assert(static_cast<int32_t>(OperationType::NEG) == ANEURALNETWORKS_NEG,
              "OperationType::NEG != ANEURALNETWORKS_NEG");
static_assert(static_cast<int32_t>(OperationType::NOT_EQUAL) == ANEURALNETWORKS_NOT_EQUAL,
              "OperationType::NOT_EQUAL != ANEURALNETWORKS_NOT_EQUAL");
static_assert(static_cast<int32_t>(OperationType::PAD_V2) == ANEURALNETWORKS_PAD_V2,
              "OperationType::PAD_V2 != ANEURALNETWORKS_PAD_V2");
static_assert(static_cast<int32_t>(OperationType::POW) == ANEURALNETWORKS_POW,
              "OperationType::POW != ANEURALNETWORKS_POW");
static_assert(static_cast<int32_t>(OperationType::PRELU) == ANEURALNETWORKS_PRELU,
              "OperationType::PRELU != ANEURALNETWORKS_PRELU");
static_assert(static_cast<int32_t>(OperationType::QUANTIZE) == ANEURALNETWORKS_QUANTIZE,
              "OperationType::QUANTIZE != ANEURALNETWORKS_QUANTIZE");
static_assert(static_cast<int32_t>(OperationType::QUANTIZED_16BIT_LSTM) ==
                      ANEURALNETWORKS_QUANTIZED_16BIT_LSTM,
              "OperationType::QUANTIZED_16BIT_LSTM != ANEURALNETWORKS_QUANTIZED_16BIT_LSTM");
static_assert(static_cast<int32_t>(OperationType::RANDOM_MULTINOMIAL) ==
                      ANEURALNETWORKS_RANDOM_MULTINOMIAL,
              "OperationType::RANDOM_MULTINOMIAL != ANEURALNETWORKS_RANDOM_MULTINOMIAL");
static_assert(static_cast<int32_t>(OperationType::REDUCE_ALL) == ANEURALNETWORKS_REDUCE_ALL,
              "OperationType::REDUCE_ALL != ANEURALNETWORKS_REDUCE_ALL");
static_assert(static_cast<int32_t>(OperationType::REDUCE_ANY) == ANEURALNETWORKS_REDUCE_ANY,
              "OperationType::REDUCE_ANY != ANEURALNETWORKS_REDUCE_ANY");
static_assert(static_cast<int32_t>(OperationType::REDUCE_MAX) == ANEURALNETWORKS_REDUCE_MAX,
              "OperationType::REDUCE_MAX != ANEURALNETWORKS_REDUCE_MAX");
static_assert(static_cast<int32_t>(OperationType::REDUCE_MIN) == ANEURALNETWORKS_REDUCE_MIN,
              "OperationType::REDUCE_MIN != ANEURALNETWORKS_REDUCE_MIN");
static_assert(static_cast<int32_t>(OperationType::REDUCE_PROD) == ANEURALNETWORKS_REDUCE_PROD,
              "OperationType::REDUCE_PROD != ANEURALNETWORKS_REDUCE_PROD");
static_assert(static_cast<int32_t>(OperationType::REDUCE_SUM) == ANEURALNETWORKS_REDUCE_SUM,
              "OperationType::REDUCE_SUM != ANEURALNETWORKS_REDUCE_SUM");
static_assert(static_cast<int32_t>(OperationType::ROI_ALIGN) == ANEURALNETWORKS_ROI_ALIGN,
              "OperationType::ROI_ALIGN != ANEURALNETWORKS_ROI_ALIGN");
static_assert(static_cast<int32_t>(OperationType::ROI_POOLING) == ANEURALNETWORKS_ROI_POOLING,
              "OperationType::ROI_POOLING != ANEURALNETWORKS_ROI_POOLING");
static_assert(static_cast<int32_t>(OperationType::RSQRT) == ANEURALNETWORKS_RSQRT,
              "OperationType::RSQRT != ANEURALNETWORKS_RSQRT");
static_assert(static_cast<int32_t>(OperationType::SELECT) == ANEURALNETWORKS_SELECT,
              "OperationType::SELECT != ANEURALNETWORKS_SELECT");
static_assert(static_cast<int32_t>(OperationType::SIN) == ANEURALNETWORKS_SIN,
              "OperationType::SIN != ANEURALNETWORKS_SIN");
static_assert(static_cast<int32_t>(OperationType::SLICE) == ANEURALNETWORKS_SLICE,
              "OperationType::SLICE != ANEURALNETWORKS_SLICE");
static_assert(static_cast<int32_t>(OperationType::SPLIT) == ANEURALNETWORKS_SPLIT,
              "OperationType::SPLIT != ANEURALNETWORKS_SPLIT");
static_assert(static_cast<int32_t>(OperationType::SQRT) == ANEURALNETWORKS_SQRT,
              "OperationType::SQRT != ANEURALNETWORKS_SQRT");
static_assert(static_cast<int32_t>(OperationType::TILE) == ANEURALNETWORKS_TILE,
              "OperationType::TILE != ANEURALNETWORKS_TILE");
static_assert(static_cast<int32_t>(OperationType::TOPK_V2) == ANEURALNETWORKS_TOPK_V2,
              "OperationType::TOPK_V2 != ANEURALNETWORKS_TOPK_V2");
static_assert(static_cast<int32_t>(OperationType::TRANSPOSE_CONV_2D) ==
                      ANEURALNETWORKS_TRANSPOSE_CONV_2D,
              "OperationType::TRANSPOSE_CONV_2D != ANEURALNETWORKS_TRANSPOSE_CONV_2D");
static_assert(static_cast<int32_t>(OperationType::UNIDIRECTIONAL_SEQUENCE_LSTM) ==
                      ANEURALNETWORKS_UNIDIRECTIONAL_SEQUENCE_LSTM,
              "OperationType::UNIDIRECTIONAL_SEQUENCE_LSTM != "
              "ANEURALNETWORKS_UNIDIRECTIONAL_SEQUENCE_LSTM");
static_assert(static_cast<int32_t>(OperationType::UNIDIRECTIONAL_SEQUENCE_RNN) ==
                      ANEURALNETWORKS_UNIDIRECTIONAL_SEQUENCE_RNN,
              "OperationType::UNIDIRECTIONAL_SEQUENCE_RNN != "
              "ANEURALNETWORKS_UNIDIRECTIONAL_SEQUENCE_RNN");
static_assert(static_cast<int32_t>(OperationType::RESIZE_NEAREST_NEIGHBOR) ==
                      ANEURALNETWORKS_RESIZE_NEAREST_NEIGHBOR,
              "OperationType::RESIZE_NEAREST_NEIGHBOR != ANEURALNETWORKS_RESIZE_NEAREST_NEIGHBOR");

static_assert(static_cast<int32_t>(DeviceType::OTHER) == ANEURALNETWORKS_DEVICE_OTHER,
              "DeviceType::OTHER != ANEURALNETWORKS_DEVICE_OTHER");
static_assert(static_cast<int32_t>(DeviceType::CPU) == ANEURALNETWORKS_DEVICE_CPU,
              "DeviceType::CPU != ANEURALNETWORKS_DEVICE_CPU");
static_assert(static_cast<int32_t>(DeviceType::GPU) == ANEURALNETWORKS_DEVICE_GPU,
              "DeviceType::GPU != ANEURALNETWORKS_DEVICE_GPU");
static_assert(static_cast<int32_t>(DeviceType::ACCELERATOR) == ANEURALNETWORKS_DEVICE_ACCELERATOR,
              "DeviceType::ACCELERATOR != ANEURALNETWORKS_DEVICE_ACCELERATOR");

// Asserts for ANeuralNetworksOperandType memory layout
static_assert(offsetof(ANeuralNetworksOperandType, type) == 0,
              "ANeuralNetworksOperandType.type offset != 0");
static_assert(offsetof(ANeuralNetworksOperandType, dimensionCount) == 4,
              "ANeuralNetworksOperandType.dimensionCount offset != 4");
static_assert(offsetof(ANeuralNetworksOperandType, dimensions) == 8,
              "ANeuralNetworksOperandType.dimensions offset != 8");
static_assert(offsetof(ANeuralNetworksOperandType, scale) == 8 + sizeof(void*),
              "ANeuralNetworksOperandType.scale offset != 8 + sizeof(void*)");
static_assert(offsetof(ANeuralNetworksOperandType, zeroPoint) == 12 + sizeof(void*),
              "ANeuralNetworksOperandType.zeroPoint offset != 12 + sizeof(void*)");
static_assert(sizeof(ANeuralNetworksOperandType) == 16 + sizeof(void*),
              "ANeuralNetworksOperandType size changed");
static_assert(alignof(ANeuralNetworksOperandType) == alignof(void*),
              "ANeuralNetworksOperandType alignment changed");

// Asserts for ANeuralNetworksSymmPerChannelQuantParams memory layout
static_assert(offsetof(ANeuralNetworksSymmPerChannelQuantParams, channelDim) == 0,
              "ANeuralNetworksSymmPerChannelQuantParams.channelDim offset != 4 + sizeof(void*)");
static_assert(offsetof(ANeuralNetworksSymmPerChannelQuantParams, scaleCount) == 4,
              "ANeuralNetworksSymmPerChannelQuantParams.scaleCount offset != 0");
static_assert(offsetof(ANeuralNetworksSymmPerChannelQuantParams, scales) == 8,
              "ANeuralNetworksSymmPerChannelQuantParams.scales offset != 4");
static_assert(sizeof(ANeuralNetworksSymmPerChannelQuantParams) == 8 + sizeof(void*),
              "ANeuralNetworksSymmPerChannelQuantParams size != 8 + sizeof(void*)");
static_assert(alignof(ANeuralNetworksSymmPerChannelQuantParams) == alignof(void*),
              "ANeuralNetworksOperandType alignment changed");

// Asserts for compilation caching
static_assert(ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN == 32,
              "ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN has changed");
static_assert(static_cast<uint32_t>(Constant::BYTE_SIZE_OF_CACHE_TOKEN) ==
                      ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN,
              "Constant::BYTE_SIZE_OF_CACHE_TOKEN != ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN");

using android::sp;
using namespace android::nn;

int ANeuralNetworks_getDeviceCount(uint32_t* numDevices) {
    if (numDevices == nullptr) {
        LOG(ERROR) << "ANeuralNetworks_getDeviceCount passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    *numDevices = DeviceManager::get()->getDrivers().size();
    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuralNetworks_getDevice(uint32_t devIndex, ANeuralNetworksDevice** device) {
    if (device == nullptr) {
        LOG(ERROR) << "ANeuralNetworks_getDevice passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    const std::vector<std::shared_ptr<Device>>& devices = DeviceManager::get()->getDrivers();
    if (devIndex >= devices.size()) {
        LOG(ERROR) << "ANeuralNetworks_getDevice passed an invalid device index";
        return ANEURALNETWORKS_BAD_DATA;
    }
    *device = reinterpret_cast<ANeuralNetworksDevice*>(devices.at(devIndex).get());
    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuralNetworksDevice_getName(const ANeuralNetworksDevice* device, const char** name) {
    if (device == nullptr || name == nullptr) {
        LOG(ERROR) << "ANeuralNetworksDevice_getName passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    const Device* d = reinterpret_cast<const Device*>(device);
    *name = d->getName();
    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuralNetworksDevice_getVersion(const ANeuralNetworksDevice* device, const char** version) {
    if (device == nullptr || version == nullptr) {
        LOG(ERROR) << "ANeuralNetworksDevice_getVersion passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    const Device* d = reinterpret_cast<const Device*>(device);
    *version = d->getVersionString();
    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuralNetworksDevice_getType(const ANeuralNetworksDevice* device, int32_t* type) {
    if (device == nullptr || type == nullptr) {
        LOG(ERROR) << "ANeuralNetworksDevice_getType passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    const Device* d = reinterpret_cast<const Device*>(device);
    int32_t dType = d->getType();
    if (dType < 0) {
        return ANEURALNETWORKS_OP_FAILED;
    }
    *type = d->getType();
    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuralNetworksDevice_getFeatureLevel(const ANeuralNetworksDevice* device,
                                          int64_t* featureLevel) {
    if (device == nullptr || featureLevel == nullptr) {
        LOG(ERROR) << "ANeuralNetworksDevice_getFeatureLevel passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    Device* d = reinterpret_cast<Device*>(const_cast<ANeuralNetworksDevice*>(device));
    int64_t dFeatureLevel = d->getFeatureLevel();
    if (dFeatureLevel < 0) {
        return ANEURALNETWORKS_BAD_STATE;
    }
    *featureLevel = dFeatureLevel;
    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuralNetworksModel_getSupportedOperationsForDevices(
        const ANeuralNetworksModel* model, const ANeuralNetworksDevice* const* devices,
        uint32_t numDevices, bool* supportedOps) {
    NNTRACE_RT(NNTRACE_PHASE_COMPILATION, "ANeuralNetworksModel_getSupportedOperationsForDevices");
    if (model == nullptr || devices == nullptr || supportedOps == nullptr) {
        LOG(ERROR) << "ANeuralNetworksModel_getSupportedOperationsForDevices passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    if (numDevices == 0) {
        LOG(ERROR) << "ANeuralNetworksModel_getSupportedOperationsForDevices passed an empty "
                      "device list";
        return ANEURALNETWORKS_BAD_DATA;
    }
    const ModelBuilder* m = reinterpret_cast<const ModelBuilder*>(model);
    if (!m->isFinished() || !m->isValid()) {
        LOG(ERROR) << "ANeuralNetworksModel_getSupportedOperationsForDevices passed an unfinished "
                      "or invalid Model";
        return ANEURALNETWORKS_BAD_STATE;
    }

    Model hidlModel;
    m->setHidlModel(&hidlModel);
    const std::vector<uint32_t>& opMap = m->getSortedOperationMapping();
    // init the output array to false for all the operations.
    std::fill(supportedOps, supportedOps + opMap.size(), false);
    for (uint32_t i = 0; i < numDevices; i++) {
        if (devices[i] == nullptr) {
            LOG(ERROR) << "ANeuralNetworksModel_getSupportedOperationsForDevices passed a nullptr "
                          "as a device";
            return ANEURALNETWORKS_UNEXPECTED_NULL;
        }
        for (uint32_t j = i + 1; j < numDevices; j++) {
            if (devices[i] == devices[j]) {
                LOG(ERROR) << "ANeuralNetworksModel_getSupportedOperationsForDevices passed "
                              "duplicate devices";
                return ANEURALNETWORKS_BAD_DATA;
            }
        }

        Device* d = reinterpret_cast<Device*>(const_cast<ANeuralNetworksDevice*>(devices[i]));
        hidl_vec<bool> supportsByDevice;
        d->getSupportedOperations(hidlModel, &supportsByDevice);
        for (uint32_t j = 0; j < supportsByDevice.size(); j++) {
            uint32_t originalIdx = opMap[j];
            supportedOps[originalIdx] |= supportsByDevice[j];
        }
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuralNetworksCompilation_createForDevices(ANeuralNetworksModel* model,
                                                const ANeuralNetworksDevice* const* devices,
                                                uint32_t numDevices,
                                                ANeuralNetworksCompilation** compilation) {
    NNTRACE_RT(NNTRACE_PHASE_COMPILATION, "ANeuralNetworksCompilation_createForDevices");
    if (model == nullptr || devices == nullptr || compilation == nullptr) {
        LOG(ERROR) << "ANeuralNetworksCompilation_createForDevices passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    if (numDevices == 0) {
        LOG(ERROR) << "ANeuralNetworksCompilation_createForDevices passed an empty device list";
        return ANEURALNETWORKS_BAD_DATA;
    }

    std::vector<std::shared_ptr<Device>> selectedDevices;
    for (uint32_t i = 0; i < numDevices; i++) {
        if (devices[i] == nullptr) {
            LOG(ERROR)
                    << "ANeuralNetworksCompilation_createForDevices passed a nullptr as a device";
            return ANEURALNETWORKS_UNEXPECTED_NULL;
        }
        for (uint32_t j = i + 1; j < numDevices; j++) {
            if (devices[i] == devices[j]) {
                LOG(ERROR)
                        << "ANeuralNetworksCompilation_createForDevices passed duplicate devices";
                return ANEURALNETWORKS_BAD_DATA;
            }
        }
        for (auto& device : DeviceManager::get()->getDrivers()) {
            if (device.get() == reinterpret_cast<const Device*>(devices[i])) {
                // Find a match
                selectedDevices.push_back(device);
                break;
            }
        }
    }

    if (selectedDevices.size() != numDevices) {
        LOG(ERROR) << "ANeuralNetworksCompilation_createForDevices passed an invalid device set";
        return ANEURALNETWORKS_BAD_DATA;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    CompilationBuilder* c = nullptr;
    // No CPU fallback when user specifies the list of devices manually.
    int result = m->createCompilation(&c, selectedDevices, /* explicitDeviceList */ true);
    *compilation = reinterpret_cast<ANeuralNetworksCompilation*>(c);
    return result;
}

int ANeuralNetworksExecution_compute(ANeuralNetworksExecution* execution) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksExecution_compute");
    if (!execution) {
        LOG(ERROR) << "ANeuralNetworksExecution_compute passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    // TODO validate the rest

    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);
    return r->computeSynchronously();
}

int ANeuralNetworksExecution_setMeasureTiming(ANeuralNetworksExecution* execution, bool measure) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksExecution_setMeasureTiming");
    if (!execution) {
        LOG(ERROR) << "ANeuralNetworksExecution_setMeasureTiming passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);
    return r->setMeasureTiming(measure);
}

int ANeuralNetworksExecution_getDuration(const ANeuralNetworksExecution* execution,
                                         int32_t durationCode, uint64_t* duration) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksExecution_getDuration");
    if (!execution || !duration) {
        LOG(ERROR) << "ANeuralNetworksExecution_getDuration passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    switch (durationCode) {
        case ANEURALNETWORKS_DURATION_ON_HARDWARE:
        case ANEURALNETWORKS_DURATION_IN_DRIVER:
            break;
        default:
            LOG(ERROR) << "ANeuralNetworksExecution_getDuration passed a bad durationCode "
                       << durationCode;
            return ANEURALNETWORKS_BAD_DATA;
    }
    const ExecutionBuilder* r = reinterpret_cast<const ExecutionBuilder*>(execution);
    return r->getDuration(durationCode, duration);
}

int ANeuralNetworksBurst_create(ANeuralNetworksCompilation* compilation,
                                ANeuralNetworksBurst** burst) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksBurst_create");
    if (!compilation || !burst) {
        LOG(ERROR) << "ANeuralNetworksBurst_create passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    CompilationBuilder* c = reinterpret_cast<CompilationBuilder*>(compilation);
    BurstBuilder* b = nullptr;
    int result = c->createBurst(&b);
    *burst = reinterpret_cast<ANeuralNetworksBurst*>(b);
    return result;
}

void ANeuralNetworksBurst_free(ANeuralNetworksBurst* burst) {
    NNTRACE_RT(NNTRACE_PHASE_TERMINATION, "ANeuralNetworksBurst_free");
    // No validation.  Free of nullptr is valid.
    BurstBuilder* b = reinterpret_cast<BurstBuilder*>(burst);
    delete b;
}

int ANeuralNetworksExecution_burstCompute(ANeuralNetworksExecution* execution,
                                          ANeuralNetworksBurst* burst) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksExecution_burstCompute");
    if (!execution || !burst) {
        LOG(ERROR) << "ANeuralNetworksExecution_burstCompute passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);
    BurstBuilder* b = reinterpret_cast<BurstBuilder*>(burst);

    if (r->getCompilation() != b->getCompilation()) {
        LOG(ERROR) << "ANeuralNetworksBurst and ANeuralNetworksExecution "
                      "used in ANeuralNetworksExecution_burstCompute must "
                      "originate from the same ANeuralNetworksCompilation";
        return ANEURALNETWORKS_BAD_DATA;
    }

    const bool locked = b->tryLock();
    if (!locked) {
        LOG(ERROR) << "ANeuralNetworksBurst is already being used in another "
                      "call to ANeuralNetworksExecution_burstCompute";
        return ANEURALNETWORKS_BAD_STATE;
    }

    const int n = r->burstCompute(b);
    b->unlock();

    return n;
}

int ANeuralNetworksMemory_createFromFd(size_t size, int prot, int fd, size_t offset,
                                       ANeuralNetworksMemory** memory) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksMemory_createFromFd");
    *memory = nullptr;
    std::unique_ptr<MemoryFd> m = std::make_unique<MemoryFd>();
    if (m == nullptr) {
        return ANEURALNETWORKS_OUT_OF_MEMORY;
    }
    int n = m->set(size, prot, fd, offset);
    if (n != ANEURALNETWORKS_NO_ERROR) {
        return n;
    }
    *memory = reinterpret_cast<ANeuralNetworksMemory*>(m.release());
    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuralNetworksMemory_createFromAHardwareBuffer(const AHardwareBuffer* ahwb,
                                                    ANeuralNetworksMemory** memory) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksMemory_createFromAHardwareBuffer");
    *memory = nullptr;
    std::unique_ptr<MemoryAHWB> m = std::make_unique<MemoryAHWB>();
    if (m == nullptr) {
        return ANEURALNETWORKS_OUT_OF_MEMORY;
    }
    int n = m->set(ahwb);
    if (n != ANEURALNETWORKS_NO_ERROR) {
        return n;
    }
    *memory = reinterpret_cast<ANeuralNetworksMemory*>(m.release());
    return ANEURALNETWORKS_NO_ERROR;
}

void ANeuralNetworksMemory_free(ANeuralNetworksMemory* memory) {
    NNTRACE_RT(NNTRACE_PHASE_TERMINATION, "ANeuralNetworksMemory_free");
    // No validation.  Free of nullptr is valid.
    Memory* m = reinterpret_cast<Memory*>(memory);
    delete m;
}

int ANeuralNetworksModel_create(ANeuralNetworksModel** model) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_create");
    initVLogMask();
    if (!model) {
        LOG(ERROR) << "ANeuralNetworksModel_create passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = new (std::nothrow) ModelBuilder();
    if (m == nullptr) {
        *model = nullptr;
        return ANEURALNETWORKS_OUT_OF_MEMORY;
    }
    *model = reinterpret_cast<ANeuralNetworksModel*>(m);
    return ANEURALNETWORKS_NO_ERROR;
}

void ANeuralNetworksModel_free(ANeuralNetworksModel* model) {
    NNTRACE_RT(NNTRACE_PHASE_TERMINATION, "ANeuralNetworksModel_free");
    // No validation.  Free of nullptr is valid.
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    delete m;
}

int ANeuralNetworksModel_finish(ANeuralNetworksModel* model) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_finish");
    if (!model) {
        LOG(ERROR) << "ANeuralNetworksModel_finish passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->finish();
}

int ANeuralNetworksModel_addOperand(ANeuralNetworksModel* model,
                                    const ANeuralNetworksOperandType* type) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_addOperand");
    if (!model || !type) {
        LOG(ERROR) << "ANeuralNetworksModel_addOperand passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->addOperand(*type);
}

int ANeuralNetworksModel_setOperandValue(ANeuralNetworksModel* model, int32_t index,
                                         const void* buffer, size_t length) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_setOperandValue");
    if (!model || (!buffer && length != 0)) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandValue passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->setOperandValue(index, buffer, length);
}

int ANeuralNetworksModel_setOperandValueFromMemory(ANeuralNetworksModel* model, int32_t index,
                                                   const ANeuralNetworksMemory* memory,
                                                   size_t offset, size_t length) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_setOperandValueFromMemory");
    if (!model || !memory) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandValue passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    const Memory* mem = reinterpret_cast<const Memory*>(memory);
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->setOperandValueFromMemory(index, mem, offset, length);
}

int ANeuralNetworksModel_addOperation(ANeuralNetworksModel* model,
                                      ANeuralNetworksOperationType type, uint32_t inputCount,
                                      const uint32_t* inputs, uint32_t outputCount,
                                      const uint32_t* outputs) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_addOperation");
    if (!model || !inputs || !outputs) {
        LOG(ERROR) << "ANeuralNetworksModel_addOperation passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->addOperation(type, inputCount, inputs, outputCount, outputs);
}

int ANeuralNetworksModel_setOperandSymmPerChannelQuantParams(
        ANeuralNetworksModel* model, int32_t index,
        const ANeuralNetworksSymmPerChannelQuantParams* channelQuant) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION,
               "ANeuralNetworksModel_setOperandSymmPerChannelQuantParams");
    if (!model || !channelQuant) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandSymmPerChannelQuantParams passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->setOperandSymmPerChannelQuantParams(index, *channelQuant);
}

int ANeuralNetworksModel_identifyInputsAndOutputs(ANeuralNetworksModel* model, uint32_t inputCount,
                                                  const uint32_t* inputs, uint32_t outputCount,
                                                  const uint32_t* outputs) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_identifyInputsAndOutputs");
    if (!model || !inputs || !outputs) {
        LOG(ERROR) << ("ANeuralNetworksModel_identifyInputsAndOutputs passed a nullptr");
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->identifyInputsAndOutputs(inputCount, inputs, outputCount, outputs);
}

int ANeuralNetworksModel_relaxComputationFloat32toFloat16(ANeuralNetworksModel* model, bool allow) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_relaxComputationFloat32toFloat16");
    if (!model) {
        LOG(ERROR) << ("ANeuralNetworksModel_relaxComputationFloat32toFloat16 passed a nullptr");
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->relaxComputationFloat32toFloat16(allow);
}

int ANeuralNetworksCompilation_create(ANeuralNetworksModel* model,
                                      ANeuralNetworksCompilation** compilation) {
    NNTRACE_RT(NNTRACE_PHASE_COMPILATION, "ANeuralNetworksCompilation_create");
    if (!model || !compilation) {
        LOG(ERROR) << "ANeuralNetworksCompilation_create passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    CompilationBuilder* c = nullptr;
    int result = m->createCompilation(&c, DeviceManager::get()->getDrivers());
    *compilation = reinterpret_cast<ANeuralNetworksCompilation*>(c);
    return result;
}

void ANeuralNetworksCompilation_free(ANeuralNetworksCompilation* compilation) {
    NNTRACE_RT(NNTRACE_PHASE_TERMINATION, "ANeuralNetworksCompilation_free");
    // No validation.  Free of nullptr is valid.
    // TODO specification says that a compilation-in-flight can be deleted
    CompilationBuilder* c = reinterpret_cast<CompilationBuilder*>(compilation);
    delete c;
}

int ANeuralNetworksCompilation_setPreference(ANeuralNetworksCompilation* compilation,
                                             int32_t preference) {
    NNTRACE_RT(NNTRACE_PHASE_COMPILATION, "ANeuralNetworksCompilation_setPreference");
    if (!compilation) {
        LOG(ERROR) << "ANeuralNetworksCompilation_setPreference passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    CompilationBuilder* c = reinterpret_cast<CompilationBuilder*>(compilation);
    return c->setPreference(preference);
}

int ANeuralNetworksCompilation_setCaching(ANeuralNetworksCompilation* compilation,
                                          const char* cacheDir, const uint8_t* token) {
    NNTRACE_RT(NNTRACE_PHASE_COMPILATION, "ANeuralNetworksCompilation_setCaching");
    if (!compilation || !cacheDir || !token) {
        LOG(ERROR) << "ANeuralNetworksCompilation_setCaching passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    CompilationBuilder* c = reinterpret_cast<CompilationBuilder*>(compilation);
    return c->setCaching(cacheDir, token);
}

int ANeuralNetworksCompilation_finish(ANeuralNetworksCompilation* compilation) {
    NNTRACE_RT(NNTRACE_PHASE_COMPILATION, "ANeuralNetworksCompilation_finish");
    if (!compilation) {
        LOG(ERROR) << "ANeuralNetworksCompilation_finish passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    CompilationBuilder* c = reinterpret_cast<CompilationBuilder*>(compilation);
    return c->finish();
}

int ANeuralNetworksExecution_create(ANeuralNetworksCompilation* compilation,
                                    ANeuralNetworksExecution** execution) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksExecution_create");
    if (!compilation || !execution) {
        LOG(ERROR) << "ANeuralNetworksExecution_create passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    CompilationBuilder* c = reinterpret_cast<CompilationBuilder*>(compilation);
    ExecutionBuilder* r = nullptr;
    int result = c->createExecution(&r);
    *execution = reinterpret_cast<ANeuralNetworksExecution*>(r);
    return result;
}

void ANeuralNetworksExecution_free(ANeuralNetworksExecution* execution) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksExecution_free");
    // TODO specification says that an execution-in-flight can be deleted
    // No validation.  Free of nullptr is valid.
    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);
    delete r;
}

int ANeuralNetworksExecution_getOutputOperandRank(ANeuralNetworksExecution* execution,
                                                  int32_t index, uint32_t* rank) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksExecution_getOutputOperandRank");
    if (!execution || !rank) {
        LOG(ERROR) << "ANeuralNetworksExecution_getOutputOperandRank passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);
    return r->getOutputOperandRank(index, rank);
}

int ANeuralNetworksExecution_getOutputOperandDimensions(ANeuralNetworksExecution* execution,
                                                        int32_t index, uint32_t* dimensions) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksExecution_getOutputOperandDimensions");
    if (!execution || !dimensions) {
        LOG(ERROR) << "ANeuralNetworksExecution_getOutputOperandDimensions passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);
    return r->getOutputOperandDimensions(index, dimensions);
}

int ANeuralNetworksExecution_setInput(ANeuralNetworksExecution* execution, int32_t index,
                                      const ANeuralNetworksOperandType* type, const void* buffer,
                                      size_t length) {
    NNTRACE_RT(NNTRACE_PHASE_INPUTS_AND_OUTPUTS, "ANeuralNetworksExecution_setInput");
    if (!execution || (!buffer && length != 0)) {
        LOG(ERROR) << "ANeuralNetworksExecution_setInput passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);
    return r->setInput(index, type, buffer, length);
}

int ANeuralNetworksExecution_setInputFromMemory(ANeuralNetworksExecution* execution, int32_t index,
                                                const ANeuralNetworksOperandType* type,
                                                const ANeuralNetworksMemory* memory, size_t offset,
                                                size_t length) {
    NNTRACE_RT(NNTRACE_PHASE_INPUTS_AND_OUTPUTS, "ANeuralNetworksExecution_setInputFromMemory");
    if (!execution || !memory) {
        LOG(ERROR) << "ANeuralNetworksExecution_setInputFromMemory passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    const Memory* m = reinterpret_cast<const Memory*>(memory);
    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);
    return r->setInputFromMemory(index, type, m, offset, length);
}

int ANeuralNetworksExecution_setOutput(ANeuralNetworksExecution* execution, int32_t index,
                                       const ANeuralNetworksOperandType* type, void* buffer,
                                       size_t length) {
    NNTRACE_RT(NNTRACE_PHASE_INPUTS_AND_OUTPUTS, "ANeuralNetworksExecution_setOutput");
    if (!execution || (!buffer && length != 0)) {
        LOG(ERROR) << "ANeuralNetworksExecution_setOutput passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);
    return r->setOutput(index, type, buffer, length);
}

int ANeuralNetworksExecution_setOutputFromMemory(ANeuralNetworksExecution* execution, int32_t index,
                                                 const ANeuralNetworksOperandType* type,
                                                 const ANeuralNetworksMemory* memory, size_t offset,
                                                 size_t length) {
    NNTRACE_RT(NNTRACE_PHASE_INPUTS_AND_OUTPUTS, "ANeuralNetworksExecution_setOutputFromMemory");
    if (!execution || !memory) {
        LOG(ERROR) << "ANeuralNetworksExecution_setOutputFromMemory passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);
    const Memory* m = reinterpret_cast<const Memory*>(memory);
    return r->setOutputFromMemory(index, type, m, offset, length);
}

int ANeuralNetworksExecution_startCompute(ANeuralNetworksExecution* execution,
                                          ANeuralNetworksEvent** event) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksExecution_startCompute");
    if (!execution || !event) {
        LOG(ERROR) << "ANeuralNetworksExecution_startCompute passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    // TODO validate the rest

    ExecutionBuilder* r = reinterpret_cast<ExecutionBuilder*>(execution);

    // Dynamically allocate an sp to wrap an ExecutionCallback, seen in the NN
    // API as an abstract event object. The sp<ExecutionCallback> object is
    // returned when the execution has been successfully launched, otherwise a
    // nullptr is returned. The sp is used for ref-counting purposes. Without
    // it, the HIDL service could attempt to communicate with a dead callback
    // object.
    std::unique_ptr<sp<ExecutionCallback>> e = std::make_unique<sp<ExecutionCallback>>();
    *event = nullptr;

    int n = r->computeAsynchronously(e.get());
    if (n != ANEURALNETWORKS_NO_ERROR) {
        return n;
    }
    *event = reinterpret_cast<ANeuralNetworksEvent*>(e.release());
    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuralNetworksEvent_wait(ANeuralNetworksEvent* event) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksEvent_wait");
    if (event == nullptr) {
        LOG(ERROR) << "ANeuralNetworksEvent_wait passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    sp<ExecutionCallback>* e = reinterpret_cast<sp<ExecutionCallback>*>(event);
    (*e)->wait();
    return convertErrorStatusToResultCode((*e)->getStatus());
}

void ANeuralNetworksEvent_free(ANeuralNetworksEvent* event) {
    NNTRACE_RT(NNTRACE_PHASE_EXECUTION, "ANeuralNetworksEvent_free");
    // No validation.  Free of nullptr is valid.
    if (event) {
        sp<ExecutionCallback>* e = reinterpret_cast<sp<ExecutionCallback>*>(event);
        (*e)->wait();
        delete e;
    }
}

int ANeuralNetworksDevice_getExtensionSupport(const ANeuralNetworksDevice* device,
                                              const char* extensionName,
                                              bool* isExtensionSupported) {
    if (device == nullptr || extensionName == nullptr || isExtensionSupported == nullptr) {
        LOG(ERROR) << "ANeuralNetworksDevice_getExtensionSupport passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    Device* d = reinterpret_cast<Device*>(const_cast<ANeuralNetworksDevice*>(device));
    hidl_vec<Extension> supportedExtensions = d->getSupportedExtensions();

    *isExtensionSupported = false;
    for (const Extension& supportedExtension : supportedExtensions) {
        if (supportedExtension.name == extensionName) {
            *isExtensionSupported = true;
            break;
        }
    }

    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuralNetworksModel_getExtensionOperandType(ANeuralNetworksModel* model,
                                                 const char* extensionName,
                                                 uint16_t operandCodeWithinExtension,
                                                 int32_t* type) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_getExtensionOperandType");
    if (!model || !extensionName || !type) {
        LOG(ERROR) << "ANeuralNetworksModel_getExtensionOperandType passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->getExtensionType(extensionName, operandCodeWithinExtension, type);
}

int ANeuralNetworksModel_getExtensionOperationType(ANeuralNetworksModel* model,
                                                   const char* extensionName,
                                                   uint16_t operationCodeWithinExtension,
                                                   ANeuralNetworksOperationType* type) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_getExtensionOperationType");
    if (!model || !extensionName || !type) {
        LOG(ERROR) << "ANeuralNetworksModel_getExtensionOperationType passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->getExtensionType(extensionName, operationCodeWithinExtension, type);
}

int ANeuralNetworksModel_setOperandExtensionData(ANeuralNetworksModel* model, int32_t index,
                                                 const void* data, size_t length) {
    NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksModel_setOperandExtensionData");
    if (!model || (!data && length != 0)) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandExtensionData passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    ModelBuilder* m = reinterpret_cast<ModelBuilder*>(model);
    return m->setOperandExtensionData(index, data, length);
}
