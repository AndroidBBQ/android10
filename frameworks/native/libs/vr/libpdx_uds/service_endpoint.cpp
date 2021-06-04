#include "uds/service_endpoint.h"

#include <poll.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <algorithm>  // std::min

#include <android-base/logging.h>
#include <android-base/strings.h>
#include <cutils/sockets.h>
#include <pdx/service.h>
#include <selinux/selinux.h>
#include <uds/channel_manager.h>
#include <uds/client_channel_factory.h>
#include <uds/ipc_helper.h>

namespace {

constexpr int kMaxBackLogForSocketListen = 1;

using android::pdx::BorrowedChannelHandle;
using android::pdx::BorrowedHandle;
using android::pdx::ChannelReference;
using android::pdx::ErrorStatus;
using android::pdx::FileReference;
using android::pdx::LocalChannelHandle;
using android::pdx::LocalHandle;
using android::pdx::Status;
using android::pdx::uds::ChannelInfo;
using android::pdx::uds::ChannelManager;

struct MessageState {
  bool GetLocalFileHandle(int index, LocalHandle* handle) {
    if (index < 0) {
      handle->Reset(index);
    } else if (static_cast<size_t>(index) < request.file_descriptors.size()) {
      *handle = std::move(request.file_descriptors[index]);
    } else {
      return false;
    }
    return true;
  }

  bool GetLocalChannelHandle(int index, LocalChannelHandle* handle) {
    if (index < 0) {
      *handle = LocalChannelHandle{nullptr, index};
    } else if (static_cast<size_t>(index) < request.channels.size()) {
      auto& channel_info = request.channels[index];
      *handle = ChannelManager::Get().CreateHandle(
          std::move(channel_info.data_fd),
          std::move(channel_info.pollin_event_fd),
          std::move(channel_info.pollhup_event_fd));
    } else {
      return false;
    }
    return true;
  }

  Status<FileReference> PushFileHandle(BorrowedHandle handle) {
    if (!handle)
      return handle.Get();
    response.file_descriptors.push_back(std::move(handle));
    return response.file_descriptors.size() - 1;
  }

  Status<ChannelReference> PushChannelHandle(BorrowedChannelHandle handle) {
    if (!handle)
      return handle.value();

    if (auto* channel_data =
            ChannelManager::Get().GetChannelData(handle.value())) {
      ChannelInfo<BorrowedHandle> channel_info{
          channel_data->data_fd(), channel_data->pollin_event_fd(),
          channel_data->pollhup_event_fd()};
      response.channels.push_back(std::move(channel_info));
      return response.channels.size() - 1;
    } else {
      return ErrorStatus{EINVAL};
    }
  }

  Status<ChannelReference> PushChannelHandle(BorrowedHandle data_fd,
                                             BorrowedHandle pollin_event_fd,
                                             BorrowedHandle pollhup_event_fd) {
    if (!data_fd || !pollin_event_fd || !pollhup_event_fd)
      return ErrorStatus{EINVAL};
    ChannelInfo<BorrowedHandle> channel_info{std::move(data_fd),
                                             std::move(pollin_event_fd),
                                             std::move(pollhup_event_fd)};
    response.channels.push_back(std::move(channel_info));
    return response.channels.size() - 1;
  }

  Status<size_t> WriteData(const iovec* vector, size_t vector_length) {
    size_t size = 0;
    for (size_t i = 0; i < vector_length; i++) {
      const auto* data = reinterpret_cast<const uint8_t*>(vector[i].iov_base);
      response_data.insert(response_data.end(), data, data + vector[i].iov_len);
      size += vector[i].iov_len;
    }
    return size;
  }

  Status<size_t> ReadData(const iovec* vector, size_t vector_length) {
    size_t size_remaining = request_data.size() - request_data_read_pos;
    size_t size = 0;
    for (size_t i = 0; i < vector_length && size_remaining > 0; i++) {
      size_t size_to_copy = std::min(size_remaining, vector[i].iov_len);
      memcpy(vector[i].iov_base, request_data.data() + request_data_read_pos,
             size_to_copy);
      size += size_to_copy;
      request_data_read_pos += size_to_copy;
      size_remaining -= size_to_copy;
    }
    return size;
  }

