#ifndef ANDROID_PDX_RPC_ENCODING_H_
#define ANDROID_PDX_RPC_ENCODING_H_

#include <array>
#include <cstdint>
#include <cstring>
#include <map>
#include <numeric>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <pdx/channel_handle.h>
#include <pdx/file_handle.h>

#include "array_wrapper.h"
#include "buffer_wrapper.h"
#include "string_wrapper.h"
#include "variant.h"

namespace android {
namespace pdx {
namespace rpc {

// This library uses a subset, or profile, of MessagePack (http://msgpack.org)
// to encode supported data types during serialization and to verify the
// expected data types during deserialization. One notable deviation from the
// MessagePack specification is that little-endian byte order is used for
// multi-byte numeric types to avoid unnecessary conversion on nearly all
// relevant architectures.
//
// Some data types, integers for example, support multiple encoding strategies.
// This library attempts to optimize for space based on the value of such types.
// However, during decode all valid encodings for a given type are accepted.

// Prefix byte for type encodings. This is the complete list of prefix bytes
// from the MessagePack specification, even though not all types are used by
// this library.
enum EncodingPrefix {
  ENCODING_TYPE_POSITIVE_FIXINT = 0x00,
  ENCODING_TYPE_POSITIVE_FIXINT_MIN = 0x00,
  ENCODING_TYPE_POSITIVE_FIXINT_MAX = 0x7f,
  ENCODING_TYPE_POSITIVE_FIXINT_MASK = 0x7f,
  ENCODING_TYPE_FIXMAP = 0x80,
  ENCODING_TYPE_FIXMAP_MIN = 0x80,
  ENCODING_TYPE_FIXMAP_MAX = 0x8f,
  ENCODING_TYPE_FIXMAP_MASK = 0x0f,
  ENCODING_TYPE_FIXARRAY = 0x90,
  ENCODING_TYPE_FIXARRAY_MIN = 0x90,
  ENCODING_TYPE_FIXARRAY_MAX = 0x9f,
  ENCODING_TYPE_FIXARRAY_MASK = 0x0f,
  ENCODING_TYPE_FIXSTR = 0xa0,
  ENCODING_TYPE_FIXSTR_MIN = 0xa0,
  ENCODING_TYPE_FIXSTR_MAX = 0xbf,
  ENCODING_TYPE_FIXSTR_MASK = 0x1f,
  ENCODING_TYPE_NIL = 0xc0,
  ENCODING_TYPE_RESERVED = 0xc1,
  ENCODING_TYPE_FALSE = 0xc2,
  ENCODING_TYPE_TRUE = 0xc3,
  ENCODING_TYPE_BIN8 = 0xc4,
  ENCODING_TYPE_BIN16 = 0xc5,
  ENCODING_TYPE_BIN32 = 0xc6,
  ENCODING_TYPE_EXT8 = 0xc7,
  ENCODING_TYPE_EXT16 = 0xc8,
  ENCODING_TYPE_EXT32 = 0xc9,
  ENCODING_TYPE_FLOAT32 = 0xca,
  ENCODING_TYPE_FLOAT64 = 0xcb,
  ENCODING_TYPE_UINT8 = 0xcc,
  ENCODING_TYPE_UINT16 = 0xcd,
  ENCODING_TYPE_UINT32 = 0xce,
  ENCODING_TYPE_UINT64 = 0xcf,
  ENCODING_TYPE_INT8 = 0xd0,
  ENCODING_TYPE_INT16 = 0xd1,
  ENCODING_TYPE_INT32 = 0xd2,
  ENCODING_TYPE_INT64 = 0xd3,
  ENCODING_TYPE_FIXEXT1 = 0xd4,
  ENCODING_TYPE_FIXEXT2 = 0xd5,
  ENCODING_TYPE_FIXEXT4 = 0xd6,
  ENCODING_TYPE_FIXEXT8 = 0xd7,
  ENCODING_TYPE_FIXEXT16 = 0xd8,
  ENCODING_TYPE_STR8 = 0xd9,
  ENCODING_TYPE_STR16 = 0xda,
  ENCODING_TYPE_STR32 = 0xdb,
  ENCODING_TYPE_ARRAY16 = 0xdc,
  ENCODING_TYPE_ARRAY32 = 0xdd,
  ENCODING_TYPE_MAP16 = 0xde,
  ENCODING_TYPE_MAP32 = 0xdf,
  ENCODING_TYPE_NEGATIVE_FIXINT = 0xe0,
  ENCODING_TYPE_NEGATIVE_FIXINT_MIN = 0xe0,
  ENCODING_TYPE_NEGATIVE_FIXINT_MAX = 0xff,
};

// Base encoding classes grouping multi-strategy encodings.
enum EncodingClass {
  ENCODING_CLASS_BOOL,
  ENCODING_CLASS_NIL,
  ENCODING_CLASS_INT,
  ENCODING_CLASS_UINT,
  ENCODING_CLASS_FLOAT,
  ENCODING_CLASS_ARRAY,
  ENCODING_CLASS_MAP,
  ENCODING_CLASS_STRING,
  ENCODING_CLASS_BINARY,
  ENCODING_CLASS_EXTENSION,
};

// Encoding prefixes are unsigned bytes.
typedef std::uint8_t EncodingType;

// Extension encoding types defined by this library.
enum EncodingExtType : int8_t {
  ENCODING_EXT_TYPE_FILE_DESCRIPTOR,
  ENCODING_EXT_TYPE_CHANNEL_HANDLE,
};

// Encoding predicates. Determines whether the given encoding is of a specific
// type.
inline constexpr bool IsFixintEncoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
    case ENCODING_TYPE_NEGATIVE_FIXINT_MIN ... ENCODING_TYPE_NEGATIVE_FIXINT_MAX:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsUnsignedFixintEncoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsInt8Encoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
    case ENCODING_TYPE_NEGATIVE_FIXINT_MIN ... ENCODING_TYPE_NEGATIVE_FIXINT_MAX:
    case ENCODING_TYPE_INT8:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsUInt8Encoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
    case ENCODING_TYPE_UINT8:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsInt16Encoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
    case ENCODING_TYPE_NEGATIVE_FIXINT_MIN ... ENCODING_TYPE_NEGATIVE_FIXINT_MAX:
    case ENCODING_TYPE_INT8:
    case ENCODING_TYPE_INT16:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsUInt16Encoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
    case ENCODING_TYPE_UINT8:
    case ENCODING_TYPE_UINT16:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsInt32Encoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
    case ENCODING_TYPE_NEGATIVE_FIXINT_MIN ... ENCODING_TYPE_NEGATIVE_FIXINT_MAX:
    case ENCODING_TYPE_INT8:
    case ENCODING_TYPE_INT16:
    case ENCODING_TYPE_INT32:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsUInt32Encoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
    case ENCODING_TYPE_UINT8:
    case ENCODING_TYPE_UINT16:
    case ENCODING_TYPE_UINT32:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsInt64Encoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
    case ENCODING_TYPE_NEGATIVE_FIXINT_MIN ... ENCODING_TYPE_NEGATIVE_FIXINT_MAX:
    case ENCODING_TYPE_INT8:
    case ENCODING_TYPE_INT16:
    case ENCODING_TYPE_INT32:
    case ENCODING_TYPE_INT64:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsUInt64Encoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
    case ENCODING_TYPE_UINT8:
    case ENCODING_TYPE_UINT16:
    case ENCODING_TYPE_UINT32:
    case ENCODING_TYPE_UINT64:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsFixmapEncoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_FIXMAP_MIN ... ENCODING_TYPE_FIXMAP_MAX:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsFixarrayEncoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_FIXARRAY_MIN ... ENCODING_TYPE_FIXARRAY_MAX:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsFixstrEncoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_FIXSTR_MIN ... ENCODING_TYPE_FIXSTR_MAX:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsFixextEncoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_FIXEXT1:
    case ENCODING_TYPE_FIXEXT2:
    case ENCODING_TYPE_FIXEXT4:
    case ENCODING_TYPE_FIXEXT8:
    case ENCODING_TYPE_FIXEXT16:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsFloat32Encoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_FLOAT32:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsFloat64Encoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_FLOAT32:
    case ENCODING_TYPE_FLOAT64:
      return true;
    default:
      return false;
  }
}

inline constexpr bool IsBoolEncoding(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_FALSE:
    case ENCODING_TYPE_TRUE:
      return true;
    default:
      return false;
  }
}

