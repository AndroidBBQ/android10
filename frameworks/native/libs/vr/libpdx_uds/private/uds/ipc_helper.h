#ifndef ANDROID_PDX_UDS_IPC_HELPER_H_
#define ANDROID_PDX_UDS_IPC_HELPER_H_

#include <sys/socket.h>
#include <utility>
#include <vector>

#include <pdx/rpc/serializable.h>
#include <pdx/rpc/serialization.h>
#include <pdx/status.h>
#include <pdx/utility.h>

namespace android {
namespace pdx {
namespace uds {

// Test interfaces used for unit-testing payload sending/receiving over sockets.
class SendInterface {
 public:
  virtual ssize_t Send(int socket_fd, const void* data, size_t size,
                       int flags) = 0;
  virtual ssize_t SendMessage(int socket_fd, const msghdr* msg, int flags) = 0;

 protected:
  virtual ~SendInterface() = default;
};

class RecvInterface {
 public:
  virtual ssize_t Receive(int socket_fd, void* data, size_t size,
                          int flags) = 0;
  virtual ssize_t ReceiveMessage(int socket_fd, msghdr* msg, int flags) = 0;

 protected:
  virtual ~RecvInterface() = default;
};

// Helper methods that allow to send/receive data through abstract interfaces.
// Useful for mocking out the underlying socket I/O.
Status<void> SendAll(SendInterface* sender, const BorrowedHandle& socket_fd,
                     const void* data, size_t size);
Status<void> SendMsgAll(SendInterface* sender, const BorrowedHandle& socket_fd,
                        const msghdr* msg);
Status<void> RecvAll(RecvInterface* receiver, const BorrowedHandle& socket_fd,
                     void* data, size_t size);
Status<void> RecvMsgAll(RecvInterface* receiver,
                        const BorrowedHandle& socket_fd, msghdr* msg);

#define RETRY_EINTR(fnc_call)                 \
  ([&]() -> decltype(fnc_call) {              \
    decltype(fnc_call) result;                \
    do {                                      \
      result = (fnc_call);                    \
    } while (result == -1 && errno == EINTR); \
    return result;                            \
  })()

class SendPayload : public MessageWriter, public OutputResourceMapper {
 public:
  explicit SendPayload(SendInterface* sender = nullptr) : sender_{sender} {}
  Status<void> Send(const BorrowedHandle& socket_fd);
  Status<void> Send(const BorrowedHandle& socket_fd, const ucred* cred,
                    const iovec* data_vec = nullptr, size_t vec_count = 0);

  // MessageWriter
  void* GetNextWriteBufferSection(size_t size) override;
  OutputResourceMapper* GetOutputResourceMapper() override;

  // OutputResourceMapper
  Status<FileReference> PushFileHandle(const LocalHandle& handle) override;
  Status<FileReference> PushFileHandle(const BorrowedHandle& handle) override;
  Status<FileReference> PushFileHandle(const RemoteHandle& handle) override;
  Status<ChannelReference> PushChannelHandle(
      const LocalChannelHandle& handle) override;
  Status<ChannelReference> PushChannelHandle(
      const BorrowedChannelHandle& handle) override;
  Status<ChannelReference> PushChannelHandle(
      const RemoteChannelHandle& handle) override;

 private:
  SendInterface* sender_;
  ByteBuffer buffer_;
  std::vector<int> file_handles_;
};

class ReceivePayload : public MessageReader, public InputResourceMapper {
 public:
  explicit ReceivePayload(RecvInterface* receiver = nullptr)
      : receiver_{receiver} {}
  Status<void> Receive(const BorrowedHandle& socket_fd);
  Status<void> Receive(const BorrowedHandle& socket_fd, ucred* cred);

  // MessageReader
  BufferSection GetNextReadBufferSection() override;
  void ConsumeReadBufferSectionData(const void* new_start) override;
  InputResourceMapper* GetInputResourceMapper() override;

  // InputResourceMapper
  bool GetFileHandle(FileReference ref, LocalHandle* handle) override;
  bool GetChannelHandle(ChannelReference ref,
                        LocalChannelHandle* handle) override;

