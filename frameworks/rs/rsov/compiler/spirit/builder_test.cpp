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

#include "builder.h"

#include "file_utils.h"
#include "instructions.h"
#include "module.h"
#include "test_utils.h"
#include "gtest/gtest.h"

namespace android {
namespace spirit {

TEST(BuilderTest, testBuildAndSerialize) {
  Builder b;

  Module *m = b.MakeModule();

  ASSERT_NE(nullptr, m);

  m->addCapability(Capability::Shader);
  m->addCapability(Capability::Addresses);
  m->setMemoryModel(AddressingModel::Physical32, MemoryModel::GLSL450);

  m->addExtInstImport("GLSL.std.450");

  // Shall we explicitly create the debug info section first?
  m->addSource(SourceLanguage::GLSL, 450);
  m->addSourceExtension("GL_ARB_separate_shader_objects");
  m->addSourceExtension("GL_ARB_shading_language_420pack");
  m->addSourceExtension("GL_GOOGLE_cpp_style_line_directive");
  m->addSourceExtension("GL_GOOGLE_include_directive");
  m->addString("Foo Bar Baz");

  auto FloatTy = m->getFloatType(32);
  auto VF4Ty = m->getVectorType(FloatTy, 4);
  auto ArrTy = m->getRuntimeArrayType(VF4Ty);
  ArrTy->decorate(Decoration::ArrayStride)->addExtraOperand(16);
  auto StructTy = m->getStructType(ArrTy);
  StructTy->decorate(Decoration::BufferBlock);
  StructTy->memberDecorate(0, Decoration::Offset)->addExtraOperand(0);

  auto StructPtrTy = m->getPointerType(StorageClass::Uniform, StructTy);

  auto InputBuffer = b.MakeVariable(StructPtrTy, StorageClass::Uniform);
  InputBuffer->decorate(Decoration::DescriptorSet)->addExtraOperand(0);
  InputBuffer->decorate(Decoration::Binding)->addExtraOperand(2);
  m->addVariable(InputBuffer);

  auto ArrTy2 = m->getRuntimeArrayType(VF4Ty);
  ArrTy2->decorate(Decoration::ArrayStride)->addExtraOperand(16);

  auto StructTy2 = m->getStructType(ArrTy2);
  StructTy2->decorate(Decoration::BufferBlock);
  StructTy2->memberDecorate(0, Decoration::Offset)->addExtraOperand(0);

  auto StructPtrTy2 = m->getPointerType(StorageClass::Uniform, StructTy2);
  auto OutputBuffer = b.MakeVariable(StructPtrTy2, StorageClass::Uniform);
  OutputBuffer->decorate(Decoration::DescriptorSet)->addExtraOperand(0);
  OutputBuffer->decorate(Decoration::Binding)->addExtraOperand(1);
  m->addVariable(OutputBuffer);

  auto VoidTy = m->getVoidType();
  auto FuncTy = m->getFunctionType(VoidTy, nullptr, 0);

  auto UIntTy = m->getUnsignedIntType(32);
  auto V3UIntTy = m->getVectorType(UIntTy, 3);

  auto InvocationID = m->getInvocationId();

  auto ConstOne = m->getConstant(UIntTy, 1);
#if 0
  auto V3UIntPtrTy = m->getPointerType(StorageClass::Input, V3UIntTy);
  auto GSize = b.MakeVariable(V3UIntPtrTy, StorageClass::Input);
  GSize->decorate(Decoration::BuiltIn)->addExtraOperand(BuiltIn::WorkgroupSize);
  m->addVariable(GSize);
#endif

  auto GNum = m->getNumWorkgroups();

  const char *funcName = "invert";

  auto Func = b.MakeFunctionDefinition(VoidTy, FunctionControl::None, FuncTy);
  // TODO: Add method setName() to class FunctionDefinition
  // Func->setName(funcName);  // I.e., OpName %func funcName
  m->addFunctionDefinition(Func);

  auto Blk = b.MakeBlock();
  Func->addBlock(Blk);

  Blk->addInstruction(b.MakeLabel());

  auto ConstZero = m->getConstant(UIntTy, 0);
  auto UIntPtrTy = m->getPointerType(StorageClass::Input, UIntTy);

  auto IID = b.MakeLoad(V3UIntTy, InvocationID);
  Blk->addInstruction(IID);

  auto XValue = b.MakeCompositeExtract(UIntTy, IID, {0});
  Blk->addInstruction(XValue);

  auto YValue = b.MakeCompositeExtract(UIntTy, IID, {1});
  Blk->addInstruction(YValue);

#if 0
  auto PtrToGSizeX = b.MakeAccessChain(UIntPtrTy, GSize, {ConstZero});
  Blk->addInstruction(PtrToGSizeX);

  auto GSizeX = b.MakeLoad(UIntTy, PtrToGSizeX);
  Blk->addInstruction(GSizeX);
#else
  auto &GSizeX = ConstOne;
#endif

  auto Tmp1 = b.MakeIMul(UIntTy, YValue, GSizeX);
  Blk->addInstruction(Tmp1);

  auto PtrToGNumX = b.MakeAccessChain(UIntPtrTy, GNum, {ConstZero});
  Blk->addInstruction(PtrToGNumX);

  auto GNumX = b.MakeLoad(UIntTy, PtrToGNumX);
  Blk->addInstruction(GNumX);

  auto OffsetY = b.MakeIMul(UIntTy, Tmp1, GNumX);
  Blk->addInstruction(OffsetY);

  auto Index = b.MakeIAdd(UIntTy, OffsetY, XValue);
  Blk->addInstruction(Index);

  auto VF4PtrTy = m->getPointerType(StorageClass::Function, VF4Ty);
  auto Ptr = b.MakeAccessChain(VF4PtrTy, InputBuffer, {ConstZero, Index});
  Blk->addInstruction(Ptr);

  auto Value = b.MakeLoad(VF4Ty, Ptr);
  Blk->addInstruction(Value);

  // Here is the place to do something about the value. As is, this is a copy
  // kernel.
  auto ConstOneF = m->getConstant(FloatTy, 1.0f);
  auto ConstOneVF4 = m->getConstantComposite(VF4Ty, ConstOneF, ConstOneF,
                                             ConstOneF, ConstOneF);
  auto Result = b.MakeFSub(VF4Ty, ConstOneVF4, Value);
  Blk->addInstruction(Result);

  auto OutPtr = b.MakeAccessChain(VF4PtrTy, OutputBuffer, {ConstZero, Index});
  Blk->addInstruction(OutPtr);

  Blk->addInstruction(b.MakeStore(OutPtr, Result));
  Blk->addInstruction(b.MakeReturn());

  m->addEntryPoint(
      b.MakeEntryPointDefinition(ExecutionModel::GLCompute, Func, funcName)
          ->addToInterface(InvocationID)
          ->addToInterface(GNum)
          //          ->addToInterface(GSize)
          ->setLocalSize(1, 1, 1));

  EXPECT_EQ(1, countEntity<MemoryModelInst>(m));
  EXPECT_EQ(1, countEntity<EntryPointInst>(m));
  EXPECT_EQ(3, countEntity<LoadInst>(m));
  EXPECT_EQ(1, countEntity<ReturnInst>(m));
  EXPECT_EQ(1, countEntity<ExecutionModeInst>(m));
  EXPECT_EQ(2, countEntity<TypeRuntimeArrayInst>(m));
  EXPECT_EQ(2, countEntity<TypeStructInst>(m));
  EXPECT_EQ(5, countEntity<TypePointerInst>(m));
  EXPECT_EQ(1, countEntity<StringInst>(m));

  m->consolidateAnnotations();

  auto words = Serialize<Module>(m);

  auto m1 = Deserialize<Module>(words);
  ASSERT_NE(nullptr, m1);

  auto words1 = Serialize<Module>(m1);

  EXPECT_TRUE(words == words1);
}

TEST(BuilderTest, testLoadAndModify) {
  static const std::string testDataPath(
      "frameworks/rs/rsov/compiler/spirit/test_data/");
  const std::string &fullPath = getAbsolutePath(testDataPath + "greyscale.spv");

  Module *m = Deserialize<Module>(readFile<uint32_t>(fullPath.c_str()));

  ASSERT_NE(nullptr, m);

  std::unique_ptr<Module> mDeleter(m);

  Builder b;

  auto IntTy = m->getIntType(32);
  auto FuncTy = m->getFunctionType(IntTy, {IntTy});

  auto Func = b.MakeFunctionDefinition(IntTy, FunctionControl::None, FuncTy);
  // TODO: Add method setName() to class FunctionDefinition
  // Func->setName(funcName);  // I.e., OpName %func funcName
  m->addFunctionDefinition(Func);

  auto Blk = b.MakeBlock();
  Func->addBlock(Blk);

  Blk->addInstruction(b.MakeReturn());

  m->consolidateAnnotations();

  auto words = Serialize<Module>(m);

  auto m1 = Deserialize<Module>(words);
  ASSERT_NE(nullptr, m1);

  auto words1 = Serialize<Module>(m1);

  EXPECT_TRUE(words == words1);
}

} // namespace spirit
} // namespace android
