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

#include "pass_queue.h"

#include "file_utils.h"
#include "spirit.h"
#include "test_utils.h"
#include "transformer.h"
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

class AddToDivTransformer : public Transformer {
public:
  Instruction *transform(IAddInst *add) override {
    auto ret = new SDivInst(add->mResultType, add->mOperand1, add->mOperand2);
    ret->setId(add->getId());
    return ret;
  }
};

class AddMulAfterAddTransformer : public Transformer {
public:
  Instruction *transform(IAddInst *add) override {
    insert(add);
    auto ret = new IMulInst(add->mResultType, add, add);
    ret->setId(add->getId());
    return ret;
  }
};

class Deleter : public Transformer {
public:
  Instruction *transform(IMulInst *) override { return nullptr; }
};

class InPlaceModifyingPass : public Pass {
public:
  Module *run(Module *m, int *error) override {
    m->getFloatType(64);
    if (error) {
      *error = 0;
    }
    return m;
  }
};

} // annonymous namespace

class PassQueueTest : public ::testing::Test {
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

TEST_F(PassQueueTest, testMulToAdd) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m);

  EXPECT_EQ(1, countEntity<IAddInst>(m.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));

  PassQueue passes;
  passes.append(new MulToAddTransformer());
  auto m1 = passes.run(m.get());

  ASSERT_NE(nullptr, m1);

  ASSERT_TRUE(m1->resolveIds());

  EXPECT_EQ(2, countEntity<IAddInst>(m1));
  EXPECT_EQ(0, countEntity<IMulInst>(m1));
}

TEST_F(PassQueueTest, testInPlaceModifying) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m);

  EXPECT_EQ(1, countEntity<IAddInst>(m.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));
  EXPECT_EQ(1, countEntity<TypeFloatInst>(m.get()));

  PassQueue passes;
  passes.append(new InPlaceModifyingPass());
  auto m1 = passes.run(m.get());

  ASSERT_NE(nullptr, m1);

  ASSERT_TRUE(m1->resolveIds());

  EXPECT_EQ(1, countEntity<IAddInst>(m1));
  EXPECT_EQ(1, countEntity<IMulInst>(m1));
  EXPECT_EQ(2, countEntity<TypeFloatInst>(m1));
}

TEST_F(PassQueueTest, testDeletion) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m.get());

  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));

  PassQueue passes;
  passes.append(new Deleter());
  auto m1 = passes.run(m.get());

  // One of the ids from the input module is missing now.
  ASSERT_EQ(nullptr, m1);
}

TEST_F(PassQueueTest, testMulToAddToDiv) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m);

  EXPECT_EQ(1, countEntity<IAddInst>(m.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));

  PassQueue passes;
  passes.append(new MulToAddTransformer());
  passes.append(new AddToDivTransformer());
  auto m1 = passes.run(m.get());

  ASSERT_NE(nullptr, m1);

  ASSERT_TRUE(m1->resolveIds());

  EXPECT_EQ(0, countEntity<IAddInst>(m1));
  EXPECT_EQ(0, countEntity<IMulInst>(m1));
  EXPECT_EQ(2, countEntity<SDivInst>(m1));
}

TEST_F(PassQueueTest, testAMix) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m);

  EXPECT_EQ(1, countEntity<IAddInst>(m.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));
  EXPECT_EQ(0, countEntity<SDivInst>(m.get()));
  EXPECT_EQ(1, countEntity<TypeFloatInst>(m.get()));

  PassQueue passes;
  passes.append(new MulToAddTransformer());
  passes.append(new AddToDivTransformer());
  passes.append(new InPlaceModifyingPass());

  std::unique_ptr<Module> m1(passes.run(m.get()));

  ASSERT_NE(nullptr, m1);

  ASSERT_TRUE(m1->resolveIds());

  EXPECT_EQ(0, countEntity<IAddInst>(m1.get()));
  EXPECT_EQ(0, countEntity<IMulInst>(m1.get()));
  EXPECT_EQ(2, countEntity<SDivInst>(m1.get()));
  EXPECT_EQ(2, countEntity<TypeFloatInst>(m1.get()));
}

TEST_F(PassQueueTest, testAnotherMix) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m);

  EXPECT_EQ(1, countEntity<IAddInst>(m.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));
  EXPECT_EQ(0, countEntity<SDivInst>(m.get()));
  EXPECT_EQ(1, countEntity<TypeFloatInst>(m.get()));

  PassQueue passes;
  passes.append(new InPlaceModifyingPass());
  passes.append(new MulToAddTransformer());
  passes.append(new AddToDivTransformer());
  auto outputWords = passes.runAndSerialize(m.get());

  std::unique_ptr<Module> m1(Deserialize<Module>(outputWords));

  ASSERT_NE(nullptr, m1);

  ASSERT_TRUE(m1->resolveIds());

  EXPECT_EQ(0, countEntity<IAddInst>(m1.get()));
  EXPECT_EQ(0, countEntity<IMulInst>(m1.get()));
  EXPECT_EQ(2, countEntity<SDivInst>(m1.get()));
  EXPECT_EQ(2, countEntity<TypeFloatInst>(m1.get()));
}

TEST_F(PassQueueTest, testMulToAddToDivFromWords) {
  PassQueue passes;
  passes.append(new MulToAddTransformer());
  passes.append(new AddToDivTransformer());
  auto outputWords = passes.run(std::move(mWordsGreyscale));

  std::unique_ptr<Module> m1(Deserialize<Module>(outputWords));

  ASSERT_NE(nullptr, m1);

  ASSERT_TRUE(m1->resolveIds());

  EXPECT_EQ(0, countEntity<IAddInst>(m1.get()));
  EXPECT_EQ(0, countEntity<IMulInst>(m1.get()));
  EXPECT_EQ(2, countEntity<SDivInst>(m1.get()));
}

TEST_F(PassQueueTest, testMulToAddToDivToWords) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m);

  EXPECT_EQ(1, countEntity<IAddInst>(m.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));

  PassQueue passes;
  passes.append(new MulToAddTransformer());
  passes.append(new AddToDivTransformer());
  auto outputWords = passes.runAndSerialize(m.get());

  std::unique_ptr<Module> m1(Deserialize<Module>(outputWords));

  ASSERT_NE(nullptr, m1);

  ASSERT_TRUE(m1->resolveIds());

  EXPECT_EQ(0, countEntity<IAddInst>(m1.get()));
  EXPECT_EQ(0, countEntity<IMulInst>(m1.get()));
  EXPECT_EQ(2, countEntity<SDivInst>(m1.get()));
}

TEST_F(PassQueueTest, testAddMulAfterAdd) {
  std::unique_ptr<Module> m(Deserialize<Module>(mWordsGreyscale));

  ASSERT_NE(nullptr, m);

  EXPECT_EQ(1, countEntity<IAddInst>(m.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));

  constexpr int kNumMulToAdd = 100;

  PassQueue passes;
  for (int i = 0; i < kNumMulToAdd; i++) {
    passes.append(new AddMulAfterAddTransformer());
  }
  auto outputWords = passes.runAndSerialize(m.get());

  std::unique_ptr<Module> m1(Deserialize<Module>(outputWords));

  ASSERT_NE(nullptr, m1);

  ASSERT_TRUE(m1->resolveIds());

  EXPECT_EQ(1, countEntity<IAddInst>(m1.get()));
  EXPECT_EQ(1 + kNumMulToAdd, countEntity<IMulInst>(m1.get()));
}

} // namespace spirit
} // namespace android
