/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef _MEDIA_UTILS_H
#define _MEDIA_UTILS_H

namespace android {

/**
   Limit the amount of memory a process can allocate using setrlimit(RLIMIT_AS).
   The value to use will be read from the specified system property, or if the
   property doesn't exist it will use the specified number of bytes or the
   specified percentage of total memory, whichever is smaller.
*/
void limitProcessMemory(
    const char *property,
    size_t numberOfBytes,
    size_t percentageOfTotalMem);

}   // namespace android

#endif  // _MEDIA_UTILS_H
