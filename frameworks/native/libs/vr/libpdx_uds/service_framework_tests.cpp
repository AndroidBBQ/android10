#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <array>
#include <atomic>
#include <memory>
#include <numeric>
#include <string>
#include <thread>

#include <gtest/gtest.h>
#include <pdx/channel_handle.h>
#include <pdx/client.h>
#include <pdx/file_handle.h>
#include <pdx/service.h>
#include <pdx/service_dispatcher.h>
#include <private/android_filesystem_config.h>
#include <uds/client_channel.h>
#include <uds/client_channel_factory.h>
#include <uds/service_endpoint.h>

using android::pdx::BorrowedChannelHandle;
using android::pdx::Channel;
using android::pdx::ChannelReference;
using android::pdx::ClientBase;
using android::pdx::ErrorStatus;
using android::pdx::LocalChannelHandle;
using android::pdx::LocalHandle;
using android::pdx::Message;
using android::pdx::MessageInfo;
using android::pdx::RemoteChannelHandle;
using android::pdx::ServiceBase;
using android::pdx::ServiceDispatcher;
using android::pdx::Status;
using android::pdx::Transaction;
using android::pdx::uds::Endpoint;

namespace {

const size_t kLargeDataSize = 100000;

const char kTestServicePath[] = "socket_test";
const char kTestService1[] = "1";
const char kTestService2[] = "2";

enum test_op_codes {
  TEST_OP_GET_SERVICE_ID,
  TEST_OP_SET_TEST_CHANNEL,
  TEST_OP_GET_THIS_CHANNEL_ID,
  TEST_OP_GET_TEST_CHANNEL_ID,
  TEST_OP_CHECK_CHANNEL_ID,
  TEST_OP_CHECK_CHANNEL_OBJECT,
  TEST_OP_CHECK_CHANNEL_FROM_OTHER_SERVICE,
  TEST_OP_GET_NEW_CHANNEL,
  TEST_OP_GET_NEW_CHANNEL_FROM_OTHER_SERVICE,
  TEST_OP_GET_THIS_PROCESS_ID,
  TEST_OP_GET_THIS_THREAD_ID,
  TEST_OP_GET_THIS_EUID,
  TEST_OP_GET_THIS_EGID,
  TEST_OP_IMPULSE,
  TEST_OP_POLLHUP_FROM_SERVICE,
  TEST_OP_POLLIN_FROM_SERVICE,
  TEST_OP_SEND_LARGE_DATA_RETURN_SUM,
};

using ImpulsePayload = std::array<std::uint8_t, sizeof(MessageInfo::impulse)>;

// The test service creates a TestChannel for every client (channel) that
// connects. This represents the service-side context for each client.
class TestChannel : public Channel {
 public:
  explicit TestChannel(int channel_id) : channel_id_(channel_id) {}

  int channel_id() const { return channel_id_; }

 private:
  friend class TestService;

  int channel_id_;

  TestChannel(const TestChannel&) = delete;
  void operator=(const TestChannel&) = delete;
};

// Test service that creates a TestChannel for each channel and responds to test
// messages.
class TestService : public ServiceBase<TestService> {
 public:
  std::shared_ptr<Channel> OnChannelOpen(Message& message) override {
    return std::make_shared<TestChannel>(message.GetChannelId());
  }

  void OnChannelClose(Message& /*message*/,
                      const std::shared_ptr<Channel>& channel) override {
    if (test_channel_ == channel)
      test_channel_ = nullptr;
  }

  void HandleImpulse(Message& message) override {
    switch (message.GetOp()) {
      case TEST_OP_SET_TEST_CHANNEL:
        test_channel_ = message.GetChannel<TestChannel>();
        break;

      case TEST_OP_IMPULSE: {
        impulse_payload_.fill(0);
        std::copy(message.ImpulseBegin(), message.ImpulseEnd(),
                  impulse_payload_.begin());
        break;
      }

      case TEST_OP_POLLHUP_FROM_SERVICE: {
        message.ModifyChannelEvents(0, EPOLLHUP);
        break;
      }
    }
  }

