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

#include "DumpstateUtil.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>

#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <android-base/unique_fd.h>
#include <log/log.h>

#include "DumpstateInternal.h"

namespace android {
namespace os {
namespace dumpstate {

namespace {

static constexpr const char* kSuPath = "/system/xbin/su";

static bool waitpid_with_timeout(pid_t pid, int timeout_ms, int* status) {
    sigset_t child_mask, old_mask;
    sigemptyset(&child_mask);
    sigaddset(&child_mask, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &child_mask, &old_mask) == -1) {
        printf("*** sigprocmask failed: %s\n", strerror(errno));
        return false;
    }

    timespec ts;
    ts.tv_sec = MSEC_TO_SEC(timeout_ms);
    ts.tv_nsec = (timeout_ms % 1000) * 1000000;
    int ret = TEMP_FAILURE_RETRY(sigtimedwait(&child_mask, nullptr, &ts));
    int saved_errno = errno;

    // Set the signals back the way they were.
    if (sigprocmask(SIG_SETMASK, &old_mask, nullptr) == -1) {
        printf("*** sigprocmask failed: %s\n", strerror(errno));
        if (ret == 0) {
            return false;
        }
    }
    if (ret == -1) {
        errno = saved_errno;
        if (errno == EAGAIN) {
            errno = ETIMEDOUT;
        } else {
            printf("*** sigtimedwait failed: %s\n", strerror(errno));
        }
        return false;
    }

