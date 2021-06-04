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
#ifndef FRAMEWORK_NATIVE_CMD_DUMPSTATE_INTERNAL_H_
#define FRAMEWORK_NATIVE_CMD_DUMPSTATE_INTERNAL_H_

#include <cstdint>
#include <string>

// TODO: rename macros to DUMPSTATE_LOGXXX
#ifndef MYLOGD
#define MYLOGD(...)               \
    fprintf(stderr, __VA_ARGS__); \
    ALOGD(__VA_ARGS__);
#endif

#ifndef MYLOGI
#define MYLOGI(...)               \
    fprintf(stderr, __VA_ARGS__); \
    ALOGI(__VA_ARGS__);
#endif

#ifndef MYLOGW
#define MYLOGW(...)               \
    fprintf(stderr, __VA_ARGS__); \
    ALOGW(__VA_ARGS__);
#endif

#ifndef MYLOGE
#define MYLOGE(...)               \
    fprintf(stderr, __VA_ARGS__); \
    ALOGE(__VA_ARGS__);
#endif

// Internal functions used by .cpp files on multiple build targets.
// TODO: move to android::os::dumpstate::internal namespace

// TODO: use functions from <chrono> instead
const uint64_t NANOS_PER_SEC = 1000000000;
uint64_t Nanotime();

// Switches to non-root user and group.
bool DropRootUser();

// TODO: move to .cpp as static once is not used by utils.cpp anymore.
int DumpFileFromFdToFd(const std::string& title, const std::string& path_string, int fd, int out_fd,
                       bool dry_run = false);

#endif  // FRAMEWORK_NATIVE_CMD_DUMPSTATE_INTERNAL_H_
