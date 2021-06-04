#ifndef ANDROID_PDX_UDS_CHANNEL_EVENT_SET_H_
#define ANDROID_PDX_UDS_CHANNEL_EVENT_SET_H_

#include <vector>

#include <pdx/client_channel.h>
#include <pdx/file_handle.h>
#include <pdx/status.h>

namespace android {
namespace pdx {
namespace uds {

class ChannelEventSet {
 public:
  ChannelEventSet();
  ChannelEventSet(ChannelEventSet&&) = default;
  ChannelEventSet& operator=(ChannelEventSet&&) = default;

  BorrowedHandle pollin_event_fd() const { return pollin_event_fd_.Borrow(); }
  BorrowedHandle pollhup_event_fd() const { return pollhup_event_fd_.Borrow(); }

  explicit operator bool() const {
    return !!pollin_event_fd_ && !!pollhup_event_fd_;
  }

  int ModifyEvents(int clear_mask, int set_mask);

 private:
  LocalHandle pollin_event_fd_;
  LocalHandle pollhup_event_fd_;
  uint32_t event_bits_ = 0;

  ChannelEventSet(const ChannelEventSet&) = delete;
  void operator=(const ChannelEventSet&) = delete;
};

class ChannelEventReceiver {
 public:
  ChannelEventReceiver() = default;
  ChannelEventReceiver(LocalHandle data_fd, LocalHandle pollin_event_fd,
                       LocalHandle pollhup_event_fd);
  ChannelEventReceiver(ChannelEventReceiver&&) = default;
  ChannelEventReceiver& operator=(ChannelEventReceiver&&) = default;

  explicit operator bool() const {
    return !!pollin_event_fd_ && !!pollhup_event_fd_ && !!data_fd_ &&
           !!epoll_fd_;
  }

  BorrowedHandle event_fd() const { return epoll_fd_.Borrow(); }

  BorrowedHandle pollin_event_fd() const { return pollin_event_fd_.Borrow(); }
  BorrowedHandle pollhup_event_fd() const { return pollhup_event_fd_.Borrow(); }
  BorrowedHandle data_fd() const { return data_fd_.Borrow(); }

  // Moves file descriptors out of ChannelEventReceiver. Note these operations
  // immediately invalidates the receiver.
  std::tuple<LocalHandle, LocalHandle, LocalHandle> TakeFds() {
    epoll_fd_.Close();
    return {std::move(data_fd_), std::move(pollin_event_fd_),
            std::move(pollhup_event_fd_)};
  }

  Status<int> GetPendingEvents() const;
  Status<int> PollPendingEvents(int timeout_ms) const;

  std::vector<ClientChannel::EventSource> GetEventSources() const;

 private:
  LocalHandle data_fd_;
  LocalHandle pollin_event_fd_;
  LocalHandle pollhup_event_fd_;
  LocalHandle epoll_fd_;

  ChannelEventReceiver(const ChannelEventReceiver&) = delete;
  void operator=(const ChannelEventReceiver&) = delete;
};

}  // namespace uds
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_UDS_CHANNEL_EVENT_SET_H_
