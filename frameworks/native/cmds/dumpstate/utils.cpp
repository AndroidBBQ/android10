/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include "dumpstate.h"

#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <math.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/inotify.h>
#include <sys/klog.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <android-base/unique_fd.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <log/log.h>
#include <private/android_filesystem_config.h>

#include "DumpstateInternal.h"

// TODO: remove once moved to namespace
using android::os::dumpstate::CommandOptions;
using android::os::dumpstate::DumpFileToFd;
using android::os::dumpstate::PropertiesHelper;

// Keep in sync with
// frameworks/base/services/core/java/com/android/server/am/ActivityManagerService.java
static const int TRACE_DUMP_TIMEOUT_MS = 10000; // 10 seconds

/* Most simple commands have 10 as timeout, so 5 is a good estimate */
static const int32_t WEIGHT_FILE = 5;

// TODO: temporary variables and functions used during C++ refactoring
static Dumpstate& ds = Dumpstate::GetInstance();
static int RunCommand(const std::string& title, const std::vector<std::string>& full_command,
                      const CommandOptions& options = CommandOptions::DEFAULT) {
    return ds.RunCommand(title, full_command, options);
}

// Reasonable value for max stats.
static const int STATS_MAX_N_RUNS = 1000;
static const long STATS_MAX_AVERAGE = 100000;

CommandOptions Dumpstate::DEFAULT_DUMPSYS = CommandOptions::WithTimeout(30).Build();

// TODO(111441001): Default DumpOptions to sensible values.
Dumpstate::Dumpstate(const std::string& version)
    : pid_(getpid()),
      options_(new Dumpstate::DumpOptions()),
      version_(version),
      now_(time(nullptr)) {
}

Dumpstate& Dumpstate::GetInstance() {
    static Dumpstate singleton_(android::base::GetProperty("dumpstate.version", VERSION_CURRENT));
    return singleton_;
}

DurationReporter::DurationReporter(const std::string& title, bool logcat_only)
    : title_(title), logcat_only_(logcat_only) {
    if (!title_.empty()) {
        started_ = Nanotime();
    }
}

DurationReporter::~DurationReporter() {
    if (!title_.empty()) {
        float elapsed = (float)(Nanotime() - started_) / NANOS_PER_SEC;
        if (elapsed < .5f) {
            return;
        }
        MYLOGD("Duration of '%s': %.2fs\n", title_.c_str(), elapsed);
        if (logcat_only_) {
            return;
        }
        // Use "Yoda grammar" to make it easier to grep|sort sections.
        printf("------ %.3fs was the duration of '%s' ------\n", elapsed, title_.c_str());
    }
}

const int32_t Progress::kDefaultMax = 5000;

Progress::Progress(const std::string& path) : Progress(Progress::kDefaultMax, 1.1, path) {
}

Progress::Progress(int32_t initial_max, int32_t progress, float growth_factor)
    : Progress(initial_max, growth_factor, "") {
    progress_ = progress;
}

Progress::Progress(int32_t initial_max, float growth_factor, const std::string& path)
    : initial_max_(initial_max),
      progress_(0),
      max_(initial_max),
      growth_factor_(growth_factor),
      n_runs_(0),
      average_max_(0),
      path_(path) {
    if (!path_.empty()) {
        Load();
    }
}

void Progress::Load() {
    MYLOGD("Loading stats from %s\n", path_.c_str());
    std::string content;
    if (!android::base::ReadFileToString(path_, &content)) {
        MYLOGI("Could not read stats from %s; using max of %d\n", path_.c_str(), max_);
        return;
    }
    if (content.empty()) {
        MYLOGE("No stats (empty file) on %s; using max of %d\n", path_.c_str(), max_);
        return;
    }
    std::vector<std::string> lines = android::base::Split(content, "\n");

    if (lines.size() < 1) {
        MYLOGE("Invalid stats on file %s: not enough lines (%d). Using max of %d\n", path_.c_str(),
               (int)lines.size(), max_);
        return;
    }
    char* ptr;
    n_runs_ = strtol(lines[0].c_str(), &ptr, 10);
    average_max_ = strtol(ptr, nullptr, 10);
    if (n_runs_ <= 0 || average_max_ <= 0 || n_runs_ > STATS_MAX_N_RUNS ||
        average_max_ > STATS_MAX_AVERAGE) {
        MYLOGE("Invalid stats line on file %s: %s\n", path_.c_str(), lines[0].c_str());
        initial_max_ = Progress::kDefaultMax;
    } else {
        initial_max_ = average_max_;
    }
    max_ = initial_max_;

    MYLOGI("Average max progress: %d in %d runs; estimated max: %d\n", average_max_, n_runs_, max_);
}