  Status<void> HandleMessage(Message& message) override {
    switch (message.GetOp()) {
      case TEST_OP_GET_SERVICE_ID:
        REPLY_MESSAGE_RETURN(message, service_id_, {});

      // Set the test channel to the TestChannel for the current channel. Other
      // messages can use this to perform tests.
      case TEST_OP_SET_TEST_CHANNEL:
        test_channel_ = message.GetChannel<TestChannel>();
        REPLY_MESSAGE_RETURN(message, 0, {});

      // Return the channel id for the current channel.
      case TEST_OP_GET_THIS_CHANNEL_ID:
        REPLY_MESSAGE_RETURN(message, message.GetChannelId(), {});

      // Return the channel id for the test channel.
      case TEST_OP_GET_TEST_CHANNEL_ID:
        if (test_channel_)
          REPLY_MESSAGE_RETURN(message, test_channel_->channel_id(), {});
        else
          REPLY_ERROR_RETURN(message, ENOENT, {});

      // Test check channel feature.
      case TEST_OP_CHECK_CHANNEL_ID: {
        ChannelReference ref = 0;
        if (!message.ReadAll(&ref, sizeof(ref)))
          REPLY_ERROR_RETURN(message, EIO, {});

        const Status<int> ret = message.CheckChannel<TestChannel>(ref, nullptr);
        REPLY_MESSAGE_RETURN(message, ret, {});
      }

      case TEST_OP_CHECK_CHANNEL_OBJECT: {
        std::shared_ptr<TestChannel> channel;
        ChannelReference ref = 0;
        if (!message.ReadAll(&ref, sizeof(ref)))
          REPLY_ERROR_RETURN(message, EIO, {});

        const Status<int> ret =
            message.CheckChannel<TestChannel>(ref, &channel);
        if (!ret)
          REPLY_MESSAGE_RETURN(message, ret, {});

        if (channel != nullptr)
          REPLY_MESSAGE_RETURN(message, channel->channel_id(), {});
        else
          REPLY_ERROR_RETURN(message, ENODATA, {});
      }

      case TEST_OP_CHECK_CHANNEL_FROM_OTHER_SERVICE: {
        ChannelReference ref = 0;
        if (!message.ReadAll(&ref, sizeof(ref)))
          REPLY_ERROR_RETURN(message, EIO, {});

        const Status<int> ret = message.CheckChannel<TestChannel>(
            other_service_.get(), ref, nullptr);
        REPLY_MESSAGE_RETURN(message, ret, {});
      }

      case TEST_OP_GET_NEW_CHANNEL: {
        auto channel = std::make_shared<TestChannel>(-1);
        Status<RemoteChannelHandle> channel_handle =
            message.PushChannel(0, channel, &channel->channel_id_);
        REPLY_MESSAGE_RETURN(message, channel_handle, {});
      }

      case TEST_OP_GET_NEW_CHANNEL_FROM_OTHER_SERVICE: {
        if (!other_service_)
          REPLY_ERROR_RETURN(message, EINVAL, {});

        auto channel = std::make_shared<TestChannel>(-1);
        Status<RemoteChannelHandle> channel_handle = message.PushChannel(
            other_service_.get(), 0, channel, &channel->channel_id_);
        REPLY_MESSAGE_RETURN(message, channel_handle, {});
      }

      case TEST_OP_GET_THIS_PROCESS_ID:
        REPLY_MESSAGE_RETURN(message, message.GetProcessId(), {});

      case TEST_OP_GET_THIS_THREAD_ID:
        REPLY_MESSAGE_RETURN(message, message.GetThreadId(), {});

      case TEST_OP_GET_THIS_EUID:
        REPLY_MESSAGE_RETURN(message, message.GetEffectiveUserId(), {});

      case TEST_OP_GET_THIS_EGID:
        REPLY_MESSAGE_RETURN(message, message.GetEffectiveGroupId(), {});

      case TEST_OP_POLLIN_FROM_SERVICE:
        REPLY_MESSAGE_RETURN(message, message.ModifyChannelEvents(0, EPOLLIN),
                             {});

      case TEST_OP_SEND_LARGE_DATA_RETURN_SUM: {
        std::array<int, kLargeDataSize> data_array;
        size_t size_to_read = data_array.size() * sizeof(int);
        if (!message.ReadAll(data_array.data(), size_to_read)) {
          REPLY_ERROR_RETURN(message, EIO, {});
        }
        int sum = std::accumulate(data_array.begin(), data_array.end(), 0);
        REPLY_MESSAGE_RETURN(message, sum, {});
      }

      default:
        return Service::DefaultHandleMessage(message);
    }
  }

