// Use ALWAYS at the tag level. Control is performed manually during command
// line processing.
#define ATRACE_TAG ATRACE_TAG_ALWAYS
#include <utils/Trace.h>

#include <base/files/file_util.h>
#include <base/logging.h>
#include <base/strings/string_split.h>
#include <errno.h>
#include <getopt.h>
#include <pdx/client.h>
#include <pdx/default_transport/client_channel_factory.h>
#include <pdx/default_transport/service_endpoint.h>
#include <pdx/rpc/buffer_wrapper.h>
#include <pdx/rpc/default_initialization_allocator.h>
#include <pdx/rpc/message_buffer.h>
#include <pdx/rpc/remote_method.h>
#include <pdx/rpc/serializable.h>
#include <pdx/service.h>
#include <sys/prctl.h>
#include <time.h>
#include <unistd.h>

#include <atomic>
#include <cstdlib>
#include <functional>
#include <future>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using android::pdx::Channel;
using android::pdx::ClientBase;
using android::pdx::Endpoint;
using android::pdx::ErrorStatus;
using android::pdx::Message;
using android::pdx::Service;
using android::pdx::ServiceBase;
using android::pdx::default_transport::ClientChannelFactory;
using android::pdx::Status;
using android::pdx::Transaction;
using android::pdx::rpc::BufferWrapper;
using android::pdx::rpc::DefaultInitializationAllocator;
using android::pdx::rpc::MessageBuffer;
using android::pdx::rpc::DispatchRemoteMethod;
using android::pdx::rpc::RemoteMethodReturn;
using android::pdx::rpc::ReplyBuffer;
using android::pdx::rpc::Void;
using android::pdx::rpc::WrapBuffer;

namespace {

constexpr size_t kMaxMessageSize = 4096 * 1024;

std::string GetServicePath(const std::string& path, int instance_id) {
  return path + std::to_string(instance_id);
}

void SetThreadName(const std::string& name) {
  prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name.c_str()), 0, 0, 0);
}

constexpr uint64_t kNanosPerSecond = 1000000000LLU;

uint64_t GetClockNs() {
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return kNanosPerSecond * t.tv_sec + t.tv_nsec;
}

template <typename T>
ssize_t ssizeof(const T&) {
  return static_cast<ssize_t>(sizeof(T));
}

class SchedStats {
 public:
  SchedStats() : SchedStats(gettid()) {}
  explicit SchedStats(pid_t task_id) : task_id_(task_id) {}
  SchedStats(const SchedStats&) = default;
  SchedStats& operator=(const SchedStats&) = default;

  void Update() {
    const std::string stats_path =
        "/proc/" + std::to_string(task_id_) + "/schedstat";

    std::string line;
    base::ReadFileToString(base::FilePath{stats_path}, &line);
    std::vector<std::string> stats = base::SplitString(
        line, " ", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);

    CHECK_EQ(3u, stats.size());

    // Calculate the deltas since the last update. Each value is absolute since
    // the task started.
    uint64_t current_cpu_time_ns = std::stoull(stats[0]);
    uint64_t current_wait_ns = std::stoull(stats[1]);
    uint64_t current_timeslices = std::stoull(stats[2]);
    cpu_time_ns_ = current_cpu_time_ns - last_cpu_time_ns_;
    wait_ns_ = current_wait_ns - last_wait_ns_;
    timeslices_ = current_timeslices - last_timeslices_;
    last_cpu_time_ns_ = current_cpu_time_ns;
    last_wait_ns_ = current_wait_ns;
    last_timeslices_ = current_timeslices;
  }

  pid_t task_id() const { return task_id_; }
  uint64_t cpu_time_ns() const { return cpu_time_ns_; }
  uint64_t wait_ns() const { return wait_ns_; }
  uint64_t timeslices() const { return timeslices_; }

  double cpu_time_s() const {
    return static_cast<double>(cpu_time_ns_) / kNanosPerSecond;
  }
  double wait_s() const {
    return static_cast<double>(wait_ns_) / kNanosPerSecond;
  }

 private:
  int32_t task_id_;
  uint64_t cpu_time_ns_ = 0;
  uint64_t last_cpu_time_ns_ = 0;
  uint64_t wait_ns_ = 0;
  uint64_t last_wait_ns_ = 0;
  uint64_t timeslices_ = 0;
  uint64_t last_timeslices_ = 0;

  PDX_SERIALIZABLE_MEMBERS(SchedStats, task_id_, cpu_time_ns_, wait_ns_,
                           timeslices_);
};

// Opcodes for client/service protocol.
struct BenchmarkOps {
  enum : int {
    Nop,
    Read,
    Write,
    Echo,
    Stats,
    WriteVector,
    EchoVector,
    Quit,
  };
};