void Progress::Save() {
    int32_t total = n_runs_ * average_max_ + progress_;
    int32_t runs = n_runs_ + 1;
    int32_t average = floor(((float)total) / runs);
    MYLOGI("Saving stats (total=%d, runs=%d, average=%d) on %s\n", total, runs, average,
           path_.c_str());
    if (path_.empty()) {
        return;
    }

    std::string content = android::base::StringPrintf("%d %d\n", runs, average);
    if (!android::base::WriteStringToFile(content, path_)) {
        MYLOGE("Could not save stats on %s\n", path_.c_str());
    }
}

int32_t Progress::Get() const {
    return progress_;
}

bool Progress::Inc(int32_t delta_sec) {
    bool changed = false;
    if (delta_sec >= 0) {
        progress_ += delta_sec;
        if (progress_ > max_) {
            int32_t old_max = max_;
            max_ = floor((float)progress_ * growth_factor_);
            MYLOGD("Adjusting max progress from %d to %d\n", old_max, max_);
            changed = true;
        }
    }
    return changed;
}

int32_t Progress::GetMax() const {
    return max_;
}

int32_t Progress::GetInitialMax() const {
    return initial_max_;
}

void Progress::Dump(int fd, const std::string& prefix) const {
    const char* pr = prefix.c_str();
    dprintf(fd, "%sprogress: %d\n", pr, progress_);
    dprintf(fd, "%smax: %d\n", pr, max_);
    dprintf(fd, "%sinitial_max: %d\n", pr, initial_max_);
    dprintf(fd, "%sgrowth_factor: %0.2f\n", pr, growth_factor_);
    dprintf(fd, "%spath: %s\n", pr, path_.c_str());
    dprintf(fd, "%sn_runs: %d\n", pr, n_runs_);
    dprintf(fd, "%saverage_max: %d\n", pr, average_max_);
}

bool Dumpstate::IsZipping() const {
    return zip_writer_ != nullptr;
}

std::string Dumpstate::GetPath(const std::string& suffix) const {
    return GetPath(bugreport_internal_dir_, suffix);
}

std::string Dumpstate::GetPath(const std::string& directory, const std::string& suffix) const {
    return android::base::StringPrintf("%s/%s-%s%s", directory.c_str(), base_name_.c_str(),
                                       name_.c_str(), suffix.c_str());
}

void Dumpstate::SetProgress(std::unique_ptr<Progress> progress) {
    progress_ = std::move(progress);
}

void for_each_userid(void (*func)(int), const char *header) {
    std::string title = header == nullptr ? "for_each_userid" : android::base::StringPrintf(
                                                                    "for_each_userid(%s)", header);
    DurationReporter duration_reporter(title);
    if (PropertiesHelper::IsDryRun()) return;

    DIR *d;
    struct dirent *de;

    if (header) printf("\n------ %s ------\n", header);
    func(0);

    if (!(d = opendir("/data/system/users"))) {
        printf("Failed to open /data/system/users (%s)\n", strerror(errno));
        return;
    }

    while ((de = readdir(d))) {
        int userid;
        if (de->d_type != DT_DIR || !(userid = atoi(de->d_name))) {
            continue;
        }
        func(userid);
    }

    closedir(d);
}

