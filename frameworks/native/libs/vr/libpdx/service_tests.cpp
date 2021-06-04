#include <pdx/service.h>

#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <pdx/mock_service_endpoint.h>

using android::pdx::BorrowedChannelHandle;
using android::pdx::BorrowedHandle;
using android::pdx::Channel;
using android::pdx::ChannelReference;
using android::pdx::ErrorStatus;
using android::pdx::FileReference;
using android::pdx::LocalChannelHandle;
using android::pdx::LocalHandle;
using android::pdx::Message;
using android::pdx::MessageInfo;
using android::pdx::MockEndpoint;
using android::pdx::RemoteChannelHandle;
using android::pdx::RemoteHandle;
using android::pdx::Service;
using android::pdx::Status;

using testing::A;
using testing::ByMove;
using testing::DoAll;
using testing::Invoke;
using testing::Matcher;
using testing::Ref;
using testing::Return;
using testing::SetArgPointee;
using testing::WithArg;
using testing::WithoutArgs;
using testing::_;

namespace {

// Helper functions to construct fake void pointers for tests.
inline void* IntToPtr(intptr_t addr) { return reinterpret_cast<void*>(addr); }

// Helper matchers for working with iovec structures in tests.
// Simple matcher for one element iovec array:
// EXPECT_CALL(mock, method(IoVecMatcher(ptr, size)));
MATCHER_P2(IoVecMatcher, ptr, size, "") {
  return arg->iov_base == ptr && arg->iov_len == size;
}

// Matcher for an array of iovecs:
// EXPECT_CALL(mock,
//             method(IoVecMatcher(IoVecArray{{ptr1, size1}, {ptr2, size2}})));
using IoVecArray = std::vector<iovec>;
MATCHER_P(IoVecMatcher, iovec_array, "") {
  auto local_arg = arg;
  for (const iovec& item : iovec_array) {
    if (local_arg->iov_base != item.iov_base || local_arg->iov_len != item.iov_len)
      return false;
    local_arg++;
  }
  return true;
}

using IoVecData = std::vector<std::string>;
MATCHER_P(IoVecDataMatcher, iovec_data, "") {
  auto local_arg = arg;
  for (const std::string& item : iovec_data) {
    std::string data{reinterpret_cast<const char*>(local_arg->iov_base),
                     local_arg->iov_len};
    if (data != item)
      return false;
    local_arg++;
  }
  return true;
}

MATCHER_P(FileHandleMatcher, value, "") { return arg.Get() == value; }
MATCHER_P(ChannelHandleMatcher, value, "") { return arg.value() == value; }

enum : int {
  kTestPid = 1,
  kTestTid,
  kTestCid,
  kTestMid,
  kTestEuid,
  kTestEgid,
  kTestOp,
};

class MockService : public Service {
 public:
  using Service::Service;
  MOCK_METHOD1(OnChannelOpen, std::shared_ptr<Channel>(Message& message));
  MOCK_METHOD2(OnChannelClose,
               void(Message& message, const std::shared_ptr<Channel>& channel));
  MOCK_METHOD1(HandleMessage, Status<void>(Message& message));
  MOCK_METHOD1(HandleImpulse, void(Message& impulse));
  MOCK_METHOD0(OnSysPropChange, void());
  MOCK_METHOD1(DumpState, std::string(size_t max_length));
};

class ServiceTest : public testing::Test {
 public:
  ServiceTest() {
    auto endpoint = std::make_unique<testing::StrictMock<MockEndpoint>>();
    EXPECT_CALL(*endpoint, SetService(_))
        .Times(2)
        .WillRepeatedly(Return(Status<void>{}));
    service_ = std::make_shared<MockService>("MockSvc", std::move(endpoint));
  }

  MockEndpoint* endpoint() {
    return static_cast<MockEndpoint*>(service_->endpoint());
  }

  void SetupMessageInfo(MessageInfo* info, int32_t op, bool impulse = false) {
    info->pid = kTestPid;
    info->tid = kTestTid;
    info->cid = kTestCid;
    info->mid = impulse ? Message::IMPULSE_MESSAGE_ID : kTestMid;
    info->euid = kTestEuid;
    info->egid = kTestEgid;
    info->op = op;
    info->flags = 0;
    info->service = service_.get();
    info->channel = nullptr;
    info->send_len = 0;
    info->recv_len = 0;
    info->fd_count = 0;
    memset(info->impulse, 0, sizeof(info->impulse));
  }

