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

#ifndef FRAMEWORKS_ML_NN_HAL_OPERATION_H
#define FRAMEWORKS_ML_NN_HAL_OPERATION_H

namespace android {
namespace hardware {
namespace neuralnetworks {
namespace V1_2 {

// Individual operation implementations should not depend on the HAL interface,
// but we have some that do. We use a forward declaration instead of an explicit
// blueprint dependency to hide this fact.
struct Operation;

}  // namespace V1_2
}  // namespace neuralnetworks
}  // namespace hardware
}  // namespace android

using ::android::hardware::neuralnetworks::V1_2::Operation;

#endif  // FRAMEWORKS_ML_NN_HAL_OPERATION_H