static void __for_each_pid(void (*helper)(int, const char *, void *), const char *header, void *arg) {
    DIR *d;
    struct dirent *de;

    if (!(d = opendir("/proc"))) {
        printf("Failed to open /proc (%s)\n", strerror(errno));
        return;
    }

    if (header) printf("\n------ %s ------\n", header);
    while ((de = readdir(d))) {
        if (ds.IsUserConsentDenied()) {
            MYLOGE(
                "Returning early because user denied consent to share bugreport with calling app.");
            closedir(d);
            return;
        }
        int pid;
        int fd;
        char cmdpath[255];
        char cmdline[255];

        if (!(pid = atoi(de->d_name))) {
            continue;
        }

        memset(cmdline, 0, sizeof(cmdline));

        snprintf(cmdpath, sizeof(cmdpath), "/proc/%d/cmdline", pid);
        if ((fd = TEMP_FAILURE_RETRY(open(cmdpath, O_RDONLY | O_CLOEXEC))) >= 0) {
            TEMP_FAILURE_RETRY(read(fd, cmdline, sizeof(cmdline) - 2));
            close(fd);
            if (cmdline[0]) {
                helper(pid, cmdline, arg);
                continue;
            }
        }

        // if no cmdline, a kernel thread has comm
        snprintf(cmdpath, sizeof(cmdpath), "/proc/%d/comm", pid);
        if ((fd = TEMP_FAILURE_RETRY(open(cmdpath, O_RDONLY | O_CLOEXEC))) >= 0) {
            TEMP_FAILURE_RETRY(read(fd, cmdline + 1, sizeof(cmdline) - 4));
            close(fd);
            if (cmdline[1]) {
                cmdline[0] = '[';
                size_t len = strcspn(cmdline, "\f\b\r\n");
                cmdline[len] = ']';
                cmdline[len+1] = '\0';
            }
        }
        if (!cmdline[0]) {
            strcpy(cmdline, "N/A");
        }
        helper(pid, cmdline, arg);
    }

    closedir(d);
}

static void for_each_pid_helper(int pid, const char *cmdline, void *arg) {
    for_each_pid_func *func = (for_each_pid_func*) arg;
    func(pid, cmdline);
}

void for_each_pid(for_each_pid_func func, const char *header) {
    std::string title = header == nullptr ? "for_each_pid"
                                          : android::base::StringPrintf("for_each_pid(%s)", header);
    DurationReporter duration_reporter(title);
    if (PropertiesHelper::IsDryRun()) return;

    __for_each_pid(for_each_pid_helper, header, (void *) func);
}

static void for_each_tid_helper(int pid, const char *cmdline, void *arg) {
    DIR *d;
    struct dirent *de;
    char taskpath[255];
    for_each_tid_func *func = (for_each_tid_func *) arg;

    snprintf(taskpath, sizeof(taskpath), "/proc/%d/task", pid);

    if (!(d = opendir(taskpath))) {
        printf("Failed to open %s (%s)\n", taskpath, strerror(errno));
        return;
    }

    func(pid, pid, cmdline);

    while ((de = readdir(d))) {
        if (ds.IsUserConsentDenied()) {
            MYLOGE(
                "Returning early because user denied consent to share bugreport with calling app.");
            closedir(d);
            return;
        }
        int tid;
        int fd;
        char commpath[255];
        char comm[255];

        if (!(tid = atoi(de->d_name))) {
            continue;
        }

        if (tid == pid)
            continue;

        snprintf(commpath, sizeof(commpath), "/proc/%d/comm", tid);
        memset(comm, 0, sizeof(comm));
        if ((fd = TEMP_FAILURE_RETRY(open(commpath, O_RDONLY | O_CLOEXEC))) < 0) {
            strcpy(comm, "N/A");
        } else {
            char *c;
            TEMP_FAILURE_RETRY(read(fd, comm, sizeof(comm) - 2));
            close(fd);

            c = strrchr(comm, '\n');
            if (c) {
                *c = '\0';
            }
        }
        func(pid, tid, comm);
    }

    closedir(d);
}

void for_each_tid(for_each_tid_func func, const char *header) {
    std::string title = header == nullptr ? "for_each_tid"
                                          : android::base::StringPrintf("for_each_tid(%s)", header);
    DurationReporter duration_reporter(title);

    if (PropertiesHelper::IsDryRun()) return;

    __for_each_pid(for_each_tid_helper, header, (void *) func);
}

void show_wchan(int pid, int tid, const char *name) {
    if (PropertiesHelper::IsDryRun()) return;

    char path[255];
    char buffer[255];
    int fd, ret, save_errno;
    char name_buffer[255];

    memset(buffer, 0, sizeof(buffer));

    snprintf(path, sizeof(path), "/proc/%d/wchan", tid);
    if ((fd = TEMP_FAILURE_RETRY(open(path, O_RDONLY | O_CLOEXEC))) < 0) {
        printf("Failed to open '%s' (%s)\n", path, strerror(errno));
        return;
    }

    ret = TEMP_FAILURE_RETRY(read(fd, buffer, sizeof(buffer)));
    save_errno = errno;
    close(fd);

    if (ret < 0) {
        printf("Failed to read '%s' (%s)\n", path, strerror(save_errno));
        return;
    }

    snprintf(name_buffer, sizeof(name_buffer), "%*s%s",
             pid == tid ? 0 : 3, "", name);

    printf("%-7d %-32s %s\n", tid, name_buffer, buffer);

    return;
}

