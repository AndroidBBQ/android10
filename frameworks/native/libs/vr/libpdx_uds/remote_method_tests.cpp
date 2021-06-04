#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <thread>

#include <gtest/gtest.h>
#include <pdx/channel_handle.h>
#include <pdx/client.h>
#include <pdx/rpc/remote_method.h>
#include <pdx/rpc/serializable.h>
#include <pdx/service.h>
#include <pdx/service_dispatcher.h>
#include <uds/client_channel.h>
#include <uds/client_channel_factory.h>
#include <uds/service_endpoint.h>

using android::pdx::BorrowedHandle;
using android::pdx::Channel;
using android::pdx::ClientBase;
using android::pdx::ErrorStatus;
using android::pdx::LocalChannelHandle;
using android::pdx::LocalHandle;
using android::pdx::Message;
using android::pdx::RemoteChannelHandle;
using android::pdx::RemoteHandle;
using android::pdx::ServiceBase;
using android::pdx::ServiceDispatcher;
using android::pdx::Status;
using android::pdx::uds::Endpoint;
using namespace android::pdx::rpc;

namespace {

std::string Rot13(const std::string& s) {
  std::string text = s;
  std::transform(std::begin(text), std::end(text), std::begin(text),
                 [](char c) -> char {
                   if (!std::isalpha(c)) {
                     return c;
                   } else {
                     const char pivot = std::isupper(c) ? 'A' : 'a';
                     return (c - pivot + 13) % 26 + pivot;
                   }
                 });
  return text;
}

// Defines a serializable user type that may be transferred between client and
// service.
struct TestType {
  int a;
  float b;
  std::string c;

  TestType() {}
  TestType(int a, float b, const std::string& c) : a(a), b(b), c(c) {}

  // Make gtest expressions simpler by defining equality operator. This is not
  // needed for serialization.
  bool operator==(const TestType& other) const {
    return a == other.a && b == other.b && c == other.c;
  }

 private:
  PDX_SERIALIZABLE_MEMBERS(TestType, a, b, c);
};

struct DerivedTestType : public TestType {
  DerivedTestType() : TestType() {}
  DerivedTestType(int a, float b) : TestType(a, b, "constant") {}
};

// Defines a serializable user type with a LocalHandle member.
struct TestFdType {
  int a;
  LocalHandle fd;

  TestFdType() {}
  TestFdType(int a, LocalHandle fd) : a(a), fd(std::move(fd)) {}

 private:
  PDX_SERIALIZABLE_MEMBERS(TestFdType, a, fd);
};

// Defines a serializable user template type with a FileHandle member.
template <typename FileHandleType>
struct TestTemplateType {
  FileHandleType fd;

  TestTemplateType() {}
  explicit TestTemplateType(FileHandleType fd) : fd(std::move(fd)) {}

 private:
  PDX_SERIALIZABLE_MEMBERS(TestTemplateType<FileHandleType>, fd);
};

struct BasicStruct {
  int a;
  int b;
  std::string c;

 private:
  PDX_SERIALIZABLE_MEMBERS(BasicStruct, a, b, c);
};

using BasicStructTraits = SerializableTraits<BasicStruct>;

struct NonSerializableType {
  int a;
  int b;
  std::string c;
};

struct IncorrectlyDefinedSerializableType {
  int a;
  int b;

 private:
  using SerializableMembers = std::tuple<int, int>;
};

// Defines the contract between the client and service, including ServiceFS
// endpoint path, method opcodes, and remote method signatures.
struct TestInterface final {
  // Service path.
  static constexpr char kClientPath[] = "socket_test";

  // Op codes.
  enum {
    kOpAdd = 0,
    kOpFoo,
    kOpConcatenate,
    kOpWriteBuffer,
    kOpStringLength,
    kOpSendTestType,
    kOpSendBasicStruct,
    kOpSendVector,
    kOpRot13,
    kOpNoArgs,
    kOpSendFile,
    kOpGetFile,
    kOpGetTestFdType,
    kOpOpenFiles,
    kOpReadFile,
    kOpPushChannel,
    kOpPositive,
  };