  const ImpulsePayload& GetImpulsePayload() const { return impulse_payload_; }

 private:
  friend BASE;

  std::shared_ptr<TestChannel> test_channel_;
  std::shared_ptr<TestService> other_service_;
  int service_id_;
  ImpulsePayload impulse_payload_;

  static std::atomic<int> service_counter_;

  TestService(const std::string& name,
              const std::shared_ptr<TestService>& other_service)
      : TestService(name, other_service, false) {}

  TestService(const std::string& name,
              const std::shared_ptr<TestService>& other_service, bool blocking)
      : BASE(std::string("TestService") + name,
             Endpoint::CreateAndBindSocket(kTestServicePath + name, blocking)),
        other_service_(other_service),
        service_id_(service_counter_++) {}

  explicit TestService(const std::string& name) : TestService(name, nullptr) {}

  TestService(const TestService&) = delete;
  void operator=(const TestService&) = delete;
};

std::atomic<int> TestService::service_counter_;

// Test client to send messages to the test service.
class TestClient : public ClientBase<TestClient> {
 public:
  // Requests the service id of the service this channel is connected to.
  int GetServiceId() {
    Transaction trans{*this};
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_GET_SERVICE_ID));
  }

  // Requests the test channel to be set to this client's channel.
  int SetTestChannel() {
    Transaction trans{*this};
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_SET_TEST_CHANNEL));
  }

  // Request the test channel to be set to this client's channel using an async
  // message.
  int SetTestChannelAsync() {
    return ReturnStatusOrError(SendImpulse(TEST_OP_SET_TEST_CHANNEL));
  }

  // Sends a test async message with payload.
  int SendAsync(const void* buffer, size_t length) {
    Transaction trans{*this};
    return ReturnStatusOrError(SendImpulse(TEST_OP_IMPULSE, buffer, length));
  }

  // Requests the channel id for this client.
  int GetThisChannelId() {
    Transaction trans{*this};
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_GET_THIS_CHANNEL_ID));
  }

  // Requests the channel id of the test channel.
  int GetTestChannelId() {
    Transaction trans{*this};
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_GET_TEST_CHANNEL_ID));
  }

  // Checks whether the fd |channel_id| is a channel to the test service.
  // Returns the channel id of the channel.
  int CheckChannelIdArgument(BorrowedChannelHandle channel) {
    Transaction trans{*this};
    ChannelReference ref = trans.PushChannelHandle(channel).get();
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_CHECK_CHANNEL_ID, &ref,
                                               sizeof(ref), nullptr, 0));
  }

  // Checks whether the fd |channel_id| is a channel to the test service.
  // Returns the channel id of the channel exercising the context pointer.
  int CheckChannelObjectArgument(BorrowedChannelHandle channel) {
    Transaction trans{*this};
    ChannelReference ref = trans.PushChannelHandle(channel).get();
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_CHECK_CHANNEL_OBJECT,
                                               &ref, sizeof(ref), nullptr, 0));
  }

  // Checks whether the fd |channel_fd| is a channel to the other test service.
  // Returns 0 on success.
  int CheckChannelFromOtherService(BorrowedChannelHandle channel) {
    Transaction trans{*this};
    ChannelReference ref = trans.PushChannelHandle(channel).get();
    return ReturnStatusOrError(
        trans.Send<int>(TEST_OP_CHECK_CHANNEL_FROM_OTHER_SERVICE, &ref,
                        sizeof(ref), nullptr, 0));
  }

  // Requests a new channel to the service.
  std::unique_ptr<TestClient> GetNewChannel() {
    Transaction trans{*this};
    auto status = trans.Send<LocalChannelHandle>(TEST_OP_GET_NEW_CHANNEL);
    if (status)
      return TestClient::Create(status.take());
    else
      return nullptr;
  }

  // Requests a new channel to the other service.
  std::unique_ptr<TestClient> GetNewChannelFromOtherService() {
    Transaction trans{*this};
    auto status = trans.Send<LocalChannelHandle>(
        TEST_OP_GET_NEW_CHANNEL_FROM_OTHER_SERVICE);
    if (status)
      return TestClient::Create(status.take());
    else
      return nullptr;
  }

  // Requests an id from the message description.
  pid_t GetThisProcessId() {
    Transaction trans{*this};
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_GET_THIS_PROCESS_ID));
  }
  pid_t GetThisThreadId() {
    Transaction trans{*this};
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_GET_THIS_THREAD_ID));
  }
  uid_t GetThisEffectiveUserId() {
    Transaction trans{*this};
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_GET_THIS_EUID));
  }
  gid_t GetThisEffectiveGroupId() {
    Transaction trans{*this};
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_GET_THIS_EGID));
  }

  int SendPollHupEvent() {
    return ReturnStatusOrError(SendImpulse(TEST_OP_POLLHUP_FROM_SERVICE));
  }

  int SendPollInEvent() {
    Transaction trans{*this};
    return ReturnStatusOrError(trans.Send<int>(TEST_OP_POLLIN_FROM_SERVICE));
  }

  int SendLargeDataReturnSum(
      const std::array<int, kLargeDataSize>& data_array) {
    Transaction trans{*this};
    return ReturnStatusOrError(
        trans.Send<int>(TEST_OP_SEND_LARGE_DATA_RETURN_SUM, data_array.data(),
                        data_array.size() * sizeof(int), nullptr, 0));
  }

  Status<int> GetEventMask(int events) {
    if (auto* client_channel = GetChannel()) {
      return client_channel->GetEventMask(events);
    } else {
      return ErrorStatus(EINVAL);
    }
  }

  using ClientBase<TestClient>::event_fd;

  enum : size_t { kMaxPayload = MAX_IMPULSE_LENGTH };

 private:
  friend BASE;

  explicit TestClient(const std::string& name)
      : BASE{android::pdx::uds::ClientChannelFactory::Create(kTestServicePath +
                                                             name)} {}

  explicit TestClient(LocalChannelHandle channel)
      : BASE{android::pdx::uds::ClientChannel::Create(std::move(channel))} {}

  TestClient(const TestClient&) = delete;
  void operator=(const TestClient&) = delete;
};

}  // anonymous namespace