// print time in centiseconds
static void snprcent(char *buffer, size_t len, size_t spc,
                     unsigned long long time) {
    static long hz; // cache discovered hz

    if (hz <= 0) {
        hz = sysconf(_SC_CLK_TCK);
        if (hz <= 0) {
            hz = 1000;
        }
    }

    // convert to centiseconds
    time = (time * 100 + (hz / 2)) / hz;

    char str[16];

    snprintf(str, sizeof(str), " %llu.%02u",
             time / 100, (unsigned)(time % 100));
    size_t offset = strlen(buffer);
    snprintf(buffer + offset, (len > offset) ? len - offset : 0,
             "%*s", (spc > offset) ? (int)(spc - offset) : 0, str);
}

// print permille as a percent
static void snprdec(char *buffer, size_t len, size_t spc, unsigned permille) {
    char str[16];

    snprintf(str, sizeof(str), " %u.%u%%", permille / 10, permille % 10);
    size_t offset = strlen(buffer);
    snprintf(buffer + offset, (len > offset) ? len - offset : 0,
             "%*s", (spc > offset) ? (int)(spc - offset) : 0, str);
}

void show_showtime(int pid, const char *name) {
    if (PropertiesHelper::IsDryRun()) return;

    char path[255];
    char buffer[1023];
    int fd, ret, save_errno;

    memset(buffer, 0, sizeof(buffer));

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    if ((fd = TEMP_FAILURE_RETRY(open(path, O_RDONLY | O_CLOEXEC))) < 0) {
        printf("Failed to open '%s' (%s)\n", path, strerror(errno));
        return;
    }

    ret = TEMP_FAILURE_RETRY(read(fd, buffer, sizeof(buffer)));
    save_errno = errno;
    close(fd);

    if (ret < 0) {
        printf("Failed to read '%s' (%s)\n", path, strerror(save_errno));
        return;
    }

    // field 14 is utime
    // field 15 is stime
    // field 42 is iotime
    unsigned long long utime = 0, stime = 0, iotime = 0;
    if (sscanf(buffer,
               "%*u %*s %*s %*d %*d %*d %*d %*d %*d %*d %*d "
               "%*d %*d %llu %llu %*d %*d %*d %*d %*d %*d "
               "%*d %*d %*d %*d %*d %*d %*d %*d %*d %*d "
               "%*d %*d %*d %*d %*d %*d %*d %*d %*d %llu ",
               &utime, &stime, &iotime) != 3) {
        return;
    }

    unsigned long long total = utime + stime;
    if (!total) {
        return;
    }

    unsigned permille = (iotime * 1000 + (total / 2)) / total;
    if (permille > 1000) {
        permille = 1000;
    }

    // try to beautify and stabilize columns at <80 characters
    snprintf(buffer, sizeof(buffer), "%-6d%s", pid, name);
    if ((name[0] != '[') || utime) {
        snprcent(buffer, sizeof(buffer), 57, utime);
    }
    snprcent(buffer, sizeof(buffer), 65, stime);
    if ((name[0] != '[') || iotime) {
        snprcent(buffer, sizeof(buffer), 73, iotime);
    }
    if (iotime) {
        snprdec(buffer, sizeof(buffer), 79, permille);
    }
    puts(buffer);  // adds a trailing newline

    return;
}

void do_dmesg() {
    const char *title = "KERNEL LOG (dmesg)";
    DurationReporter duration_reporter(title);
    printf("------ %s ------\n", title);

    if (PropertiesHelper::IsDryRun()) return;

    /* Get size of kernel buffer */
    int size = klogctl(KLOG_SIZE_BUFFER, nullptr, 0);
    if (size <= 0) {
        printf("Unexpected klogctl return value: %d\n\n", size);
        return;
    }
    char *buf = (char *) malloc(size + 1);
    if (buf == nullptr) {
        printf("memory allocation failed\n\n");
        return;
    }
    int retval = klogctl(KLOG_READ_ALL, buf, size);
    if (retval < 0) {
        printf("klogctl failure\n\n");
        free(buf);
        return;
    }
    buf[retval] = '\0';
    printf("%s\n\n", buf);
    free(buf);
    return;
}

