#include <pdx/client.h>

#include <gmock/gmock.h>
#include <sys/eventfd.h>

#include <pdx/mock_client_channel.h>
#include <pdx/mock_client_channel_factory.h>
#include <pdx/rpc/remote_method.h>

using android::pdx::BorrowedChannelHandle;
using android::pdx::BorrowedHandle;
using android::pdx::ClientBase;
using android::pdx::ClientChannel;
using android::pdx::ClientChannelFactory;
using android::pdx::ErrorStatus;
using android::pdx::LocalChannelHandle;
using android::pdx::LocalHandle;
using android::pdx::MockClientChannel;
using android::pdx::MockClientChannelFactory;
using android::pdx::RemoteChannelHandle;
using android::pdx::RemoteHandle;
using android::pdx::Status;
using android::pdx::Transaction;
using android::pdx::rpc::Void;

using testing::A;
using testing::AnyNumber;
using testing::ByMove;
using testing::Invoke;
using testing::Ne;
using testing::Return;
using testing::_;

namespace {

inline void* IntToPtr(intptr_t addr) { return reinterpret_cast<void*>(addr); }
inline const void* IntToConstPtr(intptr_t addr) {
  return reinterpret_cast<const void*>(addr);
}

struct TestInterface final {
  // Op codes.
  enum {
    kOpAdd = 0,
    kOpSendFile,
    kOpGetFile,
    kOpPushChannel,
  };

  // Methods.
  PDX_REMOTE_METHOD(Add, kOpAdd, int(int, int));
  PDX_REMOTE_METHOD(SendFile, kOpSendFile, void(const LocalHandle& fd));
  PDX_REMOTE_METHOD(GetFile, kOpGetFile, LocalHandle(const std::string&, int));
  PDX_REMOTE_METHOD(PushChannel, kOpPushChannel, LocalChannelHandle(Void));

  PDX_REMOTE_API(API, Add, SendFile, GetFile, PushChannel);
};

class SimpleClient : public ClientBase<SimpleClient> {
 public:
  explicit SimpleClient(std::unique_ptr<ClientChannel> channel)
      : BASE{std::move(channel)} {}
  SimpleClient(std::unique_ptr<ClientChannelFactory> channel_factory,
               int64_t timeout_ms)
      : BASE{std::move(channel_factory), timeout_ms} {
    EnableAutoReconnect(timeout_ms);
  }

  using BASE::SendImpulse;
  using BASE::InvokeRemoteMethod;
  using BASE::InvokeRemoteMethodInPlace;
  using BASE::Close;
  using BASE::IsConnected;
  using BASE::EnableAutoReconnect;
  using BASE::DisableAutoReconnect;
  using BASE::event_fd;
  using BASE::GetChannel;

  MOCK_METHOD0(OnConnect, void());
};

class FailingClient : public ClientBase<FailingClient> {
 public:
  explicit FailingClient(std::unique_ptr<ClientChannel> channel, int error_code)
      : BASE{std::move(channel)} {
    Close(error_code);
  }
};

class ClientChannelTest : public testing::Test {
 public:
  ClientChannelTest()
      : client_{SimpleClient::Create(
            std::make_unique<testing::StrictMock<MockClientChannel>>())} {}

  MockClientChannel* mock_channel() {
    return static_cast<MockClientChannel*>(client_->GetChannel());
  }

  std::unique_ptr<SimpleClient> client_;
};

class ClientChannelFactoryTest : public testing::Test {
 public:
  ClientChannelFactoryTest() {
    auto factory =
        std::make_unique<testing::NiceMock<MockClientChannelFactory>>();
    ON_CALL(*factory, Connect(kTimeout))
        .WillByDefault(Invoke(this, &ClientChannelFactoryTest::OnConnect));
    client_ = SimpleClient::Create(std::move(factory), kTimeout);
  }

  MockClientChannel* mock_channel() {
    return static_cast<MockClientChannel*>(client_->GetChannel());
  }

  Status<std::unique_ptr<ClientChannel>> OnConnect(int64_t /*timeout_ms*/) {
    if (on_connect_error_)
      return ErrorStatus(on_connect_error_);
    std::unique_ptr<MockClientChannel> channel =
        std::make_unique<testing::StrictMock<MockClientChannel>>();
    if (on_connect_callback_)
      on_connect_callback_(channel.get());
    return Status<std::unique_ptr<ClientChannel>>{std::move(channel)};
  }

  void OnConnectCallback(std::function<void(MockClientChannel*)> callback) {
    on_connect_callback_ = callback;
  }
  void SetOnConnectError(int error) { on_connect_error_ = error; }
  void ResetOnConnectError() { on_connect_error_ = 0; }

