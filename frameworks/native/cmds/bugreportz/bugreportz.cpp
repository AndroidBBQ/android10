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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include <android-base/file.h>
#include <android-base/strings.h>

#include "bugreportz.h"

static constexpr char BEGIN_PREFIX[] = "BEGIN:";
static constexpr char PROGRESS_PREFIX[] = "PROGRESS:";

static void write_line(const std::string& line, bool show_progress) {
    if (line.empty()) return;

    // When not invoked with the -p option, it must skip BEGIN and PROGRESS lines otherwise it
    // will break adb (which is expecting either OK or FAIL).
    if (!show_progress && (android::base::StartsWith(line, PROGRESS_PREFIX) ||
                           android::base::StartsWith(line, BEGIN_PREFIX)))
        return;

    android::base::WriteStringToFd(line, STDOUT_FILENO);
}

int bugreportz(int s, bool show_progress) {
    std::string line;
    while (1) {
        char buffer[65536];
        ssize_t bytes_read = TEMP_FAILURE_RETRY(read(s, buffer, sizeof(buffer)));
        if (bytes_read == 0) {
            break;
        } else if (bytes_read == -1) {
            // EAGAIN really means time out, so change the errno.
            if (errno == EAGAIN) {
                errno = ETIMEDOUT;
            }
            printf("FAIL:Bugreport read terminated abnormally (%s)\n", strerror(errno));
            return EXIT_FAILURE;
        }

        // Writes line by line.
        for (int i = 0; i < bytes_read; i++) {
            char c = buffer[i];
            line.append(1, c);
            if (c == '\n') {
                write_line(line, show_progress);
                line.clear();
            }
        }
    }
    // Process final line, in case it didn't finish with newline
    write_line(line, show_progress);

    return EXIT_SUCCESS;
}
