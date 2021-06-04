#include <memory>
#include <string>
#include <thread>
#include <utility>

#include <gtest/gtest.h>
#include <pdx/rpc/message_buffer.h>

namespace android {
namespace pdx {
namespace rpc {

class ThreadLocalBufferTest {
 public:
  // Returns the unique address of the thread-local buffer. Used to test the
  // correct behavior of the type-based thread local storage slot mapping
  // mechanism.
  template <typename Slot>
  static std::uintptr_t GetSlotAddress() {
    return reinterpret_cast<std::uintptr_t>(&MessageBuffer<Slot>::buffer_);
  }

  // Returns the raw value of the thread local buffer. Used to test the behavior
  // of backing buffer initialization.
  template <typename Slot>
  static std::uintptr_t GetSlotValue() {
    return reinterpret_cast<std::uintptr_t>(MessageBuffer<Slot>::buffer_);
  }
};

}  // namespace rpc
}  // namespace pdx
}  // namespace android

using namespace android::pdx::rpc;

namespace {

struct TypeTagA;
struct TypeTagB;

constexpr std::size_t kSendBufferIndex = 0;
constexpr std::size_t kReceiveBufferIndex = 1;

using SendSlotA = ThreadLocalSlot<TypeTagA, kSendBufferIndex>;
using SendSlotB = ThreadLocalSlot<TypeTagB, kSendBufferIndex>;
using ReceiveSlotA = ThreadLocalSlot<TypeTagA, kReceiveBufferIndex>;
using ReceiveSlotB = ThreadLocalSlot<TypeTagB, kReceiveBufferIndex>;

}  // anonymous namespace

// Tests that index and type-based thread-local slot addressing works by
// checking that the slot address is the same when the same index/type
// combination is used and different when different combinations are used.
TEST(ThreadLocalBufferTest, TypeSlots) {
  auto id1 = ThreadLocalBufferTest::GetSlotAddress<SendSlotA>();
  auto id2 = ThreadLocalBufferTest::GetSlotAddress<ReceiveSlotA>();
  auto id3 = ThreadLocalBufferTest::GetSlotAddress<SendSlotB>();
  auto id4 = ThreadLocalBufferTest::GetSlotAddress<ReceiveSlotB>();

  EXPECT_NE(id1, id2);
  EXPECT_NE(id3, id4);
  EXPECT_NE(id1, id3);
  EXPECT_NE(id2, id4);

  auto id1_alias = ThreadLocalBufferTest::GetSlotAddress<SendSlotA>();
  auto id2_alias = ThreadLocalBufferTest::GetSlotAddress<ReceiveSlotA>();
  auto id3_alias = ThreadLocalBufferTest::GetSlotAddress<SendSlotB>();
  auto id4_alias = ThreadLocalBufferTest::GetSlotAddress<ReceiveSlotB>();

  EXPECT_EQ(id1, id1_alias);
  EXPECT_EQ(id2, id2_alias);
  EXPECT_EQ(id3, id3_alias);
  EXPECT_EQ(id4, id4_alias);
}

// Tests that different threads get different buffers for the same slot address.
TEST(ThreadLocalBufferTest, ThreadSlots) {
  auto id1 = ThreadLocalBufferTest::GetSlotAddress<SendBuffer>();
  std::uintptr_t id2 = 0U;

  std::thread thread([&id2]() mutable {
    id2 = ThreadLocalBufferTest::GetSlotAddress<SendBuffer>();
  });
  thread.join();

  EXPECT_NE(0U, id1);
  EXPECT_NE(0U, id2);
  EXPECT_NE(id1, id2);
}

// Tests that thread-local buffers are allocated at the first buffer request.
TEST(ThreadLocalBufferTest, InitialValue) {
  struct TypeTagX;
  using SendSlotX = ThreadLocalSlot<TypeTagX, kSendBufferIndex>;

  auto value1 = ThreadLocalBufferTest::GetSlotValue<SendSlotX>();
  MessageBuffer<SendSlotX>::GetBuffer();
  auto value2 = ThreadLocalBufferTest::GetSlotValue<SendSlotX>();

  EXPECT_EQ(0U, value1);
  EXPECT_NE(0U, value2);
}

// Tests that the underlying buffers are the same for a given index/type pair
// and different across index/type combinations.
TEST(ThreadLocalBufferTest, BackingBuffer) {
  auto& buffer1 = MessageBuffer<SendSlotA>::GetBuffer();
  auto& buffer2 = MessageBuffer<SendSlotA>::GetBuffer();
  auto& buffer3 = MessageBuffer<SendSlotB>::GetBuffer();
  auto& buffer4 = MessageBuffer<SendSlotB>::GetBuffer();

  EXPECT_EQ(buffer1.data(), buffer2.data());
  EXPECT_EQ(buffer3.data(), buffer4.data());
  EXPECT_NE(buffer1.data(), buffer3.data());
  EXPECT_NE(buffer2.data(), buffer4.data());
}
