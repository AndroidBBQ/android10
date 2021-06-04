#ifndef ANDROID_PDX_CLIENT_H_
#define ANDROID_PDX_CLIENT_H_

#include <errno.h>
#include <sys/types.h>

#include <memory>
#include <string>
#include <type_traits>

#include <pdx/channel_handle.h>
#include <pdx/client_channel.h>
#include <pdx/client_channel_factory.h>
#include <pdx/file_handle.h>
#include <pdx/message_reader.h>
#include <pdx/message_writer.h>
#include <pdx/rpc/remote_method_type.h>
#include <pdx/status.h>

namespace android {
namespace pdx {

class Transaction;

/*
 * Base class of client-side service API classes.
 */
class Client {
 public:
  static const int64_t kInfiniteTimeout = -1;

  virtual ~Client() = default;

  /*
   * Returns true if the Client instance successfully initialized, false
   * otherwise. Subclasses that can fail to initialize must override this and
   * AND their initialization result with this base class method's result.
   *
   * This method is not intended to perform initialization, only to report
   * the status of the initialization.
   */
  virtual bool IsInitialized() const;

  /*
   * Returns the error code describing the Client initialization failure, or 0
   * if there was no failure.
   */
  int error() const;

  // Returns a reference to IPC channel handle.
  LocalChannelHandle& GetChannelHandle();
  const LocalChannelHandle& GetChannelHandle() const;

 protected:
  friend Transaction;
  explicit Client(std::unique_ptr<ClientChannel> channel);
  explicit Client(std::unique_ptr<ClientChannelFactory> channel_factory,
                  int64_t timeout_ms = kInfiniteTimeout);

  /*
   * Called by Client::Connect() after successfully connecting to the service
   * endpoint. Subclasses may override this method to perform additional setup,
   * including sending messages to complete the connection process.
   *
   * Subclasses may call Client::Close() within this method to terminate the
   * connection; Client::Connect() returns the negated error passed to
   * Client::Close() when this happens.
   */
  virtual void OnConnect();

  enum : size_t { MAX_IMPULSE_LENGTH = sizeof(uint64_t) * 4 };

  Status<void> SendImpulse(int opcode);
  Status<void> SendImpulse(int opcode, const void* buffer, size_t length);

  /*
   * Remote method call API using pdx::rpc serialization.
   * Include pdx/rpc/remote_method.h to use these methods.
   */
  template <typename RemoteMethodType, typename... Args>
  Status<typename RemoteMethodType::Return> InvokeRemoteMethod(Args&&... args);

  template <typename RemoteMethodType, typename ReturnType, typename... Args>
  Status<void> InvokeRemoteMethodInPlace(ReturnType* return_value,
                                         Args&&... args);

  /*
   * Close the endpoint file descriptor and optionally indicate an error, which
   * may be retrieved through error(). Subclasses may use this in their
   * constructor to signal failure during initialization or at other times
   * during operation.
   */
  void Close(int error);

  /*
   * Returns true if the client is connected to the service, false otherwise.
   */
  bool IsConnected() const;

  /*
   * Enables auto-reconnect with the given timeout. Use kInfiniteTimeout (-1)
   * for no timeout. Auto-reconnect can only be enabled if the Client class
   * was constructed with a ClientChannelFactory.
   */
  void EnableAutoReconnect(int64_t reconnect_timeout_ms);

  /*
   * Disables auto-reconnect.
   */
  void DisableAutoReconnect();

  /*
   * Returns an fd that the client may use to check/wait for asynchronous
   * notifications to the channel. It is implementation dependent how the
   * transport backend handles this feature, however all implementations must
   * support at least POLLIN/EPOLLIN/readable.
   *
   * For uses that require more than one type of event, use
   * ClientChannel::GetEventMask() to distinguish between events.
   */
  int event_fd() const;

  /*
   * Returns the underlying ClientChannel object.
   */
  ClientChannel* GetChannel() const { return channel_.get(); }
  std::unique_ptr<ClientChannel>&& TakeChannel() { return std::move(channel_); }

 private:
  Client(const Client&) = delete;
  void operator=(const Client&) = delete;

  Status<void> CheckReconnect();
  bool NeedToDisconnectChannel(int error) const;
  void CheckDisconnect(int error);

  template <typename T>
  inline void CheckDisconnect(const Status<T>& status) {
    if (!status)
      CheckDisconnect(status.error());
  }

  std::unique_ptr<ClientChannel> channel_;
  int error_{0};