  // Methods.
  PDX_REMOTE_METHOD(Add, kOpAdd, int(int, int));
  PDX_REMOTE_METHOD(Foo, kOpFoo, int(int, const std::string&));
  PDX_REMOTE_METHOD(Concatenate, kOpConcatenate,
                    std::string(const std::string&, const std::string&));
  PDX_REMOTE_METHOD(SumVector, kOpWriteBuffer, int(const std::vector<int>&));
  PDX_REMOTE_METHOD(StringLength, kOpStringLength, int(const std::string&));
  PDX_REMOTE_METHOD(SendTestType, kOpSendTestType, TestType(const TestType&));
  PDX_REMOTE_METHOD(SendBasicStruct, kOpSendBasicStruct,
                    BasicStruct(const BasicStruct&));
  PDX_REMOTE_METHOD(SendVector, kOpSendVector,
                    std::string(const std::vector<TestType>&));
  PDX_REMOTE_METHOD(Rot13, kOpRot13, std::string(const std::string&));
  PDX_REMOTE_METHOD(NoArgs, kOpNoArgs, int(Void));
  PDX_REMOTE_METHOD(SendFile, kOpSendFile, int(const LocalHandle& fd));
  PDX_REMOTE_METHOD(GetFile, kOpGetFile, LocalHandle(const std::string&, int));
  PDX_REMOTE_METHOD(GetTestFdType, kOpGetTestFdType,
                    TestFdType(int, const std::string&, int));
  PDX_REMOTE_METHOD(OpenFiles, kOpOpenFiles,
                    std::vector<LocalHandle>(
                        const std::vector<std::pair<std::string, int>>&));
  PDX_REMOTE_METHOD(ReadFile, kOpReadFile,
                    std::pair<int, BufferWrapper<std::uint8_t*>>(
                        const std::string&, int, std::size_t));
  PDX_REMOTE_METHOD(PushChannel, kOpPushChannel, LocalChannelHandle(Void));
  PDX_REMOTE_METHOD(Positive, kOpPositive, void(int));

  PDX_REMOTE_API(API, Add, Foo, Concatenate, SumVector, StringLength,
                 SendTestType, SendVector, Rot13, NoArgs, SendFile, GetFile,
                 GetTestFdType, OpenFiles, PushChannel, Positive);
};

constexpr char TestInterface::kClientPath[];

// Test client to send messages to the test service.
class TestClient : public ClientBase<TestClient> {
 public:
  int Add(int a, int b) {
    return ReturnStatusOrError(InvokeRemoteMethod<TestInterface::Add>(a, b));
  }

  int Foo(int a, const std::string& b) {
    return ReturnStatusOrError(InvokeRemoteMethod<TestInterface::Foo>(a, b));
  }

  std::string Concatenate(const std::string& a, const std::string& b) {
    std::string return_value;

    Status<std::string> status =
        InvokeRemoteMethod<TestInterface::Concatenate>(a, b);
    if (!status)
      return std::string("[Error]");
    else
      return status.take();
  }

  int SumVector(const int* buffer, std::size_t size) {
    return ReturnStatusOrError(
        InvokeRemoteMethod<TestInterface::SumVector>(WrapArray(buffer, size)));
  }

  int SumVector(const std::vector<int>& buffer) {
    return ReturnStatusOrError(
        InvokeRemoteMethod<TestInterface::SumVector>(buffer));
  }

  int StringLength(const char* string, std::size_t size) {
    return ReturnStatusOrError(InvokeRemoteMethod<TestInterface::StringLength>(
        WrapString(string, size)));
  }

  int StringLength(const std::string& string) {
    return ReturnStatusOrError(
        InvokeRemoteMethod<TestInterface::StringLength>(string));
  }

  TestType SendTestType(const TestType& tt) {
    Status<TestType> status =
        InvokeRemoteMethod<TestInterface::SendTestType>(tt);
    if (!status)
      return TestType(0, 0.0, "[Error]");
    else
      return status.take();
  }

  BasicStruct SendBasicStruct(const BasicStruct& bs) {
    Status<BasicStruct> status =
        InvokeRemoteMethod<TestInterface::SendBasicStruct>(bs);
    if (!status)
      return BasicStruct{0, 0, "[Error]"};
    else
      return status.take();
  }

  std::string SendVector(const std::vector<TestType>& v) {
    Status<std::string> status =
        InvokeRemoteMethod<TestInterface::SendVector>(v);
    if (!status)
      return "[Error]";
    else
      return status.take();
  }

  std::string Rot13(const std::string& string) {
    Status<std::string> status =
        InvokeRemoteMethod<TestInterface::Rot13>(string);
    return status ? status.get() : string;
  }

