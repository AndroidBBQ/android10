/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /*
  * rss_hwm_reset clears the RSS high-water mark counters for all currently
  * running processes. It writes "5" to /proc/PID/clear_refs for every PID.
  *
  * It runs in its own process becuase dac_override capability is required
  * in order to write to other processes' clear_refs.
  *
  * It is invoked from a system service by flipping sys.rss_hwm_reset.on
  * property to "1".
  */

#define LOG_TAG "rss_hwm_reset"

#include <dirent.h>

#include <string>

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <log/log.h>

namespace {
// Resets RSS HWM counter for the selected process by writing 5 to
// /proc/PID/clear_refs.
void reset_rss_hwm(const char* pid) {
    std::string clear_refs_path =
            ::android::base::StringPrintf("/proc/%s/clear_refs", pid);
    ::android::base::WriteStringToFile("5", clear_refs_path);
}
}

// Clears RSS HWM counters for all currently running processes.
int main(int /* argc */, char** /* argv[] */) {
    DIR* dirp = opendir("/proc");
    if (dirp == nullptr) {
        ALOGE("unable to read /proc");
        return 1;
    }
    struct dirent* entry;
    while ((entry = readdir(dirp)) != nullptr) {
        // Skip entries that are not directories.
        if (entry->d_type != DT_DIR) continue;
        // Skip entries that do not contain only numbers.
        const char* pid = entry->d_name;
        while (*pid) {
            if (*pid < '0' || *pid > '9') break;
            pid++;
        }
        if (*pid != 0) continue;

        pid = entry->d_name;
        reset_rss_hwm(pid);
    }
    closedir(dirp);
    return 0;
}
