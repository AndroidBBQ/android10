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

#ifndef IMG_UTILS_ORDERABLE
#define IMG_UTILS_ORDERABLE

#include <cutils/compiler.h>
#include <stdint.h>

namespace android {
namespace img_utils {

#define COMPARE_DEF(op) \
inline bool operator op (const Orderable& orderable) const;

/**
 * Subclasses of Orderable can be compared and sorted.  This is
 * intended to be used to create sorted arrays of TIFF entries
 * and IFDs.
 */
class ANDROID_API Orderable  {
    public:
        virtual ~Orderable();

        /**
         * Comparison operatotors are based on the value returned
         * from this method.
         */
        virtual uint32_t getComparableValue() const = 0;

        COMPARE_DEF(>)
        COMPARE_DEF(<)
        COMPARE_DEF(>=)
        COMPARE_DEF(<=)
        COMPARE_DEF(==)
        COMPARE_DEF(!=)
};

#undef COMPARE_DEF

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_ORDERABLE*/