  int NoArgs() {
    return ReturnStatusOrError(InvokeRemoteMethod<TestInterface::NoArgs>());
  }

  int SendFile(const LocalHandle& fd) {
    return ReturnStatusOrError(InvokeRemoteMethod<TestInterface::SendFile>(fd));
  }

  LocalHandle GetFile(const std::string& path, int mode) {
    Status<LocalHandle> status =
        InvokeRemoteMethod<TestInterface::GetFile>(path, mode);
    if (!status)
      return LocalHandle(-status.error());
    else
      return status.take();
  }

  int GetFile(const std::string& path, int mode, LocalHandle* fd_out) {
    Status<void> status =
        InvokeRemoteMethodInPlace<TestInterface::GetFile>(fd_out, path, mode);
    return status ? 0 : -status.error();
  }

  TestFdType GetTestFdType(int a, const std::string& path, int mode) {
    Status<TestFdType> status =
        InvokeRemoteMethod<TestInterface::GetTestFdType>(a, path, mode);
    if (!status)
      return {};
    else
      return status.take();
  }

  std::vector<LocalHandle> OpenFiles(
      const std::vector<std::pair<std::string, int>>& file_specs) {
    Status<std::vector<LocalHandle>> status =
        InvokeRemoteMethod<TestInterface::OpenFiles>(file_specs);
    if (!status)
      return {};
    else
      return status.take();
  }

  int ReadFile(void* buffer, std::size_t size, const std::string& path,
               int mode) {
    auto buffer_wrapper = WrapBuffer(buffer, size);
    auto return_value = std::make_pair(-1, buffer_wrapper);

    Status<void> status = InvokeRemoteMethodInPlace<TestInterface::ReadFile>(
        &return_value, path, mode, size);
    return status ? return_value.first : -status.error();
  }

  int PushChannel(LocalChannelHandle* fd_out) {
    auto status = InvokeRemoteMethodInPlace<TestInterface::PushChannel>(fd_out);
    return status ? 0 : -status.error();
  }

  bool Positive(int test_value) {
    auto status = InvokeRemoteMethod<TestInterface::Positive>(test_value);
    return status.ok();
  }

  int GetFd() const { return event_fd(); }

 private:
  friend BASE;

  explicit TestClient(LocalChannelHandle channel_handle)
      : BASE{android::pdx::uds::ClientChannel::Create(
            std::move(channel_handle))} {}
  TestClient()
      : BASE{android::pdx::uds::ClientChannelFactory::Create(
            TestInterface::kClientPath)} {}

  TestClient(const TestClient&) = delete;
  void operator=(const TestClient&) = delete;
};

// Test service that encodes/decodes messages from clients.
class TestService : public ServiceBase<TestService> {
 public:
  Status<void> HandleMessage(Message& message) override {
    switch (message.GetOp()) {
      case TestInterface::Add::Opcode:
        DispatchRemoteMethod<TestInterface::Add>(*this, &TestService::OnAdd,
                                                 message);
        return {};

      case TestInterface::Foo::Opcode:
        DispatchRemoteMethod<TestInterface::Foo>(*this, &TestService::OnFoo,
                                                 message);
        return {};

      case TestInterface::Concatenate::Opcode:
        DispatchRemoteMethod<TestInterface::Concatenate>(
            *this, &TestService::OnConcatenate, message);
        return {};

      case TestInterface::SumVector::Opcode:
        DispatchRemoteMethod<TestInterface::SumVector>(
            *this, &TestService::OnSumVector, message);
        return {};

      case TestInterface::StringLength::Opcode:
        DispatchRemoteMethod<TestInterface::StringLength>(
            *this, &TestService::OnStringLength, message);
        return {};

      case TestInterface::SendTestType::Opcode:
        DispatchRemoteMethod<TestInterface::SendTestType>(
            *this, &TestService::OnSendTestType, message);
        return {};

      case TestInterface::SendVector::Opcode:
        DispatchRemoteMethod<TestInterface::SendVector>(
            *this, &TestService::OnSendVector, message);
        return {};

      case TestInterface::Rot13::Opcode:
        DispatchRemoteMethod<TestInterface::Rot13>(*this, &TestService::OnRot13,
                                                   message);
        return {};

      case TestInterface::NoArgs::Opcode:
        DispatchRemoteMethod<TestInterface::NoArgs>(
            *this, &TestService::OnNoArgs, message);
        return {};

      case TestInterface::SendFile::Opcode:
        DispatchRemoteMethod<TestInterface::SendFile>(
            *this, &TestService::OnSendFile, message);
        return {};

      case TestInterface::GetFile::Opcode:
        DispatchRemoteMethod<TestInterface::GetFile>(
            *this, &TestService::OnGetFile, message);
        return {};

      case TestInterface::GetTestFdType::Opcode:
        DispatchRemoteMethod<TestInterface::GetTestFdType>(
            *this, &TestService::OnGetTestFdType, message);
        return {};

      case TestInterface::OpenFiles::Opcode:
        DispatchRemoteMethod<TestInterface::OpenFiles>(
            *this, &TestService::OnOpenFiles, message);
        return {};

      case TestInterface::ReadFile::Opcode:
        DispatchRemoteMethod<TestInterface::ReadFile>(
            *this, &TestService::OnReadFile, message);
        return {};

      case TestInterface::PushChannel::Opcode:
        DispatchRemoteMethod<TestInterface::PushChannel>(
            *this, &TestService::OnPushChannel, message);
        return {};

      case TestInterface::Positive::Opcode:
        DispatchRemoteMethod<TestInterface::Positive>(
            *this, &TestService::OnPositive, message);
        return {};

      default:
        return Service::DefaultHandleMessage(message);
    }
  }

