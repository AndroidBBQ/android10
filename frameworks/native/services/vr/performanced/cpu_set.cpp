#include "cpu_set.h"

#include <log/log.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <android-base/file.h>

#include "directory_reader.h"
#include "stdio_filebuf.h"
#include "task.h"
#include "unique_file.h"

using android::pdx::ErrorStatus;
using android::pdx::Status;

namespace {

constexpr int kDirectoryFlags = O_RDONLY | O_DIRECTORY | O_CLOEXEC;
constexpr pid_t kKernelThreadDaemonPid = 2;

}  // anonymous namespace

namespace android {
namespace dvr {

bool CpuSet::prefix_enabled_ = false;

void CpuSetManager::Load(const std::string& cpuset_root) {
  if (!root_set_)
    root_set_ = Create(cpuset_root);
}

std::unique_ptr<CpuSet> CpuSetManager::Create(const std::string& path) {
  base::unique_fd root_cpuset_fd(open(path.c_str(), kDirectoryFlags));
  if (root_cpuset_fd.get() < 0) {
    ALOGE("CpuSet::Create: Failed to open \"%s\": %s", path.c_str(),
          strerror(errno));
    return nullptr;
  }

  return Create(std::move(root_cpuset_fd), "/", nullptr);
}

std::unique_ptr<CpuSet> CpuSetManager::Create(base::unique_fd base_fd,
                                              const std::string& name,
                                              CpuSet* parent) {
  DirectoryReader directory(base::unique_fd(dup(base_fd)));
  if (!directory) {
    ALOGE("CpuSet::Create: Failed to opendir %s cpuset: %s", name.c_str(),
          strerror(directory.GetError()));
    return nullptr;
  }

  std::unique_ptr<CpuSet> group(
      new CpuSet(parent, name, base::unique_fd(dup(base_fd))));
  path_map_.insert(std::make_pair(group->path(), group.get()));

  while (dirent* entry = directory.Next()) {
    if (entry->d_type == DT_DIR) {
      std::string directory_name(entry->d_name);

      if (directory_name == "." || directory_name == "..")
        continue;

      base::unique_fd entry_fd(
          openat(base_fd.get(), directory_name.c_str(), kDirectoryFlags));
      if (entry_fd.get() >= 0) {
        auto child =
            Create(std::move(entry_fd), directory_name.c_str(), group.get());

        if (child)
          group->AddChild(std::move(child));
        else
          return nullptr;
      } else {
        ALOGE("CpuSet::Create: Failed to openat \"%s\": %s", entry->d_name,
              strerror(errno));
        return nullptr;
      }
    }
  }

  return group;
}

CpuSet* CpuSetManager::Lookup(const std::string& path) {
  auto search = path_map_.find(path);
  if (search != path_map_.end())
    return search->second;
  else
    return nullptr;
}

std::vector<CpuSet*> CpuSetManager::GetCpuSets() {
  std::vector<CpuSet*> sets(path_map_.size());

  for (const auto& pair : path_map_) {
    sets.push_back(pair.second);
  }

  return sets;
}

void CpuSetManager::DumpState(std::ostringstream& stream) const {
  size_t max_path = 0;
  std::vector<CpuSet*> sets;

  for (const auto& pair : path_map_) {
    max_path = std::max(max_path, pair.second->path().length());
    sets.push_back(pair.second);
  }

  std::sort(sets.begin(), sets.end(), [](const CpuSet* a, const CpuSet* b) {
    return a->path() < b->path();
  });

  stream << std::left;
  stream << std::setw(max_path) << "Path";
  stream << " ";
  stream << std::setw(6) << "CPUs";
  stream << " ";
  stream << std::setw(6) << "Tasks";
  stream << std::endl;

  stream << std::string(max_path, '_');
  stream << " ";
  stream << std::string(6, '_');
  stream << " ";
  stream << std::string(6, '_');
  stream << std::endl;

  for (const auto set : sets) {
    stream << std::left;
    stream << std::setw(max_path) << set->path();
    stream << " ";
    stream << std::right;
    stream << std::setw(6) << set->GetCpuList();
    stream << " ";
    stream << std::setw(6) << set->GetTasks().size();
    stream << std::endl;
  }
}

void CpuSetManager::MoveUnboundTasks(const std::string& target_set) {
  auto root = Lookup("/");
  if (!root) {
    ALOGE("CpuSetManager::MoveUnboundTasks: Failed to find root cpuset!");
    return;
  }

  auto target = Lookup(target_set);
  if (!target) {
    ALOGE(
        "CpuSetManager::MoveUnboundTasks: Failed to find target cpuset \"%s\"!",
        target_set.c_str());
    return;
  }

  auto cpu_list = root->GetCpuList();

  for (auto task_id : root->GetTasks()) {
    Task task(task_id);

    // Move only unbound kernel threads to the target cpuset.
    if (task.cpus_allowed_list() == cpu_list &&
        task.parent_process_id() == kKernelThreadDaemonPid) {
      ALOGD_IF(TRACE,
               "CpuSetManager::MoveUnboundTasks: Moving task_id=%d name=%s to "
               "target_set=%s tgid=%d ppid=%d.",
               task_id, task.name().c_str(), target_set.c_str(),
               task.thread_group_id(), task.parent_process_id());

      auto status = target->AttachTask(task_id);
      ALOGW_IF(!status && status.error() != EINVAL,
               "CpuSetManager::MoveUnboundTasks: Failed to attach task_id=%d "
               "to cpuset=%s: %s",
               task_id, target_set.c_str(), status.GetErrorMessage().c_str());
    } else {
      ALOGD_IF(TRACE,
               "CpuSet::MoveUnboundTasks: Skipping task_id=%d name=%s cpus=%s.",
               task_id, task.name().c_str(), task.cpus_allowed_list().c_str());
    }
  }
}

CpuSet::CpuSet(CpuSet* parent, const std::string& name,
               base::unique_fd&& cpuset_fd)
    : parent_(parent), name_(name), cpuset_fd_(std::move(cpuset_fd)) {
  if (parent_ == nullptr)
    path_ = name_;
  else if (parent_->IsRoot())
    path_ = parent_->name() + name_;
  else
    path_ = parent_->path() + "/" + name_;

  ALOGI("CpuSet::CpuSet: path=%s", path().c_str());
}

base::unique_fd CpuSet::OpenPropertyFile(const std::string& name) const {
  return OpenFile(prefix_enabled_ ? "cpuset." + name : name);
}

UniqueFile CpuSet::OpenPropertyFilePointer(const std::string& name) const {
  return OpenFilePointer(prefix_enabled_ ? "cpuset." + name : name);
}

base::unique_fd CpuSet::OpenFile(const std::string& name, int flags) const {
  const std::string relative_path = "./" + name;
  return base::unique_fd(
      openat(cpuset_fd_.get(), relative_path.c_str(), flags));
}

UniqueFile CpuSet::OpenFilePointer(const std::string& name, int flags) const {
  const std::string relative_path = "./" + name;
  base::unique_fd fd(openat(cpuset_fd_.get(), relative_path.c_str(), flags));
  if (fd.get() < 0) {
    ALOGE("CpuSet::OpenPropertyFilePointer: Failed to open %s/%s: %s",
          path_.c_str(), name.c_str(), strerror(errno));
    return nullptr;
  }

  UniqueFile fp(fdopen(fd.release(), "r"));
  if (!fp)
    ALOGE("CpuSet::OpenPropertyFilePointer: Failed to fdopen %s/%s: %s",
          path_.c_str(), name.c_str(), strerror(errno));

  return fp;
}

Status<void> CpuSet::AttachTask(pid_t task_id) const {
  auto file = OpenFile("tasks", O_RDWR);
  if (file.get() >= 0) {
    std::ostringstream stream;
    stream << task_id;
    std::string value = stream.str();

    const bool ret = base::WriteStringToFd(value, file.get());
    if (!ret)
      return ErrorStatus(errno);
    else
      return {};
  } else {
    const int error = errno;
    ALOGE("CpuSet::AttachTask: Failed to open %s/tasks: %s", path_.c_str(),
          strerror(error));
    return ErrorStatus(error);
  }
}

std::vector<pid_t> CpuSet::GetTasks() const {
  std::vector<pid_t> tasks;

  if (auto file = OpenFilePointer("tasks")) {
    stdio_filebuf<char> filebuf(file.get());
    std::istream file_stream(&filebuf);

    for (std::string line; std::getline(file_stream, line);) {
      pid_t task_id = std::strtol(line.c_str(), nullptr, 10);
      tasks.push_back(task_id);
    }
  }

  return tasks;
}

std::string CpuSet::GetCpuList() const {
  if (auto file = OpenPropertyFilePointer("cpus")) {
    stdio_filebuf<char> filebuf(file.get());
    std::istream file_stream(&filebuf);

    std::string line;
    if (std::getline(file_stream, line))
      return line;
  }

  ALOGE("CpuSet::GetCpuList: Failed to read cpu list!!!");
  return "";
}

void CpuSet::AddChild(std::unique_ptr<CpuSet> child) {
  children_.push_back(std::move(child));
}

}  // namespace dvr
}  // namespace android
