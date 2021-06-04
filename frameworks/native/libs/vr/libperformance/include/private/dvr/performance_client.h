#ifndef ANDROID_DVR_PERFORMANCE_CLIENT_H_
#define ANDROID_DVR_PERFORMANCE_CLIENT_H_

#include <sys/types.h>

#include <cstddef>
#include <string>
#include <tuple>

#include <pdx/client.h>

namespace android {
namespace dvr {

class PerformanceClient : public pdx::ClientBase<PerformanceClient> {
 public:
  int SetSchedulerPolicy(pid_t task_id, const std::string& scheduler_policy);
  int SetSchedulerPolicy(pid_t task_id, const char* scheduler_policy);

  // TODO(eieio): Consider deprecating this API.
  int SetCpuPartition(pid_t task_id, const std::string& partition);
  int SetCpuPartition(pid_t task_id, const char* partition);
  int SetSchedulerClass(pid_t task_id, const std::string& scheduler_class);
  int SetSchedulerClass(pid_t task_id, const char* scheduler_class);
  int GetCpuPartition(pid_t task_id, std::string* partition_out);
  int GetCpuPartition(pid_t task_id, char* partition_out, std::size_t size);

 private:
  friend BASE;

  explicit PerformanceClient(int* error);

  PerformanceClient(const PerformanceClient&) = delete;
  void operator=(const PerformanceClient&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_PERFORMANCE_CLIENT_H_