struct BenchmarkRPC {
  PDX_REMOTE_METHOD(Stats, BenchmarkOps::Stats,
                    std::tuple<uint64_t, uint64_t, SchedStats>(Void));
  PDX_REMOTE_METHOD(WriteVector, BenchmarkOps::WriteVector,
                    int(const BufferWrapper<std::vector<uint8_t>> data));
  PDX_REMOTE_METHOD(EchoVector, BenchmarkOps::EchoVector,
                    BufferWrapper<std::vector<uint8_t>>(
                        const BufferWrapper<std::vector<uint8_t>> data));
};

struct BenchmarkResult {
  int thread_id = 0;
  int service_id = 0;
  double time_delta_s = 0.0;
  uint64_t bytes_sent = 0;
  SchedStats sched_stats = {};
};

// Global command line option values.
struct Options {
  bool verbose = false;
  int threads = 1;
  int opcode = BenchmarkOps::Read;
  int blocksize = 1;
  int count = 1;
  int instances = 1;
  int timeout = 1;
  int warmup = 0;
} ProgramOptions;

// Command line option names.
const char kOptionService[] = "service";
const char kOptionClient[] = "client";
const char kOptionVerbose[] = "verbose";
const char kOptionOpcode[] = "op";
const char kOptionBlocksize[] = "bs";
const char kOptionCount[] = "count";
const char kOptionThreads[] = "threads";
const char kOptionInstances[] = "instances";
const char kOptionTimeout[] = "timeout";
const char kOptionTrace[] = "trace";
const char kOptionWarmup[] = "warmup";

// getopt() long options.
static option long_options[] = {
    {kOptionService, required_argument, 0, 0},
    {kOptionClient, required_argument, 0, 0},
    {kOptionVerbose, no_argument, 0, 0},
    {kOptionOpcode, required_argument, 0, 0},
    {kOptionBlocksize, required_argument, 0, 0},
    {kOptionCount, required_argument, 0, 0},
    {kOptionThreads, required_argument, 0, 0},
    {kOptionInstances, required_argument, 0, 0},
    {kOptionTimeout, required_argument, 0, 0},
    {kOptionTrace, no_argument, 0, 0},
    {kOptionWarmup, required_argument, 0, 0},
    {0, 0, 0, 0},
};

// Parses the argument for kOptionOpcode and sets the value of
// ProgramOptions.opcode.
void ParseOpcodeOption(const std::string& argument) {
  if (argument == "read") {
    ProgramOptions.opcode = BenchmarkOps::Read;
  } else if (argument == "write") {
    ProgramOptions.opcode = BenchmarkOps::Write;
  } else if (argument == "echo") {
    ProgramOptions.opcode = BenchmarkOps::Echo;
  } else if (argument == "writevec") {
    ProgramOptions.opcode = BenchmarkOps::WriteVector;
  } else if (argument == "echovec") {
    ProgramOptions.opcode = BenchmarkOps::EchoVector;
  } else if (argument == "quit") {
    ProgramOptions.opcode = BenchmarkOps::Quit;
  } else if (argument == "nop") {
    ProgramOptions.opcode = BenchmarkOps::Nop;
  } else if (argument == "stats") {
    ProgramOptions.opcode = BenchmarkOps::Stats;
  } else {
    ProgramOptions.opcode = std::stoi(argument);
  }
}

// Implements the service side of the benchmark.
class BenchmarkService : public ServiceBase<BenchmarkService> {
 public:
  std::shared_ptr<Channel> OnChannelOpen(Message& message) override {
    VLOG(1) << "BenchmarkService::OnChannelCreate: cid="
            << message.GetChannelId();
    return nullptr;
  }

  void OnChannelClose(Message& message,
                      const std::shared_ptr<Channel>& /*channel*/) override {
    VLOG(1) << "BenchmarkService::OnChannelClose: cid="
            << message.GetChannelId();
  }

