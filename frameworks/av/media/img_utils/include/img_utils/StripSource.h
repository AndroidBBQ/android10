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

#ifndef IMG_UTILS_STRIP_SOURCE_H
#define IMG_UTILS_STRIP_SOURCE_H

#include <img_utils/Output.h>

#include <cutils/compiler.h>
#include <utils/Errors.h>

#include <stdint.h>

namespace android {
namespace img_utils {

/**
 * This class acts as a data source for strips set in a TiffIfd.
 */
class ANDROID_API StripSource {
    public:
        virtual ~StripSource();

        /**
         * Write count bytes to the stream.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t writeToStream(Output& stream, uint32_t count) = 0;

        /**
         * Return the source IFD.
         */
        virtual uint32_t getIfd() const = 0;
};

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_STRIP_SOURCE_H*/