// Use a test fixture to ensure proper order of cleanup between clients,
// services, and the dispatcher. These objects are cleaned up in the same
// thread, order is important; either the service or the client must be
// destroyed before the dispatcher is stopped. The reason for this is that
// clients send blocking "close" messages to their respective services on
// destruction. If this happens after the dispatcher is stopped the client
// destructor will get blocked waiting for a reply that will never come. In
// normal use of the service framework this is never an issue because clients
// and the dispatcher for the same service are never destructed in the same
// thread (they live in different processes).
class ServiceFrameworkTest : public ::testing::Test {
 protected:
  std::unique_ptr<ServiceDispatcher> dispatcher_;
  std::thread dispatch_thread_;

  void SetUp() override {
    // Create a dispatcher to handle messages to services.
    dispatcher_ = android::pdx::ServiceDispatcher::Create();
    ASSERT_NE(nullptr, dispatcher_);

    // Start the message dispatch loop in a separate thread.
    dispatch_thread_ = std::thread(
        std::bind(&ServiceDispatcher::EnterDispatchLoop, dispatcher_.get()));
  }

  void TearDown() override {
    if (dispatcher_) {
      // Cancel the dispatcher and wait for the thread to terminate. Explicitly
      // join the thread so that destruction doesn't deallocate the dispatcher
      // before the thread finishes.
      dispatcher_->SetCanceled(true);
      dispatch_thread_.join();
    }
  }
};

// Test basic operation of TestService/TestClient classes.
TEST_F(ServiceFrameworkTest, BasicClientService) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create(kTestService1);
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create(kTestService1);
  ASSERT_NE(nullptr, client);

  // Get the channel id that will be returned by the next tests.
  const int channel_id = client->GetThisChannelId();
  EXPECT_LE(0, channel_id);

  // Check return value before test channel is set.
  EXPECT_EQ(-ENOENT, client->GetTestChannelId());

  // Set test channel and perform the test again.
  EXPECT_EQ(0, client->SetTestChannel());
  EXPECT_EQ(channel_id, client->GetTestChannelId());
}