  void SetupMessageInfoAndDefaultExpectations(MessageInfo* info, int32_t op,
                                              bool impulse = false) {
    SetupMessageInfo(info, op, impulse);
    EXPECT_CALL(*endpoint(), AllocateMessageState()).WillOnce(Return(kState));
    EXPECT_CALL(*endpoint(), FreeMessageState(kState));
  }

  void ExpectDefaultHandleMessage() {
    EXPECT_CALL(*endpoint(), MessageReply(_, -EOPNOTSUPP))
        .WillOnce(Return(Status<void>{}));
  }

  std::shared_ptr<MockService> service_;
  void* kState = IntToPtr(123456);
};

class ServiceMessageTest : public ServiceTest {
 public:
  ServiceMessageTest() {
    MessageInfo info;
    SetupMessageInfoAndDefaultExpectations(&info, kTestOp);
    message_ = std::make_unique<Message>(info);
  }

  std::unique_ptr<Message> message_;
};

}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////
// Service class tests
///////////////////////////////////////////////////////////////////////////////

TEST_F(ServiceTest, IsInitialized) {
  EXPECT_TRUE(service_->IsInitialized());
  service_ = std::make_shared<MockService>("MockSvc2", nullptr);
  EXPECT_FALSE(service_->IsInitialized());
}

TEST_F(ServiceTest, ConstructMessage) {
  MessageInfo info;
  SetupMessageInfo(&info, kTestOp);
  auto test_channel = std::make_shared<Channel>();
  info.channel = test_channel.get();
  EXPECT_CALL(*endpoint(), AllocateMessageState()).WillOnce(Return(kState));

  Message message{info};

  EXPECT_FALSE(message.IsImpulse());
  EXPECT_EQ(kTestPid, message.GetProcessId());
  EXPECT_EQ(kTestTid, message.GetThreadId());
  EXPECT_EQ(kTestCid, message.GetChannelId());
  EXPECT_EQ(kTestMid, message.GetMessageId());
  EXPECT_EQ((unsigned) kTestEuid, message.GetEffectiveUserId());
  EXPECT_EQ((unsigned) kTestEgid, message.GetEffectiveGroupId());
  EXPECT_EQ(kTestOp, message.GetOp());
  EXPECT_EQ(service_, message.GetService());
  EXPECT_EQ(test_channel, message.GetChannel());
  EXPECT_FALSE(message.replied());
  EXPECT_FALSE(message.IsChannelExpired());
  EXPECT_FALSE(message.IsServiceExpired());
  EXPECT_EQ(kState, message.GetState());

  ExpectDefaultHandleMessage();
  EXPECT_CALL(*endpoint(), FreeMessageState(kState));
}

TEST_F(ServiceTest, ConstructImpulseMessage) {
  MessageInfo info;
  SetupMessageInfo(&info, kTestOp, true);
  auto test_channel = std::make_shared<Channel>();
  info.channel = test_channel.get();
  EXPECT_CALL(*endpoint(), AllocateMessageState()).WillOnce(Return(nullptr));

  Message message{info};

  EXPECT_TRUE(message.IsImpulse());
  EXPECT_EQ(kTestOp, message.GetOp());
  EXPECT_EQ(service_, message.GetService());
  EXPECT_EQ(test_channel, message.GetChannel());
  EXPECT_TRUE(message.replied());
  EXPECT_FALSE(message.IsChannelExpired());
  EXPECT_FALSE(message.IsServiceExpired());

  // DefaultHandleMessage should not be called here.
  EXPECT_CALL(*endpoint(), FreeMessageState(nullptr));
}

TEST_F(ServiceTest, HandleMessageChannelOpen) {
  MessageInfo info;
  SetupMessageInfoAndDefaultExpectations(&info,
                                         android::pdx::opcodes::CHANNEL_OPEN);
  Message message{info};

  auto channel = std::make_shared<Channel>();
  EXPECT_CALL(*service_, OnChannelOpen(Ref(message))).WillOnce(Return(channel));
  EXPECT_CALL(*endpoint(), SetChannel(kTestCid, channel.get()))
      .WillOnce(Return(Status<void>{}));
  EXPECT_CALL(*endpoint(), MessageReply(&message, 0))
      .WillOnce(Return(Status<void>{}));

  EXPECT_TRUE(service_->Service::HandleMessage(message));
}

