#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <utility>

#include <android-base/strings.h>
#include <android-base/unique_fd.h>
#include <dvr/performance_client_api.h>
#include <gtest/gtest.h>
#include <private/android_filesystem_config.h>

#include "stdio_filebuf.h"
#include "unique_file.h"

using android::base::Trim;
using android::dvr::UniqueFile;
using android::dvr::stdio_filebuf;

namespace {

const char kTrustedUidEnvironmentVariable[] = "GTEST_TRUSTED_UID";

const char kProcBase[] = "/proc";

std::pair<UniqueFile, int> OpenTaskFile(pid_t task_id,
                                        const std::string& name) {
  std::ostringstream stream;
  stream << kProcBase << "/" << task_id << "/" << name;

  UniqueFile file{fopen(stream.str().c_str(), "r")};
  const int error = file ? 0 : errno;
  return {std::move(file), error};
}

std::string GetTaskCpuSet(pid_t task_id) {
  int error;
  UniqueFile file;

  std::tie(file, error) = OpenTaskFile(task_id, "cpuset");
  if (!file)
    return std::string("errno:") + strerror(error);

  stdio_filebuf<char> filebuf(file.get());
  std::istream file_stream(&filebuf);

  std::string line;
  std::getline(file_stream, line);
  return Trim(line);
}

}  // anonymous namespace

TEST(PerformanceTest, SetCpuPartition) {
  int error;

  // Test setting the the partition for the current task.
  error = dvrSetCpuPartition(0, "/application/background");
  EXPECT_EQ(0, error);

  error = dvrSetCpuPartition(0, "/application/performance");
  EXPECT_EQ(0, error);

  // Test setting the partition for one of our tasks.
  bool done = false;
  pid_t task_id = 0;
  std::mutex mutex;
  std::condition_variable done_condition, id_condition;

  std::thread thread([&] {
    std::unique_lock<std::mutex> lock(mutex);

    task_id = gettid();
    id_condition.notify_one();

    done_condition.wait(lock, [&done] { return done; });
  });

  {
    std::unique_lock<std::mutex> lock(mutex);
    id_condition.wait(lock, [&task_id] { return task_id != 0; });
  }
  EXPECT_NE(0, task_id);

  error = dvrSetCpuPartition(task_id, "/application");
  EXPECT_EQ(0, error);

  {
    std::lock_guard<std::mutex> lock(mutex);
    done = true;
    done_condition.notify_one();
  }
  thread.join();

  // Test setting the partition for a task that doesn't belong to us.
  error = dvrSetCpuPartition(1, "/application");
  EXPECT_EQ(-EINVAL, error);

  // Test setting the partition to one that doesn't exist.
  error = dvrSetCpuPartition(0, "/foobar");
  EXPECT_EQ(-ENOENT, error);

  // Set the test back to the root partition.
  error = dvrSetCpuPartition(0, "/");
  EXPECT_EQ(0, error);
}

TEST(PerformanceTest, SetSchedulerClass) {
  int error;

  // TODO(eieio): Test all supported scheduler classes and priority levels.

  error = dvrSetSchedulerClass(0, "background");
  EXPECT_EQ(0, error);
  EXPECT_EQ(SCHED_BATCH, sched_getscheduler(0));

  error = dvrSetSchedulerClass(0, "audio:low");
  EXPECT_EQ(0, error);
  EXPECT_EQ(SCHED_FIFO | SCHED_RESET_ON_FORK, sched_getscheduler(0));

  error = dvrSetSchedulerClass(0, "normal");
  EXPECT_EQ(0, error);
  EXPECT_EQ(SCHED_NORMAL, sched_getscheduler(0));

  error = dvrSetSchedulerClass(0, "foobar");
  EXPECT_EQ(-EINVAL, error);
}

