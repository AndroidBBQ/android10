#ifndef ANDROID_DVR_PERFORMANCED_TASK_H_
#define ANDROID_DVR_PERFORMANCED_TASK_H_

#include <sys/types.h>

#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <android-base/unique_fd.h>

#include "unique_file.h"

namespace android {
namespace dvr {

// Task provides access to task-related information from the procfs
// pseudo-filesystem.
class Task {
 public:
  explicit Task(pid_t task_id);

  bool IsValid() const { return task_fd_.get() >= 0; }
  explicit operator bool() const { return IsValid(); }

  pid_t task_id() const { return task_id_; }
  std::string name() const { return name_; }
  pid_t thread_group_id() const { return thread_group_id_; }
  pid_t parent_process_id() const { return parent_process_id_; }
  size_t thread_count() const { return thread_count_; }
  uint32_t cpus_allowed_mask() const { return cpus_allowed_mask_; }
  const std::string& cpus_allowed_list() const { return cpus_allowed_list_; }
  const std::array<int, 4>& user_id() const { return user_id_; }
  const std::array<int, 4>& group_id() const { return group_id_; }

  // Indices into user and group id arrays.
  enum {
    kUidReal = 0,
    kUidEffective,
    kUidSavedSet,
    kUidFilesystem,
  };

  std::string GetCpuSetPath() const;

 private:
  pid_t task_id_;
  base::unique_fd task_fd_;

  // Fields read from /proc/<task_id_>/status.
  std::string name_;
  pid_t thread_group_id_;
  pid_t parent_process_id_;
  std::array<int, 4> user_id_;
  std::array<int, 4> group_id_;
  size_t thread_count_;
  uint32_t cpus_allowed_mask_;
  std::string cpus_allowed_list_;

  // Opens the file /proc/<task_id_>/|name| and returns the open file
  // descriptor.
  base::unique_fd OpenTaskFile(const std::string& name) const;

  // Similar to OpenTaskFile() but returns a file pointer.
  UniqueFile OpenTaskFilePointer(const std::string& name) const;

  // Reads the field named |field| from /proc/<task_id_>/status.
  std::string GetStatusField(const std::string& field) const;

  // Reads a subset of the fields in /proc/<task_id_>/status.
  void ReadStatusFields();

  Task(const Task&) = delete;
  void operator=(const Task&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_PERFORMANCED_TASK_H_
