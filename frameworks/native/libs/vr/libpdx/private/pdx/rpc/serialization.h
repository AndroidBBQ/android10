#ifndef ANDROID_PDX_RPC_SERIALIZATION_H_
#define ANDROID_PDX_RPC_SERIALIZATION_H_

#include <cstdint>
#include <cstring>
#include <iterator>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <pdx/channel_handle.h>
#include <pdx/file_handle.h>
#include <pdx/message_reader.h>
#include <pdx/message_writer.h>
#include <pdx/trace.h>
#include <pdx/utility.h>

#include "array_wrapper.h"
#include "default_initialization_allocator.h"
#include "encoding.h"
#include "pointer_wrapper.h"
#include "string_wrapper.h"
#include "variant.h"

namespace android {
namespace pdx {
namespace rpc {

// Automatic serialization/deserialization library based on MessagePack
// (http://msgpack.org). This library provides top level Serialize() and
// Deserialize() functions to encode/decode a variety of data types.
//
// The following data types are supported:
//   * Standard signed integer types: int8_t, int16_t, int32_t, and int64_t.
//   * Regular signed integer types equivalent to the standard types:
//     signed char, short, int, long, and long long.
//   * Standard unsigned integer types: uint8_t, uint16_t, uint32_t, and
//     uint64_t.
//   * Regular unsigned integer types equivalent to the standard types:
//     unsigned char, unsigned short, unsigned int, unsigned long,
//     and unsigned long long.
//   * char without signed/unsigned qualifiers.
//   * bool.
//   * std::vector with value type of any supported type, including nesting.
//   * std::string.
//   * std::tuple with elements of any supported type, including nesting.
//   * std::pair with elements of any supported type, including nesting.
//   * std::map with keys and values of any supported type, including nesting.
//   * std::unordered_map with keys and values of any supported type, including
//     nesting.
//   * std::array with values of any supported type, including nesting.
//   * ArrayWrapper of any supported basic type.
//   * BufferWrapper of any POD type.
//   * StringWrapper of any supported char type.
//   * User types with correctly defined SerializableMembers member type.
//
// Planned support for:
//   * std::basic_string with all supported char types.

// Counting template for managing template recursion.
template <std::size_t N>
struct Index {};

// Forward declaration of traits type to access types with a SerializedMembers
// member type.
template <typename T>
class SerializableTraits;

template <typename T, typename... MemberPointers>
struct SerializableMembersType;

// Utility to deduce the template type from a derived type.
template <template <typename...> class TT, typename... Ts>
std::true_type DeduceTemplateType(const TT<Ts...>*);
template <template <typename...> class TT>
std::false_type DeduceTemplateType(...);

// Utility determining whether template type TT<...> is a base of type T.
template <template <typename...> class TT, typename T>
using IsTemplateBaseOf = decltype(DeduceTemplateType<TT>(std::declval<T*>()));

// Utility type for SFINAE in HasHasSerializableMembers.
template <typename... Ts>
using TrySerializableMembersType = void;

// Determines whether type T has a member type named SerializableMembers of
// template type SerializableMembersType.
template <typename, typename = void>
struct HasSerializableMembers : std::false_type {};
template <typename T>
struct HasSerializableMembers<
    T, TrySerializableMembersType<typename T::SerializableMembers>>
    : std::integral_constant<
          bool, IsTemplateBaseOf<SerializableMembersType,
                                 typename T::SerializableMembers>::value> {};

// Utility to simplify overload enable expressions for types with correctly
// defined SerializableMembers.
template <typename T>
using EnableIfHasSerializableMembers =
    typename std::enable_if<HasSerializableMembers<T>::value>::type;

// Utility to simplify overload enable expressions for enum types.
template <typename T, typename ReturnType = void>
using EnableIfEnum =
    typename std::enable_if<std::is_enum<T>::value, ReturnType>::type;

///////////////////////////////////////////////////////////////////////////////
// Error Reporting //
///////////////////////////////////////////////////////////////////////////////

// Error codes returned by the deserialization code.
enum class ErrorCode {
  NO_ERROR = 0,
  UNEXPECTED_ENCODING,
  UNEXPECTED_TYPE_SIZE,
  INSUFFICIENT_BUFFER,
  INSUFFICIENT_DESTINATION_SIZE,
  GET_FILE_DESCRIPTOR_FAILED,
  GET_CHANNEL_HANDLE_FAILED,
  INVALID_VARIANT_ELEMENT,
};

// Type for errors returned by the deserialization code.
class ErrorType {
 public:
  ErrorType() : error_code_(ErrorCode::NO_ERROR) {}

  // ErrorType constructor for generic error codes. Explicitly not explicit,
  // implicit conversion from ErrorCode to ErrorType is desirable behavior.
  // NOLINTNEXTLINE(google-explicit-constructor)
  ErrorType(ErrorCode error_code) : error_code_(error_code) {}

  // ErrorType constructor for encoding type errors.
  ErrorType(ErrorCode error_code, EncodingClass encoding_class,
            EncodingType encoding_type)
      : error_code_(error_code) {
    unexpected_encoding_.encoding_class = encoding_class;
    unexpected_encoding_.encoding_type = encoding_type;
  }

  // Evaluates to true if the ErrorType represents an error.
  explicit operator bool() const { return error_code_ != ErrorCode::NO_ERROR; }

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator ErrorCode() const { return error_code_; }
  ErrorCode error_code() const { return error_code_; }

  // Accessors for extra info about unexpected encoding errors.
  EncodingClass encoding_class() const {
    return unexpected_encoding_.encoding_class;
  }
  EncodingType encoding_type() const {
    return unexpected_encoding_.encoding_type;
  }

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator std::string() const {
    std::ostringstream stream;

    switch (error_code_) {
      case ErrorCode::NO_ERROR:
        return "NO_ERROR";
      case ErrorCode::UNEXPECTED_ENCODING:
        stream << "UNEXPECTED_ENCODING: " << static_cast<int>(encoding_class())
               << ", " << static_cast<int>(encoding_type());
        return stream.str();
      case ErrorCode::UNEXPECTED_TYPE_SIZE:
        return "UNEXPECTED_TYPE_SIZE";
      case ErrorCode::INSUFFICIENT_BUFFER:
        return "INSUFFICIENT_BUFFER";
      case ErrorCode::INSUFFICIENT_DESTINATION_SIZE:
        return "INSUFFICIENT_DESTINATION_SIZE";
      default:
        return "[Unknown Error]";
    }
  }

 private:
  ErrorCode error_code_;

