#include "EvdevInjector.h"

#include <errno.h>
#include <inttypes.h>
#include <linux/input.h>
#include <log/log.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>

namespace android {
namespace dvr {

int EvdevInjector::UInput::Open() {
  errno = 0;
  fd_.reset(open("/dev/uinput", O_WRONLY | O_NONBLOCK));
  if (fd_.get() < 0) {
    ALOGE("couldn't open uinput (r=%d errno=%d)", fd_.get(), errno);
  }
  return errno;
}

int EvdevInjector::UInput::Close() {
  errno = 0;
  fd_.reset();
  return errno;
}

int EvdevInjector::UInput::Write(const void* buf, size_t count) {
  ALOGV("UInput::Write(%zu, %02X...)", count, *static_cast<const char*>(buf));
  errno = 0;
  ssize_t r = write(fd_.get(), buf, count);
  if (r != static_cast<ssize_t>(count)) {
    ALOGE("write(%zu) failed (r=%zd errno=%d)", count, r, errno);
  }
  return errno;
}

int EvdevInjector::UInput::IoctlSetInt(int request, int value) {
  ALOGV("UInput::IoctlSetInt(0x%X, 0x%X)", request, value);
  errno = 0;
  if (const int status = ioctl(fd_.get(), request, value)) {
    ALOGE("ioctl(%d, 0x%X, 0x%X) failed (r=%d errno=%d)", fd_.get(), request,
          value, status, errno);
  }
  return errno;
}

int EvdevInjector::UInput::IoctlVoid(int request) {
  ALOGV("UInput::IoctlVoid(0x%X)", request);
  errno = 0;
  if (const int status = ioctl(fd_.get(), request)) {
    ALOGE("ioctl(%d, 0x%X) failed (r=%d errno=%d)", fd_.get(), request, status,
          errno);
  }
  return errno;
}

void EvdevInjector::Close() {
  uinput_->Close();
  state_ = State::CLOSED;
}

int EvdevInjector::ConfigureBegin(const char* device_name, int16_t bustype,
                                  int16_t vendor, int16_t product,
                                  int16_t version) {
  ALOGV("ConfigureBegin %s 0x%04" PRIX16 " 0x%04" PRIX16 " 0x%04" PRIX16
        " 0x%04" PRIX16 "",
        device_name, bustype, vendor, product, version);
  if (!device_name || strlen(device_name) >= UINPUT_MAX_NAME_SIZE) {
    return Error(ERROR_DEVICE_NAME);
  }
  if (const int status = RequireState(State::NEW)) {
    return status;
  }
  if (!uinput_) {
    owned_uinput_.reset(new EvdevInjector::UInput());
    uinput_ = owned_uinput_.get();
  }
  if (const int status = uinput_->Open()) {
    // Without uinput we're dead in the water.
    state_ = State::CLOSED;
    return Error(status);
  }
  state_ = State::CONFIGURING;
  // Initialize device setting structure.
  memset(&uidev_, 0, sizeof(uidev_));
  strncpy(uidev_.name, device_name, UINPUT_MAX_NAME_SIZE);
  uidev_.id.bustype = bustype;
  uidev_.id.vendor = vendor;
  uidev_.id.product = product;
  uidev_.id.version = version;
  return 0;
}

int EvdevInjector::ConfigureInputProperty(int property) {
  ALOGV("ConfigureInputProperty %d", property);
  if (property < 0 || property >= INPUT_PROP_CNT) {
    ALOGE("property 0x%X out of range [0,0x%X)", property, INPUT_PROP_CNT);
    return Error(ERROR_PROPERTY_RANGE);
  }
  if (const int status = RequireState(State::CONFIGURING)) {
    return status;
  }
  if (const int status = uinput_->IoctlSetInt(UI_SET_PROPBIT, property)) {
    ALOGE("failed to set property %d", property);
    return Error(status);
  }
  return 0;
}

int EvdevInjector::ConfigureKey(uint16_t key) {
  ALOGV("ConfigureKey 0x%02" PRIX16 "", key);
  if (key < 0 || key >= KEY_CNT) {
    ALOGE("key 0x%X out of range [0,0x%X)", key, KEY_CNT);
    return Error(ERROR_KEY_RANGE);
  }
  if (const int status = RequireState(State::CONFIGURING)) {
    return status;
  }
  if (const int status = EnableEventType(EV_KEY)) {
    return status;
  }
  if (const int status = uinput_->IoctlSetInt(UI_SET_KEYBIT, key)) {
    ALOGE("failed to enable EV_KEY 0x%02" PRIX16 "", key);
    return Error(status);
  }
  return 0;
}

int EvdevInjector::ConfigureAbs(uint16_t abs_type, int32_t min, int32_t max,
                                int32_t fuzz, int32_t flat) {
  ALOGV("ConfigureAbs 0x%" PRIX16 " %" PRId32 " %" PRId32 " %" PRId32
        " %" PRId32 "",
        abs_type, min, max, fuzz, flat);
  if (abs_type < 0 || abs_type >= ABS_CNT) {
    ALOGE("EV_ABS type 0x%" PRIX16 " out of range [0,0x%X)", abs_type, ABS_CNT);
    return Error(ERROR_ABS_RANGE);
  }
  if (const int status = RequireState(State::CONFIGURING)) {
    return status;
  }
  if (const int status = EnableEventType(EV_ABS)) {
    return status;
  }
  if (const int status = uinput_->IoctlSetInt(UI_SET_ABSBIT, abs_type)) {
    ALOGE("failed to enable EV_ABS 0x%" PRIX16 "", abs_type);
    return Error(status);
  }
  uidev_.absmin[abs_type] = min;
  uidev_.absmax[abs_type] = max;
  uidev_.absfuzz[abs_type] = fuzz;
  uidev_.absflat[abs_type] = flat;
  return 0;
}

int EvdevInjector::ConfigureMultiTouchXY(int x0, int y0, int x1, int y1) {
  if (const int status = ConfigureAbs(ABS_MT_POSITION_X, x0, x1, 0, 0)) {
    return status;
  }
  if (const int status = ConfigureAbs(ABS_MT_POSITION_Y, y0, y1, 0, 0)) {
    return status;
  }
  return 0;
}

int EvdevInjector::ConfigureAbsSlots(int slots) {
  return ConfigureAbs(ABS_MT_SLOT, 0, slots, 0, 0);
}

int EvdevInjector::ConfigureRel(uint16_t rel_type) {
  ALOGV("ConfigureRel 0x%" PRIX16 "", rel_type);
  if (rel_type < 0 || rel_type >= REL_CNT) {
    ALOGE("EV_REL type 0x%" PRIX16 " out of range [0,0x%X)", rel_type, REL_CNT);
    return Error(ERROR_REL_RANGE);
  }
  if (const int status = RequireState(State::CONFIGURING)) {
    return status;
  }
  if (const int status = EnableEventType(EV_REL)) {
    return status;
  }
  if (const int status = uinput_->IoctlSetInt(UI_SET_RELBIT, rel_type)) {
    ALOGE("failed to enable EV_REL 0x%" PRIX16 "", rel_type);
    return Error(status);
  }
  return 0;
}

int EvdevInjector::ConfigureEnd() {
  ALOGV("ConfigureEnd:");
  ALOGV("  name=\"%s\"", uidev_.name);
  ALOGV("  id.bustype=0x%04" PRIX16, uidev_.id.bustype);
  ALOGV("  id.vendor=0x%04" PRIX16, uidev_.id.vendor);
  ALOGV("  id.product=0x%04" PRIX16, uidev_.id.product);
  ALOGV("  id.version=0x%04" PRIX16, uidev_.id.version);
  ALOGV("  ff_effects_max=%" PRIu32, uidev_.ff_effects_max);
  for (int i = 0; i < ABS_CNT; ++i) {
    if (uidev_.absmin[i]) {
      ALOGV("  absmin[%d]=%" PRId32, i, uidev_.absmin[i]);
    }
    if (uidev_.absmax[i]) {
      ALOGV("  absmax[%d]=%" PRId32, i, uidev_.absmax[i]);
    }
    if (uidev_.absfuzz[i]) {
      ALOGV("  absfuzz[%d]=%" PRId32, i, uidev_.absfuzz[i]);
    }
    if (uidev_.absflat[i]) {
      ALOGV("  absflat[%d]=%" PRId32, i, uidev_.absflat[i]);
    }
  }

  if (const int status = RequireState(State::CONFIGURING)) {
    return status;
  }
  // Write out device settings.
  if (const int status = uinput_->Write(&uidev_, sizeof uidev_)) {
    ALOGE("failed to write device settings");
    return Error(status);
  }
  // Create device node.
  if (const int status = uinput_->IoctlVoid(UI_DEV_CREATE)) {
    ALOGE("failed to create device node");
    return Error(status);
  }
  state_ = State::READY;
  return 0;
}

int EvdevInjector::Send(uint16_t type, uint16_t code, int32_t value) {
  ALOGV("Send(0x%" PRIX16 ", 0x%" PRIX16 ", 0x%" PRIX32 ")", type, code, value);
  if (const int status = RequireState(State::READY)) {
    return status;
  }
  struct input_event event;
  memset(&event, 0, sizeof(event));
  event.type = type;
  event.code = code;
  event.value = value;
  if (const int status = uinput_->Write(&event, sizeof(event))) {
    ALOGE("failed to write event 0x%" PRIX16 ", 0x%" PRIX16 ", 0x%" PRIX32,
          type, code, value);
    return Error(status);
  }
  return 0;
}

int EvdevInjector::SendSynReport() { return Send(EV_SYN, SYN_REPORT, 0); }

int EvdevInjector::SendKey(uint16_t code, int32_t value) {
  return Send(EV_KEY, code, value);
}

int EvdevInjector::SendAbs(uint16_t code, int32_t value) {
  return Send(EV_ABS, code, value);
}

int EvdevInjector::SendRel(uint16_t code, int32_t value) {
  return Send(EV_REL, code, value);
}

int EvdevInjector::SendMultiTouchSlot(int32_t slot) {
  if (latest_slot_ != slot) {
    if (const int status = SendAbs(ABS_MT_SLOT, slot)) {
      return status;
    }
    latest_slot_ = slot;
  }
  return 0;
}

int EvdevInjector::SendMultiTouchXY(int32_t slot, int32_t id, int32_t x,
                                    int32_t y) {
  if (const int status = SendMultiTouchSlot(slot)) {
    return status;
  }
  if (const int status = SendAbs(ABS_MT_TRACKING_ID, id)) {
    return status;
  }
  if (const int status = SendAbs(ABS_MT_POSITION_X, x)) {
    return status;
  }
  if (const int status = SendAbs(ABS_MT_POSITION_Y, y)) {
    return status;
  }
  return 0;
}

int EvdevInjector::SendMultiTouchLift(int32_t slot) {
  if (const int status = SendMultiTouchSlot(slot)) {
    return status;
  }
  if (const int status = SendAbs(ABS_MT_TRACKING_ID, -1)) {
    return status;
  }
  return 0;
}

int EvdevInjector::Error(int code) {
  if (!error_) {
    error_ = code;
  }
  return code;
}

int EvdevInjector::RequireState(State required_state) {
  if (error_) {
    return error_;
  }
  if (state_ != required_state) {
    ALOGE("in state %d but require state %d", static_cast<int>(state_),
          static_cast<int>(required_state));
    return Error(ERROR_SEQUENCING);
  }
  return 0;
}

int EvdevInjector::EnableEventType(uint16_t type) {
  if (const int status = RequireState(State::CONFIGURING)) {
    return status;
  }
  if (enabled_event_types_.count(type) > 0) {
    return 0;
  }
  if (const int status = uinput_->IoctlSetInt(UI_SET_EVBIT, type)) {
    ALOGE("failed to enable event type 0x%X", type);
    return Error(status);
  }
  enabled_event_types_.insert(type);
  return 0;
}

void EvdevInjector::dumpInternal(String8& result) {
  result.appendFormat("injector_state = %d\n", static_cast<int>(state_));
  result.appendFormat("injector_error = %d\n", error_);
}

}  // namespace dvr
}  // namespace android
