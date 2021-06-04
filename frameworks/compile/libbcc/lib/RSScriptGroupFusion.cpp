/*
 * Copyright 2015, The Android Open Source Project
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

#include "RSScriptGroupFusion.h"

#include "Assert.h"
#include "Log.h"
#include "bcc/BCCContext.h"
#include "bcc/Source.h"
#include "bcinfo/MetadataExtractor.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

using llvm::Function;
using llvm::Module;

using std::string;

namespace bcc {

namespace {

const Function* getInvokeFunction(const Source& source, const int slot,
                                  Module* newModule) {

  bcinfo::MetadataExtractor &metadata = *source.getMetadata();
  const char* functionName = metadata.getExportFuncNameList()[slot];
  Function* func = newModule->getFunction(functionName);
  // Materialize the function so that later the caller can inspect its argument
  // and return types.
  newModule->materialize(func);
  return func;
}

const Function*
getFunction(Module* mergedModule, const Source* source, const int slot,
            uint32_t* signature) {

  bcinfo::MetadataExtractor &metadata = *source->getMetadata();
  const char* functionName = metadata.getExportForEachNameList()[slot];
  if (functionName == nullptr || !functionName[0]) {
    ALOGE("Kernel fusion (module %s slot %d): failed to find kernel function",
          source->getName().c_str(), slot);
    return nullptr;
  }

  if (metadata.getExportForEachInputCountList()[slot] > 1) {
    ALOGE("Kernel fusion (module %s function %s): cannot handle multiple inputs",
          source->getName().c_str(), functionName);
    return nullptr;
  }

  if (signature != nullptr) {
    *signature = metadata.getExportForEachSignatureList()[slot];
  }

  const Function* function = mergedModule->getFunction(functionName);

  return function;
}

// The whitelist of supported signature bits. Context or user data arguments are
// not currently supported in kernel fusion. To support them or any new kinds of
// arguments in the future, it requires not only listing the signature bits here,
// but also implementing additional necessary fusion logic in the getFusedFuncSig(),
// getFusedFuncType(), and fuseKernels() functions below.
constexpr uint32_t ExpectedSignatureBits =
        bcinfo::MD_SIG_In |
        bcinfo::MD_SIG_Out |
        bcinfo::MD_SIG_X |
        bcinfo::MD_SIG_Y |
        bcinfo::MD_SIG_Z |
        bcinfo::MD_SIG_Kernel;

int getFusedFuncSig(const std::vector<Source*>& sources,
                    const std::vector<int>& slots,
                    uint32_t* retSig) {
  *retSig = 0;
  uint32_t firstSignature = 0;
  uint32_t signature = 0;
  auto slotIter = slots.begin();
  for (const Source* source : sources) {
    const int slot = *slotIter++;
    bcinfo::MetadataExtractor &metadata = *source->getMetadata();

    if (metadata.getExportForEachInputCountList()[slot] > 1) {
      ALOGE("Kernel fusion (module %s slot %d): cannot handle multiple inputs",
            source->getName().c_str(), slot);
      return -1;
    }

    signature = metadata.getExportForEachSignatureList()[slot];
    if (signature & ~ExpectedSignatureBits) {
      ALOGE("Kernel fusion (module %s slot %d): Unexpected signature %x",
            source->getName().c_str(), slot, signature);
      return -1;
    }

    if (firstSignature == 0) {
      firstSignature = signature;
    }

    *retSig |= signature;
  }

  if (!bcinfo::MetadataExtractor::hasForEachSignatureIn(firstSignature)) {
    *retSig &= ~bcinfo::MD_SIG_In;
  }

  if (!bcinfo::MetadataExtractor::hasForEachSignatureOut(signature)) {
    *retSig &= ~bcinfo::MD_SIG_Out;
  }

  return 0;
}

llvm::FunctionType* getFusedFuncType(bcc::BCCContext& Context,
                                     const std::vector<Source*>& sources,
                                     const std::vector<int>& slots,
                                     Module* M,
                                     uint32_t* signature) {
  int error = getFusedFuncSig(sources, slots, signature);

  if (error < 0) {
    return nullptr;
  }

  const Function* firstF = getFunction(M, sources.front(), slots.front(), nullptr);

  bccAssert (firstF != nullptr);

  llvm::SmallVector<llvm::Type*, 8> ArgTys;

  if (bcinfo::MetadataExtractor::hasForEachSignatureIn(*signature)) {
    ArgTys.push_back(firstF->arg_begin()->getType());
  }

  llvm::Type* I32Ty = llvm::IntegerType::get(Context.getLLVMContext(), 32);
  if (bcinfo::MetadataExtractor::hasForEachSignatureX(*signature)) {
    ArgTys.push_back(I32Ty);
  }
  if (bcinfo::MetadataExtractor::hasForEachSignatureY(*signature)) {
    ArgTys.push_back(I32Ty);
  }
  if (bcinfo::MetadataExtractor::hasForEachSignatureZ(*signature)) {
    ArgTys.push_back(I32Ty);
  }

  const Function* lastF = getFunction(M, sources.back(), slots.back(), nullptr);

  bccAssert (lastF != nullptr);

  llvm::Type* retTy = lastF->getReturnType();

  return llvm::FunctionType::get(retTy, ArgTys, false);
}

}  // anonymous namespace

bool fuseKernels(bcc::BCCContext& Context,
                 const std::vector<Source *>& sources,
                 const std::vector<int>& slots,
                 const std::string& fusedName,
                 Module* mergedModule) {
  bccAssert(sources.size() == slots.size() && "sources and slots differ in size");

  uint32_t fusedFunctionSignature;

  llvm::FunctionType* fusedType =
          getFusedFuncType(Context, sources, slots, mergedModule, &fusedFunctionSignature);

  if (fusedType == nullptr) {
    return false;
  }

  Function* fusedKernel =
          (Function*)(mergedModule->getOrInsertFunction(fusedName, fusedType));

  llvm::LLVMContext& ctxt = Context.getLLVMContext();

  llvm::BasicBlock* block = llvm::BasicBlock::Create(ctxt, "entry", fusedKernel);
  llvm::IRBuilder<> builder(block);

  Function::arg_iterator argIter = fusedKernel->arg_begin();

  llvm::Value* dataElement = nullptr;
  if (bcinfo::MetadataExtractor::hasForEachSignatureIn(fusedFunctionSignature)) {
    dataElement = &*(argIter++);
    dataElement->setName("DataIn");
  }

  llvm::Value* X = nullptr;
  if (bcinfo::MetadataExtractor::hasForEachSignatureX(fusedFunctionSignature)) {
    X = &*(argIter++);
    X->setName("x");
  }

  llvm::Value* Y = nullptr;
  if (bcinfo::MetadataExtractor::hasForEachSignatureY(fusedFunctionSignature)) {
    Y = &*(argIter++);
    Y->setName("y");
  }

  llvm::Value* Z = nullptr;
  if (bcinfo::MetadataExtractor::hasForEachSignatureZ(fusedFunctionSignature)) {
    Z = &*(argIter++);
    Z->setName("z");
  }

  auto slotIter = slots.begin();
  for (const Source* source : sources) {
    int slot = *slotIter;

    uint32_t inputFunctionSignature;
    const Function* inputFunction =
            getFunction(mergedModule, source, slot, &inputFunctionSignature);
    if (inputFunction == nullptr) {
      // Either failed to find the kernel function, or the function has multiple inputs.
      return false;
    }

    // Don't try to fuse a non-kernel
    if (!bcinfo::MetadataExtractor::hasForEachSignatureKernel(inputFunctionSignature)) {
      ALOGE("Kernel fusion (module %s function %s): not a kernel",
            source->getName().c_str(), inputFunction->getName().str().c_str());
      return false;
    }

    std::vector<llvm::Value*> args;

    if (bcinfo::MetadataExtractor::hasForEachSignatureIn(inputFunctionSignature)) {
      if (dataElement == nullptr) {
        ALOGE("Kernel fusion (module %s function %s): expected input, but got null",
              source->getName().c_str(), inputFunction->getName().str().c_str());
        return false;
      }

      const llvm::FunctionType* funcTy = inputFunction->getFunctionType();
      llvm::Type* firstArgType = funcTy->getParamType(0);

      if (dataElement->getType() != firstArgType) {
        std::string msg;
        llvm::raw_string_ostream rso(msg);
        rso << "Mismatching argument type, expected ";
        firstArgType->print(rso);
        rso << ", received ";
        dataElement->getType()->print(rso);
        ALOGE("Kernel fusion (module %s function %s): %s", source->getName().c_str(),
              inputFunction->getName().str().c_str(), rso.str().c_str());
        return false;
      }

      args.push_back(dataElement);
    } else {
      // Only the first kernel in a batch is allowed to have no input
      if (slotIter != slots.begin()) {
        ALOGE("Kernel fusion (module %s function %s): function not first in batch takes no input",
              source->getName().c_str(), inputFunction->getName().str().c_str());
        return false;
      }
    }

    if (bcinfo::MetadataExtractor::hasForEachSignatureX(inputFunctionSignature)) {
      args.push_back(X);
    }

    if (bcinfo::MetadataExtractor::hasForEachSignatureY(inputFunctionSignature)) {
      args.push_back(Y);
    }

    if (bcinfo::MetadataExtractor::hasForEachSignatureZ(inputFunctionSignature)) {
      args.push_back(Z);
    }

    dataElement = builder.CreateCall((llvm::Value*)inputFunction, args);

    slotIter++;
  }

  if (fusedKernel->getReturnType()->isVoidTy()) {
    builder.CreateRetVoid();
  } else {
    builder.CreateRet(dataElement);
  }

  llvm::NamedMDNode* ExportForEachNameMD =
    mergedModule->getOrInsertNamedMetadata("#rs_export_foreach_name");

  llvm::MDString* nameMDStr = llvm::MDString::get(ctxt, fusedName);
  llvm::MDNode* nameMDNode = llvm::MDNode::get(ctxt, nameMDStr);
  ExportForEachNameMD->addOperand(nameMDNode);

  llvm::NamedMDNode* ExportForEachMD =
    mergedModule->getOrInsertNamedMetadata("#rs_export_foreach");
  llvm::MDString* sigMDStr = llvm::MDString::get(ctxt,
                                                 llvm::utostr(fusedFunctionSignature));
  llvm::MDNode* sigMDNode = llvm::MDNode::get(ctxt, sigMDStr);
  ExportForEachMD->addOperand(sigMDNode);

  return true;
}

bool renameInvoke(BCCContext& Context, const Source* source, const int slot,
                  const std::string& newName, Module* module) {
  const llvm::Function* F = getInvokeFunction(*source, slot, module);
  std::vector<llvm::Type*> params;
  for (auto I = F->arg_begin(), E = F->arg_end(); I != E; ++I) {
    params.push_back(I->getType());
  }
  llvm::Type* returnTy = F->getReturnType();

  llvm::FunctionType* batchFuncTy =
          llvm::FunctionType::get(returnTy, params, false);

  llvm::Function* newF =
          llvm::Function::Create(batchFuncTy,
                                 llvm::GlobalValue::ExternalLinkage, newName,
                                 module);

  llvm::BasicBlock* block = llvm::BasicBlock::Create(Context.getLLVMContext(),
                                                     "entry", newF);
  llvm::IRBuilder<> builder(block);

  llvm::Function::arg_iterator argIter = newF->arg_begin();
  llvm::Value* arg1 = &*(argIter++);
  builder.CreateCall((llvm::Value*)F, arg1);

  builder.CreateRetVoid();

  llvm::NamedMDNode* ExportFuncNameMD =
          module->getOrInsertNamedMetadata("#rs_export_func");
  llvm::MDString* strMD = llvm::MDString::get(module->getContext(), newName);
  llvm::MDNode* nodeMD = llvm::MDNode::get(module->getContext(), strMD);
  ExportFuncNameMD->addOperand(nodeMD);

  return true;
}

}  // namespace bcc