  // Union of extra information for different error code types.
  union {
    // UNEXPECTED_ENCODING.
    struct {
      EncodingClass encoding_class;
      EncodingType encoding_type;
    } unexpected_encoding_;
  };
};

///////////////////////////////////////////////////////////////////////////////
// Object Size //
///////////////////////////////////////////////////////////////////////////////

inline constexpr std::size_t GetSerializedSize(const bool& b) {
  return GetEncodingSize(EncodeType(b));
}

// Overloads of GetSerializedSize() for standard integer types.
inline constexpr std::size_t GetSerializedSize(const char& c) {
  return GetEncodingSize(EncodeType(c));
}
inline constexpr std::size_t GetSerializedSize(const std::uint8_t& i) {
  return GetEncodingSize(EncodeType(i));
}
inline constexpr std::size_t GetSerializedSize(const std::int8_t& i) {
  return GetEncodingSize(EncodeType(i));
}
inline constexpr std::size_t GetSerializedSize(const std::uint16_t& i) {
  return GetEncodingSize(EncodeType(i));
}
inline constexpr std::size_t GetSerializedSize(const std::int16_t& i) {
  return GetEncodingSize(EncodeType(i));
}
inline constexpr std::size_t GetSerializedSize(const std::uint32_t& i) {
  return GetEncodingSize(EncodeType(i));
}
inline constexpr std::size_t GetSerializedSize(const std::int32_t& i) {
  return GetEncodingSize(EncodeType(i));
}
inline constexpr std::size_t GetSerializedSize(const std::uint64_t& i) {
  return GetEncodingSize(EncodeType(i));
}
inline constexpr std::size_t GetSerializedSize(const std::int64_t& i) {
  return GetEncodingSize(EncodeType(i));
}

inline constexpr std::size_t GetSerializedSize(const float& f) {
  return GetEncodingSize(EncodeType(f));
}
inline constexpr std::size_t GetSerializedSize(const double& d) {
  return GetEncodingSize(EncodeType(d));
}

// Overload for enum types.
template <typename T>
inline EnableIfEnum<T, std::size_t> GetSerializedSize(T v) {
  return GetSerializedSize(static_cast<std::underlying_type_t<T>>(v));
}

// Forward declaration for nested definitions.
inline std::size_t GetSerializedSize(const EmptyVariant&);
template <typename... Types>
inline std::size_t GetSerializedSize(const Variant<Types...>&);
template <typename T, typename Enabled = EnableIfHasSerializableMembers<T>>
inline constexpr std::size_t GetSerializedSize(const T&);
template <typename T>
inline constexpr std::size_t GetSerializedSize(const PointerWrapper<T>&);
inline constexpr std::size_t GetSerializedSize(const std::string&);
template <typename T>
inline constexpr std::size_t GetSerializedSize(const StringWrapper<T>&);
template <typename T>
inline constexpr std::size_t GetSerializedSize(const BufferWrapper<T>&);
template <FileHandleMode Mode>
inline constexpr std::size_t GetSerializedSize(const FileHandle<Mode>&);
template <ChannelHandleMode Mode>
inline constexpr std::size_t GetSerializedSize(const ChannelHandle<Mode>&);
template <typename T, typename Allocator>
inline std::size_t GetSerializedSize(const std::vector<T, Allocator>& v);
template <typename Key, typename T, typename Compare, typename Allocator>
inline std::size_t GetSerializedSize(
    const std::map<Key, T, Compare, Allocator>& m);
template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename Allocator>
inline std::size_t GetSerializedSize(
    const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>&);
template <typename T>
inline std::size_t GetSerializedSize(const ArrayWrapper<T>&);
template <typename T, std::size_t Size>
inline std::size_t GetSerializedSize(const std::array<T, Size>& v);
template <typename T, typename U>
inline std::size_t GetSerializedSize(const std::pair<T, U>& p);
template <typename... T>
inline std::size_t GetSerializedSize(const std::tuple<T...>& tuple);

// Overload for empty variant type.
inline std::size_t GetSerializedSize(const EmptyVariant& empty) {
  return GetEncodingSize(EncodeType(empty));
}

// Overload for Variant types.
template <typename... Types>
inline std::size_t GetSerializedSize(const Variant<Types...>& variant) {
  return GetEncodingSize(EncodeType(variant)) +
         GetSerializedSize(variant.index()) +
         variant.Visit(
             [](const auto& value) { return GetSerializedSize(value); });
}

// Overload for structs/classes with SerializableMembers defined.
template <typename T, typename Enabled>
inline constexpr std::size_t GetSerializedSize(const T& value) {
  return SerializableTraits<T>::GetSerializedSize(value);
}

// Overload for PointerWrapper.
template <typename T>
inline constexpr std::size_t GetSerializedSize(const PointerWrapper<T>& p) {
  return GetSerializedSize(p.Dereference());
}

// Overload for std::string.
inline constexpr std::size_t GetSerializedSize(const std::string& s) {
  return GetEncodingSize(EncodeType(s)) +
         s.length() * sizeof(std::string::value_type);
}

// Overload for StringWrapper.
template <typename T>
inline constexpr std::size_t GetSerializedSize(const StringWrapper<T>& s) {
  return GetEncodingSize(EncodeType(s)) +
         s.length() * sizeof(typename StringWrapper<T>::value_type);
}

// Overload for BufferWrapper types.
template <typename T>
inline constexpr std::size_t GetSerializedSize(const BufferWrapper<T>& b) {
  return GetEncodingSize(EncodeType(b)) +
         b.size() * sizeof(typename BufferWrapper<T>::value_type);
}

// Overload for FileHandle. FileHandle is encoded as a FIXEXT2, with a type code
// of "FileHandle" and a signed 16-bit offset into the pushed fd array. Empty
// FileHandles are encoded with an array index of -1.
template <FileHandleMode Mode>
inline constexpr std::size_t GetSerializedSize(const FileHandle<Mode>& fd) {
  return GetEncodingSize(EncodeType(fd)) + sizeof(std::int16_t);
}

// Overload for ChannelHandle. ChannelHandle is encoded as a FIXEXT4, with a
// type code of "ChannelHandle" and a signed 32-bit offset into the pushed
// channel array. Empty ChannelHandles are encoded with an array index of -1.
template <ChannelHandleMode Mode>
inline constexpr std::size_t GetSerializedSize(
    const ChannelHandle<Mode>& channel_handle) {
  return GetEncodingSize(EncodeType(channel_handle)) + sizeof(std::int32_t);
}

// Overload for standard vector types.
template <typename T, typename Allocator>
inline std::size_t GetSerializedSize(const std::vector<T, Allocator>& v) {
  return std::accumulate(v.begin(), v.end(), GetEncodingSize(EncodeType(v)),
                         [](const std::size_t& sum, const T& object) {
                           return sum + GetSerializedSize(object);
                         });
}

// Overload for standard map types.
template <typename Key, typename T, typename Compare, typename Allocator>
inline std::size_t GetSerializedSize(
    const std::map<Key, T, Compare, Allocator>& v) {
  return std::accumulate(
      v.begin(), v.end(), GetEncodingSize(EncodeType(v)),
      [](const std::size_t& sum, const std::pair<Key, T>& object) {
        return sum + GetSerializedSize(object.first) +
               GetSerializedSize(object.second);
      });
}

// Overload for standard unordered_map types.
template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename Allocator>
inline std::size_t GetSerializedSize(
    const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& v) {
  return std::accumulate(
      v.begin(), v.end(), GetEncodingSize(EncodeType(v)),
      [](const std::size_t& sum, const std::pair<Key, T>& object) {
        return sum + GetSerializedSize(object.first) +
               GetSerializedSize(object.second);
      });
}

// Overload for ArrayWrapper types.
template <typename T>
inline std::size_t GetSerializedSize(const ArrayWrapper<T>& v) {
  return std::accumulate(v.begin(), v.end(), GetEncodingSize(EncodeType(v)),
                         [](const std::size_t& sum, const T& object) {
                           return sum + GetSerializedSize(object);
                         });
}

// Overload for std::array types.
template <typename T, std::size_t Size>
inline std::size_t GetSerializedSize(const std::array<T, Size>& v) {
  return std::accumulate(v.begin(), v.end(), GetEncodingSize(EncodeType(v)),
                         [](const std::size_t& sum, const T& object) {
                           return sum + GetSerializedSize(object);
                         });
}

// Overload for std::pair.
template <typename T, typename U>
inline std::size_t GetSerializedSize(const std::pair<T, U>& p) {
  return GetEncodingSize(EncodeType(p)) + GetSerializedSize(p.first) +
         GetSerializedSize(p.second);
}

// Stops template recursion when the last tuple element is reached.
template <typename... T>
inline std::size_t GetTupleSize(const std::tuple<T...>&, Index<0>) {
  return 0;
}

// Gets the size of each element in a tuple recursively.
template <typename... T, std::size_t index>
inline std::size_t GetTupleSize(const std::tuple<T...>& tuple, Index<index>) {
  return GetTupleSize(tuple, Index<index - 1>()) +
         GetSerializedSize(std::get<index - 1>(tuple));
}

// Overload for tuple types. Gets the size of the tuple, recursing
// through the elements.
template <typename... T>
inline std::size_t GetSerializedSize(const std::tuple<T...>& tuple) {
  return GetEncodingSize(EncodeType(tuple)) +
         GetTupleSize(tuple, Index<sizeof...(T)>());
}

// Stops template recursion when the last member of a Serializable
// type is reached.
template <typename Members, typename T>
inline std::size_t GetMemberSize(const T&, Index<0>) {
  return 0;
}

// Gets the size of each member of a Serializable type recursively.
template <typename Members, typename T, std::size_t index>
inline std::size_t GetMemberSize(const T& object, Index<index>) {
  return GetMemberSize<Members>(object, Index<index - 1>()) +
         GetSerializedSize(Members::template At<index - 1>::Resolve(object));
}

// Gets the size of a type using the given SerializableMembersType
// type.
template <typename Members, typename T>
inline std::size_t GetMembersSize(const T& object) {
  return GetMemberSize<Members>(object, Index<Members::MemberCount>());
}

///////////////////////////////////////////////////////////////////////////////
// Object Serialization //
///////////////////////////////////////////////////////////////////////////////

//
// SerializeRaw() converts a primitive array or type into a raw byte string.
// These functions are named differently from SerializeObject() expressly to
// avoid catch-all specialization of that template, which can be difficult to
// detect otherwise.
//

inline void WriteRawData(void*& dest, const void* src, size_t size) {
  memcpy(dest, src, size);
  dest = static_cast<uint8_t*>(dest) + size;
}

// Serializes a primitive array into a raw byte string.
template <typename T,
          typename = typename std::enable_if<std::is_pod<T>::value>::type>
inline void SerializeRaw(const T& value, void*& buffer) {
  WriteRawData(buffer, &value, sizeof(value));
}

inline void SerializeEncoding(EncodingType encoding, void*& buffer) {
  SerializeRaw(encoding, buffer);
}

inline void SerializeType(const bool& value, void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
}

// Serializes the type code, extended type code, and size for
// extension types.
inline void SerializeExtEncoding(EncodingType encoding,
                                 EncodingExtType ext_type, std::size_t size,
                                 void*& buffer) {
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_EXT8) {
    std::uint8_t length = size;
    SerializeRaw(length, buffer);
  } else if (encoding == ENCODING_TYPE_EXT16) {
    std::uint16_t length = size;
    SerializeRaw(length, buffer);
  } else if (encoding == ENCODING_TYPE_EXT32) {
    std::uint32_t length = size;
    SerializeRaw(length, buffer);
  } else /* if (IsFixextEncoding(encoding) */ {
    // Encoding byte contains the fixext length, nothing else to do.
  }
  SerializeRaw(ext_type, buffer);
}

// Serializes the type code for file descriptor types.
template <FileHandleMode Mode>
inline void SerializeType(const FileHandle<Mode>& value, void*& buffer) {
  SerializeExtEncoding(EncodeType(value), ENCODING_EXT_TYPE_FILE_DESCRIPTOR, 2,
                       buffer);
}

// Serializes the type code for channel handle types.
template <ChannelHandleMode Mode>
inline void SerializeType(const ChannelHandle<Mode>& handle, void*& buffer) {
  SerializeExtEncoding(EncodeType(handle), ENCODING_EXT_TYPE_CHANNEL_HANDLE, 4,
                       buffer);
}

// Serializes type code for variant types.
template <typename... Types>
inline void SerializeType(const Variant<Types...>& value, void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
}

// Serializes the type code for string types.
template <typename StringType>
inline void SerializeStringType(const StringType& value, void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_STR8) {
    std::uint8_t length = value.length();
    SerializeRaw(length, buffer);
  } else if (encoding == ENCODING_TYPE_STR16) {
    std::uint16_t length = value.length();
    SerializeRaw(length, buffer);
  } else if (encoding == ENCODING_TYPE_STR32) {
    std::uint32_t length = value.length();
    SerializeRaw(length, buffer);
  } else /* if (IsFixstrEncoding(encoding) */ {
    // Encoding byte contains the fixstr length, nothing else to do.
  }
}

// Serializes the type code for std::string and StringWrapper. These types are
// interchangeable and must serialize to the same format.
inline void SerializeType(const std::string& value, void*& buffer) {
  SerializeStringType(value, buffer);
}
template <typename T>
inline void SerializeType(const StringWrapper<T>& value, void*& buffer) {
  SerializeStringType(value, buffer);
}

// Serializes the type code for bin types.
inline void SerializeBinEncoding(EncodingType encoding, std::size_t size,
                                 void*& buffer) {
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_BIN8) {
    std::uint8_t length = size;
    SerializeRaw(length, buffer);
  } else if (encoding == ENCODING_TYPE_BIN16) {
    std::uint16_t length = size;
    SerializeRaw(length, buffer);
  } else if (encoding == ENCODING_TYPE_BIN32) {
    std::uint32_t length = size;
    SerializeRaw(length, buffer);
  } else {
    // Invalid encoding for BIN type.
  }
}

