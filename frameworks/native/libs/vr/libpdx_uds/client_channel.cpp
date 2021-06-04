#include "uds/channel_parcelable.h"
#include "uds/client_channel.h"

#include <errno.h>
#include <log/log.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <pdx/client.h>
#include <pdx/service_endpoint.h>
#include <uds/ipc_helper.h>

namespace android {
namespace pdx {
namespace uds {

namespace {

struct TransactionState {
  bool GetLocalFileHandle(int index, LocalHandle* handle) {
    if (index < 0) {
      handle->Reset(index);
    } else if (static_cast<size_t>(index) < response.file_descriptors.size()) {
      *handle = std::move(response.file_descriptors[index]);
    } else {
      return false;
    }
    return true;
  }

  bool GetLocalChannelHandle(int index, LocalChannelHandle* handle) {
    if (index < 0) {
      *handle = LocalChannelHandle{nullptr, index};
    } else if (static_cast<size_t>(index) < response.channels.size()) {
      auto& channel_info = response.channels[index];
      *handle = ChannelManager::Get().CreateHandle(
          std::move(channel_info.data_fd),
          std::move(channel_info.pollin_event_fd),
          std::move(channel_info.pollhup_event_fd));
    } else {
      return false;
    }
    return true;
  }

  FileReference PushFileHandle(BorrowedHandle handle) {
    if (!handle)
      return handle.Get();
    request.file_descriptors.push_back(std::move(handle));
    return request.file_descriptors.size() - 1;
  }

  ChannelReference PushChannelHandle(BorrowedChannelHandle handle) {
    if (!handle)
      return handle.value();

    if (auto* channel_data =
            ChannelManager::Get().GetChannelData(handle.value())) {
      ChannelInfo<BorrowedHandle> channel_info{
          channel_data->data_fd(), channel_data->pollin_event_fd(),
          channel_data->pollhup_event_fd()};
      request.channels.push_back(std::move(channel_info));
      return request.channels.size() - 1;
    } else {
      return -1;
    }
  }

