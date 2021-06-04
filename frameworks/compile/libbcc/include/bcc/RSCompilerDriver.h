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

#ifndef BCC_RS_COMPILER_DRIVER_H
#define BCC_RS_COMPILER_DRIVER_H

#include "bcc/Compiler.h"
#include "bcc/Script.h"

#include "bcinfo/MetadataExtractor.h"

#include <list>
#include <string>
#include <vector>

namespace bcc {

class BCCContext;
class CompilerConfig;
class RSCompilerDriver;
class Source;

// Type signature for dynamically loaded initialization of an RSCompilerDriver.
typedef void (*RSCompilerDriverInit_t) (bcc::RSCompilerDriver *);
// Name of the function that we attempt to dynamically load/execute.
#define RS_COMPILER_DRIVER_INIT_FN rsCompilerDriverInit

class RSCompilerDriver {
private:
  CompilerConfig *mConfig;
  Compiler mCompiler;

  // Are we compiling under an RS debug context with additional checks?
  bool mDebugContext;

  // Callback before linking with the runtime library.
  RSLinkRuntimeCallback mLinkRuntimeCallback;

  // Do we merge global variables on ARM using LLVM's optimization pass?
  // Disabling LLVM's global merge pass allows static globals to be correctly
  // emitted to ELF. This can result in decreased performance due to increased
  // register pressure, but it does make the resulting code easier to debug
  // and work with.
  bool mEnableGlobalMerge;

  // Specifies whether we should embed global variable information in the
  // code via special RS variables that can be examined later by the driver.
  bool mEmbedGlobalInfo;

  // Specifies whether we should skip constant (immutable) global variables
  // when potentially embedding information about globals.
  bool mEmbedGlobalInfoSkipConstant;

  // Setup the compiler config for the given script. Return true if mConfig has
  // been changed and false if it remains unchanged.
  bool setupConfig(const Script &pScript);

  // Compiles the provided bitcode, placing the binary at pOutputPath.
  // - If pDumpIR is true, a ".ll" file will also be created.
  Compiler::ErrorCode compileScript(Script& pScript, const char* pScriptName,
                                    const char* pOutputPath,
                                    const char* pRuntimePath,
                                    const char* pBuildChecksum,
                                    bool pDumpIR);

public:
  RSCompilerDriver();
  ~RSCompilerDriver();

  Compiler *getCompiler() {
    return &mCompiler;
  }

  void setConfig(CompilerConfig *config) {
    mConfig = config;
  }

  void setDebugContext(bool v) {
    mDebugContext = v;
  }

  void setLinkRuntimeCallback(RSLinkRuntimeCallback c) {
    mLinkRuntimeCallback = c;
  }

  RSLinkRuntimeCallback getLinkRuntimeCallback() const {
    return mLinkRuntimeCallback;
  }

  // This function enables/disables merging of global static variables.
  // Note that it only takes effect on ARM architectures (other architectures
  // do not offer this option).
  void setEnableGlobalMerge(bool v) {
    mEnableGlobalMerge = v;
  }

  bool getEnableGlobalMerge() const {
    return mEnableGlobalMerge;
  }

  const CompilerConfig * getConfig() const {
    return mConfig;
  }

  // Set to true if we should embed global variable information in the code.
  void setEmbedGlobalInfo(bool v) {
    mEmbedGlobalInfo = v;
  }

  // Returns true if we should embed global variable information in the code.
  bool getEmbedGlobalInfo() const {
    return mEmbedGlobalInfo;
  }

  // Set to true if we should skip constant (immutable) global variables when
  // potentially embedding information about globals.
  void setEmbedGlobalInfoSkipConstant(bool v) {
    mEmbedGlobalInfoSkipConstant = v;
  }

  // Returns true if we should skip constant (immutable) global variables when
  // potentially embedding information about globals.
  bool getEmbedGlobalInfoSkipConstant() const {
    return mEmbedGlobalInfoSkipConstant;
  }

  // FIXME: This method accompany with loadScript and compileScript should
  //        all be const-methods. They're not now because the getAddress() in
  //        SymbolResolverInterface is not a const-method.
  // Returns true if script is successfully compiled.
  bool build(BCCContext& pContext, const char* pCacheDir, const char* pResName,
             const char* pBitcode, size_t pBitcodeSize,
             const char *pBuildChecksum, const char* pRuntimePath,
             RSLinkRuntimeCallback pLinkRuntimeCallback = nullptr,
             bool pDumpIR = false);

  bool buildScriptGroup(
      BCCContext& Context, const char* pOutputFilepath, const char* pRuntimePath,
      const char* pRuntimeRelaxedPath, bool dumpIR, const char* buildChecksum,
      const std::vector<Source*>& sources,
      const std::list<std::list<std::pair<int, int>>>& toFuse,
      const std::list<std::string>& fused,
      const std::list<std::list<std::pair<int, int>>>& invokes,
      const std::list<std::string>& invokeBatchNames);

  // Returns true if script is successfully compiled.
  bool buildForCompatLib(Script &pScript, const char *pOut,
                         const char *pBuildChecksum, const char *pRuntimePath,
                         bool pDumpIR);
};

} // end namespace bcc

#endif // BCC_RS_COMPILER_DRIVER_H