// Serializes the type code for BufferWrapper types.
template <typename T>
inline void SerializeType(const BufferWrapper<T>& value, void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeBinEncoding(
      encoding, value.size() * sizeof(typename BufferWrapper<T>::value_type),
      buffer);
}

// Serializes the array encoding type and length.
inline void SerializeArrayEncoding(EncodingType encoding, std::size_t size,
                                   void*& buffer) {
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_ARRAY16) {
    std::uint16_t length = size;
    SerializeRaw(length, buffer);
  } else if (encoding == ENCODING_TYPE_ARRAY32) {
    std::uint32_t length = size;
    SerializeRaw(length, buffer);
  } else /* if (IsFixarrayEncoding(encoding) */ {
    // Encoding byte contains the fixarray length, nothing else to do.
  }
}

// Serializes the map encoding type and length.
inline void SerializeMapEncoding(EncodingType encoding, std::size_t size,
                                 void*& buffer) {
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_MAP16) {
    std::uint16_t length = size;
    SerializeRaw(length, buffer);
  } else if (encoding == ENCODING_TYPE_MAP32) {
    std::uint32_t length = size;
    SerializeRaw(length, buffer);
  } else /* if (IsFixmapEncoding(encoding) */ {
    // Encoding byte contains the fixmap length, nothing else to do.
  }
}

// Serializes the type code for array types.
template <typename ArrayType>
inline void SerializeArrayType(const ArrayType& value, std::size_t size,
                               void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeArrayEncoding(encoding, size, buffer);
}

// Serializes the type code for map types.
template <typename MapType>
inline void SerializeMapType(const MapType& value, std::size_t size,
                             void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeMapEncoding(encoding, size, buffer);
}

// Serializes the type code for std::vector and ArrayWrapper. These types are
// interchangeable and must serialize to the same format.
template <typename T, typename Allocator>
inline void SerializeType(const std::vector<T, Allocator>& value,
                          void*& buffer) {
  SerializeArrayType(value, value.size(), buffer);
}
template <typename T>
inline void SerializeType(const ArrayWrapper<T>& value, void*& buffer) {
  SerializeArrayType(value, value.size(), buffer);
}

// Serializes the type code for std::array. This type serializes to the same
// format as std::vector and ArrayWrapper and is interchangeable in certain
// situations.
template <typename T, std::size_t Size>
inline void SerializeType(const std::array<T, Size>& value, void*& buffer) {
  SerializeArrayType(value, Size, buffer);
}

// Serializes the type code for std::map types.
template <typename Key, typename T, typename Compare, typename Allocator>
inline void SerializeType(const std::map<Key, T, Compare, Allocator>& value,
                          void*& buffer) {
  SerializeMapType(value, value.size(), buffer);
}

// Serializes the type code for std::unordered_map types.
template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename Allocator>
inline void SerializeType(
    const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& value,
    void*& buffer) {
  SerializeMapType(value, value.size(), buffer);
}

// Serializes the type code for std::pair types.
template <typename T, typename U>
inline void SerializeType(const std::pair<T, U>& value, void*& buffer) {
  SerializeArrayType(value, 2, buffer);
}

// Serializes the type code for std::tuple types.
template <typename... T>
inline void SerializeType(const std::tuple<T...>& value, void*& buffer) {
  SerializeArrayType(value, sizeof...(T), buffer);
}

// Specialization of SerializeObject for boolean type.
inline void SerializeObject(const bool& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  SerializeType(value, buffer);
  // Encoding contains the boolean value, nothing else to do.
}

// Overloads of SerializeObject for float and double types.
inline void SerializeObject(const float& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  SerializeRaw(value, buffer);
}

inline void SerializeObject(const double& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  SerializeRaw(value, buffer);
}

// Overloads of SerializeObject() for standard integer types.
inline void SerializeObject(const char& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_UINT8) {
    SerializeRaw(value, buffer);
  } else /* if (IsUnsignedFixintEncoding(encoding) */ {
    // Encoding byte contains the value, nothing else to do.
  }
}

inline void SerializeObject(const int8_t& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_INT8) {
    SerializeRaw(value, buffer);
  } else /* if (IsFixintEncoding(encoding) */ {
    // Encoding byte contains the value, nothing else to do.
  }
}

inline void SerializeObject(const uint8_t& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_UINT8) {
    SerializeRaw(value, buffer);
  } else /* if (IsUnsignedFixintEncoding(encoding) */ {
    // Encoding byte contains the value, nothing else to do.
  }
}

