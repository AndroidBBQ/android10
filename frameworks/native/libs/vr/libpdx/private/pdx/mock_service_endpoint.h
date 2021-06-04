#ifndef ANDROID_PDX_MOCK_ENDPOINT_H_
#define ANDROID_PDX_MOCK_ENDPOINT_H_

#include <gmock/gmock.h>
#include <pdx/service_endpoint.h>

namespace android {
namespace pdx {

class MockEndpoint : public Endpoint {
 public:
  MOCK_CONST_METHOD0(GetIpcTag, uint32_t());
  MOCK_METHOD1(SetService, Status<void>(Service* service));
  MOCK_METHOD2(SetChannel, Status<void>(int channel_id, Channel* channel));
  MOCK_METHOD1(CloseChannel, Status<void>(int channel_id));
  MOCK_METHOD3(ModifyChannelEvents,
               Status<void>(int channel_id, int clear_mask, int set_mask));
  MOCK_METHOD4(PushChannel,
               Status<RemoteChannelHandle>(Message* message, int flags,
                                           Channel* channel, int* channel_id));
  MOCK_METHOD3(CheckChannel,
               Status<int>(const Message* message, ChannelReference ref,
                           Channel** channel));
  MOCK_METHOD1(MessageReceive, Status<void>(Message* message));
  MOCK_METHOD2(MessageReply, Status<void>(Message* message, int return_code));
  MOCK_METHOD2(MessageReplyFd,
               Status<void>(Message* message, unsigned int push_fd));
  MOCK_METHOD2(MessageReplyChannelHandle,
               Status<void>(Message* message,
                            const LocalChannelHandle& handle));
  MOCK_METHOD2(MessageReplyChannelHandle,
               Status<void>(Message* message,
                            const BorrowedChannelHandle& handle));
  MOCK_METHOD2(MessageReplyChannelHandle,
               Status<void>(Message* message,
                            const RemoteChannelHandle& handle));
  MOCK_METHOD3(ReadMessageData,
               Status<size_t>(Message* message, const iovec* vector,
                              size_t vector_length));
  MOCK_METHOD3(WriteMessageData,
               Status<size_t>(Message* message, const iovec* vector,
                              size_t vector_length));
  MOCK_METHOD2(PushFileHandle,
               Status<FileReference>(Message* message,
                                     const LocalHandle& handle));
  MOCK_METHOD2(PushFileHandle,
               Status<FileReference>(Message* message,
                                     const BorrowedHandle& handle));
  MOCK_METHOD2(PushFileHandle,
               Status<FileReference>(Message* message,
                                     const RemoteHandle& handle));
  MOCK_METHOD2(PushChannelHandle,
               Status<ChannelReference>(Message* message,
                                        const LocalChannelHandle& handle));
  MOCK_METHOD2(PushChannelHandle,
               Status<ChannelReference>(Message* message,
                                        const BorrowedChannelHandle& handle));
  MOCK_METHOD2(PushChannelHandle,
               Status<ChannelReference>(Message* message,
                                        const RemoteChannelHandle& handle));
  MOCK_CONST_METHOD2(GetFileHandle,
                     LocalHandle(Message* message, FileReference ref));
  MOCK_CONST_METHOD2(GetChannelHandle,
                     LocalChannelHandle(Message* message,
                                        ChannelReference ref));
  MOCK_METHOD0(AllocateMessageState, void*());
  MOCK_METHOD1(FreeMessageState, void(void* state));
  MOCK_METHOD0(Cancel, Status<void>());
  MOCK_CONST_METHOD0(epoll_fd, int());
};

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_MOCK_ENDPOINT_H_
