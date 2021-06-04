#ifndef ANDROID_PDX_SERVICE_H_
#define ANDROID_PDX_SERVICE_H_

#include <errno.h>
#include <log/log.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "pdx/channel_handle.h"
#include "pdx/file_handle.h"
#include "pdx/message_reader.h"
#include "pdx/message_writer.h"
#include "pdx/service_endpoint.h"

namespace android {
namespace pdx {

class Service;

namespace opcodes {

/*
 * Reserved message opcodes used by libpdx. The reserved opcodes start at the
 * max positive signed integer for the system and go down.
 * In contrast, service opcodes start at zero and go up. This scheme leaves
 * most of the positive integer space for services, a tiny fraction of the
 * positive integer space for the framework, and the entire negative integer
 * space for the kernel.
 */
#define PDX_OPCODE(name, n) name = ((-1U >> 1) - (n))  // 0x7fff..ffff - n

enum {
  // System message sent when a new client channel is open.
  CHANNEL_OPEN = -1,
  // System message sent when a channel is closed.
  CHANNEL_CLOSE = -2,
  // Request the service to reload system properties.
  PDX_OPCODE(REPORT_SYSPROP_CHANGE, 0),
  // Request the service to dump state and return it in a text buffer.
  PDX_OPCODE(DUMP_STATE, 1),
};

}  // namespace opcodes

/*
 * Base class of service-side per-channel context classes.
 */
class Channel : public std::enable_shared_from_this<Channel> {
 public:
  Channel() {}
  virtual ~Channel() {}

  /*
   * Accessors to the pid of the last active client.
   */
  pid_t GetActiveProcessId() const { return client_pid_; }
  void SetActiveProcessId(pid_t pid) { client_pid_ = pid; }

  /*
   * Utility to get a shared_ptr reference from the channel context pointer.
   */
  static std::shared_ptr<Channel> GetFromMessageInfo(const MessageInfo& info);

 private:
  pid_t client_pid_ = 0;
};

/*
 * Message class represents an RPC message, and implicitly the blocked sender
 * waiting for a response. Every message should get a reply, at some point
 * (unless the endpoint is closed), to prevent clients from blocking
 * indefinitely. In order to enforce this and prevent leaking message ids,
 * Message automatically replies with an error to the client on destruction,
 * unless one of two things happens:
 *
 *     1. The service calls one of the reply methods before the Message is
 *        destroyed.
 *     2. The responsibility for the message is moved to another instance of
 *        Message, using either move construction or move assignment.
 *
 * The second case is useful for services that need to delay responding to a
 * sender until a later time. In this situation the service can move the
 * Message to another instance in a suitable data structure for later use. The
 * moved-to Message then takes on the same behavior and responsibilities
 * described above.
 */
class Message : public OutputResourceMapper, public InputResourceMapper {
 public:
  Message();
  explicit Message(const MessageInfo& info);
  ~Message();

  /*
   * Message objects support move construction and assignment.
   */
  Message(Message&& other) noexcept;
  Message& operator=(Message&& other) noexcept;

  /*
   * Read/write payload, in either single buffer or iovec form.
   */
  Status<size_t> ReadVector(const iovec* vector, size_t vector_length);
  Status<size_t> Read(void* buffer, size_t length);
  Status<size_t> WriteVector(const iovec* vector, size_t vector_length);
  Status<size_t> Write(const void* buffer, size_t length);

  template <size_t N>
  inline Status<size_t> ReadVector(const iovec (&vector)[N]) {
    return ReadVector(vector, N);
  }

  template <size_t N>
  inline Status<size_t> WriteVector(const iovec (&vector)[N]) {
    return WriteVector(vector, N);
  }

  // Helper functions to read/write all requested bytes, and return EIO if not
  // all were read/written.
  Status<void> ReadVectorAll(const iovec* vector, size_t vector_length);
  Status<void> WriteVectorAll(const iovec* vector, size_t vector_length);

  inline Status<void> ReadAll(void* buffer, size_t length) {
    Status<size_t> status = Read(buffer, length);
    if (status && status.get() < length)
      status.SetError(EIO);
    Status<void> ret;
    ret.PropagateError(status);
    return ret;
  }
  inline Status<void> WriteAll(const void* buffer, size_t length) {
    Status<size_t> status = Write(buffer, length);
    if (status && status.get() < length)
      status.SetError(EIO);
    Status<void> ret;
    ret.PropagateError(status);
    return ret;
  }

