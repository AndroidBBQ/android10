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

#include "llvm/IR/Module.h"

#include "Builtin.h"
#include "Context.h"
#include "GlobalAllocSPIRITPass.h"
#include "RSAllocationUtils.h"
#include "bcinfo/MetadataExtractor.h"
#include "builder.h"
#include "instructions.h"
#include "module.h"
#include "pass.h"

#include <sstream>
#include <vector>

using bcinfo::MetadataExtractor;

namespace android {
namespace spirit {

VariableInst *AddBuffer(Instruction *elementType, uint32_t binding, Builder &b,
                        Module *m) {
  auto ArrTy = m->getRuntimeArrayType(elementType);
  const size_t stride = m->getSize(elementType);
  ArrTy->decorate(Decoration::ArrayStride)->addExtraOperand(stride);
  auto StructTy = m->getStructType(ArrTy);
  StructTy->decorate(Decoration::BufferBlock);
  StructTy->memberDecorate(0, Decoration::Offset)->addExtraOperand(0);

  auto StructPtrTy = m->getPointerType(StorageClass::Uniform, StructTy);

  VariableInst *bufferVar = b.MakeVariable(StructPtrTy, StorageClass::Uniform);
  bufferVar->decorate(Decoration::DescriptorSet)->addExtraOperand(0);
  bufferVar->decorate(Decoration::Binding)->addExtraOperand(binding);
  m->addVariable(bufferVar);

  return bufferVar;
}

bool AddWrapper(const char *name, const uint32_t signature,
                const uint32_t numInput, Builder &b, Module *m) {
  FunctionDefinition *kernel = m->lookupFunctionDefinitionByName(name);
  if (kernel == nullptr) {
    // In the metadata for RenderScript LLVM bitcode, the first foreach kernel
    // is always reserved for the root kernel, even though in the most recent RS
    // apps it does not exist. Simply bypass wrapper generation here, and return
    // true for this case.
    // Otherwise, if a non-root kernel function cannot be found, it is a
    // fatal internal error which is really unexpected.
    return (strncmp(name, "root", 4) == 0);
  }

  // The following three cases are not supported
  if (!MetadataExtractor::hasForEachSignatureKernel(signature)) {
    // Not handling old-style kernel
    return false;
  }

  if (MetadataExtractor::hasForEachSignatureUsrData(signature)) {
    // Not handling the user argument
    return false;
  }

  if (MetadataExtractor::hasForEachSignatureCtxt(signature)) {
    // Not handling the context argument
    return false;
  }

  TypeVoidInst *VoidTy = m->getVoidType();
  TypeFunctionInst *FuncTy = m->getFunctionType(VoidTy, nullptr, 0);
  FunctionDefinition *Func =
      b.MakeFunctionDefinition(VoidTy, FunctionControl::None, FuncTy);
  m->addFunctionDefinition(Func);

  Block *Blk = b.MakeBlock();
  Func->addBlock(Blk);

  Blk->addInstruction(b.MakeLabel());

  TypeIntInst *UIntTy = m->getUnsignedIntType(32);

  Instruction *XValue = nullptr;
  Instruction *YValue = nullptr;
  Instruction *ZValue = nullptr;
  Instruction *Index = nullptr;
  VariableInst *InvocationId = nullptr;
  VariableInst *NumWorkgroups = nullptr;

  if (MetadataExtractor::hasForEachSignatureIn(signature) ||
      MetadataExtractor::hasForEachSignatureOut(signature) ||
      MetadataExtractor::hasForEachSignatureX(signature) ||
      MetadataExtractor::hasForEachSignatureY(signature) ||
      MetadataExtractor::hasForEachSignatureZ(signature)) {
    TypeVectorInst *V3UIntTy = m->getVectorType(UIntTy, 3);
    InvocationId = m->getInvocationId();
    auto IID = b.MakeLoad(V3UIntTy, InvocationId);
    Blk->addInstruction(IID);

    XValue = b.MakeCompositeExtract(UIntTy, IID, {0});
    Blk->addInstruction(XValue);

    YValue = b.MakeCompositeExtract(UIntTy, IID, {1});
    Blk->addInstruction(YValue);

    ZValue = b.MakeCompositeExtract(UIntTy, IID, {2});
    Blk->addInstruction(ZValue);

    // TODO: Use SpecConstant for workgroup size
    auto ConstOne = m->getConstant(UIntTy, 1U);
    auto GroupSize =
        m->getConstantComposite(V3UIntTy, ConstOne, ConstOne, ConstOne);

    auto GroupSizeX = b.MakeCompositeExtract(UIntTy, GroupSize, {0});
    Blk->addInstruction(GroupSizeX);

    auto GroupSizeY = b.MakeCompositeExtract(UIntTy, GroupSize, {1});
    Blk->addInstruction(GroupSizeY);

    NumWorkgroups = m->getNumWorkgroups();
    auto NumGroup = b.MakeLoad(V3UIntTy, NumWorkgroups);
    Blk->addInstruction(NumGroup);

    auto NumGroupX = b.MakeCompositeExtract(UIntTy, NumGroup, {0});
    Blk->addInstruction(NumGroupX);

    auto NumGroupY = b.MakeCompositeExtract(UIntTy, NumGroup, {1});
    Blk->addInstruction(NumGroupY);

    auto GlobalSizeX = b.MakeIMul(UIntTy, GroupSizeX, NumGroupX);
    Blk->addInstruction(GlobalSizeX);

    auto GlobalSizeY = b.MakeIMul(UIntTy, GroupSizeY, NumGroupY);
    Blk->addInstruction(GlobalSizeY);

    auto RowsAlongZ = b.MakeIMul(UIntTy, GlobalSizeY, ZValue);
    Blk->addInstruction(RowsAlongZ);

    auto NumRows = b.MakeIAdd(UIntTy, YValue, RowsAlongZ);
    Blk->addInstruction(NumRows);

    auto NumCellsFromYZ = b.MakeIMul(UIntTy, GlobalSizeX, NumRows);
    Blk->addInstruction(NumCellsFromYZ);

    Index = b.MakeIAdd(UIntTy, NumCellsFromYZ, XValue);
    Blk->addInstruction(Index);
  }

  std::vector<IdRef> inputs;

  ConstantInst *ConstZero = m->getConstant(UIntTy, 0);

  for (uint32_t i = 0; i < numInput; i++) {
    FunctionParameterInst *param = kernel->getParameter(i);
    Instruction *elementType = param->mResultType.mInstruction;
    VariableInst *inputBuffer = AddBuffer(elementType, i + 3, b, m);

    TypePointerInst *PtrTy =
        m->getPointerType(StorageClass::Function, elementType);
    AccessChainInst *Ptr =
        b.MakeAccessChain(PtrTy, inputBuffer, {ConstZero, Index});
    Blk->addInstruction(Ptr);

    Instruction *input = b.MakeLoad(elementType, Ptr);
    Blk->addInstruction(input);

    inputs.push_back(IdRef(input));
  }

  // TODO: Convert from unsigned int to signed int if that is what the kernel
  // function takes for the coordinate parameters
  if (MetadataExtractor::hasForEachSignatureX(signature)) {
    inputs.push_back(XValue);
    if (MetadataExtractor::hasForEachSignatureY(signature)) {
      inputs.push_back(YValue);
      if (MetadataExtractor::hasForEachSignatureZ(signature)) {
        inputs.push_back(ZValue);
      }
    }
  }

  auto resultType = kernel->getReturnType();
  auto kernelCall =
      b.MakeFunctionCall(resultType, kernel->getInstruction(), inputs);
  Blk->addInstruction(kernelCall);

  if (MetadataExtractor::hasForEachSignatureOut(signature)) {
    VariableInst *OutputBuffer = AddBuffer(resultType, 2, b, m);
    auto resultPtrType = m->getPointerType(StorageClass::Function, resultType);
    AccessChainInst *OutPtr =
        b.MakeAccessChain(resultPtrType, OutputBuffer, {ConstZero, Index});
    Blk->addInstruction(OutPtr);
    Blk->addInstruction(b.MakeStore(OutPtr, kernelCall));
  }

  Blk->addInstruction(b.MakeReturn());

  std::string wrapperName("entry_");
  wrapperName.append(name);

  EntryPointDefinition *entry = b.MakeEntryPointDefinition(
      ExecutionModel::GLCompute, Func, wrapperName.c_str());

  entry->setLocalSize(1, 1, 1);

  if (Index != nullptr) {
    entry->addToInterface(InvocationId);
    entry->addToInterface(NumWorkgroups);
  }

  m->addEntryPoint(entry);

  return true;
}

bool DecorateGlobalBuffer(llvm::Module &LM, Builder &b, Module *m) {
  Instruction *inst = m->lookupByName("__GPUBlock");
  if (inst == nullptr) {
    return true;
  }

  VariableInst *bufferVar = static_cast<VariableInst *>(inst);
  bufferVar->decorate(Decoration::DescriptorSet)->addExtraOperand(0);
  bufferVar->decorate(Decoration::Binding)->addExtraOperand(0);

  TypePointerInst *StructPtrTy =
      static_cast<TypePointerInst *>(bufferVar->mResultType.mInstruction);
  TypeStructInst *StructTy =
      static_cast<TypeStructInst *>(StructPtrTy->mOperand2.mInstruction);
  StructTy->decorate(Decoration::BufferBlock);

  // Decorate each member with proper offsets

  const auto GlobalsB = LM.globals().begin();
  const auto GlobalsE = LM.globals().end();
  const auto Found =
      std::find_if(GlobalsB, GlobalsE, [](const llvm::GlobalVariable &GV) {
        return GV.getName() == "__GPUBlock";
      });

  if (Found == GlobalsE) {
    return true; // GPUBlock not found - not an error by itself.
  }

  const llvm::GlobalVariable &G = *Found;

  rs2spirv::Context &Ctxt = rs2spirv::Context::getInstance();
  bool IsCorrectTy = false;
  if (const auto *LPtrTy = llvm::dyn_cast<llvm::PointerType>(G.getType())) {
    if (auto *LStructTy =
            llvm::dyn_cast<llvm::StructType>(LPtrTy->getElementType())) {
      IsCorrectTy = true;

      const auto &DLayout = LM.getDataLayout();
      const auto *SLayout = DLayout.getStructLayout(LStructTy);
      assert(SLayout);
      if (SLayout == nullptr) {
        std::cerr << "struct layout is null" << std::endl;
        return false;
      }
      std::vector<uint32_t> offsets;
      for (uint32_t i = 0, e = LStructTy->getNumElements(); i != e; ++i) {
        auto decor = StructTy->memberDecorate(i, Decoration::Offset);
        if (!decor) {
          std::cerr << "failed creating member decoration for field " << i
                    << std::endl;
          return false;
        }
        const uint32_t offset = (uint32_t)SLayout->getElementOffset(i);
        decor->addExtraOperand(offset);
        offsets.push_back(offset);
      }
      std::stringstream ssOffsets;
      // TODO: define this string in a central place
      ssOffsets << ".rsov.ExportedVars:";
      for(uint32_t slot = 0; slot < Ctxt.getNumExportVar(); slot++) {
        const uint32_t index = Ctxt.getExportVarIndex(slot);
        const uint32_t offset = offsets[index];
        ssOffsets << offset << ';';
      }
      m->addString(ssOffsets.str().c_str());

      std::stringstream ssGlobalSize;
      ssGlobalSize << ".rsov.GlobalSize:" << Ctxt.getGlobalSize();
      m->addString(ssGlobalSize.str().c_str());
    }
  }

  if (!IsCorrectTy) {
    return false;
  }

  llvm::SmallVector<rs2spirv::RSAllocationInfo, 2> RSAllocs;
  if (!getRSAllocationInfo(LM, RSAllocs)) {
    // llvm::errs() << "Extracting rs_allocation info failed\n";
    return true;
  }

  // TODO: clean up the binding number assignment
  size_t BindingNum = 3;
  for (const auto &A : RSAllocs) {
    Instruction *inst = m->lookupByName(A.VarName.c_str());
    if (inst == nullptr) {
      return false;
    }
    VariableInst *bufferVar = static_cast<VariableInst *>(inst);
    bufferVar->decorate(Decoration::DescriptorSet)->addExtraOperand(0);
    bufferVar->decorate(Decoration::Binding)->addExtraOperand(BindingNum++);
  }

  return true;
}

void AddHeader(Module *m) {
  m->addCapability(Capability::Shader);
  m->setMemoryModel(AddressingModel::Logical, MemoryModel::GLSL450);

  m->addSource(SourceLanguage::GLSL, 450);
  m->addSourceExtension("GL_ARB_separate_shader_objects");
  m->addSourceExtension("GL_ARB_shading_language_420pack");
  m->addSourceExtension("GL_GOOGLE_cpp_style_line_directive");
  m->addSourceExtension("GL_GOOGLE_include_directive");
}

namespace {

class StorageClassVisitor : public DoNothingVisitor {
public:
  void visit(TypePointerInst *inst) override {
    matchAndReplace(inst->mOperand1);
  }

