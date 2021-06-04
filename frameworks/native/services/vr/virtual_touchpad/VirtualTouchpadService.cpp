#include "VirtualTouchpadService.h"

#include <inttypes.h>

#include <binder/IPCThreadState.h>
#include <binder/PermissionCache.h>
#include <binder/Status.h>
#include <cutils/log.h>
#include <linux/input.h>
#include <private/android_filesystem_config.h>
#include <utils/Errors.h>

namespace android {
namespace dvr {

namespace {
const String16 kDumpPermission("android.permission.DUMP");
const String16 kTouchPermission("android.permission.RESTRICTED_VR_ACCESS");
}  // anonymous namespace

VirtualTouchpadService::~VirtualTouchpadService() {
  if (client_pid_) {
    client_pid_ = 0;
    touchpad_->Detach();
  }
}

binder::Status VirtualTouchpadService::attach() {
  pid_t pid;
  if (!CheckTouchPermission(&pid)) {
    return binder::Status::fromStatusT(PERMISSION_DENIED);
  }
  if (client_pid_ == pid) {
    // The same client has called attach() twice with no intervening detach().
    // This indicates a problem with the client, so return an error.
    // However, since the client is already attached, any touchpad actions
    // it takes will still work.
    ALOGE("pid=%ld attached twice", static_cast<long>(pid));
    return binder::Status::fromStatusT(ALREADY_EXISTS);
  }
  if (client_pid_ != 0) {
    // Attach while another client is attached. This can happen if the client
    // dies without cleaning up after itself, so move ownership to the current
    // caller. If two actual clients have connected, the problem will be
    // reported when the previous client performs any touchpad action.
    ALOGE("pid=%ld replaces %ld", static_cast<long>(pid),
          static_cast<long>(client_pid_));
    client_pid_ = pid;
    return binder::Status::ok();
  }
  client_pid_ = pid;
  if (const status_t error = touchpad_->Attach()) {
    return binder::Status::fromStatusT(error);
  }
  return binder::Status::ok();
}

binder::Status VirtualTouchpadService::detach() {
  if (!CheckPermissions()) {
    return binder::Status::fromStatusT(PERMISSION_DENIED);
  }
  client_pid_ = 0;
  if (const status_t error = touchpad_->Detach()) {
    return binder::Status::fromStatusT(error);
  }
  return binder::Status::ok();
}

binder::Status VirtualTouchpadService::touch(int touchpad, float x, float y,
                                             float pressure) {
  if (!CheckPermissions()) {
    return binder::Status::fromStatusT(PERMISSION_DENIED);
  }
  if (const status_t error = touchpad_->Touch(touchpad, x, y, pressure)) {
    return binder::Status::fromStatusT(error);
  }
  return binder::Status::ok();
}

binder::Status VirtualTouchpadService::buttonState(int touchpad, int buttons) {
  if (!CheckPermissions()) {
    return binder::Status::fromStatusT(PERMISSION_DENIED);
  }
  if (const status_t error = touchpad_->ButtonState(touchpad, buttons)) {
    return binder::Status::fromStatusT(error);
  }
  return binder::Status::ok();
}

binder::Status VirtualTouchpadService::scroll(int touchpad, float x, float y) {
  if (!CheckPermissions()) {
    return binder::Status::fromStatusT(PERMISSION_DENIED);
  }
  if (const status_t error = touchpad_->Scroll(touchpad, x, y)) {
    return binder::Status::fromStatusT(error);
  }
  return binder::Status::ok();
}

status_t VirtualTouchpadService::dump(
    int fd, const Vector<String16>& args[[gnu::unused]]) {
  String8 result;
  const android::IPCThreadState* ipc = android::IPCThreadState::self();
  const pid_t pid = ipc->getCallingPid();
  const uid_t uid = ipc->getCallingUid();
  if ((uid != AID_SHELL) &&
      !PermissionCache::checkPermission(kDumpPermission, pid, uid)) {
    result.appendFormat("Permission denial: can't dump " LOG_TAG
                        " from pid=%ld, uid=%ld\n",
                        static_cast<long>(pid), static_cast<long>(uid));
  } else {
    result.appendFormat("[service]\nclient_pid = %ld\n\n",
                        static_cast<long>(client_pid_));
    touchpad_->dumpInternal(result);
  }
  write(fd, result.string(), result.size());
  return OK;
}

bool VirtualTouchpadService::CheckPermissions() {
  pid_t pid;
  if (!CheckTouchPermission(&pid)) {
    return false;
  }
  if (client_pid_ != pid) {
    ALOGE("pid=%ld is not owner", static_cast<long>(pid));
    return false;
  }
  return true;
}

bool VirtualTouchpadService::CheckTouchPermission(pid_t* out_pid) {
  const android::IPCThreadState* ipc = android::IPCThreadState::self();
  *out_pid = ipc->getCallingPid();
  const uid_t uid = ipc->getCallingUid();
  const bool permission = PermissionCache::checkPermission(kTouchPermission, *out_pid, uid);
  if (!permission) {
    ALOGE("permission denied to pid=%ld uid=%ld", static_cast<long>(*out_pid),
          static_cast<long>(uid));
  }
  return permission;
}

}  // namespace dvr
}  // namespace android