  RequestHeader<BorrowedHandle> request;
  ResponseHeader<LocalHandle> response;
};

Status<void> ReadAndDiscardData(const BorrowedHandle& socket_fd, size_t size) {
  while (size > 0) {
    // If there is more data to read in the message than the buffers provided
    // by the caller, read and discard the extra data from the socket.
    char buffer[1024];
    size_t size_to_read = std::min(sizeof(buffer), size);
    auto status = ReceiveData(socket_fd, buffer, size_to_read);
    if (!status)
      return status;
    size -= size_to_read;
  }
  // We still want to return EIO error to the caller in case we had unexpected
  // data in the socket stream.
  return ErrorStatus(EIO);
}

Status<void> SendRequest(const BorrowedHandle& socket_fd,
                         TransactionState* transaction_state, int opcode,
                         const iovec* send_vector, size_t send_count,
                         size_t max_recv_len) {
  size_t send_len = CountVectorSize(send_vector, send_count);
  InitRequest(&transaction_state->request, opcode, send_len, max_recv_len,
              false);
  if (send_len == 0) {
    send_vector = nullptr;
    send_count = 0;
  }
  return SendData(socket_fd, transaction_state->request, send_vector,
                  send_count);
}

Status<void> ReceiveResponse(const BorrowedHandle& socket_fd,
                             TransactionState* transaction_state,
                             const iovec* receive_vector, size_t receive_count,
                             size_t max_recv_len) {
  auto status = ReceiveData(socket_fd, &transaction_state->response);
  if (!status)
    return status;

  if (transaction_state->response.recv_len > 0) {
    std::vector<iovec> read_buffers;
    size_t size_remaining = 0;
    if (transaction_state->response.recv_len != max_recv_len) {
      // If the receive buffer not exactly the size of data available, recreate
      // the vector list to consume the data exactly since ReceiveDataVector()
      // validates that the number of bytes received equals the number of bytes
      // requested.
      size_remaining = transaction_state->response.recv_len;
      for (size_t i = 0; i < receive_count && size_remaining > 0; i++) {
        read_buffers.push_back(receive_vector[i]);
        iovec& last_vec = read_buffers.back();
        if (last_vec.iov_len > size_remaining)
          last_vec.iov_len = size_remaining;
        size_remaining -= last_vec.iov_len;
      }
      receive_vector = read_buffers.data();
      receive_count = read_buffers.size();
    }
    status = ReceiveDataVector(socket_fd, receive_vector, receive_count);
    if (status && size_remaining > 0)
      status = ReadAndDiscardData(socket_fd, size_remaining);
  }
  return status;
}

}  // anonymous namespace

ClientChannel::ClientChannel(LocalChannelHandle channel_handle)
    : channel_handle_{std::move(channel_handle)} {
  channel_data_ = ChannelManager::Get().GetChannelData(channel_handle_.value());
}

std::unique_ptr<pdx::ClientChannel> ClientChannel::Create(
    LocalChannelHandle channel_handle) {
  return std::unique_ptr<pdx::ClientChannel>{
      new ClientChannel{std::move(channel_handle)}};
}

ClientChannel::~ClientChannel() {
  if (channel_handle_)
    shutdown(channel_handle_.value(), SHUT_WR);
}

void* ClientChannel::AllocateTransactionState() { return new TransactionState; }

void ClientChannel::FreeTransactionState(void* state) {
  delete static_cast<TransactionState*>(state);
}

Status<void> ClientChannel::SendImpulse(int opcode, const void* buffer,
                                        size_t length) {
  std::unique_lock<std::mutex> lock(socket_mutex_);
  Status<void> status;
  android::pdx::uds::RequestHeader<BorrowedHandle> request;
  if (length > request.impulse_payload.size() ||
      (buffer == nullptr && length != 0)) {
    status.SetError(EINVAL);
    return status;
  }

  InitRequest(&request, opcode, length, 0, true);
  memcpy(request.impulse_payload.data(), buffer, length);
  return SendData(BorrowedHandle{channel_handle_.value()}, request);
}

Status<int> ClientChannel::SendAndReceive(void* transaction_state, int opcode,
                                          const iovec* send_vector,
                                          size_t send_count,
                                          const iovec* receive_vector,
                                          size_t receive_count) {
  std::unique_lock<std::mutex> lock(socket_mutex_);
  Status<int> result;
  if ((send_vector == nullptr && send_count != 0) ||
      (receive_vector == nullptr && receive_count != 0)) {
    result.SetError(EINVAL);
    return result;
  }

  auto* state = static_cast<TransactionState*>(transaction_state);
  size_t max_recv_len = CountVectorSize(receive_vector, receive_count);

  auto status = SendRequest(BorrowedHandle{channel_handle_.value()}, state,
                            opcode, send_vector, send_count, max_recv_len);
  if (status) {
    status = ReceiveResponse(BorrowedHandle{channel_handle_.value()}, state,
                             receive_vector, receive_count, max_recv_len);
  }
  if (!result.PropagateError(status)) {
    const int return_code = state->response.ret_code;
    if (return_code >= 0)
      result.SetValue(return_code);
    else
      result.SetError(-return_code);
  }
  return result;
}

Status<int> ClientChannel::SendWithInt(void* transaction_state, int opcode,
                                       const iovec* send_vector,
                                       size_t send_count,
                                       const iovec* receive_vector,
                                       size_t receive_count) {
  return SendAndReceive(transaction_state, opcode, send_vector, send_count,
                        receive_vector, receive_count);
}

Status<LocalHandle> ClientChannel::SendWithFileHandle(
    void* transaction_state, int opcode, const iovec* send_vector,
    size_t send_count, const iovec* receive_vector, size_t receive_count) {
  Status<int> int_status =
      SendAndReceive(transaction_state, opcode, send_vector, send_count,
                     receive_vector, receive_count);
  Status<LocalHandle> status;
  if (status.PropagateError(int_status))
    return status;

  auto* state = static_cast<TransactionState*>(transaction_state);
  LocalHandle handle;
  if (state->GetLocalFileHandle(int_status.get(), &handle)) {
    status.SetValue(std::move(handle));
  } else {
    status.SetError(EINVAL);
  }
  return status;
}

Status<LocalChannelHandle> ClientChannel::SendWithChannelHandle(
    void* transaction_state, int opcode, const iovec* send_vector,
    size_t send_count, const iovec* receive_vector, size_t receive_count) {
  Status<int> int_status =
      SendAndReceive(transaction_state, opcode, send_vector, send_count,
                     receive_vector, receive_count);
  Status<LocalChannelHandle> status;
  if (status.PropagateError(int_status))
    return status;

  auto* state = static_cast<TransactionState*>(transaction_state);
  LocalChannelHandle handle;
  if (state->GetLocalChannelHandle(int_status.get(), &handle)) {
    status.SetValue(std::move(handle));
  } else {
    status.SetError(EINVAL);
  }
  return status;
}

FileReference ClientChannel::PushFileHandle(void* transaction_state,
                                            const LocalHandle& handle) {
  auto* state = static_cast<TransactionState*>(transaction_state);
  return state->PushFileHandle(handle.Borrow());
}

FileReference ClientChannel::PushFileHandle(void* transaction_state,
                                            const BorrowedHandle& handle) {
  auto* state = static_cast<TransactionState*>(transaction_state);
  return state->PushFileHandle(handle.Duplicate());
}

ChannelReference ClientChannel::PushChannelHandle(
    void* transaction_state, const LocalChannelHandle& handle) {
  auto* state = static_cast<TransactionState*>(transaction_state);
  return state->PushChannelHandle(handle.Borrow());
}

ChannelReference ClientChannel::PushChannelHandle(
    void* transaction_state, const BorrowedChannelHandle& handle) {
  auto* state = static_cast<TransactionState*>(transaction_state);
  return state->PushChannelHandle(handle.Duplicate());
}

bool ClientChannel::GetFileHandle(void* transaction_state, FileReference ref,
                                  LocalHandle* handle) const {
  auto* state = static_cast<TransactionState*>(transaction_state);
  return state->GetLocalFileHandle(ref, handle);
}

bool ClientChannel::GetChannelHandle(void* transaction_state,
                                     ChannelReference ref,
                                     LocalChannelHandle* handle) const {
  auto* state = static_cast<TransactionState*>(transaction_state);
  return state->GetLocalChannelHandle(ref, handle);
}

std::unique_ptr<pdx::ChannelParcelable> ClientChannel::TakeChannelParcelable()
    {
  if (!channel_handle_)
    return nullptr;

  if (auto* channel_data =
          ChannelManager::Get().GetChannelData(channel_handle_.value())) {
    auto fds = channel_data->TakeFds();
    auto parcelable = std::make_unique<ChannelParcelable>(
        std::move(std::get<0>(fds)), std::move(std::get<1>(fds)),
        std::move(std::get<2>(fds)));

    // Here we need to explicitly close the channel handle so that the channel
    // won't get shutdown in the destructor, while the FDs in ChannelParcelable
    // can keep the channel alive so that new client can be created from it
    // later.
    channel_handle_.Close();
    return parcelable;
  } else {
    return nullptr;
  }
}

}  // namespace uds
}  // namespace pdx
}  // namespace android
