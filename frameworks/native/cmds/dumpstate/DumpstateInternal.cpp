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

#define LOG_TAG "dumpstate"

#include "DumpstateInternal.h"

#include <errno.h>
#include <grp.h>
#include <poll.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <string>
#include <vector>

#include <android-base/file.h>
#include <android-base/macros.h>
#include <log/log.h>

uint64_t Nanotime() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec * NANOS_PER_SEC + ts.tv_nsec);
}

// Switches to non-root user and group.
bool DropRootUser() {
    struct group* grp = getgrnam("shell");
    gid_t shell_gid = grp != nullptr ? grp->gr_gid : 0;
    struct passwd* pwd = getpwnam("shell");
    uid_t shell_uid = pwd != nullptr ? pwd->pw_uid : 0;

    if (!shell_gid || !shell_uid) {
        MYLOGE("Unable to get AID_SHELL: %s\n", strerror(errno));
        return false;
    }

    if (getgid() == shell_gid && getuid() == shell_uid) {
        MYLOGD("drop_root_user(): already running as Shell\n");
        return true;
    }
    /* ensure we will keep capabilities when we drop root */
    if (prctl(PR_SET_KEEPCAPS, 1) < 0) {
        MYLOGE("prctl(PR_SET_KEEPCAPS) failed: %s\n", strerror(errno));
        return false;
    }

    static const std::vector<std::string> group_names{
        "log", "sdcard_r", "sdcard_rw", "mount", "inet", "net_bw_stats", "readproc", "bluetooth"};
    std::vector<gid_t> groups(group_names.size(), 0);
    for (size_t i = 0; i < group_names.size(); ++i) {
        grp = getgrnam(group_names[i].c_str());
        groups[i] = grp != nullptr ? grp->gr_gid : 0;
        if (groups[i] == 0) {
            MYLOGE("Unable to get required gid '%s': %s\n", group_names[i].c_str(),
                   strerror(errno));
            return false;
        }
    }

    if (setgroups(groups.size(), groups.data()) != 0) {
        MYLOGE("Unable to setgroups, aborting: %s\n", strerror(errno));
        return false;
    }
    if (setgid(shell_gid) != 0) {
        MYLOGE("Unable to setgid, aborting: %s\n", strerror(errno));
        return false;
    }
    if (setuid(shell_uid) != 0) {
        MYLOGE("Unable to setuid, aborting: %s\n", strerror(errno));
        return false;
    }

    struct __user_cap_header_struct capheader;
    struct __user_cap_data_struct capdata[2];
    memset(&capheader, 0, sizeof(capheader));
    memset(&capdata, 0, sizeof(capdata));
    capheader.version = _LINUX_CAPABILITY_VERSION_3;
    capheader.pid = 0;

    if (capget(&capheader, &capdata[0]) != 0) {
        MYLOGE("capget failed: %s\n", strerror(errno));
        return false;
    }

    const uint32_t cap_syslog_mask = CAP_TO_MASK(CAP_SYSLOG);
    const uint32_t cap_syslog_index = CAP_TO_INDEX(CAP_SYSLOG);
    bool has_cap_syslog = (capdata[cap_syslog_index].effective & cap_syslog_mask) != 0;

    memset(&capdata, 0, sizeof(capdata));
    if (has_cap_syslog) {
        // Only attempt to keep CAP_SYSLOG if it was present to begin with.
        capdata[cap_syslog_index].permitted |= cap_syslog_mask;
        capdata[cap_syslog_index].effective |= cap_syslog_mask;
    }

    if (capset(&capheader, &capdata[0]) != 0) {
        MYLOGE("capset({%#x, %#x}) failed: %s\n", capdata[0].effective,
               capdata[1].effective, strerror(errno));
        return false;
    }

    return true;
}

int DumpFileFromFdToFd(const std::string& title, const std::string& path_string, int fd, int out_fd,
                       bool dry_run) {
    const char* path = path_string.c_str();
    if (!title.empty()) {
        dprintf(out_fd, "------ %s (%s", title.c_str(), path);

        struct stat st;
        // Only show the modification time of non-device files.
        size_t path_len = strlen(path);
        if ((path_len < 6 || memcmp(path, "/proc/", 6)) &&
            (path_len < 5 || memcmp(path, "/sys/", 5)) &&
            (path_len < 3 || memcmp(path, "/d/", 3)) && !fstat(fd, &st)) {
            char stamp[80];
            time_t mtime = st.st_mtime;
            strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", localtime(&mtime));
            dprintf(out_fd, ": %s", stamp);
        }
        dprintf(out_fd, ") ------\n");
        fsync(out_fd);
    }
    if (dry_run) {
        if (out_fd != STDOUT_FILENO) {
            // There is no title, but we should still print a dry-run message
            dprintf(out_fd, "%s: skipped on dry run\n", path);
        } else if (!title.empty()) {
            dprintf(out_fd, "\t(skipped on dry run)\n");
        }
        fsync(out_fd);
        return 0;
    }
    bool newline = false;
    while (true) {
        uint64_t start_time = Nanotime();
        pollfd fds[] = { { .fd = fd, .events = POLLIN } };
        int ret = TEMP_FAILURE_RETRY(poll(fds, arraysize(fds), 30 * 1000));
        if (ret == -1) {
            dprintf(out_fd, "*** %s: poll failed: %s\n", path, strerror(errno));
            newline = true;
            break;
        } else if (ret == 0) {
            uint64_t elapsed = Nanotime() - start_time;
            dprintf(out_fd, "*** %s: Timed out after %.3fs\n", path, (float)elapsed / NANOS_PER_SEC);
            newline = true;
            break;
        } else {
            char buffer[65536];
            ssize_t bytes_read = TEMP_FAILURE_RETRY(read(fd, buffer, sizeof(buffer)));
            if (bytes_read > 0) {
                android::base::WriteFully(out_fd, buffer, bytes_read);
                newline = (buffer[bytes_read - 1] == '\n');
            } else {
                if (bytes_read == -1) {
                    dprintf(out_fd, "*** %s: Failed to read from fd: %s", path, strerror(errno));
                    newline = true;
                }
                break;
            }
        }
    }

    if (!newline) dprintf(out_fd, "\n");
    if (!title.empty()) dprintf(out_fd, "\n");
    return 0;
}