void do_showmap(int pid, const char *name) {
    char title[255];
    char arg[255];

    snprintf(title, sizeof(title), "SHOW MAP %d (%s)", pid, name);
    snprintf(arg, sizeof(arg), "%d", pid);
    RunCommand(title, {"showmap", "-q", arg}, CommandOptions::AS_ROOT);
}

int Dumpstate::DumpFile(const std::string& title, const std::string& path) {
    DurationReporter duration_reporter(title);

    int status = DumpFileToFd(STDOUT_FILENO, title, path);

    UpdateProgress(WEIGHT_FILE);

    return status;
}

int read_file_as_long(const char *path, long int *output) {
    int fd = TEMP_FAILURE_RETRY(open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC));
    if (fd < 0) {
        int err = errno;
        MYLOGE("Error opening file descriptor for %s: %s\n", path, strerror(err));
        return -1;
    }
    char buffer[50];
    ssize_t bytes_read = TEMP_FAILURE_RETRY(read(fd, buffer, sizeof(buffer)));
    if (bytes_read == -1) {
        MYLOGE("Error reading file %s: %s\n", path, strerror(errno));
        return -2;
    }
    if (bytes_read == 0) {
        MYLOGE("File %s is empty\n", path);
        return -3;
    }
    *output = atoi(buffer);
    return 0;
}

/* calls skip to gate calling dump_from_fd recursively
 * in the specified directory. dump_from_fd defaults to
 * dump_file_from_fd above when set to NULL. skip defaults
 * to false when set to NULL. dump_from_fd will always be
 * called with title NULL.
 */
int dump_files(const std::string& title, const char* dir, bool (*skip)(const char* path),
               int (*dump_from_fd)(const char* title, const char* path, int fd)) {
    DurationReporter duration_reporter(title);
    DIR *dirp;
    struct dirent *d;
    char *newpath = nullptr;
    const char *slash = "/";
    int retval = 0;

    if (!title.empty()) {
        printf("------ %s (%s) ------\n", title.c_str(), dir);
    }
    if (PropertiesHelper::IsDryRun()) return 0;

    if (dir[strlen(dir) - 1] == '/') {
        ++slash;
    }
    dirp = opendir(dir);
    if (dirp == nullptr) {
        retval = -errno;
        MYLOGE("%s: %s\n", dir, strerror(errno));
        return retval;
    }

    if (!dump_from_fd) {
        dump_from_fd = dump_file_from_fd;
    }
    for (; ((d = readdir(dirp))); free(newpath), newpath = nullptr) {
        if ((d->d_name[0] == '.')
         && (((d->d_name[1] == '.') && (d->d_name[2] == '\0'))
          || (d->d_name[1] == '\0'))) {
            continue;
        }
        asprintf(&newpath, "%s%s%s%s", dir, slash, d->d_name,
                 (d->d_type == DT_DIR) ? "/" : "");
        if (!newpath) {
            retval = -errno;
            continue;
        }
        if (skip && (*skip)(newpath)) {
            continue;
        }
        if (d->d_type == DT_DIR) {
            int ret = dump_files("", newpath, skip, dump_from_fd);
            if (ret < 0) {
                retval = ret;
            }
            continue;
        }
        android::base::unique_fd fd(TEMP_FAILURE_RETRY(open(newpath, O_RDONLY | O_NONBLOCK | O_CLOEXEC)));
        if (fd.get() < 0) {
            retval = -1;
            printf("*** %s: %s\n", newpath, strerror(errno));
            continue;
        }
        (*dump_from_fd)(nullptr, newpath, fd.get());
    }
    closedir(dirp);
    if (!title.empty()) {
        printf("\n");
    }
    return retval;
}

/* fd must have been opened with the flag O_NONBLOCK. With this flag set,
 * it's possible to avoid issues where opening the file itself can get
 * stuck.
 */
