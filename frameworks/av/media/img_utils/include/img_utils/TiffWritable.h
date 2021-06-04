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

#ifndef IMG_UTILS_TIFF_WRITABLE
#define IMG_UTILS_TIFF_WRITABLE

#include <img_utils/Orderable.h>
#include <img_utils/EndianUtils.h>
#include <img_utils/Output.h>

#include <cutils/compiler.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <stdint.h>

namespace android {
namespace img_utils {

/**
 * TiffWritable subclasses represent TIFF metadata objects that can be written
 * to an EndianOutput object.  This is used for TIFF entries and IFDs.
 */
class ANDROID_API TiffWritable : public Orderable, public LightRefBase<TiffWritable> {
    public:
        TiffWritable();
        virtual ~TiffWritable();

        /**
         * Write the data to the output. The given offset is used to calculate
         * the header offset for values written.  The offset is defined
         * relative to the beginning of the TIFF header, and is word aligned.
         *
         * Returns OK on success, or a negative error code on failure.
         */
        virtual status_t writeData(uint32_t offset, /*out*/EndianOutput* out) const = 0;

        /**
         * Get the size of the data to write.
         */
        virtual size_t getSize() const = 0;

};

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_TIFF_WRITABLE*/