// Test impulses.
TEST_F(ServiceFrameworkTest, Impulse) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create(kTestService1);
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  auto client = TestClient::Create(kTestService1);
  ASSERT_NE(nullptr, client);

  // Get the channel id that will be returned by the next tests.
  const int channel_id = client->GetThisChannelId();
  EXPECT_LE(0, channel_id);

  // Check return value before test channel is set.
  EXPECT_EQ(-ENOENT, client->GetTestChannelId());

  // Set test channel with an impulse and perform the test again.
  EXPECT_EQ(0, client->SetTestChannelAsync());
  EXPECT_EQ(channel_id, client->GetTestChannelId());

  ImpulsePayload expected_payload = {{'a', 'b', 'c'}};
  EXPECT_EQ(0, client->SendAsync(expected_payload.data(), 3));
  // Send a synchronous message to make sure the async message is handled before
  // we check the payload.
  client->GetThisChannelId();
  EXPECT_EQ(expected_payload, service->GetImpulsePayload());

  // Impulse payloads are limited to 4 machine words.
  EXPECT_EQ(
      0, client->SendAsync(expected_payload.data(), TestClient::kMaxPayload));
  EXPECT_EQ(-EINVAL, client->SendAsync(expected_payload.data(),
                                       TestClient::kMaxPayload + 1));

  // Test invalid pointer.
  const std::uint8_t* invalid_pointer = nullptr;
  EXPECT_EQ(-EINVAL, client->SendAsync(invalid_pointer, sizeof(int)));
}

// Test impulses.
TEST_F(ServiceFrameworkTest, ImpulseHangup) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create(kTestService1);
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  auto client = TestClient::Create(kTestService1);
  ASSERT_NE(nullptr, client);

  const int kMaxIterations = 1000;
  for (int i = 0; i < kMaxIterations; i++) {
    auto impulse_client = TestClient::Create(kTestService1);
    ASSERT_NE(nullptr, impulse_client);

    const uint8_t a = (i >> 0) & 0xff;
    const uint8_t b = (i >> 8) & 0xff;
    const uint8_t c = (i >> 16) & 0xff;
    const uint8_t d = (i >> 24) & 0xff;
    ImpulsePayload expected_payload = {{a, b, c, d}};
    EXPECT_EQ(0, impulse_client->SendAsync(expected_payload.data(), 4));

    // Hangup the impulse test client, then send a sync message over client to
    // make sure the hangup message is handled before checking the impulse
    // payload.
    impulse_client = nullptr;
    client->GetThisChannelId();
    EXPECT_EQ(expected_payload, service->GetImpulsePayload());
  }
}

// Test Message::PushChannel/Service::PushChannel API.
TEST_F(ServiceFrameworkTest, PushChannel) {
  // Create a test service and add it to the dispatcher.
  auto other_service = TestService::Create(kTestService1);
  ASSERT_NE(nullptr, other_service);
  ASSERT_EQ(0, dispatcher_->AddService(other_service));

  // Create a second test service and add it to the dispatcher.
  auto service = TestService::Create(kTestService2, other_service);
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to the second test service.
  auto client1 = TestClient::Create(kTestService2);
  ASSERT_NE(nullptr, client1);

  // Test the creation of new channels using the push APIs.
  const int channel_id1 = client1->GetThisChannelId();
  EXPECT_LE(0, channel_id1);

  auto client2 = client1->GetNewChannel();
  EXPECT_NE(nullptr, client2);
  EXPECT_NE(client1->event_fd(), client2->event_fd());

  const int channel_id2 = client2->GetThisChannelId();
  EXPECT_LE(0, channel_id2);
  EXPECT_NE(channel_id1, channel_id2);

  auto client3 = client1->GetNewChannelFromOtherService();
  EXPECT_NE(nullptr, client3);
  EXPECT_NE(client1->event_fd(), client3->event_fd());

  const int channel_id3 = client3->GetThisChannelId();
  EXPECT_LE(0, channel_id3);

  // Test which services the channels are connected to.
  const int service_id1 = client1->GetServiceId();
  EXPECT_LE(0, service_id1);

  const int service_id2 = client2->GetServiceId();
  EXPECT_LE(0, service_id2);

  const int service_id3 = client3->GetServiceId();
  EXPECT_LE(0, service_id3);

  EXPECT_EQ(service_id1, service_id2);
  EXPECT_NE(service_id1, service_id3);
}

