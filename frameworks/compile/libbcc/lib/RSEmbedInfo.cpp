/*
 * Copyright 2012, The Android Open Source Project
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
#include "RSTransforms.h"
#include "RSUtils.h"
#include "rsDefines.h"

#include "bcc/Config.h"
#include "bcinfo/MetadataExtractor.h"

#include <string>
#include <cstdlib>
#include <vector>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Type.h>

using namespace bcc;

namespace {

/* RSEmbedInfoPass - This pass operates on the entire module and embeds a
 * string constaining relevant metadata directly as a global variable.
 * This information does not need to be consistent across Android releases,
 * because the standalone compiler + compatibility driver or system driver
 * will be using the same format (i.e. bcc_compat + libRSSupport.so or
 * bcc + libRSCpuRef are always paired together for installation).
 */
class RSEmbedInfoPass : public llvm::ModulePass {
private:
  static char ID;

  llvm::Module *M;
  llvm::LLVMContext *C;

public:
  RSEmbedInfoPass()
      : ModulePass(ID),
        M(nullptr) {
  }

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  static std::string getRSInfoString(const llvm::Module *module) {
    std::string str;
    llvm::raw_string_ostream s(str);
    bcinfo::MetadataExtractor me(module);
    if (!me.extract()) {
      bccAssert(false && "Could not extract RS metadata for module!");
      return std::string("");
    }

    size_t exportVarCount = me.getExportVarCount();
    size_t exportFuncCount = me.getExportFuncCount();
    size_t exportForEachCount = me.getExportForEachSignatureCount();
    size_t exportReduceCount = me.getExportReduceCount();
    size_t objectSlotCount = me.getObjectSlotCount();
    size_t pragmaCount = me.getPragmaCount();
    const char **exportVarNameList = me.getExportVarNameList();
    const char **exportFuncNameList = me.getExportFuncNameList();
    const char **exportForEachNameList = me.getExportForEachNameList();
    const uint32_t *exportForEachSignatureList =
        me.getExportForEachSignatureList();
    const bcinfo::MetadataExtractor::Reduce *exportReduceList =
        me.getExportReduceList();
    const uint32_t *objectSlotList = me.getObjectSlotList();
    const char **pragmaKeyList = me.getPragmaKeyList();
    const char **pragmaValueList = me.getPragmaValueList();
    bool isThreadable = me.isThreadable();
    const char *buildChecksum = me.getBuildChecksum();

    size_t i;

    // We use a simple text format here that the compatibility library
    // can easily parse. Each section starts out with its name
    // followed by a count.  The count denotes the number of lines to
    // parse for that particular category. Variables and Functions
    // merely put the appropriate identifier on the line. ForEach
    // kernels have the encoded int signature, followed by a hyphen
    // followed by the identifier (function to look up). General
    // reduce kernels have the encoded int signature, followed by a
    // hyphen followed by the accumulator data size, followed by a
    // hyphen followed by the identifier (reduction name); and then
    // for each possible constituent function, a hyphen followed by
    // the identifier (function name) -- in the case where the
    // function is omitted, "." is used in place of the identifier.
    // Object Slots are just listed as one integer per line.

    s << "exportVarCount: " << exportVarCount << "\n";
    for (i = 0; i < exportVarCount; ++i) {
      s << exportVarNameList[i] << "\n";
    }

    s << "exportFuncCount: " << exportFuncCount << "\n";
    for (i = 0; i < exportFuncCount; ++i) {
      s << exportFuncNameList[i] << "\n";
    }

    s << "exportForEachCount: " << exportForEachCount << "\n";
    for (i = 0; i < exportForEachCount; ++i) {
      s << exportForEachSignatureList[i] << " - "
        << exportForEachNameList[i] << "\n";
    }

    s << "exportReduceCount: " << exportReduceCount << "\n";
    auto reduceFnName = [](const char *Name) { return Name ? Name : "."; };
    for (i = 0; i < exportReduceCount; ++i) {
      const bcinfo::MetadataExtractor::Reduce &reduce = exportReduceList[i];
      s << reduce.mSignature << " - "
        << reduce.mAccumulatorDataSize << " - "
        << reduce.mReduceName << " - "
        << reduceFnName(reduce.mInitializerName) << " - "
        << reduceFnName(reduce.mAccumulatorName) << " - "
        << ((reduce.mCombinerName != nullptr)
            ? reduce.mCombinerName
            : nameReduceCombinerFromAccumulator(reduce.mAccumulatorName)) << " - "
        << reduceFnName(reduce.mOutConverterName) << " - "
        << reduceFnName(reduce.mHalterName)
        << "\n";
    }

    s << "objectSlotCount: " << objectSlotCount << "\n";
    for (i = 0; i < objectSlotCount; ++i) {
      s << objectSlotList[i] << "\n";
    }

    s << "pragmaCount: " << pragmaCount << "\n";
    for (i = 0; i < pragmaCount; ++i) {
      s << pragmaKeyList[i] << " - "
        << pragmaValueList[i] << "\n";
    }
    s << "isThreadable: " << ((isThreadable) ? "yes" : "no") << "\n";

    if (buildChecksum != nullptr && buildChecksum[0]) {
      s << "buildChecksum: " << buildChecksum << "\n";
    }

    {
      // As per `exportReduceCount`'s linewise fields, we use the literal `"."`
      // to signify the empty field. This makes it easy to parse when it's
      // missing.
      llvm::StringRef slangVersion(".");
      if (auto nmd = module->getNamedMetadata("slang.llvm.version")) {
        if (auto md = nmd->getOperand(0)) {
          if (const auto ver =
                  llvm::dyn_cast<llvm::MDString>(md->getOperand(0))) {
            slangVersion = ver->getString();
          }
        }
      }
      s << "versionInfo: 2\n";
      s << "bcc - " << LLVM_VERSION_STRING << "\n";
      s << "slang - " << slangVersion << "\n";
      if (slangVersion != LLVM_VERSION_STRING && me.hasDebugInfo()) {
        ALOGW(
            "The debug info in module '%s' has a different version than "
            "expected (%s, expecting %s). The debugging experience may be "
            "unreliable.",
            module->getModuleIdentifier().c_str(), slangVersion.str().c_str(),
            LLVM_VERSION_STRING);
      }
    }

    s.flush();
    return str;
  }

  virtual bool runOnModule(llvm::Module &M) {
    this->M = &M;
    C = &M.getContext();

    // Embed this as the global variable .rs.info so that it will be
    // accessible from the shared object later.
    llvm::Constant *Init = llvm::ConstantDataArray::getString(*C,
                                                              getRSInfoString(&M));
    llvm::GlobalVariable *InfoGV =
        new llvm::GlobalVariable(M, Init->getType(), true,
                                 llvm::GlobalValue::ExternalLinkage, Init,
                                 kRsInfo);
    (void) InfoGV;

    return true;
  }

  virtual const char *getPassName() const {
    return "Embed Renderscript Info";
  }

};  // end RSEmbedInfoPass

}  // end anonymous namespace

char RSEmbedInfoPass::ID = 0;

namespace bcc {

llvm::ModulePass *
createRSEmbedInfoPass() {
  return new RSEmbedInfoPass();
}

}  // end namespace bcc
