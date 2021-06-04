/*
 * Copyright 2013, The Android Open Source Project
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

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/SystemUtils.h"
#include "llvm/Support/ToolOutputFile.h"
using namespace llvm;

static cl::list<std::string>
InputFilenames(cl::Positional,
               cl::desc("<input bitcode files>"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Override output filename"), cl::init("-"),
               cl::value_desc("filename"));

static cl::opt<bool>
OutputAssembly("S",
               cl::desc("Write output as LLVM assembly"), cl::Hidden);

namespace {
  class StripAttributes : public ModulePass {
  public:
    static char ID;

    StripAttributes() : ModulePass(ID) {
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

    bool runOnFunction(Function &F) {
      // Remove any target-cpu and/or target-features attributes from each
      // Function or Function declaration.
      if (F.hasFnAttribute("target-cpu") ||
          F.hasFnAttribute("target-features")) {

        AttrBuilder B;
        B.addAttribute("target-cpu").addAttribute("target-features");
        AttributeSet ToStrip = AttributeSet::get(F.getContext(),
            AttributeSet::FunctionIndex, B);
        F.removeAttributes(AttributeSet::FunctionIndex, ToStrip);
        return true;
      }
      return false;
    }

    // We have to use a ModulePass, since a FunctionPass only gets run on
    // defined Functions (and not declared Functions).
    virtual bool runOnModule(Module &M) {
      bool Changed = false;
      for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
        Changed |= runOnFunction(*I);
      }
      return Changed;
    }
  };

  llvm::ModulePass * createStripAttributePass() {
    return new StripAttributes();
  }
}


char StripAttributes::ID = 0;
static RegisterPass<StripAttributes> RPSA("StripAttributes",
    "Strip Function Attributes Pass");


static inline std::unique_ptr<Module> LoadFile(const char *argv0,
                                               const std::string &FN,
                                               LLVMContext& Context) {
  SMDiagnostic Err;
  std::unique_ptr<Module> Result = parseIRFile(FN, Err, Context);
  if (Result) {
    return Result;   // Load successful!
  }

  Err.print(argv0, errs());
  return std::unique_ptr<Module>();
}


int main(int argc, char **argv) {
  // Print a stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);

  LLVMContext Context;
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
  cl::ParseCommandLineOptions(argc, argv, "strip function attribute pass\n");

  std::string ErrorMessage;

  std::unique_ptr<Module> M(LoadFile(argv[0], InputFilenames[0], Context));
  if (M.get() == 0) {
    errs() << argv[0] << ": error loading file '"
           << InputFilenames[0] << "'\n";
    return 1;
  }

  // Perform the actual function attribute stripping.
  legacy::PassManager PM;
  PM.add(createStripAttributePass());
  PM.run(*M.get());

  std::error_code EC;
  tool_output_file Out(OutputFilename.c_str(), EC,
                       sys::fs::F_None);
  if (EC) {
    errs() << EC.message() << '\n';
    return 1;
  }

  if (verifyModule(*M)) {
    errs() << argv[0] << ": stripped module is broken!\n";
    return 1;
  }

  if (OutputAssembly) {
    Out.os() << *M;
  } else if (!CheckBitcodeOutputToConsole(Out.os(), true)) {
    WriteBitcodeToFile(M.get(), Out.os());
  }

  Out.keep();

  return 0;
}
