#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <thread>
#include <utility>

#include <gtest/gtest.h>
#include <pdx/rpc/argument_encoder.h>
#include <pdx/rpc/array_wrapper.h>
#include <pdx/rpc/default_initialization_allocator.h>
#include <pdx/rpc/payload.h>
#include <pdx/rpc/serializable.h>
#include <pdx/rpc/serialization.h>
#include <pdx/rpc/string_wrapper.h>
#include <pdx/utility.h>

using namespace android::pdx;
using namespace android::pdx::rpc;

// Tests the serialization/deserialization of all supported types, verifying all
// reasonable boundary conditions for types with multiple encodings.
//
// NOTE: Sometimes this file uses the construct "var = decltype(var)({...})"
// instead of the equivalent "var = {...}" to construct vectors. This is to
// prevent clang-format from producing annoyingly vertical code from long
// initializers.

// TODO(eieio): Automatically generate some of these tests?

namespace {

// Test data for serialization/deserialization of floats.
const float kZeroFloat = 0.0f;
const float kOneFloat = 1.0f;
const auto kZeroFloatBytes = reinterpret_cast<const std::uint8_t*>(&kZeroFloat);
const auto kOneFloatBytes = reinterpret_cast<const std::uint8_t*>(&kOneFloat);
const double kZeroDouble = 0.0;
const double kOneDouble = 1.0;
const auto kZeroDoubleBytes =
    reinterpret_cast<const std::uint8_t*>(&kZeroDouble);
const auto kOneDoubleBytes = reinterpret_cast<const std::uint8_t*>(&kOneDouble);

struct TestType {
  enum class Foo { kFoo, kBar, kBaz };

  int a;
  float b;
  std::string c;
  Foo d;

  TestType() {}
  TestType(int a, float b, const std::string& c, Foo d)
      : a(a), b(b), c(c), d(d) {}

  // Make gtest expressions simpler by defining equality operator. This is not
  // needed for serialization.
  bool operator==(const TestType& other) const {
    return a == other.a && b == other.b && c == other.c && d == other.d;
  }

 private:
  PDX_SERIALIZABLE_MEMBERS(TestType, a, b, c, d);
};

template <typename FileHandleType>
struct TestTemplateType {
  FileHandleType fd;

  TestTemplateType() {}
  explicit TestTemplateType(FileHandleType fd) : fd(std::move(fd)) {}

  bool operator==(const TestTemplateType& other) const {
    return fd.Get() == other.fd.Get();
  }

 private:
  PDX_SERIALIZABLE_MEMBERS(TestTemplateType<FileHandleType>, fd);
};

// Utilities to generate test maps and payloads.
template <typename MapType>
MapType MakeMap(std::size_t size) {
  MapType result;
  for (std::size_t i = 0; i < size; i++) {
    result.emplace(i, i);
  }
  return result;
}

template <typename MapType>
void InsertKeyValue(MessageWriter* writer, std::size_t size) {
  MapType map;
  for (std::size_t i = 0; i < size; i++) {
    map.emplace(i, i);
  }
  for (const auto& element : map) {
    Serialize(element.first, writer);
    Serialize(element.second, writer);
  }
}

}  // anonymous namespace

TEST(SerializableTypes, Constructor) {
  TestType tt(1, 2.0, "three", TestType::Foo::kBar);
  EXPECT_EQ(1, tt.a);
  EXPECT_EQ(2.0, tt.b);
  EXPECT_EQ("three", tt.c);
  EXPECT_EQ(TestType::Foo::kBar, tt.d);
}