 private:
  friend BASE;

  TestService()
      : BASE("TestService",
             Endpoint::CreateAndBindSocket(TestInterface::kClientPath)) {}

  int OnAdd(Message&, int a, int b) { return a + b; }

  int OnFoo(Message&, int a, const std::string& b) { return a + b.length(); }

  std::string OnConcatenate(Message&, const std::string& a,
                            const std::string& b) {
    return a + b;
  }

  int OnSumVector(Message&, const std::vector<int>& vector) {
    return std::accumulate(vector.begin(), vector.end(), 0);
  }

  int OnStringLength(Message&, const std::string& string) {
    return string.length();
  }

  TestType OnSendTestType(Message&, const TestType& tt) {
    return TestType(tt.a + 20, tt.b - 2.0, tt.c + "foo");
  }

  std::string OnSendVector(Message&, const std::vector<TestType>& v) {
    std::string return_value = "";

    for (const auto& tt : v)
      return_value += tt.c;

    return return_value;
  }

  Status<std::string> OnRot13(Message&, const std::string& s) {
    return {Rot13(s)};
  }

  int OnNoArgs(Message&) { return 1; }

  int OnSendFile(Message&, const LocalHandle& fd) { return fd.Get(); }

  LocalHandle OnGetFile(Message& message, const std::string& path, int mode) {
    LocalHandle fd(path.c_str(), mode);
    if (!fd)
      message.ReplyError(errno);
    return fd;
  }

  TestFdType OnGetTestFdType(Message& message, int a, const std::string& path,
                             int mode) {
    TestFdType return_value(a, LocalHandle(path, mode));
    if (!return_value.fd)
      message.ReplyError(errno);
    return return_value;
  }

  std::vector<LocalHandle> OnOpenFiles(
      Message&, const std::vector<std::pair<std::string, int>>& file_specs) {
    std::vector<LocalHandle> return_value;
    for (auto& spec : file_specs) {
      LocalHandle fd(spec.first, spec.second);
      if (fd)
        return_value.emplace_back(std::move(fd));
      else
        return_value.emplace_back(-errno);
    }
    return return_value;
  }

  std::pair<int, BufferWrapper<std::vector<std::uint8_t>>> OnReadFile(
      Message& message, const std::string& path, int mode, std::size_t length) {
    std::pair<int, BufferWrapper<std::vector<std::uint8_t>>> return_value;
    LocalHandle fd(path, mode);
    if (!fd) {
      message.ReplyError(errno);
      return return_value;
    }

    return_value.second.reserve(length);
    const int ret = read(fd.Get(), return_value.second.data(), length);
    if (ret < 0) {
      message.ReplyError(errno);
      return return_value;
    }

    return_value.second.resize(ret);
    return_value.first = ret;
    return return_value;
  }

  RemoteChannelHandle OnPushChannel(Message& message) {
    auto status = message.PushChannel(0, nullptr, nullptr);
    if (!status) {
      message.ReplyError(status.error());
      return {};
    }
    return status.take();
  }

