//===- implTest.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/LEB128.h"
#include "LEB128Test.h"

#include "mcld/Support/SystemUtils.h"
#include <ctime>
#include <cstdlib>

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
LEB128Test::LEB128Test() {
  // Initialize the seed for random number generator using during the tests.
  sys::SetRandomSeed(::time(NULL));
}

// Destructor can do clean-up work that doesn't throw exceptions here.
LEB128Test::~LEB128Test() {
}

// SetUp() will be called immediately before each test.
void LEB128Test::SetUp() {
}

// TearDown() will be called immediately after each test.
void LEB128Test::TearDown() {
}

//==========================================================================//
// Testcases
//

TEST_F(LEB128Test, EncodeULEB_Example_from_Dwarf3_Figure22_Using_32bits) {
  leb128::ByteType buffer[2];
  leb128::ByteType* result;
  size_t size;

  result = buffer;
  size = leb128::encode<uint32_t>(result, 2);
  ASSERT_EQ(buffer[0], 2);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<uint32_t>(result, 127);
  ASSERT_EQ(buffer[0], 127);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<uint32_t>(result, 128);
  ASSERT_EQ(buffer[0], 0 + 0x80);
  ASSERT_EQ(buffer[1], 1);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint32_t>(result, 129);
  ASSERT_EQ(buffer[0], 1 + 0x80);
  ASSERT_EQ(buffer[1], 1);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint32_t>(result, 130);
  ASSERT_EQ(buffer[0], 2 + 0x80);
  ASSERT_EQ(buffer[1], 1);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint32_t>(result, 12857);
  ASSERT_EQ(buffer[0], 57 + 0x80);
  ASSERT_EQ(buffer[1], 100);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);
}

TEST_F(LEB128Test, EncodeULEB_Example_from_Dwarf3_Figure22_Using_64bits) {
  leb128::ByteType buffer[2];
  leb128::ByteType* result;
  size_t size;

  result = buffer;
  size = leb128::encode<uint64_t>(result, 2);
  ASSERT_EQ(buffer[0], 2);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 127);
  ASSERT_EQ(buffer[0], 127);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 128);
  ASSERT_EQ(buffer[0], 0 + 0x80);
  ASSERT_EQ(buffer[1], 1);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 129);
  ASSERT_EQ(buffer[0], 1 + 0x80);
  ASSERT_EQ(buffer[1], 1);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 130);
  ASSERT_EQ(buffer[0], 2 + 0x80);
  ASSERT_EQ(buffer[1], 1);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 12857);
  ASSERT_EQ(buffer[0], 57 + 0x80);
  ASSERT_EQ(buffer[1], 100);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);
}

TEST_F(LEB128Test, EncodeSLEB_Example_from_Dwarf3_Figure22) {
  leb128::ByteType buffer[2];
  leb128::ByteType* result;
  size_t size;

  result = buffer;
  size = leb128::encode<int32_t>(result, 2);
  ASSERT_EQ(buffer[0], 2);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<int32_t>(result, -2);
  ASSERT_EQ(buffer[0], 0x7e);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<int32_t>(result, 127);
  ASSERT_EQ(buffer[0], 127 + 0x80);
  ASSERT_EQ(buffer[1], 0);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<int32_t>(result, -127);
  ASSERT_EQ(buffer[0], 1 + 0x80);
  ASSERT_EQ(buffer[1], 0x7f);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<int32_t>(result, 128);
  ASSERT_EQ(buffer[0], 0 + 0x80);
  ASSERT_EQ(buffer[1], 1);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<int32_t>(result, -128);
  ASSERT_EQ(buffer[0], 0 + 0x80);
  ASSERT_EQ(buffer[1], 0x7f);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<int32_t>(result, 129);
  ASSERT_EQ(buffer[0], 1 + 0x80);
  ASSERT_EQ(buffer[1], 1);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<int32_t>(result, -129);
  ASSERT_EQ(buffer[0], 0x7f + 0x80);
  ASSERT_EQ(buffer[1], 0x7e);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);
}

TEST_F(LEB128Test, DecodeULEB_Example_from_Dwarf3_Figure22) {
  leb128::ByteType buffer[2];
  size_t size;

  buffer[0] = 2;
  buffer[1] = 0;
  ASSERT_TRUE(leb128::decode<uint64_t>(buffer, size) == 2);
  ASSERT_TRUE(size == 1);

  buffer[0] = 127;
  buffer[1] = 0;
  ASSERT_TRUE(leb128::decode<uint64_t>(buffer, size) == 127);
  ASSERT_TRUE(size == 1);

  buffer[0] = 0 + 0x80;
  buffer[1] = 1;
  ASSERT_TRUE(leb128::decode<uint64_t>(buffer, size) == 128);
  ASSERT_TRUE(size == 2);

  buffer[0] = 1 + 0x80;
  buffer[1] = 1;
  ASSERT_TRUE(leb128::decode<uint64_t>(buffer, size) == 129);
  ASSERT_TRUE(size == 2);

  buffer[0] = 2 + 0x80;
  buffer[1] = 1;
  ASSERT_TRUE(leb128::decode<uint64_t>(buffer, size) == 130);
  ASSERT_TRUE(size == 2);

  buffer[0] = 57 + 0x80;
  buffer[1] = 100;
  ASSERT_TRUE(leb128::decode<uint64_t>(buffer, size) == 12857);
  ASSERT_TRUE(size == 2);
}

