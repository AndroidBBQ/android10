#include "include/private/dvr/buffer_hub_queue_parcelable.h"

#include <binder/Parcel.h>
#include <pdx/default_transport/channel_parcelable.h>

namespace android {
namespace dvr {

template <BufferHubQueueParcelableMagic Magic>
bool BufferHubQueueParcelable<Magic>::IsValid() const {
  return !!channel_parcelable_ && channel_parcelable_->IsValid();
}

template <BufferHubQueueParcelableMagic Magic>
pdx::LocalChannelHandle BufferHubQueueParcelable<Magic>::TakeChannelHandle() {
  if (!IsValid()) {
    ALOGE(
        "BufferHubQueueParcelable::TakeChannelHandle: Invalid channel parcel.");
    return {};  // Returns an empty channel handle.
  }

  // Take channel handle out of the parcelable and reset the parcelable.
  pdx::LocalChannelHandle handle = channel_parcelable_->TakeChannelHandle();
  // Now channel_parcelable_ should already be invalid, but reset it to release
  // the invalid parcelable object from unique_ptr.
  channel_parcelable_ = nullptr;
  return handle;
}

template <BufferHubQueueParcelableMagic Magic>
status_t BufferHubQueueParcelable<Magic>::writeToParcel(Parcel* parcel) const {
  if (!IsValid()) {
    ALOGE("BufferHubQueueParcelable::writeToParcel: Invalid channel.");
    return -EINVAL;
  }

  status_t res = parcel->writeUint32(Magic);
  if (res != OK) {
    ALOGE("BufferHubQueueParcelable::writeToParcel: Cannot write magic.");
    return res;
  }

  return channel_parcelable_->writeToParcel(parcel);
}

template <BufferHubQueueParcelableMagic Magic>
status_t BufferHubQueueParcelable<Magic>::readFromParcel(const Parcel* parcel) {
  if (IsValid()) {
    ALOGE(
        "BufferHubQueueParcelable::readFromParcel: This parcelable object has "
        "been initialized already.");
    return -EINVAL;
  }

  uint32_t out_magic = 0;
  status_t res = OK;

  res = parcel->readUint32(&out_magic);
  if (res != OK)
    return res;

  if (out_magic != Magic) {
    ALOGE(
        "BufferHubQueueParcelable::readFromParcel: Unexpected magic: 0x%x, "
        "epxected: 0x%x",
        out_magic, Magic);
    return -EINVAL;
  }

  // (Re)Alocate channel parcelable object.
  channel_parcelable_ =
      std::make_unique<pdx::default_transport::ChannelParcelable>();
  return channel_parcelable_->readFromParcel(parcel);
}

template class BufferHubQueueParcelable<
    BufferHubQueueParcelableMagic::Producer>;
template class BufferHubQueueParcelable<
    BufferHubQueueParcelableMagic::Consumer>;

}  // namespace dvr
}  // namespace android