  constexpr static int64_t kTimeout = 123;
  std::unique_ptr<SimpleClient> client_;
  std::function<void(MockClientChannel*)> on_connect_callback_;
  int on_connect_error_{0};
};

constexpr int64_t ClientChannelFactoryTest::kTimeout;

class ClientTransactionTest : public ClientChannelTest {
 public:
  ClientTransactionTest() : transaction_{*client_} {}

  Transaction transaction_;
};

}  // anonymous namespace

TEST_F(ClientChannelTest, IsInitialized) {
  ASSERT_NE(client_.get(), nullptr);
  EXPECT_TRUE(client_->IsInitialized());
  EXPECT_TRUE(client_->IsConnected());
}

TEST_F(ClientChannelTest, CloseOnConstruction) {
  FailingClient failed_client1{std::make_unique<MockClientChannel>(), EACCES};
  ASSERT_FALSE(failed_client1.IsInitialized());
  EXPECT_EQ(-EACCES, failed_client1.error());

  FailingClient failed_client2{std::make_unique<MockClientChannel>(), -EACCES};
  ASSERT_FALSE(failed_client2.IsInitialized());
  EXPECT_EQ(-EACCES, failed_client2.error());

  auto failed_client3 =
      FailingClient::Create(std::make_unique<MockClientChannel>(), EIO);
  ASSERT_EQ(failed_client3.get(), nullptr);
}

TEST_F(ClientChannelTest, IsConnected) {
  EXPECT_TRUE(client_->IsConnected());
  EXPECT_EQ(0, client_->error());
  client_->Close(-EINVAL);
  EXPECT_FALSE(client_->IsConnected());
  EXPECT_EQ(-EINVAL, client_->error());
}

TEST_F(ClientChannelTest, event_fd) {
  EXPECT_CALL(*mock_channel(), event_fd()).WillOnce(Return(12));
  EXPECT_EQ(12, client_->event_fd());
}

TEST_F(ClientChannelTest, SendImpulse) {
  EXPECT_CALL(*mock_channel(), SendImpulse(123, nullptr, 0))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(client_->SendImpulse(123));

  EXPECT_CALL(*mock_channel(), SendImpulse(17, nullptr, 0))
      .WillOnce(Return(ErrorStatus{EIO}));
  auto status = client_->SendImpulse(17);
  ASSERT_FALSE(status);
  EXPECT_EQ(EIO, status.error());

  const void* const kTestPtr = IntToConstPtr(1234);
  EXPECT_CALL(*mock_channel(), SendImpulse(1, kTestPtr, 17))
      .WillOnce(Return(Status<void>{}));
  EXPECT_TRUE(client_->SendImpulse(1, kTestPtr, 17));
}

TEST_F(ClientChannelTest, InvokeRemoteMethodNullTransactionState) {
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(nullptr));
  EXPECT_CALL(*mock_channel(),
              SendWithInt(nullptr, TestInterface::kOpAdd, _, _, nullptr, 0))
      .WillOnce(Return(9));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(nullptr));
  EXPECT_TRUE(client_->InvokeRemoteMethod<TestInterface::Add>(4, 5));
}

TEST_F(ClientChannelTest, InvokeRemoteMethodAddSuccess) {
  void* const kTransactionState = IntToPtr(123);
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(
      *mock_channel(),
      SendWithInt(kTransactionState, TestInterface::kOpAdd, _, _, nullptr, 0))
      .WillOnce(Return(3));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));
  Status<int> status = client_->InvokeRemoteMethod<TestInterface::Add>(1, 2);
  ASSERT_TRUE(status);
  EXPECT_EQ(3, status.get());
}

TEST_F(ClientChannelTest, InvokeRemoteMethodAddFailure) {
  void* const kTransactionState = IntToPtr(123);
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(
      *mock_channel(),
      SendWithInt(kTransactionState, TestInterface::kOpAdd, _, _, nullptr, 0))
      .WillOnce(Return(ErrorStatus{EIO}));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));
  Status<int> status = client_->InvokeRemoteMethod<TestInterface::Add>(1, 2);
  ASSERT_FALSE(status);
  EXPECT_EQ(EIO, status.error());
}

TEST_F(ClientChannelTest, InvokeRemoteMethodGetFileSuccess) {
  void* const kTransactionState = IntToPtr(123);
  int fd = eventfd(0, 0);
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(*mock_channel(),
              SendWithFileHandle(kTransactionState, TestInterface::kOpGetFile,
                                 _, _, nullptr, 0))
      .WillOnce(Return(ByMove(LocalHandle{fd})));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));
  Status<LocalHandle> status =
      client_->InvokeRemoteMethod<TestInterface::GetFile>();
  ASSERT_TRUE(status);
  EXPECT_EQ(fd, status.get().Get());
}

