#ifndef ANDROID_PDX_UDS_CHANNEL_PARCELABLE_H_
#define ANDROID_PDX_UDS_CHANNEL_PARCELABLE_H_

#include <pdx/channel_parcelable.h>
#include <pdx/file_handle.h>

namespace android {
namespace pdx {
namespace uds {

class ChannelParcelable : public pdx::ChannelParcelable {
 public:
  ChannelParcelable() = default;
  ChannelParcelable(LocalHandle data_fd, LocalHandle pollin_event_fd,
                    LocalHandle pollhup_event_fd);

  // Implements pdx::ChannelParcelable interface.
  bool IsValid() const override;
  LocalChannelHandle TakeChannelHandle() override;

  // Implements android::Parcelable interface.
  status_t writeToParcel(Parcel* parcel) const override;
  status_t readFromParcel(const Parcel* parcel) override;

 private:
  LocalHandle data_fd_;
  LocalHandle pollin_event_fd_;
  LocalHandle pollhup_event_fd_;
};

}  // namespace uds
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_UDS_CHANNEL_PARCELABLE_H_