TEST(SerializationTest, bool) {
  Payload result;
  Payload expected;
  bool value;

  // True.
  value = true;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_TRUE};
  EXPECT_EQ(expected, result);
  result.Clear();

  // False.
  value = false;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FALSE};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, uint8_t) {
  Payload result;
  Payload expected;
  uint8_t value;

  // Min FIXINT.
  value = 0;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXINT.
  value = (1 << 7) - 1;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min UINT8.
  value = (1 << 7);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT8, (1 << 7)};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max UINT8.
  value = 0xff;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT8, 0xff};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, uint16_t) {
  Payload result;
  Payload expected;
  uint16_t value;

  // Min FIXINT.
  value = 0;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXINT.
  value = (1 << 7) - 1;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min UINT8.
  value = (1 << 7);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT8, (1 << 7)};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max UINT8.
  value = 0xff;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT8, 0xff};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min UINT16.
  value = (1 << 8);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT16, 0, 1};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max UINT16.
  value = 0xffff;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT16, 0xff, 0xff};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, uint32_t) {
  Payload result;
  Payload expected;
  uint32_t value;

  // Min FIXINT.
  value = 0;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXINT.
  value = (1 << 7) - 1;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min UINT8.
  value = (1 << 7);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT8, (1 << 7)};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max UINT8.
  value = 0xff;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT8, 0xff};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min UINT16.
  value = (1 << 8);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT16, 0, 1};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max UINT16.
  value = 0xffff;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT16, 0xff, 0xff};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min UINT32.
  value = (1 << 16);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT32, 0, 0, 1, 0};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max UINT32.
  value = 0xffffffff;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT32, 0xff, 0xff, 0xff, 0xff};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, uint64_t) {
  Payload result;
  Payload expected;
  uint64_t value;

  // Min FIXINT.
  value = 0;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXINT.
  value = (1 << 7) - 1;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min UINT8.
  value = (1 << 7);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT8, (1 << 7)};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max UINT8.
  value = 0xff;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT8, 0xff};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min UINT16.
  value = (1 << 8);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT16, 0, 1};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max UINT16.
  value = 0xffff;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT16, 0xff, 0xff};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min UINT32.
  value = (1 << 16);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT32, 0, 0, 1, 0};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max UINT32.
  value = 0xffffffff;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT32, 0xff, 0xff, 0xff, 0xff};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min UINT64.
  value = (1ULL << 32);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_UINT64, 0, 0, 0, 0, 1, 0, 0, 0};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max UINT64.
  value = 0xffffffffffffffffULL;
  Serialize(value, &result);
  expected = {
      ENCODING_TYPE_UINT64, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, int8_t) {
  Payload result;
  Payload expected;
  int8_t value;

  // Min NEGATIVE FIXINT.
  value = -32;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_NEGATIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max NEGATIVE FIXINT.
  value = -1;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_NEGATIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min FIXINT.
  value = 0;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXINT.
  value = 127;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min INT8.
  value = -128;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT8, 0x80};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max INT8.
  value = -33;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT8, 0xdf};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, int16_t) {
  Payload result;
  Payload expected;
  int16_t value;

  // Min NEGATIVE FIXINT.
  value = -32;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_NEGATIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max NEGATIVE FIXINT.
  value = -1;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_NEGATIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min FIXINT.
  value = 0;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXINT.
  value = 127;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min INT8.
  value = -128;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT8, 0x80};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max INT8.
  value = -33;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT8, 0xdf};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min INT16.
  value = -32768;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT16, 0x00, 0x80};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max INT16.
  value = 32767;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT16, 0xff, 0x7f};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, int32_t) {
  Payload result;
  Payload expected;
  int32_t value;

  // Min NEGATIVE FIXINT.
  value = -32;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_NEGATIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max NEGATIVE FIXINT.
  value = -1;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_NEGATIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min FIXINT.
  value = 0;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXINT.
  value = 127;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min INT8.
  value = -128;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT8, 0x80};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max INT8.
  value = -33;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT8, 0xdf};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min INT16.
  value = -32768;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT16, 0x00, 0x80};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max INT16.
  value = 32767;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT16, 0xff, 0x7f};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min INT32.
  value = -2147483648;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT32, 0x00, 0x00, 0x00, 0x80};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max INT32.
  value = 2147483647;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT32, 0xff, 0xff, 0xff, 0x7f};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, int64_t) {
  Payload result;
  Payload expected;
  int64_t value;

  // Min NEGATIVE FIXINT.
  value = -32;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_NEGATIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max NEGATIVE FIXINT.
  value = -1;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_NEGATIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min FIXINT.
  value = 0;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXINT.
  value = 127;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min INT8.
  value = -128;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT8, 0x80};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max INT8.
  value = -33;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT8, 0xdf};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min INT16.
  value = -32768;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT16, 0x00, 0x80};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max INT16.
  value = 32767;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT16, 0xff, 0x7f};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min INT32.
  value = -2147483648;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT32, 0x00, 0x00, 0x00, 0x80};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max INT32.
  value = 2147483647;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_INT32, 0xff, 0xff, 0xff, 0x7f};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min INT64.
  value = -9223372036854775808ULL;
  Serialize(value, &result);
  expected = {
      ENCODING_TYPE_INT64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max INT64.
  value = 9223372036854775807ULL;
  Serialize(value, &result);
  expected = {
      ENCODING_TYPE_INT64, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, float) {
  Payload result;
  Payload expected;
  float value;

  value = 0.0f;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FLOAT32, kZeroFloatBytes[0], kZeroFloatBytes[1],
              kZeroFloatBytes[2], kZeroFloatBytes[3]};
  EXPECT_EQ(expected, result);
  result.Clear();

  value = 1.0f;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FLOAT32, kOneFloatBytes[0], kOneFloatBytes[1],
              kOneFloatBytes[2], kOneFloatBytes[3]};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, double) {
  Payload result;
  Payload expected;
  double value;

  value = 0.0f;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FLOAT64, kZeroDoubleBytes[0], kZeroDoubleBytes[1],
              kZeroDoubleBytes[2],   kZeroDoubleBytes[3], kZeroDoubleBytes[4],
              kZeroDoubleBytes[5],   kZeroDoubleBytes[6], kZeroDoubleBytes[7]};
  EXPECT_EQ(expected, result);
  result.Clear();

  value = 1.0f;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FLOAT64, kOneDoubleBytes[0], kOneDoubleBytes[1],
              kOneDoubleBytes[2],    kOneDoubleBytes[3], kOneDoubleBytes[4],
              kOneDoubleBytes[5],    kOneDoubleBytes[6], kOneDoubleBytes[7]};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, Enum) {
  Payload result;
  Payload expected;

  enum Foo { kFoo, kBar, kBaz };
  Foo value = kBar;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MIN + 1};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, EnumClass) {
  Payload result;
  Payload expected;

  enum class Foo { kFoo, kBar, kBaz };
  Foo value = Foo::kBaz;
  Serialize(value, &result);
  expected = {ENCODING_TYPE_POSITIVE_FIXINT_MIN + 2};
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, LocalHandle) {
  Payload result;
  Payload expected;
  LocalHandle fd1;
  LocalHandle fd2;

  fd1 = LocalHandle(100);
  Serialize(fd1, &result);
  expected = {ENCODING_TYPE_FIXEXT2, ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 0, 0};
  EXPECT_EQ(expected, result);
  EXPECT_EQ(1u, result.FdCount());
  EXPECT_EQ(100, result.FdArray()[0]);
  result.Clear();

  fd2 = LocalHandle(200);
  Serialize(std::forward_as_tuple(fd1, fd2), &result);
  expected = decltype(expected)(
      {ENCODING_TYPE_FIXARRAY_MIN + 2, ENCODING_TYPE_FIXEXT2,
       ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 0, 0, ENCODING_TYPE_FIXEXT2,
       ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 1, 0});
  EXPECT_EQ(expected, result);
  EXPECT_EQ(2u, result.FdCount());
  EXPECT_EQ(100, result.FdArray()[0]);
  EXPECT_EQ(200, result.FdArray()[1]);
  result.Clear();

  fd1.Release();  // Don't try to close fd 100.
  fd2.Release();  // Don't try to close fd 200.

  fd1 = LocalHandle(-2);
  Serialize(fd1, &result);
  expected = {ENCODING_TYPE_FIXEXT2, ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 0xfe,
              0xff};
  EXPECT_EQ(expected, result);
  EXPECT_EQ(0u, result.FdCount());
  result.Clear();
}

