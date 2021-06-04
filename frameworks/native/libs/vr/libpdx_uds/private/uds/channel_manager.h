#ifndef ANDROID_PDX_UDS_CHANNEL_MANAGER_H_
#define ANDROID_PDX_UDS_CHANNEL_MANAGER_H_

#include <mutex>
#include <unordered_map>

#include <pdx/channel_handle.h>
#include <pdx/file_handle.h>
#include <uds/channel_event_set.h>

namespace android {
namespace pdx {
namespace uds {

class ChannelManager : public ChannelManagerInterface {
 public:
  static ChannelManager& Get();

  LocalChannelHandle CreateHandle(LocalHandle data_fd,
                                  LocalHandle pollin_event_fd,
                                  LocalHandle pollhup_event_fd);

  ChannelEventReceiver* GetChannelData(int32_t handle);

 private:
  ChannelManager() = default;

  void CloseHandle(int32_t handle) override;

  std::mutex mutex_;
  std::unordered_map<int32_t, ChannelEventReceiver> channels_;
};

}  // namespace uds
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_UDS_CHANNEL_MANAGER_H_
