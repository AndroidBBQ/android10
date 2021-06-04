#include "include/private/dvr/performance_client.h"

#include <sys/types.h>

#include <pdx/default_transport/client_channel_factory.h>
#include <pdx/rpc/remote_method.h>
#include <pdx/rpc/string_wrapper.h>
#include <private/dvr/performance_rpc.h>

using android::pdx::rpc::WrapString;

namespace android {
namespace dvr {

PerformanceClient::PerformanceClient(int* error)
    : BASE(pdx::default_transport::ClientChannelFactory::Create(
          PerformanceRPC::kClientPath)) {
  if (error)
    *error = Client::error();
}

int PerformanceClient::SetCpuPartition(pid_t task_id,
                                       const std::string& partition) {
  if (task_id == 0)
    task_id = gettid();

  return ReturnStatusOrError(
      InvokeRemoteMethod<PerformanceRPC::SetCpuPartition>(task_id, partition));
}

int PerformanceClient::SetCpuPartition(pid_t task_id, const char* partition) {
  if (task_id == 0)
    task_id = gettid();

  return ReturnStatusOrError(
      InvokeRemoteMethod<PerformanceRPC::SetCpuPartition>(
          task_id, WrapString(partition)));
}

int PerformanceClient::SetSchedulerPolicy(pid_t task_id,
                                          const std::string& scheduler_policy) {
  if (task_id == 0)
    task_id = gettid();

  return ReturnStatusOrError(
      InvokeRemoteMethod<PerformanceRPC::SetSchedulerPolicy>(task_id,
                                                             scheduler_policy));
}

int PerformanceClient::SetSchedulerPolicy(pid_t task_id,
                                          const char* scheduler_policy) {
  if (task_id == 0)
    task_id = gettid();

  return ReturnStatusOrError(
      InvokeRemoteMethod<PerformanceRPC::SetSchedulerPolicy>(
          task_id, WrapString(scheduler_policy)));
}

int PerformanceClient::SetSchedulerClass(pid_t task_id,
                                         const std::string& scheduler_class) {
  if (task_id == 0)
    task_id = gettid();

  return ReturnStatusOrError(
      InvokeRemoteMethod<PerformanceRPC::SetSchedulerClass>(task_id,
                                                            scheduler_class));
}

int PerformanceClient::SetSchedulerClass(pid_t task_id,
                                         const char* scheduler_class) {
  if (task_id == 0)
    task_id = gettid();

  return ReturnStatusOrError(
      InvokeRemoteMethod<PerformanceRPC::SetSchedulerClass>(
          task_id, WrapString(scheduler_class)));
}

int PerformanceClient::GetCpuPartition(pid_t task_id,
                                       std::string* partition_out) {
  if (partition_out == nullptr)
    return -EINVAL;

  if (task_id == 0)
    task_id = gettid();

  auto status = InvokeRemoteMethodInPlace<PerformanceRPC::GetCpuPartition>(
      partition_out, task_id);
  return status ? 0 : -status.error();
}

int PerformanceClient::GetCpuPartition(pid_t task_id, char* partition_out,
                                       std::size_t size) {
  if (partition_out == nullptr)
    return -EINVAL;

  if (task_id == 0)
    task_id = gettid();

  auto wrapper = WrapString(partition_out, size);
  auto status = InvokeRemoteMethodInPlace<PerformanceRPC::GetCpuPartition>(
      &wrapper, task_id);
  if (!status)
    return -status.error();

  if (wrapper.size() < size)
    partition_out[wrapper.size()] = '\0';

  return 0;
}

}  // namespace dvr
}  // namespace android

extern "C" int dvrSetCpuPartition(pid_t task_id, const char* partition) {
  int error;
  if (auto client = android::dvr::PerformanceClient::Create(&error))
    return client->SetCpuPartition(task_id, partition);
  else
    return error;
}

extern "C" int dvrSetSchedulerPolicy(pid_t task_id,
                                     const char* scheduler_policy) {
  int error;
  if (auto client = android::dvr::PerformanceClient::Create(&error))
    return client->SetSchedulerPolicy(task_id, scheduler_policy);
  else
    return error;
}

extern "C" int dvrSetSchedulerClass(pid_t task_id,
                                    const char* scheduler_class) {
  int error;
  if (auto client = android::dvr::PerformanceClient::Create(&error))
    return client->SetSchedulerClass(task_id, scheduler_class);
  else
    return error;
}

extern "C" int dvrGetCpuPartition(pid_t task_id, char* partition, size_t size) {
  int error;
  if (auto client = android::dvr::PerformanceClient::Create(&error))
    return client->GetCpuPartition(task_id, partition, size);
  else
    return error;
}
