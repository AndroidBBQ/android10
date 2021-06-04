#include "android/dvr/parcelable_unique_fd.h"

#include <binder/Parcel.h>

namespace android {
namespace dvr {

ParcelableUniqueFd::ParcelableUniqueFd() {}

ParcelableUniqueFd::ParcelableUniqueFd(const base::unique_fd& fence)
    : fence_(dup(fence.get())) {}

ParcelableUniqueFd::~ParcelableUniqueFd() {}

status_t ParcelableUniqueFd::writeToParcel(Parcel* parcel) const {
  status_t ret = parcel->writeBool(fence_.get() >= 0);
  if (ret != OK) return ret;

  if (fence_.get() >= 0)
    ret = parcel->writeUniqueFileDescriptor(fence_);

  return ret;
}

status_t ParcelableUniqueFd::readFromParcel(const Parcel* parcel) {
  bool has_fence = 0;
  status_t ret = parcel->readBool(&has_fence);
  if (ret != OK) return ret;

  if (has_fence)
    ret = parcel->readUniqueFileDescriptor(&fence_);

  return ret;
}

}  // namespace dvr
}  // namespace android
