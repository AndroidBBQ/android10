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

#include "module.h"

#include "file_utils.h"
#include "instructions.h"
#include "test_utils.h"
#include "gtest/gtest.h"

#include <fstream>
#include <memory>

namespace android {
namespace spirit {

class ModuleTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    mWordsGlobal = readWords("global.spv");
    mWordsGreyscale = readWords("greyscale.spv");
    mWordsGreyscale2 = readWords("greyscale2.spv");
    mWordsInvert = readWords("invert.spv");
  }

  std::vector<uint32_t> mWordsGlobal;
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

TEST_F(ModuleTest, testDeserialization1) {
  auto m = Deserialize<Module>(mWordsGreyscale);

  ASSERT_NE(nullptr, m);

  std::unique_ptr<Module> mDeleter(m);

  int count = 0;
  std::unique_ptr<IVisitor> v(
      CreateInstructionVisitor([&count](Instruction *) -> void { count++; }));
  v->visit(m);

  ASSERT_EQ(count, 123);

  // TODO:: checkCountEntity<Instruction>() does not work correctly
  //  EXPECT_TRUE(checkCountEntity<Instruction>(m, 123));
  EXPECT_EQ(5, countEntity<AccessChainInst>(m));
  EXPECT_EQ(2, countEntity<BitcastInst>(m));
  EXPECT_EQ(1, countEntity<CapabilityInst>(m));
  EXPECT_EQ(1, countEntity<CompositeConstructInst>(m));
  EXPECT_EQ(5, countEntity<ConstantInst>(m));
  EXPECT_EQ(1, countEntity<ConstantCompositeInst>(m));
  EXPECT_EQ(11, countEntity<DecorateInst>(m));
  EXPECT_EQ(1, countEntity<DotInst>(m));
  EXPECT_EQ(1, countEntity<EntryPointInst>(m));
  EXPECT_EQ(1, countEntity<ExecutionModeInst>(m));
  EXPECT_EQ(1, countEntity<ExtInstImportInst>(m));
  EXPECT_EQ(2, countEntity<FunctionInst>(m));
  EXPECT_EQ(1, countEntity<FunctionCallInst>(m));
  EXPECT_EQ(2, countEntity<FunctionEndInst>(m));
  EXPECT_EQ(1, countEntity<FunctionParameterInst>(m));
  EXPECT_EQ(1, countEntity<IAddInst>(m));
  EXPECT_EQ(1, countEntity<IMulInst>(m));
  EXPECT_EQ(1, countEntity<ImageInst>(m));
  EXPECT_EQ(1, countEntity<ImageFetchInst>(m));
  EXPECT_EQ(2, countEntity<LabelInst>(m));
  EXPECT_EQ(11, countEntity<LoadInst>(m));
  EXPECT_EQ(4, countEntity<MemberDecorateInst>(m));
  EXPECT_EQ(4, countEntity<MemberNameInst>(m));
  EXPECT_EQ(1, countEntity<MemoryModelInst>(m));
  EXPECT_EQ(14, countEntity<NameInst>(m));
  EXPECT_EQ(1, countEntity<ReturnInst>(m));
  EXPECT_EQ(1, countEntity<ReturnValueInst>(m));
  EXPECT_EQ(1, countEntity<SourceInst>(m));
  EXPECT_EQ(3, countEntity<SourceExtensionInst>(m));
  EXPECT_EQ(6, countEntity<StoreInst>(m));
  EXPECT_EQ(1, countEntity<TypeFloatInst>(m));
  EXPECT_EQ(2, countEntity<TypeFunctionInst>(m));
  EXPECT_EQ(1, countEntity<TypeImageInst>(m));
  EXPECT_EQ(2, countEntity<TypeIntInst>(m));
  EXPECT_EQ(10, countEntity<TypePointerInst>(m));
  EXPECT_EQ(1, countEntity<TypeRuntimeArrayInst>(m));
  EXPECT_EQ(1, countEntity<TypeSampledImageInst>(m));
  EXPECT_EQ(2, countEntity<TypeStructInst>(m));
  EXPECT_EQ(4, countEntity<TypeVectorInst>(m));
  EXPECT_EQ(1, countEntity<TypeVoidInst>(m));
  EXPECT_EQ(9, countEntity<VariableInst>(m));
  EXPECT_EQ(1, countEntity<VectorShuffleInst>(m));
  EXPECT_EQ(1, countEntity<EntryPointDefinition>(m));
  EXPECT_EQ(1, countEntity<DebugInfoSection>(m));
  EXPECT_EQ(1, countEntity<GlobalSection>(m));
  EXPECT_EQ(2, countEntity<FunctionDefinition>(m));
}

TEST_F(ModuleTest, testDeserialization2) {
  Module *m = Deserialize<Module>(mWordsInvert);
  ASSERT_NE(nullptr, m);

  std::unique_ptr<Module> mDeleter(m);

  auto outwords = Serialize<Module>(m);

  EXPECT_TRUE(mWordsInvert == outwords);
}

TEST_F(ModuleTest, testSerialization1) {
  Module *m = Deserialize<Module>(mWordsGreyscale);
  ASSERT_NE(nullptr, m);

  std::unique_ptr<Module> mDeleter(m);

  EXPECT_EQ(2, countEntity<FunctionDefinition>(m));

  auto outwords = Serialize<Module>(m);

  EXPECT_TRUE(mWordsGreyscale == outwords);
}

TEST_F(ModuleTest, testSerialization2) {
  Module *m = Deserialize<Module>(mWordsGreyscale2);
  ASSERT_NE(nullptr, m);

  std::unique_ptr<Module> mDeleter(m);

  EXPECT_EQ(1, countEntity<FunctionDefinition>(m));

  auto outwords = Serialize<Module>(m);

  EXPECT_TRUE(mWordsGreyscale2 == outwords);
}

TEST_F(ModuleTest, testLookupByName) {
  Module *m = Deserialize<Module>(mWordsGreyscale);

  ASSERT_NE(nullptr, m);

  std::unique_ptr<Module> mDeleter(m);

  m->resolveIds();

  Instruction *mainFunc = m->lookupByName("main");

  EXPECT_NE(nullptr, mainFunc);
  EXPECT_STREQ("main", m->lookupNameByInstruction(mainFunc));

  auto i = static_cast<FunctionInst *>(m->lookupByName("greyscale(vf4;"));
  ASSERT_NE(nullptr, i);

  auto kernel = m->getFunctionDefinitionFromInstruction(i);
  ASSERT_NE(nullptr, kernel);

  EXPECT_NE(nullptr, kernel->getParameter(0));
  EXPECT_NE(nullptr, kernel->getReturnType());

  EXPECT_NE(nullptr, m->lookupFunctionDefinitionByName("greyscale(vf4;"));
}

TEST_F(ModuleTest, testGetSize) {
  std::unique_ptr<Module> m(new Module());
  EXPECT_EQ(4UL, m->getSize(m->getIntType(32)));
  EXPECT_EQ(4UL, m->getSize(m->getIntType(32, 0)));
  EXPECT_EQ(4UL, m->getSize(m->getFloatType(32)));
  EXPECT_EQ(16UL, m->getSize(m->getVectorType(m->getFloatType(32), 4)));
}

TEST_F(ModuleTest, testFindStringOfPrefix) {
  Module *m = Deserialize<Module>(mWordsGlobal);

  ASSERT_NE(nullptr, m);

  std::unique_ptr<Module> mDeleter(m);
  ASSERT_STREQ(".rsov.ExportedVars:0;",
               m->findStringOfPrefix(".rsov.ExportedVars:").c_str());

}

} // namespace spirit
} // namespace android
