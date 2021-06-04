/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "view_compiler.h"

#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

#include "android-base/logging.h"
#include "android-base/stringprintf.h"
#include "android-base/unique_fd.h"

namespace android {
namespace installd {

using base::unique_fd;

bool view_compiler(const char* apk_path, const char* package_name, const char* out_dex_file,
                   int uid) {
    CHECK(apk_path != nullptr);
    CHECK(package_name != nullptr);
    CHECK(out_dex_file != nullptr);

    // viewcompiler won't have permission to open anything, so we have to open the files first
    // and pass file descriptors.

    // Open input file
    unique_fd infd{open(apk_path, O_RDONLY)}; // NOLINT(android-cloexec-open)
    if (infd.get() < 0) {
        PLOG(ERROR) << "Could not open input file: " << apk_path;
        return false;
    }

    // Set up output file. viewcompiler can't open outputs by fd, but it can write to stdout, so
    // we close stdout and open it towards the right output.
    unique_fd outfd{open(out_dex_file, O_CREAT | O_TRUNC | O_WRONLY | O_CLOEXEC, 0644)};
    if (outfd.get() < 0) {
        PLOG(ERROR) << "Could not open output file: " << out_dex_file;
        return false;
    }
    if (fchmod(outfd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0) {
        PLOG(ERROR) << "Could not change output file permissions";
        return false;
    }
    if (dup2(outfd, STDOUT_FILENO) < 0) {
        PLOG(ERROR) << "Could not duplicate output file descriptor";
        return false;
    }

    // Prepare command line arguments for viewcompiler
    std::string args[] = {"/system/bin/viewcompiler",
                          "--apk",
                          "--infd",
                          android::base::StringPrintf("%d", infd.get()),
                          "--dex",
                          "--package",
                          package_name};
    char* const argv[] = {const_cast<char*>(args[0].c_str()), const_cast<char*>(args[1].c_str()),
                          const_cast<char*>(args[2].c_str()), const_cast<char*>(args[3].c_str()),
                          const_cast<char*>(args[4].c_str()), const_cast<char*>(args[5].c_str()),
                          const_cast<char*>(args[6].c_str()), nullptr};

    pid_t pid = fork();
    if (pid == 0) {
        // Now that we've opened the files we need, drop privileges.
        drop_capabilities(uid);
        execv("/system/bin/viewcompiler", argv);
        _exit(1);
    }

    return wait_child(pid) == 0;
}

} // namespace installd
} // namespace android
