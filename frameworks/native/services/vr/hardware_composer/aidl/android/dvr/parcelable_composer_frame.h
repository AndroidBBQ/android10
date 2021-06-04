#ifndef ANDROID_DVR_HARDWARE_COMPOSER_AIDL_ANDROID_DVR_PARCELABLE_COMPOSER_FRAME_H
#define ANDROID_DVR_HARDWARE_COMPOSER_AIDL_ANDROID_DVR_PARCELABLE_COMPOSER_FRAME_H

#include <binder/Parcelable.h>
#include <impl/vr_hwc.h>

namespace android {
namespace dvr {

class ParcelableComposerFrame : public Parcelable {
 public:
  ParcelableComposerFrame();
  explicit ParcelableComposerFrame(const ComposerView::Frame& frame);
  ~ParcelableComposerFrame() override;

  ComposerView::Frame frame() const { return frame_; }

  status_t writeToParcel(Parcel* parcel) const override;
  status_t readFromParcel(const Parcel* parcel) override;

 private:
  ComposerView::Frame frame_;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_HARDWARE_COMPOSER_AIDL_ANDROID_DVR_PARCELABLE_COMPOSER_FRAME_H
