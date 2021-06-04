#include "task.h"

#include <errno.h>
#include <fcntl.h>
#include <log/log.h>
#include <stdio.h>

#include <cctype>
#include <cstdlib>
#include <memory>
#include <sstream>

#include <android-base/strings.h>
#include <android-base/unique_fd.h>

#include "stdio_filebuf.h"

namespace {

const char kProcBase[] = "/proc";

android::base::unique_fd OpenTaskDirectory(pid_t task_id) {
  std::ostringstream stream;
  stream << kProcBase << "/" << task_id;

  return android::base::unique_fd(
      open(stream.str().c_str(), O_RDONLY | O_DIRECTORY));
}

void ParseUidStatusField(const std::string& value, std::array<int, 4>& ids) {
  const char* start = value.c_str();

  ids[0] = std::strtol(start, const_cast<char**>(&start), 10);
  ids[1] = std::strtol(start, const_cast<char**>(&start), 10);
  ids[2] = std::strtol(start, const_cast<char**>(&start), 10);
  ids[3] = std::strtol(start, const_cast<char**>(&start), 10);
}

}  // anonymous namespace

namespace android {
namespace dvr {

Task::Task(pid_t task_id)
    : task_id_(task_id),
      thread_group_id_(-1),
      parent_process_id_(-1),
      thread_count_(0),
      cpus_allowed_mask_(0) {
  task_fd_ = OpenTaskDirectory(task_id_);
  const int error = errno;
  ALOGE_IF(task_fd_.get() < 0 && error != EACCES,
           "Task::Task: Failed to open task directory for task_id=%d: %s",
           task_id, strerror(error));

  if (IsValid()) {
    ReadStatusFields();
    ALOGD_IF(TRACE,
             "Task::Task: task_id=%d name=%s tgid=%d ppid=%d cpu_mask=%x",
             task_id_, name_.c_str(), thread_group_id_, parent_process_id_,
             cpus_allowed_mask_);
  }
}

base::unique_fd Task::OpenTaskFile(const std::string& name) const {
  const std::string relative_path = "./" + name;
  return base::unique_fd(
      openat(task_fd_.get(), relative_path.c_str(), O_RDONLY));
}

UniqueFile Task::OpenTaskFilePointer(const std::string& name) const {
  const std::string relative_path = "./" + name;
  base::unique_fd fd(openat(task_fd_.get(), relative_path.c_str(), O_RDONLY));
  if (fd.get() < 0) {
    ALOGE("Task::OpenTaskFilePointer: Failed to open /proc/%d/%s: %s", task_id_,
          name.c_str(), strerror(errno));
    return nullptr;
  }

  UniqueFile fp(fdopen(fd.release(), "r"));
  if (!fp)
    ALOGE("Task::OpenTaskFilePointer: Failed to fdopen /proc/%d/%s: %s",
          task_id_, name.c_str(), strerror(errno));

  return fp;
}

std::string Task::GetStatusField(const std::string& field) const {
  if (auto file = OpenTaskFilePointer("status")) {
    stdio_filebuf<char> filebuf(file.get());
    std::istream file_stream(&filebuf);

    for (std::string line; std::getline(file_stream, line);) {
      auto offset = line.find(field);

      ALOGD_IF(TRACE,
               "Task::GetStatusField: field=\"%s\" line=\"%s\" offset=%zd",
               field.c_str(), line.c_str(), offset);

      if (offset == std::string::npos)
        continue;

      // The status file has lines with the format <field>:<value>. Extract the
      // value after the colon.
      return android::base::Trim(line.substr(offset + field.size() + 1));
    }
  }

  return "[unknown]";
}

void Task::ReadStatusFields() {
  if (auto file = OpenTaskFilePointer("status")) {
    stdio_filebuf<char> filebuf(file.get());
    std::istream file_stream(&filebuf);

    for (std::string line; std::getline(file_stream, line);) {
      auto offset = line.find(':');
      if (offset == std::string::npos) {
        ALOGW("ReadStatusFields: Failed to find delimiter \":\" in line=\"%s\"",
              line.c_str());
        continue;
      }

      std::string key = line.substr(0, offset);
      std::string value = android::base::Trim(line.substr(offset + 1));

      ALOGD_IF(TRACE, "Task::ReadStatusFields: key=\"%s\" value=\"%s\"",
               key.c_str(), value.c_str());

      if (key == "Name")
        name_ = value;
      else if (key == "Tgid")
        thread_group_id_ = std::strtol(value.c_str(), nullptr, 10);
      else if (key == "PPid")
        parent_process_id_ = std::strtol(value.c_str(), nullptr, 10);
      else if (key == "Uid")
        ParseUidStatusField(value, user_id_);
      else if (key == "Gid")
        ParseUidStatusField(value, group_id_);
      else if (key == "Threads")
        thread_count_ = std::strtoul(value.c_str(), nullptr, 10);
      else if (key == "Cpus_allowed")
        cpus_allowed_mask_ = std::strtoul(value.c_str(), nullptr, 16);
      else if (key == "Cpus_allowed_list")
        cpus_allowed_list_ = value;
    }
  }
}

std::string Task::GetCpuSetPath() const {
  if (auto file = OpenTaskFilePointer("cpuset")) {
    stdio_filebuf<char> filebuf(file.get());
    std::istream file_stream(&filebuf);

    std::string line = "";
    std::getline(file_stream, line);

    return android::base::Trim(line);
  } else {
    return "";
  }
}

}  // namespace dvr
}  // namespace android
