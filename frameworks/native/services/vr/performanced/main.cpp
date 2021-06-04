#include <errno.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/stat.h>

#include <cutils/properties.h>
#include <cutils/sched_policy.h>
#include <log/log.h>
#include <sys/resource.h>
#include <utils/threads.h>

#include <pdx/service_dispatcher.h>
#include <private/android_filesystem_config.h>

#include "performance_service.h"

namespace {

// Annoying that sys/capability.h doesn't define this directly.
constexpr int kMaxCapNumber = (CAP_TO_INDEX(CAP_LAST_CAP) + 1);

}  // anonymous namespace

int main(int /*argc*/, char** /*argv*/) {
  int ret = -1;

  struct __user_cap_header_struct capheader;
  struct __user_cap_data_struct capdata[kMaxCapNumber];

  std::shared_ptr<android::pdx::Service> service;
  std::unique_ptr<android::pdx::ServiceDispatcher> dispatcher;

  ALOGI("Starting up...");

  // We need to be able to create endpoints with full perms.
  umask(0000);

  // Keep capabilities when switching UID to AID_SYSTEM.
  ret = prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
  CHECK_ERROR(ret < 0, error, "Failed to set KEEPCAPS: %s", strerror(errno));

  // Set UID and GID to system.
  ret = setresgid(AID_SYSTEM, AID_SYSTEM, AID_SYSTEM);
  CHECK_ERROR(ret < 0, error, "Failed to set GID: %s", strerror(errno));
  ret = setresuid(AID_SYSTEM, AID_SYSTEM, AID_SYSTEM);
  CHECK_ERROR(ret < 0, error, "Failed to set UID: %s", strerror(errno));

  // Keep CAP_SYS_NICE, allowing control of scheduler class, priority, and
  // cpuset for other tasks in the system.
  memset(&capheader, 0, sizeof(capheader));
  memset(&capdata, 0, sizeof(capdata));
  capheader.version = _LINUX_CAPABILITY_VERSION_3;
  capdata[CAP_TO_INDEX(CAP_SYS_NICE)].effective |= CAP_TO_MASK(CAP_SYS_NICE);
  capdata[CAP_TO_INDEX(CAP_SYS_NICE)].permitted |= CAP_TO_MASK(CAP_SYS_NICE);

  // Drop all caps but the ones configured above.
  ret = capset(&capheader, capdata);
  CHECK_ERROR(ret < 0, error, "Could not set capabilities: %s",
              strerror(errno));

  dispatcher = android::pdx::ServiceDispatcher::Create();
  CHECK_ERROR(!dispatcher, error, "Failed to create service dispatcher.");

  service = android::dvr::PerformanceService::Create();
  CHECK_ERROR(!service, error, "Failed to create performance service service.");
  dispatcher->AddService(service);

  ALOGI("Entering message loop.");

  ret = dispatcher->EnterDispatchLoop();
  CHECK_ERROR(ret < 0, error, "Dispatch loop exited because: %s\n",
              strerror(-ret));

error:
  return ret;
}