inline constexpr std::size_t GetFixstrSize(EncodingType encoding) {
  return encoding & ENCODING_TYPE_FIXSTR_MASK;
}

inline constexpr std::size_t GetFixarraySize(EncodingType encoding) {
  return encoding & ENCODING_TYPE_FIXARRAY_MASK;
}

inline constexpr std::size_t GetFixmapSize(EncodingType encoding) {
  return encoding & ENCODING_TYPE_FIXMAP_MASK;
}

inline constexpr std::size_t GetFixextSize(EncodingType encoding) {
  switch (encoding) {
    case ENCODING_TYPE_FIXEXT1:
      return 1;
    case ENCODING_TYPE_FIXEXT2:
      return 2;
    case ENCODING_TYPE_FIXEXT4:
      return 4;
    case ENCODING_TYPE_FIXEXT8:
      return 8;
    case ENCODING_TYPE_FIXEXT16:
      return 16;
    default:
      return 0;  // Invalid fixext size.
  }
}

// Gets the size of the encoding in bytes, not including external payload data.
inline constexpr std::size_t GetEncodingSize(EncodingType encoding) {
  switch (encoding) {
    // Encoding is fully contained within the type value.
    case ENCODING_TYPE_POSITIVE_FIXINT_MIN ... ENCODING_TYPE_POSITIVE_FIXINT_MAX:
    case ENCODING_TYPE_NEGATIVE_FIXINT_MIN ... ENCODING_TYPE_NEGATIVE_FIXINT_MAX:
    case ENCODING_TYPE_FIXMAP_MIN ... ENCODING_TYPE_FIXMAP_MAX:
    case ENCODING_TYPE_FIXARRAY_MIN ... ENCODING_TYPE_FIXARRAY_MAX:
    case ENCODING_TYPE_FIXSTR_MIN ... ENCODING_TYPE_FIXSTR_MAX:
    case ENCODING_TYPE_NIL:
    case ENCODING_TYPE_RESERVED:
    case ENCODING_TYPE_FALSE:
    case ENCODING_TYPE_TRUE:
      return 1;

    // Encoding type followed by one-byte size or immediate value.
    case ENCODING_TYPE_BIN8:
    case ENCODING_TYPE_EXT8:
    case ENCODING_TYPE_UINT8:
    case ENCODING_TYPE_INT8:
    case ENCODING_TYPE_STR8:
    // Encoding type followed by one-byte extension type.
    case ENCODING_TYPE_FIXEXT1:
    case ENCODING_TYPE_FIXEXT2:
    case ENCODING_TYPE_FIXEXT4:
    case ENCODING_TYPE_FIXEXT8:
    case ENCODING_TYPE_FIXEXT16:
      return 2;

    // Encoding type followed by two-byte size or immediate value.
    case ENCODING_TYPE_BIN16:
    case ENCODING_TYPE_EXT16:
    case ENCODING_TYPE_UINT16:
    case ENCODING_TYPE_INT16:
    case ENCODING_TYPE_STR16:
    case ENCODING_TYPE_ARRAY16:
    case ENCODING_TYPE_MAP16:
      return 3;

    // Encoding type followed by four-byte size or immediate value.
    case ENCODING_TYPE_BIN32:
    case ENCODING_TYPE_EXT32:
    case ENCODING_TYPE_FLOAT32:
    case ENCODING_TYPE_UINT32:
    case ENCODING_TYPE_INT32:
    case ENCODING_TYPE_STR32:
    case ENCODING_TYPE_ARRAY32:
    case ENCODING_TYPE_MAP32:
      return 5;

    // Encoding type followed by eight-byte immediate value.
    case ENCODING_TYPE_FLOAT64:
    case ENCODING_TYPE_UINT64:
    case ENCODING_TYPE_INT64:
      return 9;

    default:
      return 0;
  }
}