  Status<void> OnPositive(Message& /*message*/, int test_value) {
    if (test_value >= 0)
      return {};
    else
      return ErrorStatus(EINVAL);
  }

  TestService(const TestService&) = delete;
  void operator=(const TestService&) = delete;
};

}  // anonymous namespace

// Use a test fixture to ensure proper order of cleanup between clients,
// services, and the dispatcher. As these objects are cleaned up in the same
// thread, either the service or client must be destroyed before stopping the
// dispatcher. The reason for this is that clients send blocking "close"
// messages to their respective services on destruction. If this happens after
// stopping the dispatcher the client destructor will get blocked waiting for a
// reply that will never come. In normal use of the service framework this is
// never an issue because clients and the dispatcher for the same service are
// never destructed in the same thread (they live in different processes).
class RemoteMethodTest : public ::testing::Test {
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
      // Cancel the dispatcher and wait for the thread to terminate.
      // Explicitly
      // join the thread so that destruction doesn't deallocate the
      // dispatcher
      // before the thread finishes.
      dispatcher_->SetCanceled(true);
      dispatch_thread_.join();
    }
  }
};

// Test basic operation of TestService/TestClient classes.
TEST_F(RemoteMethodTest, BasicClientService) {
  // Create a test service and add it to the dispatcher.

  auto service = TestService::Create();
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create();
  ASSERT_NE(nullptr, client);

  const int sum = client->Add(10, 25);
  EXPECT_GE(35, sum);

  const auto cat = client->Concatenate("This is a string", ", that it is.");
  EXPECT_EQ("This is a string, that it is.", cat);

  std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
  const auto rot13_alphabet = client->Rot13(alphabet);
  EXPECT_EQ(Rot13(alphabet), rot13_alphabet);

  const auto length = client->Foo(10, "123");
  EXPECT_EQ(13, length);

  const std::vector<int> vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  const int vector_sum = client->SumVector(vector.data(), vector.size());
  const int vector_sum2 = client->SumVector(vector);
  EXPECT_EQ(std::accumulate(vector.begin(), vector.end(), 0), vector_sum);
  EXPECT_EQ(std::accumulate(vector.begin(), vector.end(), 0), vector_sum2);

  const auto string_length1 = client->StringLength("This is a string");
  EXPECT_EQ(16, string_length1);

  const auto string_length2 = client->StringLength("1234567890");
  EXPECT_EQ(10, string_length2);

  std::string string = "1234567890";
  const auto string_length3 =
      client->StringLength(string.c_str(), string.length());
  EXPECT_EQ(10, string_length3);

  TestType tt{10, 0.0, "string"};
  const auto tt_result = client->SendTestType(tt);
  EXPECT_EQ(TestType(30, -2.0, "stringfoo"), tt_result);

  std::vector<TestType> ttv = {TestType(0, 0.0, "abc"),
                               TestType(0, 0.0, "123")};
  const std::string string_result = client->SendVector(ttv);
  EXPECT_EQ("abc123", string_result);

  const int int_result = client->NoArgs();
  EXPECT_EQ(1, int_result);
}

TEST_F(RemoteMethodTest, LocalHandle) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create();
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create();
  ASSERT_NE(nullptr, client);

  LocalHandle fd("/dev/zero", O_RDONLY);
  ASSERT_TRUE(fd.IsValid());

  int fd_result = client->SendFile(fd);
  EXPECT_LE(0, fd_result);
  EXPECT_NE(fd.Get(), fd_result);
  fd = LocalHandle(-3);
  fd_result = client->SendFile(fd);
  EXPECT_EQ(fd.Get(), fd_result);

  fd = client->GetFile("/dev/zero", O_RDONLY);
  ASSERT_TRUE(fd.IsValid()) << "Error code: " << fd.Get();

  std::array<uint8_t, 10> buffer;
  buffer.fill(1);
  EXPECT_EQ(10, read(fd.Get(), buffer.data(), buffer.size()));
  EXPECT_EQ(buffer, decltype(buffer){{0}});
  fd.Close();

  const int error = client->GetFile("/dev/zero", O_RDONLY, &fd);
  EXPECT_EQ(0, error);
  EXPECT_TRUE(fd.IsValid());

  buffer.fill(1);
  EXPECT_EQ(10, read(fd.Get(), buffer.data(), buffer.size()));
  EXPECT_EQ(buffer, decltype(buffer){{0}});

  /*
    Seg fault.
    fd = client->GetFile("/dev/foobar", O_RDONLY);
    EXPECT_FALSE(fd.IsValid());
   */
}