TEST(SerializationTest, string) {
  Payload result;
  Payload expected;
  std::string value;

  // Min FIXSTR.
  value = "";
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FIXSTR_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXSTR.
  value = std::string((1 << 5) - 1, 'x');
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FIXSTR_MAX};
  expected.Append((1 << 5) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min STR8.
  value = std::string((1 << 5), 'x');
  Serialize(value, &result);
  expected = {ENCODING_TYPE_STR8, (1 << 5)};
  expected.Append((1 << 5), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max STR8.
  value = std::string((1 << 8) - 1, 'x');
  Serialize(value, &result);
  expected = {ENCODING_TYPE_STR8, (1 << 8) - 1};
  expected.Append((1 << 8) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min STR16.
  value = std::string((1 << 8), 'x');
  Serialize(value, &result);
  expected = {ENCODING_TYPE_STR16, 0x00, 0x01};
  expected.Append((1 << 8), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max STR16.
  value = std::string((1 << 16) - 1, 'x');
  Serialize(value, &result);
  expected = {ENCODING_TYPE_STR16, 0xff, 0xff};
  expected.Append((1 << 16) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min STR32.
  value = std::string((1 << 16), 'x');
  Serialize(value, &result);
  expected = {ENCODING_TYPE_STR32, 0x00, 0x00, 0x01, 0x00};
  expected.Append((1 << 16), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, StringWrapper) {
  Payload result;
  Payload expected;
  std::string value;

  // Min FIXSTR.
  value = "";
  Serialize(WrapString(value), &result);
  expected = {ENCODING_TYPE_FIXSTR_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXSTR.
  value = std::string((1 << 5) - 1, 'x');
  Serialize(WrapString(value), &result);
  expected = {ENCODING_TYPE_FIXSTR_MAX};
  expected.Append((1 << 5) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min STR8.
  value = std::string((1 << 5), 'x');
  Serialize(WrapString(value), &result);
  expected = {ENCODING_TYPE_STR8, (1 << 5)};
  expected.Append((1 << 5), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max STR8.
  value = std::string((1 << 8) - 1, 'x');
  Serialize(WrapString(value), &result);
  expected = {ENCODING_TYPE_STR8, (1 << 8) - 1};
  expected.Append((1 << 8) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min STR16.
  value = std::string((1 << 8), 'x');
  Serialize(WrapString(value), &result);
  expected = {ENCODING_TYPE_STR16, 0x00, 0x01};
  expected.Append((1 << 8), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max STR16.
  value = std::string((1 << 16) - 1, 'x');
  Serialize(WrapString(value), &result);
  expected = {ENCODING_TYPE_STR16, 0xff, 0xff};
  expected.Append((1 << 16) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min STR32.
  value = std::string((1 << 16), 'x');
  Serialize(WrapString(value), &result);
  expected = {ENCODING_TYPE_STR32, 0x00, 0x00, 0x01, 0x00};
  expected.Append((1 << 16), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, vector) {
  Payload result;
  Payload expected;
  std::vector<uint8_t> value;

  // Min FIXARRAY.
  value = {};
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FIXARRAY_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXARRAY.
  value = decltype(value)((1 << 4) - 1, 'x');
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FIXARRAY_MAX};
  expected.Append((1 << 4) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min ARRAY16.
  value = decltype(value)((1 << 4), 'x');
  Serialize(value, &result);
  expected = {ENCODING_TYPE_ARRAY16, 0x10, 0x00};
  expected.Append((1 << 4), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max ARRAY16.
  value = decltype(value)((1 << 16) - 1, 'x');
  Serialize(value, &result);
  expected = {ENCODING_TYPE_ARRAY16, 0xff, 0xff};
  expected.Append((1 << 16) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min ARRAY32.
  value = decltype(value)((1 << 16), 'x');
  Serialize(value, &result);
  expected = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x01, 0x00};
  expected.Append((1 << 16), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, map) {
  Payload result;
  Payload expected;
  std::map<std::uint32_t, std::uint32_t> value;

  // Min FIXMAP.
  value = {};
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FIXMAP_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXMAP.
  value = MakeMap<decltype(value)>((1 << 4) - 1);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FIXMAP_MAX};
  InsertKeyValue<decltype(value)>(&expected, (1 << 4) - 1);
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min MAP16.
  value = MakeMap<decltype(value)>((1 << 4));
  Serialize(value, &result);
  expected = {ENCODING_TYPE_MAP16, 0x10, 0x00};
  InsertKeyValue<decltype(value)>(&expected, (1 << 4));
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max MAP16.
  value = MakeMap<decltype(value)>((1 << 16) - 1);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_MAP16, 0xff, 0xff};
  InsertKeyValue<decltype(value)>(&expected, (1 << 16) - 1);
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min MAP32.
  value = MakeMap<decltype(value)>((1 << 16));
  Serialize(value, &result);
  expected = {ENCODING_TYPE_MAP32, 0x00, 0x00, 0x01, 0x00};
  InsertKeyValue<decltype(value)>(&expected, (1 << 16));
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, unordered_map) {
  Payload result;
  Payload expected;
  std::unordered_map<std::uint32_t, std::uint32_t> value;

  // Min FIXMAP.
  value = {};
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FIXMAP_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXMAP.
  value = MakeMap<decltype(value)>((1 << 4) - 1);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_FIXMAP_MAX};
  InsertKeyValue<decltype(value)>(&expected, (1 << 4) - 1);
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min MAP16.
  value = MakeMap<decltype(value)>((1 << 4));
  Serialize(value, &result);
  expected = {ENCODING_TYPE_MAP16, 0x10, 0x00};
  InsertKeyValue<decltype(value)>(&expected, (1 << 4));
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max MAP16.
  value = MakeMap<decltype(value)>((1 << 16) - 1);
  Serialize(value, &result);
  expected = {ENCODING_TYPE_MAP16, 0xff, 0xff};
  InsertKeyValue<decltype(value)>(&expected, (1 << 16) - 1);
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min MAP32.
  value = MakeMap<decltype(value)>((1 << 16));
  Serialize(value, &result);
  expected = {ENCODING_TYPE_MAP32, 0x00, 0x00, 0x01, 0x00};
  InsertKeyValue<decltype(value)>(&expected, (1 << 16));
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, array) {
  Payload result;
  Payload expected;

  // Min FIXARRAY.
  std::array<std::uint8_t, 0> a0;
  Serialize(a0, &result);
  expected = {ENCODING_TYPE_FIXARRAY_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXARRAY.
  std::array<std::uint8_t, (1 << 4) - 1> a1;
  for (auto& element : a1)
    element = 'x';
  Serialize(a1, &result);
  expected = {ENCODING_TYPE_FIXARRAY_MAX};
  expected.Append((1 << 4) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min ARRAY16.
  std::array<std::uint8_t, (1 << 4)> a2;
  for (auto& element : a2)
    element = 'x';
  Serialize(a2, &result);
  expected = {ENCODING_TYPE_ARRAY16, 0x10, 0x00};
  expected.Append((1 << 4), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max ARRAY16.
  std::array<std::uint8_t, (1 << 16) - 1> a3;
  for (auto& element : a3)
    element = 'x';
  Serialize(a3, &result);
  expected = {ENCODING_TYPE_ARRAY16, 0xff, 0xff};
  expected.Append((1 << 16) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min ARRAY32.
  std::array<std::uint8_t, (1 << 16)> a4;
  for (auto& element : a4)
    element = 'x';
  Serialize(a4, &result);
  expected = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x01, 0x00};
  expected.Append((1 << 16), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, ArrayWrapper) {
  Payload result;
  Payload expected;
  std::vector<std::uint8_t, DefaultInitializationAllocator<std::uint8_t>> value;
  ArrayWrapper<std::uint8_t> wrapper;

  // Min FIXARRAY.
  value = {};
  Serialize(wrapper, &result);
  expected = {ENCODING_TYPE_FIXARRAY_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXARRAY.
  value = decltype(value)((1 << 4) - 1, 'x');
  wrapper = decltype(wrapper)(value.data(), value.capacity(), value.size());
  Serialize(wrapper, &result);
  expected = {ENCODING_TYPE_FIXARRAY_MAX};
  expected.Append((1 << 4) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min ARRAY16.
  value = decltype(value)((1 << 4), 'x');
  wrapper = decltype(wrapper)(value.data(), value.capacity(), value.size());
  Serialize(wrapper, &result);
  expected = {ENCODING_TYPE_ARRAY16, 0x10, 0x00};
  expected.Append((1 << 4), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max ARRAY16.
  value = decltype(value)((1 << 16) - 1, 'x');
  wrapper = decltype(wrapper)(value.data(), value.capacity(), value.size());
  Serialize(wrapper, &result);
  expected = {ENCODING_TYPE_ARRAY16, 0xff, 0xff};
  expected.Append((1 << 16) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min ARRAY32.
  value = decltype(value)((1 << 16), 'x');
  wrapper = decltype(wrapper)(value.data(), value.capacity(), value.size());
  Serialize(wrapper, &result);
  expected = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x01, 0x00};
  expected.Append((1 << 16), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, pair) {
  Payload result;
  Payload expected;

  auto p1 = std::make_pair(1, 2);
  Serialize(p1, &result);
  expected = {ENCODING_TYPE_FIXARRAY_MIN + 2, 1, 2};
  EXPECT_EQ(expected, result);
  result.Clear();

  auto p2 = std::make_pair('x', std::string("12345"));
  Serialize(p2, &result);
  expected = decltype(expected)({ENCODING_TYPE_FIXARRAY_MIN + 2, 'x',
                                 ENCODING_TYPE_FIXSTR_MIN + 5, '1', '2', '3',
                                 '4', '5'});
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, tuple) {
  Payload result;
  Payload expected;

  // Min FIXARRAY.
  auto t1 = std::make_tuple();
  Serialize(t1, &result);
  expected = {ENCODING_TYPE_FIXARRAY_MIN};
  EXPECT_EQ(expected, result);
  result.Clear();

  // Max FIXARRAY.
  auto t2 = GetNTuple<15>('x');
  Serialize(t2, &result);
  expected = {ENCODING_TYPE_FIXARRAY_MAX};
  expected.Append((1 << 4) - 1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min ARRAY16.
  auto t3 = GetNTuple<(1 << 4)>('x');
  Serialize(t3, &result);
  expected = {ENCODING_TYPE_ARRAY16, 0x10, 0x00};
  expected.Append((1 << 4), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

// Template instantiation depth is an issue for these tests. They are commented
// out to document the expected behavior, even though tuples of this order are
// not expected in practice.
#if 0
  // Max ARRAY16.
  auto t4 = GetNTuple<(1 << 16)-1>('x');
  Serialize(t4, &result);
  expected = {ENCODING_TYPE_ARRAY16, 0xff, 0xff};
  expected.Append((1 << 16)-1, 'x');
  EXPECT_EQ(expected, result);
  result.Clear();

  // Min ARRAY32.
  auto t5 = GetNTuple<(1 << 16)>('x');
  Serialize(t5, &result);
  expected = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x01, 0x00};
  expected.Append((1 << 16), 'x');
  EXPECT_EQ(expected, result);
  result.Clear();
#endif
}

// TODO(eieio): More exhaustive testing of type nesting.
TEST(SerializationTest, NestedTuple) {
  Payload result;
  Payload expected;

  auto t1 = std::make_tuple('x', std::make_tuple<int, int>(1, 2));
  Serialize(t1, &result);
  expected = decltype(expected)({ENCODING_TYPE_FIXARRAY_MIN + 2, 'x',
                                 ENCODING_TYPE_FIXARRAY_MIN + 2, 1, 2});
  EXPECT_EQ(expected, result);
  result.Clear();

  auto t2 = std::make_tuple('x', std::make_tuple<int, int>(1, 2),
                            std::string("0123456789"));
  Serialize(t2, &result);
  expected = decltype(expected)({ENCODING_TYPE_FIXARRAY_MIN + 3, 'x',
                                 ENCODING_TYPE_FIXARRAY_MIN + 2, 1, 2,
                                 ENCODING_TYPE_FIXSTR | 10, '0', '1', '2', '3',
                                 '4', '5', '6', '7', '8', '9'});
  EXPECT_EQ(expected, result);
  result.Clear();

  auto t3 = std::make_tuple(0.0f, std::uint64_t(10ULL),
                            std::vector<char>{'a', 'b', 'c'});
  Serialize(t3, &result);
  expected = decltype(expected)(
      {ENCODING_TYPE_FIXARRAY_MIN + 3, ENCODING_TYPE_FLOAT32,
       kZeroFloatBytes[0], kZeroFloatBytes[1], kZeroFloatBytes[2],
       kZeroFloatBytes[3], ENCODING_TYPE_POSITIVE_FIXINT_MIN + 10,
       ENCODING_TYPE_FIXARRAY_MIN + 3, 'a', 'b', 'c'});
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, NestedMap) {
  Payload result;
  Payload expected;

  std::map<int, std::pair<std::string, int>> m1 = {{0, {"a", 2}},
                                                   {1, {"b", 10}}};
  Serialize(m1, &result);
  expected = decltype(expected)(
      {ENCODING_TYPE_FIXMAP_MIN + 2, 0, ENCODING_TYPE_FIXARRAY_MIN + 2,
       ENCODING_TYPE_FIXSTR_MIN + 1, 'a', 2, 1, ENCODING_TYPE_FIXARRAY_MIN + 2,
       ENCODING_TYPE_FIXSTR_MIN + 1, 'b', 10});
  EXPECT_EQ(expected, result);
  result.Clear();
}

TEST(SerializationTest, Serializable) {
  Payload result;
  Payload expected;

  TestType t1{10, 0.0, "12345", TestType::Foo::kBaz};
  Serialize(t1, &result);
  expected = decltype(expected)(
      {ENCODING_TYPE_FIXARRAY_MIN + 4, 10, ENCODING_TYPE_FLOAT32,
       kZeroFloatBytes[0], kZeroFloatBytes[1], kZeroFloatBytes[2],
       kZeroFloatBytes[3], ENCODING_TYPE_FIXSTR_MIN + 5, '1', '2', '3', '4',
       '5', ENCODING_TYPE_POSITIVE_FIXINT_MIN + 2});
  EXPECT_EQ(expected, result);
  result.Clear();

  TestTemplateType<LocalHandle> tt{LocalHandle(-1)};
  Serialize(tt, &result);
  expected =
      decltype(expected)({ENCODING_TYPE_FIXARRAY_MIN + 1, ENCODING_TYPE_FIXEXT2,
                          ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 0xff, 0xff});
  EXPECT_EQ(expected, result);
}

TEST(SerializationTest, Variant) {
  Payload result;
  Payload expected;

  Variant<int, bool, float> v;

  // Empty variant.
  Serialize(v, &result);
  expected = {ENCODING_TYPE_FIXMAP_MIN + 1, ENCODING_TYPE_NEGATIVE_FIXINT_MAX,
              ENCODING_TYPE_NIL};
  EXPECT_EQ(expected, result);
  result.Clear();

  v = 10;
  Serialize(v, &result);
  expected = {ENCODING_TYPE_FIXMAP_MIN + 1,
              ENCODING_TYPE_POSITIVE_FIXINT_MIN + 0,
              ENCODING_TYPE_POSITIVE_FIXINT_MIN + 10};
  EXPECT_EQ(expected, result);
  result.Clear();

  v = true;
  Serialize(v, &result);
  expected = {ENCODING_TYPE_FIXMAP_MIN + 1,
              ENCODING_TYPE_POSITIVE_FIXINT_MIN + 1, ENCODING_TYPE_TRUE};
  EXPECT_EQ(expected, result);
  result.Clear();

  v = false;
  Serialize(v, &result);
  expected = {ENCODING_TYPE_FIXMAP_MIN + 1,
              ENCODING_TYPE_POSITIVE_FIXINT_MIN + 1, ENCODING_TYPE_FALSE};
  EXPECT_EQ(expected, result);
  result.Clear();

  v = 1.0f;
  Serialize(v, &result);
  expected = {ENCODING_TYPE_FIXMAP_MIN + 1,
              ENCODING_TYPE_POSITIVE_FIXINT_MIN + 2,
              ENCODING_TYPE_FLOAT32,
              kOneFloatBytes[0],
              kOneFloatBytes[1],
              kOneFloatBytes[2],
              kOneFloatBytes[3]};
  EXPECT_EQ(expected, result);
  result.Clear();

  // TODO(eieio): Add more serialization tests for Variant.
}

TEST(DeserializationTest, bool) {
  Payload buffer;
  bool result = false;
  ErrorType error;

  // True.
  buffer = {ENCODING_TYPE_TRUE};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(1, result);  // Gtest generates warning from bool literals.

  // False.
  buffer = {ENCODING_TYPE_FALSE};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0, result);  // Gtest generates warning from bool literals.
}

TEST(DeserializationTest, uint8_t) {
  Payload buffer;
  std::uint8_t result = 0;
  ErrorType error;

  // Min FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127U, result);

  // Min UINT8.
  buffer = {ENCODING_TYPE_UINT8, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max UINT8.
  buffer = {ENCODING_TYPE_UINT8, 0xff};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0xffU, result);

  // UINT16 out of range.
  buffer = {ENCODING_TYPE_UINT16};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_UINT16, error.encoding_type());

  // UINT32 out of range.
  buffer = {ENCODING_TYPE_UINT32};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_UINT32, error.encoding_type());

  // UINT64 out of range.
  buffer = {ENCODING_TYPE_UINT64};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_UINT64, error.encoding_type());
}

TEST(DeserializationTest, uint16_t) {
  Payload buffer;
  std::uint16_t result = 0;
  ErrorType error;

  // Min FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127U, result);

  // Min UINT8.
  buffer = {ENCODING_TYPE_UINT8, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max UINT8.
  buffer = {ENCODING_TYPE_UINT8, 0xff};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0xffU, result);

  // Min UINT16.
  buffer = {ENCODING_TYPE_UINT16, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max UINT16.
  buffer = {ENCODING_TYPE_UINT16, 0xff, 0xff};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0xffffU, result);

  // UINT32 out of range.
  buffer = {ENCODING_TYPE_UINT32};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_UINT32, error.encoding_type());

  // UINT64 out of range.
  buffer = {ENCODING_TYPE_UINT64};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_UINT64, error.encoding_type());
}

TEST(DeserializationTest, uint32_t) {
  Payload buffer;
  std::uint32_t result = 0;
  ErrorType error;

  // Min FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127U, result);

  // Min UINT8.
  buffer = {ENCODING_TYPE_UINT8, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max UINT8.
  buffer = {ENCODING_TYPE_UINT8, 0xff};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0xffU, result);

  // Min UINT16.
  buffer = {ENCODING_TYPE_UINT16, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max UINT16.
  buffer = {ENCODING_TYPE_UINT16, 0xff, 0xff};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0xffffU, result);

  // Min UINT32.
  buffer = {ENCODING_TYPE_UINT32, 0x00, 0x00, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max UINT32.
  buffer = {ENCODING_TYPE_UINT32, 0xff, 0xff, 0xff, 0xff};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0xffffffffU, result);

  // UINT64 out of range.
  buffer = {ENCODING_TYPE_UINT64};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_UINT64, error.encoding_type());
}

TEST(DeserializationTest, uint64_t) {
  Payload buffer;
  std::uint64_t result = 0;
  ErrorType error;

  // Min FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127U, result);

  // Min UINT8.
  buffer = {ENCODING_TYPE_UINT8, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max UINT8.
  buffer = {ENCODING_TYPE_UINT8, 0xff};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0xffU, result);

  // Min UINT16.
  buffer = {ENCODING_TYPE_UINT16, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max UINT16.
  buffer = {ENCODING_TYPE_UINT16, 0xff, 0xff};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0xffffU, result);

  // Min UINT32.
  buffer = {ENCODING_TYPE_UINT32, 0x00, 0x00, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max UINT32.
  buffer = {ENCODING_TYPE_UINT32, 0xff, 0xff, 0xff, 0xff};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0xffffffffU, result);

  // Min UINT64.
  buffer = {
      ENCODING_TYPE_UINT64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0U, result);

  // Max UINT64.
  buffer = {
      ENCODING_TYPE_UINT64, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0xffffffffffffffffUL, result);
}

TEST(DeserializationTest, int8_t) {
  Payload buffer;
  std::int8_t result = 0;
  ErrorType error;

  // Min NEGATIVE FIXINT.
  buffer = {ENCODING_TYPE_NEGATIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-32, result);

  // Max NEGATIVE FIXINT.
  buffer = {ENCODING_TYPE_NEGATIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-1, result);

  // Min FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0, result);

  // Max FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127, result);

  // Min INT8.
  buffer = {ENCODING_TYPE_INT8, 0x80};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-128, result);

  // Max INT8.
  buffer = {ENCODING_TYPE_INT8, 0x7f};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127, result);

  // INT16 out of range.
  buffer = {ENCODING_TYPE_INT16};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_INT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_INT16, error.encoding_type());

  // INT32 out of range.
  buffer = {ENCODING_TYPE_INT32};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_INT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_INT32, error.encoding_type());

  // INT64 out of range.
  buffer = {ENCODING_TYPE_INT64};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_INT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_INT64, error.encoding_type());
}

TEST(DeserializationTest, int16_t) {
  Payload buffer;
  std::int16_t result = 0;
  ErrorType error;

  // Min NEGATIVE FIXINT.
  buffer = {ENCODING_TYPE_NEGATIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-32, result);

  // Max NEGATIVE FIXINT.
  buffer = {ENCODING_TYPE_NEGATIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-1, result);

  // Min FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0, result);

  // Max FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127, result);

  // Min INT8.
  buffer = {ENCODING_TYPE_INT8, 0x80};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-128, result);

  // Max INT8.
  buffer = {ENCODING_TYPE_INT8, 0x7f};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127, result);

  // Min INT16.
  buffer = {ENCODING_TYPE_INT16, 0x00, 0x80};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-32768, result);

  // Max INT16.
  buffer = {ENCODING_TYPE_INT16, 0xff, 0x7f};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(32767, result);

  // INT32 out of range.
  buffer = {ENCODING_TYPE_INT32};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_INT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_INT32, error.encoding_type());

  // INT64 out of range.
  buffer = {ENCODING_TYPE_INT64};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_INT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_INT64, error.encoding_type());
}

TEST(DeserializationTest, int32_t) {
  Payload buffer;
  std::int32_t result = 0;
  ErrorType error;

  // Min NEGATIVE FIXINT.
  buffer = {ENCODING_TYPE_NEGATIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-32, result);

  // Max NEGATIVE FIXINT.
  buffer = {ENCODING_TYPE_NEGATIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-1, result);

  // Min FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0, result);

  // Max FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127, result);

  // Min INT8.
  buffer = {ENCODING_TYPE_INT8, 0x80};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-128, result);

  // Max INT8.
  buffer = {ENCODING_TYPE_INT8, 0x7f};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127, result);

  // Min INT16.
  buffer = {ENCODING_TYPE_INT16, 0x00, 0x80};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-32768, result);

  // Max INT16.
  buffer = {ENCODING_TYPE_INT16, 0xff, 0x7f};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(32767, result);

  // Min INT32.
  buffer = {ENCODING_TYPE_INT32, 0x00, 0x00, 0x00, 0x80};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-2147483648, result);

  // Max INT32.
  buffer = {ENCODING_TYPE_INT32, 0xff, 0xff, 0xff, 0x7f};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(2147483647, result);

  // INT64 out of range.
  buffer = {ENCODING_TYPE_INT64};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_INT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_INT64, error.encoding_type());
}