TEST_F(ClientChannelTest, InvokeRemoteMethodGetFileFailure) {
  void* const kTransactionState = IntToPtr(123);
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(*mock_channel(),
              SendWithFileHandle(kTransactionState, TestInterface::kOpGetFile,
                                 _, _, nullptr, 0))
      .WillOnce(Return(ByMove(ErrorStatus{EACCES})));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));
  Status<LocalHandle> status =
      client_->InvokeRemoteMethod<TestInterface::GetFile>("file", 0);
  ASSERT_FALSE(status);
  EXPECT_EQ(EACCES, status.error());
}

TEST_F(ClientChannelTest, InvokeRemoteMethodPushChannelSuccess) {
  void* const kTransactionState = IntToPtr(123);
  const int32_t kHandleValue = 17;
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(
      *mock_channel(),
      SendWithChannelHandle(kTransactionState, TestInterface::kOpPushChannel, _,
                            _, nullptr, 0))
      .WillOnce(Return(ByMove(LocalChannelHandle{nullptr, kHandleValue})));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));
  Status<LocalChannelHandle> status =
      client_->InvokeRemoteMethod<TestInterface::PushChannel>();
  ASSERT_TRUE(status);
  EXPECT_EQ(kHandleValue, status.get().value());
}

TEST_F(ClientChannelTest, InvokeRemoteMethodPushChannelFailure) {
  void* const kTransactionState = IntToPtr(123);
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(
      *mock_channel(),
      SendWithChannelHandle(kTransactionState, TestInterface::kOpPushChannel, _,
                            _, nullptr, 0))
      .WillOnce(Return(ByMove(ErrorStatus{EACCES})));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));
  Status<LocalChannelHandle> status =
      client_->InvokeRemoteMethod<TestInterface::PushChannel>();
  ASSERT_FALSE(status);
  EXPECT_EQ(EACCES, status.error());
}

TEST_F(ClientChannelTest, InvokeRemoteMethodSendFileSuccess) {
  void* const kTransactionState = IntToPtr(123);
  LocalHandle fd{eventfd(0, 0)};
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(*mock_channel(),
              PushFileHandle(kTransactionState, A<const LocalHandle&>()))
      .WillOnce(Return(1));
  EXPECT_CALL(*mock_channel(),
              SendWithInt(kTransactionState, TestInterface::kOpSendFile, _, _,
                          nullptr, 0))
      .WillOnce(Return(0));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));
  EXPECT_TRUE(client_->InvokeRemoteMethod<TestInterface::SendFile>(fd));
}

TEST_F(ClientChannelTest, InvokeRemoteMethodSendFileFailure) {
  void* const kTransactionState = IntToPtr(123);
  LocalHandle fd{eventfd(0, 0)};
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(*mock_channel(),
              PushFileHandle(kTransactionState, A<const LocalHandle&>()))
      .WillOnce(Return(1));
  EXPECT_CALL(*mock_channel(),
              SendWithInt(kTransactionState, TestInterface::kOpSendFile, _, _,
                          nullptr, 0))
      .WillOnce(Return(ErrorStatus{EACCES}));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));
  EXPECT_FALSE(client_->InvokeRemoteMethod<TestInterface::SendFile>(fd));
}

TEST_F(ClientChannelFactoryTest, IsInitialized) {
  ASSERT_NE(client_.get(), nullptr);
  EXPECT_TRUE(client_->IsInitialized());
  EXPECT_TRUE(client_->IsConnected());
}

TEST_F(ClientChannelFactoryTest, NotConnectedButInitialized) {
  auto factory =
      std::make_unique<testing::NiceMock<MockClientChannelFactory>>();
  EXPECT_CALL(*factory, Connect(kTimeout))
      .WillOnce(Return(ByMove(ErrorStatus(ESHUTDOWN))))
      .WillOnce(Invoke(this, &ClientChannelFactoryTest::OnConnect));
  client_ = SimpleClient::Create(std::move(factory), kTimeout);
  ASSERT_NE(client_.get(), nullptr);
  EXPECT_TRUE(client_->IsInitialized());
  EXPECT_FALSE(client_->IsConnected());
  client_->DisableAutoReconnect();
  ASSERT_FALSE(client_->SendImpulse(17));
  EXPECT_FALSE(client_->IsConnected());
  client_->EnableAutoReconnect(kTimeout);
  EXPECT_CALL(*client_, OnConnect());
  OnConnectCallback([](auto* mock) {
    EXPECT_CALL(*mock, SendImpulse(17, nullptr, 0))
        .WillOnce(Return(Status<void>{}));
  });
  ASSERT_TRUE(client_->SendImpulse(17));
  EXPECT_TRUE(client_->IsConnected());
}

