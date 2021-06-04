#ifndef ANDROID_PDX_RPC_DEFAULT_INITIALIZATION_ALLOCATOR_H_
#define ANDROID_PDX_RPC_DEFAULT_INITIALIZATION_ALLOCATOR_H_

#include <memory>

namespace android {
namespace pdx {
namespace rpc {

// Allocator adaptor that interposes construct() calls to convert value
// initialization into default initialization. All standard containers
// value-initialize their elements when constructed with a single size_type
// argument or when grown by a call to resize. This allocator avoids potentially
// costly value-initialization in these situations for value types that are
// default constructible. As a consequence, elements of non-class types are left
// uninitialized; this is desirable when using std::vector as a resizable
// buffer, for example.
template <typename T, typename Allocator = std::allocator<T>>
class DefaultInitializationAllocator : public Allocator {
  typedef std::allocator_traits<Allocator> AllocatorTraits;

 public:
  template <typename U>
  struct rebind {
    using other = DefaultInitializationAllocator<
        U, typename AllocatorTraits::template rebind_alloc<U>>;
  };

  using Allocator::Allocator;

  template <typename U>
  void construct(U* pointer) noexcept(
      std::is_nothrow_default_constructible<U>::value) {
    ::new (static_cast<void*>(pointer)) U;
  }
  template <typename U, typename... Args>
  void construct(U* pointer, Args&&... args) {
    AllocatorTraits::construct(static_cast<Allocator&>(*this), pointer,
                               std::forward<Args>(args)...);
  }
};

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  //  ANDROID_PDX_RPC_DEFAULT_INITIALIZATION_ALLOCATOR_H_
