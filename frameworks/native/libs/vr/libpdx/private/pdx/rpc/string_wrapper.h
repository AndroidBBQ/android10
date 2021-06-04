#ifndef ANDROID_PDX_RPC_STRING_WRAPPER_H_
#define ANDROID_PDX_RPC_STRING_WRAPPER_H_

#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>

namespace android {
namespace pdx {
namespace rpc {

// Wrapper class for C string buffers, providing an interface suitable for
// SerializeObject and DeserializeObject. This class serializes to the same
// format as std::basic_string, and may be substituted for std::basic_string
// during serialization and deserialization. This substitution makes handling of
// C strings more efficient by avoiding unnecessary copies when remote method
// signatures specify std::basic_string arguments or return values.
template <typename CharT = std::string::value_type,
          typename Traits = std::char_traits<CharT>>
class StringWrapper {
 public:
  // Define types in the style of STL strings to support STL operators.
  typedef Traits traits_type;
  typedef typename Traits::char_type value_type;
  typedef std::size_t size_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;

  StringWrapper() : buffer_(nullptr), capacity_(0), end_(0) {}

  StringWrapper(pointer buffer, size_type capacity, size_type size)
      : buffer_(&buffer[0]),
        capacity_(capacity),
        end_(capacity < size ? capacity : size) {}

  StringWrapper(pointer buffer, size_type size)
      : StringWrapper(buffer, size, size) {}

  explicit StringWrapper(pointer buffer)
      : StringWrapper(buffer, std::strlen(buffer)) {}

  StringWrapper(const StringWrapper& other) { *this = other; }

  StringWrapper(StringWrapper&& other) noexcept { *this = std::move(other); }

  StringWrapper& operator=(const StringWrapper& other) {
    if (&other == this) {
      return *this;
    } else {
      buffer_ = other.buffer_;
      capacity_ = other.capacity_;
      end_ = other.end_;
    }

    return *this;
  }

  StringWrapper& operator=(StringWrapper&& other) noexcept {
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
  size_type length() const { return end_; }
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

// Utility functions that infer the underlying type of the string, simplifying
// the wrapper interface.

// TODO(eieio): Wrapping std::basic_string is here for completeness, but is it
// useful?
template <typename T, typename... Any>
StringWrapper<const T> WrapString(const std::basic_string<T, Any...>& s) {
  return StringWrapper<const T>(s.c_str(), s.length());
}

template <typename T, typename SizeType = std::size_t>
StringWrapper<T> WrapString(T* s, SizeType size) {
  return StringWrapper<T>(s, size);
}

template <typename T>
StringWrapper<T> WrapString(T* s) {
  return StringWrapper<T>(s);
}

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_STRING_WRAPPER_H_
