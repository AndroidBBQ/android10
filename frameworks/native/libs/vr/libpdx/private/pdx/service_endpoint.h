#ifndef ANDROID_PDX_ENDPOINT_H_
#define ANDROID_PDX_ENDPOINT_H_

#include <pdx/channel_handle.h>
#include <pdx/file_handle.h>
#include <pdx/status.h>

struct iovec;

namespace android {
namespace pdx {

class Service;
class Channel;
class Message;

struct MessageInfo {
  int pid{0};
  int tid{0};
  int cid{0};
  int mid{0};
  int euid{0};
  int egid{0};
  int32_t op{0};
  uint32_t flags{0};
  Service* service{nullptr};
  Channel* channel{nullptr};
  size_t send_len{0};
  size_t recv_len{0};
  size_t fd_count{0};
  uint64_t impulse[4] = {};
};

// Wrapper around transport endpoint. Abstracts the underlying transport APIs in
// a way, that the underlying IPC can be substituted for another technology
// without changing the Service, Client and Message classes of this library.
class Endpoint {
 public:
  virtual ~Endpoint() = default;

  // Returns a tag that uniquely identifies a specific underlying IPC transport.
  virtual uint32_t GetIpcTag() const = 0;

  // Associates a Service instance with an endpoint by setting the service
  // context pointer to the address of the Service. Only one Service may be
  // associated with a given endpoint.
  virtual Status<void> SetService(Service* service) = 0;

  // Set the channel context for the given channel.
  virtual Status<void> SetChannel(int channel_id, Channel* channel) = 0;

  // Close a channel, signaling the client file object and freeing the channel
  // id. Once closed, the client side of the channel always returns the error
  // ESHUTDOWN and signals the poll/epoll events POLLHUP and POLLFREE.
  virtual Status<void> CloseChannel(int channel_id) = 0;

  // Update the event bits for the given channel (given by id), using the
  // given clear and set masks.
  virtual Status<void> ModifyChannelEvents(int channel_id, int clear_mask,
                                           int set_mask) = 0;

  // Create a new channel and push it as a file descriptor to the process
  // sending the |message|. |flags| may be set to O_NONBLOCK and/or
  // O_CLOEXEC to control the initial behavior of the new file descriptor (the
  // sending process may change these later using fcntl()). The internal Channel
  // instance associated with this channel is set to |channel|, which may be
  // nullptr. The new channel id allocated for this channel is returned in
  // |channel_id|, which may also be nullptr if not needed.
  virtual Status<RemoteChannelHandle> PushChannel(Message* message, int flags,
                                                  Channel* channel,
                                                  int* channel_id) = 0;

  // Check whether the |ref| is a reference to a channel to the service
  // represented by the |endpoint|. If the channel reference in question is
  // valid, the Channel object is returned in |channel| when non-nullptr and
  // the channel ID is returned through the Status object.
  virtual Status<int> CheckChannel(const Message* message, ChannelReference ref,
                                   Channel** channel) = 0;

  // Receives a message on the given endpoint file descriptor.
  virtual Status<void> MessageReceive(Message* message) = 0;

  // Replies to the message with a return code.
  virtual Status<void> MessageReply(Message* message, int return_code) = 0;

  // Replies to the message with a file descriptor.
  virtual Status<void> MessageReplyFd(Message* message,
                                      unsigned int push_fd) = 0;

  // Replies to the message with a local channel handle.
  virtual Status<void> MessageReplyChannelHandle(
      Message* message, const LocalChannelHandle& handle) = 0;

  // Replies to the message with a borrowed local channel handle.
  virtual Status<void> MessageReplyChannelHandle(
      Message* message, const BorrowedChannelHandle& handle) = 0;

  // Replies to the message with a remote channel handle.
  virtual Status<void> MessageReplyChannelHandle(
      Message* message, const RemoteChannelHandle& handle) = 0;

  // Reads message data into an array of memory buffers.
  virtual Status<size_t> ReadMessageData(Message* message, const iovec* vector,
                                         size_t vector_length) = 0;

  // Sends reply data for message.
  virtual Status<size_t> WriteMessageData(Message* message, const iovec* vector,
                                          size_t vector_length) = 0;

  // Records a file descriptor into the message buffer and returns the remapped
  // reference to be sent to the remote process.
  virtual Status<FileReference> PushFileHandle(Message* message,
                                               const LocalHandle& handle) = 0;
  virtual Status<FileReference> PushFileHandle(
      Message* message, const BorrowedHandle& handle) = 0;
  virtual Status<FileReference> PushFileHandle(Message* message,
                                               const RemoteHandle& handle) = 0;
  virtual Status<ChannelReference> PushChannelHandle(
      Message* message, const LocalChannelHandle& handle) = 0;
  virtual Status<ChannelReference> PushChannelHandle(
      Message* message, const BorrowedChannelHandle& handle) = 0;
  virtual Status<ChannelReference> PushChannelHandle(
      Message* message, const RemoteChannelHandle& handle) = 0;

  // Obtains a file descriptor/channel handle from a message for the given
  // reference.
  virtual LocalHandle GetFileHandle(Message* message,
                                    FileReference ref) const = 0;
  virtual LocalChannelHandle GetChannelHandle(Message* message,
                                              ChannelReference ref) const = 0;

  // Transport-specific message state management.
  virtual void* AllocateMessageState() = 0;
  virtual void FreeMessageState(void* state) = 0;

  // Cancels the endpoint, unblocking any receiver threads waiting for a
  // message.
  virtual Status<void> Cancel() = 0;

  // Returns an fd that can be used with epoll() to wait for incoming messages
  // from this endpoint.
  virtual int epoll_fd() const = 0;
};

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_ENDPOINT_H_