TEST(DeserializationTest, int64_t) {
  Payload buffer;
  std::int64_t result = 0;
  ErrorType error;

  // Min NEGATIVE FIXINT.
  buffer = {ENCODING_TYPE_NEGATIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-32, result);

  // Max NEGATIVE FIXINT.
  buffer = {ENCODING_TYPE_NEGATIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-1, result);

  // Min FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0, result);

  // Max FIXINT.
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MAX};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127, result);

  // Min INT8.
  buffer = {ENCODING_TYPE_INT8, 0x80};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-128, result);

  // Max INT8.
  buffer = {ENCODING_TYPE_INT8, 0x7f};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(127, result);

  // Min INT16.
  buffer = {ENCODING_TYPE_INT16, 0x00, 0x80};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-32768, result);

  // Max INT16.
  buffer = {ENCODING_TYPE_INT16, 0xff, 0x7f};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(32767, result);

  // Min INT32.
  buffer = {ENCODING_TYPE_INT32, 0x00, 0x00, 0x00, 0x80};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-2147483648, result);

  // Max INT32.
  buffer = {ENCODING_TYPE_INT32, 0xff, 0xff, 0xff, 0x7f};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(2147483647, result);

  // Min INT64.
  buffer = {
      ENCODING_TYPE_INT64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  // Believe it or not, this is actually the correct way to specify the most
  // negative signed long long.
  EXPECT_EQ(-9223372036854775807LL - 1, result);

  // Max INT64.
  buffer = {
      ENCODING_TYPE_INT64, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(9223372036854775807LL, result);
}

TEST(DeserializationTest, float) {
  Payload buffer;
  float result;
  ErrorType error;

  // FLOAT32.
  buffer = {ENCODING_TYPE_FLOAT32, kZeroFloatBytes[0], kZeroFloatBytes[1],
            kZeroFloatBytes[2], kZeroFloatBytes[3]};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(kZeroFloat, result);

  // FLOAT32.
  buffer = {ENCODING_TYPE_FLOAT32, kOneFloatBytes[0], kOneFloatBytes[1],
            kOneFloatBytes[2], kOneFloatBytes[3]};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(kOneFloat, result);
}

TEST(DeserializationTest, double) {
  Payload buffer;
  double result;
  ErrorType error;

  // FLOAT32.
  buffer = {ENCODING_TYPE_FLOAT32, kZeroFloatBytes[0], kZeroFloatBytes[1],
            kZeroFloatBytes[2], kZeroFloatBytes[3]};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(kZeroDouble, result);

  // FLOAT64.
  buffer = {ENCODING_TYPE_FLOAT64, kZeroDoubleBytes[0], kZeroDoubleBytes[1],
            kZeroDoubleBytes[2],   kZeroDoubleBytes[3], kZeroDoubleBytes[4],
            kZeroDoubleBytes[5],   kZeroDoubleBytes[6], kZeroDoubleBytes[7]};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(kZeroDouble, result);

  // FLOAT32.
  buffer = {ENCODING_TYPE_FLOAT32, kOneFloatBytes[0], kOneFloatBytes[1],
            kOneFloatBytes[2], kOneFloatBytes[3]};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(kOneDouble, result);

  // FLOAT64.
  buffer = {ENCODING_TYPE_FLOAT64, kOneDoubleBytes[0], kOneDoubleBytes[1],
            kOneDoubleBytes[2],    kOneDoubleBytes[3], kOneDoubleBytes[4],
            kOneDoubleBytes[5],    kOneDoubleBytes[6], kOneDoubleBytes[7]};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(kOneDouble, result);
}

TEST(DeserializationTest, Enum) {
  Payload buffer;
  enum Foo { kFoo, kBar, kBaz } result;
  ErrorType error;

  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MIN + 1};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(kBar, result);
}

