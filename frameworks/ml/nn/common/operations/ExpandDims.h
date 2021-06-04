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

#ifndef FRAMEWORKS_ML_NN_EXPAND_DIMS_H
#define FRAMEWORKS_ML_NN_EXPAND_DIMS_H

#include "CpuOperationUtils.h"

namespace android {
namespace nn {
namespace expand_dims {

bool prepare(const Shape& input, int32_t axis, Shape* output);

bool eval(const uint8_t* inputData, const Shape& inputShape, int32_t axis, uint8_t* outputData,
          const Shape& outputShape);

}  // namespace expand_dims
}  // namespace nn
}  // namespace android

#endif  // FRAMEWORKS_ML_NN_EXPAND_DIMS_H