  Status<void> HandleMessage(Message& message) override {
    ATRACE_NAME("BenchmarkService::HandleMessage");

    switch (message.GetOp()) {
      case BenchmarkOps::Nop:
        VLOG(1) << "BenchmarkService::HandleMessage: op=nop";
        {
          ATRACE_NAME("Reply");
          CHECK(message.Reply(0));
        }
        return {};

      case BenchmarkOps::Write: {
        VLOG(1) << "BenchmarkService::HandleMessage: op=write send_length="
                << message.GetSendLength()
                << " receive_length=" << message.GetReceiveLength();

        Status<void> status;
        if (message.GetSendLength())
          status = message.ReadAll(send_buffer.data(), message.GetSendLength());

        {
          ATRACE_NAME("Reply");
          if (!status)
            CHECK(message.ReplyError(status.error()));
          else
            CHECK(message.Reply(message.GetSendLength()));
        }
        return {};
      }

      case BenchmarkOps::Read: {
        VLOG(1) << "BenchmarkService::HandleMessage: op=read send_length="
                << message.GetSendLength()
                << " receive_length=" << message.GetReceiveLength();

        Status<void> status;
        if (message.GetReceiveLength()) {
          status = message.WriteAll(receive_buffer.data(),
                                    message.GetReceiveLength());
        }

        {
          ATRACE_NAME("Reply");
          if (!status)
            CHECK(message.ReplyError(status.error()));
          else
            CHECK(message.Reply(message.GetReceiveLength()));
        }
        return {};
      }

      case BenchmarkOps::Echo: {
        VLOG(1) << "BenchmarkService::HandleMessage: op=echo send_length="
                << message.GetSendLength()
                << " receive_length=" << message.GetReceiveLength();

        Status<void> status;
        if (message.GetSendLength())
          status = message.ReadAll(send_buffer.data(), message.GetSendLength());

        if (!status) {
          CHECK(message.ReplyError(status.error()));
          return {};
        }

        if (message.GetSendLength()) {
          status =
              message.WriteAll(send_buffer.data(), message.GetSendLength());
        }

        {
          ATRACE_NAME("Reply");
          if (!status)
            CHECK(message.ReplyError(status.error()));
          else
            CHECK(message.Reply(message.GetSendLength()));
        }
        return {};
      }

      case BenchmarkOps::Stats: {
        VLOG(1) << "BenchmarkService::HandleMessage: op=echo send_length="
                << message.GetSendLength()
                << " receive_length=" << message.GetReceiveLength();

        // Snapshot the stats when the message is received.
        const uint64_t receive_time_ns = GetClockNs();
        sched_stats_.Update();

        // Use the RPC system to return the results.
        RemoteMethodReturn<BenchmarkRPC::Stats>(
            message, BenchmarkRPC::Stats::Return{receive_time_ns, GetClockNs(),
                                                 sched_stats_});
        return {};
      }

      case BenchmarkOps::WriteVector:
        VLOG(1) << "BenchmarkService::HandleMessage: op=writevec send_length="
                << message.GetSendLength()
                << " receive_length=" << message.GetReceiveLength();

        DispatchRemoteMethod<BenchmarkRPC::WriteVector>(
            *this, &BenchmarkService::OnWriteVector, message, kMaxMessageSize);
        return {};

      case BenchmarkOps::EchoVector:
        VLOG(1) << "BenchmarkService::HandleMessage: op=echovec send_length="
                << message.GetSendLength()
                << " receive_length=" << message.GetReceiveLength();

        DispatchRemoteMethod<BenchmarkRPC::EchoVector>(
            *this, &BenchmarkService::OnEchoVector, message, kMaxMessageSize);
        return {};

      case BenchmarkOps::Quit:
        Cancel();
        return ErrorStatus{ESHUTDOWN};

      default:
        VLOG(1) << "BenchmarkService::HandleMessage: default case; op="
                << message.GetOp();
        return Service::DefaultHandleMessage(message);
    }
  }

  // Updates the scheduler stats from procfs for this thread.
  void UpdateSchedStats() { sched_stats_.Update(); }

 private:
  friend BASE;

  explicit BenchmarkService(std::unique_ptr<Endpoint> endpoint)
      : BASE("BenchmarkService", std::move(endpoint)),
        send_buffer(kMaxMessageSize),
        receive_buffer(kMaxMessageSize) {}

  std::vector<uint8_t> send_buffer;
  std::vector<uint8_t> receive_buffer;

  // Each service thread has its own scheduler stats object.
  static thread_local SchedStats sched_stats_;

  using BufferType = BufferWrapper<
      std::vector<uint8_t, DefaultInitializationAllocator<uint8_t>>>;

  int OnWriteVector(Message&, const BufferType& data) { return data.size(); }
  BufferType OnEchoVector(Message&, BufferType&& data) {
    return std::move(data);
  }

  BenchmarkService(const BenchmarkService&) = delete;
  void operator=(const BenchmarkService&) = delete;
};

thread_local SchedStats BenchmarkService::sched_stats_;

// Implements the client side of the benchmark.
class BenchmarkClient : public ClientBase<BenchmarkClient> {
 public:
  int Nop() {
    ATRACE_NAME("BenchmarkClient::Nop");
    VLOG(1) << "BenchmarkClient::Nop";
    Transaction transaction{*this};
    return ReturnStatusOrError(transaction.Send<int>(BenchmarkOps::Nop));
  }

  int Write(const void* buffer, size_t length) {
    ATRACE_NAME("BenchmarkClient::Write");
    VLOG(1) << "BenchmarkClient::Write: buffer=" << buffer
            << " length=" << length;
    Transaction transaction{*this};
    return ReturnStatusOrError(
        transaction.Send<int>(BenchmarkOps::Write, buffer, length, nullptr, 0));
    // return write(endpoint_fd(), buffer, length);
  }

  int Read(void* buffer, size_t length) {
    ATRACE_NAME("BenchmarkClient::Read");
    VLOG(1) << "BenchmarkClient::Read: buffer=" << buffer
            << " length=" << length;
    Transaction transaction{*this};
    return ReturnStatusOrError(
        transaction.Send<int>(BenchmarkOps::Read, nullptr, 0, buffer, length));
    // return read(endpoint_fd(), buffer, length);
  }

