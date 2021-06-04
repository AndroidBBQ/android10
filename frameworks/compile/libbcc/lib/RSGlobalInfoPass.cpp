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

#include "Assert.h"
#include "Log.h"
#include "RSUtils.h"

#include "rsDefines.h"

#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Pass.h>

#include <sstream>
#include <vector>

namespace {

const bool kDebugGlobalInfo = false;

/* RSGlobalInfoPass: Embeds additional information about RenderScript global
 * variables into the Module. The 5 variables added are specified as follows:
 * 1) .rs.global_entries
 *    i32 - int
 *    Optional number of global variables.
 * 2) .rs.global_names
 *    [N * i8*] - const char *[N]
 *    Optional global variable name info. Each entry corresponds to the name
 *    of 1 of the N global variables.
 * 3) .rs.global_addresses
 *    [N * i8*] - void*[N] or void**
 *    Optional global variable address info. Each entry corresponds to the
 *    address of 1 of the N global variables.
 * 4) .rs.global_sizes
 *    [N * i32] or [N * i64] - size_t[N]
 *    Optional global variable size info. Each entry corresponds to the size
 *    of 1 of the N global variables.
 * 5) .rs.global_properties
 *    [N * i32]
 *    Optional global properties. Each entry corresponds to the properties
 *    for 1 of the N global variables. The 32-bit integer for properties
 *    can be broken down as follows:
 *    bit(s)    Encoded value
 *    ------    -------------
 *        18    Pointer (1 is pointer, 0 is non-pointer)
 *        17    Static (1 is static, 0 is extern)
 *        16    Constant (1 is const, 0 is non-const)
 *    15 - 0    RsDataType (see frameworks/rs/rsDefines.h for more info)
 */
class RSGlobalInfoPass: public llvm::ModulePass {
private:
  // If true, we don't include information about immutable global variables
  // in our various exported data structures.
  bool mSkipConstants;

  // Encodes properties of the GlobalVariable into a uint32_t.
  // These values are used to populate the .rs.global_properties array.
  static uint32_t getEncodedProperties(const llvm::GlobalVariable &GV) {
    auto GlobalType = GV.getType()->getPointerElementType();

    // We start by getting the RsDataType and placing it into our result.
    uint32_t result = getRsDataTypeForType(GlobalType);
    bccAssert(!(result & ~RS_GLOBAL_TYPE));  // Can only alter lower 16-bits.

    if (GlobalType->isPointerTy()) {
      // Global variables that are pointers can all be used with "bind".
      result |= RS_GLOBAL_POINTER;
    }

    if (GV.isConstant()) {
      result |= RS_GLOBAL_CONSTANT;
    }

    if (GV.getLinkage() == llvm::GlobalValue::InternalLinkage) {
      // We only have internal linkage in RS to signify static.
      result |= RS_GLOBAL_STATIC;
    }

    return result;
  }

public:
  static char ID;

  explicit RSGlobalInfoPass(bool pSkipConstants = false)
    : ModulePass (ID), mSkipConstants(pSkipConstants) {
  }

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    // This pass does not use any other analysis passes, but it does
    // add new global variables.
  }

