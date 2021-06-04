//===- LEB128.h -----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_LEB128_H_
#define MCLD_SUPPORT_LEB128_H_

#include <stdint.h>
#include <sys/types.h>

namespace mcld {

namespace leb128 {

typedef unsigned char ByteType;

/* Forward declarations */
template <typename IntType>
size_t encode(ByteType*& pBuf, IntType pValue);

template <typename IntType>
IntType decode(const ByteType* pBuf, size_t& pSize);

template <typename IntType>
IntType decode(const ByteType*& pBuf);

/*
 * Given an integer, this function returns the number of bytes required to
 * encode it in ULEB128 format.
 */
template <typename IntType>
size_t size(IntType pValue) {
  size_t size = 1;
  while (pValue > 0x80) {
    pValue >>= 7;
    ++size;
  }
  return size;
}

/*
 * Write an unsigned integer in ULEB128 to the given buffer. The client should
 * ensure there's enough space in the buffer to hold the result. Update the
 * given buffer pointer to the point just past the end of the write value and
 * return the number of bytes being written.
 */
template <>
size_t encode<uint64_t>(ByteType*& pBuf, uint64_t pValue);

template <>
size_t encode<uint32_t>(ByteType*& pBuf, uint32_t pValue);

/*
 * Encoding functions for signed LEB128.
 */
template <>
size_t encode<int64_t>(ByteType*& pBuf, int64_t pValue);

template <>
size_t encode<int32_t>(ByteType*& pBuf, int32_t pValue);

/*
 * Read an integer encoded in ULEB128 format from the given buffer. pSize will
 * contain the number of bytes used in the buffer to encode the returned
 * integer.
 */
template <>
uint64_t decode<uint64_t>(const ByteType* pBuf, size_t& pSize);

/*
 * Read an integer encoded in ULEB128 format from the given buffer. Update the
 * given buffer pointer to the point just past the end of the read value.
 */
template <>
uint64_t decode<uint64_t>(const ByteType*& pBuf);

/*
 * Decoding functions for signed LEB128.
 */
template <>
int64_t decode<int64_t>(const ByteType* pBuf, size_t& pSize);

template <>
int64_t decode<int64_t>(const ByteType*& pBuf);

/*
 * The functions below handle the signed byte stream. This helps the user to get
 * rid of annoying type conversions when using the LEB128 encoding/decoding APIs
 * defined above.
 */
template <typename IntType>
size_t encode(char*& pBuf, IntType pValue) {
  return encode<IntType>(reinterpret_cast<ByteType*&>(pBuf), pValue);
}

template <typename IntType>
IntType decode(const char* pBuf, size_t& pSize) {
  return decode<IntType>(reinterpret_cast<const ByteType*>(pBuf), pSize);
}

template <typename IntType>
IntType decode(const char*& pBuf) {
  return decode<IntType>(reinterpret_cast<const ByteType*&>(pBuf));
}

}  // namespace leb128
}  // namespace mcld

#endif  // MCLD_SUPPORT_LEB128_H_
