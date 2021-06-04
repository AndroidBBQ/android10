#ifndef ANDROID_DVR_PERFORMANCE_H_
#define ANDROID_DVR_PERFORMANCE_H_

#include <stddef.h>
#include <unistd.h>

__BEGIN_DECLS

/// Sets the scheduler policy for a task.
///
/// Sets the scheduler policy for a task to the class described by a semantic
/// string.
///
/// Supported policies are device-specific.
///
/// @param task_id The task id of task to set the policy for. When task_id is 0
/// the current task id is substituted.
/// @param scheduler_policy NULL-terminated ASCII string containing the desired
/// scheduler policy.
/// @returns Returns 0 on success or a negative errno error code on error.
int dvrPerformanceSetSchedulerPolicy(pid_t task_id, const char* scheduler_policy);

__END_DECLS

#endif  // ANDROID_DVR_PERFORMANCE_H_

