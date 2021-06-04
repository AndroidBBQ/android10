#include "private/uds/channel_event_set.h"

#include <errno.h>
#include <log/log.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <uds/ipc_helper.h>

namespace android {
namespace pdx {
namespace uds {

namespace {

template <typename FileHandleType>
Status<void> SetupHandle(int fd, FileHandleType* handle,
                         const char* error_name) {
  const int error = errno;
  handle->Reset(fd);
  if (!*handle) {
    ALOGE("SetupHandle: Failed to setup %s handle: %s", error_name,
          strerror(error));
    return ErrorStatus{error};
  }
  return {};
}

}  // anonymous namespace

ChannelEventSet::ChannelEventSet() {
  const int flags = EFD_CLOEXEC | EFD_NONBLOCK;
  LocalHandle pollin_event_fd, pollhup_event_fd;

  if (!SetupHandle(eventfd(0, flags), &pollin_event_fd, "pollin_event") ||
      !SetupHandle(eventfd(0, flags), &pollhup_event_fd, "pollhup_event")) {
    return;
  }

  pollin_event_fd_ = std::move(pollin_event_fd);
  pollhup_event_fd_ = std::move(pollhup_event_fd);
}

int ChannelEventSet::ModifyEvents(int clear_mask, int set_mask) {
  ALOGD_IF(TRACE, "ChannelEventSet::ModifyEvents: clear_mask=%x set_mask=%x",
           clear_mask, set_mask);
  const int old_bits = event_bits_;
  const int new_bits = (event_bits_ & ~clear_mask) | set_mask;
  event_bits_ = new_bits;
  eventfd_t value;

  // Calculate which bits changed and how. Bits that haven't changed since last
  // modification will not change the state of an eventfd.
  const int set_bits = new_bits & ~old_bits;
  const int clear_bits = ~new_bits & old_bits;

  if (set_bits & EPOLLIN)
    eventfd_write(pollin_event_fd_.Get(), 1);
  else if (clear_bits & EPOLLIN)
    eventfd_read(pollin_event_fd_.Get(), &value);

  if (set_bits & EPOLLHUP)
    eventfd_write(pollhup_event_fd_.Get(), 1);
  else if (clear_bits & EPOLLHUP)
    eventfd_read(pollhup_event_fd_.Get(), &value);

  return 0;
}

ChannelEventReceiver::ChannelEventReceiver(LocalHandle data_fd,
                                           LocalHandle pollin_event_fd,
                                           LocalHandle pollhup_event_fd) {
  LocalHandle epoll_fd;
  if (!SetupHandle(epoll_create1(EPOLL_CLOEXEC), &epoll_fd, "epoll")) {
    return;
  }

  epoll_event event;
  event.events = EPOLLHUP | EPOLLRDHUP;
  event.data.u32 = 0;
  if (epoll_ctl(epoll_fd.Get(), EPOLL_CTL_ADD, data_fd.Get(), &event) < 0) {
    const int error = errno;
    ALOGE("ChannelEventSet::ChannelEventSet: Failed to add data_fd: %s",
          strerror(error));
    return;
  }

  event.events = EPOLLIN;
  event.data.u32 = 0;
  if (epoll_ctl(epoll_fd.Get(), EPOLL_CTL_ADD, pollin_event_fd.Get(), &event) <
      0) {
    const int error = errno;
    ALOGE("ChannelEventSet::ChannelEventSet: Failed to add pollin_event_fd: %s",
          strerror(error));
    return;
  }

  event.events = EPOLLIN;
  event.data.u32 = 0;
  if (epoll_ctl(epoll_fd.Get(), EPOLL_CTL_ADD, pollhup_event_fd.Get(), &event) <
      0) {
    const int error = errno;
    ALOGE(
        "ChannelEventSet::ChannelEventSet: Failed to add pollhup_event_fd: %s",
        strerror(error));
    return;
  }

  pollin_event_fd_ = std::move(pollin_event_fd);
  pollhup_event_fd_ = std::move(pollhup_event_fd);
  data_fd_ = std::move(data_fd);
  epoll_fd_ = std::move(epoll_fd);
}

Status<int> ChannelEventReceiver::PollPendingEvents(int timeout_ms) const {
  std::array<pollfd, 3> pfds = {{{pollin_event_fd_.Get(), POLLIN, 0},
                                 {pollhup_event_fd_.Get(), POLLIN, 0},
                                 {data_fd_.Get(), POLLHUP | POLLRDHUP, 0}}};
  if (RETRY_EINTR(poll(pfds.data(), pfds.size(), timeout_ms)) < 0) {
    const int error = errno;
    ALOGE(
        "ChannelEventReceiver::PollPendingEvents: Failed to poll for events: "
        "%s",
        strerror(error));
    return ErrorStatus{error};
  }

  const int event_mask =
      ((pfds[0].revents & POLLIN) ? EPOLLIN : 0) |
      ((pfds[1].revents & POLLIN) ? EPOLLHUP : 0) |
      ((pfds[2].revents & (POLLHUP | POLLRDHUP)) ? EPOLLHUP : 0);
  return {event_mask};
}

Status<int> ChannelEventReceiver::GetPendingEvents() const {
  constexpr long kTimeoutMs = 0;
  return PollPendingEvents(kTimeoutMs);
}

std::vector<ClientChannel::EventSource> ChannelEventReceiver::GetEventSources()
    const {
  return {{data_fd_.Get(), EPOLLHUP | EPOLLRDHUP},
          {pollin_event_fd_.Get(), EPOLLIN},
          {pollhup_event_fd_.Get(), POLLIN}};
}

}  // namespace uds
}  // namespace pdx
}  // namespace android
