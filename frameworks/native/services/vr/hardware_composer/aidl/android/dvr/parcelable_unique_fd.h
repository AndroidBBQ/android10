#ifndef ANDROID_DVR_HARDWARE_COMPOSER_AIDL_ANDROID_DVR_PARCELABLE_UNIQUE_FD_H
#define ANDROID_DVR_HARDWARE_COMPOSER_AIDL_ANDROID_DVR_PARCELABLE_UNIQUE_FD_H

#include <android-base/unique_fd.h>
#include <binder/Parcelable.h>

namespace android {
namespace dvr {

// Provide a wrapper to serialized base::unique_fd. The wrapper also handles the
// case where the FD is invalid (-1), unlike FileDescriptor which expects a
// valid FD.
class ParcelableUniqueFd : public Parcelable {
 public:
  ParcelableUniqueFd();
  explicit ParcelableUniqueFd(const base::unique_fd& fence);
  ~ParcelableUniqueFd() override;

  void set_fence(const base::unique_fd& fence) {
    fence_.reset(dup(fence.get()));
  }
  base::unique_fd fence() const { return base::unique_fd(dup(fence_.get())); }

  status_t writeToParcel(Parcel* parcel) const override;
  status_t readFromParcel(const Parcel* parcel) override;

 private:
  base::unique_fd fence_;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_HARDWARE_COMPOSER_AIDL_ANDROID_DVR_PARCELABLE_UNIQUE_FD_H
