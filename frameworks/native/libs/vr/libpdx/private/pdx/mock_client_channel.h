#ifndef ANDROID_PDX_MOCK_CLIENT_CHANNEL_H_
#define ANDROID_PDX_MOCK_CLIENT_CHANNEL_H_

#include <gmock/gmock.h>
#include <pdx/client_channel.h>

namespace android {
namespace pdx {

class MockClientChannel : public ClientChannel {
 public:
  MOCK_CONST_METHOD0(GetIpcTag, uint32_t());
  MOCK_CONST_METHOD0(event_fd, int());
  MOCK_CONST_METHOD0(GetEventSources, std::vector<EventSource>());
  MOCK_METHOD1(GetEventMask, Status<int>(int));
  MOCK_METHOD0(GetChannelHandle, LocalChannelHandle&());
  MOCK_CONST_METHOD0(GetChannelHandle, const LocalChannelHandle&());
  MOCK_METHOD0(AllocateTransactionState, void*());
  MOCK_METHOD1(FreeTransactionState, void(void* state));
  MOCK_METHOD3(SendImpulse,
               Status<void>(int opcode, const void* buffer, size_t length));
  MOCK_METHOD6(SendWithInt,
               Status<int>(void* transaction_state, int opcode,
                           const iovec* send_vector, size_t send_count,
                           const iovec* receive_vector, size_t receive_count));
  MOCK_METHOD6(SendWithFileHandle,
               Status<LocalHandle>(void* transaction_state, int opcode,
                                   const iovec* send_vector, size_t send_count,
                                   const iovec* receive_vector,
                                   size_t receive_count));
  MOCK_METHOD6(SendWithChannelHandle,
               Status<LocalChannelHandle>(void* transaction_state, int opcode,
                                          const iovec* send_vector,
                                          size_t send_count,
                                          const iovec* receive_vector,
                                          size_t receive_count));
  MOCK_METHOD2(PushFileHandle, FileReference(void* transaction_state,
                                             const LocalHandle& handle));
  MOCK_METHOD2(PushFileHandle, FileReference(void* transaction_state,
                                             const BorrowedHandle& handle));
  MOCK_METHOD2(PushChannelHandle,
               ChannelReference(void* transaction_state,
                                const LocalChannelHandle& handle));
  MOCK_METHOD2(PushChannelHandle,
               ChannelReference(void* transaction_state,
                                const BorrowedChannelHandle& handle));
  MOCK_CONST_METHOD3(GetFileHandle,
                     bool(void* transaction_state, FileReference ref,
                          LocalHandle* handle));
  MOCK_CONST_METHOD3(GetChannelHandle,
                     bool(void* transaction_state, ChannelReference ref,
                          LocalChannelHandle* handle));
  MOCK_METHOD0(TakeChannelParcelable, std::unique_ptr<ChannelParcelable>());
};

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_MOCK_CLIENT_CHANNEL_H_
