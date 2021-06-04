#ifndef ANDROID_DVR_PERFORMANCED_PERFORMANCE_SERVICE_H_
#define ANDROID_DVR_PERFORMANCED_PERFORMANCE_SERVICE_H_

#include <functional>
#include <string>
#include <unordered_map>

#include <pdx/service.h>

#include "cpu_set.h"
#include "task.h"

namespace android {
namespace dvr {

// PerformanceService manages compute partitions usings cpusets. Different
// cpusets are assigned specific purposes and performance characteristics;
// clients may request for threads to be moved into these cpusets to help
// achieve system performance goals.
class PerformanceService : public pdx::ServiceBase<PerformanceService> {
 public:
  pdx::Status<void> HandleMessage(pdx::Message& message) override;
  bool IsInitialized() const override;

  std::string DumpState(size_t max_length) override;

 private:
  friend BASE;

  PerformanceService();

  pdx::Status<void> OnSetSchedulerPolicy(pdx::Message& message, pid_t task_id,
                                         const std::string& scheduler_class);

  pdx::Status<void> OnSetCpuPartition(pdx::Message& message, pid_t task_id,
                                      const std::string& partition);
  pdx::Status<void> OnSetSchedulerClass(pdx::Message& message, pid_t task_id,
                                        const std::string& scheduler_class);
  pdx::Status<std::string> OnGetCpuPartition(pdx::Message& message,
                                             pid_t task_id);

  // Set which thread gets the vr:app:render policy. Only one thread at a time
  // is allowed to have vr:app:render. If multiple threads are allowed
  // vr:app:render, and those threads busy loop, the system can freeze. When
  // SetVrAppRenderThread() is called, the thread which we had previously
  // assigned vr:app:render will have its scheduling policy reset to default
  // values.
  void SetVrAppRenderThread(pid_t new_vr_app_render_thread);

  CpuSetManager cpuset_;

  int sched_fifo_min_priority_;
  int sched_fifo_max_priority_;

  struct SchedulerPolicyConfig {
    unsigned long timer_slack;
    int scheduler_policy;
    int priority;
    std::function<bool(const pdx::Message& message, const Task& task)>
        permission_check;
    std::string cpuset;

    // Check the permisison of the given task to use this scheduler class. If a
    // permission check function is not set then operations are only allowed on
    // tasks in the sender's process.
    bool IsAllowed(const pdx::Message& sender, const Task& task) const {
      if (permission_check)
        return permission_check(sender, task);
      else if (!task || task.thread_group_id() != sender.GetProcessId())
        return false;
      else
        return true;
    }
  };

  std::unordered_map<std::string, SchedulerPolicyConfig> scheduler_policies_;

  std::function<bool(const pdx::Message& message, const Task& task)>
      partition_permission_check_;

  pid_t vr_app_render_thread_ = -1;

  PerformanceService(const PerformanceService&) = delete;
  void operator=(const PerformanceService&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_PERFORMANCED_PERFORMANCE_SERVICE_H_