TEST_F(ClientChannelFactoryTest, CheckDisconnect) {
  EXPECT_CALL(*mock_channel(), SendImpulse(17, nullptr, 0))
      .WillOnce(Return(ErrorStatus{ESHUTDOWN}));
  ASSERT_FALSE(client_->SendImpulse(17));
  EXPECT_FALSE(client_->IsConnected());
  EXPECT_EQ(-ESHUTDOWN, client_->error());
}

TEST_F(ClientChannelFactoryTest, CheckReconnect) {
  client_->Close(ESHUTDOWN);
  ASSERT_FALSE(client_->IsConnected());

  EXPECT_CALL(*client_, OnConnect());
  OnConnectCallback([](auto* mock) {
    EXPECT_CALL(*mock, SendImpulse(17, nullptr, 0))
        .WillOnce(Return(Status<void>{}));
  });
  ASSERT_TRUE(client_->SendImpulse(17));
  EXPECT_TRUE(client_->IsConnected());
}

TEST_F(ClientChannelFactoryTest, CloseOnConnect) {
  client_->Close(ESHUTDOWN);

  EXPECT_CALL(*client_, OnConnect()).WillOnce(Invoke([this] {
    client_->Close(EIO);
  }));
  auto status = client_->SendImpulse(17);
  ASSERT_FALSE(status);
  EXPECT_EQ(EIO, status.error());
  EXPECT_FALSE(client_->IsConnected());
  EXPECT_EQ(-EIO, client_->error());
}

TEST_F(ClientChannelFactoryTest, DisableAutoReconnect) {
  client_->Close(EIO);
  ASSERT_FALSE(client_->IsConnected());
  client_->DisableAutoReconnect();
  auto status = client_->SendImpulse(17);
  ASSERT_FALSE(status);
  EXPECT_EQ(ESHUTDOWN, status.error());
  EXPECT_FALSE(client_->IsConnected());
  client_->EnableAutoReconnect(kTimeout);
  EXPECT_CALL(*client_, OnConnect());
  OnConnectCallback([](auto* mock) {
    EXPECT_CALL(*mock, SendImpulse(17, nullptr, 0))
        .WillOnce(Return(Status<void>{}));
  });
  ASSERT_TRUE(client_->SendImpulse(17));
  EXPECT_TRUE(client_->IsConnected());
}

TEST_F(ClientTransactionTest, SendNoData) {
  void* const kTransactionState = IntToPtr(123);
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));
  EXPECT_CALL(*mock_channel(),
              SendWithInt(kTransactionState, 1, nullptr, 0, nullptr, 0))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.Send<void>(1));
  EXPECT_CALL(*mock_channel(),
              SendWithFileHandle(kTransactionState, 2, nullptr, 0, nullptr, 0))
      .WillOnce(Return(ByMove(LocalHandle{-1})));
  EXPECT_TRUE(transaction_.Send<LocalHandle>(2));
  EXPECT_CALL(*mock_channel(), SendWithChannelHandle(kTransactionState, 3,
                                                     nullptr, 0, nullptr, 0))
      .WillOnce(Return(ByMove(LocalChannelHandle{nullptr, 1})));
  EXPECT_TRUE(transaction_.Send<LocalChannelHandle>(3));
}

TEST_F(ClientTransactionTest, SendNoState) {
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(nullptr));
  EXPECT_CALL(*mock_channel(), SendWithInt(nullptr, 1, nullptr, 0, nullptr, 0))
      .WillOnce(Return(0));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(nullptr));
  EXPECT_TRUE(transaction_.Send<void>(1));
}