  android::pdx::uds::RequestHeader<LocalHandle> request;
  android::pdx::uds::ResponseHeader<BorrowedHandle> response;
  std::vector<LocalHandle> sockets_to_close;
  std::vector<uint8_t> request_data;
  size_t request_data_read_pos{0};
  std::vector<uint8_t> response_data;
};

}  // anonymous namespace

namespace android {
namespace pdx {
namespace uds {

Endpoint::Endpoint(const std::string& endpoint_path, bool blocking,
                   bool use_init_socket_fd)
    : endpoint_path_{ClientChannelFactory::GetEndpointPath(endpoint_path)},
      is_blocking_{blocking} {
  LocalHandle fd;
  if (use_init_socket_fd) {
    // Cut off the /dev/socket/ prefix from the full socket path and use the
    // resulting "name" to retrieve the file descriptor for the socket created
    // by the init process.
    constexpr char prefix[] = "/dev/socket/";
    CHECK(android::base::StartsWith(endpoint_path_, prefix))
        << "Endpoint::Endpoint: Socket name '" << endpoint_path_
        << "' must begin with '" << prefix << "'";
    std::string socket_name = endpoint_path_.substr(sizeof(prefix) - 1);
    fd.Reset(android_get_control_socket(socket_name.c_str()));
    CHECK(fd.IsValid())
        << "Endpoint::Endpoint: Unable to obtain the control socket fd for '"
        << socket_name << "'";
    fcntl(fd.Get(), F_SETFD, FD_CLOEXEC);
  } else {
    fd.Reset(socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0));
    CHECK(fd.IsValid()) << "Endpoint::Endpoint: Failed to create socket: "
                        << strerror(errno);

    sockaddr_un local;
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, endpoint_path_.c_str(), sizeof(local.sun_path));
    local.sun_path[sizeof(local.sun_path) - 1] = '\0';

    unlink(local.sun_path);
    int ret =
        bind(fd.Get(), reinterpret_cast<sockaddr*>(&local), sizeof(local));
    CHECK_EQ(ret, 0) << "Endpoint::Endpoint: bind error: " << strerror(errno);
  }
  Init(std::move(fd));
}

Endpoint::Endpoint(LocalHandle socket_fd) { Init(std::move(socket_fd)); }

void Endpoint::Init(LocalHandle socket_fd) {
  if (socket_fd) {
    CHECK_EQ(listen(socket_fd.Get(), kMaxBackLogForSocketListen), 0)
        << "Endpoint::Endpoint: listen error: " << strerror(errno);
  }
  cancel_event_fd_.Reset(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));
  CHECK(cancel_event_fd_.IsValid())
      << "Endpoint::Endpoint: Failed to create event fd: " << strerror(errno);

  epoll_fd_.Reset(epoll_create1(EPOLL_CLOEXEC));
  CHECK(epoll_fd_.IsValid())
      << "Endpoint::Endpoint: Failed to create epoll fd: " << strerror(errno);

  if (socket_fd) {
    epoll_event socket_event;
    socket_event.events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT;
    socket_event.data.fd = socket_fd.Get();
    int ret = epoll_ctl(epoll_fd_.Get(), EPOLL_CTL_ADD, socket_fd.Get(),
                        &socket_event);
    CHECK_EQ(ret, 0)
        << "Endpoint::Endpoint: Failed to add socket fd to epoll fd: "
        << strerror(errno);
  }

  epoll_event cancel_event;
  cancel_event.events = EPOLLIN;
  cancel_event.data.fd = cancel_event_fd_.Get();

  int ret = epoll_ctl(epoll_fd_.Get(), EPOLL_CTL_ADD, cancel_event_fd_.Get(),
                      &cancel_event);
  CHECK_EQ(ret, 0)
      << "Endpoint::Endpoint: Failed to add cancel event fd to epoll fd: "
      << strerror(errno);
  socket_fd_ = std::move(socket_fd);
}

void* Endpoint::AllocateMessageState() { return new MessageState; }

void Endpoint::FreeMessageState(void* state) {
  delete static_cast<MessageState*>(state);
}

