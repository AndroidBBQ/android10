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

#ifndef IMG_UTILS_BYTE_ARRAY_OUTPUT_H
#define IMG_UTILS_BYTE_ARRAY_OUTPUT_H

#include <img_utils/Output.h>

#include <utils/Errors.h>
#include <utils/Vector.h>

#include <cutils/compiler.h>
#include <stdint.h>

namespace android {
namespace img_utils {

/**
 * Utility class that accumulates written bytes into a buffer.
 */
class ANDROID_API ByteArrayOutput : public Output {
    public:

        ByteArrayOutput();

        virtual ~ByteArrayOutput();

        /**
         * Open this ByteArrayOutput.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t open();

        /**
         * Write bytes from the given buffer.  The number of bytes given in the count
         * argument will be written.  Bytes will be written from the given buffer starting
         * at the index given in the offset argument.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t write(const uint8_t* buf, size_t offset, size_t count);

        /**
         * Close this ByteArrayOutput.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t close();

        /**
         * Get current size of the array of bytes written.
         */
        virtual size_t getSize() const;

        /**
         * Get pointer to array of bytes written.  It is not valid to use this pointer if
         * open, write, or close is called after this method.
         */
        virtual const uint8_t* getArray() const;

    protected:
        Vector<uint8_t> mByteArray;
};

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_BYTE_ARRAY_OUTPUT_H*/
