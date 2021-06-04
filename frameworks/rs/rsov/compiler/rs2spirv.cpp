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
#include "bcinfo/MetadataExtractor.h"
#include "spirit/file_utils.h"

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataStream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"

#include "Context.h"
#include "GlobalMergePass.h"
#include "RSSPIRVWriter.h"

#define DEBUG_TYPE "rs2spirv"

namespace kExt {
const char SPIRVBinary[] = ".spv";
} // namespace kExt

using namespace llvm;

static cl::opt<std::string> InputFile(cl::Positional, cl::desc("<input file>"),
                                      cl::init("-"));

static cl::opt<std::string> OutputFile("o",
                                       cl::desc("Override output filename"),
                                       cl::value_desc("filename"));

static cl::opt<std::string> OutputBitcodeFile("bc",
                                              cl::desc("Override output bitcode filename"),
                                              cl::value_desc("bitcode filename"));

static std::string removeExt(const std::string &FileName) {
  size_t Pos = FileName.find_last_of(".");
  if (Pos != std::string::npos)
    return FileName.substr(0, Pos);
  return FileName;
}

static bool WriteBitcode(rs2spirv::Context &Ctxt, Module *M,
                         raw_ostream &OS, std::string &ErrMsg) {
  llvm::legacy::PassManager PassMgr;
  PassMgr.add(rs2spirv::createGlobalMergePass(true));
  PassMgr.run(*M);

  WriteBitcodeToFile(M, OS);

  return true;
}

static int convertLLVMToSPIRV() {
  LLVMContext Context;

  std::string Err;
  auto DS = getDataFileStreamer(InputFile, &Err);
  if (!DS) {
    errs() << "Fails to open input file: " << Err;
    return -1;
  }
  ErrorOr<std::unique_ptr<Module>> MOrErr =
      getStreamedBitcodeModule(InputFile, std::move(DS), Context);

  if (std::error_code EC = MOrErr.getError()) {
    errs() << "Fails to load bitcode: " << EC.message();
    return -1;
  }

  std::unique_ptr<Module> M = std::move(*MOrErr);

  if (std::error_code EC = M->materializeAll()) {
    errs() << "Fails to materialize: " << EC.message();
    return -1;
  }

  std::error_code EC;

  std::vector<char> bitcode = android::spirit::readFile<char>(InputFile);
  std::unique_ptr<bcinfo::MetadataExtractor> ME(
      new bcinfo::MetadataExtractor(bitcode.data(), bitcode.size()));

  rs2spirv::Context &Ctxt = rs2spirv::Context::getInstance();

  if (!Ctxt.Initialize(std::move(ME))) {
    return -2;
  }

  if (!OutputBitcodeFile.empty()) {
    llvm::StringRef outBCFile(OutputBitcodeFile);
    llvm::raw_fd_ostream OFS_BC(outBCFile, EC, llvm::sys::fs::F_None);
    if (!WriteBitcode(Ctxt, M.get(), OFS_BC, Err)) {
      errs() << "compiler error: " << Err << '\n';
      return -3;
    }
    return 0;
  }

  if (OutputFile.empty()) {
    if (InputFile == "-")
      OutputFile = "-";
    else
      OutputFile = removeExt(InputFile) + kExt::SPIRVBinary;
  }

  llvm::StringRef outFile(OutputFile);
  llvm::raw_fd_ostream OFS(outFile, EC, llvm::sys::fs::F_None);

  if (!rs2spirv::WriteSPIRV(Ctxt, M.get(), OFS, Err)) {
    errs() << "compiler error: " << Err << '\n';
    return -4;
  }

  return 0;
}

int main(int ac, char **av) {
  EnablePrettyStackTrace();
  sys::PrintStackTraceOnErrorSignal(av[0]);
  PrettyStackTraceProgram X(ac, av);

  cl::ParseCommandLineOptions(ac, av, "RenderScript to SPIRV translator");

  return convertLLVMToSPIRV();
}
