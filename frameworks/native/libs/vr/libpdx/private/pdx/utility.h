#ifndef ANDROID_PDX_UTILITY_H_
#define ANDROID_PDX_UTILITY_H_

#include <cstdint>
#include <cstdlib>
#include <iterator>

#include <pdx/rpc/sequence.h>

// Utilities for testing object serialization.

namespace android {
namespace pdx {

class ByteBuffer {
 public:
  using iterator = uint8_t*;
  using const_iterator = const uint8_t*;
  using size_type = size_t;

  ByteBuffer() = default;
  ByteBuffer(const ByteBuffer& other) {
    resize(other.size());
    if (other.size())
      memcpy(data_, other.data(), other.size());
  }
  ~ByteBuffer() { std::free(data_); }

  ByteBuffer& operator=(const ByteBuffer& other) {
    resize(other.size());
    if (other.size())
      memcpy(data_, other.data(), other.size());
    return *this;
  }

  ByteBuffer& operator=(ByteBuffer&& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    other.clear();
    return *this;
  }

  inline const uint8_t* data() const { return data_; }
  inline uint8_t* data() { return data_; }
  inline size_t size() const { return size_; }
  inline size_t capacity() const { return capacity_; }

  iterator begin() { return data_; }
  const_iterator begin() const { return data_; }
  iterator end() { return data_ + size_; }
  const_iterator end() const { return data_ + size_; }

  inline bool operator==(const ByteBuffer& other) const {
    return size_ == other.size_ &&
           (size_ == 0 || memcmp(data_, other.data_, size_) == 0);
  }

  inline bool operator!=(const ByteBuffer& other) const {
    return !operator==(other);
  }

  inline void reserve(size_t size) {
    if (size <= capacity_)
      return;
    // Find next power of 2 (assuming the size is 32 bits for now).
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size++;
    void* new_data = data_ ? std::realloc(data_, size) : std::malloc(size);
    // TODO(avakulenko): Check for allocation failures.
    data_ = static_cast<uint8_t*>(new_data);
    capacity_ = size;
  }

  inline void resize(size_t size) {
    reserve(size);
    size_ = size;
  }

  inline uint8_t* grow_by(size_t size_delta) {
    size_t old_size = size_;
    resize(old_size + size_delta);
    return data_ + old_size;
  }

  inline void clear() { size_ = 0; }

 private:
  uint8_t* data_{nullptr};
  size_t size_{0};
  size_t capacity_{0};
};

// Utility functions to increment/decrement void pointers to data buffers.
template <typename OFFSET_T>
inline const void* AdvancePointer(const void* ptr, OFFSET_T offset) {
  return static_cast<const uint8_t*>(ptr) + offset;
}

template <typename OFFSET_T>
inline void* AdvancePointer(void* ptr, OFFSET_T offset) {
  return static_cast<uint8_t*>(ptr) + offset;
}

inline ptrdiff_t PointerDistance(const void* end, const void* begin) {
  return static_cast<const uint8_t*>(end) - static_cast<const uint8_t*>(begin);
}

// Utility to build sequences of types.
template <typename, typename>
struct AppendTypeSequence;

template <typename T, typename... S, template <typename...> class TT>
struct AppendTypeSequence<T, TT<S...>> {
  using type = TT<S..., T>;
};

// Utility to generate repeated types.
template <typename T, std::size_t N, template <typename...> class TT>
struct RepeatedType {
  using type = typename AppendTypeSequence<
      T, typename RepeatedType<T, N - 1, TT>::type>::type;
};

template <typename T, template <typename...> class TT>
struct RepeatedType<T, 0, TT> {
  using type = TT<>;
};

template <typename V, typename S>
inline V ReturnValueHelper(V value, S /*ignore*/) {
  return value;
}

template <typename R, typename V, size_t... S>
inline R GetNTupleHelper(V value, rpc::IndexSequence<S...>) {
  return std::make_tuple(ReturnValueHelper(value, S)...);
}

// Returns an N-tuple of type std::tuple<T,...T> containing |value| in each
// element.
template <size_t N, typename T,
          typename R = typename RepeatedType<T, N, std::tuple>::type>
inline R GetNTuple(T value) {
  return GetNTupleHelper<R>(value, rpc::MakeIndexSequence<N>{});
}

class NoOpOutputResourceMapper : public OutputResourceMapper {
 public:
  Status<FileReference> PushFileHandle(const LocalHandle& handle) override {
    return handle.Get();
  }

  Status<FileReference> PushFileHandle(const BorrowedHandle& handle) override {
    return handle.Get();
  }

  Status<FileReference> PushFileHandle(const RemoteHandle& handle) override {
    return handle.Get();
  }

  Status<ChannelReference> PushChannelHandle(
      const LocalChannelHandle& handle) override {
    return handle.value();
  }

  Status<ChannelReference> PushChannelHandle(
      const BorrowedChannelHandle& handle) override {
    return handle.value();
  }

  Status<ChannelReference> PushChannelHandle(
      const RemoteChannelHandle& handle) override {
    return handle.value();
  }
};

class NoOpInputResourceMapper : public InputResourceMapper {
 public:
  bool GetFileHandle(FileReference ref, LocalHandle* handle) override {
    *handle = LocalHandle{ref};
    return true;
  }