  template <size_t N>
  inline Status<void> ReadVectorAll(const iovec (&vector)[N]) {
    return ReadVectorAll(vector, N);
  }

  template <size_t N>
  inline Status<void> WriteVectorAll(const iovec (&vector)[N]) {
    return WriteVectorAll(vector, N);
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

  /*
   * Various ways to reply to a message.
   */
  Status<void> Reply(int return_code);
  Status<void> ReplyError(unsigned int error);
  Status<void> ReplyFileDescriptor(unsigned int fd);
  Status<void> Reply(const LocalHandle& handle);
  Status<void> Reply(const BorrowedHandle& handle);
  Status<void> Reply(const RemoteHandle& handle);
  Status<void> Reply(const LocalChannelHandle& handle);
  Status<void> Reply(const BorrowedChannelHandle& handle);
  Status<void> Reply(const RemoteChannelHandle& handle);

  template <typename T>
  inline Status<void> Reply(const Status<T>& status) {
    return status ? Reply(status.get()) : ReplyError(status.error());
  }

  inline Status<void> Reply(const Status<void>& status) {
    return status ? Reply(0) : ReplyError(status.error());
  }

  /*
   * Update the channel event bits with the given clear and set masks.
   */
  Status<void> ModifyChannelEvents(int clear_mask, int set_mask);

  /*
   * Create a new channel and push it as a file descriptor to the client. See
   * Service::PushChannel() for a detail description of this method's operation.
   */
  Status<RemoteChannelHandle> PushChannel(
      int flags, const std::shared_ptr<Channel>& channel, int* channel_id);

  /*
   * Create a new channel and push it as a file descriptor to the client. See
   * Service::PushChannel() for a detail description of this method's operation.
   */
  Status<RemoteChannelHandle> PushChannel(
      Service* service, int flags, const std::shared_ptr<Channel>& channel,
      int* channel_id);

  /*
   * Check whether the |ref| is a reference to channel to this service.
   * If the channel reference in question is valid, the Channel object is
   * returned in |channel| when non-nullptr.
   *
   * Return values:
   *  channel_id - id of the channel if the |ref| is a valid reference to
   *               this service's channel.
   * Errors:
   *  EOPNOTSUPP - the file descriptor is not a channel or is a channel to
   *  another service.
   *  EBADF - the file descriptor is invalid.
   *  FAULT - |channel_id| or |channel| are non-nullptr and point to invalid
   *  memory addresses.
   *  EINVAL - the value of |ref| is invalid or the message id for this
   *           message is no longer valid.
   */
  Status<int> CheckChannel(ChannelReference ref,
                           std::shared_ptr<Channel>* channel) const;

  /*
   * Overload of CheckChannel() that checks whether the channel reference is for
   * a channel to the service |service|.
   */
  Status<int> CheckChannel(const Service* service, ChannelReference ref,
                           std::shared_ptr<Channel>* channel) const;

  /*
   * Overload of CheckChannel() that automatically converts to shared pointers
   * to types derived from Channel.
   */
  template <class C>
  Status<int> CheckChannel(ChannelReference ref,
                           std::shared_ptr<C>* channel) const {
    std::shared_ptr<Channel> base_pointer;
    const Status<int> ret =
        CheckChannel(ref, channel ? &base_pointer : nullptr);
    if (channel)
      *channel = std::static_pointer_cast<C>(base_pointer);
    return ret;
  }

  template <class C>
  Status<int> CheckChannel(const Service* service, ChannelReference ref,
                           std::shared_ptr<C>* channel) const {
    std::shared_ptr<Channel> base_pointer;
    const Status<int> ret =
        CheckChannel(service, ref, channel ? &base_pointer : nullptr);
    if (channel)
      *channel = std::static_pointer_cast<C>(base_pointer);
    return ret;
  }

  /*
   * MessageInfo accessors.
   */
  pid_t GetProcessId() const;
  pid_t GetThreadId() const;
  uid_t GetEffectiveUserId() const;
  gid_t GetEffectiveGroupId() const;
  int GetChannelId() const;
  int GetMessageId() const;
  int GetOp() const;
  int GetFlags() const;
  size_t GetSendLength() const;
  size_t GetReceiveLength() const;
  size_t GetFileDescriptorCount() const;

  /*
   * Impulses are asynchronous and cannot be replied to. All impulses have this
   * invalid message id.
   */
  enum { IMPULSE_MESSAGE_ID = -1 };

  /*
   * Returns true if this Message describes an asynchronous "impulse" message.
   */
  bool IsImpulse() const { return GetMessageId() == IMPULSE_MESSAGE_ID; }

  /*
   * Returns a pointer to the impulse payload. Impulses are a maximum of 32
   * bytes in size and the start of the impulse payload is guaranteed to be
   * 8-byte aligned. Use GetSendLength() to determine the payload size.
   */
  const std::uint8_t* ImpulseBegin() const;

  /*
   * Returns one byte past the end of the impulse payload, as conventional for
   * STL iterators.
   */
  const std::uint8_t* ImpulseEnd() const;

  /*
   * Get/set the Channel object for the channel associated
   * with this message. It is up to the caller to synchronize
   * these in multi-threaded services.
   */
  std::shared_ptr<Channel> GetChannel() const;
  Status<void> SetChannel(const std::shared_ptr<Channel>& channnel);

  /*
   * Get the Channel object for the channel associated with this message,
   * automatically converted to the desired subclass of Channel.
   */
  template <class C>
  std::shared_ptr<C> GetChannel() const {
    return std::static_pointer_cast<C>(GetChannel());
  }

  /*
   * Gets the service this message was received on. Returns nullptr if the
   * service was destroyed.
   */
  std::shared_ptr<Service> GetService() const;

  /*
   * Raw access to the MessageInfo for this message.
   */
  const MessageInfo& GetInfo() const;

  bool replied() const { return replied_; }
  bool IsChannelExpired() const { return channel_.expired(); }
  bool IsServiceExpired() const { return service_.expired(); }

  /*
   * Returns true if the message is non-empty; that is the message can be
   * replied to using this instance.
   */
  explicit operator bool() const { return !replied_; }

  const void* GetState() const { return state_; }
  void* GetState() { return state_; }

 private:
  friend class Service;

  Message(const Message&) = delete;
  void operator=(const Message&) = delete;
  void Destroy();

  std::weak_ptr<Service> service_;
  std::weak_ptr<Channel> channel_;
  MessageInfo info_;
  void* state_{nullptr};
  bool replied_;
};

// Base class for RPC services.
class Service : public std::enable_shared_from_this<Service> {
 public:
  Service(const std::string& name, std::unique_ptr<Endpoint> endpoint);
  virtual ~Service();

