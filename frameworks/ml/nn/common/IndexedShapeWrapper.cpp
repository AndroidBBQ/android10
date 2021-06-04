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

#define LOG_TAG "IndexedShapeWrapper"

#include "IndexedShapeWrapper.h"

namespace android {
namespace nn {

IndexedShapeWrapper::IndexedShapeWrapper(const Shape& wrapped_shape) : shape(&wrapped_shape) {
    strides.resize(shape->dimensions.size());
    strides.back() = 1;
    for (int i = strides.size() - 2; i >= 0; --i) {
        strides[i] = shape->dimensions[i + 1] * strides[i + 1];
    }
}

bool IndexedShapeWrapper::nextIndexInplace(std::vector<uint32_t>* index, bool* lastIndex) const {
    NN_CHECK(isValid(*index));

    bool anyIndicesLeft = false;
    for (int i = 0; i < index->size(); ++i) {
        if (index->at(i) < shape->dimensions[i] - 1) {
            anyIndicesLeft = true;
            break;
        }
    }
    if (!anyIndicesLeft) {
        *lastIndex = true;
        return true;
    }
    for (int i = index->size() - 1; i >= 0; --i) {
        ++index->at(i);
        if (index->at(i) == shape->dimensions[i]) {
            index->at(i) = 0;
        } else {
            break;
        }
    }
    return true;
}

bool IndexedShapeWrapper::indexToFlatIndex(const std::vector<uint32_t>& index,
                                           uint32_t* flatIndex) const {
    NN_CHECK(isValid(index));

    *flatIndex = 0;
    for (int i = 0; i < index.size(); ++i) {
        *flatIndex += strides[i] * index[i];
    }
    return true;
}

bool IndexedShapeWrapper::broadcastedIndexToFlatIndex(const std::vector<uint32_t>& index,
                                                      uint32_t* flatIndex) const {
    NN_CHECK(index.size() >= strides.size());

    *flatIndex = 0;
    for (int i = 1; i <= strides.size(); ++i) {
        uint32_t currentIndex = index[index.size() - i];
        uint32_t currentDimSize = shape->dimensions[shape->dimensions.size() - i];
        NN_CHECK(currentIndex < currentDimSize || currentDimSize == 1);
        if (currentDimSize != 1) {
            *flatIndex += strides[strides.size() - i] * index[index.size() - i];
        }
    }
    return true;
}

bool IndexedShapeWrapper::isValid(const std::vector<uint32_t>& index) const {
    if (index.size() != shape->dimensions.size()) {
        LOG(ERROR) << "Index: " << toString(index)
                   << " has a different number of dimensions from shape: "
                   << toString(shape->dimensions);
        return false;
    }
    for (int i = 0; i < index.size(); ++i) {
        if (index[i] >= shape->dimensions[i]) {
            LOG(ERROR) << "Invalid index: " << toString(index)
                       << " is out of range for shape: " << toString(shape->dimensions);
            return false;
        }
    }
    return true;
}

}  // namespace nn
}  // namespace android
