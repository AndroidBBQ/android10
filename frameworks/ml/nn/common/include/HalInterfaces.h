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

#ifndef ANDROID_ML_NN_COMMON_HAL_INTERFACES_H
#define ANDROID_ML_NN_COMMON_HAL_INTERFACES_H

#include <android/hardware/neuralnetworks/1.0/IDevice.h>
#include <android/hardware/neuralnetworks/1.0/IExecutionCallback.h>
#include <android/hardware/neuralnetworks/1.0/IPreparedModel.h>
#include <android/hardware/neuralnetworks/1.0/IPreparedModelCallback.h>
#include <android/hardware/neuralnetworks/1.0/types.h>
#include <android/hardware/neuralnetworks/1.1/IDevice.h>
#include <android/hardware/neuralnetworks/1.1/types.h>
#include <android/hardware/neuralnetworks/1.2/IDevice.h>
#include <android/hardware/neuralnetworks/1.2/IExecutionCallback.h>
#include <android/hardware/neuralnetworks/1.2/IPreparedModel.h>
#include <android/hardware/neuralnetworks/1.2/IPreparedModelCallback.h>
#include <android/hardware/neuralnetworks/1.2/types.h>
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>

using ::android::sp;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::neuralnetworks::V1_0::DataLocation;
using ::android::hardware::neuralnetworks::V1_0::DeviceStatus;
using ::android::hardware::neuralnetworks::V1_0::ErrorStatus;
using ::android::hardware::neuralnetworks::V1_0::FusedActivationFunc;
using ::android::hardware::neuralnetworks::V1_0::OperandLifeTime;
using ::android::hardware::neuralnetworks::V1_0::PerformanceInfo;
using ::android::hardware::neuralnetworks::V1_0::Request;
using ::android::hardware::neuralnetworks::V1_0::RequestArgument;
using ::android::hardware::neuralnetworks::V1_1::ExecutionPreference;
using ::android::hardware::neuralnetworks::V1_2::Capabilities;
using ::android::hardware::neuralnetworks::V1_2::Constant;
using ::android::hardware::neuralnetworks::V1_2::DeviceType;
using ::android::hardware::neuralnetworks::V1_2::Extension;
using ::android::hardware::neuralnetworks::V1_2::FmqRequestDatum;
using ::android::hardware::neuralnetworks::V1_2::FmqResultDatum;
using ::android::hardware::neuralnetworks::V1_2::IBurstCallback;
using ::android::hardware::neuralnetworks::V1_2::IBurstContext;
using ::android::hardware::neuralnetworks::V1_2::IDevice;
using ::android::hardware::neuralnetworks::V1_2::IExecutionCallback;
using ::android::hardware::neuralnetworks::V1_2::IPreparedModel;
using ::android::hardware::neuralnetworks::V1_2::IPreparedModelCallback;
using ::android::hardware::neuralnetworks::V1_2::MeasureTiming;
using ::android::hardware::neuralnetworks::V1_2::Model;
using ::android::hardware::neuralnetworks::V1_2::Operand;
using ::android::hardware::neuralnetworks::V1_2::OperandType;
using ::android::hardware::neuralnetworks::V1_2::OperandTypeRange;
using ::android::hardware::neuralnetworks::V1_2::Operation;
using ::android::hardware::neuralnetworks::V1_2::OperationType;
using ::android::hardware::neuralnetworks::V1_2::OperationTypeRange;
using ::android::hardware::neuralnetworks::V1_2::OutputShape;
using ::android::hardware::neuralnetworks::V1_2::SymmPerChannelQuantParams;
using ::android::hardware::neuralnetworks::V1_2::Timing;
using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hidl::memory::V1_0::IMemory;

namespace V1_0 = ::android::hardware::neuralnetworks::V1_0;
namespace V1_1 = ::android::hardware::neuralnetworks::V1_1;
namespace V1_2 = ::android::hardware::neuralnetworks::V1_2;

namespace android {
namespace nn {

} // namespace nn
} // namespace android

#endif // ANDROID_ML_NN_COMMON_HAL_INTERFACES_H
