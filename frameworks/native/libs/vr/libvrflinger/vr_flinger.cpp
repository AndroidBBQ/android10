#include <dvr/vr_flinger.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <memory>

#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <cutils/properties.h>
#include <log/log.h>
#include <private/dvr/display_client.h>
#include <processgroup/sched_policy.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include <functional>

#include "DisplayHardware/ComposerHal.h"
#include "display_manager_service.h"
#include "display_service.h"

namespace android {
namespace dvr {

std::unique_ptr<VrFlinger> VrFlinger::Create(
    Hwc2::Composer* hidl, hwc2_display_t primary_display_id,
    RequestDisplayCallback request_display_callback) {
  std::unique_ptr<VrFlinger> vr_flinger(new VrFlinger);
  if (vr_flinger->Init(hidl, primary_display_id, request_display_callback))
    return vr_flinger;
  else
    return nullptr;
}

VrFlinger::VrFlinger() {}

VrFlinger::~VrFlinger() {
  if (persistent_vr_state_callback_.get()) {
    sp<IVrManager> vr_manager = interface_cast<IVrManager>(
        defaultServiceManager()->checkService(String16("vrmanager")));
    if (vr_manager.get()) {
      vr_manager->unregisterPersistentVrStateListener(
          persistent_vr_state_callback_);
    }
  }

  if (dispatcher_)
    dispatcher_->SetCanceled(true);
  if (dispatcher_thread_.joinable())
    dispatcher_thread_.join();
}

bool VrFlinger::Init(Hwc2::Composer* hidl,
                     hwc2_display_t primary_display_id,
                     RequestDisplayCallback request_display_callback) {
  if (!hidl || !request_display_callback)
    return false;

  std::shared_ptr<android::pdx::Service> service;

  ALOGI("Starting up VrFlinger...");

  // We need to be able to create endpoints with full perms.
  umask(0000);

  android::ProcessState::self()->startThreadPool();

  request_display_callback_ = request_display_callback;

  dispatcher_ = android::pdx::ServiceDispatcher::Create();
  CHECK_ERROR(!dispatcher_, error, "Failed to create service dispatcher.");

  display_service_ = android::dvr::DisplayService::Create(
      hidl, primary_display_id, request_display_callback);
  CHECK_ERROR(!display_service_, error, "Failed to create display service.");
  dispatcher_->AddService(display_service_);

  service = android::dvr::DisplayManagerService::Create(display_service_);
  CHECK_ERROR(!service, error, "Failed to create display manager service.");
  dispatcher_->AddService(service);

  dispatcher_thread_ = std::thread([this]() {
    prctl(PR_SET_NAME, reinterpret_cast<unsigned long>("VrDispatch"), 0, 0, 0);
    ALOGI("Entering message loop.");

    setpriority(PRIO_PROCESS, 0, android::PRIORITY_URGENT_DISPLAY);
    set_sched_policy(0, SP_FOREGROUND);

    int ret = dispatcher_->EnterDispatchLoop();
    if (ret < 0) {
      ALOGE("Dispatch loop exited because: %s\n", strerror(-ret));
    }
  });

  return true;

error:
  return false;
}

void VrFlinger::OnBootFinished() {
  display_service_->OnBootFinished();
  sp<IVrManager> vr_manager = interface_cast<IVrManager>(
      defaultServiceManager()->checkService(String16("vrmanager")));
  if (vr_manager.get()) {
    persistent_vr_state_callback_ =
        new PersistentVrStateCallback(request_display_callback_);
    vr_manager->registerPersistentVrStateListener(
        persistent_vr_state_callback_);
  } else {
    ALOGE("Unable to register vr flinger for persistent vr mode changes");
  }
}

void VrFlinger::GrantDisplayOwnership() {
  display_service_->GrantDisplayOwnership();
}

void VrFlinger::SeizeDisplayOwnership() {
  display_service_->SeizeDisplayOwnership();
}

std::string VrFlinger::Dump() {
  // TODO(karthikrs): Add more state information here.
  return display_service_->DumpState(0/*unused*/);
}

void VrFlinger::PersistentVrStateCallback::onPersistentVrStateChanged(
    bool enabled) {
  ALOGV("Notified persistent vr mode is %s", enabled ? "on" : "off");
  // TODO(eieio): Determine the correct signal to request display control.
  // Persistent VR mode is not enough.
  // request_display_callback_(enabled);
}
}  // namespace dvr
}  // namespace android