TEST_F(RemoteMethodTest, PushChannel) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create();
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create();
  ASSERT_NE(nullptr, client);

  // Get a new channel as an fd.
  LocalChannelHandle channel;
  const int ret = client->PushChannel(&channel);
  EXPECT_EQ(0, ret);
  EXPECT_TRUE(channel.valid());

  // Create a new client from the channel.
  auto client2 = TestClient::Create(std::move(channel));
  ASSERT_NE(nullptr, client2);

  // Test that the new channel works.
  const int sum = client2->Add(10, 25);
  EXPECT_GE(35, sum);
}

TEST_F(RemoteMethodTest, Positive) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create();
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create();
  ASSERT_NE(nullptr, client);

  ASSERT_TRUE(client->Positive(0));
  ASSERT_TRUE(client->Positive(1));
  ASSERT_FALSE(client->Positive(-1));
}

TEST_F(RemoteMethodTest, AggregateLocalHandle) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create();
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create();
  ASSERT_NE(nullptr, client);

  TestFdType result = client->GetTestFdType(10, "/dev/zero", O_RDONLY);
  EXPECT_TRUE(result.fd.IsValid());
  EXPECT_EQ(10, result.a);

  std::vector<LocalHandle> files =
      client->OpenFiles({{{"/dev/zero", O_RDONLY},
                          {"/dev/null", O_WRONLY},
                          {"/dev/zero", O_RDONLY}}});
  ASSERT_EQ(3u, files.size());
  EXPECT_TRUE(files[0].IsValid());
  EXPECT_TRUE(files[1].IsValid());
  EXPECT_TRUE(files[2].IsValid());
}

TEST_F(RemoteMethodTest, BufferWrapper) {
  // Create a test service and add it to the dispatcher.
  auto service = TestService::Create();
  ASSERT_NE(nullptr, service);
  ASSERT_EQ(0, dispatcher_->AddService(service));

  // Create a client to service.
  auto client = TestClient::Create();
  ASSERT_NE(nullptr, client);

  const int buffer_size = 20;
  std::vector<std::uint8_t> buffer(buffer_size, 'x');
  std::vector<std::uint8_t> expected(buffer_size, 0);
  int ret =
      client->ReadFile(buffer.data(), buffer.size(), "/dev/zero", O_RDONLY);
  EXPECT_EQ(buffer_size, ret);
  EXPECT_EQ(expected, buffer);
}

//
// RemoteMethodFramework: Tests the type-based framework that remote method
// support is built upon.
//

// Test logical And template.
TEST(RemoteMethodFramework, And) {
  EXPECT_TRUE((And<std::true_type, std::true_type>::value));
  EXPECT_FALSE((And<std::true_type, std::false_type>::value));
  EXPECT_FALSE((And<std::false_type, std::true_type>::value));
  EXPECT_FALSE((And<std::false_type, std::false_type>::value));

  EXPECT_TRUE((And<std::true_type, std::true_type, std::true_type>::value));
  EXPECT_FALSE((And<std::true_type, std::true_type, std::false_type>::value));
  EXPECT_FALSE((And<std::true_type, std::false_type, std::true_type>::value));
  EXPECT_FALSE((And<std::true_type, std::false_type, std::false_type>::value));
  EXPECT_FALSE((And<std::false_type, std::true_type, std::true_type>::value));
  EXPECT_FALSE((And<std::false_type, std::true_type, std::false_type>::value));
  EXPECT_FALSE((And<std::false_type, std::false_type, std::true_type>::value));
  EXPECT_FALSE((And<std::false_type, std::false_type, std::false_type>::value));
}

