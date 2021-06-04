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

#ifndef IMG_UTILS_SORTED_ENTRY_VECTOR_H
#define IMG_UTILS_SORTED_ENTRY_VECTOR_H

#include <img_utils/TiffEntry.h>

#include <utils/StrongPointer.h>
#include <utils/SortedVector.h>

namespace android {
namespace img_utils {

/**
 * Subclass of SortedVector that has been extended to
 * do comparisons/lookups based on the tag ID of the entries.
 */
class SortedEntryVector : public SortedVector<sp<TiffEntry> > {
    public:
        virtual ~SortedEntryVector();

        /**
         * Returns the index of the entry with the given tag ID, or
         * -1 if none exists.
         */
        ssize_t indexOfTag(uint16_t tag) const;

    protected:
        /**
         * Compare tag ID.
         */
        virtual int do_compare(const void* lhs, const void* rhs) const;
};


} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_SORTED_ENTRY_VECTOR_H*/