Status<void> Endpoint::AcceptConnection(Message* message) {
  if (!socket_fd_)
    return ErrorStatus(EBADF);

  sockaddr_un remote;
  socklen_t addrlen = sizeof(remote);
  LocalHandle connection_fd{accept4(socket_fd_.Get(),
                                    reinterpret_cast<sockaddr*>(&remote),
                                    &addrlen, SOCK_CLOEXEC)};
  if (!connection_fd) {
    ALOGE("Endpoint::AcceptConnection: failed to accept connection: %s",
          strerror(errno));
    return ErrorStatus(errno);
  }

  LocalHandle local_socket;
  LocalHandle remote_socket;
  auto status = CreateChannelSocketPair(&local_socket, &remote_socket);
  if (!status)
    return status;

  // Borrow the local channel handle before we move it into OnNewChannel().
  BorrowedHandle channel_handle = local_socket.Borrow();
  status = OnNewChannel(std::move(local_socket));
  if (!status)
    return status;

  // Send the channel socket fd to the client.
  ChannelConnectionInfo<LocalHandle> connection_info;
  connection_info.channel_fd = std::move(remote_socket);
  status = SendData(connection_fd.Borrow(), connection_info);

  if (status) {
    // Get the CHANNEL_OPEN message from client over the channel socket.
    status = ReceiveMessageForChannel(channel_handle, message);
  } else {
    CloseChannel(GetChannelId(channel_handle));
  }

  // Don't need the connection socket anymore. Further communication should
  // happen over the channel socket.
  shutdown(connection_fd.Get(), SHUT_WR);
  return status;
}

Status<void> Endpoint::SetService(Service* service) {
  service_ = service;
  return {};
}

Status<void> Endpoint::SetChannel(int channel_id, Channel* channel) {
  std::lock_guard<std::mutex> autolock(channel_mutex_);
  auto channel_data = channels_.find(channel_id);
  if (channel_data == channels_.end())
    return ErrorStatus{EINVAL};
  channel_data->second.channel_state = channel;
  return {};
}

Status<void> Endpoint::OnNewChannel(LocalHandle channel_fd) {
  std::lock_guard<std::mutex> autolock(channel_mutex_);
  Status<void> status;
  status.PropagateError(OnNewChannelLocked(std::move(channel_fd), nullptr));
  return status;
}

Status<std::pair<int32_t, Endpoint::ChannelData*>> Endpoint::OnNewChannelLocked(
    LocalHandle channel_fd, Channel* channel_state) {
  epoll_event event;
  event.events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT;
  event.data.fd = channel_fd.Get();
  if (epoll_ctl(epoll_fd_.Get(), EPOLL_CTL_ADD, channel_fd.Get(), &event) < 0) {
    ALOGE(
        "Endpoint::OnNewChannelLocked: Failed to add channel to endpoint: %s\n",
        strerror(errno));
    return ErrorStatus(errno);
  }
  ChannelData channel_data;
  channel_data.data_fd = std::move(channel_fd);
  channel_data.channel_state = channel_state;
  for (;;) {
    // Try new channel IDs until we find one which is not already in the map.
    if (last_channel_id_++ == std::numeric_limits<int32_t>::max())
      last_channel_id_ = 1;
    auto iter = channels_.lower_bound(last_channel_id_);
    if (iter == channels_.end() || iter->first != last_channel_id_) {
      channel_fd_to_id_.emplace(channel_data.data_fd.Get(), last_channel_id_);
      iter = channels_.emplace_hint(iter, last_channel_id_,
                                    std::move(channel_data));
      return std::make_pair(last_channel_id_, &iter->second);
    }
  }
}

Status<void> Endpoint::ReenableEpollEvent(const BorrowedHandle& fd) {
  epoll_event event;
  event.events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT;
  event.data.fd = fd.Get();
  if (epoll_ctl(epoll_fd_.Get(), EPOLL_CTL_MOD, fd.Get(), &event) < 0) {
    ALOGE(
        "Endpoint::ReenableEpollEvent: Failed to re-enable channel to "
        "endpoint: %s\n",
        strerror(errno));
    return ErrorStatus(errno);
  }
  return {};
}

Status<void> Endpoint::CloseChannel(int channel_id) {
  std::lock_guard<std::mutex> autolock(channel_mutex_);
  return CloseChannelLocked(channel_id);
}