// Test convertible type constraints.
TEST(RemoteMethodFramework, IsConvertible) {
  // std::pair.
  EXPECT_TRUE(
      (IsConvertible<std::pair<int, float>, std::pair<int, float>>::value));
  EXPECT_FALSE(
      (IsConvertible<std::pair<int, float>, std::pair<float, float>>::value));
  EXPECT_FALSE(
      (IsConvertible<std::pair<int, float>, std::pair<float, int>>::value));

  // Nested std::pair.
  EXPECT_TRUE((IsConvertible<std::pair<std::pair<int, float>, float>,
                             std::pair<std::pair<int, float>, float>>::value));
  EXPECT_FALSE((IsConvertible<std::pair<std::pair<int, float>, float>,
                              std::pair<std::pair<float, int>, float>>::value));

  // std::tuple and std::pair.
  EXPECT_TRUE(
      (IsConvertible<std::pair<int, float>, std::tuple<int, float>>::value));
  EXPECT_TRUE(
      (IsConvertible<std::tuple<int, float>, std::pair<int, float>>::value));
  EXPECT_FALSE(
      (IsConvertible<std::pair<float, float>, std::tuple<int, float>>::value));
  EXPECT_FALSE(
      (IsConvertible<std::tuple<float, float>, std::pair<int, float>>::value));
  EXPECT_FALSE(
      (IsConvertible<std::pair<int, int>, std::tuple<int, float>>::value));
  EXPECT_FALSE(
      (IsConvertible<std::tuple<int, int>, std::pair<int, float>>::value));
  EXPECT_FALSE(
      (IsConvertible<std::pair<int, int>, std::tuple<int, int, int>>::value));
  EXPECT_FALSE(
      (IsConvertible<std::tuple<int, int, int>, std::pair<int, int>>::value));

  // std::vector.
  EXPECT_TRUE((IsConvertible<std::vector<int>, std::vector<int>>::value));
  EXPECT_FALSE((IsConvertible<std::vector<int>, std::vector<float>>::value));

  // Nested std::vector.
  EXPECT_TRUE((IsConvertible<std::vector<std::pair<int, int>>,
                             std::vector<std::pair<int, int>>>::value));
  EXPECT_FALSE((IsConvertible<std::vector<std::pair<int, int>>,
                              std::vector<std::pair<int, float>>>::value));
  EXPECT_FALSE((IsConvertible<std::vector<std::pair<int, int>>,
                              std::vector<std::pair<float, int>>>::value));
  EXPECT_FALSE((IsConvertible<std::vector<std::pair<int, int>>,
                              std::vector<std::pair<float, float>>>::value));

  // std::vector with nested convertible types.
  EXPECT_TRUE((IsConvertible<std::vector<StringWrapper<char>>,
                             std::vector<std::string>>::value));

  // std::map and std::unordered_map.
  EXPECT_TRUE((IsConvertible<std::map<int, float>,
                             std::unordered_map<int, float>>::value));
  EXPECT_FALSE((IsConvertible<std::map<float, float>,
                              std::unordered_map<int, float>>::value));
  EXPECT_FALSE((IsConvertible<std::map<float, float>,
                              std::unordered_map<float, int>>::value));
  EXPECT_FALSE((IsConvertible<std::map<float, float>,
                              std::unordered_map<int, int>>::value));
  EXPECT_TRUE((IsConvertible<std::unordered_map<int, float>,
                             std::map<int, float>>::value));
  EXPECT_FALSE((IsConvertible<std::unordered_map<float, float>,
                              std::map<int, float>>::value));
  EXPECT_FALSE((IsConvertible<std::unordered_map<float, float>,
                              std::map<float, int>>::value));
  EXPECT_FALSE((IsConvertible<std::unordered_map<float, float>,
                              std::map<int, int>>::value));

  // std::map with nested convertible types.
  EXPECT_TRUE((IsConvertible<std::map<int, std::string>,
                             std::map<int, StringWrapper<char>>>::value));
  EXPECT_TRUE(
      (IsConvertible<std::map<std::tuple<int, int>, std::string>,
                     std::map<std::pair<int, int>, std::string>>::value));

  // std::unordered_map with nested convertible types.
  EXPECT_TRUE(
      (IsConvertible<std::unordered_map<int, std::string>,
                     std::unordered_map<int, StringWrapper<char>>>::value));
  EXPECT_TRUE((IsConvertible<
               std::unordered_map<std::tuple<int, int>, std::string>,
               std::unordered_map<std::pair<int, int>, std::string>>::value));

  // std::string.
  EXPECT_TRUE((IsConvertible<std::string, std::string>::value));
  EXPECT_FALSE((IsConvertible<std::string, int>::value));
  EXPECT_FALSE((IsConvertible<int, std::string>::value));

  // Nested std::string.
  EXPECT_TRUE((IsConvertible<std::pair<std::string, std::string>,
                             std::pair<std::string, std::string>>::value));
  EXPECT_FALSE((IsConvertible<std::pair<std::string, std::string>,
                              std::pair<std::string, int>>::value));
  EXPECT_FALSE((IsConvertible<std::pair<std::string, std::string>,
                              std::pair<int, std::string>>::value));
  EXPECT_FALSE((IsConvertible<std::pair<std::string, std::string>,
                              std::pair<int, int>>::value));

  // StringWrapper.
  EXPECT_TRUE((IsConvertible<StringWrapper<char>, StringWrapper<char>>::value));
  EXPECT_TRUE((IsConvertible<StringWrapper<char>, std::string>::value));
  EXPECT_TRUE((IsConvertible<std::string, StringWrapper<char>>::value));
  EXPECT_FALSE((IsConvertible<StringWrapper<char>, int>::value));
  EXPECT_FALSE(
      (IsConvertible<StringWrapper<char>, BufferWrapper<char*>>::value));

  // BufferWrapper.
  EXPECT_TRUE(
      (IsConvertible<BufferWrapper<char*>, BufferWrapper<char*>>::value));
  EXPECT_TRUE(
      (IsConvertible<BufferWrapper<char*>, BufferWrapper<const char*>>::value));
  EXPECT_FALSE(
      (IsConvertible<BufferWrapper<char*>, BufferWrapper<int*>>::value));
  EXPECT_TRUE((IsConvertible<BufferWrapper<char*>,
                             BufferWrapper<std::vector<char>>>::value));

  // RemoteHandle and BorrowedHandle.
  EXPECT_TRUE((IsConvertible<LocalHandle, RemoteHandle>::value));
  EXPECT_TRUE((IsConvertible<LocalHandle, BorrowedHandle>::value));

  // Test rewriting user defined types.
  EXPECT_TRUE((IsConvertible<TestTemplateType<LocalHandle>,
                             TestTemplateType<RemoteHandle>>::value));
  EXPECT_TRUE((IsConvertible<TestTemplateType<LocalHandle>,
                             TestTemplateType<BorrowedHandle>>::value));
  EXPECT_FALSE((IsConvertible<TestTemplateType<RemoteHandle>,
                              TestTemplateType<LocalHandle>>::value));
  EXPECT_FALSE((IsConvertible<TestTemplateType<BorrowedHandle>,
                              TestTemplateType<LocalHandle>>::value));

  // TODO(eieio): More thorough testing of convertible types.
}