  int Echo(const void* send_buffer, size_t send_length, void* receive_buffer,
           size_t receive_length) {
    ATRACE_NAME("BenchmarkClient::Echo");
    VLOG(1) << "BenchmarkClient::Echo: send_buffer=" << send_buffer
            << " send_length=" << send_length
            << " receive_buffer=" << receive_buffer
            << " receive_length=" << receive_length;
    Transaction transaction{*this};
    return ReturnStatusOrError(
        transaction.Send<int>(BenchmarkOps::Echo, send_buffer, send_length,
                              receive_buffer, receive_length));
  }

  int Stats(std::tuple<uint64_t, uint64_t, SchedStats>* stats_out) {
    ATRACE_NAME("BenchmarkClient::Stats");
    VLOG(1) << "BenchmarkClient::Stats";

    auto status = InvokeRemoteMethodInPlace<BenchmarkRPC::Stats>(stats_out);
    return status ? 0 : -status.error();
  }

  int WriteVector(const BufferWrapper<std::vector<uint8_t>>& data) {
    ATRACE_NAME("BenchmarkClient::Stats");
    VLOG(1) << "BenchmarkClient::Stats";

    auto status = InvokeRemoteMethod<BenchmarkRPC::WriteVector>(data);
    return ReturnStatusOrError(status);
  }

  template <typename T>
  int WriteVector(const BufferWrapper<T>& data) {
    ATRACE_NAME("BenchmarkClient::WriteVector");
    VLOG(1) << "BenchmarkClient::WriteVector";

    auto status = InvokeRemoteMethod<BenchmarkRPC::WriteVector>(data);
    return ReturnStatusOrError(status);
  }

  template <typename T, typename U>
  int EchoVector(const BufferWrapper<T>& data, BufferWrapper<U>* data_out) {
    ATRACE_NAME("BenchmarkClient::EchoVector");
    VLOG(1) << "BenchmarkClient::EchoVector";

    MessageBuffer<ReplyBuffer>::Reserve(kMaxMessageSize - 1);
    auto status =
        InvokeRemoteMethodInPlace<BenchmarkRPC::EchoVector>(data_out, data);
    return status ? 0 : -status.error();
  }

  int Quit() {
    VLOG(1) << "BenchmarkClient::Quit";
    Transaction transaction{*this};
    return ReturnStatusOrError(transaction.Send<int>(BenchmarkOps::Echo));
  }

 private:
  friend BASE;

  explicit BenchmarkClient(const std::string& service_path)
      : BASE(ClientChannelFactory::Create(service_path),
             ProgramOptions.timeout) {}

  BenchmarkClient(const BenchmarkClient&) = delete;
  void operator=(const BenchmarkClient&) = delete;
};

// Creates a benchmark service at |path| and dispatches messages.
int ServiceCommand(const std::string& path) {
  if (path.empty())
    return -EINVAL;

  // Start the requested number of dispatch threads.
  std::vector<std::thread> dispatch_threads;
  int service_count = ProgramOptions.instances;
  int service_id_counter = 0;
  int thread_id_counter = 0;
  std::atomic<bool> done(false);

  while (service_count--) {
    std::cerr << "Starting service instance " << service_id_counter
              << std::endl;
    auto service = BenchmarkService::Create(
        android::pdx::default_transport::Endpoint::CreateAndBindSocket(
            GetServicePath(path, service_id_counter),
            android::pdx::default_transport::Endpoint::kBlocking));
    if (!service) {
      std::cerr << "Failed to create service instance!!" << std::endl;
      done = true;
      break;
    }

    int thread_count = ProgramOptions.threads;
    while (thread_count--) {
      std::cerr << "Starting dispatch thread " << thread_id_counter
                << " service " << service_id_counter << std::endl;

      dispatch_threads.emplace_back(
          [&](const int thread_id, const int service_id,
              const std::shared_ptr<BenchmarkService>& local_service) {
            SetThreadName("service" + std::to_string(service_id));

            // Read the initial schedstats for this thread from procfs.
            local_service->UpdateSchedStats();

            ATRACE_NAME("BenchmarkService::Dispatch");
            while (!done) {
              auto ret = local_service->ReceiveAndDispatch();
              if (!ret) {
                if (ret.error() != ESHUTDOWN) {
                  std::cerr << "Error while dispatching message on thread "
                            << thread_id << " service " << service_id << ": "
                            << ret.GetErrorMessage() << std::endl;
                } else {
                  std::cerr << "Quitting thread " << thread_id << " service "
                            << service_id << std::endl;
                }
                done = true;
                return;
              }
            }
          },
          thread_id_counter++, service_id_counter, service);
    }

    service_id_counter++;
  }

  // Wait for the dispatch threads to exit.
  for (auto& thread : dispatch_threads) {
    thread.join();
  }

  return 0;
}