TEST(PerformanceTest, SetSchedulerPolicy) {
  int error;

  error = dvrSetSchedulerPolicy(0, "background");
  EXPECT_EQ(0, error);
  EXPECT_EQ(SCHED_BATCH, sched_getscheduler(0));

  error = dvrSetSchedulerPolicy(0, "audio:low");
  EXPECT_EQ(0, error);
  EXPECT_EQ(SCHED_FIFO | SCHED_RESET_ON_FORK, sched_getscheduler(0));

  error = dvrSetSchedulerPolicy(0, "normal");
  EXPECT_EQ(0, error);
  EXPECT_EQ(SCHED_NORMAL, sched_getscheduler(0));

  error = dvrSetSchedulerPolicy(0, "foobar");
  EXPECT_EQ(-EINVAL, error);

  // Set the test back to the root partition.
  error = dvrSetCpuPartition(0, "/");
  EXPECT_EQ(0, error);

  const std::string original_cpuset = GetTaskCpuSet(gettid());
  EXPECT_EQ("/", original_cpuset);

  error = dvrSetSchedulerPolicy(0, "vr:system:arp");
  EXPECT_EQ(0, error);
  EXPECT_EQ(SCHED_FIFO | SCHED_RESET_ON_FORK, sched_getscheduler(0));

  const std::string new_cpuset = GetTaskCpuSet(gettid());
  EXPECT_NE(original_cpuset, new_cpuset);

  // The cpuset for the thread group is now new_cpuset. Scheduler profiles that
  // do not specify a cpuset should not change the cpuset of a thread, except to
  // restore it to the thread group cpuset.
  std::string thread_original_cpuset;
  std::string thread_new_cpuset;
  std::string thread_final_cpuset;

  std::thread thread{
      [&thread_original_cpuset, &thread_new_cpuset, &thread_final_cpuset]() {
        thread_original_cpuset = GetTaskCpuSet(gettid());

        int error = dvrSetSchedulerPolicy(0, "vr:app:render");
        EXPECT_EQ(0, error);

        thread_new_cpuset = GetTaskCpuSet(gettid());

        error = dvrSetSchedulerPolicy(0, "normal");
        EXPECT_EQ(0, error);

        thread_final_cpuset = GetTaskCpuSet(gettid());
      }};
  thread.join();

  EXPECT_EQ(new_cpuset, thread_original_cpuset);
  EXPECT_NE(new_cpuset, thread_new_cpuset);
  EXPECT_EQ(new_cpuset, thread_final_cpuset);

  error = dvrSetCpuPartition(0, original_cpuset.c_str());
  EXPECT_EQ(0, error);
}

TEST(PerformanceTest, SchedulerClassResetOnFork) {
  int error;

  error = dvrSetSchedulerClass(0, "graphics:high");
  EXPECT_EQ(0, error);
  EXPECT_EQ(SCHED_FIFO | SCHED_RESET_ON_FORK, sched_getscheduler(0));

  int scheduler = -1;
  std::thread thread([&]() { scheduler = sched_getscheduler(0); });
  thread.join();

  EXPECT_EQ(SCHED_NORMAL, scheduler);

  // Return to SCHED_NORMAL.
  error = dvrSetSchedulerClass(0, "normal");
  EXPECT_EQ(0, error);
  EXPECT_EQ(SCHED_NORMAL, sched_getscheduler(0));
}

TEST(PerformanceTest, GetCpuPartition) {
  int error;
  char partition[PATH_MAX + 1];

  error = dvrSetCpuPartition(0, "/");
  ASSERT_EQ(0, error);

  error = dvrGetCpuPartition(0, partition, sizeof(partition));
  EXPECT_EQ(0, error);
  EXPECT_EQ("/", std::string(partition));

  error = dvrSetCpuPartition(0, "/application");
  EXPECT_EQ(0, error);

  error = dvrGetCpuPartition(0, partition, sizeof(partition));
  EXPECT_EQ(0, error);
  EXPECT_EQ("/application", std::string(partition));

  // Test passing a buffer that is too short.
  error = dvrGetCpuPartition(0, partition, 5);
  EXPECT_EQ(-ENOBUFS, error);

  // Test getting the partition for a task that doesn't belong to us.
  error = dvrGetCpuPartition(1, partition, sizeof(partition));
  EXPECT_EQ(-EINVAL, error);

  // Test passing a nullptr value for partition buffer.
  error = dvrGetCpuPartition(0, nullptr, sizeof(partition));
  EXPECT_EQ(-EINVAL, error);
}

