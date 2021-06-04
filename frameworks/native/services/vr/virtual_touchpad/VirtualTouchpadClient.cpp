#include "VirtualTouchpadClient.h"

#include <android/dvr/IVirtualTouchpadService.h>
#include <binder/IServiceManager.h>

namespace android {
namespace dvr {

namespace {

class VirtualTouchpadClientImpl : public VirtualTouchpadClient {
 public:
  VirtualTouchpadClientImpl() {}
  ~VirtualTouchpadClientImpl() override {
    if (service_ != nullptr) {
      Detach();
    }
  }

  status_t Attach() {
    if (service_ != nullptr) {
      return ALREADY_EXISTS;
    }
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == nullptr) {
      ALOGE("no service manager");
      return NO_INIT;
    }
    sp<IVirtualTouchpadService> service =
        interface_cast<IVirtualTouchpadService>(
            sm->getService(IVirtualTouchpadService::SERVICE_NAME()));
    if (service == nullptr) {
      ALOGE("failed to get service");
      return NAME_NOT_FOUND;
    }
    service_ = service;
    return service_->attach().transactionError();
  }

  status_t Detach() {
    if (service_ == nullptr) {
      return NO_INIT;
    }
    status_t status = service_->detach().transactionError();
    service_ = nullptr;
    return status;
  }

  status_t Touch(int touchpad, float x, float y, float pressure) override {
    if (service_ == nullptr) {
      return NO_INIT;
    }
    return service_->touch(touchpad, x, y, pressure).transactionError();
  }

  status_t ButtonState(int touchpad, int buttons) override {
    if (service_ == nullptr) {
      return NO_INIT;
    }
    return service_->buttonState(touchpad, buttons).transactionError();
  }

  status_t Scroll(int touchpad, float x, float y) override {
    if (service_ == nullptr) {
      return NO_INIT;
    }
    return service_->scroll(touchpad, x, y).transactionError();
  }

  void dumpInternal(String8& result) override {
    result.append("[virtual touchpad]\n");
    result.appendFormat("connected = %s\n\n",
                        service_ != nullptr ? "true" : "false");
  }

 private:
  sp<IVirtualTouchpadService> service_;
};

}  // anonymous namespace

std::unique_ptr<VirtualTouchpad> VirtualTouchpadClient::Create() {
  return std::unique_ptr<VirtualTouchpad>(new VirtualTouchpadClientImpl());
}

}  // namespace dvr
}  // namespace android
