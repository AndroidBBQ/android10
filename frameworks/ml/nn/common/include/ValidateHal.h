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

#ifndef ANDROID_ML_NN_COMMON_VALIDATEHAL_H
#define ANDROID_ML_NN_COMMON_VALIDATEHAL_H

#include "HalInterfaces.h"

namespace android {
namespace nn {

enum class HalVersion : int32_t {
    UNKNOWN,
    V1_0,
    V1_1,
    V1_2,
    LATEST = V1_2,
};

// Verifies that the model is valid, i.e. it is consistent, takes
// only acceptable values, the constants don't extend outside the memory
// regions they are part of, etc.
// IMPORTANT: This function cannot validate that OEM operation and operands
// are correctly defined, as these are specific to each implementation.
// Each driver should do their own validation of OEM types.
template <class T_Model>
bool validateModel(const T_Model& model);

// Verfies that the request for the given model is valid.
// IMPORTANT: This function cannot validate that OEM operation and operands
// are correctly defined, as these are specific to each implementation.
// Each driver should do their own validation of OEM types.
template <class T_Model>
bool validateRequest(const Request& request, const T_Model& model);

// Verfies that the execution preference is valid.
bool validateExecutionPreference(ExecutionPreference preference);

bool validOperationType(V1_0::OperationType operation);
bool validOperationType(V1_1::OperationType operation);
bool validOperationType(V1_2::OperationType operation);

bool validOperandType(V1_0::OperandType operand);
bool validOperandType(V1_2::OperandType operand);

// Verfies that the memory pool is valid in the specified HAL version.
bool validatePool(const hidl_memory& pool, HalVersion ver = HalVersion::LATEST);

}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_COMMON_VALIDATEHAL_H