  /*
   * Utility to get a shared_ptr reference from the service context pointer.
   */
  static std::shared_ptr<Service> GetFromMessageInfo(const MessageInfo& info);

  /*
   * Returns whether initialization was successful. Subclasses that override
   * this must call this base method and AND the results with their own. This
   * method is not intended to do any initialization work itself, only to
   * signal success or failure.
   */
  virtual bool IsInitialized() const;

  /*
   * Called by defaultHandleMessage in response to a CHANNEL_OPEN message.
   * This gives subclasses of Service a convenient hook to create per-channel
   * context in the form of a Channel subclass.
   *
   * The Channel instance returned by this is used to set the channel context
   * pointer for the channel that was just opened.
   */
  virtual std::shared_ptr<Channel> OnChannelOpen(Message& message);

  /*
   * Called by defaultHandleMessage in response to a CHANNEL_CLOSE message.
   * This give subclasses of Service a convenient hook to clean up per-channel
   * context.
   */
  virtual void OnChannelClose(Message& message,
                              const std::shared_ptr<Channel>& channel);

  /*
   * Set the channel context for the given channel. This keeps a reference to
   * the Channel object until the channel is closed or another call replaces
   * the current value.
   */
  Status<void> SetChannel(int channel_id,
                          const std::shared_ptr<Channel>& channel);