int dump_file_from_fd(const char *title, const char *path, int fd) {
    if (PropertiesHelper::IsDryRun()) return 0;

    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        printf("*** %s: failed to get flags on fd %d: %s\n", path, fd, strerror(errno));
        return -1;
    } else if (!(flags & O_NONBLOCK)) {
        printf("*** %s: fd must have O_NONBLOCK set.\n", path);
        return -1;
    }
    return DumpFileFromFdToFd(title, path, fd, STDOUT_FILENO, PropertiesHelper::IsDryRun());
}

int Dumpstate::RunCommand(const std::string& title, const std::vector<std::string>& full_command,
                          const CommandOptions& options) {
    DurationReporter duration_reporter(title);

    int status = RunCommandToFd(STDOUT_FILENO, title, full_command, options);

    /* TODO: for now we're simplifying the progress calculation by using the
     * timeout as the weight. It's a good approximation for most cases, except when calling dumpsys,
     * where its weight should be much higher proportionally to its timeout.
     * Ideally, it should use a options.EstimatedDuration() instead...*/
    UpdateProgress(options.Timeout());

    return status;
}

void Dumpstate::RunDumpsys(const std::string& title, const std::vector<std::string>& dumpsys_args,
                           const CommandOptions& options, long dumpsysTimeoutMs) {
    long timeout_ms = dumpsysTimeoutMs > 0 ? dumpsysTimeoutMs : options.TimeoutInMs();
    std::vector<std::string> dumpsys = {"/system/bin/dumpsys", "-T", std::to_string(timeout_ms)};
    dumpsys.insert(dumpsys.end(), dumpsys_args.begin(), dumpsys_args.end());
    RunCommand(title, dumpsys, options);
}

int open_socket(const char *service) {
    int s = android_get_control_socket(service);
    if (s < 0) {
        MYLOGE("android_get_control_socket(%s): %s\n", service, strerror(errno));
        return -1;
    }
    fcntl(s, F_SETFD, FD_CLOEXEC);
    if (listen(s, 4) < 0) {
        MYLOGE("listen(control socket): %s\n", strerror(errno));
        return -1;
    }

    struct sockaddr addr;
    socklen_t alen = sizeof(addr);
    int fd = accept(s, &addr, &alen);
    if (fd < 0) {
        MYLOGE("accept(control socket): %s\n", strerror(errno));
        return -1;
    }

    return fd;
}

/* redirect output to a service control socket */
bool redirect_to_socket(FILE* redirect, const char* service) {
    int fd = open_socket(service);
    if (fd == -1) {
        return false;
    }
    fflush(redirect);
    // TODO: handle dup2 failure
    TEMP_FAILURE_RETRY(dup2(fd, fileno(redirect)));
    close(fd);
    return true;
}

// TODO: should call is_valid_output_file and/or be merged into it.
void create_parent_dirs(const char *path) {
    char *chp = const_cast<char *> (path);

    /* skip initial slash */
    if (chp[0] == '/')
        chp++;

    /* create leading directories, if necessary */
    struct stat dir_stat;
    while (chp && chp[0]) {
        chp = strchr(chp, '/');
        if (chp) {
            *chp = 0;
            if (stat(path, &dir_stat) == -1 || !S_ISDIR(dir_stat.st_mode)) {
                MYLOGI("Creating directory %s\n", path);
                if (mkdir(path, 0770)) { /* drwxrwx--- */
                    MYLOGE("Unable to create directory %s: %s\n", path, strerror(errno));
                } else if (chown(path, AID_SHELL, AID_SHELL)) {
                    MYLOGE("Unable to change ownership of dir %s: %s\n", path, strerror(errno));
                }
            }
            *chp++ = '/';
        }
    }
}