    pid_t child_pid = waitpid(pid, status, WNOHANG);
    if (child_pid != pid) {
        if (child_pid != -1) {
            printf("*** Waiting for pid %d, got pid %d instead\n", pid, child_pid);
        } else {
            printf("*** waitpid failed: %s\n", strerror(errno));
        }
        return false;
    }
    return true;
}
}  // unnamed namespace

CommandOptions CommandOptions::DEFAULT = CommandOptions::WithTimeout(10).Build();
CommandOptions CommandOptions::AS_ROOT = CommandOptions::WithTimeout(10).AsRoot().Build();

CommandOptions::CommandOptionsBuilder::CommandOptionsBuilder(int64_t timeout_ms) : values(timeout_ms) {
}

CommandOptions::CommandOptionsBuilder& CommandOptions::CommandOptionsBuilder::Always() {
    values.always_ = true;
    return *this;
}

CommandOptions::CommandOptionsBuilder& CommandOptions::CommandOptionsBuilder::AsRoot() {
    if (!PropertiesHelper::IsUnroot()) {
        values.account_mode_ = SU_ROOT;
    }
    return *this;
}

CommandOptions::CommandOptionsBuilder& CommandOptions::CommandOptionsBuilder::AsRootIfAvailable() {
    if (!PropertiesHelper::IsUserBuild()) {
        return AsRoot();
    }
    return *this;
}

CommandOptions::CommandOptionsBuilder& CommandOptions::CommandOptionsBuilder::DropRoot() {
    values.account_mode_ = DROP_ROOT;
    return *this;
}

CommandOptions::CommandOptionsBuilder& CommandOptions::CommandOptionsBuilder::RedirectStderr() {
    values.output_mode_ = REDIRECT_TO_STDERR;
    return *this;
}

CommandOptions::CommandOptionsBuilder& CommandOptions::CommandOptionsBuilder::Log(
    const std::string& message) {
    values.logging_message_ = message;
    return *this;
}

CommandOptions CommandOptions::CommandOptionsBuilder::Build() {
    return CommandOptions(values);
}

CommandOptions::CommandOptionsValues::CommandOptionsValues(int64_t timeout_ms)
    : timeout_ms_(timeout_ms),
      always_(false),
      account_mode_(DONT_DROP_ROOT),
      output_mode_(NORMAL_OUTPUT),
      logging_message_("") {
}

CommandOptions::CommandOptions(const CommandOptionsValues& values) : values(values) {
}

int64_t CommandOptions::Timeout() const {
    return MSEC_TO_SEC(values.timeout_ms_);
}

int64_t CommandOptions::TimeoutInMs() const {
    return values.timeout_ms_;
}

bool CommandOptions::Always() const {
    return values.always_;
}

PrivilegeMode CommandOptions::PrivilegeMode() const {
    return values.account_mode_;
}

OutputMode CommandOptions::OutputMode() const {
    return values.output_mode_;
}

std::string CommandOptions::LoggingMessage() const {
    return values.logging_message_;
}

CommandOptions::CommandOptionsBuilder CommandOptions::WithTimeout(int64_t timeout_sec) {
    return CommandOptions::CommandOptionsBuilder(SEC_TO_MSEC(timeout_sec));
}

CommandOptions::CommandOptionsBuilder CommandOptions::WithTimeoutInMs(int64_t timeout_ms) {
    return CommandOptions::CommandOptionsBuilder(timeout_ms);
}

std::string PropertiesHelper::build_type_ = "";
int PropertiesHelper::dry_run_ = -1;
int PropertiesHelper::unroot_ = -1;

bool PropertiesHelper::IsUserBuild() {
    if (build_type_.empty()) {
        build_type_ = android::base::GetProperty("ro.build.type", "user");
    }
    return "user" == build_type_;
}

bool PropertiesHelper::IsDryRun() {
    if (dry_run_ == -1) {
        dry_run_ = android::base::GetBoolProperty("dumpstate.dry_run", false) ? 1 : 0;
    }
    return dry_run_ == 1;
}

bool PropertiesHelper::IsUnroot() {
    if (unroot_ == -1) {
        unroot_ = android::base::GetBoolProperty("dumpstate.unroot", false) ? 1 : 0;
    }
    return unroot_ == 1;
}

int DumpFileToFd(int out_fd, const std::string& title, const std::string& path) {
    android::base::unique_fd fd(TEMP_FAILURE_RETRY(open(path.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC)));
    if (fd.get() < 0) {
        int err = errno;
        if (title.empty()) {
            dprintf(out_fd, "*** Error dumping %s: %s\n", path.c_str(), strerror(err));
        } else {
            dprintf(out_fd, "*** Error dumping %s (%s): %s\n", path.c_str(), title.c_str(),
                    strerror(err));
        }
        fsync(out_fd);
        return -1;
    }
    return DumpFileFromFdToFd(title, path, fd.get(), out_fd, PropertiesHelper::IsDryRun());
}

int RunCommandToFd(int fd, const std::string& title, const std::vector<std::string>& full_command,
                   const CommandOptions& options) {
    if (full_command.empty()) {
        MYLOGE("No arguments on RunCommandToFd(%s)\n", title.c_str());
        return -1;
    }

    int size = full_command.size() + 1;  // null terminated
    int starting_index = 0;
    if (options.PrivilegeMode() == SU_ROOT) {
        starting_index = 2;  // "su" "root"
        size += starting_index;
    }

    std::vector<const char*> args;
    args.resize(size);

    std::string command_string;
    if (options.PrivilegeMode() == SU_ROOT) {
        args[0] = kSuPath;
        command_string += kSuPath;
        args[1] = "root";
        command_string += " root ";
    }
    for (size_t i = 0; i < full_command.size(); i++) {
        args[i + starting_index] = full_command[i].data();
        command_string += args[i + starting_index];
        if (i != full_command.size() - 1) {
            command_string += " ";
        }
    }
    args[size - 1] = nullptr;

    const char* command = command_string.c_str();

    if (options.PrivilegeMode() == SU_ROOT && PropertiesHelper::IsUserBuild()) {
        dprintf(fd, "Skipping '%s' on user build.\n", command);
        return 0;
    }

    if (!title.empty()) {
        dprintf(fd, "------ %s (%s) ------\n", title.c_str(), command);
        fsync(fd);
    }

    const std::string& logging_message = options.LoggingMessage();
    if (!logging_message.empty()) {
        MYLOGI(logging_message.c_str(), command_string.c_str());
    }

    bool silent = (options.OutputMode() == REDIRECT_TO_STDERR);
    bool redirecting_to_fd = STDOUT_FILENO != fd;

    if (PropertiesHelper::IsDryRun() && !options.Always()) {
        if (!title.empty()) {
            dprintf(fd, "\t(skipped on dry run)\n");
        } else if (redirecting_to_fd) {
            // There is no title, but we should still print a dry-run message
            dprintf(fd, "%s: skipped on dry run\n", command_string.c_str());
        }
        fsync(fd);
        return 0;
    }

    const char* path = args[0];

    uint64_t start = Nanotime();
    pid_t pid = fork();

    /* handle error case */
    if (pid < 0) {
        if (!silent) dprintf(fd, "*** fork: %s\n", strerror(errno));
        MYLOGE("*** fork: %s\n", strerror(errno));
        return pid;
    }

    /* handle child case */
    if (pid == 0) {
        if (options.PrivilegeMode() == DROP_ROOT && !DropRootUser()) {
            if (!silent) {
                dprintf(fd, "*** failed to drop root before running %s: %s\n", command,
                        strerror(errno));
            }
            MYLOGE("*** could not drop root before running %s: %s\n", command, strerror(errno));
            return -1;
        }

        if (silent) {
            // Redirects stdout to stderr
            TEMP_FAILURE_RETRY(dup2(STDERR_FILENO, STDOUT_FILENO));
        } else if (redirecting_to_fd) {
            // Redirect stdout to fd
            TEMP_FAILURE_RETRY(dup2(fd, STDOUT_FILENO));
            close(fd);
        }

        /* make sure the child dies when dumpstate dies */
        prctl(PR_SET_PDEATHSIG, SIGKILL);

        /* just ignore SIGPIPE, will go down with parent's */
        struct sigaction sigact;
        memset(&sigact, 0, sizeof(sigact));
        sigact.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &sigact, nullptr);

        execvp(path, (char**)args.data());
        // execvp's result will be handled after waitpid_with_timeout() below, but
        // if it failed, it's safer to exit dumpstate.
        MYLOGD("execvp on command '%s' failed (error: %s)\n", command, strerror(errno));
        // Must call _exit (instead of exit), otherwise it will corrupt the zip
        // file.
        _exit(EXIT_FAILURE);
    }