Status<void> Endpoint::CloseChannelLocked(int32_t channel_id) {
  ALOGD_IF(TRACE, "Endpoint::CloseChannelLocked: channel_id=%d", channel_id);

  auto iter = channels_.find(channel_id);
  if (iter == channels_.end())
    return ErrorStatus{EINVAL};

  int channel_fd = iter->second.data_fd.Get();
  Status<void> status;
  epoll_event dummy;  // See BUGS in man 2 epoll_ctl.
  if (epoll_ctl(epoll_fd_.Get(), EPOLL_CTL_DEL, channel_fd, &dummy) < 0) {
    status.SetError(errno);
    ALOGE(
        "Endpoint::CloseChannelLocked: Failed to remove channel from endpoint: "
        "%s\n",
        strerror(errno));
  } else {
    status.SetValue();
  }

  channel_fd_to_id_.erase(channel_fd);
  channels_.erase(iter);
  return status;
}

Status<void> Endpoint::ModifyChannelEvents(int channel_id, int clear_mask,
                                           int set_mask) {
  std::lock_guard<std::mutex> autolock(channel_mutex_);

  auto search = channels_.find(channel_id);
  if (search != channels_.end()) {
    auto& channel_data = search->second;
    channel_data.event_set.ModifyEvents(clear_mask, set_mask);
    return {};
  }

  return ErrorStatus{EINVAL};
}

Status<void> Endpoint::CreateChannelSocketPair(LocalHandle* local_socket,
                                               LocalHandle* remote_socket) {
  Status<void> status;
  char* endpoint_context = nullptr;
  // Make sure the channel socket has the correct SELinux label applied.
  // Here we get the label from the endpoint file descriptor, which should be
  // something like "u:object_r:pdx_service_endpoint_socket:s0" and replace
  // "endpoint" with "channel" to produce the channel label such as this:
  // "u:object_r:pdx_service_channel_socket:s0".
  if (fgetfilecon_raw(socket_fd_.Get(), &endpoint_context) > 0) {
    std::string channel_context = endpoint_context;
    freecon(endpoint_context);
    const std::string suffix = "_endpoint_socket";
    auto pos = channel_context.find(suffix);
    if (pos != std::string::npos) {
      channel_context.replace(pos, suffix.size(), "_channel_socket");
    } else {
      ALOGW(
          "Endpoint::CreateChannelSocketPair: Endpoint security context '%s' "
          "does not contain expected substring '%s'",
          channel_context.c_str(), suffix.c_str());
    }
    ALOGE_IF(setsockcreatecon_raw(channel_context.c_str()) == -1,
             "Endpoint::CreateChannelSocketPair: Failed to set channel socket "
             "security context: %s",
             strerror(errno));
  } else {
    ALOGE(
        "Endpoint::CreateChannelSocketPair: Failed to obtain the endpoint "
        "socket's security context: %s",
        strerror(errno));
  }

  int channel_pair[2] = {};
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, channel_pair) == -1) {
    ALOGE("Endpoint::CreateChannelSocketPair: Failed to create socket pair: %s",
          strerror(errno));
    status.SetError(errno);
    return status;
  }

  setsockcreatecon_raw(nullptr);

  local_socket->Reset(channel_pair[0]);
  remote_socket->Reset(channel_pair[1]);

  int optval = 1;
  if (setsockopt(local_socket->Get(), SOL_SOCKET, SO_PASSCRED, &optval,
                 sizeof(optval)) == -1) {
    ALOGE(
        "Endpoint::CreateChannelSocketPair: Failed to enable the receiving of "
        "the credentials for channel %d: %s",
        local_socket->Get(), strerror(errno));
    status.SetError(errno);
  }
  return status;
}

Status<RemoteChannelHandle> Endpoint::PushChannel(Message* message,
                                                  int /*flags*/,
                                                  Channel* channel,
                                                  int* channel_id) {
  LocalHandle local_socket;
  LocalHandle remote_socket;
  auto status = CreateChannelSocketPair(&local_socket, &remote_socket);
  if (!status)
    return status.error_status();

  std::lock_guard<std::mutex> autolock(channel_mutex_);
  auto channel_data_status =
      OnNewChannelLocked(std::move(local_socket), channel);
  if (!channel_data_status)
    return channel_data_status.error_status();

  ChannelData* channel_data;
  std::tie(*channel_id, channel_data) = channel_data_status.take();

  // Flags are ignored for now.
  // TODO(xiaohuit): Implement those.

  auto* state = static_cast<MessageState*>(message->GetState());
  Status<ChannelReference> ref = state->PushChannelHandle(
      remote_socket.Borrow(), channel_data->event_set.pollin_event_fd(),
      channel_data->event_set.pollhup_event_fd());
  if (!ref)
    return ref.error_status();
  state->sockets_to_close.push_back(std::move(remote_socket));
  return RemoteChannelHandle{ref.get()};
}