TEST_F(ServiceTest, HandleMessageChannelClose) {
  MessageInfo info;
  SetupMessageInfoAndDefaultExpectations(&info,
                                         android::pdx::opcodes::CHANNEL_CLOSE);
  auto channel = std::make_shared<Channel>();
  info.channel = channel.get();
  Message message{info};

  EXPECT_CALL(*service_, OnChannelClose(Ref(message), channel));
  EXPECT_CALL(*endpoint(), SetChannel(kTestCid, nullptr))
      .WillOnce(Return(Status<void>{}));
  EXPECT_CALL(*endpoint(), MessageReply(&message, 0))
      .WillOnce(Return(Status<void>{}));

  EXPECT_TRUE(service_->Service::HandleMessage(message));
}

TEST_F(ServiceTest, HandleMessageOnSysPropChange) {
  MessageInfo info;
  SetupMessageInfoAndDefaultExpectations(
      &info, android::pdx::opcodes::REPORT_SYSPROP_CHANGE);
  Message message{info};

  EXPECT_CALL(*service_, OnSysPropChange());
  EXPECT_CALL(*endpoint(), MessageReply(&message, 0))
      .WillOnce(Return(Status<void>{}));

  EXPECT_TRUE(service_->Service::HandleMessage(message));
}

TEST_F(ServiceTest, HandleMessageOnDumpState) {
  const size_t kRecvBufSize = 1000;
  MessageInfo info;
  SetupMessageInfoAndDefaultExpectations(&info,
                                         android::pdx::opcodes::DUMP_STATE);
  info.recv_len = kRecvBufSize;
  Message message{info};

  const std::string kReply = "foo";
  EXPECT_CALL(*service_, DumpState(kRecvBufSize)).WillOnce(Return(kReply));
  EXPECT_CALL(
      *endpoint(),
      WriteMessageData(&message, IoVecDataMatcher(IoVecData{kReply}), 1))
      .WillOnce(Return(kReply.size()));
  EXPECT_CALL(*endpoint(), MessageReply(&message, kReply.size()))
      .WillOnce(Return(Status<void>{}));

  EXPECT_TRUE(service_->Service::HandleMessage(message));
}

TEST_F(ServiceTest, HandleMessageOnDumpStateTooLarge) {
  const size_t kRecvBufSize = 3;
  MessageInfo info;
  SetupMessageInfoAndDefaultExpectations(&info,
                                         android::pdx::opcodes::DUMP_STATE);
  info.recv_len = kRecvBufSize;
  Message message{info};

  const std::string kReply = "0123456789";
  const std::string kActualReply = kReply.substr(0, kRecvBufSize);
  EXPECT_CALL(*service_, DumpState(kRecvBufSize)).WillOnce(Return(kReply));
  EXPECT_CALL(
      *endpoint(),
      WriteMessageData(&message, IoVecDataMatcher(IoVecData{kActualReply}), 1))
      .WillOnce(Return(kActualReply.size()));
  EXPECT_CALL(*endpoint(), MessageReply(&message, kActualReply.size()))
      .WillOnce(Return(Status<void>{}));

  EXPECT_TRUE(service_->Service::HandleMessage(message));
}

TEST_F(ServiceTest, HandleMessageOnDumpStateFail) {
  const size_t kRecvBufSize = 1000;
  MessageInfo info;
  SetupMessageInfoAndDefaultExpectations(&info,
                                         android::pdx::opcodes::DUMP_STATE);
  info.recv_len = kRecvBufSize;
  Message message{info};

  const std::string kReply = "foo";
  EXPECT_CALL(*service_, DumpState(kRecvBufSize)).WillOnce(Return(kReply));
  EXPECT_CALL(
      *endpoint(),
      WriteMessageData(&message, IoVecDataMatcher(IoVecData{kReply}), 1))
      .WillOnce(Return(1));
  EXPECT_CALL(*endpoint(), MessageReply(&message, -EIO))
      .WillOnce(Return(Status<void>{}));

  EXPECT_TRUE(service_->Service::HandleMessage(message));
}

