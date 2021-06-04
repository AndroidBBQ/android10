/*
 * Copyright 2016, The Android Open Source Project
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

#include "RSSPIRVWriter.h"

#include "Builtin.h"
#include "Context.h"
#include "GlobalAllocPass.h"
#include "GlobalAllocSPIRITPass.h"
#include "GlobalMergePass.h"
#include "InlinePreparationPass.h"
#include "RemoveNonkernelsPass.h"
#include "SPIRVModule.h"
#include "Wrapper.h"
#include "bcinfo/MetadataExtractor.h"
#include "pass_queue.h"

#include "llvm/ADT/Triple.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/SPIRV.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"

#define DEBUG_TYPE "rs2spirv-writer"

using namespace llvm;
using namespace SPIRV;

namespace rs2spirv {

void addPassesForRS2SPIRV(llvm::legacy::PassManager &PassMgr) {
  PassMgr.add(createGlobalMergePass());

  PassMgr.add(createInlinePreparationPass());
  PassMgr.add(createAlwaysInlinerPass());
  PassMgr.add(createRemoveNonkernelsPass());
  // Delete unreachable globals.
  PassMgr.add(createGlobalDCEPass());
  // Remove dead debug info.
  PassMgr.add(createStripDeadDebugInfoPass());
  // Remove dead func decls.
  PassMgr.add(createStripDeadPrototypesPass());
  // Transform global allocations and accessors (rs[GS]etElementAt)
  PassMgr.add(createGlobalAllocPass());
  // Removed dead MemCpys in 64-bit targets after global alloc pass
  PassMgr.add(createDeadStoreEliminationPass());
  PassMgr.add(createAggressiveDCEPass());
  // Delete unreachable globals (after removing global allocations)
  PassMgr.add(createRemoveAllGlobalAllocPass());
  PassMgr.add(createPromoteMemoryToRegisterPass());
  PassMgr.add(createTransOCLMD());
  // TODO: investigate removal of OCLTypeToSPIRV pass.
  PassMgr.add(createOCLTypeToSPIRV());
  PassMgr.add(createSPIRVRegularizeLLVM());
  PassMgr.add(createSPIRVLowerConstExpr());
  PassMgr.add(createSPIRVLowerBool());
}

bool WriteSPIRV(Context &Ctxt, Module *M,
                llvm::raw_ostream &OS, std::string &ErrMsg) {
  llvm::legacy::PassManager PassMgr;
  addPassesForRS2SPIRV(PassMgr);

  std::unique_ptr<SPIRVModule> BM(SPIRVModule::createSPIRVModule());

  PassMgr.add(createLLVMToSPIRV(BM.get()));
  PassMgr.run(*M);
  DEBUG(M->dump());

  if (BM->getError(ErrMsg) != SPIRVEC_Success) {
    return false;
  }

  llvm::SmallString<4096> O;
  llvm::raw_svector_ostream SVOS(O);

  SVOS << *BM;

  llvm::StringRef str = SVOS.str();
  std::vector<uint32_t> words(str.size() / 4);

  memcpy(words.data(), str.data(), str.size());

  android::spirit::PassQueue spiritPasses;
  spiritPasses.append(CreateWrapperPass(*M));
  spiritPasses.append(CreateBuiltinPass());
  spiritPasses.append(CreateGAPass());

  int error;
  auto wordsOut = spiritPasses.run(words, &error);

  if (error != 0) {
    OS << *BM;
    ErrMsg = "Failed to generate wrappers for kernels";
    return false;
  }

  OS.write(reinterpret_cast<const char *>(wordsOut.data()),
           wordsOut.size() * 4);

  return true;
}

} // namespace rs2spirv