int ClientCommand(const std::string& path) {
  // Start the requested number of client threads.
  std::vector<std::thread> client_threads;
  std::vector<std::future<BenchmarkResult>> client_results;
  int service_count = ProgramOptions.instances;
  int thread_id_counter = 0;
  int service_id_counter = 0;

  // Aggregate statistics, updated when worker threads exit.
  std::atomic<uint64_t> total_bytes(0);
  std::atomic<uint64_t> total_time_ns(0);

  // Samples for variance calculation.
  std::vector<uint64_t> latency_samples_ns(
      ProgramOptions.instances * ProgramOptions.threads * ProgramOptions.count);
  const size_t samples_per_thread = ProgramOptions.count;

  std::vector<uint8_t> send_buffer(ProgramOptions.blocksize);
  std::vector<uint8_t> receive_buffer(kMaxMessageSize);

  // Barriers for synchronizing thread start.
  std::vector<std::future<void>> ready_barrier_futures;
  std::promise<void> go_barrier_promise;
  std::future<void> go_barrier_future = go_barrier_promise.get_future();

  // Barrier for synchronizing thread tear down.
  std::promise<void> done_barrier_promise;
  std::future<void> done_barrier_future = done_barrier_promise.get_future();

  while (service_count--) {
    int thread_count = ProgramOptions.threads;
    while (thread_count--) {
      std::cerr << "Starting client thread " << thread_id_counter << " service "
                << service_id_counter << std::endl;

      std::promise<BenchmarkResult> result_promise;
      client_results.push_back(result_promise.get_future());

      std::promise<void> ready_barrier_promise;
      ready_barrier_futures.push_back(ready_barrier_promise.get_future());

      client_threads.emplace_back(
          [&](const int thread_id, const int service_id,
              std::promise<BenchmarkResult> result, std::promise<void> ready) {
            SetThreadName("client" + std::to_string(thread_id) + "/" +
                          std::to_string(service_id));

            ATRACE_NAME("BenchmarkClient::Dispatch");

            auto client =
                BenchmarkClient::Create(GetServicePath(path, service_id));
            if (!client) {
              std::cerr << "Failed to create client for service " << service_id
                        << std::endl;
              return -ENOMEM;
            }

            uint64_t* thread_samples =
                &latency_samples_ns[samples_per_thread * thread_id];

            // Per-thread statistics.
            uint64_t bytes_sent = 0;
            uint64_t time_start_ns;
            uint64_t time_end_ns;
            SchedStats sched_stats;

            // Signal ready and wait for go.
            ready.set_value();
            go_barrier_future.wait();

            // Warmup the scheduler.
            int warmup = ProgramOptions.warmup;
            while (warmup--) {
              for (int i = 0; i < 1000000; i++)
                ;
            }

            sched_stats.Update();
            time_start_ns = GetClockNs();

            int count = ProgramOptions.count;
            while (count--) {
              uint64_t iteration_start_ns = GetClockNs();

              switch (ProgramOptions.opcode) {
                case BenchmarkOps::Nop: {
                  const int ret = client->Nop();
                  if (ret < 0) {
                    std::cerr << "Failed to send nop: " << strerror(-ret)
                              << std::endl;
                    return ret;
                  } else {
                    VLOG(1) << "Success";
                  }
                  break;
                }

                case BenchmarkOps::Read: {
                  const int ret = client->Read(receive_buffer.data(),
                                               ProgramOptions.blocksize);
                  if (ret < 0) {
                    std::cerr << "Failed to read: " << strerror(-ret)
                              << std::endl;
                    return ret;
                  } else if (ret != ProgramOptions.blocksize) {
                    std::cerr << "Expected ret=" << ProgramOptions.blocksize
                              << "; actual ret=" << ret << std::endl;
                    return -EINVAL;
                  } else {
                    VLOG(1) << "Success";
                    bytes_sent += ret;
                  }
                  break;
                }

                case BenchmarkOps::Write: {
                  const int ret =
                      client->Write(send_buffer.data(), send_buffer.size());
                  if (ret < 0) {
                    std::cerr << "Failed to write: " << strerror(-ret)
                              << std::endl;
                    return ret;
                  } else if (ret != ProgramOptions.blocksize) {
                    std::cerr << "Expected ret=" << ProgramOptions.blocksize
                              << "; actual ret=" << ret << std::endl;
                    return -EINVAL;
                  } else {
                    VLOG(1) << "Success";
                    bytes_sent += ret;
                  }
                  break;
                }

                case BenchmarkOps::Echo: {
                  const int ret = client->Echo(
                      send_buffer.data(), send_buffer.size(),
                      receive_buffer.data(), receive_buffer.size());
                  if (ret < 0) {
                    std::cerr << "Failed to echo: " << strerror(-ret)
                              << std::endl;
                    return ret;
                  } else if (ret != ProgramOptions.blocksize) {
                    std::cerr << "Expected ret=" << ProgramOptions.blocksize
                              << "; actual ret=" << ret << std::endl;
                    return -EINVAL;
                  } else {
                    VLOG(1) << "Success";
                    bytes_sent += ret * 2;
                  }
                  break;
                }

                case BenchmarkOps::Stats: {
                  std::tuple<uint64_t, uint64_t, SchedStats> stats;
                  const int ret = client->Stats(&stats);
                  if (ret < 0) {
                    std::cerr << "Failed to get stats: " << strerror(-ret)
                              << std::endl;
                    return ret;
                  } else {
                    VLOG(1) << "Success";
                    std::cerr
                        << "Round trip: receive_time_ns=" << std::get<0>(stats)
                        << " reply_time_ns=" << std::get<1>(stats)
                        << " cpu_time_s=" << std::get<2>(stats).cpu_time_s()
                        << " wait_s=" << std::get<2>(stats).wait_s()
                        << std::endl;
                  }
                  break;
                }

                case BenchmarkOps::WriteVector: {
                  const int ret = client->WriteVector(
                      WrapBuffer(send_buffer.data(), ProgramOptions.blocksize));
                  if (ret < 0) {
                    std::cerr << "Failed to write vector: " << strerror(-ret)
                              << std::endl;
                    return ret;
                  } else {
                    VLOG(1) << "Success";
                    bytes_sent += ret;
                  }
                  break;
                }

                case BenchmarkOps::EchoVector: {
                  thread_local BufferWrapper<std::vector<
                      uint8_t, DefaultInitializationAllocator<uint8_t>>>
                      response_buffer;
                  const int ret = client->EchoVector(
                      WrapBuffer(send_buffer.data(), ProgramOptions.blocksize),
                      &response_buffer);
                  if (ret < 0) {
                    std::cerr << "Failed to echo vector: " << strerror(-ret)
                              << std::endl;
                    return ret;
                  } else {
                    VLOG(1) << "Success";
                    bytes_sent += send_buffer.size() + response_buffer.size();
                  }
                  break;
                }

                case BenchmarkOps::Quit: {
                  const int ret = client->Quit();
                  if (ret < 0 && ret != -ESHUTDOWN) {
                    std::cerr << "Failed to send quit: " << strerror(-ret);
                    return ret;
                  } else {
                    VLOG(1) << "Success";
                  }
                  break;
                }

                default:
                  std::cerr
                      << "Invalid client operation: " << ProgramOptions.opcode
                      << std::endl;
                  return -EINVAL;
              }

              uint64_t iteration_end_ns = GetClockNs();
              uint64_t iteration_delta_ns =
                  iteration_end_ns - iteration_start_ns;
              thread_samples[count] = iteration_delta_ns;

              if (iteration_delta_ns > (kNanosPerSecond / 100)) {
                SchedStats stats = sched_stats;
                stats.Update();
                std::cerr << "Thread " << thread_id << " iteration_delta_s="
                          << (static_cast<double>(iteration_delta_ns) /
                              kNanosPerSecond)
                          << " " << stats.cpu_time_s() << " " << stats.wait_s()
                          << std::endl;
              }
            }

            time_end_ns = GetClockNs();
            sched_stats.Update();

            const double time_delta_s =
                static_cast<double>(time_end_ns - time_start_ns) /
                kNanosPerSecond;

            total_bytes += bytes_sent;
            total_time_ns += time_end_ns - time_start_ns;

            result.set_value(
                {thread_id, service_id, time_delta_s, bytes_sent, sched_stats});
            done_barrier_future.wait();

            return 0;
          },
          thread_id_counter++, service_id_counter, std::move(result_promise),
          std::move(ready_barrier_promise));
    }

    service_id_counter++;
  }

  // Wait for workers to be ready.
  std::cerr << "Waiting for workers to be ready..." << std::endl;
  for (auto& ready : ready_barrier_futures)
    ready.wait();

  // Signal workers to go.
  std::cerr << "Kicking off benchmark." << std::endl;
  go_barrier_promise.set_value();

  // Wait for all the worker threas to finish.
  for (auto& result : client_results)
    result.wait();

  // Report worker thread results.
  for (auto& result : client_results) {
    BenchmarkResult benchmark_result = result.get();
    std::cerr << std::fixed << "Thread " << benchmark_result.thread_id
              << " service " << benchmark_result.service_id << ":" << std::endl;
    std::cerr << "\t " << benchmark_result.bytes_sent << " bytes in "
              << benchmark_result.time_delta_s << " seconds ("
              << std::setprecision(0) << (benchmark_result.bytes_sent / 1024.0 /
                                          benchmark_result.time_delta_s)
              << " K/s; " << std::setprecision(3)
              << (ProgramOptions.count / benchmark_result.time_delta_s)
              << " txn/s; " << std::setprecision(9)
              << (benchmark_result.time_delta_s / ProgramOptions.count)
              << " s/txn)" << std::endl;
    std::cerr << "\tStats: " << benchmark_result.sched_stats.cpu_time_s() << " "
              << (benchmark_result.sched_stats.cpu_time_s() /
                  ProgramOptions.count)
              << " " << benchmark_result.sched_stats.wait_s() << " "
              << (benchmark_result.sched_stats.wait_s() / ProgramOptions.count)
              << " " << benchmark_result.sched_stats.timeslices() << std::endl;
  }

  // Signal worker threads to exit.
  done_barrier_promise.set_value();

  // Wait for the worker threads to exit.
  for (auto& thread : client_threads) {
    thread.join();
  }

  // Report aggregate results.
  const int total_threads = ProgramOptions.threads * ProgramOptions.instances;
  const int iterations = ProgramOptions.count;
  const double total_time_s =
      static_cast<double>(total_time_ns) / kNanosPerSecond;
  // This is about how much wall time it took to completely transfer all the
  // paylaods.
  const double average_time_s = total_time_s / total_threads;

  const uint64_t min_sample_time_ns =
      *std::min_element(latency_samples_ns.begin(), latency_samples_ns.end());
  const double min_sample_time_s =
      static_cast<double>(min_sample_time_ns) / kNanosPerSecond;

  const uint64_t max_sample_time_ns =
      *std::max_element(latency_samples_ns.begin(), latency_samples_ns.end());
  const double max_sample_time_s =
      static_cast<double>(max_sample_time_ns) / kNanosPerSecond;

  const double total_sample_time_s =
      std::accumulate(latency_samples_ns.begin(), latency_samples_ns.end(), 0.0,
                      [](double s, uint64_t ns) {
                        return s + static_cast<double>(ns) / kNanosPerSecond;
                      });
  const double average_sample_time_s =
      total_sample_time_s / latency_samples_ns.size();

  const double sum_of_squared_deviations = std::accumulate(
      latency_samples_ns.begin(), latency_samples_ns.end(), 0.0,
      [&](double s, uint64_t ns) {
        const double delta =
            static_cast<double>(ns) / kNanosPerSecond - average_sample_time_s;
        return s + delta * delta;
      });
  const double variance = sum_of_squared_deviations / latency_samples_ns.size();
  const double standard_deviation = std::sqrt(variance);

  const int num_buckets = 200;
  const uint64_t sample_range_ns = max_sample_time_ns - min_sample_time_ns;
  const uint64_t ns_per_bucket = sample_range_ns / num_buckets;
  std::array<uint64_t, num_buckets> sample_buckets = {{0}};

  // Count samples in each bucket range.
  for (uint64_t sample_ns : latency_samples_ns) {
    sample_buckets[(sample_ns - min_sample_time_ns) / (ns_per_bucket + 1)] += 1;
  }

  // Calculate population percentiles.
  const uint64_t percent_50 =
      static_cast<uint64_t>(latency_samples_ns.size() * 0.5);
  const uint64_t percent_90 =
      static_cast<uint64_t>(latency_samples_ns.size() * 0.9);
  const uint64_t percent_95 =
      static_cast<uint64_t>(latency_samples_ns.size() * 0.95);
  const uint64_t percent_99 =
      static_cast<uint64_t>(latency_samples_ns.size() * 0.99);

  uint64_t sample_count = 0;
  double latency_50th_percentile_s, latency_90th_percentile_s,
      latency_95th_percentile_s, latency_99th_percentile_s;
  for (int i = 0; i < num_buckets; i++) {
    // Report the midpoint of the bucket range as the value of the
    // corresponding
    // percentile.
    const double bucket_midpoint_time_s =
        (ns_per_bucket * i + 0.5 * ns_per_bucket + min_sample_time_ns) /
        kNanosPerSecond;
    if (sample_count < percent_50 &&
        (sample_count + sample_buckets[i]) >= percent_50) {
      latency_50th_percentile_s = bucket_midpoint_time_s;
    }
    if (sample_count < percent_90 &&
        (sample_count + sample_buckets[i]) >= percent_90) {
      latency_90th_percentile_s = bucket_midpoint_time_s;
    }
    if (sample_count < percent_95 &&
        (sample_count + sample_buckets[i]) >= percent_95) {
      latency_95th_percentile_s = bucket_midpoint_time_s;
    }
    if (sample_count < percent_99 &&
        (sample_count + sample_buckets[i]) >= percent_99) {
      latency_99th_percentile_s = bucket_midpoint_time_s;
    }
    sample_count += sample_buckets[i];
  }

  std::cerr << std::fixed << "Total throughput over " << total_threads
            << " threads:\n\t " << total_bytes << " bytes in " << average_time_s
            << " seconds (" << std::setprecision(0)
            << (total_bytes / 1024.0 / average_time_s) << " K/s; "
            << std::setprecision(3)
            << (iterations * total_threads / average_time_s)
            << std::setprecision(9) << " txn/s; "
            << (average_time_s / (iterations * total_threads)) << " s/txn)"
            << std::endl;
  std::cerr << "Sample statistics: " << std::endl;
  std::cerr << total_sample_time_s << " s total sample time" << std::endl;
  std::cerr << average_sample_time_s << " s avg" << std::endl;
  std::cerr << standard_deviation << " s std dev" << std::endl;
  std::cerr << min_sample_time_s << " s min" << std::endl;
  std::cerr << max_sample_time_s << " s max" << std::endl;
  std::cerr << "Latency percentiles:" << std::endl;
  std::cerr << "50th: " << latency_50th_percentile_s << " s" << std::endl;
  std::cerr << "90th: " << latency_90th_percentile_s << " s" << std::endl;
  std::cerr << "95th: " << latency_95th_percentile_s << " s" << std::endl;
  std::cerr << "99th: " << latency_99th_percentile_s << " s" << std::endl;

  std::cout << total_time_ns << " " << std::fixed << std::setprecision(9)
            << average_sample_time_s << " " << std::fixed
            << std::setprecision(9) << standard_deviation << std::endl;
  return 0;
}

