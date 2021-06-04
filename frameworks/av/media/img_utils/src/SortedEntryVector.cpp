/*
 * Copyright 2014 The Android Open Source Project
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

#include <img_utils/SortedEntryVector.h>

#include <utils/TypeHelpers.h>
#include <utils/Log.h>

namespace android {
namespace img_utils {

SortedEntryVector::~SortedEntryVector() {}

ssize_t SortedEntryVector::indexOfTag(uint16_t tag) const {
    // TODO: Use binary search here.
    for (size_t i = 0; i < size(); ++i) {
        if (itemAt(i)->getTag() == tag) {
            return i;
        }
    }
    return -1;
}

int SortedEntryVector::do_compare(const void* lhs, const void* rhs) const {
    const sp<TiffEntry>* lEntry = reinterpret_cast<const sp<TiffEntry>*>(lhs);
    const sp<TiffEntry>* rEntry = reinterpret_cast<const sp<TiffEntry>*>(rhs);
    return compare_type(**lEntry, **rEntry);
}

} /*namespace img_utils*/
} /*namespace android*/
