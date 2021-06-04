//===- LEB128.cpp ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/LEB128.h"

namespace mcld {

namespace leb128 {

//===---------------------- LEB128 Encoding APIs -------------------------===//
template <>
size_t encode<uint64_t>(ByteType*& pBuf, uint64_t pValue) {
  size_t size = 0;
  do {
    ByteType byte = pValue & 0x7f;
    pValue >>= 7;
    if (pValue)
      byte |= 0x80;
    *pBuf++ = byte;
    size++;
  } while (pValue);

  return size;
}

/*
 * Fast version for encoding 32-bit integer. This unrolls the loop in the
 * generic version defined above.
 */
template <>
size_t encode<uint32_t>(ByteType*& pBuf, uint32_t pValue) {
  if ((pValue & ~0x7f) == 0) {
    *pBuf++ = static_cast<ByteType>(pValue);
    return 1;
  } else if ((pValue & ~0x3fff) == 0) {
    *pBuf++ = static_cast<ByteType>((pValue & 0x7f) | 0x80);
    *pBuf++ = static_cast<ByteType>((pValue >> 7) & 0x7f);
    return 2;
  } else if ((pValue & ~0x1fffff) == 0) {
    *pBuf++ = static_cast<ByteType>((pValue & 0x7f) | 0x80);
    *pBuf++ = static_cast<ByteType>(((pValue >> 7) & 0x7f) | 0x80);
    *pBuf++ = static_cast<ByteType>((pValue >> 14) & 0x7f);
    return 3;
  } else if ((pValue & ~0xfffffff) == 0) {
    *pBuf++ = static_cast<ByteType>((pValue & 0x7f) | 0x80);
    *pBuf++ = static_cast<ByteType>(((pValue >> 7) & 0x7f) | 0x80);
    *pBuf++ = static_cast<ByteType>(((pValue >> 14) & 0x7f) | 0x80);
    *pBuf++ = static_cast<ByteType>((pValue >> 21) & 0x7f);
    return 4;
  } else {
    *pBuf++ = static_cast<ByteType>((pValue & 0x7f) | 0x80);
    *pBuf++ = static_cast<ByteType>(((pValue >> 7) & 0x7f) | 0x80);
    *pBuf++ = static_cast<ByteType>(((pValue >> 14) & 0x7f) | 0x80);
    *pBuf++ = static_cast<ByteType>(((pValue >> 21) & 0x7f) | 0x80);
    *pBuf++ = static_cast<ByteType>((pValue >> 28) & 0x7f);
    return 5;
  }
  // unreachable
}

template <>
size_t encode<int64_t>(ByteType*& pBuf, int64_t pValue) {
  size_t size = 0;
  bool more = true;

  do {
    ByteType byte = pValue & 0x7f;
    pValue >>= 7;

    if (((pValue == 0) && ((byte & 0x40) == 0)) ||
        ((pValue == -1) && ((byte & 0x40) == 0x40)))
      more = false;
    else
      byte |= 0x80;

    *pBuf++ = byte;
    size++;
  } while (more);

  return size;
}

template <>
size_t encode<int32_t>(ByteType*& pBuf, int32_t pValue) {
  return encode<int64_t>(pBuf, static_cast<int64_t>(pValue));
}

//===---------------------- LEB128 Decoding APIs -------------------------===//

template <>
uint64_t decode<uint64_t>(const ByteType* pBuf, size_t& pSize) {
  uint64_t result = 0;

  if ((*pBuf & 0x80) == 0) {
    pSize = 1;
    return *pBuf;
  } else if ((*(pBuf + 1) & 0x80) == 0) {
    pSize = 2;
    return ((*(pBuf + 1) & 0x7f) << 7) | (*pBuf & 0x7f);
  } else if ((*(pBuf + 2) & 0x80) == 0) {
    pSize = 3;
    return ((*(pBuf + 2) & 0x7f) << 14) | ((*(pBuf + 1) & 0x7f) << 7) |
           (*pBuf & 0x7f);
  } else {
    pSize = 4;
    result = ((*(pBuf + 3) & 0x7f) << 21) | ((*(pBuf + 2) & 0x7f) << 14) |
             ((*(pBuf + 1) & 0x7f) << 7) | (*pBuf & 0x7f);
  }

  if ((*(pBuf + 3) & 0x80) != 0) {
    // Large number which is an unusual case.
    unsigned shift;
    ByteType byte;

    // Start the read from the 4th byte.
    shift = 28;
    pBuf += 4;
    do {
      byte = *pBuf;
      pBuf++;
      pSize++;
      result |= (static_cast<uint64_t>(byte & 0x7f) << shift);
      shift += 7;
    } while (byte & 0x80);
  }

  return result;
}

template <>
uint64_t decode<uint64_t>(const ByteType*& pBuf) {
  ByteType byte;
  uint64_t result;

  byte = *pBuf++;
  result = byte & 0x7f;
  if ((byte & 0x80) == 0) {
    return result;
  } else {
    byte = *pBuf++;
    result |= ((byte & 0x7f) << 7);
    if ((byte & 0x80) == 0) {
      return result;
    } else {
      byte = *pBuf++;
      result |= (byte & 0x7f) << 14;
      if ((byte & 0x80) == 0) {
        return result;
      } else {
        byte = *pBuf++;
        result |= (byte & 0x7f) << 21;
        if ((byte & 0x80) == 0) {
          return result;
        }
      }
    }
  }

  // Large number which is an unusual case.
  unsigned shift;

  // Start the read from the 4th byte.
  shift = 28;
  do {
    byte = *pBuf++;
    result |= (static_cast<uint64_t>(byte & 0x7f) << shift);
    shift += 7;
  } while (byte & 0x80);

  return result;
}

/*
 * Signed LEB128 decoding is Similar to the unsigned version but setup the sign
 * bit if necessary. This is rarely used, therefore we don't provide unrolling
 * version like decode() to save the code size.
 */
template <>
int64_t decode<int64_t>(const ByteType* pBuf, size_t& pSize) {
  uint64_t result = 0;
  ByteType byte;
  unsigned shift = 0;

  pSize = 0;
  do {
    byte = *pBuf;
    pBuf++;
    pSize++;
    result |= (static_cast<uint64_t>(byte & 0x7f) << shift);
    shift += 7;
  } while (byte & 0x80);

  if ((shift < (8 * sizeof(result))) && (byte & 0x40))
    result |= ((static_cast<uint64_t>(-1)) << shift);

  return result;
}

template <>
int64_t decode<int64_t>(const ByteType*& pBuf) {
  uint64_t result = 0;
  ByteType byte;
  unsigned shift = 0;

  do {
    byte = *pBuf;
    pBuf++;
    result |= (static_cast<uint64_t>(byte & 0x7f) << shift);
    shift += 7;
  } while (byte & 0x80);

  if ((shift < (8 * sizeof(result))) && (byte & 0x40))
    result |= ((static_cast<uint64_t>(-1)) << shift);

  return result;
}

}  // namespace leb128
}  // namespace mcld
