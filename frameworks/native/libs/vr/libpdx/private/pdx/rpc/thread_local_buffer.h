#ifndef ANDROID_PDX_RPC_THREAD_LOCAL_BUFFER_H_
#define ANDROID_PDX_RPC_THREAD_LOCAL_BUFFER_H_

#include <cstdint>
#include <memory>
#include <vector>

#include <pdx/rpc/default_initialization_allocator.h>
#include <pdx/trace.h>

namespace android {
namespace pdx {
namespace rpc {

// Utility class to distinguish between different thread local entries or
// "slots" in the thread local variable table. Each slot is uniquely identified
// by (T,Index) and is independent of any other slot.
template <typename T, std::size_t Index>
struct ThreadLocalSlot;

// Utility class to specify thread local slots using only a type.
template <typename T>
struct ThreadLocalTypeSlot;

// Utility class to specify thread local slots using only an index.
template <std::size_t Index>
struct ThreadLocalIndexSlot;

// Initial capacity of thread local buffer, unless otherwise specified.
constexpr std::size_t InitialBufferCapacity = 4096;

// Thread local slots for buffers used by this library to send, receive, and
// reply to messages.
using SendBuffer = ThreadLocalIndexSlot<0>;
using ReceiveBuffer = ThreadLocalIndexSlot<1>;
using ReplyBuffer = ThreadLocalIndexSlot<2>;

// Provides a simple interface to thread local buffers for large IPC messages.
// Slot provides multiple thread local slots for a given T, Allocator, Capacity
// combination.
template <typename T, typename Allocator = DefaultInitializationAllocator<T>,
          std::size_t Capacity = InitialBufferCapacity,
          typename Slot = ThreadLocalSlot<void, 0>>
class ThreadLocalBuffer {
 public:
  using BufferType = std::vector<T, Allocator>;
  using ValueType = T;

  // Reserves |capacity| number of elements of capacity in the underlying
  // buffer. Call this during startup to avoid allocation during use.
  static void Reserve(std::size_t capacity) {
    PDX_TRACE_NAME("ThreadLocalBuffer::Reserve");
    InitializeBuffer(capacity);
    buffer_->reserve(capacity);
  }

  // Resizes the buffer to |size| elements.
  static void Resize(std::size_t size) {
    PDX_TRACE_NAME("ThreadLocalBuffer::Resize");
    InitializeBuffer(size);
    buffer_->resize(size);
  }

  // Gets a reference to the underlying buffer after reserving |capacity|
  // elements. The current size of the buffer is left intact. The returned
  // reference is valid until FreeBuffer() is called.
  static BufferType& GetBuffer(std::size_t capacity = Capacity) {
    PDX_TRACE_NAME("ThreadLocalBuffer::GetBuffer");
    Reserve(capacity);
    return *buffer_;
  }

  // Gets a reference to the underlying buffer after reserving |Capacity|
  // elements. The current size of the buffer is set to zero. The returned
  // reference is valid until FreeBuffer() is called.
  static BufferType& GetEmptyBuffer() {
    PDX_TRACE_NAME("ThreadLocalBuffer::GetEmptyBuffer");
    Reserve(Capacity);
    buffer_->clear();
    return *buffer_;
  }

  // Gets a reference to the underlying buffer after resizing it to |size|
  // elements. The returned reference is valid until FreeBuffer() is called.
  static BufferType& GetSizedBuffer(std::size_t size = Capacity) {
    PDX_TRACE_NAME("ThreadLocalBuffer::GetSizedBuffer");
    Resize(size);
    return *buffer_;
  }

  // Frees the underlying buffer. The buffer will be reallocated if any of the
  // methods above are called.
  static void FreeBuffer() {
    if (buffer_) {
      GetBufferGuard().reset(buffer_ = nullptr);
    }
  }

 private:
  friend class ThreadLocalBufferTest;

  static void InitializeBuffer(std::size_t capacity) {
    if (!buffer_) {
      GetBufferGuard().reset(buffer_ = new BufferType(capacity));
    }
  }

  // Work around performance issues with thread-local dynamic initialization
  // semantics by using a normal pointer in parallel with a std::unique_ptr. The
  // std::unique_ptr is never dereferenced, only assigned, to avoid the high
  // cost of dynamic initialization checks, while still providing automatic
  // cleanup. The normal pointer provides fast access to the buffer object.
  // Never dereference buffer_guard or performance could be severely impacted
  // by slow implementations of TLS dynamic initialization.
  static thread_local BufferType* buffer_;

  static std::unique_ptr<BufferType>& GetBufferGuard() {
    PDX_TRACE_NAME("ThreadLocalBuffer::GetBufferGuard");
    static thread_local std::unique_ptr<BufferType> buffer_guard;
    return buffer_guard;
  }
};

// Instantiation of the static ThreadLocalBuffer::buffer_ member.
template <typename T, typename Allocator, std::size_t Capacity, typename Slot>
thread_local
    typename ThreadLocalBuffer<T, Allocator, Capacity, Slot>::BufferType*
        ThreadLocalBuffer<T, Allocator, Capacity, Slot>::buffer_;

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_THREAD_LOCAL_BUFFER_H_
