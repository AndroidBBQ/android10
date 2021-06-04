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

#include "instructions.h"

#include "gtest/gtest.h"

#include <memory>
#include <vector>

namespace android {
namespace spirit {

TEST(InstructionTest, testOpCapability) {
  std::vector<uint32_t> words = {0x00020011, 0x00000001};
  auto *i = Deserialize<CapabilityInst>(words);
  EXPECT_NE(nullptr, i);
}

TEST(InstructionTest, testOpExtension) {
  uint8_t bytes[] = {0x0a, 0x00, 0x03, 0x00, 0x41, 0x42,
                     0x43, 0x44, 0x45, 0x46, 'G',  0x00};
  std::vector<uint32_t> words((uint32_t *)bytes,
                              (uint32_t *)(bytes + sizeof(bytes)));
  auto *i = Deserialize<ExtensionInst>(words);
  ASSERT_NE(nullptr, i);
  EXPECT_STREQ("ABCDEFG", i->mOperand1.c_str());
}

TEST(InstructionTest, testOpExtInstImport) {
  uint8_t bytes[] = {0x0b, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
                     0x47, 0x4c, 0x53, 0x4c, 0x2e, 0x73, 0x74, 0x64,
                     0x2e, 0x34, 0x35, 0x30, 0x00, 0x00, 0x00, 0x00};
  std::vector<uint32_t> words((uint32_t *)bytes,
                              (uint32_t *)(bytes + sizeof(bytes)));
  auto *i = Deserialize<ExtInstImportInst>(words);
  ASSERT_NE(nullptr, i);
  EXPECT_STREQ("GLSL.std.450", i->mOperand1.c_str());
}

} // namespace spirit
} // namespace android
