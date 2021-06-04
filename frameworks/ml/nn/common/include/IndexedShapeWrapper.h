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

#ifndef FRAMEWORKS_ML_NN_INDEXED_SHAPE_WRAPPER_H
#define FRAMEWORKS_ML_NN_INDEXED_SHAPE_WRAPPER_H

#include "OperationsUtils.h"

namespace android {
namespace nn {

// A wrapper over a Shape class implementing some indexing logic for a wrapped
// shape.
// To get an offset for an element in a tensor from vector index, one needs to
// calculate strides first. This class removes the need to recalculate strides
// for every indexing and also provides some utility functions.
class IndexedShapeWrapper {
   public:
    IndexedShapeWrapper(const Shape& wrapped_shape);

    // Calculates the next index in a lexicograpical order for a wrapped shape
    // inplace. Only accepts valid index for a given shape as an input.
    // Sets lastIndex to true if the received index was the last in a
    // lexicographical order for a given shape. In this case, index stays the
    // same.
    bool nextIndexInplace(std::vector<uint32_t>* index, bool* lastIndex) const;

    // Given an index as a vector with per-dimension indices, calculates an
    // offset of the element in a flattened tensor.
    bool indexToFlatIndex(const std::vector<uint32_t>& index, uint32_t* flatIndex) const;

    // Same as indexToFlatIndex, only ignores first dimensions of an index if
    // they are not present in the shape. Also ignores dimensions of a shape of
    // size 1.
    // For example:
    //    for shape:    [3, 1, 2]
    //    and index: [4, 2, 5, 1]
    // the function will ignore dimensions with indices 4 and 5 and set
    // flatIndex to 5 as a result.
    bool broadcastedIndexToFlatIndex(const std::vector<uint32_t>& index, uint32_t* flatIndex) const;

   private:
    const Shape* const shape;
    std::vector<uint32_t> strides;

    bool isValid(const std::vector<uint32_t>& index) const;
};

}  // namespace nn
}  // namespace android

#endif  // FRAMEWORKS_ML_NN_INDEXED_SHAPE_WRAPPER_H