int Usage(const std::string& command_name) {
  // clang-format off
  std::cout << "Usage: " << command_name << " [options]" << std::endl;
  std::cout << "\t--verbose                   : Use verbose messages." << std::endl;
  std::cout << "\t--service <endpoint path>   : Start service at the given path." << std::endl;
  std::cout << "\t--client <endpoint path>    : Start client to the given path." << std::endl;
  std::cout << "\t--op <read | write | echo>  : Sepcify client operation mode." << std::endl;
  std::cout << "\t--bs <block size bytes>     : Sepcify block size to use." << std::endl;
  std::cout << "\t--count <count>             : Sepcify number of transactions to make." << std::endl;
  std::cout << "\t--instances <count>         : Specify number of service instances." << std::endl;
  std::cout << "\t--threads <count>           : Sepcify number of threads per instance." << std::endl;
  std::cout << "\t--timeout <timeout ms | -1> : Timeout to wait for services." << std::endl;
  std::cout << "\t--trace                     : Enable systrace logging." << std::endl;
  std::cout << "\t--warmup <iterations>       : Busy loops before running benchmarks." << std::endl;
  // clang-format on
  return -1;
}

}  // anonymous namespace

int main(int argc, char** argv) {
  logging::LoggingSettings logging_settings;
  logging_settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
  logging::InitLogging(logging_settings);

  int getopt_code;
  int option_index;
  std::string option = "";
  std::string command = "";
  std::string command_argument = "";
  bool tracing_enabled = false;

  // Process command line options.
  while ((getopt_code =
              getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
    option = long_options[option_index].name;
    VLOG(1) << "option=" << option;
    switch (getopt_code) {
      case 0:
        if (option == kOptionVerbose) {
          ProgramOptions.verbose = true;
          logging::SetMinLogLevel(-1);
        } else if (option == kOptionOpcode) {
          ParseOpcodeOption(optarg);
        } else if (option == kOptionBlocksize) {
          ProgramOptions.blocksize = std::stoi(optarg);
          if (ProgramOptions.blocksize < 0) {
            std::cerr << "Invalid blocksize argument: "
                      << ProgramOptions.blocksize << std::endl;
            return -EINVAL;
          }
        } else if (option == kOptionCount) {
          ProgramOptions.count = std::stoi(optarg);
          if (ProgramOptions.count < 1) {
            std::cerr << "Invalid count argument: " << ProgramOptions.count
                      << std::endl;
            return -EINVAL;
          }
        } else if (option == kOptionThreads) {
          ProgramOptions.threads = std::stoi(optarg);
          if (ProgramOptions.threads < 1) {
            std::cerr << "Invalid threads argument: " << ProgramOptions.threads
                      << std::endl;
            return -EINVAL;
          }
        } else if (option == kOptionInstances) {
          ProgramOptions.instances = std::stoi(optarg);
          if (ProgramOptions.instances < 1) {
            std::cerr << "Invalid instances argument: "
                      << ProgramOptions.instances << std::endl;
            return -EINVAL;
          }
        } else if (option == kOptionTimeout) {
          ProgramOptions.timeout = std::stoi(optarg);
        } else if (option == kOptionTrace) {
          tracing_enabled = true;
        } else if (option == kOptionWarmup) {
          ProgramOptions.warmup = std::stoi(optarg);
        } else {
          command = option;
          if (optarg)
            command_argument = optarg;
        }
        break;
    }
  }

  // Setup ATRACE/systrace based on command line.
  atrace_setup();
  atrace_set_tracing_enabled(tracing_enabled);

  VLOG(1) << "command=" << command << " command_argument=" << command_argument;

  if (command == "") {
    return Usage(argv[0]);
  } else if (command == kOptionService) {
    return ServiceCommand(command_argument);
  } else if (command == kOptionClient) {
    return ClientCommand(command_argument);
  } else {
    return Usage(argv[0]);
  }
}
