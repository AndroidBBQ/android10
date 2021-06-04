#include "VirtualTouchpadEvdev.h"

#include <android/input.h>
#include <inttypes.h>
#include <linux/input.h>
#include <log/log.h>

// References:
//  [0] Multi-touch (MT) Protocol,
//      https://www.kernel.org/doc/Documentation/input/multi-touch-protocol.txt

namespace android {
namespace dvr {

namespace {

// Virtual evdev device properties. The name is arbitrary, but Android can
// use it to look up device configuration, so it must be unique. Vendor and
// product values must be 0 to indicate an internal device and prevent a
// similar lookup that could conflict with a physical device.
static const char* const kDeviceNameFormat = "vr-virtual-touchpad-%d";
static constexpr int16_t kDeviceBusType = BUS_VIRTUAL;
static constexpr int16_t kDeviceVendor = 0;
static constexpr int16_t kDeviceProduct = 0;
static constexpr int16_t kDeviceVersion = 0x0001;

static constexpr int32_t kWidth = 0x10000;
static constexpr int32_t kHeight = 0x10000;
static constexpr int32_t kSlots = 2;

static constexpr float kScrollScale = 100.0f;

int32_t scale_relative_scroll(float x) {
  return kScrollScale * x;
}

}  // anonymous namespace

std::unique_ptr<VirtualTouchpad> VirtualTouchpadEvdev::Create() {
  std::unique_ptr<VirtualTouchpadEvdev> touchpad(new VirtualTouchpadEvdev());
  touchpad->Reset();
  return touchpad;
}

void VirtualTouchpadEvdev::Reset() {
  for (auto& touchpad : touchpad_) {
    if (touchpad.injector) {
      touchpad.injector->Close();
    }
    touchpad.injector = nullptr;
    touchpad.owned_injector.reset();
    touchpad.last_device_x = INT32_MIN;
    touchpad.last_device_y = INT32_MIN;
    touchpad.touches = 0;
    touchpad.last_motion_event_buttons = 0;
  }
}

status_t VirtualTouchpadEvdev::Attach() {
  status_t status = OK;
  for (int i = 0; i < kTouchpads; ++i) {
    Touchpad& touchpad = touchpad_[i];
    if (!touchpad.injector) {
      touchpad.owned_injector.reset(new EvdevInjector());
      touchpad.injector = touchpad.owned_injector.get();
    }
    String8 DeviceName;
    DeviceName.appendFormat(kDeviceNameFormat, i);
    touchpad.injector->ConfigureBegin(DeviceName, kDeviceBusType,
                                      kDeviceVendor, kDeviceProduct,
                                      kDeviceVersion);
    touchpad.injector->ConfigureInputProperty(INPUT_PROP_DIRECT);
    touchpad.injector->ConfigureMultiTouchXY(0, 0, kWidth - 1, kHeight - 1);
    touchpad.injector->ConfigureAbsSlots(kSlots);
    touchpad.injector->ConfigureRel(REL_WHEEL);
    touchpad.injector->ConfigureRel(REL_HWHEEL);
    touchpad.injector->ConfigureKey(BTN_TOUCH);
    touchpad.injector->ConfigureKey(BTN_BACK);
    touchpad.injector->ConfigureEnd();
    if (const status_t configuration_status =  touchpad.injector->GetError()) {
      status = configuration_status;
    }
  }
  return status;
}

status_t VirtualTouchpadEvdev::Detach() {
  Reset();
  return OK;
}

int VirtualTouchpadEvdev::Touch(int touchpad_id, float x, float y,
                                float pressure) {
  if (touchpad_id < 0 || touchpad_id >= kTouchpads) {
    return EINVAL;
  }
  int32_t device_x = x * kWidth;
  int32_t device_y = y * kHeight;
  Touchpad& touchpad = touchpad_[touchpad_id];
  touchpad.touches = ((touchpad.touches & 1) << 1) | (pressure > 0);
  ALOGV("(%f,%f) %f -> (%" PRId32 ",%" PRId32 ") %d", x, y, pressure, device_x,
        device_y, touchpad.touches);

  if (!touchpad.injector) {
    return EvdevInjector::ERROR_SEQUENCING;
  }
  touchpad.injector->ResetError();
  switch (touchpad.touches) {
    case 0b00:  // Hover continues.
      if (device_x != touchpad.last_device_x ||
          device_y != touchpad.last_device_y) {
        touchpad.injector->SendMultiTouchXY(0, 0, device_x, device_y);
        touchpad.injector->SendSynReport();
      }
      break;
    case 0b01:  // Touch begins.
      // Press.
      touchpad.injector->SendMultiTouchXY(0, 0, device_x, device_y);
      touchpad.injector->SendKey(BTN_TOUCH, EvdevInjector::KEY_PRESS);
      touchpad.injector->SendSynReport();
      break;
    case 0b10:  // Touch ends.
      touchpad.injector->SendKey(BTN_TOUCH, EvdevInjector::KEY_RELEASE);
      touchpad.injector->SendMultiTouchLift(0);
      touchpad.injector->SendSynReport();
      break;
    case 0b11:  // Touch continues.
      if (device_x != touchpad.last_device_x ||
          device_y != touchpad.last_device_y) {
        touchpad.injector->SendMultiTouchXY(0, 0, device_x, device_y);
        touchpad.injector->SendSynReport();
      }
      break;
  }
  touchpad.last_device_x = device_x;
  touchpad.last_device_y = device_y;

  return touchpad.injector->GetError();
}

int VirtualTouchpadEvdev::ButtonState(int touchpad_id, int buttons) {
  if (touchpad_id < 0 || touchpad_id >= kTouchpads) {
    return EINVAL;
  }
  Touchpad& touchpad = touchpad_[touchpad_id];
  const int changes = touchpad.last_motion_event_buttons ^ buttons;
  if (!changes) {
    return 0;
  }
  if (buttons & ~AMOTION_EVENT_BUTTON_BACK) {
    return ENOTSUP;
  }
  ALOGV("change %X from %X to %X", changes, touchpad.last_motion_event_buttons,
        buttons);

  if (!touchpad.injector) {
    return EvdevInjector::ERROR_SEQUENCING;
  }
  touchpad.injector->ResetError();
  if (changes & AMOTION_EVENT_BUTTON_BACK) {
    touchpad.injector->SendKey(BTN_BACK, (buttons & AMOTION_EVENT_BUTTON_BACK)
                                             ? EvdevInjector::KEY_PRESS
                                             : EvdevInjector::KEY_RELEASE);
    touchpad.injector->SendSynReport();
  }
  touchpad.last_motion_event_buttons = buttons;
  return touchpad.injector->GetError();
}

int VirtualTouchpadEvdev::Scroll(int touchpad_id, float x, float y) {
  if (touchpad_id < 0 || touchpad_id >= kTouchpads) {
    return EINVAL;
  }
  if ((x < -1.0f) || (x > 1.0f) || (y < -1.0f) || (y > 1.0f)) {
    return EINVAL;
  }
  Touchpad& touchpad = touchpad_[touchpad_id];
  if (!touchpad.injector) {
    return EvdevInjector::ERROR_SEQUENCING;
  }
  touchpad.injector->ResetError();
  const int32_t scaled_x = scale_relative_scroll(x);
  const int32_t scaled_y = scale_relative_scroll(y);
  ALOGV("(%f,%f) -> (%" PRId32 ",%" PRId32 ")", x, y, scaled_x, scaled_y);
  if (scaled_x) {
    touchpad.injector->SendRel(REL_HWHEEL, scaled_x);
  }
  if (scaled_y) {
    touchpad.injector->SendRel(REL_WHEEL, scaled_y);
  }
  if (scaled_x || scaled_y) {
    touchpad.injector->SendSynReport();
  }
  return touchpad.injector->GetError();
}

void VirtualTouchpadEvdev::dumpInternal(String8& result) {
  for (int i = 0; i < kTouchpads; ++i) {
    const auto& touchpad = touchpad_[i];
    result.appendFormat("[virtual touchpad %d]\n", i);
    if (!touchpad.injector) {
      result.append("injector = none\n");
      return;
    }
    result.appendFormat("injector = %s\n",
                        touchpad.owned_injector ? "normal" : "test");
    result.appendFormat("touches = %d\n", touchpad.touches);
    result.appendFormat("last_position = (%" PRId32 ", %" PRId32 ")\n",
                        touchpad.last_device_x, touchpad.last_device_y);
    result.appendFormat("last_buttons = 0x%" PRIX32 "\n",
                        touchpad.last_motion_event_buttons);
    touchpad.injector->dumpInternal(result);
    result.append("\n");
  }
}

}  // namespace dvr
}  // namespace android