  /*
   * Get the channel context for the given channel id. This method should be
   * used sparingly because of the performance characteristics of the underlying
   * map; it is intended for limited, non-critical path access from outside of
   * message dispatch. In most cases lookup by id should be unnecessary in a
   * properly designed service; Message::GetChannel() should be used instead
   * whenever an operation is in the context of a message.
   *
   * Again, if you lookup a channel context object for a service by id in a
   * message handling path for the same service, you're probably doing something
   * wrong.
   */
  std::shared_ptr<Channel> GetChannel(int channel_id) const;

  /*
   * Get a snapshot of the active channels for this service. This is the
   * preferred way to access the set of channels because it avoids potential
   * deadlocks and race conditions that may occur when operating on the channel
   * map directly. However, it is more expensive than direct iteration because
   * of dynamic memory allocation and shared pointer reference costs.
   *
   * Automatically converts returned items to shared pointers of the type
   * std::shared_ptr<C>, where C is the subclass of Channel used by the service.
   */
  template <class C>
  std::vector<std::shared_ptr<C>> GetChannels() const {
    std::lock_guard<std::mutex> autolock(channels_mutex_);
    std::vector<std::shared_ptr<C>> items;
    items.reserve(channels_.size());

    for (const auto& pair : channels_) {
      items.push_back(std::static_pointer_cast<C>(pair.second));
    }

    return items;
  }

  /*
   * Close a channel, signaling the client file object and freeing the channel
   * id. Once closed, the client side of the channel always returns the error
   * ESHUTDOWN and signals the poll/epoll events POLLHUP and POLLFREE.
   *
   * The internal reference to the Channel instance associated with the channel
   * is removed, which may result in the Channel object being freed.
   *
   * OnChannelClosed is not called in response to this method call.
   */
  Status<void> CloseChannel(int channel_id);

  /*
   * Update the event bits for the given channel (given by id), using the
   * given clear and set masks.
   *
   * This is useful for asynchronously signaling events that clients may be
   * waiting for using select/poll/epoll.
   */
  Status<void> ModifyChannelEvents(int channel_id, int clear_mask,
                                   int set_mask);

  /*
   * Create a new channel and push it as a file descriptor to the process
   * sending the |message|. |flags| may be set to O_NONBLOCK and/or
   * O_CLOEXEC to control the initial behavior of the new file descriptor (the
   * sending process may change these later using fcntl()). The internal Channel
   * instance associated with this channel is set to |channel|, which may be
   * nullptr. The new channel id allocated for this channel is returned in
   * |channel_id|, which may also be nullptr if not needed.
   *
   * On success, returns the remote channel handle for the new channel in the
   * sending process' handle space. This MUST be returned to the sender via
   * Message::Reply(), Message::Write(), or Message::WriteVector().
   *
   * On error, returns an errno code describing the cause of the error.
   *
   * Service::OnChannelCreate() is not called in response to the creation of the
   * new channel.
   */
  Status<RemoteChannelHandle> PushChannel(
      Message* message, int flags, const std::shared_ptr<Channel>& channel,
      int* channel_id);

  /*
   * Check whether the |ref| is a reference to a channel to this service.
   * If the channel reference in question is valid, the Channel object is
   * returned in |channel| when non-nullptr.
   *
   * Return values:
   *  channel_id - id of the channel if the channel reference.
   * Errors:
   *  EOPNOTSUPP - the file descriptor is not a channel or is a channel to
   *  another service.
   *  EBADF - the file descriptor is invalid.
   *  FAULT - |channel_id| or |channel| are non-nullptr and point to invalid
   *  memory addresses.
   *  EINVAL - the value of |ref| is invalid or the message id for this
   *  message is no longer valid.
   */
  Status<int> CheckChannel(const Message* message, ChannelReference ref,
                           std::shared_ptr<Channel>* channel) const;

  /*
   * Overload of CheckChannel() that automatically converts to shared pointers
   * of types derived from Channel.
   */
  template <class C>
  Status<int> CheckChannel(const Message* message, ChannelReference ref,
                           std::shared_ptr<C>* channel) const {
    std::shared_ptr<Channel> base_pointer;
    const Status<int> ret =
        CheckChannel(message, ref, channel ? &base_pointer : nullptr);
    if (channel)
      *channel = std::static_pointer_cast<C>(base_pointer);
    return ret;
  }