TEST_F(LEB128Test, DecodeSLEB_Example_from_Dwarf3_Figure22) {
  leb128::ByteType buffer[2];
  size_t size;

  buffer[0] = 2;
  buffer[1] = 0;
  ASSERT_EQ(leb128::decode<int64_t>(buffer, size), 2);
  ASSERT_TRUE(size == 1);

  buffer[0] = 0x7e;
  buffer[1] = 0;
  ASSERT_EQ(leb128::decode<int64_t>(buffer, size), -2);
  ASSERT_TRUE(size == 1);

  buffer[0] = 127 + 0x80;
  buffer[1] = 0;
  ASSERT_EQ(leb128::decode<int64_t>(buffer, size), 127);
  ASSERT_TRUE(size == 2);

  buffer[0] = 1 + 0x80;
  buffer[1] = 0x7f;
  ASSERT_EQ(leb128::decode<int64_t>(buffer, size), -127);
  ASSERT_TRUE(size == 2);

  buffer[0] = 0 + 0x80;
  buffer[1] = 1;
  ASSERT_EQ(leb128::decode<int64_t>(buffer, size), 128);
  ASSERT_TRUE(size == 2);

  buffer[0] = 0 + 0x80;
  buffer[1] = 0x7f;
  ASSERT_EQ(leb128::decode<int64_t>(buffer, size), -128);
  ASSERT_TRUE(size == 2);

  buffer[0] = 1 + 0x80;
  buffer[1] = 1;
  ASSERT_EQ(leb128::decode<int64_t>(buffer, size), 129);
  ASSERT_TRUE(size == 2);

  buffer[0] = 0x7f + 0x80;
  buffer[1] = 0x7e;
  ASSERT_EQ(leb128::decode<int64_t>(buffer, size), -129);
  ASSERT_TRUE(size == 2);
}

TEST_F(LEB128Test, DecodeULEB_Tests_Found_in_Android_dalvik_dx) {
  leb128::ByteType content[2];
  const leb128::ByteType* p;

  content[0] = 0;
  p = content;
  ASSERT_TRUE(leb128::decode<uint64_t>(p) == 0);
  ASSERT_EQ(p, content + 1);

  content[0] = 1;
  p = content;
  ASSERT_TRUE(leb128::decode<uint64_t>(p) == 1);
  ASSERT_EQ(p, content + 1);

  content[0] = 0x80;
  content[1] = 0x7f;
  p = content;
  ASSERT_TRUE(leb128::decode<uint64_t>(p) == 16256);
  ASSERT_EQ(p, content + 2);
}

TEST_F(LEB128Test, EncodeULEB_Tests_Found_in_Android_dalvik_dx) {
  leb128::ByteType buffer[5];
  leb128::ByteType* result;
  size_t size;

  result = buffer;
  size = leb128::encode<uint32_t>(result, 0);
  ASSERT_EQ(buffer[0], 0);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 0);
  ASSERT_EQ(buffer[0], 0);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<uint32_t>(result, 1);
  ASSERT_EQ(buffer[0], 1);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 1);
  ASSERT_EQ(buffer[0], 1);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<uint32_t>(result, 16256);
  ASSERT_EQ(buffer[0], 0x80);
  ASSERT_EQ(buffer[1], 0x7f);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 16256);
  ASSERT_EQ(buffer[0], 0x80);
  ASSERT_EQ(buffer[1], 0x7f);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint32_t>(result, 0x3b4);
  ASSERT_EQ(buffer[0], 0xb4);
  ASSERT_EQ(buffer[1], 0x07);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 0x3b4);
  ASSERT_EQ(buffer[0], 0xb4);
  ASSERT_EQ(buffer[1], 0x07);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint32_t>(result, 0x40c);
  ASSERT_EQ(buffer[0], 0x8c);
  ASSERT_EQ(buffer[1], 0x08);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 0x40c);
  ASSERT_EQ(buffer[0], 0x8c);
  ASSERT_EQ(buffer[1], 0x08);
  ASSERT_EQ(result, buffer + 2);
  ASSERT_TRUE(size == 2);

  result = buffer;
  size = leb128::encode<uint32_t>(result, 0xffffffff);
  ASSERT_EQ(buffer[0], 0xff);
  ASSERT_EQ(buffer[1], 0xff);
  ASSERT_EQ(buffer[2], 0xff);
  ASSERT_EQ(buffer[3], 0xff);
  ASSERT_EQ(buffer[4], 0xf);
  ASSERT_EQ(result, buffer + 5);
  ASSERT_TRUE(size == 5);

  result = buffer;
  size = leb128::encode<uint64_t>(result, 0xffffffff);
  ASSERT_EQ(buffer[0], 0xff);
  ASSERT_EQ(buffer[1], 0xff);
  ASSERT_EQ(buffer[2], 0xff);
  ASSERT_EQ(buffer[3], 0xff);
  ASSERT_EQ(buffer[4], 0xf);
  ASSERT_EQ(result, buffer + 5);
  ASSERT_TRUE(size == 5);
}

