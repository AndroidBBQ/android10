/*
 ** Copyright 2019, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#include "otapreopt_utils.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>

using android::base::Join;
using android::base::StringPrintf;

namespace android {
namespace installd {

bool Exec(const std::vector<std::string>& arg_vector, std::string* error_msg) {
    const std::string command_line = Join(arg_vector, ' ');

    CHECK_GE(arg_vector.size(), 1U) << command_line;

    // Convert the args to char pointers.
    const char* program = arg_vector[0].c_str();
    std::vector<char*> args;
    for (size_t i = 0; i < arg_vector.size(); ++i) {
        const std::string& arg = arg_vector[i];
        char* arg_str = const_cast<char*>(arg.c_str());
        CHECK(arg_str != nullptr) << i;
        args.push_back(arg_str);
    }
    args.push_back(nullptr);

    // Fork and exec.
    pid_t pid = fork();
    if (pid == 0) {
        // No allocation allowed between fork and exec.

        // Change process groups, so we don't get reaped by ProcessManager.
        setpgid(0, 0);

        execv(program, &args[0]);

        PLOG(ERROR) << "Failed to execv(" << command_line << ")";
        // _exit to avoid atexit handlers in child.
        _exit(1);
    } else {
        if (pid == -1) {
            *error_msg = StringPrintf("Failed to execv(%s) because fork failed: %s",
                    command_line.c_str(), strerror(errno));
            return false;
        }

        // wait for subprocess to finish
        int status;
        pid_t got_pid = TEMP_FAILURE_RETRY(waitpid(pid, &status, 0));
        if (got_pid != pid) {
            *error_msg = StringPrintf("Failed after fork for execv(%s) because waitpid failed: "
                    "wanted %d, got %d: %s",
                    command_line.c_str(), pid, got_pid, strerror(errno));
            return false;
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            *error_msg = StringPrintf("Failed execv(%s) because non-0 exit status",
                    command_line.c_str());
            return false;
        }
    }
    return true;
}

}  // namespace installd
}  // namespace android
