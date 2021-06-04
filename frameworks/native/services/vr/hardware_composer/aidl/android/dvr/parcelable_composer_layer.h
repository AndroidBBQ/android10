#ifndef ANDROID_DVR_HARDWARE_COMPOSER_AIDL_ANDROID_DVR_PARCELABLE_COMPOSER_LAYER_H
#define ANDROID_DVR_HARDWARE_COMPOSER_AIDL_ANDROID_DVR_PARCELABLE_COMPOSER_LAYER_H

#include <binder/Parcelable.h>
#include <impl/vr_hwc.h>

#include <memory>

namespace android {
namespace dvr {

class ParcelableComposerLayer : public Parcelable {
 public:
  ParcelableComposerLayer();
  explicit ParcelableComposerLayer(const ComposerView::ComposerLayer& layer);
  ~ParcelableComposerLayer() override;

  ComposerView::ComposerLayer layer() const { return layer_; }

  status_t writeToParcel(Parcel* parcel) const override;
  status_t readFromParcel(const Parcel* parcel) override;

 private:
  ComposerView::ComposerLayer layer_;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_HARDWARE_COMPOSER_AIDL_ANDROID_DVR_PARCELABLE_COMPOSER_LAYER_H
