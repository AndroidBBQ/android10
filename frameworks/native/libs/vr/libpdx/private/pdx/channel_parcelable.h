#ifndef ANDROID_PDX_CHANNEL_PARCELABLE_H_
#define ANDROID_PDX_CHANNEL_PARCELABLE_H_

#include <binder/Parcelable.h>
#include <pdx/channel_handle.h>

namespace android {
namespace pdx {

/**
 * A parcelable object holds all necessary objects to recreate a ClientChannel.
 * In addition to the android::Parcelable interface, this interface exposees
 * more PDX-related interface.
 */
class ChannelParcelable : public Parcelable {
 public:
  virtual ~ChannelParcelable() = default;

  // Returns whether the parcelable object holds a valid client channel.
  virtual bool IsValid() const = 0;

  // Returns a channel handle constructed from this parcelable object and takes
  // the ownership of all resources from the parcelable object. In another word,
  // the parcelable object will become invalid after TakeChannelHandle returns.
  virtual LocalChannelHandle TakeChannelHandle() = 0;
};

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_CHANNEL_PARCELABLE_H_