// Encoding for standard types. Each supported data type has an associated
// encoding or set of encodings. These functions determine the MessagePack
// encoding based on the data type, value, and size of their arguments.

inline constexpr EncodingType EncodeArrayType(std::size_t size) {
  if (size < (1U << 4))
    return ENCODING_TYPE_FIXARRAY | (size & ENCODING_TYPE_FIXARRAY_MASK);
  else if (size < (1U << 16))
    return ENCODING_TYPE_ARRAY16;
  else
    return ENCODING_TYPE_ARRAY32;
}

inline constexpr EncodingType EncodeMapType(std::size_t size) {
  if (size < (1U << 4))
    return ENCODING_TYPE_FIXMAP | (size & ENCODING_TYPE_FIXMAP_MASK);
  else if (size < (1U << 16))
    return ENCODING_TYPE_MAP16;
  else
    return ENCODING_TYPE_MAP32;
}

inline constexpr EncodingType EncodeStringType(std::size_t size) {
  if (size < (1U << 5))
    return ENCODING_TYPE_FIXSTR | (size & ENCODING_TYPE_FIXSTR_MASK);
  else if (size < (1U << 8))
    return ENCODING_TYPE_STR8;
  else if (size < (1U << 16))
    return ENCODING_TYPE_STR16;
  else
    return ENCODING_TYPE_STR32;
}