TEST_F(ServiceTest, HandleMessageCustom) {
  MessageInfo info;
  SetupMessageInfoAndDefaultExpectations(&info, kTestOp);
  Message message{info};

  EXPECT_CALL(*endpoint(), MessageReply(&message, -EOPNOTSUPP))
      .WillOnce(Return(Status<void>{}));

  EXPECT_TRUE(service_->Service::HandleMessage(message));
}

TEST_F(ServiceTest, ReplyMessageWithoutService) {
  MessageInfo info;
  SetupMessageInfo(&info, kTestOp);
  EXPECT_CALL(*endpoint(), AllocateMessageState()).WillOnce(Return(nullptr));

  Message message{info};

  EXPECT_FALSE(message.IsServiceExpired());
  service_.reset();
  EXPECT_TRUE(message.IsServiceExpired());

  EXPECT_EQ(EINVAL, message.Reply(12).error());
}

TEST_F(ServiceTest, ReceiveAndDispatchMessage) {
  MessageInfo info;
  SetupMessageInfoAndDefaultExpectations(&info, kTestOp);
  ExpectDefaultHandleMessage();

  auto on_receive = [&info](Message* message) -> Status<void> {
    *message = Message{info};
    return {};
  };
  EXPECT_CALL(*endpoint(), MessageReceive(_)).WillOnce(Invoke(on_receive));
  EXPECT_CALL(*service_, HandleMessage(_)).WillOnce(Return(Status<void>{}));

  EXPECT_TRUE(service_->ReceiveAndDispatch());
}

TEST_F(ServiceTest, ReceiveAndDispatchImpulse) {
  MessageInfo info;
  SetupMessageInfoAndDefaultExpectations(&info, kTestOp, true);

  auto on_receive = [&info](Message* message) -> Status<void> {
    *message = Message{info};
    return {};
  };
  EXPECT_CALL(*endpoint(), MessageReceive(_)).WillOnce(Invoke(on_receive));
  EXPECT_CALL(*service_, HandleImpulse(_));

  EXPECT_TRUE(service_->ReceiveAndDispatch());
}

TEST_F(ServiceTest, Cancel) {
  EXPECT_CALL(*endpoint(), Cancel()).WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(service_->Cancel());
}

///////////////////////////////////////////////////////////////////////////////
// Message class tests
///////////////////////////////////////////////////////////////////////////////

TEST_F(ServiceMessageTest, Reply) {
  EXPECT_CALL(*endpoint(), MessageReply(message_.get(), 12))
      .WillOnce(Return(Status<void>{}));
  EXPECT_FALSE(message_->replied());
  EXPECT_TRUE(message_->Reply(12));
  EXPECT_TRUE(message_->replied());

  EXPECT_EQ(EINVAL, message_->Reply(12).error());  // Already replied.
}

TEST_F(ServiceMessageTest, ReplyFail) {
  EXPECT_CALL(*endpoint(), MessageReply(message_.get(), 12))
      .WillOnce(Return(ErrorStatus{EIO}));
  EXPECT_EQ(EIO, message_->Reply(12).error());

  ExpectDefaultHandleMessage();
}

TEST_F(ServiceMessageTest, ReplyError) {
  EXPECT_CALL(*endpoint(), MessageReply(message_.get(), -12))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->ReplyError(12));
}

TEST_F(ServiceMessageTest, ReplyFileDescriptor) {
  EXPECT_CALL(*endpoint(), MessageReplyFd(message_.get(), 5))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->ReplyFileDescriptor(5));
}

TEST_F(ServiceMessageTest, ReplyLocalFileHandle) {
  const int kFakeFd = 12345;
  LocalHandle handle{kFakeFd};
  EXPECT_CALL(*endpoint(), MessageReplyFd(message_.get(), kFakeFd))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(handle));
  handle.Release();  // Make sure we do not close the fake file descriptor.
}

TEST_F(ServiceMessageTest, ReplyLocalFileHandleError) {
  LocalHandle handle{-EINVAL};
  EXPECT_CALL(*endpoint(), MessageReply(message_.get(), -EINVAL))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(handle));
}

TEST_F(ServiceMessageTest, ReplyBorrowedFileHandle) {
  const int kFakeFd = 12345;
  BorrowedHandle handle{kFakeFd};
  EXPECT_CALL(*endpoint(), MessageReplyFd(message_.get(), kFakeFd))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(handle));
}

