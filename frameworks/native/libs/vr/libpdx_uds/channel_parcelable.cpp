#include "uds/channel_parcelable.h"

#include <binder/Parcel.h>
#include <uds/channel_manager.h>

namespace android {
namespace pdx {
namespace uds {

namespace {

static constexpr uint32_t kUdsMagicParcelHeader = 0x7564736d;  // 'udsm'.

}  // namespace

ChannelParcelable::ChannelParcelable(LocalHandle data_fd,
                                     LocalHandle pollin_event_fd,
                                     LocalHandle pollhup_event_fd)
    : data_fd_{std::move(data_fd)},
      pollin_event_fd_{std::move(pollin_event_fd)},
      pollhup_event_fd_{std::move(pollhup_event_fd)} {}

bool ChannelParcelable::IsValid() const {
  return !!data_fd_ && !!pollin_event_fd_ && !!pollhup_event_fd_;
}

LocalChannelHandle ChannelParcelable::TakeChannelHandle() {
  if (!IsValid()) {
    ALOGE("ChannelParcelable::TakeChannelHandle: Invalid channel parcel.");
    return {};  // Returns an empty channel handle.
  }

  return ChannelManager::Get().CreateHandle(std::move(data_fd_),
                                            std::move(pollin_event_fd_),
                                            std::move(pollhup_event_fd_));
}

status_t ChannelParcelable::writeToParcel(Parcel* parcel) const {
  status_t res = OK;

  if (!IsValid()) {
    ALOGE("ChannelParcelable::writeToParcel: Invalid channel parcel.");
    return BAD_VALUE;
  }

  res = parcel->writeUint32(kUdsMagicParcelHeader);
  if (res != OK) {
    ALOGE("ChannelParcelable::writeToParcel: Cannot write magic: res=%d.", res);
    return res;
  }

  res = parcel->writeFileDescriptor(data_fd_.Get());
  if (res != OK) {
    ALOGE("ChannelParcelable::writeToParcel: Cannot write data fd: res=%d.",
          res);
    return res;
  }

  res = parcel->writeFileDescriptor(pollin_event_fd_.Get());
  if (res != OK) {
    ALOGE(
        "ChannelParcelable::writeToParcel: Cannot write pollin event fd: "
        "res=%d.",
        res);
    return res;
  }

  res = parcel->writeFileDescriptor(pollhup_event_fd_.Get());
  if (res != OK) {
    ALOGE(
        "ChannelParcelable::writeToParcel: Cannot write pollhup event fd: "
        "res=%d.",
        res);
    return res;
  }

  return res;
}

status_t ChannelParcelable::readFromParcel(const Parcel* parcel) {
  uint32_t magic = 0;
  status_t res = OK;

  if (IsValid()) {
    ALOGE(
        "ChannelParcelable::readFromParcel: This channel parcel is already "
        "initailzied.");
    return ALREADY_EXISTS;
  }

  res = parcel->readUint32(&magic);
  if (res != OK) {
    ALOGE("ChannelParcelable::readFromParcel: Failed to read magic: res=%d.",
          res);
    return res;
  }

  if (magic != kUdsMagicParcelHeader) {
    ALOGE(
        "ChannelParcelable::readFromParcel: Unknown magic: 0x%x, epxected: "
        "0x%x",
        magic, kUdsMagicParcelHeader);
    return BAD_VALUE;
  }

  // TODO(b/69010509): We have to dup() the FD from android::Parcel as it
  // doesn't support taking out the FD's ownership. We can remove the dup() here
  // once android::Parcel support such operation.
  data_fd_.Reset(dup(parcel->readFileDescriptor()));
  pollin_event_fd_.Reset(dup(parcel->readFileDescriptor()));
  pollhup_event_fd_.Reset(dup(parcel->readFileDescriptor()));
  if (!IsValid()) {
    ALOGE(
        "ChannelParcelable::readFromParcel: Cannot read fd from parcel: "
        "data_fd=%d, pollin_event_fd=%d, pollhup_event_fd=%d.",
        data_fd_.Get(), pollin_event_fd_.Get(), pollhup_event_fd_.Get());
    return DEAD_OBJECT;
  }

  return res;
}

}  // namespace uds
}  // namespace pdx
}  // namespace android
