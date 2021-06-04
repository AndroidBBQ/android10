#include <uds/channel_manager.h>

#include <log/log.h>

namespace android {
namespace pdx {
namespace uds {

ChannelManager& ChannelManager::Get() {
  static ChannelManager instance;
  return instance;
}

void ChannelManager::CloseHandle(int32_t handle) {
  std::lock_guard<std::mutex> autolock(mutex_);
  auto channel = channels_.find(handle);
  if (channel == channels_.end()) {
    ALOGE("Invalid channel handle: %d", handle);
  } else {
    channels_.erase(channel);
  }
}

LocalChannelHandle ChannelManager::CreateHandle(LocalHandle data_fd,
                                                LocalHandle pollin_event_fd,
                                                LocalHandle pollhup_event_fd) {
  if (data_fd && pollin_event_fd && pollhup_event_fd) {
    std::lock_guard<std::mutex> autolock(mutex_);
    const int32_t handle = data_fd.Get();
    channels_.emplace(
        handle,
        ChannelEventReceiver{std::move(data_fd), std::move(pollin_event_fd),
                             std::move(pollhup_event_fd)});
    return LocalChannelHandle(this, handle);
  } else {
    ALOGE(
        "ChannelManager::CreateHandle: Invalid arguments: data_fd=%d "
        "pollin_event_fd=%d pollhup_event_fd=%d",
        data_fd.Get(), pollin_event_fd.Get(), pollhup_event_fd.Get());
    return LocalChannelHandle(nullptr, -1);
  }
}

ChannelEventReceiver* ChannelManager::GetChannelData(int32_t handle) {
  std::lock_guard<std::mutex> autolock(mutex_);
  auto channel = channels_.find(handle);
  return channel != channels_.end() ? &channel->second : nullptr;
}

}  // namespace uds
}  // namespace pdx
}  // namespace android