inline constexpr EncodingType EncodeBinType(std::size_t size) {
  if (size < (1U << 8))
    return ENCODING_TYPE_BIN8;
  else if (size < (1U << 16))
    return ENCODING_TYPE_BIN16;
  else
    return ENCODING_TYPE_BIN32;
}

inline EncodingType EncodeType(const EmptyVariant& /*empty*/) {
  return ENCODING_TYPE_NIL;
}

// Variant is encoded as a single-element map, with the type index as the key.
template <typename... Types>
inline EncodingType EncodeType(const Variant<Types...>& /*variant*/) {
  return EncodeMapType(1);
}

template <typename T>
inline constexpr EncodingType EncodeType(const StringWrapper<T>& value) {
  return EncodeStringType(value.length());
}

inline constexpr EncodingType EncodeType(const std::string& value) {
  return EncodeStringType(value.length());
}

template <typename T, std::size_t Size>
inline constexpr EncodingType EncodeType(const std::array<T, Size>& /*value*/) {
  return EncodeArrayType(Size);
}

template <typename T>
inline constexpr EncodingType EncodeType(const ArrayWrapper<T>& value) {
  return EncodeArrayType(value.size());
}

template <typename T, typename Allocator>
inline constexpr EncodingType EncodeType(
    const std::vector<T, Allocator>& value) {
  return EncodeArrayType(value.size());
}

template <typename Key, typename T, typename Compare, typename Allocator>
inline constexpr EncodingType EncodeType(
    const std::map<Key, T, Compare, Allocator>& value) {
  return EncodeMapType(value.size());
}

template <typename Key, typename T, typename Hash, typename KeyEqual,
          typename Allocator>
inline constexpr EncodingType EncodeType(
    const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& value) {
  return EncodeMapType(value.size());
}

template <typename T>
inline constexpr EncodingType EncodeType(const BufferWrapper<T>& value) {
  // BIN size is in bytes.
  return EncodeBinType(value.size() *
                       sizeof(typename BufferWrapper<T>::value_type));
}

template <typename T, typename U>
inline constexpr EncodingType EncodeType(const std::pair<T, U>& /*value*/) {
  return EncodeArrayType(2);
}

template <typename... T>
inline constexpr EncodingType EncodeType(const std::tuple<T...>& /*value*/) {
  return EncodeArrayType(sizeof...(T));
}

// FileHandle is encoded as a FIXEXT2 with a type code for "FileDescriptor"
// and a signed 16-bit index into the pushed fd array. Empty file descriptor
// have an array index of -1.
template <FileHandleMode Mode>
inline constexpr EncodingType EncodeType(const FileHandle<Mode>& /*fd*/) {
  return ENCODING_TYPE_FIXEXT2;
}

