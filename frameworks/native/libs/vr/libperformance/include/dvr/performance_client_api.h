#ifndef ANDROID_DVR_PERFORMANCE_CLIENT_API_H_
#define ANDROID_DVR_PERFORMANCE_CLIENT_API_H_

#include <stddef.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

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
int dvrSetSchedulerPolicy(pid_t task_id, const char* scheduler_policy);

/// Sets the CPU partition for a task.
///
/// Sets the CPU partition for a task to the partition described by a CPU
/// partition path.
///
/// TODO(eieio): Describe supported partitions and rules governing assignment.
///
/// @param task_id The task id of task to attach to a partition. When task_id is
/// 0 the current task id is substituted.
/// @param partition NULL-terminated ASCII string describing the CPU partition
/// to attach the task to.
/// @returns Returns 0 on success or a negative errno error code on error.
int dvrSetCpuPartition(pid_t task_id, const char* partition);

/// Sets the scheduler class for a task.
///
/// Sets the scheduler class for a task to the class described by a semantic
/// string.
///
/// Supported classes for applications are: audio, graphics, normal, and
/// background. Additional options following a ':' to be supported in the
/// future.
///
/// @param task_id The task id of task to attach to a partition. When task_id is
/// 0 the current task id is substituted.
/// @param scheduler_class NULL-terminated ASCII string containing the desired
/// scheduler class.
/// @returns Returns 0 on success or a negative errno error code on error.
int dvrSetSchedulerClass(pid_t task_id, const char* scheduler_class);

/// Gets the CPU partition for a task.
///
/// Gets the CPU partition path for a task as a NULL-terminated ASCII string. If
/// the path is too large to fit in the supplied buffer, -ENOBUFS is returned.
///
/// @param task_id The task id of the task to retrieve the partition for. When
/// task_id is 0 the current task id is substituted.
/// @param partition Pointer to an ASCII string buffer to store the partition
/// path.
/// @param size Size of the string buffer in bytes, including space for the NULL
/// terminator.
/// @returns Returns 0 on success or a negative errno error code on error.
int dvrGetCpuPartition(pid_t task_id, char* partition, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANDROID_DVR_PERFORMANCE_CLIENT_API_H_
