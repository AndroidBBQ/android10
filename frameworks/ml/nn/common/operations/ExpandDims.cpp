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

#define LOG_TAG "Operations"

#include "ExpandDims.h"

#include "Utils.h"

namespace android {
namespace nn {
namespace expand_dims {

bool prepare(const Shape& input, int32_t axis, Shape* output) {
    NN_CHECK(handleNegativeAxis(getNumberOfDimensions(input) + 1, &axis));

    output->type = input.type;
    output->offset = input.offset;
    output->scale = input.scale;

    output->dimensions.assign(input.dimensions.begin(), input.dimensions.end());
    output->dimensions.insert(output->dimensions.begin() + axis, 1);

    return true;
}

bool eval(const uint8_t* inputData, const Shape& inputShape, int32_t axis, uint8_t* outputData,
          const Shape& outputShape) {
    memcpy(outputData, inputData,
           nonExtensionOperandSizeOfData(inputShape.type, inputShape.dimensions));
    return true;
}

}  // namespace expand_dims
}  // namespace nn
}  // namespace android
