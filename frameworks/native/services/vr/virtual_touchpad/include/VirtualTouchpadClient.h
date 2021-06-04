#ifndef ANDROID_DVR_VIRTUAL_TOUCHPAD_CLIENT_H
#define ANDROID_DVR_VIRTUAL_TOUCHPAD_CLIENT_H

#include "VirtualTouchpad.h"

namespace android {
namespace dvr {

// VirtualTouchpadClient implements a VirtualTouchpad by connecting to
// a VirtualTouchpadService over Binder.
//
class VirtualTouchpadClient : public VirtualTouchpad {
 public:
  // VirtualTouchpad implementation:
  static std::unique_ptr<VirtualTouchpad> Create();

 protected:
  VirtualTouchpadClient() {}
  ~VirtualTouchpadClient() override {}

 private:
  VirtualTouchpadClient(const VirtualTouchpadClient&) = delete;
  void operator=(const VirtualTouchpadClient&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_VIRTUAL_TOUCHPAD_CLIENT_H
