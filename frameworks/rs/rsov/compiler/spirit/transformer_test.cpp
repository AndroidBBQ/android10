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

#include "transformer.h"

#include "file_utils.h"
#include "spirit.h"
#include "test_utils.h"
#include "gtest/gtest.h"

#include <stdint.h>

namespace android {
namespace spirit {

namespace {

class MulToAddTransformer : public Transformer {
public:
  Instruction *transform(IMulInst *mul) override {
    auto ret = new IAddInst(mul->mResultType, mul->mOperand1, mul->mOperand2);
    ret->setId(mul->getId());
    return ret;
  }
};

class Deleter : public Transformer {
public:
  Instruction *transform(IMulInst *) override { return nullptr; }
};

class NewDataTypeTransformer : public Transformer {
public:
  Instruction *transform(IMulInst *mul) override {
    insert(mul);
    auto *DoubleTy = getModule()->getFloatType(64);
    ConstantInst *ConstDouble2 = getModule()->getConstant(DoubleTy, 2.0);
    auto ret = new IAddInst(DoubleTy, mul, ConstDouble2);

    IdResult id = ret->getId();
    ret->setId(mul->getId());
    mul->setId(id);

    return ret;
  }
};

} // annonymous namespace

class TransformerTest : public ::testing::Test {
protected:
  virtual void SetUp() { mWordsGreyscale = readWords("greyscale.spv"); }

  std::vector<uint32_t> mWordsGreyscale;

private:
  std::vector<uint32_t> readWords(const char *testFile) {
    static const std::string testDataPath(
        "frameworks/rs/rsov/compiler/spirit/test_data/");
    const std::string &fullPath = getAbsolutePath(testDataPath + testFile);
    return readFile<uint32_t>(fullPath);
  }
};

TEST_F(TransformerTest, testMulToAdd) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m);

  EXPECT_EQ(1, countEntity<IAddInst>(m.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));

  MulToAddTransformer trans;
  std::unique_ptr<Module> m1(trans.run(m.get()));

  ASSERT_NE(nullptr, m1);

  ASSERT_TRUE(m1->resolveIds());

  EXPECT_EQ(2, countEntity<IAddInst>(m1.get()));
  EXPECT_EQ(0, countEntity<IMulInst>(m1.get()));
}

TEST_F(TransformerTest, testDeletion) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m.get());

  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));

  Deleter trans;
  std::unique_ptr<Module> m1(trans.run(m.get()));

  ASSERT_NE(nullptr, m1.get());

  EXPECT_EQ(1, countEntity<IAddInst>(m1.get()));
  EXPECT_EQ(0, countEntity<IMulInst>(m1.get()));
}

TEST_F(TransformerTest, testAddInstructionUsingNewDataType) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m.get());

  EXPECT_EQ(5, countEntity<ConstantInst>(m.get()));
  EXPECT_EQ(1, countEntity<TypeFloatInst>(m.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));

  NewDataTypeTransformer trans;
  std::unique_ptr<Module> m1(trans.run(m.get()));

  ASSERT_NE(nullptr, m1.get());

  EXPECT_EQ(6, countEntity<ConstantInst>(m.get()));
  EXPECT_EQ(2, countEntity<TypeFloatInst>(m1.get()));
  EXPECT_EQ(2, countEntity<IAddInst>(m1.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m1.get()));
}

} // namespace spirit
} // namespace android