inline void SerializeObject(const int16_t& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_INT8) {
    const int8_t byte = value;
    SerializeRaw(byte, buffer);
  } else if (encoding == ENCODING_TYPE_INT16) {
    SerializeRaw(value, buffer);
  } else /* if (IsFixintEncoding(encoding) */ {
    // Encoding byte contains the value, nothing else to do.
  }
}

inline void SerializeObject(const uint16_t& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_UINT8) {
    const uint8_t byte = value;
    SerializeRaw(byte, buffer);
  } else if (encoding == ENCODING_TYPE_UINT16) {
    SerializeRaw(value, buffer);
  } else /* if (IsUnsignedFixintEncoding(encoding) */ {
    // Encoding byte contains the value, nothing else to do.
  }
}

inline void SerializeObject(const int32_t& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_INT8) {
    const int8_t byte = value;
    SerializeRaw(byte, buffer);
  } else if (encoding == ENCODING_TYPE_INT16) {
    const int16_t half = value;
    SerializeRaw(half, buffer);
  } else if (encoding == ENCODING_TYPE_INT32) {
    SerializeRaw(value, buffer);
  } else /* if (IsFixintEncoding(encoding) */ {
    // Encoding byte contains the value, nothing else to do.
  }
}

inline void SerializeObject(const uint32_t& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_UINT8) {
    const uint8_t byte = value;
    SerializeRaw(byte, buffer);
  } else if (encoding == ENCODING_TYPE_UINT16) {
    const uint16_t half = value;
    SerializeRaw(half, buffer);
  } else if (encoding == ENCODING_TYPE_UINT32) {
    SerializeRaw(value, buffer);
  } else /* if (IsUnsignedFixintEncoding(encoding) */ {
    // Encoding byte contains the value, nothing else to do.
  }
}

inline void SerializeObject(const int64_t& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_INT8) {
    const int8_t byte = value;
    SerializeRaw(byte, buffer);
  } else if (encoding == ENCODING_TYPE_INT16) {
    const int16_t half = value;
    SerializeRaw(half, buffer);
  } else if (encoding == ENCODING_TYPE_INT32) {
    const int32_t word = value;
    SerializeRaw(word, buffer);
  } else if (encoding == ENCODING_TYPE_INT64) {
    SerializeRaw(value, buffer);
  } else /* if (IsFixintEncoding(encoding) */ {
    // Encoding byte contains the value, nothing else to do.
  }
}

inline void SerializeObject(const uint64_t& value, MessageWriter* /*writer*/,
                            void*& buffer) {
  const EncodingType encoding = EncodeType(value);
  SerializeEncoding(encoding, buffer);
  if (encoding == ENCODING_TYPE_UINT8) {
    const uint8_t byte = value;
    SerializeRaw(byte, buffer);
  } else if (encoding == ENCODING_TYPE_UINT16) {
    const uint16_t half = value;
    SerializeRaw(half, buffer);
  } else if (encoding == ENCODING_TYPE_UINT32) {
    const uint32_t word = value;
    SerializeRaw(word, buffer);
  } else if (encoding == ENCODING_TYPE_UINT64) {
    SerializeRaw(value, buffer);
  } else /* if (IsUnsignedFixintEncoding(encoding) */ {
    // Encoding byte contains the value, nothing else to do.
  }
}

// Serialize enum types.
template <typename T>
inline EnableIfEnum<T> SerializeObject(const T& value, MessageWriter* writer,
                                       void*& buffer) {
  SerializeObject(static_cast<std::underlying_type_t<T>>(value), writer,
                  buffer);
}

// Forward declaration for nested definitions.
inline void SerializeObject(const EmptyVariant&, MessageWriter*, void*&);
template <typename... Types>
inline void SerializeObject(const Variant<Types...>&, MessageWriter*, void*&);
template <typename T, typename Enabled = EnableIfHasSerializableMembers<T>>
inline void SerializeObject(const T&, MessageWriter*, void*&);
template <typename T>
inline void SerializeObject(const PointerWrapper<T>&, MessageWriter*, void*&);
template <FileHandleMode Mode>
inline void SerializeObject(const FileHandle<Mode>&, MessageWriter*, void*&);
template <ChannelHandleMode Mode>
inline void SerializeObject(const ChannelHandle<Mode>&, MessageWriter*, void*&);
template <typename T, typename Allocator>
inline void SerializeObject(const BufferWrapper<std::vector<T, Allocator>>&, MessageWriter*, void*&);
template <typename T>
inline void SerializeObject(const BufferWrapper<T*>&, MessageWriter*, void*&);
inline void SerializeObject(const std::string&, MessageWriter*, void*&);
template <typename T>
inline void SerializeObject(const StringWrapper<T>&, MessageWriter*, void*&);
template <typename T, typename Allocator>
inline void SerializeObject(const std::vector<T, Allocator>&, MessageWriter*, void*&);
template <typename T>
inline void SerializeObject(const ArrayWrapper<T>&, MessageWriter*, void*&);
template <typename T, std::size_t Size>
inline void SerializeObject(const std::array<T, Size>&, MessageWriter*, void*&);
template <typename Key, typename T, typename Compare, typename Allocator>
inline void SerializeObject(const std::map<Key, T, Compare, Allocator>&, MessageWriter*, void*&);
template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename Allocator>
inline void SerializeObject(
    const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>&, MessageWriter*, void*&);
template <typename T, typename U>
inline void SerializeObject(const std::pair<T, U>&, MessageWriter*, void*&);
template <typename... T>
inline void SerializeObject(const std::tuple<T...>&, MessageWriter*, void*&);

// Overload for empty variant type.
inline void SerializeObject(const EmptyVariant& empty,
                            MessageWriter* /*writer*/, void*& buffer) {
  const EncodingType encoding = EncodeType(empty);
  SerializeEncoding(encoding, buffer);
}

// Overload for Variant types.
template <typename... Types>
inline void SerializeObject(const Variant<Types...>& variant,
                            MessageWriter* writer, void*& buffer) {
  SerializeType(variant, buffer);
  SerializeObject(variant.index(), writer, buffer);
  return variant.Visit([writer, &buffer](const auto& value) {
    return SerializeObject(value, writer, buffer);
  });
}

// Overload for serializable structure/class types.
template <typename T, typename Enabled>
inline void SerializeObject(const T& value, MessageWriter* writer,
                            void*& buffer) {
  SerializableTraits<T>::SerializeObject(value, writer, buffer);
}

// Serializes the payload of a PointerWrapper.
template <typename T>
inline void SerializeObject(const PointerWrapper<T>& pointer,
                            MessageWriter* writer, void*& buffer) {
  SerializeObject(pointer.Dereference(), writer, buffer);
}

// Serializes the payload of file descriptor types.
template <FileHandleMode Mode>
inline void SerializeObject(const FileHandle<Mode>& fd, MessageWriter* writer,
                            void*& buffer) {
  SerializeType(fd, buffer);
  const Status<FileReference> status =
      writer->GetOutputResourceMapper()->PushFileHandle(fd);
  FileReference value = status ? status.get() : -status.error();
  SerializeRaw(value, buffer);
}

// Serializes the payload of channel handle types.
template <ChannelHandleMode Mode>
inline void SerializeObject(const ChannelHandle<Mode>& handle,
                            MessageWriter* writer, void*& buffer) {
  SerializeType(handle, buffer);
  const Status<ChannelReference> status =
      writer->GetOutputResourceMapper()->PushChannelHandle(handle);
  ChannelReference value = status ? status.get() : -status.error();
  SerializeRaw(value, buffer);
}

// Serializes the payload of BufferWrapper types.
template <typename T, typename Allocator>
inline void SerializeObject(const BufferWrapper<std::vector<T, Allocator>>& b,
                            MessageWriter* /*writer*/, void*& buffer) {
  const auto value_type_size =
      sizeof(typename BufferWrapper<std::vector<T, Allocator>>::value_type);
  SerializeType(b, buffer);
  WriteRawData(buffer, b.data(), b.size() * value_type_size);
}
template <typename T>
inline void SerializeObject(const BufferWrapper<T*>& b,
                            MessageWriter* /*writer*/, void*& buffer) {
  const auto value_type_size = sizeof(typename BufferWrapper<T*>::value_type);
  SerializeType(b, buffer);
  WriteRawData(buffer, b.data(), b.size() * value_type_size);
}

// Serializes the payload of string types.
template <typename StringType>
inline void SerializeString(const StringType& s, void*& buffer) {
  const auto value_type_size = sizeof(typename StringType::value_type);
  SerializeType(s, buffer);
  WriteRawData(buffer, s.data(), s.length() * value_type_size);
}