Status<int> Endpoint::CheckChannel(const Message* /*message*/,
                                   ChannelReference /*ref*/,
                                   Channel** /*channel*/) {
  // TODO(xiaohuit): Implement this.
  return ErrorStatus(EFAULT);
}

Channel* Endpoint::GetChannelState(int32_t channel_id) {
  std::lock_guard<std::mutex> autolock(channel_mutex_);
  auto channel_data = channels_.find(channel_id);
  return (channel_data != channels_.end()) ? channel_data->second.channel_state
                                           : nullptr;
}

BorrowedHandle Endpoint::GetChannelSocketFd(int32_t channel_id) {
  std::lock_guard<std::mutex> autolock(channel_mutex_);
  BorrowedHandle handle;
  auto channel_data = channels_.find(channel_id);
  if (channel_data != channels_.end())
    handle = channel_data->second.data_fd.Borrow();
  return handle;
}

Status<std::pair<BorrowedHandle, BorrowedHandle>> Endpoint::GetChannelEventFd(
    int32_t channel_id) {
  std::lock_guard<std::mutex> autolock(channel_mutex_);
  auto channel_data = channels_.find(channel_id);
  if (channel_data != channels_.end()) {
    return {{channel_data->second.event_set.pollin_event_fd(),
             channel_data->second.event_set.pollhup_event_fd()}};
  }
  return ErrorStatus(ENOENT);
}

int32_t Endpoint::GetChannelId(const BorrowedHandle& channel_fd) {
  std::lock_guard<std::mutex> autolock(channel_mutex_);
  auto iter = channel_fd_to_id_.find(channel_fd.Get());
  return (iter != channel_fd_to_id_.end()) ? iter->second : -1;
}

Status<void> Endpoint::ReceiveMessageForChannel(
    const BorrowedHandle& channel_fd, Message* message) {
  RequestHeader<LocalHandle> request;
  int32_t channel_id = GetChannelId(channel_fd);
  auto status = ReceiveData(channel_fd.Borrow(), &request);
  if (!status) {
    if (status.error() == ESHUTDOWN) {
      BuildCloseMessage(channel_id, message);
      return {};
    } else {
      CloseChannel(channel_id);
      return status;
    }
  }

  MessageInfo info;
  info.pid = request.cred.pid;
  info.tid = -1;
  info.cid = channel_id;
  info.mid = request.is_impulse ? Message::IMPULSE_MESSAGE_ID
                                : GetNextAvailableMessageId();
  info.euid = request.cred.uid;
  info.egid = request.cred.gid;
  info.op = request.op;
  info.flags = 0;
  info.service = service_;
  info.channel = GetChannelState(channel_id);
  if (info.channel != nullptr) {
    info.channel->SetActiveProcessId(request.cred.pid);
  }
  info.send_len = request.send_len;
  info.recv_len = request.max_recv_len;
  info.fd_count = request.file_descriptors.size();
  static_assert(sizeof(info.impulse) == request.impulse_payload.size(),
                "Impulse payload sizes must be the same in RequestHeader and "
                "MessageInfo");
  memcpy(info.impulse, request.impulse_payload.data(),
         request.impulse_payload.size());
  *message = Message{info};
  auto* state = static_cast<MessageState*>(message->GetState());
  state->request = std::move(request);
  if (request.send_len > 0 && !request.is_impulse) {
    state->request_data.resize(request.send_len);
    status = ReceiveData(channel_fd, state->request_data.data(),
                         state->request_data.size());
  }

  if (status && request.is_impulse)
    status = ReenableEpollEvent(channel_fd);

  if (!status) {
    if (status.error() == ESHUTDOWN) {
      BuildCloseMessage(channel_id, message);
      return {};
    } else {
      CloseChannel(channel_id);
      return status;
    }
  }

  return status;
}

