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

#include "bcc/Initialization.h"

#include "Log.h"
#include "bcc/Config.h"

#include <cstdlib>

#include <llvm/InitializePasses.h>
#include <llvm/PassRegistry.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/TargetSelect.h>

namespace {

void llvm_error_handler(void *pUserData, const std::string &pMessage,
                        bool pGenCrashDiag) {
  ALOGE("bcc: Internal Error - %s", pMessage.c_str());
  ::exit(1);
}

} // end anonymous namespace

void bcc::init::Initialize() {
  static bool is_initialized = false;

  if (is_initialized) {
    return;
  }

  // Setup error handler for LLVM.
  llvm::remove_fatal_error_handler();
  llvm::install_fatal_error_handler(llvm_error_handler, nullptr);


  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();

  llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
  llvm::initializeCore(Registry);
  llvm::initializeScalarOpts(Registry);
  llvm::initializeVectorization(Registry);
  llvm::initializeIPO(Registry);
  llvm::initializeAnalysis(Registry);
  llvm::initializeTransformUtils(Registry);
  llvm::initializeInstCombine(Registry);
  llvm::initializeInstrumentation(Registry);
  llvm::initializeTarget(Registry);
  llvm::initializeCodeGenPreparePass(Registry);
  llvm::initializeAtomicExpandPass(Registry);
  llvm::initializeRewriteSymbolsPass(Registry);

  is_initialized = true;

  return;
}