// Overload of SerializeObject() for std::string and StringWrapper. These types
// are interchangeable and must serialize to the same format.
inline void SerializeObject(const std::string& s, MessageWriter* /*writer*/,
                            void*& buffer) {
  SerializeString(s, buffer);
}
template <typename T>
inline void SerializeObject(const StringWrapper<T>& s,
                            MessageWriter* /*writer*/, void*& buffer) {
  SerializeString(s, buffer);
}

// Serializes the payload of array types.
template <typename ArrayType>
inline void SerializeArray(const ArrayType& v, MessageWriter* writer,
                           void*& buffer) {
  SerializeType(v, buffer);
  for (const auto& element : v)
    SerializeObject(element, writer, buffer);
}

// Serializes the payload for map types.
template <typename MapType>
inline void SerializeMap(const MapType& v, MessageWriter* writer,
                         void*& buffer) {
  SerializeType(v, buffer);
  for (const auto& element : v) {
    SerializeObject(element.first, writer, buffer);
    SerializeObject(element.second, writer, buffer);
  }
}

// Overload of SerializeObject() for std::vector and ArrayWrapper types. These
// types are interchangeable and must serialize to the same format.
template <typename T, typename Allocator>
inline void SerializeObject(const std::vector<T, Allocator>& v,
                            MessageWriter* writer, void*& buffer) {
  SerializeArray(v, writer, buffer);
}
template <typename T>
inline void SerializeObject(const ArrayWrapper<T>& v, MessageWriter* writer,
                            void*& buffer) {
  SerializeArray(v, writer, buffer);
}

// Overload of SerializeObject() for std::array types. These types serialize to
// the same format at std::vector and ArrayWrapper and are interchangeable in
// certain situations.
template <typename T, std::size_t Size>
inline void SerializeObject(const std::array<T, Size>& v, MessageWriter* writer,
                            void*& buffer) {
  SerializeArray(v, writer, buffer);
}

// Overload of SerializeObject() for std::map types.
template <typename Key, typename T, typename Compare, typename Allocator>
inline void SerializeObject(const std::map<Key, T, Compare, Allocator>& v,
                            MessageWriter* writer, void*& buffer) {
  SerializeMap(v, writer, buffer);
}

// Overload of SerializeObject() for std::unordered_map types.
template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename Allocator>
inline void SerializeObject(
    const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& v,
    MessageWriter* writer, void*& buffer) {
  SerializeMap(v, writer, buffer);
}

// Overload of SerializeObject() for std:pair types.
template <typename T, typename U>
inline void SerializeObject(const std::pair<T, U>& pair, MessageWriter* writer,
                            void*& buffer) {
  SerializeType(pair, buffer);
  SerializeObject(pair.first, writer, buffer);
  SerializeObject(pair.second, writer, buffer);
}

// Stops template recursion when the last tuple element is reached.
template <typename... T>
inline void SerializeTuple(const std::tuple<T...>&, MessageWriter*, void*&,
                           Index<0>) {}

// Serializes each element of a tuple recursively.
template <typename... T, std::size_t index>
inline void SerializeTuple(const std::tuple<T...>& tuple, MessageWriter* writer,
                           void*& buffer, Index<index>) {
  SerializeTuple(tuple, writer, buffer, Index<index - 1>());
  SerializeObject(std::get<index - 1>(tuple), writer, buffer);
}

// Overload of SerializeObject() for tuple types.
template <typename... T>
inline void SerializeObject(const std::tuple<T...>& tuple,
                            MessageWriter* writer, void*& buffer) {
  SerializeType(tuple, buffer);
  SerializeTuple(tuple, writer, buffer, Index<sizeof...(T)>());
}

// Stops template recursion when the last member pointer is reached.
template <typename Members, typename T>
inline void SerializeMember(const T&, MessageWriter*, void*&, Index<0>) {}

// Serializes each member pointer recursively.
template <typename Members, typename T, std::size_t index>
inline void SerializeMember(const T& object, MessageWriter* writer,
                            void*& buffer, Index<index>) {
  SerializeMember<Members>(object, writer, buffer, Index<index - 1>());
  SerializeObject(Members::template At<index - 1>::Resolve(object), writer,
                  buffer);
}

// Serializes the members of a type using the given SerializableMembersType
// type.
template <typename Members, typename T>
inline void SerializeMembers(const T& object, MessageWriter* writer,
                             void*& buffer) {
  SerializeMember<Members>(object, writer, buffer,
                           Index<Members::MemberCount>());
}

// Top level serialization function that replaces the buffer's contents.
template <typename T>
inline void Serialize(const T& object, MessageWriter* writer) {
  PDX_TRACE_NAME("Serialize");
  const std::size_t size = GetSerializedSize(object);

  // Reserve the space needed for the object(s).
  void* buffer = writer->GetNextWriteBufferSection(size);
  SerializeObject(object, writer, buffer);
}

///////////////////////////////////////////////////////////////////////////////
// Object Deserialization //
///////////////////////////////////////////////////////////////////////////////

inline ErrorType ReadRawDataFromNextSection(void* dest, MessageReader* reader,
                                            const void*& start,
                                            const void*& end, size_t size) {
  while (AdvancePointer(start, size) > end) {
    auto remaining_size = PointerDistance(end, start);
    if (remaining_size > 0) {
      memcpy(dest, start, remaining_size);
      dest = AdvancePointer(dest, remaining_size);
      size -= remaining_size;
    }
    reader->ConsumeReadBufferSectionData(AdvancePointer(start, remaining_size));
    std::tie(start, end) = reader->GetNextReadBufferSection();
    if (start == end)
      return ErrorCode::INSUFFICIENT_BUFFER;
  }
  memcpy(dest, start, size);
  start = AdvancePointer(start, size);
  return ErrorCode::NO_ERROR;
}

inline ErrorType ReadRawData(void* dest, MessageReader* /*reader*/,
                             const void*& start, const void*& end,
                             size_t size) {
  if (PDX_UNLIKELY(AdvancePointer(start, size) > end)) {
    // TODO(avakulenko): Enabling reading from next sections of input buffer
    // (using ReadRawDataFromNextSection) screws up clang compiler optimizations
    // (probably inefficient inlining) making the whole deserialization
    // code path about twice as slow. Investigate and enable more generic
    // deserialization code, but right now we don't really need/support this
    // scenario, so I keep this commented out for the time being...

    // return ReadRawDataFromNextSection(dest, reader, start, end, size);
    return ErrorCode::INSUFFICIENT_BUFFER;
  }
  memcpy(dest, start, size);
  start = AdvancePointer(start, size);
  return ErrorCode::NO_ERROR;
}

// Deserializes a primitive object from raw bytes.
template <typename T,
          typename = typename std::enable_if<std::is_pod<T>::value>::type>
inline ErrorType DeserializeRaw(T* value, MessageReader* reader,
                                const void*& start, const void*& end) {
  return ReadRawData(value, reader, start, end, sizeof(T));
}

// Utility to deserialize POD types when the serialized type is different
// (smaller) than the target real type. This happens when values are serialized
// into more compact encodings.
template <typename SerializedType, typename RealType>
ErrorType DeserializeValue(RealType* real_value, MessageReader* reader,
                           const void*& start, const void*& end) {
  SerializedType serialized_value;
  if (const auto error =
          DeserializeRaw(&serialized_value, reader, start, end)) {
    return error;
  } else {
    *real_value = serialized_value;
    return ErrorCode::NO_ERROR;
  }
}

inline ErrorType DeserializeEncoding(EncodingType* encoding,
                                     MessageReader* reader, const void*& start,
                                     const void*& end) {
  return DeserializeRaw(encoding, reader, start, end);
}

// Overload to deserialize bool type.
inline ErrorType DeserializeObject(bool* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsBoolEncoding(encoding)) {
    *value = (encoding == ENCODING_TYPE_TRUE);
    return ErrorCode::NO_ERROR;
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_BOOL,
                     encoding);
  }
}

// Specializations to deserialize float and double types.
inline ErrorType DeserializeObject(float* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsFloat32Encoding(encoding)) {
    return DeserializeValue<float>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_FLOAT,
                     encoding);
  }
}

inline ErrorType DeserializeObject(double* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsFloat32Encoding(encoding)) {
    return DeserializeValue<float>(value, reader, start, end);
  } else if (IsFloat64Encoding(encoding)) {
    return DeserializeValue<double>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_FLOAT,
                     encoding);
  }
}

