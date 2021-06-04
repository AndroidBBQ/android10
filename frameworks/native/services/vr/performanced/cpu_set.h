#ifndef ANDROID_DVR_PERFORMANCED_CPU_SET_H_
#define ANDROID_DVR_PERFORMANCED_CPU_SET_H_

#include <fcntl.h>

#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <android-base/unique_fd.h>

#include <pdx/status.h>

#include "unique_file.h"

namespace android {
namespace dvr {

class CpuSet {
 public:
  // Returns the parent group for this group, if any. This pointer is owned by
  // the group hierarchy and is only valid as long as the hierarchy is valid.
  CpuSet* parent() const { return parent_; }
  std::string name() const { return name_; }
  std::string path() const { return path_; }

  bool IsRoot() const { return parent_ == nullptr; }

  std::string GetCpuList() const;

  pdx::Status<void> AttachTask(pid_t task_id) const;
  std::vector<pid_t> GetTasks() const;

 private:
  friend class CpuSetManager;

  CpuSet(CpuSet* parent, const std::string& name, base::unique_fd&& cpuset_fd);

  void AddChild(std::unique_ptr<CpuSet> child);

  base::unique_fd OpenPropertyFile(const std::string& name) const;
  UniqueFile OpenPropertyFilePointer(const std::string& name) const;

  base::unique_fd OpenFile(const std::string& name, int flags = O_RDONLY) const;
  UniqueFile OpenFilePointer(const std::string& name,
                             int flags = O_RDONLY) const;

  CpuSet* parent_;
  std::string name_;
  std::string path_;
  base::unique_fd cpuset_fd_;
  std::vector<std::unique_ptr<CpuSet>> children_;

  static void SetPrefixEnabled(bool enabled) { prefix_enabled_ = enabled; }
  static bool prefix_enabled_;

  CpuSet(const CpuSet&) = delete;
  void operator=(const CpuSet&) = delete;
};

class CpuSetManager {
 public:
  CpuSetManager() {}

  // Creats a CpuSet hierarchy by walking the directory tree starting at
  // |cpuset_root|. This argument must be the path to the root cpuset for the
  // system, which is usually /dev/cpuset.
  void Load(const std::string& cpuset_root);

  // Lookup and return a CpuSet from a cpuset path. Ownership of the pointer
  // DOES NOT pass to the caller; the pointer remains valid as long as the
  // CpuSet hierarchy is valid.
  CpuSet* Lookup(const std::string& path);

  // Returns a vector of all the cpusets found at initializaiton. Ownership of
  // the pointers to CpuSets DOES NOT pass to the caller; the pointers remain
  // valid as long as the CpuSet hierarchy is valid.
  std::vector<CpuSet*> GetCpuSets();

  // Moves all unbound tasks from the root set into the target set. This is used
  // to shield the system from interference from unbound kernel threads.
  void MoveUnboundTasks(const std::string& target_set);

  void DumpState(std::ostringstream& stream) const;

  operator bool() const { return root_set_ != nullptr; }

 private:
  // Creates a CpuSet from a path to a cpuset cgroup directory. Recursively
  // creates child groups for each directory found under |path|.
  std::unique_ptr<CpuSet> Create(const std::string& path);
  std::unique_ptr<CpuSet> Create(base::unique_fd base_fd,
                                 const std::string& name, CpuSet* parent);

  std::unique_ptr<CpuSet> root_set_;
  std::unordered_map<std::string, CpuSet*> path_map_;

  CpuSetManager(const CpuSetManager&) = delete;
  void operator=(const CpuSetManager&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_PERFORMANCED_CPU_SET_H_