void Endpoint::BuildCloseMessage(int32_t channel_id, Message* message) {
  ALOGD_IF(TRACE, "Endpoint::BuildCloseMessage: channel_id=%d", channel_id);
  MessageInfo info;
  info.pid = -1;
  info.tid = -1;
  info.cid = channel_id;
  info.mid = GetNextAvailableMessageId();
  info.euid = -1;
  info.egid = -1;
  info.op = opcodes::CHANNEL_CLOSE;
  info.flags = 0;
  info.service = service_;
  info.channel = GetChannelState(channel_id);
  info.send_len = 0;
  info.recv_len = 0;
  info.fd_count = 0;
  *message = Message{info};
}

Status<void> Endpoint::MessageReceive(Message* message) {
  // Receive at most one event from the epoll set. This should prevent multiple
  // dispatch threads from attempting to handle messages on the same socket at
  // the same time.
  epoll_event event;
  int count = RETRY_EINTR(
      epoll_wait(epoll_fd_.Get(), &event, 1, is_blocking_ ? -1 : 0));
  if (count < 0) {
    ALOGE("Endpoint::MessageReceive: Failed to wait for epoll events: %s\n",
          strerror(errno));
    return ErrorStatus{errno};
  } else if (count == 0) {
    return ErrorStatus{ETIMEDOUT};
  }

  if (event.data.fd == cancel_event_fd_.Get()) {
    return ErrorStatus{ESHUTDOWN};
  }

  if (socket_fd_ && event.data.fd == socket_fd_.Get()) {
    auto status = AcceptConnection(message);
    auto reenable_status = ReenableEpollEvent(socket_fd_.Borrow());
    if (!reenable_status)
      return reenable_status;
    return status;
  }

  BorrowedHandle channel_fd{event.data.fd};
  return ReceiveMessageForChannel(channel_fd, message);
}

Status<void> Endpoint::MessageReply(Message* message, int return_code) {
  const int32_t channel_id = message->GetChannelId();
  auto channel_socket = GetChannelSocketFd(channel_id);
  if (!channel_socket)
    return ErrorStatus{EBADF};

  auto* state = static_cast<MessageState*>(message->GetState());
  switch (message->GetOp()) {
    case opcodes::CHANNEL_CLOSE:
      return CloseChannel(channel_id);

    case opcodes::CHANNEL_OPEN:
      if (return_code < 0) {
        return CloseChannel(channel_id);
      } else {
        // Open messages do not have a payload and may not transfer any channels
        // or file descriptors on behalf of the service.
        state->response_data.clear();
        state->response.file_descriptors.clear();
        state->response.channels.clear();

        // Return the channel event-related fds in a single ChannelInfo entry
        // with an empty data_fd member.
        auto status = GetChannelEventFd(channel_id);
        if (!status)
          return status.error_status();

        auto handles = status.take();
        state->response.channels.push_back({BorrowedHandle(),
                                            std::move(handles.first),
                                            std::move(handles.second)});
        return_code = 0;
      }
      break;
  }

  state->response.ret_code = return_code;
  state->response.recv_len = state->response_data.size();
  auto status = SendData(channel_socket, state->response);
  if (status && !state->response_data.empty()) {
    status = SendData(channel_socket, state->response_data.data(),
                      state->response_data.size());
  }

  if (status)
    status = ReenableEpollEvent(channel_socket);

  return status;
}

Status<void> Endpoint::MessageReplyFd(Message* message, unsigned int push_fd) {
  auto* state = static_cast<MessageState*>(message->GetState());
  auto ref = state->PushFileHandle(BorrowedHandle{static_cast<int>(push_fd)});
  if (!ref)
    return ref.error_status();
  return MessageReply(message, ref.get());
}

Status<void> Endpoint::MessageReplyChannelHandle(
    Message* message, const LocalChannelHandle& handle) {
  auto* state = static_cast<MessageState*>(message->GetState());
  auto ref = state->PushChannelHandle(handle.Borrow());
  if (!ref)
    return ref.error_status();
  return MessageReply(message, ref.get());
}

