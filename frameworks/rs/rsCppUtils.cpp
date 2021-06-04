/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include "rsUtils.h"
#include "rsCppUtils.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <string>

#ifndef RS_COMPATIBILITY_LIB
#include <sys/wait.h>
#endif


namespace android {
namespace renderscript {

const char * rsuCopyString(const char *name) {
    return rsuCopyString(name, strlen(name));
}

const char * rsuCopyString(const char *name, size_t len) {
    char *n = new char[len+1];
    memcpy(n, name, len);
    n[len] = 0;
    return n;
}

const char* rsuJoinStrings(int n, const char* const* strs) {
    std::string tmp;
    for (int i = 0; i < n; i++) {
        if (i > 0) {
            tmp.append(" ");
        }
        tmp.append(strs[i]);
    }
    return strndup(tmp.c_str(), tmp.size());
}

#ifndef RS_COMPATIBILITY_LIB
bool rsuExecuteCommand(const char *exe, int nArgs, const char * const *args) {
    std::unique_ptr<const char> joined(rsuJoinStrings(nArgs, args));
    ALOGV("Invoking %s with args '%s'", exe, joined.get());

    pid_t pid = fork();

    switch (pid) {
    case -1: {  // Error occurred (we attempt no recovery)
        ALOGE("Fork of \"%s\" failed with error %s", exe, strerror(errno));
        return false;
    }
    case 0: {  // Child process
        // No (direct or indirect) call to malloc between fork and exec.  It is
        // possible that a different thread holds the heap lock before the fork.

        // ProcessManager in libcore can reap unclaimed SIGCHLDs in its process
        // group.  To ensure that the exit signal is not caught by
        // ProcessManager and instead sent to libRS, set the child's PGID to its
        // PID.
        setpgid(0, 0);

        execv(exe, (char * const *)args);

        ALOGE("execv() failed: %s", strerror(errno));
        abort();
        return false;
    }
    default: {  // Parent process (actual driver)
        // Wait on child process to finish execution.
        int status = 0;
        pid_t w = TEMP_FAILURE_RETRY(waitpid(pid, &status, 0));
        if (w == -1) {
            ALOGE("Waitpid of \"%s\" failed with error %s", exe,
                  strerror(errno));
            return false;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        }

        ALOGE("Child process \"%s\" terminated with status %d", exe, status);
        return false;
    }
    }
}
#endif // RS_COMPATIBILITY_LIB

// Implementation of property_get from libcutils
int property_get(const char *key, char *value, const char *default_value) {
    int len;
    len = __system_property_get(key, value);
    if (len > 0) {
        return len;
    }

    if (default_value) {
        len = strlen(default_value);
        memcpy(value, default_value, len + 1);
    }
    return len;
}

} // namespace renderscript
} // namespace android
