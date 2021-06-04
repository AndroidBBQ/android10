#include "include/dvr/dvr_performance.h"

#include <private/dvr/performance_client.h>

using android::dvr::PerformanceClient;

extern "C" {

int dvrPerformanceSetSchedulerPolicy(pid_t task_id,
                                     const char* scheduler_policy) {
  int error;
  if (auto client = PerformanceClient::Create(&error))
    return client->SetSchedulerPolicy(task_id, scheduler_policy);
  else
    return error;
}

}  // extern "C"
