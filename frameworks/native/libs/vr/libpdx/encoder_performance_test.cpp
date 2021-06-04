#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

#include <pdx/rpc/argument_encoder.h>
#include <pdx/rpc/message_buffer.h>
#include <pdx/rpc/payload.h>
#include <pdx/utility.h>

using namespace android::pdx::rpc;
using namespace android::pdx;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;
using std::placeholders::_6;

namespace {

constexpr size_t kMaxStaticBufferSize = 20480;

// Provide numpunct facet that formats numbers with ',' as thousands separators.
class CommaNumPunct : public std::numpunct<char> {
 protected:
  char do_thousands_sep() const override { return ','; }
  std::string do_grouping() const override { return "\03"; }
};

class TestPayload : public MessagePayload<SendBuffer>,
                    public MessageWriter,
                    public MessageReader,
                    public NoOpResourceMapper {
 public:
  // MessageWriter
  void* GetNextWriteBufferSection(size_t size) override {
    const size_t section_offset = Size();
    Extend(size);
    return Data() + section_offset;
  }

  OutputResourceMapper* GetOutputResourceMapper() override { return this; }

  // MessageReader
  BufferSection GetNextReadBufferSection() override {
    return {&*ConstCursor(), &*ConstEnd()};
  }

  void ConsumeReadBufferSectionData(const void* new_start) override {
    std::advance(ConstCursor(), PointerDistance(new_start, &*ConstCursor()));
  }

  InputResourceMapper* GetInputResourceMapper() override { return this; }
};

class StaticBuffer : public MessageWriter,
                     public MessageReader,
                     public NoOpResourceMapper {
 public:
  void Clear() {
    read_ptr_ = buffer_;
    write_ptr_ = 0;
  }
  void Rewind() { read_ptr_ = buffer_; }

  // MessageWriter
  void* GetNextWriteBufferSection(size_t size) override {
    void* ptr = buffer_ + write_ptr_;
    write_ptr_ += size;
    return ptr;
  }

  OutputResourceMapper* GetOutputResourceMapper() override { return this; }

  // MessageReader
  BufferSection GetNextReadBufferSection() override {
    return {read_ptr_, std::end(buffer_)};
  }

  void ConsumeReadBufferSectionData(const void* new_start) override {
    read_ptr_ = static_cast<const uint8_t*>(new_start);
  }

  InputResourceMapper* GetInputResourceMapper() override { return this; }

 private:
  uint8_t buffer_[kMaxStaticBufferSize];
  const uint8_t* read_ptr_{buffer_};
  size_t write_ptr_{0};
};

// Simple callback function to clear/reset the input/output buffers for
// serialization. Using raw function pointer here instead of std::function to
// minimize the overhead of invocation in the tight test loop over millions of
// iterations.
using ResetFunc = void(void*);

// Serialization test function signature, used by the TestRunner.
using SerializeTestSignature = std::chrono::nanoseconds(MessageWriter* writer,
                                                        size_t iterations,
                                                        ResetFunc* write_reset,
                                                        void* reset_data);

// Deserialization test function signature, used by the TestRunner.
using DeserializeTestSignature = std::chrono::nanoseconds(
    MessageReader* reader, MessageWriter* writer, size_t iterations,
    ResetFunc* read_reset, ResetFunc* write_reset, void* reset_data);

// Generic serialization test runner method. Takes the |value| of type T and
// serializes it into the output buffer represented by |writer|.
template <typename T>
std::chrono::nanoseconds SerializeTestRunner(MessageWriter* writer,
                                             size_t iterations,
                                             ResetFunc* write_reset,
                                             void* reset_data, const T& value) {
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < iterations; i++) {
    write_reset(reset_data);
    Serialize(value, writer);
  }
  auto stop = std::chrono::high_resolution_clock::now();
  return stop - start;
}

// Generic deserialization test runner method. Takes the |value| of type T and
// temporarily serializes it into the output buffer, then repeatedly
// deserializes the data back from that buffer.
template <typename T>
std::chrono::nanoseconds DeserializeTestRunner(
    MessageReader* reader, MessageWriter* writer, size_t iterations,
    ResetFunc* read_reset, ResetFunc* write_reset, void* reset_data,
    const T& value) {
  write_reset(reset_data);
  Serialize(value, writer);
  T output_data;
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < iterations; i++) {
    read_reset(reset_data);
    Deserialize(&output_data, reader);
  }
  auto stop = std::chrono::high_resolution_clock::now();
  if (output_data != value)
    return start - stop;  // Return negative value to indicate error.
  return stop - start;
}