  void visit(TypeForwardPointerInst *inst) override {
    matchAndReplace(inst->mOperand2);
  }

  void visit(VariableInst *inst) override { matchAndReplace(inst->mOperand1); }

private:
  void matchAndReplace(StorageClass &storage) {
    if (storage == StorageClass::Function) {
      storage = StorageClass::Uniform;
    }
  }
};

void FixGlobalStorageClass(Module *m) {
  StorageClassVisitor v;
  m->getGlobalSection()->accept(&v);
}

} // anonymous namespace

bool AddWrappers(llvm::Module &LM,
                 android::spirit::Module *m) {
  rs2spirv::Context &Ctxt = rs2spirv::Context::getInstance();
  const bcinfo::MetadataExtractor &metadata = Ctxt.getMetadata();
  android::spirit::Builder b;

  m->setBuilder(&b);

  FixGlobalStorageClass(m);

  AddHeader(m);

  DecorateGlobalBuffer(LM, b, m);

  const size_t numKernel = metadata.getExportForEachSignatureCount();
  const char **kernelName = metadata.getExportForEachNameList();
  const uint32_t *kernelSigature = metadata.getExportForEachSignatureList();
  const uint32_t *inputCount = metadata.getExportForEachInputCountList();

  for (size_t i = 0; i < numKernel; i++) {
    bool success =
        AddWrapper(kernelName[i], kernelSigature[i], inputCount[i], b, m);
    if (!success) {
      return false;
    }
  }

  m->consolidateAnnotations();
  return true;
}

class WrapperPass : public Pass {
public:
  WrapperPass(const llvm::Module &LM) : mLLVMModule(const_cast<llvm::Module&>(LM)) {}

  Module *run(Module *m, int *error) override {
    bool success = AddWrappers(mLLVMModule, m);
    if (error) {
      *error = success ? 0 : -1;
    }
    return m;
  }

private:
  llvm::Module &mLLVMModule;
};

} // namespace spirit
} // namespace android

namespace rs2spirv {

android::spirit::Pass* CreateWrapperPass(const llvm::Module &LLVMModule) {
  return new android::spirit::WrapperPass(LLVMModule);
}

} // namespace rs2spirv