// Specializations to deserialize standard integer types.
inline ErrorType DeserializeObject(char* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsUnsignedFixintEncoding(encoding)) {
    *value = static_cast<char>(encoding);
    return ErrorCode::NO_ERROR;
  } else if (IsUInt8Encoding(encoding)) {
    return DeserializeValue<char>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_UINT,
                     encoding);
  }
}

inline ErrorType DeserializeObject(std::int8_t* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsFixintEncoding(encoding)) {
    *value = static_cast<std::int8_t>(encoding);
    return ErrorCode::NO_ERROR;
  } else if (IsInt8Encoding(encoding)) {
    return DeserializeValue<std::int8_t>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_INT,
                     encoding);
  }
}

inline ErrorType DeserializeObject(std::uint8_t* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsUnsignedFixintEncoding(encoding)) {
    *value = encoding;
    return ErrorCode::NO_ERROR;
  } else if (IsUInt8Encoding(encoding)) {
    return DeserializeValue<std::uint8_t>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_UINT,
                     encoding);
  }
}

inline ErrorType DeserializeObject(std::int16_t* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsFixintEncoding(encoding)) {
    *value = static_cast<std::int8_t>(encoding);
    return ErrorCode::NO_ERROR;
  } else if (IsInt8Encoding(encoding)) {
    return DeserializeValue<std::int8_t>(value, reader, start, end);
  } else if (IsInt16Encoding(encoding)) {
    return DeserializeValue<std::int16_t>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_INT,
                     encoding);
  }
}

inline ErrorType DeserializeObject(std::uint16_t* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsUnsignedFixintEncoding(encoding)) {
    *value = encoding;
    return ErrorCode::NO_ERROR;
  } else if (IsUInt8Encoding(encoding)) {
    return DeserializeValue<std::uint8_t>(value, reader, start, end);
  } else if (IsUInt16Encoding(encoding)) {
    return DeserializeValue<std::uint16_t>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_UINT,
                     encoding);
  }
}

inline ErrorType DeserializeObject(std::int32_t* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsFixintEncoding(encoding)) {
    *value = static_cast<std::int8_t>(encoding);
    return ErrorCode::NO_ERROR;
  } else if (IsInt8Encoding(encoding)) {
    return DeserializeValue<std::int8_t>(value, reader, start, end);
  } else if (IsInt16Encoding(encoding)) {
    return DeserializeValue<std::int16_t>(value, reader, start, end);
  } else if (IsInt32Encoding(encoding)) {
    return DeserializeValue<std::int32_t>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_INT,
                     encoding);
  }
}

inline ErrorType DeserializeObject(std::uint32_t* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsUnsignedFixintEncoding(encoding)) {
    *value = encoding;
    return ErrorCode::NO_ERROR;
  } else if (IsUInt8Encoding(encoding)) {
    return DeserializeValue<std::uint8_t>(value, reader, start, end);
  } else if (IsUInt16Encoding(encoding)) {
    return DeserializeValue<std::uint16_t>(value, reader, start, end);
  } else if (IsUInt32Encoding(encoding)) {
    return DeserializeValue<std::uint32_t>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_UINT,
                     encoding);
  }
}

inline ErrorType DeserializeObject(std::int64_t* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsFixintEncoding(encoding)) {
    *value = static_cast<std::int8_t>(encoding);
    return ErrorCode::NO_ERROR;
  } else if (IsInt8Encoding(encoding)) {
    return DeserializeValue<std::int8_t>(value, reader, start, end);
  } else if (IsInt16Encoding(encoding)) {
    return DeserializeValue<std::int16_t>(value, reader, start, end);
  } else if (IsInt32Encoding(encoding)) {
    return DeserializeValue<std::int32_t>(value, reader, start, end);
  } else if (IsInt64Encoding(encoding)) {
    return DeserializeValue<std::int64_t>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_INT,
                     encoding);
  }
}

inline ErrorType DeserializeObject(std::uint64_t* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (IsUnsignedFixintEncoding(encoding)) {
    *value = encoding;
    return ErrorCode::NO_ERROR;
  } else if (IsUInt8Encoding(encoding)) {
    return DeserializeValue<std::uint8_t>(value, reader, start, end);
  } else if (IsUInt16Encoding(encoding)) {
    return DeserializeValue<std::uint16_t>(value, reader, start, end);
  } else if (IsUInt32Encoding(encoding)) {
    return DeserializeValue<std::uint32_t>(value, reader, start, end);
  } else if (IsUInt64Encoding(encoding)) {
    return DeserializeValue<std::uint64_t>(value, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_UINT,
                     encoding);
  }
}

template <typename T>
inline EnableIfEnum<T, ErrorType> DeserializeObject(T* value,
                                                    MessageReader* reader,
                                                    const void*& start,
                                                    const void*& end) {
  std::underlying_type_t<T> enum_value;
  ErrorType error = DeserializeObject(&enum_value, reader, start, end);
  if (!error)
    *value = static_cast<T>(enum_value);
  return error;
}

// Forward declarations for nested definitions.
template <typename T, typename Enabled = EnableIfHasSerializableMembers<T>>
inline ErrorType DeserializeObject(T*, MessageReader*, const void*&,
                                   const void*&);
template <typename T>
inline ErrorType DeserializeObject(PointerWrapper<T>*, MessageReader*,
                                   const void*&, const void*&);
inline ErrorType DeserializeObject(LocalHandle*, MessageReader*, const void*&,
                                   const void*&);
inline ErrorType DeserializeObject(LocalChannelHandle*, MessageReader*,
                                   const void*&, const void*&);
template <typename T, typename Allocator>
inline ErrorType DeserializeObject(BufferWrapper<std::vector<T, Allocator>>*,
                                   MessageReader*, const void*&, const void*&);
template <typename T>
inline ErrorType DeserializeObject(BufferWrapper<T*>*, MessageReader*,
                                   const void*&, const void*&);
inline ErrorType DeserializeObject(std::string*, MessageReader*, const void*&,
                                   const void*&);
template <typename T>
inline ErrorType DeserializeObject(StringWrapper<T>*, MessageReader*,
                                   const void*&, const void*&);
template <typename T, typename U>
inline ErrorType DeserializeObject(std::pair<T, U>*, MessageReader*,
                                   const void*&, const void*&);
template <typename... T>
inline ErrorType DeserializeObject(std::tuple<T...>*, MessageReader*,
                                   const void*&, const void*&);
template <typename T, typename Allocator>
inline ErrorType DeserializeObject(std::vector<T, Allocator>*, MessageReader*,
                                   const void*&, const void*&);
template <typename Key, typename T, typename Compare, typename Allocator>
inline ErrorType DeserializeObject(std::map<Key, T, Compare, Allocator>*,
                                   MessageReader*, const void*&, const void*&);
template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename Allocator>
inline ErrorType DeserializeObject(
    std::unordered_map<Key, T, Hash, KeyEqual, Allocator>*, MessageReader*,
    const void*&, const void*&);
template <typename T>
inline ErrorType DeserializeObject(ArrayWrapper<T>*, MessageReader*,
                                   const void*&, const void*&);
template <typename T, std::size_t Size>
inline ErrorType DeserializeObject(std::array<T, Size>*, MessageReader*,
                                   const void*&, const void*&);
template <typename T, typename U>
inline ErrorType DeserializeObject(std::pair<T, U>*, MessageReader*,
                                   const void*&, const void*&);
template <typename... T>
inline ErrorType DeserializeObject(std::tuple<T...>*, MessageReader*,
                                   const void*&, const void*&);
inline ErrorType DeserializeObject(EmptyVariant*,
                                   MessageReader*, const void*&,
                                   const void*&);
template <typename... Types>
inline ErrorType DeserializeObject(Variant<Types...>*,
                                   MessageReader*, const void*&,
                                   const void*&);

// Deserializes a Serializable type.
template <typename T, typename Enable>
inline ErrorType DeserializeObject(T* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  return SerializableTraits<T>::DeserializeObject(value, reader, start, end);
}

// Deserializes a PointerWrapper.
template <typename T>
inline ErrorType DeserializeObject(PointerWrapper<T>* pointer,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  return DeserializeObject(&pointer->Dereference(), reader, start, end);
}