TEST(DeserializationTest, EnumClass) {
  Payload buffer;
  enum Foo { kFoo, kBar, kBaz } result;
  ErrorType error;

  buffer = {ENCODING_TYPE_POSITIVE_FIXINT_MIN + 2};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(Foo::kBaz, result);
}

TEST(DeserializationTest, LocalHandle) {
  Payload buffer;
  LocalHandle result1;
  LocalHandle result2;
  ErrorType error;

  buffer = {ENCODING_TYPE_FIXEXT2, ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 0, 0};
  error = Deserialize(&result1, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0, result1.Get());
  result1.Release();  // Don't close fd 0.

  std::tuple<LocalHandle&, LocalHandle&> t1(result1, result2);
  buffer = decltype(buffer)(
      {ENCODING_TYPE_FIXARRAY_MIN + 2, ENCODING_TYPE_FIXEXT2,
       ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 0, 0, ENCODING_TYPE_FIXEXT2,
       ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 1, 0});
  error = Deserialize(&t1, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(0, result1.Get());
  EXPECT_EQ(1, result2.Get());
  result1.Release();  // Don't close fd 0.
  result2.Release();  // Don't close fd 1.

  buffer = {ENCODING_TYPE_FIXEXT2, ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 0xfe,
            0xff};
  error = Deserialize(&result1, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(-2, result1.Get());
}

TEST(DeserializationTest, string) {
  Payload buffer;
  std::string result = "";
  ErrorType error;

  // Min FIXSTR.
  buffer = {ENCODING_TYPE_FIXSTR_MIN};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ("", result);

  // Max FIXSTR.
  buffer = {ENCODING_TYPE_FIXSTR_MAX};
  buffer.Append((1 << 5) - 1, 'x');
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(std::string((1 << 5) - 1, 'x'), result);

  // Min STR8.
  buffer = {ENCODING_TYPE_STR8, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ("", result);

  // Max STR8.
  buffer = {ENCODING_TYPE_STR8, 0xff};
  buffer.Append(0xff, 'x');
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(std::string(0xff, 'x'), result);

  // Min STR16.
  buffer = {ENCODING_TYPE_STR16, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ("", result);

  // Max STR16.
  buffer = {ENCODING_TYPE_STR16, 0xff, 0xff};
  buffer.Append(0xffff, 'x');
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(std::string(0xffff, 'x'), result);

  // Min STR32.
  buffer = {ENCODING_TYPE_STR32, 0x00, 0x00, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ("", result);

  // Test STR32 with max STR16 + 1 bytes. It's not practical to test max
  // STR32.
  buffer = {ENCODING_TYPE_STR32, 0x00, 0x00, 0x01, 0x00};
  buffer.Append(0x10000, 'x');
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(std::string(0x10000, 'x'), result);
}

TEST(DeserializationTest, vector) {
  Payload buffer;
  std::vector<std::uint8_t, DefaultInitializationAllocator<std::uint8_t>>
      result;
  Payload expected;
  ErrorType error;

  // Min FIXARRAY.
  buffer = {ENCODING_TYPE_FIXARRAY_MIN};
  error = Deserialize(&result, &buffer);
  expected = {};
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Max FIXARRAY.
  buffer = {ENCODING_TYPE_FIXARRAY_MAX};
  buffer.Append((1 << 4) - 1, 1);
  error = Deserialize(&result, &buffer);
  expected = decltype(expected)((1 << 4) - 1, 1);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Min ARRAY16.
  buffer = {ENCODING_TYPE_ARRAY16, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  expected = {};
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Max ARRAY16.
  buffer = {ENCODING_TYPE_ARRAY16, 0xff, 0xff};
  buffer.Append(0xffff, 1);
  error = Deserialize(&result, &buffer);
  expected = decltype(expected)(0xffff, 1);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Min ARRAY32.
  buffer = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  expected = {};
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // ARRAY32 with max ARRAY16 + 1. It's not practical to test max ARRAY32.
  buffer = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x01, 0x00};
  buffer.Append(0x10000, 1);
  error = Deserialize(&result, &buffer);
  expected = decltype(expected)(0x10000, 1);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);
}

TEST(DeserializationTest, map) {
  Payload buffer;
  std::map<std::uint32_t, std::uint32_t> result;
  std::map<std::uint32_t, std::uint32_t> expected;
  ErrorType error;

  // Min FIXMAP.
  buffer = {ENCODING_TYPE_FIXMAP_MIN};
  error = Deserialize(&result, &buffer);
  expected = {};
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Size mismatch.
  buffer = {ENCODING_TYPE_FIXMAP_MIN + 1};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::INSUFFICIENT_BUFFER, error);

  // Max FIXMAP.
  buffer = {ENCODING_TYPE_FIXMAP_MAX};
  InsertKeyValue<decltype(result)>(&buffer, (1 << 4) - 1);
  error = Deserialize(&result, &buffer);
  expected = MakeMap<decltype(expected)>((1 << 4) - 1);
  EXPECT_EQ(ErrorCode::NO_ERROR, error) << std::string(error);
  EXPECT_EQ(expected, result);

  // Min MAP16.
  buffer = {ENCODING_TYPE_MAP16, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  expected = {};
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Max MAP16.
  buffer = {ENCODING_TYPE_MAP16, 0xff, 0xff};
  InsertKeyValue<decltype(result)>(&buffer, (1 << 16) - 1);
  error = Deserialize(&result, &buffer);
  expected = MakeMap<decltype(expected)>((1 << 16) - 1);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Min MAP32.
  buffer = {ENCODING_TYPE_MAP32, 0x00, 0x00, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  expected = {};
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // MAP32 with max MAP16 + 1. It's not practical to test max MAP32.
  buffer = {ENCODING_TYPE_MAP32, 0x00, 0x00, 0x01, 0x00};
  InsertKeyValue<decltype(result)>(&buffer, (1 << 16));
  error = Deserialize(&result, &buffer);
  expected = MakeMap<decltype(expected)>((1 << 16));
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);
}

TEST(DeserializationTest, unordered_map) {
  Payload buffer;
  std::unordered_map<std::uint32_t, std::uint32_t> result;
  std::unordered_map<std::uint32_t, std::uint32_t> expected;
  ErrorType error;

  // Min FIXMAP.
  buffer = {ENCODING_TYPE_FIXMAP_MIN};
  error = Deserialize(&result, &buffer);
  expected = {};
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Size mismatch.
  buffer = {ENCODING_TYPE_FIXMAP_MIN + 1};
  error = Deserialize(&result, &buffer);
  EXPECT_EQ(ErrorCode::INSUFFICIENT_BUFFER, error);

  // Max FIXMAP.
  buffer = {ENCODING_TYPE_FIXMAP_MAX};
  InsertKeyValue<decltype(result)>(&buffer, (1 << 4) - 1);
  error = Deserialize(&result, &buffer);
  expected = MakeMap<decltype(expected)>((1 << 4) - 1);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Min MAP16.
  buffer = {ENCODING_TYPE_MAP16, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  expected = {};
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Max MAP16.
  buffer = {ENCODING_TYPE_MAP16, 0xff, 0xff};
  InsertKeyValue<decltype(result)>(&buffer, (1 << 16) - 1);
  error = Deserialize(&result, &buffer);
  expected = MakeMap<decltype(expected)>((1 << 16) - 1);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Min MAP32.
  buffer = {ENCODING_TYPE_MAP32, 0x00, 0x00, 0x00, 0x00};
  error = Deserialize(&result, &buffer);
  expected = {};
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // MAP32 with max MAP16 + 1. It's not practical to test max MAP32.
  buffer = {ENCODING_TYPE_MAP32, 0x00, 0x00, 0x01, 0x00};
  InsertKeyValue<decltype(result)>(&buffer, (1 << 16));
  error = Deserialize(&result, &buffer);
  expected = MakeMap<decltype(expected)>((1 << 16));
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);
}

TEST(DeserializationTest, array) {
  Payload buffer;
  ErrorType error;

  // Min FIXARRAY.
  buffer = {ENCODING_TYPE_FIXARRAY_MIN};
  std::array<std::uint8_t, 0> a0;
  error = Deserialize(&a0, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);

  // Size mismatch.
  buffer = {ENCODING_TYPE_FIXARRAY_MIN + 1};
  error = Deserialize(&a0, &buffer);
  EXPECT_EQ(ErrorCode::INSUFFICIENT_DESTINATION_SIZE, error);

  // Max FIXARRAY.
  buffer = {ENCODING_TYPE_FIXARRAY_MAX};
  buffer.Append((1 << 4) - 1, 'x');
  std::array<std::uint8_t, (1 << 4) - 1> a1, expected1;
  for (auto& element : expected1)
    element = 'x';
  error = Deserialize(&a1, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected1, a1);

  // Min ARRAY16.
  buffer = {ENCODING_TYPE_ARRAY16, 0x00, 0x00};
  error = Deserialize(&a0, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);

  // Max ARRAY16.
  buffer = {ENCODING_TYPE_ARRAY16, 0xff, 0xff};
  buffer.Append((1 << 16) - 1, 'x');
  std::array<std::uint8_t, (1 << 16) - 1> a3, expected3;
  for (auto& element : expected3)
    element = 'x';
  error = Deserialize(&a3, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected3, a3);

  // Min ARRAY32.
  buffer = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x00, 0x00};
  error = Deserialize(&a0, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);

  // ARRAY32 with max ARRAY16 + 1. It's not practical to test max ARRAY32.
  buffer = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x01, 0x00};
  buffer.Append((1 << 16), 'x');
  std::array<std::uint8_t, (1 << 16)> a4, expected4;
  for (auto& element : expected4)
    element = 'x';
  error = Deserialize(&a4, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected4, a4);
}

TEST(DeserializationTest, ArrayWrapper) {
  Payload buffer;
  std::vector<std::uint8_t, DefaultInitializationAllocator<std::uint8_t>>
      result;
  std::vector<std::uint8_t, DefaultInitializationAllocator<std::uint8_t>>
      expected;
  ErrorType error;

  result.reserve(0x10000);
  ArrayWrapper<std::uint8_t> wrapper(result.data(), result.capacity());

  // Min FIXARRAY.
  buffer = {ENCODING_TYPE_FIXARRAY_MIN};
  error = Deserialize(&wrapper, &buffer);
  expected = {};
  result.resize(wrapper.size());
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Max FIXARRAY.
  buffer = {ENCODING_TYPE_FIXARRAY_MAX};
  buffer.Append((1 << 4) - 1, 1);
  error = Deserialize(&wrapper, &buffer);
  expected = decltype(expected)((1 << 4) - 1, 1);
  result.resize(wrapper.size());
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Min ARRAY16.
  buffer = {ENCODING_TYPE_ARRAY16, 0x00, 0x00};
  error = Deserialize(&wrapper, &buffer);
  expected = {};
  result.resize(wrapper.size());
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Max ARRAY16.
  buffer = {ENCODING_TYPE_ARRAY16, 0xff, 0xff};
  buffer.Append(0xffff, 1);
  error = Deserialize(&wrapper, &buffer);
  expected = decltype(expected)(0xffff, 1);
  result.resize(wrapper.size());
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // Min ARRAY32.
  buffer = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x00, 0x00};
  error = Deserialize(&wrapper, &buffer);
  expected = {};
  result.resize(wrapper.size());
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);

  // ARRAY32 with max ARRAY16 + 1. It's not practical to test max ARRAY32.
  buffer = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x01, 0x00};
  buffer.Append(0x10000, 1);
  error = Deserialize(&wrapper, &buffer);
  expected = decltype(expected)(0x10000, 1);
  result.resize(wrapper.size());
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(expected, result);
}

