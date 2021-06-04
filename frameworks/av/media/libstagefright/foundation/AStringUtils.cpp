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

#include <string.h>
#include <AStringUtils.h>

namespace android {

// static
int AStringUtils::Compare(const char *a, const char *b, size_t len, bool ignoreCase) {
    // this method relies on a trailing '\0' if a or b are shorter than len
    return ignoreCase ? strncasecmp(a, b, len) : strncmp(a, b, len);
}

// static
bool AStringUtils::MatchesGlob(
        const char *glob, size_t globLen, const char *str, size_t strLen, bool ignoreCase) {
    // this method does not assume a trailing '\0'
    size_t ix = 0, globIx = 0;

    // pattern must match until first '*'
    while (globIx < globLen && glob[globIx] != '*') {
        ++globIx;
    }
    if (strLen < globIx || Compare(str, glob, globIx /* len */, ignoreCase)) {
        return false;
    }
    ix = globIx;

    // process by * separated sections
    while (globIx < globLen) {
        ++globIx;
        size_t start = globIx;
        while (globIx < globLen && glob[globIx] != '*') {
            ++globIx;
        }
        size_t len = globIx - start;
        const char *pattern = glob + start;

        if (globIx == globLen) {
            // last pattern must match tail
            if (ix + len > strLen) {
                return false;
            }
            const char *tail = str + strLen - len;
            return !Compare(tail, pattern, len, ignoreCase);
        }
        // progress after first occurrence of pattern
        while (ix + len <= strLen && Compare(str + ix, pattern, len, ignoreCase)) {
            ++ix;
        }
        if (ix + len > strLen) {
            return false;
        }
        ix += len;
        // we will loop around as globIx < globLen
    }

    // we only get here if there were no * in the pattern
    return ix == strLen;
}

}  // namespace android