// Deserializes the type code and size for extension types.
inline ErrorType DeserializeExtType(EncodingType* encoding,
                                    EncodingExtType* type, std::size_t* size,
                                    MessageReader* reader, const void*& start,
                                    const void*& end) {
  if (const auto error = DeserializeEncoding(encoding, reader, start, end)) {
    return error;
  } else if (IsFixextEncoding(*encoding)) {
    *size = GetFixextSize(*encoding);
  } else if (*encoding == ENCODING_TYPE_EXT8) {
    if (const auto error =
            DeserializeValue<std::uint8_t>(size, reader, start, end))
      return error;
  } else if (*encoding == ENCODING_TYPE_EXT16) {
    if (const auto error =
            DeserializeValue<std::uint16_t>(size, reader, start, end))
      return error;
  } else if (*encoding == ENCODING_TYPE_EXT32) {
    if (const auto error =
            DeserializeValue<std::uint32_t>(size, reader, start, end))
      return error;
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_EXTENSION,
                     *encoding);
  }

  // The extension type code follows the encoding and size.
  return DeserializeRaw(type, reader, start, end);
}

// Deserializes a file handle and performs handle space translation, if
// required.
inline ErrorType DeserializeObject(LocalHandle* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  EncodingExtType type;
  std::size_t size;

  if (const auto error =
          DeserializeExtType(&encoding, &type, &size, reader, start, end)) {
    return error;
  } else if (size != 2) {
    return ErrorType(ErrorCode::UNEXPECTED_TYPE_SIZE, ENCODING_CLASS_EXTENSION,
                     encoding);
  } else if (type == ENCODING_EXT_TYPE_FILE_DESCRIPTOR) {
    // Read the encoded file descriptor value.
    FileReference ref;
    if (const auto error = DeserializeRaw(&ref, reader, start, end)) {
      return error;
    }

    return reader->GetInputResourceMapper()->GetFileHandle(ref, value)
               ? ErrorCode::NO_ERROR
               : ErrorCode::GET_FILE_DESCRIPTOR_FAILED;
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_EXTENSION,
                     encoding);
  }
}

inline ErrorType DeserializeObject(LocalChannelHandle* value,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  EncodingType encoding;
  EncodingExtType type;
  std::size_t size;

  if (const auto error =
          DeserializeExtType(&encoding, &type, &size, reader, start, end)) {
    return error;
  } else if (size != 4) {
    return ErrorType(ErrorCode::UNEXPECTED_TYPE_SIZE, ENCODING_CLASS_EXTENSION,
                     encoding);
  } else if (type == ENCODING_EXT_TYPE_CHANNEL_HANDLE) {
    // Read the encoded channel handle value.
    ChannelReference ref;
    if (const auto error = DeserializeRaw(&ref, reader, start, end)) {
      return error;
    }
    return reader->GetInputResourceMapper()->GetChannelHandle(ref, value)
               ? ErrorCode::NO_ERROR
               : ErrorCode::GET_CHANNEL_HANDLE_FAILED;
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_EXTENSION,
                     encoding);
  }
}

// Deserializes the type code and size for bin types.
inline ErrorType DeserializeBinType(EncodingType* encoding, std::size_t* size,
                                    MessageReader* reader, const void*& start,
                                    const void*& end) {
  if (const auto error = DeserializeEncoding(encoding, reader, start, end)) {
    return error;
  } else if (*encoding == ENCODING_TYPE_BIN8) {
    return DeserializeValue<std::uint8_t>(size, reader, start, end);
  } else if (*encoding == ENCODING_TYPE_BIN16) {
    return DeserializeValue<std::uint16_t>(size, reader, start, end);
  } else if (*encoding == ENCODING_TYPE_BIN32) {
    return DeserializeValue<std::uint32_t>(size, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_BINARY,
                     *encoding);
  }
}

// Overload of DeserializeObject() for BufferWrapper types.
template <typename T, typename Allocator>
inline ErrorType DeserializeObject(
    BufferWrapper<std::vector<T, Allocator>>* value, MessageReader* reader,
    const void*& start, const void*& end) {
  const auto value_type_size =
      sizeof(typename BufferWrapper<std::vector<T, Allocator>>::value_type);
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeBinType(&encoding, &size, reader, start, end))
    return error;

  // Try to resize the BufferWrapper to the size of the payload.
  value->resize(size / value_type_size);

  if (size > value->size() * value_type_size || size % value_type_size != 0) {
    return ErrorCode::INSUFFICIENT_DESTINATION_SIZE;
  } else if (size == 0U) {
    return ErrorCode::NO_ERROR;
  } else {
    return ReadRawData(value->data(), reader, start, end, size);
  }
}
template <typename T>
inline ErrorType DeserializeObject(BufferWrapper<T*>* value,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  const auto value_type_size = sizeof(typename BufferWrapper<T*>::value_type);
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeBinType(&encoding, &size, reader, start, end))
    return error;

  // Try to resize the BufferWrapper to the size of the payload.
  value->resize(size / value_type_size);

  if (size > value->size() * value_type_size || size % value_type_size != 0) {
    return ErrorCode::INSUFFICIENT_DESTINATION_SIZE;
  } else if (size == 0U) {
    return ErrorCode::NO_ERROR;
  } else {
    return ReadRawData(value->data(), reader, start, end, size);
  }
}

// Deserializes the type code and size for string types.
inline ErrorType DeserializeStringType(EncodingType* encoding,
                                       std::size_t* size, MessageReader* reader,
                                       const void*& start, const void*& end) {
  if (const auto error = DeserializeEncoding(encoding, reader, start, end)) {
    return error;
  } else if (IsFixstrEncoding(*encoding)) {
    *size = GetFixstrSize(*encoding);
    return ErrorCode::NO_ERROR;
  } else if (*encoding == ENCODING_TYPE_STR8) {
    return DeserializeValue<std::uint8_t>(size, reader, start, end);
  } else if (*encoding == ENCODING_TYPE_STR16) {
    return DeserializeValue<std::uint16_t>(size, reader, start, end);
  } else if (*encoding == ENCODING_TYPE_STR32) {
    return DeserializeValue<std::uint32_t>(size, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_STRING,
                     *encoding);
  }
}

// Overload of DeserializeObject() for std::string types.
inline ErrorType DeserializeObject(std::string* value, MessageReader* reader,
                                   const void*& start, const void*& end) {
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeStringType(&encoding, &size, reader, start, end)) {
    return error;
  } else if (size == 0U) {
    value->clear();
    return ErrorCode::NO_ERROR;
  } else {
    value->resize(size);
    return ReadRawData(&(*value)[0], reader, start, end, size);
  }
}

// Overload of DeserializeObject() for StringWrapper types.
template <typename T>
inline ErrorType DeserializeObject(StringWrapper<T>* value,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  const auto value_type_size = sizeof(typename StringWrapper<T>::value_type);
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeStringType(&encoding, &size, reader, start, end))
    return error;

  // Try to resize the StringWrapper to the size of the payload
  // string.
  value->resize(size / value_type_size);

  if (size > value->length() * value_type_size || size % value_type_size != 0) {
    return ErrorCode::INSUFFICIENT_DESTINATION_SIZE;
  } else if (size == 0U) {
    return ErrorCode::NO_ERROR;
  } else {
    return ReadRawData(value->data(), reader, start, end, size);
  }
}

// Deserializes the type code and size of array types.
inline ErrorType DeserializeArrayType(EncodingType* encoding, std::size_t* size,
                                      MessageReader* reader, const void*& start,
                                      const void*& end) {
  if (const auto error = DeserializeEncoding(encoding, reader, start, end)) {
    return error;
  } else if (IsFixarrayEncoding(*encoding)) {
    *size = GetFixarraySize(*encoding);
    return ErrorCode::NO_ERROR;
  } else if (*encoding == ENCODING_TYPE_ARRAY16) {
    return DeserializeValue<std::uint16_t>(size, reader, start, end);
  } else if (*encoding == ENCODING_TYPE_ARRAY32) {
    return DeserializeValue<std::uint32_t>(size, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_ARRAY,
                     *encoding);
  }
}