TEST_F(ServiceMessageTest, ReplyBorrowedFileHandleError) {
  BorrowedHandle handle{-EACCES};
  EXPECT_CALL(*endpoint(), MessageReply(message_.get(), -EACCES))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(handle));
}

TEST_F(ServiceMessageTest, ReplyRemoteFileHandle) {
  RemoteHandle handle{123};
  EXPECT_CALL(*endpoint(), MessageReply(message_.get(), handle.Get()))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(handle));
}

TEST_F(ServiceMessageTest, ReplyRemoteFileHandleError) {
  RemoteHandle handle{-EIO};
  EXPECT_CALL(*endpoint(), MessageReply(message_.get(), -EIO))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(handle));
}

TEST_F(ServiceMessageTest, ReplyLocalChannelHandle) {
  LocalChannelHandle handle{nullptr, 12345};
  EXPECT_CALL(*endpoint(), MessageReplyChannelHandle(
                               message_.get(), A<const LocalChannelHandle&>()))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(handle));
}

TEST_F(ServiceMessageTest, ReplyBorrowedChannelHandle) {
  BorrowedChannelHandle handle{12345};
  EXPECT_CALL(*endpoint(),
              MessageReplyChannelHandle(message_.get(),
                                        A<const BorrowedChannelHandle&>()))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(handle));
}

TEST_F(ServiceMessageTest, ReplyRemoteChannelHandle) {
  RemoteChannelHandle handle{12345};
  EXPECT_CALL(*endpoint(), MessageReplyChannelHandle(
                               message_.get(), A<const RemoteChannelHandle&>()))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(handle));
}

TEST_F(ServiceMessageTest, ReplyStatusInt) {
  Status<int> status{123};
  EXPECT_CALL(*endpoint(), MessageReply(message_.get(), status.get()))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(status));
}

TEST_F(ServiceMessageTest, ReplyStatusError) {
  Status<int> status{ErrorStatus{EIO}};
  EXPECT_CALL(*endpoint(), MessageReply(message_.get(), -status.error()))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->Reply(status));
}

TEST_F(ServiceMessageTest, Read) {
  ExpectDefaultHandleMessage();
  void* const kDataBuffer = IntToPtr(12345);
  const size_t kDataSize = 100;
  EXPECT_CALL(
      *endpoint(),
      ReadMessageData(message_.get(), IoVecMatcher(kDataBuffer, kDataSize), 1))
      .WillOnce(Return(50))
      .WillOnce(Return(ErrorStatus{EACCES}));
  EXPECT_EQ(50u, message_->Read(kDataBuffer, kDataSize).get());
  EXPECT_EQ(EACCES, message_->Read(kDataBuffer, kDataSize).error());
}

TEST_F(ServiceMessageTest, ReadVector) {
  ExpectDefaultHandleMessage();
  char buffer1[10];
  char buffer2[20];
  iovec vec[] = {{buffer1, sizeof(buffer1)}, {buffer2, sizeof(buffer2)}};
  EXPECT_CALL(*endpoint(),
              ReadMessageData(
                  message_.get(),
                  IoVecMatcher(IoVecArray{std::begin(vec), std::end(vec)}), 2))
      .WillOnce(Return(30))
      .WillOnce(Return(15))
      .WillOnce(Return(ErrorStatus{EBADF}));
  EXPECT_EQ(30u, message_->ReadVector(vec, 2).get());
  EXPECT_EQ(15u, message_->ReadVector(vec).get());
  EXPECT_EQ(EBADF, message_->ReadVector(vec).error());
}

TEST_F(ServiceMessageTest, Write) {
  ExpectDefaultHandleMessage();
  void* const kDataBuffer = IntToPtr(12345);
  const size_t kDataSize = 100;
  EXPECT_CALL(
      *endpoint(),
      WriteMessageData(message_.get(), IoVecMatcher(kDataBuffer, kDataSize), 1))
      .WillOnce(Return(50))
      .WillOnce(Return(ErrorStatus{EBADMSG}));
  EXPECT_EQ(50u, message_->Write(kDataBuffer, kDataSize).get());
  EXPECT_EQ(EBADMSG, message_->Write(kDataBuffer, kDataSize).error());
}