TEST_F(LEB128Test, DecodeSLEB_Tests_Found_in_Android_dalvik_dx) {
  leb128::ByteType content[2];
  const leb128::ByteType* p;

  content[0] = 0;
  p = content;
  ASSERT_EQ(leb128::decode<int64_t>(p), 0);
  ASSERT_EQ(p, content + 1);

  content[0] = 1;
  p = content;
  ASSERT_EQ(leb128::decode<int64_t>(p), 1);
  ASSERT_EQ(p, content + 1);

  content[0] = 0x7f;
  p = content;
  ASSERT_EQ(leb128::decode<int64_t>(p), -1);
  ASSERT_EQ(p, content + 1);

  content[0] = 0x3c;
  p = content;
  ASSERT_EQ(leb128::decode<int64_t>(p), 0x3c);
  ASSERT_EQ(p, content + 1);
}

TEST_F(LEB128Test, EncodeSLEB_Tests_Found_in_Android_dalvik_dx) {
  leb128::ByteType buffer[5];
  leb128::ByteType* result;
  size_t size;

  result = buffer;
  size = leb128::encode<int32_t>(result, 0);
  ASSERT_EQ(buffer[0], 0);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<int64_t>(result, 0);
  ASSERT_EQ(buffer[0], 0);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<int32_t>(result, 1);
  ASSERT_EQ(buffer[0], 1);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<int64_t>(result, 1);
  ASSERT_EQ(buffer[0], 1);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<int32_t>(result, -1);
  ASSERT_EQ(buffer[0], 0x7f);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);

  result = buffer;
  size = leb128::encode<int64_t>(result, -1);
  ASSERT_EQ(buffer[0], 0x7f);
  ASSERT_EQ(result, buffer + 1);
  ASSERT_TRUE(size == 1);
}

TEST_F(LEB128Test, Random_Regression_Test) {
  leb128::ByteType buffer[9];

  for (int i = 0; i < 20; i++) {
    unsigned long int value = sys::GetRandomNum();
    uint64_t value2 = value * value;
    int64_t value3 = value * value;
    leb128::ByteType* result;
    size_t encode_size, decode_size;

    // Test encode<uint32_t> and decode<uint64_t> on value
    result = buffer;
    encode_size = leb128::encode<uint32_t>(result, value);
    ASSERT_EQ(result, buffer + encode_size);
    ASSERT_EQ(leb128::decode<uint64_t>(buffer, decode_size), value);
    ASSERT_EQ(encode_size, decode_size);

    // Test encode<uint64_t> and decode<uint64_t> on (value * value)
    result = buffer;
    encode_size = leb128::encode<uint64_t>(result, value2);
    ASSERT_EQ(result, buffer + encode_size);
    ASSERT_EQ(leb128::decode<uint64_t>(buffer, decode_size), value2);
    ASSERT_EQ(encode_size, decode_size);

    // Test encode<uint64_t> and decode<uint64_t> on (value * value)
    result = buffer;
    encode_size = leb128::encode<int64_t>(result, value3);
    ASSERT_EQ(result, buffer + encode_size);
    ASSERT_EQ(leb128::decode<int64_t>(buffer, decode_size), value3);
    ASSERT_EQ(encode_size, decode_size);

    // Test encode<uint64_t> and decode<uint64_t> on -(value * value)
    result = buffer;
    encode_size = leb128::encode<int64_t>(result, -value3);
    ASSERT_EQ(result, buffer + encode_size);
    ASSERT_EQ(leb128::decode<int64_t>(buffer, decode_size), -value3);
    ASSERT_EQ(encode_size, decode_size);
  }
}

TEST_F(LEB128Test, Other_Test) {
  leb128::ByteType buffer[5];
  leb128::ByteType* result;
  size_t size;

  result = buffer;
  leb128::encode<uint64_t>(result, 154452);
  ASSERT_EQ(result, buffer + 3);
  ASSERT_EQ(buffer[0], 0xd4);
  ASSERT_EQ(buffer[1], 0xb6);
  ASSERT_EQ(buffer[2], 0x9);

  ASSERT_TRUE(leb128::decode<uint64_t>(buffer, size) == 154452);
  ASSERT_TRUE(size == 3);
}

TEST_F(LEB128Test, Type_Conversion_Test) {
  char buffer[5];
  char* result;
  size_t size;

  result = buffer;
  leb128::encode<uint64_t>(result, 154452);
  ASSERT_EQ(result, buffer + 3);
  ASSERT_EQ(buffer[0], '\xd4');
  ASSERT_EQ(buffer[1], '\xb6');
  ASSERT_EQ(buffer[2], '\x09');

  ASSERT_TRUE(leb128::decode<uint64_t>(buffer, size) == 154452);
  ASSERT_TRUE(size == 3);

  const char* p = buffer;
  ASSERT_TRUE(leb128::decode<uint64_t>(p) == 154452);
  ASSERT_TRUE(p == (buffer + 3));
}
