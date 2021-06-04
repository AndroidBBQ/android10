//===--- llvm-as.cpp - The low-level LLVM assembler -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This utility may be invoked in the following manner:
//   llvm-as --help         - Output information about command line switches
//   llvm-as [options]      - Read LLVM asm from stdin, write bitcode to stdout
//   llvm-as [options] x.ll - Read LLVM asm from the x.ll file, write bitcode
//                            to the x.bc file.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/SystemUtils.h"
#include "llvm/Support/ToolOutputFile.h"

#include "slang_bitcode_gen.h"
#include "slang_version.h"

#include "StripUnkAttr/strip_unknown_attributes.h"

#include <memory>
using namespace llvm;

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input .llvm file>"), cl::init("-"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Override output filename"),
               cl::value_desc("filename"));

static cl::opt<bool>
Force("f", cl::desc("Enable binary output on terminals"));

static cl::opt<bool>
DisableOutput("disable-output", cl::desc("Disable output"), cl::init(false));

static cl::opt<uint32_t>
TargetAPI("target-api", cl::desc("Specify RenderScript target API version "
                                 "(0 = development API) (default is 0)"),
          cl::init(0));

static cl::opt<bool>
DumpAsm("d", cl::desc("Print assembly as parsed"), cl::Hidden);

static cl::opt<bool>
DisableVerify("disable-verify", cl::Hidden,
              cl::desc("Do not run verifier on input LLVM (dangerous!)"));

static void stripUnknownAttributes(llvm::Module *M) {
  for (llvm::Function &F : *M)
    slang::stripUnknownAttributes(F);
}

static void WriteOutputFile(const Module *M, uint32_t ModuleTargetAPI) {
  // Infer the output filename if needed.
  if (OutputFilename.empty()) {
    if (InputFilename == "-") {
      OutputFilename = "-";
    } else {
      std::string IFN = InputFilename;
      int Len = IFN.length();
      if (IFN[Len-3] == '.' && IFN[Len-2] == 'l' && IFN[Len-1] == 'l') {
        // Source ends in .ll
        OutputFilename = std::string(IFN.begin(), IFN.end()-3);
      } else {
        OutputFilename = IFN;   // Append a .bc to it
      }
      OutputFilename += ".bc";
    }
  }

  std::error_code EC;
  std::unique_ptr<tool_output_file> Out
  (new tool_output_file(OutputFilename.c_str(), EC, llvm::sys::fs::F_None));
  if (EC) {
    // TODO(srhines): This isn't actually very specific and needs cleanup.
    errs() << EC.message() << '\n';
    exit(1);
  }

  if (Force || !CheckBitcodeOutputToConsole(Out->os(), true)) {
    slang::writeBitcode(Out->os(), *M,
        /* TargetAPI = */ ModuleTargetAPI,
        /* OptimizationLevel = */ 3,
        /* GenerateDebugInfo = */ false);

    if (!Out->os().has_error()) {
      // Declare success.
      Out->keep();
    }
  }
}

int main(int argc, char **argv) {
  // Print a stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);
  LLVMContext Context;
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
  cl::ParseCommandLineOptions(argc, argv, "llvm .ll -> .bc assembler\n");

  // Check target API.
  uint32_t ActualTargetAPI = (TargetAPI == 0) ? RS_DEVELOPMENT_API : TargetAPI;

  if (ActualTargetAPI != RS_DEVELOPMENT_API &&
      (ActualTargetAPI < SLANG_MINIMUM_TARGET_API ||
       ActualTargetAPI > SLANG_MAXIMUM_TARGET_API)) {
    errs() << "target API level '" << ActualTargetAPI << "' is out of range "
           << "('" << SLANG_MINIMUM_TARGET_API << "' - '"
           << SLANG_MAXIMUM_TARGET_API << "')\n";
    return 1;
  }

  // Parse the file now...
  SMDiagnostic Err;
  std::unique_ptr<Module> M(parseAssemblyFile(InputFilename, Err, Context));
  if (M.get() == 0) {
    Err.print(argv[0], errs());
    return 1;
  }

  if (!DisableVerify) {
    std::string Err;
    raw_string_ostream stream(Err);
    if (verifyModule(*M.get(), &stream)) {
      errs() << argv[0]
             << ": assembly parsed, but does not verify as correct!\n";
      errs() << Err;
      return 1;
    }
  }

  stripUnknownAttributes(M.get());

  if (DumpAsm) errs() << "Here's the assembly:\n" << *M.get();

  if (!DisableOutput)
    WriteOutputFile(M.get(), ActualTargetAPI);

  return 0;
}
