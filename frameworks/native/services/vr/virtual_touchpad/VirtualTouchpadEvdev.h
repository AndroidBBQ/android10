#ifndef ANDROID_DVR_VIRTUAL_TOUCHPAD_EVDEV_H
#define ANDROID_DVR_VIRTUAL_TOUCHPAD_EVDEV_H

#include "EvdevInjector.h"
#include "VirtualTouchpad.h"

namespace android {
namespace dvr {

class EvdevInjector;

// VirtualTouchpadEvdev implements a VirtualTouchpad by injecting evdev events.
//
class VirtualTouchpadEvdev : public VirtualTouchpad {
 public:
  static std::unique_ptr<VirtualTouchpad> Create();
  ~VirtualTouchpadEvdev() override {}

  // VirtualTouchpad implementation:
  status_t Attach() override;
  status_t Detach() override;
  status_t Touch(int touchpad, float x, float y, float pressure) override;
  status_t ButtonState(int touchpad, int buttons) override;
  status_t Scroll(int touchpad, float x, float y) override;
  void dumpInternal(String8& result) override;

 protected:
  static constexpr int kTouchpads = 2;

  VirtualTouchpadEvdev() {}
  void Reset();

  // Must be called only between construction (or Detach()) and Attach().
  inline void SetEvdevInjectorForTesting(int touchpad,
                                         EvdevInjector* injector) {
    touchpad_[touchpad].injector = injector;
  }

 private:
  // Per-touchpad state.
  struct Touchpad {
    // Except for testing, the |EvdevInjector| used to inject evdev events.
    std::unique_ptr<EvdevInjector> owned_injector;

    // Active pointer to |owned_injector_| or to a testing injector.
    EvdevInjector* injector = nullptr;

    // Previous (x, y) position in device space, to suppress redundant events.
    int32_t last_device_x;
    int32_t last_device_y;

    // Records current touch state (0=up 1=down) in bit 0, and previous state
    // in bit 1, to track transitions.
    int touches;

    // Previous injected button state, to detect changes.
    int32_t last_motion_event_buttons;
  };
  Touchpad touchpad_[kTouchpads];

  VirtualTouchpadEvdev(const VirtualTouchpadEvdev&) = delete;
  void operator=(const VirtualTouchpadEvdev&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_VIRTUAL_TOUCHPAD_EVDEV_H