  /*
   * Handle a message. Subclasses override this to receive messages and decide
   * how to dispatch them.
   *
   * The default implementation simply calls defaultHandleMessage().
   * Subclasses should call the same for any unrecognized message opcodes.
   */
  virtual Status<void> HandleMessage(Message& message);

  /*
   * Handle an asynchronous message. Subclasses override this to receive
   * asynchronous "impulse" messages. Impulses have a limited-size payload that
   * is transferred upfront with the message description.
   */
  virtual void HandleImpulse(Message& impulse);

  /*
   * The default message handler. It is important that all messages
   * (eventually) get a reply. This method should be called by subclasses for
   * any unrecognized opcodes or otherwise unhandled messages to prevent
   * erroneous requests from blocking indefinitely.
   *
   * Provides default handling of CHANNEL_OPEN and CHANNEL_CLOSE, calling
   * OnChannelOpen() and OnChannelClose(), respectively.
   *
   * For all other message opcodes, this method replies with ENOTSUP.
   */
  Status<void> DefaultHandleMessage(Message& message);

  /*
   * Called when system properties have changed. Subclasses should implement
   * this method if they need to handle when system properties change.
   */
  virtual void OnSysPropChange();

  /*
   * Get the endpoint for the service.
   */
  Endpoint* endpoint() const { return endpoint_.get(); }

  /*
   * Cancels the endpoint, unblocking any receiver threads waiting in
   * ReceiveAndDispatch().
   */
  Status<void> Cancel();

  /*
   * Iterator type for Channel map iterators.
   */
  using ChannelIterator =
      std::unordered_map<int, std::shared_ptr<Channel>>::iterator;

  /*
   * Iterates over the Channel map and performs the action given by |action| on
   * each channel map item (const ChannelIterator::value_type).
   * |channels_mutex_| is not held; it is the responsibility of the caller to
   * ensure serialization between threads that modify or iterate over the
   * Channel map.
   */
  template <class A>
  void ForEachChannelUnlocked(A action) const {
    std::for_each(channels_.begin(), channels_.end(), action);
  }

  /*
   * Iterates over the Channel map and performs the action given by |action| on
   * each channel map item (const ChannelIterator::value_type).
   * |channels_mutex_| is held to serialize access to the map; care must be
   * taken to avoid recursively acquiring the mutex, for example, by calling
   * Service::{GetChannel,SetChannel,CloseChannel,PushChannel}() or
   * Message::SetChannel() in the action.
   */
  template <class A>
  void ForEachChannel(A action) const {
    std::lock_guard<std::mutex> autolock(channels_mutex_);
    ForEachChannelUnlocked(action);
  }

  /*
   * Return true if a channel with the given ID exists in the Channel map.
   */
  bool HasChannelId(int channel_id) const {
    std::lock_guard<std::mutex> autolock(channels_mutex_);
    return channels_.find(channel_id) != channels_.end();
  }

  /*
   * Subclasses of Service may override this method to provide a text string
   * describing the state of the service. This method is called by
   * HandleSystemMessage in response to the standard
   * DUMP_STATE message. The string returned to the dump state client is
   * truncated to |max_length| and reflects the maximum size the client can
   * handle.
   */
  virtual std::string DumpState(size_t max_length);

  /*
   * Receives a message on this Service instance's endpoint and dispatches it.
   * If the endpoint is in blocking mode this call blocks until a message is
   * received, a signal is delivered to this thread, or the service is canceled.
   * If the endpoint is in non-blocking mode and a message is not pending this
   * call returns immediately with ETIMEDOUT.
   */
  Status<void> ReceiveAndDispatch();

 private:
  friend class Message;

  Status<void> HandleSystemMessage(Message& message);

  Service(const Service&);
  void operator=(const Service&) = delete;

  const std::string name_;
  std::unique_ptr<Endpoint> endpoint_;