// Special version of SerializeTestRunner that doesn't perform any serialization
// but does all the same setup steps and moves data of size |data_size| into
// the output buffer. Useful to determine the baseline to calculate time used
// just for serialization layer.
std::chrono::nanoseconds SerializeBaseTest(MessageWriter* writer,
                                           size_t iterations,
                                           ResetFunc* write_reset,
                                           void* reset_data, size_t data_size) {
  std::vector<uint8_t> dummy_data(data_size);
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < iterations; i++) {
    write_reset(reset_data);
    memcpy(writer->GetNextWriteBufferSection(dummy_data.size()),
           dummy_data.data(), dummy_data.size());
  }
  auto stop = std::chrono::high_resolution_clock::now();
  return stop - start;
}

// Special version of DeserializeTestRunner that doesn't perform any
// deserialization but invokes Rewind on the input buffer repeatedly.
// Useful to determine the baseline to calculate time used just for
// deserialization layer.
std::chrono::nanoseconds DeserializeBaseTest(
    MessageReader* reader, MessageWriter* writer, size_t iterations,
    ResetFunc* read_reset, ResetFunc* write_reset, void* reset_data,
    size_t data_size) {
  std::vector<uint8_t> dummy_data(data_size);
  write_reset(reset_data);
  memcpy(writer->GetNextWriteBufferSection(dummy_data.size()),
         dummy_data.data(), dummy_data.size());
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < iterations; i++) {
    read_reset(reset_data);
    auto section = reader->GetNextReadBufferSection();
    memcpy(dummy_data.data(), section.first, dummy_data.size());
    reader->ConsumeReadBufferSectionData(
        AdvancePointer(section.first, dummy_data.size()));
  }
  auto stop = std::chrono::high_resolution_clock::now();
  return stop - start;
}

// The main class that accumulates individual tests to be executed.
class TestRunner {
 public:
  struct BufferInfo {
    BufferInfo(const std::string& buffer_name, MessageReader* reader,
               MessageWriter* writer, ResetFunc* read_reset_func,
               ResetFunc* write_reset_func, void* reset_data)
        : name{buffer_name},
          reader{reader},
          writer{writer},
          read_reset_func{read_reset_func},
          write_reset_func{write_reset_func},
          reset_data{reset_data} {}
    std::string name;
    MessageReader* reader;
    MessageWriter* writer;
    ResetFunc* read_reset_func;
    ResetFunc* write_reset_func;
    void* reset_data;
  };

  void AddTestFunc(const std::string& name,
                   std::function<SerializeTestSignature> serialize_test,
                   std::function<DeserializeTestSignature> deserialize_test,
                   size_t data_size) {
    tests_.emplace_back(name, std::move(serialize_test),
                        std::move(deserialize_test), data_size);
  }

  template <typename T>
  void AddSerializationTest(const std::string& name, T&& value) {
    const size_t data_size = GetSerializedSize(value);
    auto serialize_test =
        std::bind(static_cast<std::chrono::nanoseconds (*)(
                      MessageWriter*, size_t, ResetFunc*, void*, const T&)>(
                      &SerializeTestRunner),
                  _1, _2, _3, _4, std::forward<T>(value));
    tests_.emplace_back(name, std::move(serialize_test),
                        std::function<DeserializeTestSignature>{}, data_size);
  }

  template <typename T>
  void AddDeserializationTest(const std::string& name, T&& value) {
    const size_t data_size = GetSerializedSize(value);
    auto deserialize_test =
        std::bind(static_cast<std::chrono::nanoseconds (*)(
                      MessageReader*, MessageWriter*, size_t, ResetFunc*,
                      ResetFunc*, void*, const T&)>(&DeserializeTestRunner),
                  _1, _2, _3, _4, _5, _6, std::forward<T>(value));
    tests_.emplace_back(name, std::function<SerializeTestSignature>{},
                        std::move(deserialize_test), data_size);
  }

  template <typename T>
  void AddTest(const std::string& name, T&& value) {
    const size_t data_size = GetSerializedSize(value);
    if (data_size > kMaxStaticBufferSize) {
      std::cerr << "Test '" << name << "' requires " << data_size
                << " bytes in the serialization buffer but only "
                << kMaxStaticBufferSize << " are available." << std::endl;
      exit(1);
    }
    auto serialize_test =
        std::bind(static_cast<std::chrono::nanoseconds (*)(
                      MessageWriter*, size_t, ResetFunc*, void*, const T&)>(
                      &SerializeTestRunner),
                  _1, _2, _3, _4, value);
    auto deserialize_test =
        std::bind(static_cast<std::chrono::nanoseconds (*)(
                      MessageReader*, MessageWriter*, size_t, ResetFunc*,
                      ResetFunc*, void*, const T&)>(&DeserializeTestRunner),
                  _1, _2, _3, _4, _5, _6, std::forward<T>(value));
    tests_.emplace_back(name, std::move(serialize_test),
                        std::move(deserialize_test), data_size);
  }

  std::string CenterString(std::string text, size_t column_width) {
    if (text.size() < column_width) {
      text = std::string((column_width - text.size()) / 2, ' ') + text;
    }
    return text;
  }