  bool runOnModule(llvm::Module &M) override {
    std::vector<llvm::Constant *> GVAddresses;
    std::vector<llvm::Constant *> GVNames;
    std::vector<std::string> GVNameStrings;
    std::vector<uint32_t> GVSizes32;
    std::vector<uint64_t> GVSizes64;
    std::vector<uint32_t> GVProperties;

    const llvm::DataLayout &DL = M.getDataLayout();
    const size_t PointerSizeInBits = DL.getPointerSizeInBits();

    bccAssert(PointerSizeInBits == 32 || PointerSizeInBits == 64);

    int GlobalNumber = 0;

    // i8* - LLVM uses this to represent void* and char*
    llvm::Type *VoidPtrTy = llvm::Type::getInt8PtrTy(M.getContext());

    // i32
    llvm::Type *Int32Ty = llvm::Type::getInt32Ty(M.getContext());

    // i32 or i64 depending on our actual size_t
    llvm::Type *SizeTy = llvm::Type::getIntNTy(M.getContext(),
                                               PointerSizeInBits);

    for (auto &GV : M.globals()) {
      // Skip constant variables if we were configured to do so.
      if (mSkipConstants && GV.isConstant()) {
        continue;
      }

      // Skip intrinsic variables.
      if (GV.getName().startswith("llvm.")) {
        continue;
      }

      // In LLVM, an instance of GlobalVariable is actually a Value
      // corresponding to the address of it.
      GVAddresses.push_back(llvm::ConstantExpr::getBitCast(&GV, VoidPtrTy));
      GVNameStrings.push_back(GV.getName());

      // Since these are all global variables, their type is actually a
      // pointer to the underlying data. We can extract the total underlying
      // storage size by looking at the first contained type.
      auto GlobalType = GV.getType()->getPointerElementType();
      auto TypeSize = DL.getTypeAllocSize(GlobalType);
      if (PointerSizeInBits == 32) {
        GVSizes32.push_back(TypeSize);
      } else {
        GVSizes64.push_back(TypeSize);
      }

      GVProperties.push_back(getEncodedProperties(GV));
    }

    // Create the new strings for storing the names of the global variables.
    // This has to be done as a separate pass (over the original global
    // variables), because these strings are new global variables themselves.
    for (const auto &GVN : GVNameStrings) {
      llvm::Constant *C =
          llvm::ConstantDataArray::getString(M.getContext(), GVN);
      std::stringstream VarName;
      VarName << ".rs.name_str_" << GlobalNumber++;
      llvm::Value *V = M.getOrInsertGlobal(VarName.str(), C->getType());
      llvm::GlobalVariable *VarAsStr = llvm::dyn_cast<llvm::GlobalVariable>(V);
      VarAsStr->setInitializer(C);
      VarAsStr->setConstant(true);
      VarAsStr->setLinkage(llvm::GlobalValue::PrivateLinkage);
      VarAsStr->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
      // VarAsStr has type [_ x i8]*. Cast to i8* for storing in
      // .rs.global_names.
      GVNames.push_back(llvm::ConstantExpr::getBitCast(VarAsStr, VoidPtrTy));
    }

    if (PointerSizeInBits == 32) {
      bccAssert(GVAddresses.size() == GVSizes32.size());
      bccAssert(GVSizes64.size() == 0);
      bccAssert(GVAddresses.size() == GVProperties.size());
    } else {
      bccAssert(GVSizes32.size() == 0);
      bccAssert(GVAddresses.size() == GVSizes64.size());
      bccAssert(GVAddresses.size() == GVProperties.size());
    }

    size_t NumGlobals = GVAddresses.size();

    // [NumGlobals * i8*]
    llvm::ArrayType *VoidPtrArrayTy = llvm::ArrayType::get(VoidPtrTy,
                                                           NumGlobals);
    // [NumGlobals * i32] or [NumGlobals * i64]
    llvm::ArrayType *SizeArrayTy = llvm::ArrayType::get(SizeTy, NumGlobals);

    // [NumGlobals * i32]
    llvm::ArrayType *Int32ArrayTy = llvm::ArrayType::get(Int32Ty, NumGlobals);

    // 1) @.rs.global_entries = constant i32 NumGlobals
    llvm::Value *V = M.getOrInsertGlobal(kRsGlobalEntries, Int32Ty);
    llvm::GlobalVariable *GlobalEntries =
        llvm::dyn_cast<llvm::GlobalVariable>(V);
    llvm::Constant *GlobalEntriesInit =
        llvm::ConstantInt::get(Int32Ty, NumGlobals);
    GlobalEntries->setInitializer(GlobalEntriesInit);
    GlobalEntries->setConstant(true);

    // 2) @.rs.global_names = constant [N * i8*] [...]
    V = M.getOrInsertGlobal(kRsGlobalNames, VoidPtrArrayTy);
    llvm::GlobalVariable *GlobalNames =
        llvm::dyn_cast<llvm::GlobalVariable>(V);
    llvm::Constant *GlobalNamesInit =
        llvm::ConstantArray::get(VoidPtrArrayTy, GVNames);
    GlobalNames->setInitializer(GlobalNamesInit);
    GlobalNames->setConstant(true);

    // 3) @.rs.global_addresses = constant [N * i8*] [...]
    V = M.getOrInsertGlobal(kRsGlobalAddresses, VoidPtrArrayTy);
    llvm::GlobalVariable *GlobalAddresses =
        llvm::dyn_cast<llvm::GlobalVariable>(V);
    llvm::Constant *GlobalAddressesInit =
        llvm::ConstantArray::get(VoidPtrArrayTy, GVAddresses);
    GlobalAddresses->setInitializer(GlobalAddressesInit);
    GlobalAddresses->setConstant(true);


    // 4) @.rs.global_sizes = constant [N * i32 or i64] [...]
    V = M.getOrInsertGlobal(kRsGlobalSizes, SizeArrayTy);
    llvm::GlobalVariable *GlobalSizes =
        llvm::dyn_cast<llvm::GlobalVariable>(V);
    llvm::Constant *GlobalSizesInit;
    if (PointerSizeInBits == 32) {
      GlobalSizesInit = llvm::ConstantDataArray::get(M.getContext(), GVSizes32);
    } else {
      GlobalSizesInit = llvm::ConstantDataArray::get(M.getContext(), GVSizes64);
    }
    GlobalSizes->setInitializer(GlobalSizesInit);
    GlobalSizes->setConstant(true);

    // 5) @.rs.global_properties = constant i32 NumGlobals
    V = M.getOrInsertGlobal(kRsGlobalProperties, Int32ArrayTy);
    llvm::GlobalVariable *GlobalProperties =
        llvm::dyn_cast<llvm::GlobalVariable>(V);
    llvm::Constant *GlobalPropertiesInit =
        llvm::ConstantDataArray::get(M.getContext(), GVProperties);
    GlobalProperties->setInitializer(GlobalPropertiesInit);
    GlobalProperties->setConstant(true);

    if (kDebugGlobalInfo) {
      GlobalEntries->dump();
      GlobalNames->dump();
      GlobalAddresses->dump();
      GlobalSizes->dump();
      GlobalProperties->dump();
    }

    // Upon completion, this pass has always modified the Module.
    return true;
  }
};

}

char RSGlobalInfoPass::ID = 0;

static llvm::RegisterPass<RSGlobalInfoPass> X("embed-rs-global-info",
  "Embed additional information about RenderScript global variables");

namespace bcc {

llvm::ModulePass * createRSGlobalInfoPass(bool pSkipConstants) {
  return new RSGlobalInfoPass(pSkipConstants);
}

}
