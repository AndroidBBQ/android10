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

#include "Wrapper.h"
#include "bcinfo/MetadataExtractor.h"
#include "builder.h"
#include "file_utils.h"
#include "instructions.h"
#include "module.h"
#include "test_utils.h"
#include "gtest/gtest.h"

namespace android {
namespace spirit {

class WrapperTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    mWordsGreyscale = readWords("greyscale.spv");
    mWordsGreyscale2 = readWords("greyscale2.spv");
    mWordsInvert = readWords("invert.spv");
  }

  std::vector<uint32_t> mWordsGreyscale;
  std::vector<uint32_t> mWordsGreyscale2;
  std::vector<uint32_t> mWordsInvert;

private:
  std::vector<uint32_t> readWords(const char *testFile) {
    static const std::string testDataPath(
        "frameworks/rs/rsov/compiler/spirit/test_data/");
    const std::string &fullPath = getAbsolutePath(testDataPath + testFile);
    return readFile<uint32_t>(fullPath);
  }
};

TEST_F(WrapperTest, testAddBuffer) {
  Builder b;
  Module m(&b);
  auto elemType = m.getIntType(32);
  VariableInst *buffer = AddBuffer(elemType, 2, b, &m);
  ASSERT_NE(nullptr, buffer);

  GlobalSection *gs = m.getGlobalSection();

  EXPECT_EQ(1, countEntity<TypeRuntimeArrayInst>(gs));
  EXPECT_EQ(1, countEntity<TypeStructInst>(gs));
  EXPECT_EQ(1, countEntity<VariableInst>(gs));
}

TEST_F(WrapperTest, testAddWrapper1) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m);

  m->resolveIds();

  Builder b;
  m->setBuilder(&b);

  constexpr uint32_t sig =
      bcinfo::MD_SIG_Kernel | bcinfo::MD_SIG_In | bcinfo::MD_SIG_Out;

  EXPECT_FALSE(AddWrapper("foo", sig, 1, b, m.get()));

  EXPECT_TRUE(AddWrapper("greyscale(vf4;", sig, 1, b, m.get()));

  // The input already has an entry point
  EXPECT_EQ(2, countEntity<EntryPointDefinition>(m.get()));
}

TEST_F(WrapperTest, testAddWrapper2) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsInvert));

  ASSERT_NE(nullptr, m);

  m->resolveIds();

  Builder b;
  m->setBuilder(&b);

  uint32_t sig = bcinfo::MD_SIG_Kernel | bcinfo::MD_SIG_In | bcinfo::MD_SIG_Out;

  EXPECT_FALSE(AddWrapper("foo", sig, 1, b, m.get()));

  ASSERT_TRUE(AddWrapper("invert", sig, 1, b, m.get()));

  EXPECT_EQ(1, countEntity<EntryPointDefinition>(m.get()));
}

TEST_F(WrapperTest, testAddWrapperForRoot) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsInvert));

  ASSERT_NE(nullptr, m);

  Builder b;
  m->setBuilder(&b);

  bool success = AddWrapper("root", 0, 1, b, m.get());
  ASSERT_TRUE(success);
}

} // namespace spirit
} // namespace android
