#include <uds/client_channel.h>

#include <sys/socket.h>

#include <algorithm>
#include <limits>
#include <random>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <pdx/client.h>
#include <pdx/rpc/remote_method.h>
#include <pdx/service.h>
#include <pdx/service_dispatcher.h>

#include <uds/client_channel_factory.h>
#include <uds/service_endpoint.h>

using testing::Return;
using testing::_;

using android::pdx::ClientBase;
using android::pdx::LocalChannelHandle;
using android::pdx::LocalHandle;
using android::pdx::Message;
using android::pdx::ServiceBase;
using android::pdx::ServiceDispatcher;
using android::pdx::Status;
using android::pdx::rpc::DispatchRemoteMethod;
using android::pdx::uds::ClientChannel;
using android::pdx::uds::ClientChannelFactory;
using android::pdx::uds::Endpoint;

namespace {

struct TestProtocol {
  using DataType = int8_t;
  enum {
    kOpSum = 0,
  };
  PDX_REMOTE_METHOD(Sum, kOpSum, int64_t(const std::vector<DataType>&));
};

class TestService : public ServiceBase<TestService> {
 public:
  explicit TestService(std::unique_ptr<Endpoint> endpoint)
      : ServiceBase{"TestService", std::move(endpoint)} {}

  Status<void> HandleMessage(Message& message) override {
    switch (message.GetOp()) {
      case TestProtocol::kOpSum:
        DispatchRemoteMethod<TestProtocol::Sum>(*this, &TestService::OnSum,
                                                message);
        return {};

      default:
        return Service::HandleMessage(message);
    }
  }

  int64_t OnSum(Message& /*message*/,
                const std::vector<TestProtocol::DataType>& data) {
    return std::accumulate(data.begin(), data.end(), int64_t{0});
  }
};

class TestClient : public ClientBase<TestClient> {
 public:
  using ClientBase::ClientBase;

  int64_t Sum(const std::vector<TestProtocol::DataType>& data) {
    auto status = InvokeRemoteMethod<TestProtocol::Sum>(data);
    return status ? status.get() : -1;
  }
};

class TestServiceRunner {
 public:
  explicit TestServiceRunner(LocalHandle channel_socket) {
    auto endpoint = Endpoint::CreateFromSocketFd(LocalHandle{});
    endpoint->RegisterNewChannelForTests(std::move(channel_socket));
    service_ = TestService::Create(std::move(endpoint));
    dispatcher_ = ServiceDispatcher::Create();
    dispatcher_->AddService(service_);
    dispatch_thread_ = std::thread(
        std::bind(&ServiceDispatcher::EnterDispatchLoop, dispatcher_.get()));
  }

  ~TestServiceRunner() {
    dispatcher_->SetCanceled(true);
    dispatch_thread_.join();
    dispatcher_->RemoveService(service_);
  }

 private:
  std::shared_ptr<TestService> service_;
  std::unique_ptr<ServiceDispatcher> dispatcher_;
  std::thread dispatch_thread_;
};

class ClientChannelTest : public testing::Test {
 public:
  void SetUp() override {
    int channel_sockets[2] = {};
    ASSERT_EQ(
        0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, channel_sockets));
    LocalHandle service_channel{channel_sockets[0]};
    LocalHandle client_channel{channel_sockets[1]};

    service_runner_.reset(new TestServiceRunner{std::move(service_channel)});
    auto factory = ClientChannelFactory::Create(std::move(client_channel));
    auto status = factory->Connect(android::pdx::Client::kInfiniteTimeout);
    ASSERT_TRUE(status);
    client_ = TestClient::Create(status.take());
  }

  void TearDown() override {
    service_runner_.reset();
    client_.reset();
  }

 protected:
  std::unique_ptr<TestServiceRunner> service_runner_;
  std::shared_ptr<TestClient> client_;
};

TEST_F(ClientChannelTest, MultithreadedClient) {
  constexpr int kNumTestThreads = 8;
  constexpr size_t kDataSize = 1000;  // Try to keep RPC buffer size below 4K.

  std::random_device rd;
  std::mt19937 gen{rd()};
  std::uniform_int_distribution<TestProtocol::DataType> dist{
      std::numeric_limits<TestProtocol::DataType>::min(),
      std::numeric_limits<TestProtocol::DataType>::max()};

  auto worker = [](std::shared_ptr<TestClient> client,
                   std::vector<TestProtocol::DataType> data) {
    constexpr int kMaxIterations = 500;
    int64_t expected = std::accumulate(data.begin(), data.end(), int64_t{0});
    for (int i = 0; i < kMaxIterations; i++) {
      ASSERT_EQ(expected, client->Sum(data));
    }
  };

  // Start client threads.
  std::vector<TestProtocol::DataType> data;
  data.resize(kDataSize);
  std::vector<std::thread> threads;
  for (int i = 0; i < kNumTestThreads; i++) {
    std::generate(data.begin(), data.end(),
                  [&dist, &gen]() { return dist(gen); });
    threads.emplace_back(worker, client_, data);
  }

  // Wait for threads to finish.
  for (auto& thread : threads)
    thread.join();
}

}  // namespace
