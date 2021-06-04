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

#ifndef IMG_UTILS_FILE_OUTPUT_H
#define IMG_UTILS_FILE_OUTPUT_H

#include <img_utils/Output.h>
#include <cutils/compiler.h>
#include <utils/Errors.h>
#include <utils/String8.h>
#include <stdio.h>
#include <stdint.h>

namespace android {
namespace img_utils {

class ANDROID_API FileOutput : public Output {
    public:
        explicit FileOutput(String8 path);
        virtual ~FileOutput();
        virtual status_t open();
        virtual status_t write(const uint8_t* buf, size_t offset, size_t count);
        virtual status_t close();
    private:
        FILE *mFp;
        String8 mPath;
        bool mOpen;
};

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_FILE_OUTPUT_H*/
