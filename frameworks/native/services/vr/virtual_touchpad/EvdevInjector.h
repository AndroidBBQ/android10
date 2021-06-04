#ifndef ANDROID_DVR_EVDEV_INJECTOR_H
#define ANDROID_DVR_EVDEV_INJECTOR_H

#include <android-base/unique_fd.h>
#include <linux/uinput.h>
#include <utils/String8.h>

#include <cstdint>
#include <memory>
#include <unordered_set>

namespace android {
namespace dvr {

// Simulated evdev input device.
//
class EvdevInjector {
 public:
  // EvdevInjector-specific error codes are negative integers; other non-zero
  // values returned from public routines are |errno| codes from underlying I/O.
  // EvdevInjector maintains a 'sticky' error state, similar to |errno|, so that
  // a caller can perform a sequence of operations and check for errors at the
  // end using |GetError()|. In general, the first such error will be recorded
  // and will suppress effects of further device operations until |ResetError()|
  // is called.
  //
  enum : int {
    ERROR_DEVICE_NAME = -1,     // Invalid device name.
    ERROR_PROPERTY_RANGE = -2,  // |INPUT_PROP_*| code out of range.
    ERROR_KEY_RANGE = -3,       // |KEY_*|/|BTN_*| code out of range.
    ERROR_ABS_RANGE = -4,       // |ABS_*| code out of range.
    ERROR_SEQUENCING = -5,      // Configure/Send out of order.
    ERROR_REL_RANGE = -6,       // |REL_*| code out of range.
  };

  // Key event |value| is not defined in <linux/input.h>.
  enum : int32_t { KEY_RELEASE = 0, KEY_PRESS = 1, KEY_REPEAT = 2 };

  // UInput provides a shim to intercept /dev/uinput operations
  // just above the system call level, for testing.
  //
  class UInput {
   public:
    UInput() {}
    virtual ~UInput() {}
    virtual int Open();
    virtual int Close();
    virtual int Write(const void* buf, size_t count);
    virtual int IoctlVoid(int request);
    virtual int IoctlSetInt(int request, int value);

   private:
    base::unique_fd fd_;
  };

  EvdevInjector() {}
  ~EvdevInjector() { Close(); }
  void Close();

  int GetError() const { return error_; }
  void ResetError() { error_ = 0; }

  // Configuration must be performed before sending any events.
  // |ConfigureBegin()| must be called first, and |ConfigureEnd()| last,
  // with zero or more other |Configure...()| calls in between in any order.

  // Configure the basic evdev device properties; must be called first.
  int ConfigureBegin(const char* device_name, int16_t bustype, int16_t vendor,
                     int16_t product, int16_t version);

  // Configure an optional input device property.
  // @param property  One of the |INPUT_PROP_*| constants from <linux/input.h>.
  int ConfigureInputProperty(int property);

  // Configure an input key.
  // @param key One of the |KEY_*| or |BTN_*| constants from <linux/input.h>.
  int ConfigureKey(uint16_t key);

  // Configure an absolute axis.
  // @param abs_type One of the |KEY_*| or |BTN_*| constants from
  // <linux/input.h>.
  int ConfigureAbs(uint16_t abs_type, int32_t min, int32_t max, int32_t fuzz,
                   int32_t flat);

  // Configure the number of multitouch slots.
  int ConfigureAbsSlots(int slots);

  // Configure multitouch coordinate range.
  int ConfigureMultiTouchXY(int32_t x0, int32_t y0, int32_t x1, int32_t y1);

  // Configure a relative axis.
  // @param rel_type One of the |REL_*| constants from <linux/input.h>.
  int ConfigureRel(uint16_t rel_type);

  // Complete configuration and create the input device.
  int ConfigureEnd();

  // Send various events.
  //
  int Send(uint16_t type, uint16_t code, int32_t value);
  int SendSynReport();
  int SendKey(uint16_t code, int32_t value);
  int SendAbs(uint16_t code, int32_t value);
  int SendRel(uint16_t code, int32_t value);
  int SendMultiTouchSlot(int32_t slot);
  int SendMultiTouchXY(int32_t slot, int32_t id, int32_t x, int32_t y);
  int SendMultiTouchLift(int32_t slot);

  void dumpInternal(String8& result);

 protected:
  // Must be called only between construction and ConfigureBegin().
  inline void SetUInputForTesting(UInput* uinput) { uinput_ = uinput; }
  // Caller must not retain pointer longer than EvdevInjector.
  inline const uinput_user_dev* GetUiDevForTesting() const { return &uidev_; }

 private:
  // Phase to enforce that configuration is complete before events are sent.
  enum class State { NEW, CONFIGURING, READY, CLOSED };

  // Sets |error_| if it is not already set; returns |code|.
  int Error(int code);

  // Returns a nonzero error if the injector is not in the required |state|.
  int RequireState(State state);

  // Configures an event type if necessary.
  // @param type One of the |EV_*| constants from <linux/input.h>.
  int EnableEventType(uint16_t type);

  // Active pointer to owned or testing UInput.
  UInput* uinput_ = nullptr;
  std::unique_ptr<UInput> owned_uinput_;

  State state_ = State::NEW;
  int error_ = 0;
  uinput_user_dev uidev_;
  std::unordered_set<uint16_t> enabled_event_types_;
  int32_t latest_slot_ = -1;

  EvdevInjector(const EvdevInjector&) = delete;
  void operator=(const EvdevInjector&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_EVDEV_INJECTOR_H
