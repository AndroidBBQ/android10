#ifndef ANDROID_DVR_VR_FLINGER_H_
#define ANDROID_DVR_VR_FLINGER_H_

#include <thread>
#include <memory>

#define HWC2_INCLUDE_STRINGIFICATION
#define HWC2_USE_CPP11
#include <hardware/hwcomposer2.h>
#undef HWC2_INCLUDE_STRINGIFICATION
#undef HWC2_USE_CPP11

#include <pdx/service_dispatcher.h>
#include <vr/vr_manager/vr_manager.h>

namespace android {

namespace Hwc2 {
class Composer;
}  // namespace Hwc2

namespace dvr {

class DisplayService;

class VrFlinger {
 public:
  using RequestDisplayCallback = std::function<void(bool)>;
  static std::unique_ptr<VrFlinger> Create(
      Hwc2::Composer* hidl,
      hwc2_display_t primary_display_id,
      RequestDisplayCallback request_display_callback);
  ~VrFlinger();

  // These functions are all called on surface flinger's main thread.
  void OnBootFinished();
  void GrantDisplayOwnership();
  void SeizeDisplayOwnership();

  // dump all vr flinger state.
  std::string Dump();

 private:
  VrFlinger();
  bool Init(Hwc2::Composer* hidl,
            hwc2_display_t primary_display_id,
            RequestDisplayCallback request_display_callback);

  // Needs to be a separate class for binder's ref counting
  class PersistentVrStateCallback : public BnPersistentVrStateCallbacks {
   public:
    explicit PersistentVrStateCallback(
        RequestDisplayCallback request_display_callback)
        : request_display_callback_(request_display_callback) {}
    void onPersistentVrStateChanged(bool enabled) override;
   private:
    RequestDisplayCallback request_display_callback_;
  };

  std::thread dispatcher_thread_;
  std::unique_ptr<android::pdx::ServiceDispatcher> dispatcher_;
  std::shared_ptr<android::dvr::DisplayService> display_service_;
  sp<PersistentVrStateCallback> persistent_vr_state_callback_;
  RequestDisplayCallback request_display_callback_;
};

} // namespace dvr
} // namespace android

#endif // ANDROID_DVR_VR_FLINGER_H_