// Deserializes the type code and size of map types.
inline ErrorType DeserializeMapType(EncodingType* encoding, std::size_t* size,
                                    MessageReader* reader, const void*& start,
                                    const void*& end) {
  if (const auto error = DeserializeEncoding(encoding, reader, start, end)) {
    return error;
  } else if (IsFixmapEncoding(*encoding)) {
    *size = GetFixmapSize(*encoding);
    return ErrorCode::NO_ERROR;
  } else if (*encoding == ENCODING_TYPE_MAP16) {
    return DeserializeValue<std::uint16_t>(size, reader, start, end);
  } else if (*encoding == ENCODING_TYPE_MAP32) {
    return DeserializeValue<std::uint32_t>(size, reader, start, end);
  } else {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_MAP,
                     *encoding);
  }
}

// Overload for std::vector types.
template <typename T, typename Allocator>
inline ErrorType DeserializeObject(std::vector<T, Allocator>* value,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeArrayType(&encoding, &size, reader, start, end))
    return error;

  std::vector<T, Allocator> result(size);
  for (std::size_t i = 0; i < size; i++) {
    if (const auto error = DeserializeObject(&result[i], reader, start, end))
      return error;
  }

  *value = std::move(result);
  return ErrorCode::NO_ERROR;

// TODO(eieio): Consider the benefits and trade offs of this alternative.
#if 0
  value->resize(size);
  for (std::size_t i = 0; i < size; i++) {
    if (const auto error = DeserializeObject(&(*value)[i], reader, start, end))
      return error;
  }
  return ErrorCode::NO_ERROR;
#endif
}

// Deserializes an EmptyVariant value.
inline ErrorType DeserializeObject(EmptyVariant* /*empty*/,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  EncodingType encoding;

  if (const auto error = DeserializeEncoding(&encoding, reader, start, end)) {
    return error;
  } else if (encoding != ENCODING_TYPE_NIL) {
    return ErrorType(ErrorCode::UNEXPECTED_ENCODING, ENCODING_CLASS_MAP,
                     encoding);
  } else {
    return ErrorCode::NO_ERROR;
  }
}

// Deserializes a Variant type.
template <typename... Types>
inline ErrorType DeserializeObject(Variant<Types...>* variant,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeMapType(&encoding, &size, reader, start, end)) {
    return error;
  }

  if (size != 1)
    return ErrorType(ErrorCode::UNEXPECTED_TYPE_SIZE, ENCODING_CLASS_MAP,
                     encoding);

  std::int32_t type;
  if (const auto error = DeserializeObject(&type, reader, start, end)) {
    return error;
  } else if (type < Variant<Types...>::kEmptyIndex ||
             type >= static_cast<std::int32_t>(sizeof...(Types))) {
    return ErrorCode::INVALID_VARIANT_ELEMENT;
  } else {
    variant->Become(type);
    return variant->Visit([reader, &start, &end](auto&& value) {
      return DeserializeObject(&value, reader, start, end);
    });
  }
}

// Deserializes map types.
template <typename MapType>
inline ErrorType DeserializeMap(MapType* value, MessageReader* reader,
                                const void*& start, const void*& end) {
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeMapType(&encoding, &size, reader, start, end))
    return error;

  MapType result;
  for (std::size_t i = 0; i < size; i++) {
    std::pair<typename MapType::key_type, typename MapType::mapped_type>
        element;
    if (const auto error =
            DeserializeObject(&element.first, reader, start, end))
      return error;
    if (const auto error =
            DeserializeObject(&element.second, reader, start, end))
      return error;
    result.emplace(std::move(element));
  }

  *value = std::move(result);
  return ErrorCode::NO_ERROR;
}

// Overload for std::map types.
template <typename Key, typename T, typename Compare, typename Allocator>
inline ErrorType DeserializeObject(std::map<Key, T, Compare, Allocator>* value,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  return DeserializeMap(value, reader, start, end);
}

// Overload for std::unordered_map types.
template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename Allocator>
inline ErrorType DeserializeObject(
    std::unordered_map<Key, T, Hash, KeyEqual, Allocator>* value,
    MessageReader* reader, const void*& start, const void*& end) {
  return DeserializeMap(value, reader, start, end);
}

// Overload for ArrayWrapper types.
template <typename T>
inline ErrorType DeserializeObject(ArrayWrapper<T>* value,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeArrayType(&encoding, &size, reader, start, end)) {
    return error;
  }

  // Try to resize the wrapper.
  value->resize(size);

  // Make sure there is enough space in the ArrayWrapper for the
  // payload.
  if (size > value->capacity())
    return ErrorCode::INSUFFICIENT_DESTINATION_SIZE;

  for (std::size_t i = 0; i < size; i++) {
    if (const auto error = DeserializeObject(&(*value)[i], reader, start, end))
      return error;
  }

  return ErrorCode::NO_ERROR;
}

// Overload for std::array types.
template <typename T, std::size_t Size>
inline ErrorType DeserializeObject(std::array<T, Size>* value,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeArrayType(&encoding, &size, reader, start, end)) {
    return error;
  }

  if (size != Size)
    return ErrorCode::INSUFFICIENT_DESTINATION_SIZE;

  for (std::size_t i = 0; i < size; i++) {
    if (const auto error = DeserializeObject(&(*value)[i], reader, start, end))
      return error;
  }

  return ErrorCode::NO_ERROR;
}

// Deserializes std::pair types.
template <typename T, typename U>
inline ErrorType DeserializeObject(std::pair<T, U>* value,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeArrayType(&encoding, &size, reader, start, end)) {
    return error;
  } else if (size != 2) {
    return ErrorCode::UNEXPECTED_TYPE_SIZE;
  } else if (const auto error =
                 DeserializeObject(&value->first, reader, start, end)) {
    return error;
  } else if (const auto error =
                 DeserializeObject(&value->second, reader, start, end)) {
    return error;
  } else {
    return ErrorCode::NO_ERROR;
  }
}

// Stops template recursion when the last tuple element is reached.
template <typename... T>
inline ErrorType DeserializeTuple(std::tuple<T...>*, MessageReader*,
                                  const void*&, const void*, Index<0>) {
  return ErrorCode::NO_ERROR;
}

// Deserializes each element of a tuple recursively.
template <typename... T, std::size_t index>
inline ErrorType DeserializeTuple(std::tuple<T...>* tuple,
                                  MessageReader* reader, const void*& start,
                                  const void*& end, Index<index>) {
  if (const auto error =
          DeserializeTuple(tuple, reader, start, end, Index<index - 1>()))
    return error;
  else
    return DeserializeObject(&std::get<index - 1>(*tuple), reader, start, end);
}

// Overload for standard tuple types.
template <typename... T>
inline ErrorType DeserializeObject(std::tuple<T...>* value,
                                   MessageReader* reader, const void*& start,
                                   const void*& end) {
  EncodingType encoding;
  std::size_t size;

  if (const auto error =
          DeserializeArrayType(&encoding, &size, reader, start, end)) {
    return error;
  } else if (size != sizeof...(T)) {
    return ErrorCode::UNEXPECTED_TYPE_SIZE;
  } else {
    return DeserializeTuple(value, reader, start, end, Index<sizeof...(T)>());
  }
}

// Stops template recursion when the last member of a Serializable type is
// reached.
template <typename Members, typename T>
inline ErrorType DeserializeMember(T*, MessageReader*, const void*&,
                                   const void*, Index<0>) {
  return ErrorCode::NO_ERROR;
}

// Deserializes each member of a Serializable type recursively.
template <typename Members, typename T, std::size_t index>
inline ErrorType DeserializeMember(T* value, MessageReader* reader,
                                   const void*& start, const void*& end,
                                   Index<index>) {
  if (const auto error = DeserializeMember<Members>(value, reader, start, end,
                                                    Index<index - 1>()))
    return error;
  else
    return DeserializeObject(&Members::template At<index - 1>::Resolve(*value),
                             reader, start, end);
}

// Deserializes the members of a Serializable type using the given
// SerializableMembersType type.
template <typename Members, typename T>
inline ErrorType DeserializeMembers(T* value, MessageReader* reader,
                                    const void*& start, const void*& end) {
  return DeserializeMember<Members>(value, reader, start, end,
                                    Index<Members::MemberCount>());
}

// Top level deserialization function.
template <typename T>
inline ErrorType Deserialize(T* value, MessageReader* reader) {
  PDX_TRACE_NAME("Deserialize");
  MessageReader::BufferSection section = reader->GetNextReadBufferSection();
  if (section.first == section.second)
    return ErrorCode::INSUFFICIENT_BUFFER;
  ErrorType error =
      DeserializeObject(value, reader, section.first, section.second);
  reader->ConsumeReadBufferSectionData(section.first);
  return error;
}

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_SERIALIZATION_H_