TEST_F(ClientTransactionTest, SendBuffers) {
  const void* const kSendBuffer = IntToConstPtr(123);
  const size_t kSendSize = 12;
  void* const kReceiveBuffer = IntToPtr(456);
  const size_t kReceiveSize = 34;

  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(nullptr));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(nullptr));

  EXPECT_CALL(*mock_channel(), SendWithInt(nullptr, 1, nullptr, 0, nullptr, 0))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.Send<void>(1, nullptr, 0, nullptr, 0));

  EXPECT_CALL(*mock_channel(),
              SendWithInt(nullptr, 2, Ne(nullptr), 1, nullptr, 0))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.Send<void>(2, kSendBuffer, kSendSize, nullptr, 0));

  EXPECT_CALL(*mock_channel(), SendWithInt(nullptr, 3, nullptr, 0, nullptr, 0))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.Send<void>(3, kSendBuffer, 0, nullptr, 0));

  EXPECT_CALL(*mock_channel(),
              SendWithInt(nullptr, 4, nullptr, 0, Ne(nullptr), 1))
      .WillOnce(Return(0));
  EXPECT_TRUE(
      transaction_.Send<void>(4, nullptr, 0, kReceiveBuffer, kReceiveSize));

  EXPECT_CALL(*mock_channel(), SendWithInt(nullptr, 5, nullptr, 0, nullptr, 0))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.Send<void>(5, nullptr, 0, kReceiveBuffer, 0));

  EXPECT_CALL(*mock_channel(),
              SendWithInt(nullptr, 5, Ne(nullptr), 1, Ne(nullptr), 1))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.Send<void>(5, kSendBuffer, kSendSize, kReceiveBuffer,
                                      kReceiveSize));
}

TEST_F(ClientTransactionTest, SendVector) {
  iovec send[3] = {};
  iovec recv[4] = {};

  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(nullptr));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(nullptr));

  EXPECT_CALL(*mock_channel(), SendWithInt(nullptr, 1, nullptr, 0, nullptr, 0))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.SendVector<void>(1, nullptr, 0, nullptr, 0));

  EXPECT_CALL(*mock_channel(), SendWithInt(nullptr, 2, send, 3, recv, 4))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.SendVector<void>(2, send, 3, recv, 4));

  EXPECT_CALL(*mock_channel(), SendWithInt(nullptr, 3, send, 3, nullptr, 0))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.SendVector<void>(3, send, nullptr));

  EXPECT_CALL(*mock_channel(), SendWithInt(nullptr, 4, nullptr, 0, recv, 4))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.SendVector<void>(4, nullptr, recv));

  EXPECT_CALL(*mock_channel(), SendWithInt(nullptr, 5, send, 3, recv, 4))
      .WillOnce(Return(0));
  EXPECT_TRUE(transaction_.SendVector<void>(5, send, recv));
}

TEST_F(ClientTransactionTest, PushHandle) {
  void* const kTransactionState = IntToPtr(123);
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));

  EXPECT_CALL(*mock_channel(),
              PushFileHandle(kTransactionState, A<const LocalHandle&>()))
      .WillOnce(Return(1));
  EXPECT_EQ(1, transaction_.PushFileHandle(LocalHandle{-1}).get());

  EXPECT_CALL(*mock_channel(),
              PushFileHandle(kTransactionState, A<const BorrowedHandle&>()))
      .WillOnce(Return(2));
  EXPECT_EQ(2, transaction_.PushFileHandle(BorrowedHandle{-1}).get());

  EXPECT_EQ(3, transaction_.PushFileHandle(RemoteHandle{3}).get());

  EXPECT_CALL(
      *mock_channel(),
      PushChannelHandle(kTransactionState, A<const LocalChannelHandle&>()))
      .WillOnce(Return(11));
  EXPECT_EQ(
      11, transaction_.PushChannelHandle(LocalChannelHandle{nullptr, 1}).get());

  EXPECT_CALL(
      *mock_channel(),
      PushChannelHandle(kTransactionState, A<const BorrowedChannelHandle&>()))
      .WillOnce(Return(12));
  EXPECT_EQ(12, transaction_.PushChannelHandle(BorrowedChannelHandle{2}).get());

  EXPECT_EQ(13, transaction_.PushChannelHandle(RemoteChannelHandle{13}).get());
}

TEST_F(ClientTransactionTest, GetHandle) {
  void* const kTransactionState = IntToPtr(123);
  EXPECT_CALL(*mock_channel(), AllocateTransactionState())
      .WillOnce(Return(kTransactionState));
  EXPECT_CALL(*mock_channel(), FreeTransactionState(kTransactionState));

  EXPECT_CALL(*mock_channel(), GetFileHandle(kTransactionState, 1, _))
      .WillOnce(Return(false))
      .WillOnce(Return(true));

  LocalHandle file_handle;
  EXPECT_FALSE(transaction_.GetFileHandle(1, &file_handle));
  EXPECT_TRUE(transaction_.GetFileHandle(1, &file_handle));

  EXPECT_CALL(*mock_channel(), GetChannelHandle(kTransactionState, 2, _))
      .WillOnce(Return(false))
      .WillOnce(Return(true));

  LocalChannelHandle channel_handle;
  EXPECT_FALSE(transaction_.GetChannelHandle(2, &channel_handle));
  EXPECT_TRUE(transaction_.GetChannelHandle(2, &channel_handle));
}
