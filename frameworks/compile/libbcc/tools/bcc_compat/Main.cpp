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

#include <string>
#include <vector>

#include <stdlib.h>

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Config/config.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

#include <bcc/BCCContext.h>
#include <bcc/CompilerConfig.h>
#include <bcc/Config.h>
#include <bcc/Initialization.h>
#include <bcc/RSCompilerDriver.h>
#include <bcc/Source.h>

using namespace bcc;

//===----------------------------------------------------------------------===//
// General Options
//===----------------------------------------------------------------------===//
namespace {

llvm::cl::list<std::string>
OptInputFilenames(llvm::cl::Positional, llvm::cl::OneOrMore,
                  llvm::cl::desc("<input bitcode files>"));

llvm::cl::opt<std::string>
OptOutputFilename("o", llvm::cl::desc("Specify the output filename"),
                  llvm::cl::value_desc("filename"));

llvm::cl::opt<std::string>
OptRuntimePath("rt-path", llvm::cl::desc("Specify the runtime library path"),
               llvm::cl::value_desc("path"));

llvm::cl::opt<std::string>
OptTargetTriple("mtriple",
                llvm::cl::desc("Specify the target triple (default: "
                               DEFAULT_TARGET_TRIPLE_STRING ")"),
                llvm::cl::init(DEFAULT_TARGET_TRIPLE_STRING),
                llvm::cl::value_desc("triple"));

llvm::cl::alias OptTargetTripleC("C", llvm::cl::NotHidden,
                                 llvm::cl::desc("Alias for -mtriple"),
                                 llvm::cl::aliasopt(OptTargetTriple));

//===----------------------------------------------------------------------===//
// Compiler Options
//===----------------------------------------------------------------------===//
llvm::cl::opt<bool>
OptPIC("fPIC", llvm::cl::desc("Generate fully relocatable, position independent"
                              " code"));

llvm::cl::opt<char>
OptOptLevel("O", llvm::cl::desc("Optimization level. [-O0, -O1, -O2, or -O3] "
                                "(default: -O2)"),
            llvm::cl::Prefix, llvm::cl::ZeroOrMore, llvm::cl::init('2'));

llvm::cl::opt<bool>
OptC("c", llvm::cl::desc("Compile and assemble, but do not link."));

//===----------------------------------------------------------------------===//
// Linker Options
//===----------------------------------------------------------------------===//
// FIXME: this option will be removed in the future when MCLinker is capable
//        of generating shared library directly from given bitcode. It only
//        takes effect when -shared is supplied.
llvm::cl::opt<std::string>
OptImmObjectOutput("or", llvm::cl::desc("Specify the filename for output the "
                                        "intermediate relocatable when linking "
                                        "the input bitcode to the shared "
                                        "library"), llvm::cl::ValueRequired);

llvm::cl::opt<bool>
OptShared("shared", llvm::cl::desc("Create a shared library from input bitcode "
                                   "files"));


// Override "bcc -version" since the LLVM version information is not correct on
// Android build.
void BCCVersionPrinter() {
  llvm::raw_ostream &os = llvm::outs();
  os << "libbcc (The Android Open Source Project, http://www.android.com/):\n"
     << "  Default target: " << DEFAULT_TARGET_TRIPLE_STRING << "\n\n"
     << "LLVM (http://llvm.org/):\n"
     << "  Version: " << PACKAGE_VERSION << "\n";
  return;
}

} // end anonymous namespace

Script *PrepareScript(BCCContext &pContext,
                      const llvm::cl::list<std::string> &pBitcodeFiles) {
  Script *result = nullptr;

  for (unsigned i = 0; i < pBitcodeFiles.size(); i++) {
    const std::string &input_bitcode = pBitcodeFiles[i];
    Source *source = Source::CreateFromFile(pContext, input_bitcode);
    if (source == nullptr) {
      llvm::errs() << "Failed to load llvm module from file `" << input_bitcode
                   << "'!\n";
      return nullptr;
    }

    if (result != nullptr) {
      if (!result->mergeSource(*source)) {
        llvm::errs() << "Failed to merge the llvm module `" << input_bitcode
                     << "' to compile!\n";
        delete source;
        return nullptr;
      }
    } else {
      result = new (std::nothrow) Script(source);
      if (result == nullptr) {
        llvm::errs() << "Out of memory when create script for file `"
                     << input_bitcode << "'!\n";
        delete source;
        return nullptr;
      }
    }
  }

  return result;
}