  void RunTests(size_t iteration_count,
                const std::vector<BufferInfo>& buffers) {
    using float_seconds = std::chrono::duration<double>;
    const std::string name_column_separator = " : ";
    const std::string buffer_column_separator = " || ";
    const std::string buffer_timing_column_separator = " | ";
    const size_t data_size_column_width = 6;
    const size_t time_column_width = 9;
    const size_t qps_column_width = 18;
    const size_t buffer_column_width = time_column_width +
                                       buffer_timing_column_separator.size() +
                                       qps_column_width;

    auto compare_name_length = [](const TestEntry& t1, const TestEntry& t2) {
      return t1.name.size() < t2.name.size();
    };
    auto test_with_longest_name =
        std::max_element(tests_.begin(), tests_.end(), compare_name_length);
    size_t name_column_width = test_with_longest_name->name.size();

    size_t total_width =
        name_column_width + name_column_separator.size() +
        data_size_column_width + buffer_column_separator.size() +
        buffers.size() * (buffer_column_width + buffer_column_separator.size());

    const std::string dbl_separator(total_width, '=');
    const std::string separator(total_width, '-');

    auto print_header = [&](const std::string& header) {
      std::cout << dbl_separator << std::endl;
      std::stringstream ss;
      ss.imbue(std::locale(ss.getloc(), new CommaNumPunct));
      ss << header << " (" << iteration_count << " iterations)";
      std::cout << CenterString(ss.str(), total_width) << std::endl;
      std::cout << dbl_separator << std::endl;
      std::cout << std::setw(name_column_width) << "Test Name" << std::left
                << name_column_separator << std::setw(data_size_column_width)
                << CenterString("Size", data_size_column_width)
                << buffer_column_separator;
      for (const auto& buffer_info : buffers) {
        std::cout << std::setw(buffer_column_width)
                  << CenterString(buffer_info.name, buffer_column_width)
                  << buffer_column_separator;
      }
      std::cout << std::endl;
      std::cout << std::setw(name_column_width) << "" << name_column_separator
                << std::setw(data_size_column_width)
                << CenterString("bytes", data_size_column_width)
                << buffer_column_separator << std::left;
      for (size_t i = 0; i < buffers.size(); i++) {
        std::cout << std::setw(time_column_width)
                  << CenterString("Time, s", time_column_width)
                  << buffer_timing_column_separator
                  << std::setw(qps_column_width)
                  << CenterString("QPS", qps_column_width)
                  << buffer_column_separator;
      }
      std::cout << std::right << std::endl;
      std::cout << separator << std::endl;
    };

    print_header("Serialization benchmarks");
    for (const auto& test : tests_) {
      if (test.serialize_test) {
        std::cout << std::setw(name_column_width) << test.name << " : "
                  << std::setw(data_size_column_width) << test.data_size
                  << buffer_column_separator;
        for (const auto& buffer_info : buffers) {
          auto seconds =
              std::chrono::duration_cast<float_seconds>(test.serialize_test(
                  buffer_info.writer, iteration_count,
                  buffer_info.write_reset_func, buffer_info.reset_data));
          double qps = iteration_count / seconds.count();
          std::cout << std::fixed << std::setprecision(3)
                    << std::setw(time_column_width) << seconds.count()
                    << buffer_timing_column_separator
                    << std::setw(qps_column_width) << qps
                    << buffer_column_separator;
        }
        std::cout << std::endl;
      }
    }

    print_header("Deserialization benchmarks");
    for (const auto& test : tests_) {
      if (test.deserialize_test) {
        std::cout << std::setw(name_column_width) << test.name << " : "
                  << std::setw(data_size_column_width) << test.data_size
                  << buffer_column_separator;
        for (const auto& buffer_info : buffers) {
          auto seconds =
              std::chrono::duration_cast<float_seconds>(test.deserialize_test(
                  buffer_info.reader, buffer_info.writer, iteration_count,
                  buffer_info.read_reset_func, buffer_info.write_reset_func,
                  buffer_info.reset_data));
          double qps = iteration_count / seconds.count();
          std::cout << std::fixed << std::setprecision(3)
                    << std::setw(time_column_width) << seconds.count()
                    << buffer_timing_column_separator
                    << std::setw(qps_column_width) << qps
                    << buffer_column_separator;
        }
        std::cout << std::endl;
      }
    }
    std::cout << dbl_separator << std::endl;
  }

 private:
  struct TestEntry {
    TestEntry(const std::string& test_name,
              std::function<SerializeTestSignature> serialize_test,
              std::function<DeserializeTestSignature> deserialize_test,
              size_t data_size)
        : name{test_name},
          serialize_test{std::move(serialize_test)},
          deserialize_test{std::move(deserialize_test)},
          data_size{data_size} {}
    std::string name;
    std::function<SerializeTestSignature> serialize_test;
    std::function<DeserializeTestSignature> deserialize_test;
    size_t data_size;
  };

