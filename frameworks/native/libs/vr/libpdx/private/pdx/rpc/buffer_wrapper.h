#ifndef ANDROID_PDX_RPC_BUFFER_WRAPPER_H_
#define ANDROID_PDX_RPC_BUFFER_WRAPPER_H_

#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>

namespace android {
namespace pdx {
namespace rpc {

// Wrapper class for buffers, providing an interface suitable for
// SerializeObject and DeserializeObject. This class supports serialization of
// buffers as raw bytes.
template <typename T>
class BufferWrapper;

template <typename T>
class BufferWrapper<T*> {
 public:
  // Define types in the style of STL containers to support STL operators.
  typedef T value_type;
  typedef std::size_t size_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T* pointer;
  typedef const T* const_pointer;

  BufferWrapper() : buffer_(nullptr), capacity_(0), end_(0) {}

  BufferWrapper(pointer buffer, size_type capacity, size_type size)
      : buffer_(&buffer[0]),
        capacity_(capacity),
        end_(capacity < size ? capacity : size) {}

  BufferWrapper(pointer buffer, size_type size)
      : BufferWrapper(buffer, size, size) {}

  BufferWrapper(const BufferWrapper& other) { *this = other; }

  BufferWrapper(BufferWrapper&& other) noexcept { *this = std::move(other); }

  BufferWrapper& operator=(const BufferWrapper& other) {
    if (&other == this) {
      return *this;
    } else {
      buffer_ = other.buffer_;
      capacity_ = other.capacity_;
      end_ = other.end_;
    }

    return *this;
  }

  BufferWrapper& operator=(BufferWrapper&& other) noexcept {
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

template <typename T, typename Allocator>
class BufferWrapper<std::vector<T, Allocator>> {
 public:
  using BufferType = typename std::vector<T, Allocator>;
  using value_type = typename BufferType::value_type;
  using size_type = typename BufferType::size_type;
  using reference = typename BufferType::reference;
  using const_reference = typename BufferType::const_reference;
  using pointer = typename BufferType::pointer;
  using const_pointer = typename BufferType::const_pointer;
  using iterator = typename BufferType::iterator;
  using const_iterator = typename BufferType::const_iterator;

  BufferWrapper() {}
  explicit BufferWrapper(const BufferType& buffer) : buffer_(buffer) {}
  BufferWrapper(const BufferType& buffer, const Allocator& allocator)
      : buffer_(buffer, allocator) {}
  explicit BufferWrapper(BufferType&& buffer) : buffer_(std::move(buffer)) {}
  BufferWrapper(BufferType&& buffer, const Allocator& allocator)
      : buffer_(std::move(buffer), allocator) {}
  BufferWrapper(const BufferWrapper&) = default;
  BufferWrapper(BufferWrapper&&) noexcept = default;
  BufferWrapper& operator=(const BufferWrapper&) = default;
  BufferWrapper& operator=(BufferWrapper&&) noexcept = default;

  pointer data() { return buffer_.data(); }
  const_pointer data() const { return buffer_.data(); }

  iterator begin() { return buffer_.begin(); }
  iterator end() { return buffer_.end(); }
  const_iterator begin() const { return buffer_.begin(); }
  const_iterator end() const { return buffer_.end(); }

  size_type size() const { return buffer_.size(); }
  size_type max_size() const { return buffer_.capacity(); }
  size_type capacity() const { return buffer_.capacity(); }

  void resize(size_type size) { buffer_.resize(size); }
  void reserve(size_type size) { buffer_.reserve(size); }

  reference operator[](size_type pos) { return buffer_[pos]; }
  const_reference operator[](size_type pos) const { return buffer_[pos]; }

  BufferType& buffer() { return buffer_; }
  const BufferType& buffer() const { return buffer_; }

 private:
  BufferType buffer_;
};

template <typename T, typename SizeType = std::size_t>
BufferWrapper<T*> WrapBuffer(T* buffer, SizeType size) {
  return BufferWrapper<T*>(buffer, size);
}

template <typename SizeType = std::size_t>
BufferWrapper<std::uint8_t*> WrapBuffer(void* buffer, SizeType size) {
  return BufferWrapper<std::uint8_t*>(static_cast<std::uint8_t*>(buffer), size);
}

template <typename SizeType = std::size_t>
BufferWrapper<const std::uint8_t*> WrapBuffer(const void* buffer,
                                              SizeType size) {
  return BufferWrapper<const std::uint8_t*>(
      static_cast<const std::uint8_t*>(buffer), size);
}

template <typename T, typename Allocator = std::allocator<T>>
BufferWrapper<std::vector<T, Allocator>> WrapBuffer(
    std::vector<T, Allocator>&& buffer) {
  return BufferWrapper<std::vector<T, Allocator>>(
      std::forward<std::vector<T, Allocator>>(buffer));
}

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_BUFFER_WRAPPER_H_
