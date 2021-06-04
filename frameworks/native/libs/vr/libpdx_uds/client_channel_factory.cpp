#include <uds/client_channel_factory.h>

#include <errno.h>
#include <log/log.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <chrono>
#include <thread>

#include <uds/channel_manager.h>
#include <uds/client_channel.h>
#include <uds/ipc_helper.h>

using std::chrono::duration_cast;
using std::chrono::steady_clock;

namespace android {
namespace pdx {
namespace uds {

std::string ClientChannelFactory::GetRootEndpointPath() {
  return "/dev/socket/pdx";
}

std::string ClientChannelFactory::GetEndpointPath(
    const std::string& endpoint_path) {
  std::string path;
  if (!endpoint_path.empty()) {
    if (endpoint_path.front() == '/')
      path = endpoint_path;
    else
      path = GetRootEndpointPath() + '/' + endpoint_path;
  }
  return path;
}

ClientChannelFactory::ClientChannelFactory(const std::string& endpoint_path)
    : endpoint_path_{GetEndpointPath(endpoint_path)} {}

ClientChannelFactory::ClientChannelFactory(LocalHandle socket)
    : socket_{std::move(socket)} {}

std::unique_ptr<pdx::ClientChannelFactory> ClientChannelFactory::Create(
    const std::string& endpoint_path) {
  return std::unique_ptr<pdx::ClientChannelFactory>{
      new ClientChannelFactory{endpoint_path}};
}

std::unique_ptr<pdx::ClientChannelFactory> ClientChannelFactory::Create(
    LocalHandle socket) {
  return std::unique_ptr<pdx::ClientChannelFactory>{
      new ClientChannelFactory{std::move(socket)}};
}

Status<std::unique_ptr<pdx::ClientChannel>> ClientChannelFactory::Connect(
    int64_t timeout_ms) const {
  Status<void> status;

  bool connected = socket_.IsValid();
  if (!connected) {
    socket_.Reset(socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0));
    LOG_ALWAYS_FATAL_IF(
        endpoint_path_.empty(),
        "ClientChannelFactory::Connect: unspecified socket path");
  }

  if (!socket_) {
    ALOGE("ClientChannelFactory::Connect: socket error: %s", strerror(errno));
    return ErrorStatus(errno);
  }

  bool use_timeout = (timeout_ms >= 0);
  auto now = steady_clock::now();
  auto time_end = now + std::chrono::milliseconds{timeout_ms};

  int max_eaccess = 5;  // Max number of times to retry when EACCES returned.
  while (!connected) {
    int64_t timeout = -1;
    if (use_timeout) {
      auto remaining = time_end - now;
      timeout = duration_cast<std::chrono::milliseconds>(remaining).count();
      if (timeout < 0)
        return ErrorStatus(ETIMEDOUT);
    }
    sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, endpoint_path_.c_str(), sizeof(remote.sun_path));
    remote.sun_path[sizeof(remote.sun_path) - 1] = '\0';
    ALOGD("ClientChannelFactory: Waiting for endpoint at %s", remote.sun_path);
    status = WaitForEndpoint(endpoint_path_, timeout);
    if (!status)
      return ErrorStatus(status.error());

    ALOGD("ClientChannelFactory: Connecting to %s", remote.sun_path);
    int ret = RETRY_EINTR(connect(
        socket_.Get(), reinterpret_cast<sockaddr*>(&remote), sizeof(remote)));
    if (ret == -1) {
      ALOGD("ClientChannelFactory: Connect error %d: %s", errno,
            strerror(errno));
      // if |max_eaccess| below reaches zero when errno is EACCES, the control
      // flows into the next "else if" statement and a permanent error is
      // returned from this function.
      if (errno == ECONNREFUSED || (errno == EACCES && max_eaccess-- > 0)) {
        // Connection refused/Permission denied can be the result of connecting
        // too early (the service socket is created but its access rights are
        // not set or not being listened to yet).
        ALOGD("ClientChannelFactory: %s, waiting...", strerror(errno));
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(100ms);
      } else if (errno != ENOENT && errno != ENOTDIR) {
        // ENOENT/ENOTDIR might mean that the socket file/directory containing
        // it has been just deleted. Try to wait for its creation and do not
        // return an error immediately.
        ALOGE(
            "ClientChannelFactory::Connect: Failed to initialize connection "
            "when connecting: %s",
            strerror(errno));
        return ErrorStatus(errno);
      }
    } else {
      connected = true;
      ALOGD("ClientChannelFactory: Connected successfully to %s...",
            remote.sun_path);
      ChannelConnectionInfo<LocalHandle> connection_info;
      status = ReceiveData(socket_.Borrow(), &connection_info);
      if (!status)
        return status.error_status();
      socket_ = std::move(connection_info.channel_fd);
      if (!socket_) {
        ALOGE("ClientChannelFactory::Connect: Failed to obtain channel socket");
        return ErrorStatus(EIO);
      }
    }
    if (use_timeout)
      now = steady_clock::now();
  }  // while (!connected)

  RequestHeader<BorrowedHandle> request;
  InitRequest(&request, opcodes::CHANNEL_OPEN, 0, 0, false);

  status = SendData(socket_.Borrow(), request);
  if (!status)
    return status.error_status();

  ResponseHeader<LocalHandle> response;
  status = ReceiveData(socket_.Borrow(), &response);
  if (!status)
    return status.error_status();
  else if (response.ret_code < 0 || response.channels.size() != 1)
    return ErrorStatus(EIO);

  LocalHandle pollin_event_fd = std::move(response.channels[0].pollin_event_fd);
  LocalHandle pollhup_event_fd =
      std::move(response.channels[0].pollhup_event_fd);

  if (!pollin_event_fd || !pollhup_event_fd) {
    ALOGE(
        "ClientChannelFactory::Connect: Required fd was not returned from the "
        "service: pollin_event_fd=%d pollhup_event_fd=%d",
        pollin_event_fd.Get(), pollhup_event_fd.Get());
    return ErrorStatus(EIO);
  }

  return ClientChannel::Create(ChannelManager::Get().CreateHandle(
      std::move(socket_), std::move(pollin_event_fd),
      std::move(pollhup_event_fd)));
}

}  // namespace uds
}  // namespace pdx
}  // namespace android
