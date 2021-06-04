#ifndef ANDROID_PDX_RPC_ARRAY_WRAPPER_H_
#define ANDROID_PDX_RPC_ARRAY_WRAPPER_H_

#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>

namespace android {
namespace pdx {
namespace rpc {

// Wrapper class for C array buffers, providing an interface suitable for
// SerializeObject and DeserializeObject. This class serializes to the same
// format as std::vector, and may be substituted for std::vector during
// serialization and deserialization. This substitution makes handling of C
// arrays more efficient by avoiding unnecessary copies when remote method
// signatures specify std::vector arguments or return values.
template <typename T>
class ArrayWrapper {
 public:
  // Define types in the style of STL containers to support STL operators.
  typedef T value_type;
  typedef std::size_t size_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T* pointer;
  typedef const T* const_pointer;

  ArrayWrapper() : buffer_(nullptr), capacity_(0), end_(0) {}

  ArrayWrapper(pointer buffer, size_type capacity, size_type size)
      : buffer_(&buffer[0]),
        capacity_(capacity),
        end_(capacity < size ? capacity : size) {}

  ArrayWrapper(pointer buffer, size_type size)
      : ArrayWrapper(buffer, size, size) {}

  ArrayWrapper(const ArrayWrapper& other) { *this = other; }

  ArrayWrapper(ArrayWrapper&& other) noexcept { *this = std::move(other); }

  ArrayWrapper& operator=(const ArrayWrapper& other) {
    if (&other == this) {
      return *this;
    } else {
      buffer_ = other.buffer_;
      capacity_ = other.capacity_;
      end_ = other.end_;
    }

    return *this;
  }

  ArrayWrapper& operator=(ArrayWrapper&& other) noexcept {
    if (&other == this) {
      return *this;
    } else {
      buffer_ = other.buffer_;
      capacity_ = other.capacity_;
      end_ = other.end_;
      other.buffer_ = nullptr;
      other.capacity_ = 0;
      other.end_ = 0;
    }

    return *this;
  }

  pointer data() { return buffer_; }
  const_pointer data() const { return buffer_; }

  pointer begin() { return &buffer_[0]; }
  pointer end() { return &buffer_[end_]; }
  const_pointer begin() const { return &buffer_[0]; }
  const_pointer end() const { return &buffer_[end_]; }

  size_type size() const { return end_; }
  size_type max_size() const { return capacity_; }
  size_type capacity() const { return capacity_; }

  // Moves the end marker to |size|, clamping the end marker to the max capacity
  // of the underlying array. This method does not change the size of the
  // underlying array.
  void resize(size_type size) {
    if (size <= capacity_)
      end_ = size;
    else
      end_ = capacity_;
  }

  reference operator[](size_type pos) { return buffer_[pos]; }
  const_reference operator[](size_type pos) const { return buffer_[pos]; }

 private:
  pointer buffer_;
  size_type capacity_;
  size_type end_;
};

template <typename T, typename SizeType = std::size_t>
ArrayWrapper<T> WrapArray(T* buffer, SizeType size) {
  return ArrayWrapper<T>(buffer, size);
}

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_ARRAY_WRAPPER_H_