static inline
bool ConfigCompiler(RSCompilerDriver &pCompilerDriver) {
  Compiler *compiler = pCompilerDriver.getCompiler();
  CompilerConfig *config = nullptr;

  config = new (std::nothrow) CompilerConfig(OptTargetTriple);
  if (config == nullptr) {
    llvm::errs() << "Out of memory when create the compiler configuration!\n";
    return false;
  }

  // Explicitly set ARM feature vector
  if (config->getTriple().find("arm") != std::string::npos) {
    std::vector<std::string> fv;
    fv.push_back("+vfp3");
    fv.push_back("+d16");
    fv.push_back("-neon");
    fv.push_back("-neonfp");
    config->setFeatureString(fv);
  }

  // Explicitly set X86 feature vector
  if ((config->getTriple().find("i686") != std::string::npos) ||
    (config->getTriple().find("x86_64") != std::string::npos)) {
    std::vector<std::string> fv;
    fv.push_back("+sse3");
    config->setFeatureString(fv);
  }

  // Compatibility mode on x86 requires atom code generation.
  if (config->getTriple().find("i686") != std::string::npos) {
    config->setCPU("atom");
  }

  // Setup the config according to the value of command line option.
  if (OptPIC) {
    config->setRelocationModel(llvm::Reloc::PIC_);
    // For x86_64, CodeModel needs to be small if PIC_ reloc is used.
    // Otherwise, we end up with TEXTRELs in the shared library.
    if (config->getTriple().find("x86_64") != std::string::npos) {
        config->setCodeModel(llvm::CodeModel::Small);
    }
  }
  switch (OptOptLevel) {
    case '0': config->setOptimizationLevel(llvm::CodeGenOpt::None); break;
    case '1': config->setOptimizationLevel(llvm::CodeGenOpt::Less); break;
    case '3': config->setOptimizationLevel(llvm::CodeGenOpt::Aggressive); break;
    case '2':
    default: {
      config->setOptimizationLevel(llvm::CodeGenOpt::Default);
      break;
    }
  }

  pCompilerDriver.setConfig(config);
  Compiler::ErrorCode result = compiler->config(*config);

  if (result != Compiler::kSuccess) {
    llvm::errs() << "Failed to configure the compiler! (detail: "
                 << Compiler::GetErrorString(result) << ")\n";
    return false;
  }

  return true;
}

#define DEFAULT_OUTPUT_PATH   "/sdcard/a.out"
static inline
std::string DetermineOutputFilename(const std::string &pOutputPath) {
  if (!pOutputPath.empty()) {
    return pOutputPath;
  }

  // User doesn't specify the value to -o.
  if (OptInputFilenames.size() > 1) {
    llvm::errs() << "Use " DEFAULT_OUTPUT_PATH " for output file!\n";
    return DEFAULT_OUTPUT_PATH;
  }

  // There's only one input bitcode file.
  const std::string &input_path = OptInputFilenames[0];
  llvm::SmallString<200> output_path(input_path);

  std::error_code err = llvm::sys::fs::make_absolute(output_path);
  if (err) {
    llvm::errs() << "Failed to determine the absolute path of `" << input_path
                 << "'! (detail: " << err.message() << ")\n";
    return "";
  }

  if (OptC) {
    // -c was specified. Replace the extension to .o.
    llvm::sys::path::replace_extension(output_path, "o");
  } else {
    // Use a.out under current working directory when compile executable or
    // shared library.
    llvm::sys::path::remove_filename(output_path);
    llvm::sys::path::append(output_path, "a.out");
  }

  return output_path.c_str();
}

int main(int argc, char **argv) {
  llvm::cl::SetVersionPrinter(BCCVersionPrinter);
  llvm::cl::ParseCommandLineOptions(argc, argv);
  init::Initialize();

  if (OptRuntimePath.empty()) {
    fprintf(stderr, "You must set \"-rt-path </path/to/libclcore.bc>\" with "
                    "this tool\n");
    return EXIT_FAILURE;
  }

  BCCContext context;
  RSCompilerDriver rscd;
  Compiler compiler;

  if (!ConfigCompiler(rscd)) {
    return EXIT_FAILURE;
  }

  std::string OutputFilename = DetermineOutputFilename(OptOutputFilename);
  if (OutputFilename.empty()) {
    return EXIT_FAILURE;
  }

  std::unique_ptr<Script> s(PrepareScript(context, OptInputFilenames));
  if (!rscd.buildForCompatLib(*s, OutputFilename.c_str(), nullptr, OptRuntimePath.c_str(), false)) {
    fprintf(stderr, "Failed to compile script!");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