  /*
   * Maintains references to active channels.
   */
  mutable std::mutex channels_mutex_;
  std::unordered_map<int, std::shared_ptr<Channel>> channels_;
};

/*
 * Utility base class for services. This template handles allocation and
 * initialization checks, reducing boiler plate code.
 */
template <typename TYPE>
class ServiceBase : public Service {
 public:
  /*
   * Static service allocation method that check for initialization errors.
   * If errors are encountered these automatically clean up and return
   * nullptr.
   */
  template <typename... Args>
  static inline std::shared_ptr<TYPE> Create(Args&&... args) {
    std::shared_ptr<TYPE> service(new TYPE(std::forward<Args>(args)...));
    if (service->IsInitialized())
      return service;
    else
      return nullptr;
  }

 protected:
  /*
   * Shorthand for subclasses to refer to this base, particularly
   * to call the base class constructor.
   */
  typedef ServiceBase<TYPE> BASE;

  ServiceBase(const std::string& name, std::unique_ptr<Endpoint> endpoint)
      : Service(name, std::move(endpoint)) {}
};

#ifndef STRINGIFY
#define STRINGIFY2(s) #s
#define STRINGIFY(s) STRINGIFY2(s)
#endif

#define PDX_ERROR_PREFIX "[" __FILE__ ":" STRINGIFY(__LINE__) "]"

/*
 * Macros for replying to messages. Error handling can be tedious;
 * these macros make things a little cleaner.
 */
#define CHECK_ERROR(cond, error, fmt, ...) \
  do {                                     \
    if ((cond)) {                          \
      ALOGE(fmt, ##__VA_ARGS__);           \
      goto error;                          \
    }                                      \
  } while (0)

#define REPLY_ERROR(message, error, error_label)                              \
  do {                                                                        \
    auto __status = message.ReplyError(error);                                \
    CHECK_ERROR(!__status, error_label,                                       \
                PDX_ERROR_PREFIX " Failed to reply to message because: %s\n", \
                __status.GetErrorMessage().c_str());                          \
    goto error_label;                                                         \
  } while (0)

#define REPLY_ERROR_RETURN(message, error, ...)                          \
  do {                                                                   \
    auto __status = message.ReplyError(error);                           \
    ALOGE_IF(!__status,                                                  \
             PDX_ERROR_PREFIX " Failed to reply to message because: %s", \
             __status.GetErrorMessage().c_str());                        \
    return __VA_ARGS__;                                                  \
  } while (0)

#define REPLY_MESSAGE(message, message_return_code, error_label)              \
  do {                                                                        \
    auto __status = message.Reply(message_return_code);                       \
    CHECK_ERROR(!__status, error_label,                                       \
                PDX_ERROR_PREFIX " Failed to reply to message because: %s\n", \
                __status.GetErrorMessage().c_str());                          \
    goto error_label;                                                         \
  } while (0)

#define REPLY_SUCCESS(message, message_return_code, error_label) \
  REPLY_MESSAGE(message, message_return_code, error_label)

#define REPLY_MESSAGE_RETURN(message, message_return_code, ...)          \
  do {                                                                   \
    auto __status = message.Reply(message_return_code);                  \
    ALOGE_IF(!__status,                                                  \
             PDX_ERROR_PREFIX " Failed to reply to message because: %s", \
             __status.GetErrorMessage().c_str());                        \
    return __VA_ARGS__;                                                  \
  } while (0)

#define REPLY_SUCCESS_RETURN(message, message_return_code, ...) \
  REPLY_MESSAGE_RETURN(message, message_return_code, __VA_ARGS__)

#define REPLY_FD(message, push_fd, error_label)                               \
  do {                                                                        \
    auto __status = message.ReplyFileDescriptor(push_fd);                     \
    CHECK_ERROR(!__status, error_label,                                       \
                PDX_ERROR_PREFIX " Failed to reply to message because: %s\n", \
                __status.GetErrorMessage().c_str());                          \
    goto error_label;                                                         \
  } while (0)

#define REPLY_FD_RETURN(message, push_fd, ...)                           \
  do {                                                                   \
    auto __status = message.ReplyFileDescriptor(push_fd);                \
    ALOGE_IF(__status < 0,                                               \
             PDX_ERROR_PREFIX " Failed to reply to message because: %s", \
             __status.GetErrorMessage().c_str());                        \
    return __VA_ARGS__;                                                  \
  } while (0)

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_SERVICE_H_
