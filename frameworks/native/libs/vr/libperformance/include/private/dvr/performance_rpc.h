#ifndef ANDROID_DVR_PERFORMANCE_RPC_H_
#define ANDROID_DVR_PERFORMANCE_RPC_H_

#include <sys/types.h>

#include <string>

#include <pdx/rpc/remote_method_type.h>

namespace android {
namespace dvr {

// Performance Service RPC interface. Defines the endpoint paths, op codes, and
// method type signatures supported by performanced.
struct PerformanceRPC {
  // Service path.
  static constexpr char kClientPath[] = "system/performance/client";

  // Op codes.
  enum {
    kOpSetCpuPartition = 0,
    kOpSetSchedulerClass,
    kOpGetCpuPartition,
    kOpSetSchedulerPolicy,
  };

  // Methods.
  PDX_REMOTE_METHOD(SetCpuPartition, kOpSetCpuPartition,
                    void(pid_t, const std::string&));
  PDX_REMOTE_METHOD(SetSchedulerClass, kOpSetSchedulerClass,
                    void(pid_t, const std::string&));
  PDX_REMOTE_METHOD(GetCpuPartition, kOpGetCpuPartition, std::string(pid_t));
  PDX_REMOTE_METHOD(SetSchedulerPolicy, kOpSetSchedulerPolicy,
                    void(pid_t, const std::string&));
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_PERFORMANCE_RPC_H_