    /* handle parent case */
    int status;
    bool ret = waitpid_with_timeout(pid, options.TimeoutInMs(), &status);
    fsync(fd);

    uint64_t elapsed = Nanotime() - start;
    if (!ret) {
        if (errno == ETIMEDOUT) {
            if (!silent)
                dprintf(fd, "*** command '%s' timed out after %.3fs (killing pid %d)\n", command,
                        static_cast<float>(elapsed) / NANOS_PER_SEC, pid);
            MYLOGE("*** command '%s' timed out after %.3fs (killing pid %d)\n", command,
                   static_cast<float>(elapsed) / NANOS_PER_SEC, pid);
        } else {
            if (!silent)
                dprintf(fd, "*** command '%s': Error after %.4fs (killing pid %d)\n", command,
                        static_cast<float>(elapsed) / NANOS_PER_SEC, pid);
            MYLOGE("command '%s': Error after %.4fs (killing pid %d)\n", command,
                   static_cast<float>(elapsed) / NANOS_PER_SEC, pid);
        }
        kill(pid, SIGTERM);
        if (!waitpid_with_timeout(pid, 5000, nullptr)) {
            kill(pid, SIGKILL);
            if (!waitpid_with_timeout(pid, 5000, nullptr)) {
                if (!silent)
                    dprintf(fd, "could not kill command '%s' (pid %d) even with SIGKILL.\n",
                            command, pid);
                MYLOGE("could not kill command '%s' (pid %d) even with SIGKILL.\n", command, pid);
            }
        }
        return -1;
    }

    if (WIFSIGNALED(status)) {
        if (!silent)
            dprintf(fd, "*** command '%s' failed: killed by signal %d\n", command, WTERMSIG(status));
        MYLOGE("*** command '%s' failed: killed by signal %d\n", command, WTERMSIG(status));
    } else if (WIFEXITED(status) && WEXITSTATUS(status) > 0) {
        status = WEXITSTATUS(status);
        if (!silent) dprintf(fd, "*** command '%s' failed: exit code %d\n", command, status);
        MYLOGE("*** command '%s' failed: exit code %d\n", command, status);
    }

    return status;
}

int GetPidByName(const std::string& ps_name) {
    DIR* proc_dir;
    struct dirent* ps;
    unsigned int pid;
    std::string cmdline;

    if (!(proc_dir = opendir("/proc"))) {
        MYLOGE("Can't open /proc\n");
        return -1;
    }

    while ((ps = readdir(proc_dir))) {
        if (!(pid = atoi(ps->d_name))) {
            continue;
        }
        android::base::ReadFileToString("/proc/" + std::string(ps->d_name) + "/cmdline", &cmdline);
        if (cmdline.find(ps_name) == std::string::npos) {
            continue;
        } else {
            closedir(proc_dir);
            return pid;
        }
    }
    MYLOGE("can't find the pid\n");
    closedir(proc_dir);
    return -1;
}

}  // namespace dumpstate
}  // namespace os
}  // namespace android