TEST_F(ServiceMessageTest, WriteVector) {
  ExpectDefaultHandleMessage();
  char buffer1[10];
  char buffer2[20];
  iovec vec[] = {{buffer1, sizeof(buffer1)}, {buffer2, sizeof(buffer2)}};
  EXPECT_CALL(*endpoint(),
              WriteMessageData(
                  message_.get(),
                  IoVecMatcher(IoVecArray{std::begin(vec), std::end(vec)}), 2))
      .WillOnce(Return(30))
      .WillOnce(Return(15))
      .WillOnce(Return(ErrorStatus{EIO}));
  EXPECT_EQ(30u, message_->WriteVector(vec, 2).get());
  EXPECT_EQ(15u, message_->WriteVector(vec).get());
  EXPECT_EQ(EIO, message_->WriteVector(vec, 2).error());
}

TEST_F(ServiceMessageTest, PushLocalFileHandle) {
  ExpectDefaultHandleMessage();
  const int kFakeFd = 12345;
  LocalHandle handle{kFakeFd};
  EXPECT_CALL(*endpoint(),
              PushFileHandle(message_.get(), Matcher<const LocalHandle&>(
                                                 FileHandleMatcher(kFakeFd))))
      .WillOnce(Return(12))
      .WillOnce(Return(ErrorStatus{EIO}));
  EXPECT_EQ(12, message_->PushFileHandle(handle).get());
  EXPECT_EQ(EIO, message_->PushFileHandle(handle).error());
  handle.Release();  // Make sure we do not close the fake file descriptor.
}

TEST_F(ServiceMessageTest, PushBorrowedFileHandle) {
  ExpectDefaultHandleMessage();
  const int kFakeFd = 12345;
  BorrowedHandle handle{kFakeFd};
  EXPECT_CALL(*endpoint(),
              PushFileHandle(message_.get(), Matcher<const BorrowedHandle&>(
                                                 FileHandleMatcher(kFakeFd))))
      .WillOnce(Return(13))
      .WillOnce(Return(ErrorStatus{EACCES}));
  EXPECT_EQ(13, message_->PushFileHandle(handle).get());
  EXPECT_EQ(EACCES, message_->PushFileHandle(handle).error());
}

TEST_F(ServiceMessageTest, PushRemoteFileHandle) {
  ExpectDefaultHandleMessage();
  const int kFakeFd = 12345;
  RemoteHandle handle{kFakeFd};
  EXPECT_CALL(*endpoint(),
              PushFileHandle(message_.get(), Matcher<const RemoteHandle&>(
                                                 FileHandleMatcher(kFakeFd))))
      .WillOnce(Return(kFakeFd))
      .WillOnce(Return(ErrorStatus{EIO}));
  EXPECT_EQ(kFakeFd, message_->PushFileHandle(handle).get());
  EXPECT_EQ(EIO, message_->PushFileHandle(handle).error());
}

TEST_F(ServiceMessageTest, PushLocalChannelHandle) {
  ExpectDefaultHandleMessage();
  int32_t kValue = 12345;
  LocalChannelHandle handle{nullptr, kValue};
  EXPECT_CALL(*endpoint(), PushChannelHandle(message_.get(),
                                             Matcher<const LocalChannelHandle&>(
                                                 ChannelHandleMatcher(kValue))))
      .WillOnce(Return(7))
      .WillOnce(Return(ErrorStatus{EIO}));
  EXPECT_EQ(7, message_->PushChannelHandle(handle).get());
  EXPECT_EQ(EIO, message_->PushChannelHandle(handle).error());
}

TEST_F(ServiceMessageTest, PushBorrowedChannelHandle) {
  ExpectDefaultHandleMessage();
  int32_t kValue = 12345;
  BorrowedChannelHandle handle{kValue};
  EXPECT_CALL(
      *endpoint(),
      PushChannelHandle(message_.get(), Matcher<const BorrowedChannelHandle&>(
                                            ChannelHandleMatcher(kValue))))
      .WillOnce(Return(8))
      .WillOnce(Return(ErrorStatus{EIO}));
  EXPECT_EQ(8, message_->PushChannelHandle(handle).get());
  EXPECT_EQ(EIO, message_->PushChannelHandle(handle).error());
}