 private:
  RecvInterface* receiver_;
  ByteBuffer buffer_;
  std::vector<LocalHandle> file_handles_;
  size_t read_pos_{0};
};

template <typename FileHandleType>
class ChannelInfo {
 public:
  FileHandleType data_fd;
  FileHandleType pollin_event_fd;
  FileHandleType pollhup_event_fd;

 private:
  PDX_SERIALIZABLE_MEMBERS(ChannelInfo, data_fd, pollin_event_fd,
                           pollhup_event_fd);
};

template <typename FileHandleType>
class ChannelConnectionInfo {
 public:
  FileHandleType channel_fd;

 private:
  PDX_SERIALIZABLE_MEMBERS(ChannelConnectionInfo, channel_fd);
};

template <typename FileHandleType>
class RequestHeader {
 public:
  int32_t op{0};
  ucred cred;
  uint32_t send_len{0};
  uint32_t max_recv_len{0};
  std::vector<FileHandleType> file_descriptors;
  std::vector<ChannelInfo<FileHandleType>> channels;
  std::array<uint8_t, 32> impulse_payload;
  bool is_impulse{false};

 private:
  PDX_SERIALIZABLE_MEMBERS(RequestHeader, op, send_len, max_recv_len,
                           file_descriptors, channels, impulse_payload,
                           is_impulse);
};

template <typename FileHandleType>
class ResponseHeader {
 public:
  int32_t ret_code{0};
  uint32_t recv_len{0};
  std::vector<FileHandleType> file_descriptors;
  std::vector<ChannelInfo<FileHandleType>> channels;

 private:
  PDX_SERIALIZABLE_MEMBERS(ResponseHeader, ret_code, recv_len, file_descriptors,
                           channels);
};

template <typename T>
inline Status<void> SendData(const BorrowedHandle& socket_fd, const T& data,
                             const iovec* data_vec = nullptr,
                             size_t vec_count = 0) {
  SendPayload payload;
  rpc::Serialize(data, &payload);
  return payload.Send(socket_fd, nullptr, data_vec, vec_count);
}

template <typename FileHandleType>
inline Status<void> SendData(const BorrowedHandle& socket_fd,
                             const RequestHeader<FileHandleType>& request,
                             const iovec* data_vec = nullptr,
                             size_t vec_count = 0) {
  SendPayload payload;
  rpc::Serialize(request, &payload);
  return payload.Send(socket_fd, &request.cred, data_vec, vec_count);
}

Status<void> SendData(const BorrowedHandle& socket_fd, const void* data,
                      size_t size);
Status<void> SendDataVector(const BorrowedHandle& socket_fd, const iovec* data,
                            size_t count);

template <typename T>
inline Status<void> ReceiveData(const BorrowedHandle& socket_fd, T* data) {
  ReceivePayload payload;
  Status<void> status = payload.Receive(socket_fd);
  if (status && rpc::Deserialize(data, &payload) != rpc::ErrorCode::NO_ERROR)
    status.SetError(EIO);
  return status;
}

template <typename FileHandleType>
inline Status<void> ReceiveData(const BorrowedHandle& socket_fd,
                                RequestHeader<FileHandleType>* request) {
  ReceivePayload payload;
  Status<void> status = payload.Receive(socket_fd, &request->cred);
  if (status && rpc::Deserialize(request, &payload) != rpc::ErrorCode::NO_ERROR)
    status.SetError(EIO);
  return status;
}

Status<void> ReceiveData(const BorrowedHandle& socket_fd, void* data,
                         size_t size);
Status<void> ReceiveDataVector(const BorrowedHandle& socket_fd,
                               const iovec* data, size_t count);

size_t CountVectorSize(const iovec* data, size_t count);
void InitRequest(android::pdx::uds::RequestHeader<BorrowedHandle>* request,
                 int opcode, uint32_t send_len, uint32_t max_recv_len,
                 bool is_impulse);

Status<void> WaitForEndpoint(const std::string& endpoint_path,
                             int64_t timeout_ms);

}  // namespace uds
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_UDS_IPC_HELPER_H_
