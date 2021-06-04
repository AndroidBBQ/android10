#ifndef ANDROID_DVR_VIRTUAL_TOUCHPAD_SERVICE_H
#define ANDROID_DVR_VIRTUAL_TOUCHPAD_SERVICE_H

#include <android/dvr/BnVirtualTouchpadService.h>

#include "VirtualTouchpad.h"

namespace android {
namespace dvr {

// VirtualTouchpadService implements the service side of
// the Binder interface defined in VirtualTouchpadService.aidl.
//
class VirtualTouchpadService : public BnVirtualTouchpadService {
 public:
  explicit VirtualTouchpadService(std::unique_ptr<VirtualTouchpad> touchpad)
      : touchpad_(std::move(touchpad)), client_pid_(0) {}
  ~VirtualTouchpadService() override;

 protected:
  // Implements IVirtualTouchpadService.
  binder::Status attach() override;
  binder::Status detach() override;
  binder::Status touch(int touchpad, float x, float y, float pressure) override;
  binder::Status buttonState(int touchpad, int buttons) override;
  binder::Status scroll(int touchpad, float x, float y) override;

  // Implements BBinder::dump().
  status_t dump(int fd, const Vector<String16>& args) override;

 private:
  bool CheckPermissions();
  bool CheckTouchPermission(pid_t* out_pid);

  std::unique_ptr<VirtualTouchpad> touchpad_;

  // Only one client at a time can use the virtual touchpad.
  pid_t client_pid_;

  VirtualTouchpadService(const VirtualTouchpadService&) = delete;
  void operator=(const VirtualTouchpadService&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_VIRTUAL_TOUCHPAD_SERVICE_H