  bool GetChannelHandle(ChannelReference ref,
                        LocalChannelHandle* handle) override {
    *handle = LocalChannelHandle{nullptr, ref};
    return true;
  }
};

class NoOpResourceMapper : public NoOpOutputResourceMapper,
                           public NoOpInputResourceMapper {};

// Simple implementation of the payload interface, required by
// Serialize/Deserialize. This is intended for test cases, where compatibility
// with std::vector is helpful.
class Payload : public MessageWriter,
                public MessageReader,
                public OutputResourceMapper {
 public:
  using BaseType = ByteBuffer;
  using iterator = typename BaseType::iterator;
  using const_iterator = typename BaseType::const_iterator;
  using size_type = typename BaseType::size_type;

  Payload() = default;
  explicit Payload(size_type count, uint8_t value = 0) { Append(count, value); }
  Payload(const Payload& other) : buffer_(other.buffer_) {}
  Payload(const std::initializer_list<uint8_t>& initializer) {
    buffer_.resize(initializer.size());
    std::copy(initializer.begin(), initializer.end(), buffer_.begin());
  }

  Payload& operator=(const Payload& other) {
    buffer_ = other.buffer_;
    read_pos_ = 0;
    return *this;
  }
  Payload& operator=(const std::initializer_list<uint8_t>& initializer) {
    buffer_.resize(initializer.size());
    std::copy(initializer.begin(), initializer.end(), buffer_.begin());
    read_pos_ = 0;
    return *this;
  }

  // Compares Payload with Payload.
  bool operator==(const Payload& other) const {
    return buffer_ == other.buffer_;
  }
  bool operator!=(const Payload& other) const {
    return buffer_ != other.buffer_;
  }

  // Compares Payload with std::vector.
  template <typename Type, typename AllocatorType>
  typename std::enable_if<sizeof(Type) == sizeof(uint8_t), bool>::type
  operator==(const std::vector<Type, AllocatorType>& other) const {
    return buffer_.size() == other.size() &&
           memcmp(buffer_.data(), other.data(), other.size()) == 0;
  }
  template <typename Type, typename AllocatorType>
  typename std::enable_if<sizeof(Type) == sizeof(uint8_t), bool>::type
  operator!=(const std::vector<Type, AllocatorType>& other) const {
    return !operator!=(other);
  }

  iterator begin() { return buffer_.begin(); }
  const_iterator begin() const { return buffer_.begin(); }
  iterator end() { return buffer_.end(); }
  const_iterator end() const { return buffer_.end(); }

  void Append(size_type count, uint8_t value) {
    auto* data = buffer_.grow_by(count);
    std::fill(data, data + count, value);
  }

  void Clear() {
    buffer_.clear();
    file_handles_.clear();
    read_pos_ = 0;
  }

  void Rewind() { read_pos_ = 0; }

  uint8_t* Data() { return buffer_.data(); }
  const uint8_t* Data() const { return buffer_.data(); }
  size_type Size() const { return buffer_.size(); }

  // MessageWriter
  void* GetNextWriteBufferSection(size_t size) override {
    return buffer_.grow_by(size);
  }

  OutputResourceMapper* GetOutputResourceMapper() override { return this; }

  // OutputResourceMapper
  Status<FileReference> PushFileHandle(const LocalHandle& handle) override {
    if (handle) {
      const int ref = file_handles_.size();
      file_handles_.push_back(handle.Get());
      return ref;
    } else {
      return handle.Get();
    }
  }

  Status<FileReference> PushFileHandle(const BorrowedHandle& handle) override {
    if (handle) {
      const int ref = file_handles_.size();
      file_handles_.push_back(handle.Get());
      return ref;
    } else {
      return handle.Get();
    }
  }

  Status<FileReference> PushFileHandle(const RemoteHandle& handle) override {
    return handle.Get();
  }

  Status<ChannelReference> PushChannelHandle(
      const LocalChannelHandle& handle) override {
    if (handle) {
      const int ref = file_handles_.size();
      file_handles_.push_back(handle.value());
      return ref;
    } else {
      return handle.value();
    }
  }

  Status<ChannelReference> PushChannelHandle(
      const BorrowedChannelHandle& handle) override {
    if (handle) {
      const int ref = file_handles_.size();
      file_handles_.push_back(handle.value());
      return ref;
    } else {
      return handle.value();
    }
  }

  Status<ChannelReference> PushChannelHandle(
      const RemoteChannelHandle& handle) override {
    return handle.value();
  }

  // MessageReader
  BufferSection GetNextReadBufferSection() override {
    return {buffer_.data() + read_pos_, &*buffer_.end()};
  }

  void ConsumeReadBufferSectionData(const void* new_start) override {
    read_pos_ = PointerDistance(new_start, buffer_.data());
  }

  InputResourceMapper* GetInputResourceMapper() override {
    return &input_resource_mapper_;
  }

  const int* FdArray() const { return file_handles_.data(); }
  std::size_t FdCount() const { return file_handles_.size(); }

 private:
  NoOpInputResourceMapper input_resource_mapper_;
  ByteBuffer buffer_;
  std::vector<int> file_handles_;
  size_t read_pos_{0};
};

}  // namespace pdx
}  // namespace android

// Helper macros for branch prediction hinting.
#ifdef __GNUC__
#define PDX_LIKELY(x) __builtin_expect(!!(x), true)
#define PDX_UNLIKELY(x) __builtin_expect(!!(x), false)
#else
#define PDX_LIKELY(x) (x)
#define PDX_UNLIKELY(x) (x)
#endif

#endif  // ANDROID_PDX_UTILITY_H_