  // Reconnection state.
  std::unique_ptr<ClientChannelFactory> channel_factory_;
  int64_t reconnect_timeout_ms_{0};
  bool auto_reconnect_enabled_{false};
};

/*
 * Utility template base class for client-side service API classes. Handles
 * initialization checks during allocation and automatically cleans up on
 * failure.
 *
 * @tparam T Type of the class extending this one.
 * @tparam C Client class to wrap. Defaults to the Client class.
 */
template <typename T, typename ParentClient = Client>
class ClientBase : public ParentClient {
 public:
  // Type of the client this class wraps.
  using ClientType = ParentClient;

  static_assert(std::is_base_of<Client, ParentClient>::value,
                "The provided parent client is not a Client subclass.");

  /*
   * Allocates a new instance of the superclass and checks for successful
   * initialization.
   *
   * The variadic arguments must expand to match one of type T's constructors
   * and are passed through unchanged. If a timeout is desired, subclasses are
   * responsible for passing this through to the appropriate ClientBase
   * constructor.
   *
   * Returns a unique_ptr to the new instance on success, or an empty unique_ptr
   * otherwise.
   */
  template <typename... Args>
  static inline std::unique_ptr<T> Create(Args&&... args) {
    std::unique_ptr<T> client(new T(std::forward<Args>(args)...));
    if (client->IsInitialized())
      return client;
    else
      return nullptr;
  }

 protected:
  /*
   * Type of the base class. Useful for referencing the base class type and
   * constructor in subclasses. Subclasses with non-public constructors
   * must declare BASE a friend.
   */
  using BASE = ClientBase<T, ParentClient>;

  /*
   * Type of the unique_ptr deleter. Useful for friend declarations.
   */
  using deleter_type = typename std::unique_ptr<T>::deleter_type;

  using ParentClient::ParentClient;
};

class Transaction final : public OutputResourceMapper,
                          public InputResourceMapper {
 public:
  explicit Transaction(Client& client);
  ~Transaction();

  template <typename T>
  Status<T> Send(int opcode) {
    return SendVector<T>(opcode, nullptr, 0, nullptr, 0);
  }

  template <typename T>
  Status<T> Send(int opcode, const void* send_buffer, size_t send_length,
                 void* receive_buffer, size_t receive_length) {
    const bool send = (send_buffer && send_length);
    const bool receive = (receive_buffer && receive_length);
    const iovec send_vector = {const_cast<void*>(send_buffer), send_length};
    const iovec receive_vector = {receive_buffer, receive_length};
    return SendVector<T>(opcode, send ? &send_vector : nullptr, send ? 1 : 0,
                         receive ? &receive_vector : nullptr, receive ? 1 : 0);
  }

  template <typename T>
  Status<T> SendVector(int opcode, const iovec* send_vector, size_t send_count,
                       const iovec* receive_vector, size_t receive_count) {
    Status<T> ret;
    SendTransaction(opcode, &ret, send_vector, send_count, receive_vector,
                    receive_count);
    return ret;
  }

  template <typename T, size_t send_count, size_t receive_count>
  Status<T> SendVector(int opcode, const iovec (&send_vector)[send_count],
                       const iovec (&receive_vector)[receive_count]) {
    return SendVector<T>(opcode, send_vector, send_count, receive_vector,
                         receive_count);
  }

  template <typename T, size_t send_count>
  Status<T> SendVector(int opcode, const iovec (&send_vector)[send_count],
                       std::nullptr_t) {
    return SendVector<T>(opcode, send_vector, send_count, nullptr, 0);
  }

  template <typename T, size_t receive_count>
  Status<T> SendVector(int opcode, std::nullptr_t,
                       const iovec (&receive_vector)[receive_count]) {
    return SendVector<T>(opcode, nullptr, 0, receive_vector, receive_count);
  }

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

  // InputResourceMapper
  bool GetFileHandle(FileReference ref, LocalHandle* handle) override;
  bool GetChannelHandle(ChannelReference ref,
                        LocalChannelHandle* handle) override;

 private:
  bool EnsureStateAllocated();
  void SendTransaction(int opcode, Status<void>* ret, const iovec* send_vector,
                       size_t send_count, const iovec* receive_vector,
                       size_t receive_count);
  void SendTransaction(int opcode, Status<int>* ret, const iovec* send_vector,
                       size_t send_count, const iovec* receive_vector,
                       size_t receive_count);
  void SendTransaction(int opcode, Status<LocalHandle>* ret,
                       const iovec* send_vector, size_t send_count,
                       const iovec* receive_vector, size_t receive_count);
  void SendTransaction(int opcode, Status<LocalChannelHandle>* ret,
                       const iovec* send_vector, size_t send_count,
                       const iovec* receive_vector, size_t receive_count);
  void CheckDisconnect(int error);

  template <typename T>
  inline void CheckDisconnect(const Status<T>& status) {
    if (!status)
      CheckDisconnect(status.error());
  }

  Client& client_;
  void* state_{nullptr};
  bool state_allocated_{false};
};

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_CLIENT_H_
