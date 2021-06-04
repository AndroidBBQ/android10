#include "pdx/client.h"

#include <log/log.h>

#include <pdx/trace.h>

namespace android {
namespace pdx {

void Client::EnableAutoReconnect(int64_t reconnect_timeout_ms) {
  if (channel_factory_) {
    reconnect_timeout_ms_ = reconnect_timeout_ms;
    auto_reconnect_enabled_ = true;
  }
}

void Client::DisableAutoReconnect() { auto_reconnect_enabled_ = false; }

bool Client::IsConnected() const { return channel_.get() != nullptr; }

Status<void> Client::CheckReconnect() {
  Status<void> ret;
  bool was_disconnected = !IsConnected();
  if (auto_reconnect_enabled_ && was_disconnected && channel_factory_) {
    auto status = channel_factory_->Connect(reconnect_timeout_ms_);
    if (!status) {
      error_ = -status.error();
      ret.SetError(status.error());
      return ret;
    }
    channel_ = status.take();
  }

  if (!IsConnected()) {
    ret.SetError(ESHUTDOWN);
  } else {
    // Call the subclass OnConnect handler. The subclass may choose to close the
    // connection in the handler, in which case error_ will be non-zero.
    if (was_disconnected)
      OnConnect();
    if (!IsConnected())
      ret.SetError(-error_);
    else
      ret.SetValue();
  }

  return ret;
}

bool Client::NeedToDisconnectChannel(int error) const {
  return error == ESHUTDOWN && auto_reconnect_enabled_;
}

void Client::CheckDisconnect(int error) {
  if (NeedToDisconnectChannel(error))
    Close(error);
}

Client::Client(std::unique_ptr<ClientChannel> channel)
    : channel_{std::move(channel)} {}

Client::Client(std::unique_ptr<ClientChannelFactory> channel_factory,
               int64_t timeout_ms)
    : channel_factory_{std::move(channel_factory)} {
  auto status = channel_factory_->Connect(timeout_ms);
  if (!status) {
    ALOGE("Client::Client: Failed to connect to service because: %s",
          status.GetErrorMessage().c_str());
    error_ = -status.error();
  } else {
    channel_ = status.take();
  }
}

bool Client::IsInitialized() const {
  return IsConnected() || (channel_factory_ && auto_reconnect_enabled_);
}

void Client::OnConnect() {}

int Client::error() const { return error_; }

Status<void> Client::SendImpulse(int opcode) {
  PDX_TRACE_NAME("Client::SendImpulse");

  auto status = CheckReconnect();
  if (!status)
    return status;

  status = channel_->SendImpulse(opcode, nullptr, 0);
  CheckDisconnect(status);
  return status;
}

Status<void> Client::SendImpulse(int opcode, const void* buffer,
                                 size_t length) {
  PDX_TRACE_NAME("Client::SendImpulse");

  auto status = CheckReconnect();
  if (!status)
    return status;

  status = channel_->SendImpulse(opcode, buffer, length);
  CheckDisconnect(status);
  return status;
}

void Client::Close(int error) {
  channel_.reset();
  // Normalize error codes to negative integer space.
  error_ = error <= 0 ? error : -error;
}

int Client::event_fd() const {
  return IsConnected() ? channel_->event_fd() : -1;
}

LocalChannelHandle& Client::GetChannelHandle() {
  return channel_->GetChannelHandle();
}

const LocalChannelHandle& Client::GetChannelHandle() const {
  return channel_->GetChannelHandle();
}

///////////////////////////// Transaction implementation //////////////////////

Transaction::Transaction(Client& client) : client_{client} {}

Transaction::~Transaction() {
  if (state_allocated_ && client_.GetChannel())
    client_.GetChannel()->FreeTransactionState(state_);
}

bool Transaction::EnsureStateAllocated() {
  if (!state_allocated_ && client_.GetChannel()) {
    state_ = client_.GetChannel()->AllocateTransactionState();
    state_allocated_ = true;
  }
  return state_allocated_;
}

void Transaction::SendTransaction(int opcode, Status<void>* ret,
                                  const iovec* send_vector, size_t send_count,
                                  const iovec* receive_vector,
                                  size_t receive_count) {
  *ret = client_.CheckReconnect();
  if (!*ret)
    return;

  if (!EnsureStateAllocated()) {
    ret->SetError(ESHUTDOWN);
    return;
  }

  auto status = client_.GetChannel()->SendWithInt(
      state_, opcode, send_vector, send_count, receive_vector, receive_count);

  if (status) {
    ret->SetValue();
  } else {
    ret->SetError(status.error());
  }
  CheckDisconnect(status);
}

void Transaction::SendTransaction(int opcode, Status<int>* ret,
                                  const iovec* send_vector, size_t send_count,
                                  const iovec* receive_vector,
                                  size_t receive_count) {
  auto status = client_.CheckReconnect();
  if (!status) {
    ret->SetError(status.error());
    return;
  }

  if (!EnsureStateAllocated()) {
    ret->SetError(ESHUTDOWN);
    return;
  }

  *ret = client_.GetChannel()->SendWithInt(
      state_, opcode, send_vector, send_count, receive_vector, receive_count);

  CheckDisconnect(*ret);
}

void Transaction::SendTransaction(int opcode, Status<LocalHandle>* ret,
                                  const iovec* send_vector, size_t send_count,
                                  const iovec* receive_vector,
                                  size_t receive_count) {
  auto status = client_.CheckReconnect();
  if (!status) {
    ret->SetError(status.error());
    return;
  }

  if (!EnsureStateAllocated()) {
    ret->SetError(ESHUTDOWN);
    return;
  }

  *ret = client_.GetChannel()->SendWithFileHandle(
      state_, opcode, send_vector, send_count, receive_vector, receive_count);

  CheckDisconnect(*ret);
}

void Transaction::SendTransaction(int opcode, Status<LocalChannelHandle>* ret,
                                  const iovec* send_vector, size_t send_count,
                                  const iovec* receive_vector,
                                  size_t receive_count) {
  auto status = client_.CheckReconnect();
  if (!status) {
    ret->SetError(status.error());
    return;
  }

  if (!EnsureStateAllocated()) {
    ret->SetError(ESHUTDOWN);
    return;
  }

  *ret = client_.GetChannel()->SendWithChannelHandle(
      state_, opcode, send_vector, send_count, receive_vector, receive_count);

  CheckDisconnect(*ret);
}

Status<FileReference> Transaction::PushFileHandle(const LocalHandle& handle) {
  if (client_.CheckReconnect() && EnsureStateAllocated())
    return client_.GetChannel()->PushFileHandle(state_, handle);
  return ErrorStatus{ESHUTDOWN};
}

Status<FileReference> Transaction::PushFileHandle(
    const BorrowedHandle& handle) {
  if (client_.CheckReconnect() && EnsureStateAllocated())
    return client_.GetChannel()->PushFileHandle(state_, handle);
  return ErrorStatus{ESHUTDOWN};
}

Status<FileReference> Transaction::PushFileHandle(const RemoteHandle& handle) {
  return handle.Get();
}

Status<ChannelReference> Transaction::PushChannelHandle(
    const LocalChannelHandle& handle) {
  if (client_.CheckReconnect() && EnsureStateAllocated())
    return client_.GetChannel()->PushChannelHandle(state_, handle);
  return ErrorStatus{ESHUTDOWN};
}

Status<ChannelReference> Transaction::PushChannelHandle(
    const BorrowedChannelHandle& handle) {
  if (client_.CheckReconnect() && EnsureStateAllocated())
    return client_.GetChannel()->PushChannelHandle(state_, handle);
  return ErrorStatus{ESHUTDOWN};
}

Status<ChannelReference> Transaction::PushChannelHandle(
    const RemoteChannelHandle& handle) {
  return handle.value();
}

bool Transaction::GetFileHandle(FileReference ref, LocalHandle* handle) {
  return client_.CheckReconnect() && EnsureStateAllocated() &&
         client_.GetChannel()->GetFileHandle(state_, ref, handle);
}

bool Transaction::GetChannelHandle(ChannelReference ref,
                                   LocalChannelHandle* handle) {
  return client_.CheckReconnect() && EnsureStateAllocated() &&
         client_.GetChannel()->GetChannelHandle(state_, ref, handle);
}

void Transaction::CheckDisconnect(int error) {
  if (client_.NeedToDisconnectChannel(error)) {
    if (state_allocated_) {
      if (client_.GetChannel())
        client_.GetChannel()->FreeTransactionState(state_);
      state_ = nullptr;
      state_allocated_ = false;
    }
    client_.Close(error);
  }
}

}  // namespace pdx
}  // namespace android
