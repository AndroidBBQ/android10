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

#include <img_utils/Input.h>

namespace android {
namespace img_utils {

Input::~Input() {}

status_t Input::open() { return OK; }

status_t Input::close() { return OK; }

ssize_t Input::skip(size_t count) {
    const size_t SKIP_BUF_SIZE = 1024;
    uint8_t skipBuf[SKIP_BUF_SIZE];

    size_t remaining = count;
    while (remaining > 0) {
        size_t amt = (SKIP_BUF_SIZE > remaining) ? remaining : SKIP_BUF_SIZE;
        ssize_t ret = read(skipBuf, 0, amt);
        if (ret < 0) {
            if(ret == NOT_ENOUGH_DATA) {
                // End of file encountered
                if (remaining == count) {
                    // Read no bytes, return EOF
                    return NOT_ENOUGH_DATA;
                } else {
                    // Return num bytes read
                    return count - remaining;
                }
            }
            // Return error code.
            return ret;
        }
        remaining -= ret;
    }
    return count;
}

} /*namespace img_utils*/
} /*namespace android*/