// ChannelHandle is encoded as a FIXEXT4 with a type of
// ENCODING_EXT_TYPE_CHANNEL_HANDLE and a signed 32-bit value representing
// a client channel in a remote process. Empty handle has a value of -1.
template <ChannelHandleMode Mode>
inline constexpr EncodingType EncodeType(
    const ChannelHandle<Mode>& /*handle*/) {
  return ENCODING_TYPE_FIXEXT4;
}

inline constexpr EncodingType EncodeType(const bool& value) {
  return value ? ENCODING_TYPE_TRUE : ENCODING_TYPE_FALSE;
}

// Type 'char' is a little bit special in that it is distinct from 'signed char'
// and 'unsigned char'. Treating it as an unsigned 8-bit value is safe for
// encoding purposes and nicely handles 7-bit ASCII encodings as FIXINT.
inline constexpr EncodingType EncodeType(const char& value) {
  if (value < static_cast<char>(1 << 7))
    return value;
  else
    return ENCODING_TYPE_UINT8;
}

inline constexpr EncodingType EncodeType(const uint8_t& value) {
  if (value < (1U << 7))
    return value;
  else
    return ENCODING_TYPE_UINT8;
}
inline constexpr EncodingType EncodeType(const int8_t& value) {
  if (value >= -32)
    return value;
  else
    return ENCODING_TYPE_INT8;
}
inline constexpr EncodingType EncodeType(const uint16_t& value) {
  if (value < (1U << 7))
    return static_cast<EncodingType>(value);
  else if (value < (1U << 8))
    return ENCODING_TYPE_UINT8;
  else
    return ENCODING_TYPE_UINT16;
}
inline constexpr EncodingType EncodeType(const int16_t& value) {
  if (value >= -32 && value <= 127)
    return static_cast<EncodingType>(value);
  else if (value >= -128 && value <= 127)
    return ENCODING_TYPE_INT8;
  else
    return ENCODING_TYPE_INT16;
}
inline constexpr EncodingType EncodeType(const uint32_t& value) {
  if (value < (1U << 7))
    return static_cast<EncodingType>(value);
  else if (value < (1U << 8))
    return ENCODING_TYPE_UINT8;
  else if (value < (1U << 16))
    return ENCODING_TYPE_UINT16;
  else
    return ENCODING_TYPE_UINT32;
}
inline constexpr EncodingType EncodeType(const int32_t& value) {
  if (value >= -32 && value <= 127)
    return static_cast<EncodingType>(value);
  else if (value >= -128 && value <= 127)
    return ENCODING_TYPE_INT8;
  else if (value >= -32768 && value <= 32767)
    return ENCODING_TYPE_INT16;
  else
    return ENCODING_TYPE_INT32;
}
inline constexpr EncodingType EncodeType(const uint64_t& value) {
  if (value < (1ULL << 7))
    return static_cast<EncodingType>(value);
  else if (value < (1ULL << 8))
    return ENCODING_TYPE_UINT8;
  else if (value < (1ULL << 16))
    return ENCODING_TYPE_UINT16;
  else if (value < (1ULL << 32))
    return ENCODING_TYPE_UINT32;
  else
    return ENCODING_TYPE_UINT64;
}
inline constexpr EncodingType EncodeType(const int64_t& value) {
  if (value >= -32 && value <= 127)
    return static_cast<EncodingType>(value);
  else if (value >= -128 && value <= 127)  // Effectively [-128, -32).
    return ENCODING_TYPE_INT8;
  else if (value >= -32768 && value <= 32767)
    return ENCODING_TYPE_INT16;
  else if (value >= -2147483648 && value <= 2147483647)
    return ENCODING_TYPE_INT32;
  else
    return ENCODING_TYPE_INT64;
}

inline constexpr EncodingType EncodeType(const float& /*value*/) {
  return ENCODING_TYPE_FLOAT32;
}

inline constexpr EncodingType EncodeType(const double& /*value*/) {
  return ENCODING_TYPE_FLOAT64;
}

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_ENCODING_H_
