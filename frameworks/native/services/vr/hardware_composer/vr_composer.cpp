#include "vr_composer.h"

#include <binder/IPCThreadState.h>
#include <binder/PermissionCache.h>

namespace android {
namespace dvr {
namespace {

bool CheckPermission() {
  const android::IPCThreadState* ipc = android::IPCThreadState::self();
  const pid_t pid = ipc->getCallingPid();
  const uid_t uid = ipc->getCallingUid();
  const bool permission = PermissionCache::checkPermission(
      String16("android.permission.RESTRICTED_VR_ACCESS"), pid, uid);
  if (!permission)
    ALOGE("permission denied to pid=%d uid=%u", pid, uid);

  return permission;
}

}  // namespace

VrComposer::VrComposer(ComposerView* composer_view)
  : composer_view_(composer_view) {
  composer_view_->RegisterObserver(this);
}

VrComposer::~VrComposer() {
  composer_view_->UnregisterObserver(this);
}

binder::Status VrComposer::registerObserver(
    const sp<IVrComposerCallback>& callback) {
  {
    std::lock_guard<std::mutex> guard(mutex_);

    if (!CheckPermission())
      return binder::Status::fromStatusT(PERMISSION_DENIED);

    if (callback_.get()) {
      ALOGE("Failed to register callback, already registered");
      return binder::Status::fromStatusT(ALREADY_EXISTS);
    }

    callback_ = callback;
    IInterface::asBinder(callback_)->linkToDeath(this);
  }

  // Don't take the lock to force display refresh otherwise it could end in a
  // deadlock since HWC calls this with new frames and it has a lock of its own
  // to serialize access to the display information.
  composer_view_->ForceDisplaysRefresh();
  return binder::Status::ok();
}

binder::Status VrComposer::clearObserver() {
  std::lock_guard<std::mutex> guard(mutex_);
  callback_ = nullptr;
  return binder::Status::ok();
}

base::unique_fd VrComposer::OnNewFrame(const ComposerView::Frame& frame) {
  std::lock_guard<std::mutex> guard(mutex_);

  if (!callback_.get())
    return base::unique_fd();

  ParcelableComposerFrame parcelable_frame(frame);
  ParcelableUniqueFd fence;
  binder::Status ret = callback_->onNewFrame(parcelable_frame, &fence);
  if (!ret.isOk())
    ALOGE("Failed to send new frame: %s", ret.toString8().string());

  return fence.fence();
}

void VrComposer::binderDied(const wp<IBinder>& /* who */) {
  std::lock_guard<std::mutex> guard(mutex_);

  callback_ = nullptr;
}

}  // namespace dvr
}  // namespace android