TEST(PerformanceTest, Permissions) {
  int error;

  const int original_uid = getuid();
  const int original_gid = getgid();
  int trusted_uid = -1;

  // See if the environment variable GTEST_TRUSTED_UID is set. If it is enable
  // testing the ActivityManager trusted uid permission checks using that uid.
  const char* trusted_uid_env = std::getenv(kTrustedUidEnvironmentVariable);
  if (trusted_uid_env)
    trusted_uid = std::atoi(trusted_uid_env);

  ASSERT_EQ(AID_ROOT, original_uid)
      << "This test must run as root to function correctly!";

  // Test unprivileged policies on a task that does not belong to this process.
  // Use the init process (task_id=1) as the target.
  error = dvrSetSchedulerPolicy(1, "batch");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(1, "background");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(1, "foreground");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(1, "normal");
  EXPECT_EQ(-EINVAL, error);

  // Switch the uid/gid to an id that should not have permission to access any
  // privileged actions.
  ASSERT_EQ(0, setresgid(AID_NOBODY, AID_NOBODY, -1))
      << "Failed to set gid: " << strerror(errno);
  ASSERT_EQ(0, setresuid(AID_NOBODY, AID_NOBODY, -1))
      << "Failed to set uid: " << strerror(errno);

  // Unprivileged policies.
  error = dvrSetSchedulerPolicy(0, "batch");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "background");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "foreground");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "normal");
  EXPECT_EQ(0, error);

  // Privileged policies.
  error = dvrSetSchedulerPolicy(0, "audio:low");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "audio:high");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "graphics");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "graphics:low");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "graphics:high");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "sensors");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "sensors:low");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "sensors:high");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "vr:system:arp");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "vr:app:render");
  EXPECT_EQ(-EINVAL, error);

  // uid=AID_SYSTEM / gid=AID_NOBODY
  ASSERT_EQ(0, setresuid(original_uid, original_uid, -1))
      << "Failed to restore uid: " << strerror(errno);
  ASSERT_EQ(0, setresuid(AID_SYSTEM, AID_SYSTEM, -1))
      << "Failed to set uid: " << strerror(errno);

  // Unprivileged policies.
  error = dvrSetSchedulerPolicy(0, "batch");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "background");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "foreground");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "normal");
  EXPECT_EQ(0, error);

  // Privileged policies.
  error = dvrSetSchedulerPolicy(0, "audio:low");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "audio:high");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "graphics");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "graphics:low");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "graphics:high");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "sensors");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "sensors:low");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "sensors:high");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "vr:system:arp");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "vr:app:render");
  EXPECT_EQ(0, error);

  // uid=AID_NOBODY / gid=AID_SYSTEM
  ASSERT_EQ(0, setresuid(original_uid, original_uid, -1))
      << "Failed to restore uid: " << strerror(errno);
  ASSERT_EQ(0, setresgid(original_gid, original_gid, -1))
      << "Failed to restore gid: " << strerror(errno);
  ASSERT_EQ(0, setresgid(AID_SYSTEM, AID_SYSTEM, -1))
      << "Failed to set gid: " << strerror(errno);
  ASSERT_EQ(0, setresuid(AID_SYSTEM, AID_NOBODY, -1))
      << "Failed to set uid: " << strerror(errno);

  // Unprivileged policies.
  error = dvrSetSchedulerPolicy(0, "batch");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "background");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "foreground");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "normal");
  EXPECT_EQ(0, error);

  // Privileged policies.
  error = dvrSetSchedulerPolicy(0, "audio:low");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "audio:high");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "graphics");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "graphics:low");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "graphics:high");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "sensors");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "sensors:low");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "sensors:high");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "vr:system:arp");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "vr:app:render");
  EXPECT_EQ(0, error);

  // uid=AID_GRAPHICS / gid=AID_NOBODY
  ASSERT_EQ(0, setresuid(original_uid, original_uid, -1))
      << "Failed to restore uid: " << strerror(errno);
  ASSERT_EQ(0, setresgid(original_gid, original_gid, -1))
      << "Failed to restore gid: " << strerror(errno);
  ASSERT_EQ(0, setresgid(AID_NOBODY, AID_NOBODY, -1))
      << "Failed to set gid: " << strerror(errno);
  ASSERT_EQ(0, setresuid(AID_GRAPHICS, AID_GRAPHICS, -1))
      << "Failed to set uid: " << strerror(errno);

  // Unprivileged policies.
  error = dvrSetSchedulerPolicy(0, "batch");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "background");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "foreground");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "normal");
  EXPECT_EQ(0, error);

  // Privileged policies.
  error = dvrSetSchedulerPolicy(0, "audio:low");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "audio:high");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "graphics");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "graphics:low");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "graphics:high");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "sensors");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "sensors:low");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "sensors:high");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "vr:system:arp");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "vr:app:render");
  EXPECT_EQ(-EINVAL, error);

  // uid=AID_NOBODY / gid=AID_GRAPHICS
  ASSERT_EQ(0, setresuid(original_uid, original_uid, -1))
      << "Failed to restore uid: " << strerror(errno);
  ASSERT_EQ(0, setresgid(original_gid, original_gid, -1))
      << "Failed to restore gid: " << strerror(errno);
  ASSERT_EQ(0, setresgid(AID_GRAPHICS, AID_GRAPHICS, -1))
      << "Failed to set gid: " << strerror(errno);
  ASSERT_EQ(0, setresuid(AID_NOBODY, AID_NOBODY, -1))
      << "Failed to set uid: " << strerror(errno);

  // Unprivileged policies.
  error = dvrSetSchedulerPolicy(0, "batch");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "background");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "foreground");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "normal");
  EXPECT_EQ(0, error);

  // Privileged policies.
  error = dvrSetSchedulerPolicy(0, "audio:low");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "audio:high");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "graphics");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "graphics:low");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "graphics:high");
  EXPECT_EQ(0, error);
  error = dvrSetSchedulerPolicy(0, "sensors");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "sensors:low");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "sensors:high");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "vr:system:arp");
  EXPECT_EQ(-EINVAL, error);
  error = dvrSetSchedulerPolicy(0, "vr:app:render");
  EXPECT_EQ(-EINVAL, error);

  if (trusted_uid != -1) {
    // uid=<trusted uid> / gid=AID_NOBODY
    ASSERT_EQ(0, setresuid(original_uid, original_uid, -1))
        << "Failed to restore uid: " << strerror(errno);
    ASSERT_EQ(0, setresgid(original_gid, original_gid, -1))
        << "Failed to restore gid: " << strerror(errno);
    ASSERT_EQ(0, setresgid(AID_NOBODY, AID_NOBODY, -1))
        << "Failed to set gid: " << strerror(errno);
    ASSERT_EQ(0, setresuid(trusted_uid, trusted_uid, -1))
        << "Failed to set uid: " << strerror(errno);

    // Unprivileged policies.
    error = dvrSetSchedulerPolicy(0, "batch");
    EXPECT_EQ(0, error);
    error = dvrSetSchedulerPolicy(0, "background");
    EXPECT_EQ(0, error);
    error = dvrSetSchedulerPolicy(0, "foreground");
    EXPECT_EQ(0, error);
    error = dvrSetSchedulerPolicy(0, "normal");
    EXPECT_EQ(0, error);

    // Privileged policies.
    error = dvrSetSchedulerPolicy(0, "audio:low");
    EXPECT_EQ(-EINVAL, error);
    error = dvrSetSchedulerPolicy(0, "audio:high");
    EXPECT_EQ(-EINVAL, error);
    error = dvrSetSchedulerPolicy(0, "graphics");
    EXPECT_EQ(-EINVAL, error);
    error = dvrSetSchedulerPolicy(0, "graphics:low");
    EXPECT_EQ(-EINVAL, error);
    error = dvrSetSchedulerPolicy(0, "graphics:high");
    EXPECT_EQ(-EINVAL, error);
    error = dvrSetSchedulerPolicy(0, "sensors");
    EXPECT_EQ(-EINVAL, error);
    error = dvrSetSchedulerPolicy(0, "sensors:low");
    EXPECT_EQ(-EINVAL, error);
    error = dvrSetSchedulerPolicy(0, "sensors:high");
    EXPECT_EQ(-EINVAL, error);
    error = dvrSetSchedulerPolicy(0, "vr:system:arp");
    EXPECT_EQ(0, error);
    error = dvrSetSchedulerPolicy(0, "vr:app:render");
    EXPECT_EQ(0, error);
  }

  // Restore original effective uid/gid.
  ASSERT_EQ(0, setresgid(original_gid, original_gid, -1))
      << "Failed to restore gid: " << strerror(errno);
  ASSERT_EQ(0, setresuid(original_uid, original_uid, -1))
      << "Failed to restore uid: " << strerror(errno);
}
