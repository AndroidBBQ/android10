/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "word_stream.h"

#include "gtest/gtest.h"

#include <vector>

namespace android {
namespace spirit {

TEST(WordStreamTest, testStringOutput1) {
  std::unique_ptr<OutputWordStream> OS(OutputWordStream::Create());
  *OS << "ABCDEFG";
  auto words = OS->getWords();
  const std::vector<uint8_t> bytes((uint8_t *)words.data(),
                                   (uint8_t *)(words.data() + words.size()));
  const std::vector<uint8_t> bytesExpected = {0x41, 0x42, 0x43, 0x44,
                                              0x45, 0x46, 0x47, 0x00};
  EXPECT_EQ(bytesExpected, bytes);
}

TEST(WordStreamTest, testStringOutput2) {
  std::unique_ptr<OutputWordStream> OS(OutputWordStream::Create());
  *OS << "GLSL.std.450";
  auto words = OS->getWords();
  const std::vector<uint8_t> bytes((uint8_t *)words.data(),
                                   (uint8_t *)(words.data() + words.size()));
  const std::vector<uint8_t> bytesExpected = {
      0x47, 0x4c, 0x53, 0x4c, 0x2e, 0x73, 0x74, 0x64,
      0x2e, 0x34, 0x35, 0x30, 0x00, 0x00, 0x00, 0x00};
  EXPECT_EQ(bytesExpected, bytes);
}

TEST(WordStreamTest, testStringInput1) {
  uint8_t bytes[] = {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x00};
  std::vector<uint32_t> words((uint32_t *)bytes,
                              (uint32_t *)(bytes + sizeof(bytes)));
  std::unique_ptr<InputWordStream> IS(InputWordStream::Create(words));
  std::string s;
  *IS >> &s;
  EXPECT_STREQ("ABCDEFG", s.c_str());
}

TEST(WordStreamTest, testStringInput2) {
  uint8_t bytes[] = {0x47, 0x4c, 0x53, 0x4c, 0x2e, 0x73, 0x74, 0x64,
                     0x2e, 0x34, 0x35, 0x30, 0x00, 0x00, 0x00, 0x00};
  std::vector<uint32_t> words((uint32_t *)bytes,
                              (uint32_t *)(bytes + sizeof(bytes)));
  std::unique_ptr<InputWordStream> IS(InputWordStream::Create(words));
  std::string s;
  *IS >> &s;
  EXPECT_STREQ("GLSL.std.450", s.c_str());
}

} // namespace spirit
} // namespace android