  std::vector<TestEntry> tests_;
};

std::string GenerateContainerName(const std::string& type, size_t count) {
  std::stringstream ss;
  ss << type << "(" << count << ")";
  return ss.str();
}

}  // anonymous namespace

int main(int /*argc*/, char** /*argv*/) {
  const size_t iteration_count = 10000000;  // 10M iterations.
  TestRunner test_runner;
  std::cout.imbue(std::locale(std::cout.getloc(), new CommaNumPunct));

  // Baseline tests to figure out the overhead of buffer resizing and data
  // transfers.
  for (size_t len : {0, 1, 9, 66, 259}) {
    auto serialize_base_test =
        std::bind(&SerializeBaseTest, _1, _2, _3, _4, len);
    auto deserialize_base_test =
        std::bind(&DeserializeBaseTest, _1, _2, _3, _4, _5, _6, len);
    test_runner.AddTestFunc("--Baseline--", std::move(serialize_base_test),
                            std::move(deserialize_base_test), len);
  }

  // Individual serialization/deserialization tests.
  test_runner.AddTest("bool", true);
  test_runner.AddTest("int32_t", 12);

  for (size_t len : {0, 1, 8, 64, 256}) {
    test_runner.AddTest(GenerateContainerName("string", len),
                        std::string(len, '*'));
  }
  // Serialization is too slow to handle such large strings, add this test for
  // deserialization only.
  test_runner.AddDeserializationTest(GenerateContainerName("string", 10240),
                                     std::string(10240, '*'));

  for (size_t len : {0, 1, 8, 64, 256}) {
    std::vector<int32_t> int_vector(len);
    std::iota(int_vector.begin(), int_vector.end(), 0);
    test_runner.AddTest(GenerateContainerName("vector<int32_t>", len),
                        std::move(int_vector));
  }

  std::vector<std::string> vector_of_strings = {
      "012345678901234567890123456789", "012345678901234567890123456789",
      "012345678901234567890123456789", "012345678901234567890123456789",
      "012345678901234567890123456789",
  };
  test_runner.AddTest(
      GenerateContainerName("vector<string>", vector_of_strings.size()),
      std::move(vector_of_strings));

  test_runner.AddTest("tuple<int, bool, string, double>",
                      std::make_tuple(123, true, std::string{"foobar"}, 1.1));

  for (size_t len : {0, 1, 8, 64}) {
    std::map<int, std::string> test_map;
    for (size_t i = 0; i < len; i++)
      test_map.emplace(i, std::to_string(i));
    test_runner.AddTest(GenerateContainerName("map<int, string>", len),
                        std::move(test_map));
  }

  for (size_t len : {0, 1, 8, 64}) {
    std::unordered_map<int, std::string> test_map;
    for (size_t i = 0; i < len; i++)
      test_map.emplace(i, std::to_string(i));
    test_runner.AddTest(
        GenerateContainerName("unordered_map<int, string>", len),
        std::move(test_map));
  }

  // BufferWrapper can't be used with deserialization tests right now because
  // it requires external buffer to be filled in, which is not available.
  std::vector<std::vector<uint8_t>> data_buffers;
  for (size_t len : {0, 1, 8, 64, 256}) {
    data_buffers.emplace_back(len);
    test_runner.AddSerializationTest(
        GenerateContainerName("BufferWrapper<uint8_t*>", len),
        BufferWrapper<uint8_t*>(data_buffers.back().data(),
                                data_buffers.back().size()));
  }

  // Various backing buffers to run the tests on.
  std::vector<TestRunner::BufferInfo> buffers;

  Payload buffer;
  buffers.emplace_back("Non-TLS Buffer", &buffer, &buffer,
                       [](void* ptr) { static_cast<Payload*>(ptr)->Rewind(); },
                       [](void* ptr) { static_cast<Payload*>(ptr)->Clear(); },
                       &buffer);

  TestPayload tls_buffer;
  buffers.emplace_back(
      "TLS Buffer", &tls_buffer, &tls_buffer,
      [](void* ptr) { static_cast<TestPayload*>(ptr)->Rewind(); },
      [](void* ptr) { static_cast<TestPayload*>(ptr)->Clear(); }, &tls_buffer);

  StaticBuffer static_buffer;
  buffers.emplace_back(
      "Static Buffer", &static_buffer, &static_buffer,
      [](void* ptr) { static_cast<StaticBuffer*>(ptr)->Rewind(); },
      [](void* ptr) { static_cast<StaticBuffer*>(ptr)->Clear(); },
      &static_buffer);

  // Finally, run all the tests.
  test_runner.RunTests(iteration_count, buffers);
  return 0;
}
