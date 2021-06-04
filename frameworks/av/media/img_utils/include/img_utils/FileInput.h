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

#ifndef IMG_UTILS_FILE_INPUT_H
#define IMG_UTILS_FILE_INPUT_H

#include <img_utils/Input.h>

#include <cutils/compiler.h>
#include <utils/Errors.h>
#include <utils/String8.h>
#include <stdio.h>
#include <stdint.h>

namespace android {
namespace img_utils {

/**
 * Utility class for reading from a file.
 */
class ANDROID_API FileInput : public Input {
    public:
        /**
         * Create a file input for the given path.
         */
        explicit FileInput(String8 path);

        virtual ~FileInput();

        /**
         * Open a file descriptor to the path given in the constructor.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t open();

        /**
         * Read bytes from the file into the given buffer.  At most, the number
         * of bytes given in the count argument will be read.  Bytes will be written
         * into the given buffer starting at the index given in the offset argument.
         *
         * Returns the number of bytes read, or NOT_ENOUGH_DATA if at the end of the file.  If an
         * error has occurred, this will return a negative error code other than NOT_ENOUGH_DATA.
         */
        virtual ssize_t read(uint8_t* buf, size_t offset, size_t count);

        /**
         * Close the file descriptor to the path given in the constructor.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t close();
    private:
        FILE *mFp;
        String8 mPath;
        bool mOpen;
};

} /*namespace img_utils*/
} /*namespace android*/


#endif /*IMG_UTILS_INPUT_H*/