TEST(RemoteMethodFramework, SerializableMembers) {
  EXPECT_TRUE(HasSerializableMembers<TestTemplateType<LocalHandle>>::value);
  EXPECT_TRUE(HasSerializableMembers<TestTemplateType<RemoteHandle>>::value);
  EXPECT_TRUE(HasSerializableMembers<TestTemplateType<BorrowedHandle>>::value);

  EXPECT_TRUE(std::is_void<EnableIfHasSerializableMembers<
                  TestTemplateType<LocalHandle>>>::value);
  EXPECT_TRUE(std::is_void<EnableIfHasSerializableMembers<
                  TestTemplateType<RemoteHandle>>>::value);
  EXPECT_TRUE(std::is_void<EnableIfHasSerializableMembers<
                  TestTemplateType<BorrowedHandle>>>::value);

  EXPECT_TRUE(HasSerializableMembers<DerivedTestType>::value);

  EXPECT_TRUE(HasSerializableMembers<BasicStruct>::value);
  EXPECT_TRUE(HasSerializableMembers<TestType>::value);
  EXPECT_TRUE(HasSerializableMembers<TestTemplateType<LocalHandle>>::value);
  EXPECT_TRUE(HasSerializableMembers<TestTemplateType<RemoteHandle>>::value);
  EXPECT_TRUE(HasSerializableMembers<TestTemplateType<BorrowedHandle>>::value);
  EXPECT_TRUE(HasSerializableMembers<DerivedTestType>::value);
  EXPECT_FALSE(HasSerializableMembers<NonSerializableType>::value);
  EXPECT_FALSE(
      HasSerializableMembers<IncorrectlyDefinedSerializableType>::value);
}

TEST(RemoteMethodFramework, RemoteAPITypes) {
  EXPECT_EQ(0u, TestInterface::API::MethodIndex<TestInterface::Add>());
}