TEST_F(ServiceMessageTest, PushRemoteChannelHandle) {
  ExpectDefaultHandleMessage();
  int32_t kValue = 12345;
  RemoteChannelHandle handle{kValue};
  EXPECT_CALL(
      *endpoint(),
      PushChannelHandle(message_.get(), Matcher<const RemoteChannelHandle&>(
                                            ChannelHandleMatcher(kValue))))
      .WillOnce(Return(kValue))
      .WillOnce(Return(ErrorStatus{EIO}));
  EXPECT_EQ(kValue, message_->PushChannelHandle(handle).get());
  EXPECT_EQ(EIO, message_->PushChannelHandle(handle).error());
}

TEST_F(ServiceMessageTest, GetFileHandle) {
  ExpectDefaultHandleMessage();
  auto make_file_handle = [](FileReference ref) { return LocalHandle{ref}; };
  EXPECT_CALL(*endpoint(), GetFileHandle(message_.get(), _))
      .WillOnce(WithArg<1>(Invoke(make_file_handle)));
  LocalHandle handle;
  FileReference kRef = 12345;
  EXPECT_TRUE(message_->GetFileHandle(kRef, &handle));
  EXPECT_EQ(kRef, handle.Get());
  handle.Release();  // Make sure we do not close the fake file descriptor.
}

TEST_F(ServiceMessageTest, GetFileHandleInvalid) {
  ExpectDefaultHandleMessage();
  LocalHandle handle;
  FileReference kRef = -12;
  EXPECT_TRUE(message_->GetFileHandle(kRef, &handle));
  EXPECT_EQ(kRef, handle.Get());
}

TEST_F(ServiceMessageTest, GetFileHandleError) {
  ExpectDefaultHandleMessage();
  EXPECT_CALL(*endpoint(), GetFileHandle(message_.get(), _))
      .WillOnce(WithoutArgs(Invoke([] { return LocalHandle{-EIO}; })));
  LocalHandle handle;
  FileReference kRef = 12345;
  EXPECT_FALSE(message_->GetFileHandle(kRef, &handle));
  EXPECT_EQ(-EIO, handle.Get());
}

TEST_F(ServiceMessageTest, GetChannelHandle) {
  ExpectDefaultHandleMessage();
  auto make_channel_handle = [](ChannelReference ref) {
    return LocalChannelHandle{nullptr, ref};
  };
  EXPECT_CALL(*endpoint(), GetChannelHandle(message_.get(), _))
      .WillOnce(WithArg<1>(Invoke(make_channel_handle)));
  LocalChannelHandle handle;
  ChannelReference kRef = 12345;
  EXPECT_TRUE(message_->GetChannelHandle(kRef, &handle));
  EXPECT_EQ(kRef, handle.value());
}

TEST_F(ServiceMessageTest, GetChannelHandleInvalid) {
  ExpectDefaultHandleMessage();
  LocalChannelHandle handle;
  ChannelReference kRef = -12;
  EXPECT_TRUE(message_->GetChannelHandle(kRef, &handle));
  EXPECT_EQ(-12, handle.value());
}

TEST_F(ServiceMessageTest, GetChannelHandleError) {
  ExpectDefaultHandleMessage();
  EXPECT_CALL(*endpoint(), GetChannelHandle(message_.get(), _))
      .WillOnce(WithoutArgs(Invoke([] {
        return LocalChannelHandle{nullptr, -EIO};
      })));
  LocalChannelHandle handle;
  ChannelReference kRef = 12345;
  EXPECT_FALSE(message_->GetChannelHandle(kRef, &handle));
  EXPECT_EQ(-EIO, handle.value());
}

TEST_F(ServiceMessageTest, ModifyChannelEvents) {
  ExpectDefaultHandleMessage();
  int kClearMask = 1;
  int kSetMask = 2;
  EXPECT_CALL(*endpoint(), ModifyChannelEvents(kTestCid, kClearMask, kSetMask))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(message_->ModifyChannelEvents(kClearMask, kSetMask));
}