bool _redirect_to_file(FILE* redirect, char* path, int truncate_flag) {
    create_parent_dirs(path);

    int fd = TEMP_FAILURE_RETRY(open(path,
                                     O_WRONLY | O_CREAT | truncate_flag | O_CLOEXEC | O_NOFOLLOW,
                                     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if (fd < 0) {
        MYLOGE("%s: %s\n", path, strerror(errno));
        return false;
    }

    TEMP_FAILURE_RETRY(dup2(fd, fileno(redirect)));
    close(fd);
    return true;
}

bool redirect_to_file(FILE* redirect, char* path) {
    return _redirect_to_file(redirect, path, O_TRUNC);
}

bool redirect_to_existing_file(FILE* redirect, char* path) {
    return _redirect_to_file(redirect, path, O_APPEND);
}

void dump_route_tables() {
    DurationReporter duration_reporter("DUMP ROUTE TABLES");
    if (PropertiesHelper::IsDryRun()) return;
    const char* const RT_TABLES_PATH = "/data/misc/net/rt_tables";
    ds.DumpFile("RT_TABLES", RT_TABLES_PATH);
    FILE* fp = fopen(RT_TABLES_PATH, "re");
    if (!fp) {
        printf("*** %s: %s\n", RT_TABLES_PATH, strerror(errno));
        return;
    }
    char table[16];
    // Each line has an integer (the table number), a space, and a string (the table name). We only
    // need the table number. It's a 32-bit unsigned number, so max 10 chars. Skip the table name.
    // Add a fixed max limit so this doesn't go awry.
    for (int i = 0; i < 64 && fscanf(fp, " %10s %*s", table) == 1; ++i) {
        RunCommand("ROUTE TABLE IPv4", {"ip", "-4", "route", "show", "table", table});
        RunCommand("ROUTE TABLE IPv6", {"ip", "-6", "route", "show", "table", table});
    }
    fclose(fp);
}

// TODO: make this function thread safe if sections are generated in parallel.
void Dumpstate::UpdateProgress(int32_t delta_sec) {
    if (progress_ == nullptr) {
        MYLOGE("UpdateProgress: progress_ not set\n");
        return;
    }

    // Always update progess so stats can be tuned...
    bool max_changed = progress_->Inc(delta_sec);

    // ...but only notifiy listeners when necessary.
    if (!options_->do_progress_updates) return;

    int progress = progress_->Get();
    int max = progress_->GetMax();

    // adjusts max on the fly
    if (max_changed && listener_ != nullptr) {
        listener_->onMaxProgressUpdated(max);
    }

    int32_t last_update_delta = progress - last_updated_progress_;
    if (last_updated_progress_ > 0 && last_update_delta < update_progress_threshold_) {
        return;
    }
    last_updated_progress_ = progress;

    if (control_socket_fd_ >= 0) {
        dprintf(control_socket_fd_, "PROGRESS:%d/%d\n", progress, max);
        fsync(control_socket_fd_);
    }

    int percent = 100 * progress / max;
    if (listener_ != nullptr) {
        if (percent % 5 == 0) {
            // We don't want to spam logcat, so only log multiples of 5.
            MYLOGD("Setting progress (%s): %d/%d (%d%%)\n", listener_name_.c_str(), progress, max,
                   percent);
        } else {
            // stderr is ignored on normal invocations, but useful when calling
            // /system/bin/dumpstate directly for debuggging.
            fprintf(stderr, "Setting progress (%s): %d/%d (%d%%)\n", listener_name_.c_str(),
                    progress, max, percent);
        }
        // TODO(b/111441001): Remove in favor of onProgress
        listener_->onProgressUpdated(progress);

        listener_->onProgress(percent);
    }
}

void Dumpstate::TakeScreenshot(const std::string& path) {
    const std::string& real_path = path.empty() ? screenshot_path_ : path;
    int status =
        RunCommand("", {"/system/bin/screencap", "-p", real_path},
                   CommandOptions::WithTimeout(10).Always().DropRoot().RedirectStderr().Build());
    if (status == 0) {
        MYLOGD("Screenshot saved on %s\n", real_path.c_str());
    } else {
        MYLOGE("Failed to take screenshot on %s\n", real_path.c_str());
    }
}

bool is_dir(const char* pathname) {
    struct stat info;
    if (stat(pathname, &info) == -1) {
        return false;
    }
    return S_ISDIR(info.st_mode);
}

time_t get_mtime(int fd, time_t default_mtime) {
    struct stat info;
    if (fstat(fd, &info) == -1) {
        return default_mtime;
    }
    return info.st_mtime;
}

void dump_emmc_ecsd(const char *ext_csd_path) {
    // List of interesting offsets
    struct hex {
        char str[2];
    };
    static const size_t EXT_CSD_REV = 192 * sizeof(hex);
    static const size_t EXT_PRE_EOL_INFO = 267 * sizeof(hex);
    static const size_t EXT_DEVICE_LIFE_TIME_EST_TYP_A = 268 * sizeof(hex);
    static const size_t EXT_DEVICE_LIFE_TIME_EST_TYP_B = 269 * sizeof(hex);

    std::string buffer;
    if (!android::base::ReadFileToString(ext_csd_path, &buffer)) {
        return;
    }

    printf("------ %s Extended CSD ------\n", ext_csd_path);

    if (buffer.length() < (EXT_CSD_REV + sizeof(hex))) {
        printf("*** %s: truncated content %zu\n\n", ext_csd_path, buffer.length());
        return;
    }

    int ext_csd_rev = 0;
    std::string sub = buffer.substr(EXT_CSD_REV, sizeof(hex));
    if (sscanf(sub.c_str(), "%2x", &ext_csd_rev) != 1) {
        printf("*** %s: EXT_CSD_REV parse error \"%s\"\n\n", ext_csd_path, sub.c_str());
        return;
    }

    static const char *ver_str[] = {
        "4.0", "4.1", "4.2", "4.3", "Obsolete", "4.41", "4.5", "5.0"
    };
    printf("rev 1.%d (MMC %s)\n", ext_csd_rev,
           (ext_csd_rev < (int)(sizeof(ver_str) / sizeof(ver_str[0]))) ? ver_str[ext_csd_rev]
                                                                       : "Unknown");
    if (ext_csd_rev < 7) {
        printf("\n");
        return;
    }

    if (buffer.length() < (EXT_PRE_EOL_INFO + sizeof(hex))) {
        printf("*** %s: truncated content %zu\n\n", ext_csd_path, buffer.length());
        return;
    }

    int ext_pre_eol_info = 0;
    sub = buffer.substr(EXT_PRE_EOL_INFO, sizeof(hex));
    if (sscanf(sub.c_str(), "%2x", &ext_pre_eol_info) != 1) {
        printf("*** %s: PRE_EOL_INFO parse error \"%s\"\n\n", ext_csd_path, sub.c_str());
        return;
    }

    static const char *eol_str[] = {
        "Undefined",
        "Normal",
        "Warning (consumed 80% of reserve)",
        "Urgent (consumed 90% of reserve)"
    };
    printf(
        "PRE_EOL_INFO %d (MMC %s)\n", ext_pre_eol_info,
        eol_str[(ext_pre_eol_info < (int)(sizeof(eol_str) / sizeof(eol_str[0]))) ? ext_pre_eol_info
                                                                                 : 0]);

    for (size_t lifetime = EXT_DEVICE_LIFE_TIME_EST_TYP_A;
            lifetime <= EXT_DEVICE_LIFE_TIME_EST_TYP_B;
            lifetime += sizeof(hex)) {
        int ext_device_life_time_est;
        static const char *est_str[] = {
            "Undefined",
            "0-10% of device lifetime used",
            "10-20% of device lifetime used",
            "20-30% of device lifetime used",
            "30-40% of device lifetime used",
            "40-50% of device lifetime used",
            "50-60% of device lifetime used",
            "60-70% of device lifetime used",
            "70-80% of device lifetime used",
            "80-90% of device lifetime used",
            "90-100% of device lifetime used",
            "Exceeded the maximum estimated device lifetime",
        };

        if (buffer.length() < (lifetime + sizeof(hex))) {
            printf("*** %s: truncated content %zu\n", ext_csd_path, buffer.length());
            break;
        }

        ext_device_life_time_est = 0;
        sub = buffer.substr(lifetime, sizeof(hex));
        if (sscanf(sub.c_str(), "%2x", &ext_device_life_time_est) != 1) {
            printf("*** %s: DEVICE_LIFE_TIME_EST_TYP_%c parse error \"%s\"\n", ext_csd_path,
                   (unsigned)((lifetime - EXT_DEVICE_LIFE_TIME_EST_TYP_A) / sizeof(hex)) + 'A',
                   sub.c_str());
            continue;
        }
        printf("DEVICE_LIFE_TIME_EST_TYP_%c %d (MMC %s)\n",
               (unsigned)((lifetime - EXT_DEVICE_LIFE_TIME_EST_TYP_A) / sizeof(hex)) + 'A',
               ext_device_life_time_est,
               est_str[(ext_device_life_time_est < (int)(sizeof(est_str) / sizeof(est_str[0])))
                           ? ext_device_life_time_est
                           : 0]);
    }

    printf("\n");
}
