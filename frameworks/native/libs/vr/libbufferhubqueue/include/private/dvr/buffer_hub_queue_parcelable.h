#ifndef ANDROID_DVR_BUFFER_HUB_QUEUE_PARCELABLE_H_
#define ANDROID_DVR_BUFFER_HUB_QUEUE_PARCELABLE_H_

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

// The following headers are included without checking every warning.
// TODO(b/72172820): Remove the workaround once we have enforced -Weverything
// in these headers and their dependencies.
#include <pdx/channel_parcelable.h>

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

namespace android {
namespace dvr {

enum BufferHubQueueParcelableMagic : uint32_t {
  Producer = 0x62687170,  // 'bhqp'
  Consumer = 0x62687163,  // 'bhqc'
};

template <BufferHubQueueParcelableMagic Magic>
class BufferHubQueueParcelable : public Parcelable {
 public:
  BufferHubQueueParcelable() = default;

  BufferHubQueueParcelable(BufferHubQueueParcelable&& other) noexcept = default;
  BufferHubQueueParcelable& operator=(BufferHubQueueParcelable&& other) noexcept {
    channel_parcelable_ = std::move(other.channel_parcelable_);
    return *this;
  }

  // Constructs an parcelable contains the channel parcelable.
  explicit BufferHubQueueParcelable(
      std::unique_ptr<pdx::ChannelParcelable> channel_parcelable)
      : channel_parcelable_(std::move(channel_parcelable)) {}

  BufferHubQueueParcelable(const BufferHubQueueParcelable&) = delete;
  void operator=(const BufferHubQueueParcelable&) = delete;

  bool IsValid() const;

  // Returns a channel handle constructed from this parcelable object and takes
  // the ownership of all resources from the parcelable object.
  pdx::LocalChannelHandle TakeChannelHandle();

  // Serializes the queue parcelable into the given parcel. Note that no system
  // resources are getting duplicated, nor did the parcel takes ownership of the
  // queue parcelable. Thus, the parcelable object must remain valid for the
  // lifetime of the parcel.
  status_t writeToParcel(Parcel* parcel) const override;

  // Deserialize the queue parcelable from the given parcel. Note that system
  // resources are duplicated from the parcel into the queue parcelable. Returns
  // error if the targeting parcelable object is already valid.
  status_t readFromParcel(const Parcel* parcel) override;

 private:
  std::unique_ptr<pdx::ChannelParcelable> channel_parcelable_;
};

using ProducerQueueParcelable =
    BufferHubQueueParcelable<BufferHubQueueParcelableMagic::Producer>;
using ConsumerQueueParcelable =
    BufferHubQueueParcelable<BufferHubQueueParcelableMagic::Consumer>;

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_BUFFER_HUB_QUEUE_PARCELABLE_H_
