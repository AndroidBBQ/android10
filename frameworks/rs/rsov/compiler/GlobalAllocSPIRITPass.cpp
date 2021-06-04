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

#include "GlobalAllocSPIRITPass.h"

#include "Context.h"
#include "spirit.h"
#include "transformer.h"

#include <sstream>

namespace android {
namespace spirit {

namespace {

// Metadata buffer for global allocations
// struct metadata {
//  uint32_t element_size;
//  uint32_t x_size;
//  uint32_t y_size;
//  uint32_t unused
// };

VariableInst *AddGAMetadata(Builder &b, Module *m) {
  TypeIntInst *UInt32Ty = m->getUnsignedIntType(32);
  std::vector<Instruction *> metadata{UInt32Ty, UInt32Ty, UInt32Ty, UInt32Ty};
  auto MetadataStructTy = m->getStructType(metadata.data(), metadata.size());
  // FIXME: workaround on a weird OpAccessChain member offset problem. Somehow
  // when given constant indices, OpAccessChain returns pointers that are 4
  // bytes less than what are supposed to be (at runtime). For now workaround
  // this with +4 the member offsets.
  MetadataStructTy->memberDecorate(0, Decoration::Offset)->addExtraOperand(4);
  MetadataStructTy->memberDecorate(1, Decoration::Offset)->addExtraOperand(8);
  MetadataStructTy->memberDecorate(2, Decoration::Offset)->addExtraOperand(12);
  MetadataStructTy->memberDecorate(3, Decoration::Offset)->addExtraOperand(16);
  // TBD: Implement getArrayType. RuntimeArray requires buffers and hence we
  // cannot use PushConstant underneath
  auto MetadataBufSTy = m->getRuntimeArrayType(MetadataStructTy);
  // Stride of metadata.
  MetadataBufSTy->decorate(Decoration::ArrayStride)
      ->addExtraOperand(metadata.size() * sizeof(uint32_t));
  auto MetadataSSBO = m->getStructType(MetadataBufSTy);
  MetadataSSBO->decorate(Decoration::BufferBlock);
  auto MetadataPtrTy = m->getPointerType(StorageClass::Uniform, MetadataSSBO);

  VariableInst *MetadataVar =
      b.MakeVariable(MetadataPtrTy, StorageClass::Uniform);
  MetadataVar->decorate(Decoration::DescriptorSet)->addExtraOperand(0);
  MetadataVar->decorate(Decoration::Binding)->addExtraOperand(1);
  m->addVariable(MetadataVar);

  return MetadataVar;
}

std::string CreateGAIDMetadata(
    const llvm::SmallVectorImpl<rs2spirv::RSAllocationInfo> &Allocs) {

  std::stringstream mapping;
  bool printed = false;

  mapping << "{\"__RSoV_GA\": {";
  for (auto &A : Allocs) {
    // Skip unused GAs
    if (!A.hasID()) {
      continue;
    }
    if (printed)
      mapping << ", ";
    // "GA name" to the ID of the GA
    mapping << "\"" << A.VarName.substr(1) << "\":" << A.ID;
    printed = true;
  }
  mapping << "}}";

  if (printed)
    return mapping.str().c_str();
  else
    return "";
}
} // anonymous namespace

// Replacing calls to lowered accessors, e.g., __rsov_rsAllocationGetDimX
// which was created from rsAllocationGetDimX by replacing the allocation
// with an ID in an earlier LLVM pass (see GlobalAllocationPass.cpp),
// to access the global allocation metadata.
//
// For example, the source code may look like:
//
// rs_allocation g;
// ...
//    uint32_t foo = rsAllocationGetDimX(g);
//
// After the  GlobalAllocPass, it would look like the LLVM IR
// equivalent of:
//
//    uint32_t foo = __rsov_rsAllocationGetDimX(0);
//
// After that pass, g is removed, and references in intrinsics
// to g would be replaced with an assigned unique id (0 here), and
// rsAllocationGetDimX() would be replaced by __rsov_rsAllocationGetDimX()
// where the only difference is the argument being replaced by the unique
// ID. __rsov_rsAllocationGetDimX() does not really exist - it is used
// as a marker for this pass to work on.
//
// After this GAAccessTransformer pass, it would look like (in SPIRIT):
//
//   uint32_t foo = Metadata[0].size_x;
//
// where the OpFunctionCall to __rsov_rsAllocationGetDim() is replaced by
// an OpAccessChain and OpLoad from the metadata buffer.

class GAAccessorTransformer : public Transformer {
public:
  GAAccessorTransformer()
      : mBuilder(), mMetadata(nullptr),
        mAllocs(rs2spirv::Context::getInstance().getGlobalAllocs()) {}

  std::vector<uint32_t> runAndSerialize(Module *module, int *error) override {
    std::string GAMD = CreateGAIDMetadata(mAllocs);
    if (GAMD.size() > 0) {
      module->addString(GAMD.c_str());
    }
    mMetadata = AddGAMetadata(mBuilder, module);
    return Transformer::runAndSerialize(module, error);
  }

  Instruction *transform(FunctionCallInst *call) {
    FunctionInst *func =
        static_cast<FunctionInst *>(call->mOperand1.mInstruction);
    const char *name = getModule()->lookupNameByInstruction(func);
    if (!name) {
      return call;
    }

    Instruction *inst = nullptr;
    // Maps name into a SPIR-V instruction
    // TODO: generalize it to support more accessors
    if (!strcmp(name, "__rsov_rsAllocationGetDimX")) {
      TypeIntInst *UInt32Ty = getModule()->getUnsignedIntType(32);
      // TODO: hardcoded layout
      auto ConstZero = getModule()->getConstant(UInt32Ty, 0U);
      auto ConstOne = getModule()->getConstant(UInt32Ty, 1U);

      // TODO: Use constant memory later
      auto resultPtrType =
          getModule()->getPointerType(StorageClass::Uniform, UInt32Ty);
      AccessChainInst *LoadPtr = mBuilder.MakeAccessChain(
          resultPtrType, mMetadata, {ConstZero, ConstZero, ConstOne});
      insert(LoadPtr);

      inst = mBuilder.MakeLoad(UInt32Ty, LoadPtr);
      inst->setId(call->getId());
    } else {
      inst = call;
    }
    return inst;
  }

private:
  Builder mBuilder;
  VariableInst *mMetadata;
  llvm::SmallVectorImpl<rs2spirv::RSAllocationInfo> &mAllocs;
};

} // namespace spirit
} // namespace android

namespace rs2spirv {

android::spirit::Pass *CreateGAPass(void) {
  return new android::spirit::GAAccessorTransformer();
}

} // namespace rs2spirv
