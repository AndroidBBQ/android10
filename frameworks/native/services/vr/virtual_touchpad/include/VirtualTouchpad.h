#ifndef ANDROID_DVR_VIRTUAL_TOUCHPAD_INTERFACE_H
#define ANDROID_DVR_VIRTUAL_TOUCHPAD_INTERFACE_H

#include "dvr/virtual_touchpad_client.h"

#include <memory>
#include <utils/Errors.h>
#include <utils/String8.h>

namespace android {
namespace dvr {

// Provides a virtual touchpad for injecting events into the input system.
//
class VirtualTouchpad {
 public:
  enum : int {
    PRIMARY = DVR_VIRTUAL_TOUCHPAD_PRIMARY,
    VIRTUAL = DVR_VIRTUAL_TOUCHPAD_VIRTUAL,
  };

  virtual ~VirtualTouchpad() {}

  // Create a virtual touchpad.
  // Implementations should provide this, and hide their constructors.
  // For the user, switching implementations should be as simple as changing
  // the class whose |Create()| is called.
  // Implementations should be minimial; major resource allocation should
  // be performed in Attach().
  static std::unique_ptr<VirtualTouchpad> Create() {
    return nullptr;
  }

  // Initialize a virtual touchpad.
  virtual status_t Attach() = 0;

  // Shut down a virtual touchpad.
  virtual status_t Detach() = 0;

  // Generate a simulated touch event.
  //
  // @param touchpad Touchpad selector index.
  // @param x Horizontal touch position.
  // @param y Vertical touch position.
  //            Values must be in the range [0.0, 1.0).
  // @param pressure Touch pressure.
  //            Positive values represent contact; use 1.0f if contact
  //            is binary. Use 0.0f for no contact.
  // @returns OK on success.
  //
  virtual status_t Touch(int touchpad, float x, float y, float pressure) = 0;

  // Generate a simulated touchpad button state.
  //
  // @param touchpad Touchpad selector index.
  // @param buttons A union of MotionEvent BUTTON_* values.
  // @returns OK on success.
  //
  // Currently only BUTTON_BACK is supported, as the implementation
  // restricts itself to operations actually required by VrWindowManager.
  //
  virtual status_t ButtonState(int touchpad, int buttons) = 0;

  // Generate a simulated scroll event.
  //
  // @param touchpad Touchpad selector index.
  // @param x Horizontal scroll increment.
  // @param y Vertical scroll increment.
  //            Values must be in the range [-1.0, 1.0].
  // @returns OK on success.
  //
  virtual status_t Scroll(int touchpad, float x, float y) = 0;

  // Report state for 'dumpsys'.
  virtual void dumpInternal(String8& result) = 0;

 protected:
  VirtualTouchpad() {}

 private:
  VirtualTouchpad(const VirtualTouchpad&) = delete;
  void operator=(const VirtualTouchpad&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_VIRTUAL_TOUCHPAD_INTERFACE_H
