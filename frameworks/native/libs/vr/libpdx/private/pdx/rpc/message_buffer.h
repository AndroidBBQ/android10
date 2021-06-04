#ifndef ANDROID_PDX_RPC_MESSAGE_BUFFER_H_
#define ANDROID_PDX_RPC_MESSAGE_BUFFER_H_

#include <pdx/rpc/thread_local_buffer.h>

namespace android {
namespace pdx {
namespace rpc {

// Utility type for thread-local buffers, providing suitable defaults for most
// situations. Independent thread-local buffers may be created by using
// different types for Slot -- ThreadLocalSlot, ThreadLocalTypedSlot and
// ThreadLocalIndexedSlot provide utilities for building these types.
template <typename Slot, std::size_t Capacity = 4096, typename T = std::uint8_t,
          typename Allocator = DefaultInitializationAllocator<T>>
using MessageBuffer = ThreadLocalBuffer<T, Allocator, Capacity, Slot>;

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_MESSAGE_BUFFER_H_