TEST_F(ServiceMessageTest, PushChannelSameService) {
  ExpectDefaultHandleMessage();
  int kFlags = 123;
  int32_t kValue = 12;
  EXPECT_CALL(*endpoint(), PushChannel(message_.get(), kFlags, nullptr, _))
      .WillOnce(DoAll(SetArgPointee<3>(kTestCid),
                      Return(ByMove(RemoteChannelHandle{kValue}))));
  int channel_id = -1;
  auto status = message_->PushChannel(kFlags, nullptr, &channel_id);
  ASSERT_TRUE(status);
  EXPECT_EQ(kValue, status.get().value());
  EXPECT_EQ(kTestCid, channel_id);
}

TEST_F(ServiceMessageTest, PushChannelFailure) {
  ExpectDefaultHandleMessage();
  int kFlags = 123;
  EXPECT_CALL(*endpoint(), PushChannel(message_.get(), kFlags, nullptr, _))
      .WillOnce(Return(ByMove(ErrorStatus{EIO})));
  int channel_id = -1;
  auto status = message_->PushChannel(kFlags, nullptr, &channel_id);
  ASSERT_FALSE(status);
  EXPECT_EQ(EIO, status.error());
}

TEST_F(ServiceMessageTest, PushChannelDifferentService) {
  ExpectDefaultHandleMessage();
  auto endpoint2 = std::make_unique<testing::StrictMock<MockEndpoint>>();
  EXPECT_CALL(*endpoint2, SetService(_))
      .Times(2)
      .WillRepeatedly(Return(Status<void>{}));
  auto service2 =
      std::make_shared<MockService>("MockSvc2", std::move(endpoint2));

  int kFlags = 123;
  int32_t kValue = 12;
  EXPECT_CALL(*static_cast<MockEndpoint*>(service2->endpoint()),
              PushChannel(message_.get(), kFlags, nullptr, _))
      .WillOnce(DoAll(SetArgPointee<3>(kTestCid),
                      Return(ByMove(RemoteChannelHandle{kValue}))));
  int channel_id = -1;
  auto status =
      message_->PushChannel(service2.get(), kFlags, nullptr, &channel_id);
  ASSERT_TRUE(status);
  EXPECT_EQ(kValue, status.get().value());
  EXPECT_EQ(kTestCid, channel_id);
}

TEST_F(ServiceMessageTest, CheckChannelSameService) {
  ExpectDefaultHandleMessage();

  auto test_channel = std::make_shared<Channel>();
  ChannelReference kRef = 123;
  EXPECT_CALL(*endpoint(), CheckChannel(message_.get(), kRef, _))
      .WillOnce(DoAll(SetArgPointee<2>(test_channel.get()), Return(kTestCid)));
  std::shared_ptr<Channel> channel;
  auto status = message_->CheckChannel(kRef, &channel);
  ASSERT_TRUE(status);
  EXPECT_EQ(kTestCid, status.get());
  EXPECT_EQ(test_channel, channel);
}

TEST_F(ServiceMessageTest, CheckChannelFailure) {
  ExpectDefaultHandleMessage();
  ChannelReference kRef = 123;
  EXPECT_CALL(*endpoint(), CheckChannel(message_.get(), kRef, _))
      .WillOnce(Return(ByMove(ErrorStatus{EOPNOTSUPP})));
  std::shared_ptr<Channel> channel;
  auto status = message_->CheckChannel(kRef, &channel);
  ASSERT_FALSE(status);
  EXPECT_EQ(EOPNOTSUPP, status.error());
}

TEST_F(ServiceMessageTest, CheckChannelDifferentService) {
  ExpectDefaultHandleMessage();
  auto endpoint2 = std::make_unique<testing::StrictMock<MockEndpoint>>();
  EXPECT_CALL(*endpoint2, SetService(_))
      .Times(2)
      .WillRepeatedly(Return(Status<void>{}));
  auto service2 =
      std::make_shared<MockService>("MockSvc2", std::move(endpoint2));

  auto test_channel = std::make_shared<Channel>();
  ChannelReference kRef = 123;
  EXPECT_CALL(*static_cast<MockEndpoint*>(service2->endpoint()),
              CheckChannel(message_.get(), kRef, _))
      .WillOnce(DoAll(SetArgPointee<2>(test_channel.get()), Return(kTestCid)));
  std::shared_ptr<Channel> channel;
  auto status = message_->CheckChannel(service2.get(), kRef, &channel);
  ASSERT_TRUE(status);
  EXPECT_EQ(kTestCid, status.get());
  EXPECT_EQ(test_channel, channel);
}
