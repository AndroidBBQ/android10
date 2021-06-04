#ifndef ANDROID_DVR_HARDWARE_COMPOSER_VR_COMPOSER_H
#define ANDROID_DVR_HARDWARE_COMPOSER_VR_COMPOSER_H

#include <android/dvr/BnVrComposer.h>
#include <impl/vr_hwc.h>

namespace android {
namespace dvr {

class VrComposerCallback;

// Implementation of the IVrComposer service used to notify VR Window Manager
// when SurfaceFlinger presents 2D UI changes.
//
// VR HWC updates the presented frame via the ComposerView::Observer interface.
// On notification |callback_| is called to update VR Window Manager.
// NOTE: If VR Window Manager isn't connected, the notification is a no-op.
class VrComposer
    : public BnVrComposer,
      public ComposerView::Observer,
      public IBinder::DeathRecipient {
 public:
  explicit VrComposer(ComposerView* composer_view);
  ~VrComposer() override;

  // BnVrComposer:
  binder::Status registerObserver(
      const sp<IVrComposerCallback>& callback) override;

  binder::Status clearObserver() override;

  // ComposerView::Observer:
  base::unique_fd OnNewFrame(const ComposerView::Frame& frame) override;

 private:
  // IBinder::DeathRecipient:
  void binderDied(const wp<IBinder>& who) override;

  std::mutex mutex_;

  sp<IVrComposerCallback> callback_;

  ComposerView* composer_view_;  // Not owned.

  VrComposer(const VrComposer&) = delete;
  void operator=(const VrComposer&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  //  ANDROID_DVR_HARDWARE_COMPOSER_VR_COMPOSER_H
