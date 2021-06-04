/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <unistd.h>

#include <android/log.h>
#define ATRACE_TAG ATRACE_TAG_NNAPI
#include "utils/Trace.h"

int parseMemInfo(const char* name) {
    std::ifstream meminfoStream("/proc/meminfo");
    if (!meminfoStream.good()) {
        perror("Failed to open /proc/meminfo");
        return -1;
    }
    std::string line;
    while (std::getline(meminfoStream, line)) {
        if (line.find(name) != std::string::npos) {
            std::istringstream lineStream(line);
            std::string name;
            int size;
            lineStream >> name;
            lineStream >> size;
            return size;
        }
    }
    std::cerr << "Failed to find " << name << " in /proc/meminfo\n";
    return -1;
}

int main(void) {
    if (!(atrace_get_enabled_tags() & ATRACE_TAG)) {
        std::cerr << "systrace not running, logcat output only\n";
    }
    int size = 0;
    while (true) {
      const int newSize = parseMemInfo("ION_heap");
      if (newSize < 0) {
          return newSize;
      }
      if (newSize != size) {
        size = newSize;
        std::cout << size << "\n";
        ATRACE_INT("ION_heap", size);
        __android_log_print(ANDROID_LOG_INFO, "ion", "ION_heap %d", size);
      }
      usleep(10);
    }
}