TEST(DeserializationTest, pair) {
  Payload buffer;
  ErrorType error;

  std::pair<int, int> p1;
  buffer = {ENCODING_TYPE_FIXARRAY_MIN + 2, 1, 2};
  error = Deserialize(&p1, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(std::make_pair(1, 2), p1);
}

TEST(DeserializationTest, tuple) {
  Payload buffer;
  ErrorType error;

  // Min FIXARRAY.
  std::tuple<> t1;
  buffer = {ENCODING_TYPE_FIXARRAY_MIN};
  error = Deserialize(&t1, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(std::make_tuple(), t1);  // Superfluous.

  // Max FIXARRAY.
  auto t2 = GetNTuple<15, int>(0);
  buffer = {ENCODING_TYPE_FIXARRAY_MAX};
  buffer.Append((1 << 4) - 1, 1);
  error = Deserialize(&t2, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ((GetNTuple<15, int>(1)), t2);

  // Min ARRAY16.
  // Using t1 above.
  buffer = {ENCODING_TYPE_ARRAY16, 0x00, 0x00};
  error = Deserialize(&t1, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(std::make_tuple(), t1);

  // ARRAY16 at Max FIXARRAY + 1
  auto t3 = GetNTuple<(1 << 4), int>(0);
  buffer = {ENCODING_TYPE_ARRAY16, 0x10, 0x00};
  buffer.Append((1 << 4), 1);
  error = Deserialize(&t3, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ((GetNTuple<(1 << 4), int>(1)), t3);

  // Min ARRAY32.
  // Using t1 from above.
  buffer = {ENCODING_TYPE_ARRAY32, 0x00, 0x00, 0x00, 0x00};
  error = Deserialize(&t1, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(std::make_tuple(), t1);

  // ARRAY32 at Max FIXARRAY + 1
  auto t4 = GetNTuple<(1 << 4), int>(0);
  buffer = {ENCODING_TYPE_ARRAY32, 0x10, 0x00, 0x00, 0x00};
  buffer.Append((1 << 4), 1);
  error = Deserialize(&t4, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ((GetNTuple<(1 << 4), int>(1)), t4);

  // Template instantiation depth is an issue for tuples with large numbers of
  // elements. As these are not expected in practice, the limits of ARRAY16
  // and ARRAY32 are not tested.
}

TEST(DeserializationTest, Serializable) {
  Payload buffer;
  ErrorType error;

  buffer = decltype(buffer)(
      {ENCODING_TYPE_FIXARRAY_MIN + 4, 10, ENCODING_TYPE_FLOAT32,
       kZeroFloatBytes[0], kZeroFloatBytes[1], kZeroFloatBytes[2],
       kZeroFloatBytes[3], ENCODING_TYPE_FIXSTR_MIN + 5, '1', '2', '3', '4',
       '5', ENCODING_TYPE_POSITIVE_FIXINT_MIN + 1});
  TestType t1;
  error = Deserialize(&t1, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(TestType(10, 0.f, "12345", TestType::Foo::kBar), t1);

  buffer =
      decltype(buffer)({ENCODING_TYPE_FIXARRAY_MIN + 1, ENCODING_TYPE_FIXEXT2,
                        ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 0xff, 0xff});
  TestTemplateType<LocalHandle> tt;
  error = Deserialize(&tt, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_EQ(TestTemplateType<LocalHandle>(LocalHandle(-1)), tt);
}

TEST(DeserializationTest, Variant) {
  Payload buffer;
  ErrorType error;

  Variant<int, bool, float> v;

  buffer = {ENCODING_TYPE_FIXMAP_MIN + 1, ENCODING_TYPE_NEGATIVE_FIXINT_MAX,
            ENCODING_TYPE_NIL};
  error = Deserialize(&v, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  EXPECT_TRUE(v.empty());

  buffer = {ENCODING_TYPE_FIXMAP_MIN + 1, ENCODING_TYPE_POSITIVE_FIXINT_MIN + 0,
            ENCODING_TYPE_POSITIVE_FIXINT_MIN + 10};
  error = Deserialize(&v, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  ASSERT_TRUE(v.is<int>());
  EXPECT_EQ(10, std::get<int>(v));

  buffer = {ENCODING_TYPE_FIXMAP_MIN + 1, ENCODING_TYPE_POSITIVE_FIXINT_MIN + 1,
            ENCODING_TYPE_TRUE};
  error = Deserialize(&v, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  ASSERT_TRUE(v.is<bool>());
  EXPECT_EQ(true, std::get<bool>(v));

  buffer = {ENCODING_TYPE_FIXMAP_MIN + 1, ENCODING_TYPE_POSITIVE_FIXINT_MIN + 1,
            ENCODING_TYPE_FALSE};
  error = Deserialize(&v, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  ASSERT_TRUE(v.is<bool>());
  EXPECT_EQ(false, std::get<bool>(v));

  buffer = {ENCODING_TYPE_FIXMAP_MIN + 1,
            ENCODING_TYPE_POSITIVE_FIXINT_MIN + 2,
            ENCODING_TYPE_FLOAT32,
            kOneFloatBytes[0],
            kOneFloatBytes[1],
            kOneFloatBytes[2],
            kOneFloatBytes[3]};
  error = Deserialize(&v, &buffer);
  EXPECT_EQ(ErrorCode::NO_ERROR, error);
  ASSERT_TRUE(v.is<float>());
  EXPECT_FLOAT_EQ(1.0, std::get<float>(v));

  // TODO(eieio): Add more deserialization tests for Variant.
}

TEST(DeserializationTest, ErrorType) {
  Payload buffer;
  ErrorType error;

  std::uint8_t u8;
  buffer = {ENCODING_TYPE_STR8};
  error = Deserialize(&u8, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_STR8, error.encoding_type());

  std::uint16_t u16;
  buffer = {ENCODING_TYPE_STR8};
  error = Deserialize(&u16, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_STR8, error.encoding_type());

  std::uint32_t u32;
  buffer = {ENCODING_TYPE_STR8};
  error = Deserialize(&u32, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_STR8, error.encoding_type());

  std::uint64_t u64;
  buffer = {ENCODING_TYPE_STR8};
  error = Deserialize(&u64, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_STR8, error.encoding_type());

  std::int8_t i8;
  buffer = {ENCODING_TYPE_STR8};
  error = Deserialize(&i8, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_INT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_STR8, error.encoding_type());

  std::int16_t i16;
  buffer = {ENCODING_TYPE_STR8};
  error = Deserialize(&i16, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_INT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_STR8, error.encoding_type());

  std::int32_t i32;
  buffer = {ENCODING_TYPE_STR8};
  error = Deserialize(&i32, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_INT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_STR8, error.encoding_type());

  std::int64_t i64;
  buffer = {ENCODING_TYPE_STR8};
  error = Deserialize(&i64, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_INT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_STR8, error.encoding_type());

  std::string s;
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT};
  error = Deserialize(&s, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_STRING, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_POSITIVE_FIXINT, error.encoding_type());

  std::vector<std::uint8_t> v;
  buffer = {ENCODING_TYPE_POSITIVE_FIXINT};
  error = Deserialize(&v, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_ARRAY, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_POSITIVE_FIXINT, error.encoding_type());

  buffer = {ENCODING_TYPE_FIXARRAY_MIN + 1, ENCODING_TYPE_STR8};
  error = Deserialize(&v, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_ENCODING, error);
  EXPECT_EQ(ENCODING_CLASS_UINT, error.encoding_class());
  EXPECT_EQ(ENCODING_TYPE_STR8, error.encoding_type());

  buffer = {ENCODING_TYPE_FIXARRAY_MIN + 2, 0, 1};
  std::tuple<int> t;
  error = Deserialize(&t, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_TYPE_SIZE, error);

  buffer = {ENCODING_TYPE_FIXARRAY_MIN + 3, 0, 1, 2};
  std::pair<int, int> p;
  error = Deserialize(&p, &buffer);
  EXPECT_EQ(ErrorCode::UNEXPECTED_TYPE_SIZE, error);
}