// Tests process id, thread id, effective user id, and effective group id
// returned in the message description.
TEST_F(ServiceFrameworkTest, Ids) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create(kTestService1);
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create(kTestService1);
  ASSERT_NE(nullptr, client);

  // Pids 0-2 are defined, no user task should have them.

  const pid_t process_id1 = client->GetThisProcessId();
  EXPECT_LT(2, process_id1);

  pid_t process_id2;

  std::thread thread([&]() { process_id2 = client->GetThisProcessId(); });
  thread.join();

  EXPECT_LT(2, process_id2);
  EXPECT_EQ(process_id1, process_id2);

  // This test must run as root for the rest of these tests to work.
  const int euid1 = client->GetThisEffectiveUserId();
  ASSERT_EQ(0, euid1);

  const int egid1 = client->GetThisEffectiveGroupId();
  EXPECT_EQ(0, egid1);

  // Set effective uid/gid to system.
  ASSERT_EQ(0, setegid(AID_SYSTEM));
  ASSERT_EQ(0, seteuid(AID_SYSTEM));

  const int euid2 = client->GetThisEffectiveUserId();
  EXPECT_EQ(AID_SYSTEM, euid2);

  const int egid2 = client->GetThisEffectiveGroupId();
  EXPECT_EQ(AID_SYSTEM, egid2);

  // Set the euid/egid back to root.
  ASSERT_EQ(0, setegid(0));
  ASSERT_EQ(0, seteuid(0));
}

TEST_F(ServiceFrameworkTest, PollIn) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create(kTestService1);
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create(kTestService1);
  ASSERT_NE(nullptr, client);

  pollfd pfd{client->event_fd(), POLLIN, 0};
  int count = poll(&pfd, 1, 0);
  ASSERT_EQ(0, count);

  client->SendPollInEvent();

  count = poll(&pfd, 1, 10000 /*10s*/);
  ASSERT_EQ(1, count);
  ASSERT_TRUE((POLLIN & pfd.revents) != 0);
}

TEST_F(ServiceFrameworkTest, PollHup) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create(kTestService1);
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create(kTestService1);
  ASSERT_NE(nullptr, client);

  pollfd pfd{client->event_fd(), POLLIN, 0};
  int count = poll(&pfd, 1, 0);
  ASSERT_EQ(0, count);

  client->SendPollHupEvent();

  count = poll(&pfd, 1, 10000 /*10s*/);
  ASSERT_EQ(1, count);
  auto event_status = client->GetEventMask(pfd.revents);
  ASSERT_TRUE(event_status.ok());
  ASSERT_TRUE((EPOLLHUP & event_status.get()) != 0);
}

TEST_F(ServiceFrameworkTest, LargeDataSum) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create(kTestService1);
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create(kTestService1);
  ASSERT_NE(nullptr, client);

  std::array<int, kLargeDataSize> data_array;
  std::iota(data_array.begin(), data_array.end(), 0);
  int expected_sum = std::accumulate(data_array.begin(), data_array.end(), 0);
  int sum = client->SendLargeDataReturnSum(data_array);
  ASSERT_EQ(expected_sum, sum);
}

TEST_F(ServiceFrameworkTest, Cancel) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create(kTestService1, nullptr, true);
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create(kTestService1);
  ASSERT_NE(nullptr, client);

  auto previous_time = std::chrono::system_clock::now();
  dispatcher_->ReceiveAndDispatch(100);  // 0.1 seconds should block.
  auto time = std::chrono::system_clock::now();
  ASSERT_LE(100, std::chrono::duration_cast<std::chrono::milliseconds>(
                     time - previous_time)
                     .count());
  service->Cancel();
  // Non-blocking. Return immediately.
  dispatcher_->ReceiveAndDispatch(-1);
  dispatcher_->ReceiveAndDispatch(-1);
}