Status<void> Endpoint::MessageReplyChannelHandle(
    Message* message, const BorrowedChannelHandle& handle) {
  auto* state = static_cast<MessageState*>(message->GetState());
  auto ref = state->PushChannelHandle(handle.Duplicate());
  if (!ref)
    return ref.error_status();
  return MessageReply(message, ref.get());
}

Status<void> Endpoint::MessageReplyChannelHandle(
    Message* message, const RemoteChannelHandle& handle) {
  return MessageReply(message, handle.value());
}

Status<size_t> Endpoint::ReadMessageData(Message* message, const iovec* vector,
                                         size_t vector_length) {
  auto* state = static_cast<MessageState*>(message->GetState());
  return state->ReadData(vector, vector_length);
}

Status<size_t> Endpoint::WriteMessageData(Message* message, const iovec* vector,
                                          size_t vector_length) {
  auto* state = static_cast<MessageState*>(message->GetState());
  return state->WriteData(vector, vector_length);
}

Status<FileReference> Endpoint::PushFileHandle(Message* message,
                                               const LocalHandle& handle) {
  auto* state = static_cast<MessageState*>(message->GetState());
  return state->PushFileHandle(handle.Borrow());
}

Status<FileReference> Endpoint::PushFileHandle(Message* message,
                                               const BorrowedHandle& handle) {
  auto* state = static_cast<MessageState*>(message->GetState());
  return state->PushFileHandle(handle.Duplicate());
}

Status<FileReference> Endpoint::PushFileHandle(Message* /*message*/,
                                               const RemoteHandle& handle) {
  return handle.Get();
}

Status<ChannelReference> Endpoint::PushChannelHandle(
    Message* message, const LocalChannelHandle& handle) {
  auto* state = static_cast<MessageState*>(message->GetState());
  return state->PushChannelHandle(handle.Borrow());
}

Status<ChannelReference> Endpoint::PushChannelHandle(
    Message* message, const BorrowedChannelHandle& handle) {
  auto* state = static_cast<MessageState*>(message->GetState());
  return state->PushChannelHandle(handle.Duplicate());
}

Status<ChannelReference> Endpoint::PushChannelHandle(
    Message* /*message*/, const RemoteChannelHandle& handle) {
  return handle.value();
}

LocalHandle Endpoint::GetFileHandle(Message* message, FileReference ref) const {
  LocalHandle handle;
  auto* state = static_cast<MessageState*>(message->GetState());
  state->GetLocalFileHandle(ref, &handle);
  return handle;
}

LocalChannelHandle Endpoint::GetChannelHandle(Message* message,
                                              ChannelReference ref) const {
  LocalChannelHandle handle;
  auto* state = static_cast<MessageState*>(message->GetState());
  state->GetLocalChannelHandle(ref, &handle);
  return handle;
}

Status<void> Endpoint::Cancel() {
  if (eventfd_write(cancel_event_fd_.Get(), 1) < 0)
    return ErrorStatus{errno};
  return {};
}

std::unique_ptr<Endpoint> Endpoint::Create(const std::string& endpoint_path,
                                           mode_t /*unused_mode*/,
                                           bool blocking) {
  return std::unique_ptr<Endpoint>(new Endpoint(endpoint_path, blocking));
}

std::unique_ptr<Endpoint> Endpoint::CreateAndBindSocket(
    const std::string& endpoint_path, bool blocking) {
  return std::unique_ptr<Endpoint>(
      new Endpoint(endpoint_path, blocking, false));
}

std::unique_ptr<Endpoint> Endpoint::CreateFromSocketFd(LocalHandle socket_fd) {
  return std::unique_ptr<Endpoint>(new Endpoint(std::move(socket_fd)));
}

Status<void> Endpoint::RegisterNewChannelForTests(LocalHandle channel_fd) {
  int optval = 1;
  if (setsockopt(channel_fd.Get(), SOL_SOCKET, SO_PASSCRED, &optval,
                 sizeof(optval)) == -1) {
    ALOGE(
        "Endpoint::RegisterNewChannelForTests: Failed to enable the receiving"
        "of the credentials for channel %d: %s",
        channel_fd.Get(), strerror(errno));
    return ErrorStatus(errno);
  }
  return OnNewChannel(std::move(channel_fd));
}

}  // namespace uds
}  // namespace pdx
}  // namespace android
